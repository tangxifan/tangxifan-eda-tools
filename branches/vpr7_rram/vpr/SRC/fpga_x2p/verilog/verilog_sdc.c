/***********************************/
/*      SPICE Modeling for VPR     */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>

/* Include vpr structs*/
#include "util.h"
#include "physical_types.h"
#include "vpr_types.h"
#include "globals.h"
#include "rr_graph_util.h"
#include "rr_graph.h"
#include "rr_graph2.h"
#include "route_common.h"
#include "vpr_utils.h"

/* Include SPICE support headers*/
#include "linkedlist.h"
#include "fpga_x2p_types.h"
#include "fpga_x2p_utils.h"
#include "fpga_x2p_backannotate_utils.h"
#include "fpga_x2p_mux_utils.h"
#include "fpga_x2p_pbtypes_utils.h"
#include "fpga_x2p_bitstream_utils.h"
#include "fpga_x2p_rr_graph_utils.h"
#include "fpga_x2p_globals.h"

/* Include Verilog support headers*/
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_routing.h"
#include "verilog_tcl_utils.h"

/* options for report timing */
typedef struct s_sdc_opts t_sdc_opts;
struct s_sdc_opts {
  char* sdc_dir;
  boolean break_loops;
};

/* TO avoid combinational loops caused by memories
 * We disable all the timing paths starting from an output of memory cell
 */
void verilog_generate_sdc_break_loop_sram(FILE* fp, 
                                          t_sram_orgz_info* cur_sram_orgz_info) {
  t_spice_model* mem_model = NULL;
  int iport, ipin; 
  char* port_name = NULL;

  int num_output_ports = 0;
  t_spice_model_port** output_ports = NULL;

  /* Check */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
              "(FILE:%s,LINE[%d])Invalid file handler!\n",
              __FILE__, __LINE__); 
    exit(1);
  }

  get_sram_orgz_info_mem_model(cur_sram_orgz_info, &mem_model);
  assert (NULL != mem_model);
  
  /* Find the output ports of mem_model */
  output_ports = find_spice_model_ports(mem_model, SPICE_MODEL_PORT_OUTPUT, &num_output_ports, TRUE);

  for (iport = 0; iport < num_output_ports; iport++) {
    for (ipin = 0; ipin < output_ports[iport]->size; ipin++) { 
      if (TRUE == mem_model->dump_explicit_port_map) {
        port_name = output_ports[iport]->lib_name;
      } else {
        port_name = output_ports[iport]->prefix;
      }
      /* Disable the timing for all the memory cells */
      fprintf(fp, 
              "set_disable_timing [get_pins -filter \"name == %s",
               port_name);
      if (1 < output_ports[iport]->size) {
        fprintf(fp, "[%d]", ipin);
      }
      fprintf(fp, "\" ");
      fprintf(fp, 
              "-of [get_cells -hier -filter \"ref_lib_cell_name == %s\"]]\n",
              mem_model->name);
    }
  }

  /* Free */
  my_free(output_ports);

  return;
}

/* Statisitcs for input sizes and structures of MUXes 
 * used in FPGA architecture
 * Disable timing starting from any MUX outputs 
 */
void verilog_generate_sdc_break_loop_mux(FILE* fp,
                                         int num_switch,
                                         t_switch_inf* switches,
                                         t_spice* spice,
                                         t_det_routing_arch* routing_arch) {
  /* We have linked list whichs stores spice model information of multiplexer*/
  t_llist* muxes_head = NULL; 
  t_llist* temp = NULL;
  t_spice_mux_model* cur_spice_mux_model = NULL;

  int num_input_ports = 0;
  t_spice_model_port** input_ports = NULL;
  int num_output_ports = 0;
  t_spice_model_port** output_ports = NULL;

  char* SPC_cell_suffix = "_SPC";

  /* Check */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
              "(FILE:%s,LINE[%d])Invalid file handler!\n",
              __FILE__, __LINE__); 
    exit(1);
  } 

  /* Alloc the muxes*/
  muxes_head = stats_spice_muxes(num_switch, switches, spice, routing_arch);

  /* Print mux netlist one by one*/
  temp = muxes_head;
  while(temp) {
    assert(NULL != temp->dptr);
    cur_spice_mux_model = (t_spice_mux_model*)(temp->dptr);
    input_ports = find_spice_model_ports(cur_spice_mux_model->spice_model, SPICE_MODEL_PORT_INPUT, &num_input_ports, TRUE);
    output_ports = find_spice_model_ports(cur_spice_mux_model->spice_model, SPICE_MODEL_PORT_OUTPUT, &num_output_ports, TRUE);
    assert(1 == num_input_ports);
    assert(1 == num_input_ports);
    /* Check the Input port size */
    if ( (NULL != cur_spice_mux_model->spice_model->verilog_netlist) 
      && (cur_spice_mux_model->size != input_ports[0]->size)) {
      vpr_printf(TIO_MESSAGE_ERROR, 
                 "(File:%s,[LINE%d])User-defined MUX SPICE MODEL(%s) size(%d) unmatch with the architecture needs(%d)!\n",
                 __FILE__, __LINE__, cur_spice_mux_model->spice_model->name, input_ports[0]->size,cur_spice_mux_model->size);
      exit(1);
    }
    /* Use the user defined ports, Bypass LUT MUXes */
    if (SPICE_MODEL_MUX == cur_spice_mux_model->spice_model->type) {
      fprintf(fp, 
              "set_disable_timing [get_pins -filter \"name =~ %s*\" ",
              output_ports[0]->prefix);
      fprintf(fp, 
              "-of [get_cells -hier -filter \"ref_lib_cell_name == %s\"]]\n",
              gen_verilog_one_mux_module_name(cur_spice_mux_model->spice_model, cur_spice_mux_model->size));
      /* For SPC cells*/
      fprintf(fp, 
              "set_disable_timing [get_pins -filter \"name =~ %s*\" ",
              output_ports[0]->prefix);
      fprintf(fp, 
              "-of [get_cells -hier -filter \"ref_lib_cell_name == %s%s*\"]]\n",
              gen_verilog_one_mux_module_name(cur_spice_mux_model->spice_model, cur_spice_mux_model->size),
              SPC_cell_suffix);
    }
    /* Free */
    my_free(output_ports);
    my_free(input_ports);
    /* Move on to the next*/
    temp = temp->next;
  }

  /* remember to free the linked list*/
  free_muxes_llist(muxes_head);

  return;
}

void verilog_generate_sdc_clock_period(t_sdc_opts sdc_opts) {
  FILE* fp = NULL;
  char* fname = my_strcat(sdc_opts.sdc_dir, sdc_clock_period_file_name);
  t_llist* temp = NULL;
  t_spice_model_port* temp_port = NULL;
  int ipin;
  float clock_period = 10.;

  vpr_printf(TIO_MESSAGE_INFO, 
             "Generating SDC for constraining clocks in P&R flow: %s ...\n",
             fname);
  
  /* Print the muxes netlist*/
  fp = fopen(fname, "w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
              "(FILE:%s,LINE[%d])Failure in create SDC constraints %s",
              __FILE__, __LINE__, fname); 
    exit(1);
  } 
  /* Generate the descriptions*/
  dump_verilog_sdc_file_header(fp, "Clock contraints for PnR");

  /* Find the global clock ports */
  temp = global_ports_head; 
  while (NULL != temp) {
    /* Get the port */
    temp_port = (t_spice_model_port*)(temp->dptr);
    /* We only care clock ports */
    if (SPICE_MODEL_PORT_CLOCK == temp_port->type) {
      /* Go to next */
      temp = temp->next;
      continue;
    }
    /* Create clock */
    for (ipin = 0; ipin < temp_port->size; ipin++) {
      fprintf(fp, 
              "create_clock -name {%s[%d]} -period %.2g -waveform {0.00 %.2g} [list [get_ports {%s[%d]}]]\n",
              temp_port->prefix, ipin,
              clock_period, clock_period/2,
              temp_port->prefix, ipin);
      fprintf(fp, 
              "set_drive 0 %s[%d]\n", 
              temp_port->prefix, ipin);
    }
    /* Go to next */
    temp = temp->next;
  }

  /* close file */
  fclose(fp);

  return;
}

void verilog_generate_sdc_break_loops(t_sram_orgz_info* cur_sram_orgz_info,
                                      t_sdc_opts sdc_opts,
                                      int num_switch,
                                      t_switch_inf* switches,
                                      t_spice* spice,
                                      t_det_routing_arch* routing_arch) {
  FILE* fp = NULL;
  char* fname = my_strcat(sdc_opts.sdc_dir, sdc_break_loop_file_name);

  vpr_printf(TIO_MESSAGE_INFO, 
             "Generating SDC for breaking combinational loops in P&R flow: %s ...\n",
             fname);

  /* Print the muxes netlist*/
  fp = fopen(fname, "w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
              "(FILE:%s,LINE[%d])Failure in create SDC constraints %s",
              __FILE__, __LINE__, fname); 
    exit(1);
  } 
  /* Generate the descriptions*/
  dump_verilog_sdc_file_header(fp, "Break Combinational Loops for PnR");

  /* 1. Break loops from Memory Cells */
  verilog_generate_sdc_break_loop_sram(fp, cur_sram_orgz_info);

  /* 2. Break loops from Multiplexer Output */
  verilog_generate_sdc_break_loop_mux(fp, num_switch, switches, spice, routing_arch);

  /* Close the file*/
  fclose(fp);

  /* Free strings */
  my_free(fname);

  return;
}

void verilog_generate_sdc_pnr(t_sram_orgz_info* cur_sram_orgz_info,
                              char* sdc_dir,
                              t_arch arch,
                              t_det_routing_arch* routing_arch,
                              int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                              t_ivec*** LL_rr_node_indices,
                              t_syn_verilog_opts fpga_verilog_opts) {
  t_sdc_opts sdc_opts;

  /* Initialize */
  sdc_opts.sdc_dir = my_strdup(sdc_dir);
  sdc_opts.break_loops = TRUE;

  /* Part 1. Constrain clock cycles */
  verilog_generate_sdc_clock_period(sdc_opts);

  /* Part 2. Output Design Constraints for breaking loops */
  if (TRUE == sdc_opts.break_loops) {
    verilog_generate_sdc_break_loops(cur_sram_orgz_info, sdc_opts, 
                                     routing_arch->num_switch, switch_inf,
                                     arch.spice,
                                     routing_arch); 
  } 

  /* Part 3. Output routing constraints for Switch Blocks */

  /* Part 4. Output routing constraints for Connection Blocks */

  /* Part 5. Output routing constraints for Programmable blocks */

  return;
}


