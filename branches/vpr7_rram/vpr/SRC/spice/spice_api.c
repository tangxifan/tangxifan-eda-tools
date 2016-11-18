/***********************************/
/*      SPICE Modeling for VPR     */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "rr_graph.h"
#include "vpr_utils.h"
#include "path_delay.h"
#include "stats.h"

/* Include spice support headers*/
#include "read_xml_spice_util.h"
#include "linkedlist.h"
#include "fpga_spice_globals.h"
#include "spice_globals.h"
#include "spice_utils.h"
#include "spice_backannotate_utils.h"
#include "spice_subckt.h"
#include "spice_pbtypes.h"
#include "spice_heads.h"
#include "spice_lut.h"
#include "spice_top_netlist.h"
#include "spice_mux_testbench.h"
#include "spice_grid_testbench.h"
#include "spice_lut_testbench.h"
#include "spice_dff_testbench.h"
#include "spice_run_scripts.h"

/* For mrFPGA */
#ifdef MRFPGA_H
#include "mrfpga_globals.h"
#endif

/* RUN HSPICE Shell Script Name */
static char* default_spice_dir_path = "spice_netlists/";
static char* spice_top_tb_dir_name = "top_tb/";
static char* spice_grid_tb_dir_name = "grid_tb/";
static char* spice_pb_mux_tb_dir_name = "pb_mux_tb/";
static char* spice_cb_mux_tb_dir_name = "cb_mux_tb/";
static char* spice_sb_mux_tb_dir_name = "sb_mux_tb/";
static char* spice_cb_tb_dir_name = "cb_tb/";
static char* spice_sb_tb_dir_name = "sb_tb/";
static char* spice_lut_tb_dir_name = "lut_tb/";
static char* spice_dff_tb_dir_name = "dff_tb/";
  
/***** Subroutines Declarations *****/
static 
void init_list_include_netlists(t_spice* spice); 

static 
void free_spice_tb_llist();

/***** Subroutines *****/
static 
void init_list_include_netlists(t_spice* spice) { 
  int i, j, cur;
  int to_include = 0;
  int num_to_include = 0;

  /* Initialize */
  for (i = 0; i < spice->num_include_netlist; i++) { 
    FreeSpiceModelNetlist(&(spice->include_netlists[i]));
  }
  my_free(spice->include_netlists);
  spice->include_netlists = NULL;
  spice->num_include_netlist = 0;

  /* Generate include netlist list */
  vpr_printf(TIO_MESSAGE_INFO, "Listing SPICE Netlist Names to be included...\n");
  for (i = 0; i < spice->num_spice_model; i++) {
    if (NULL != spice->spice_models[i].model_netlist) {
      /* Check if this netlist name has already existed in the list */
      to_include = 1;
      for (j = 0; j < i; j++) {
        if (NULL == spice->spice_models[j].model_netlist) {
          continue;
        }
        if (0 == strcmp(spice->spice_models[j].model_netlist, spice->spice_models[i].model_netlist)) {
          to_include = 0;
          break;
        }
      }
      /* Increamental */
      if (1 == to_include) {
        num_to_include++;
      }
    }
  }

  /* realloc */
  spice->include_netlists = (t_spice_model_netlist*)my_realloc(spice->include_netlists, 
                              sizeof(t_spice_model_netlist)*(num_to_include + spice->num_include_netlist));

  /* Fill the new included netlists */
  cur = spice->num_include_netlist;
  for (i = 0; i < spice->num_spice_model; i++) {
    if (NULL != spice->spice_models[i].model_netlist) {
      /* Check if this netlist name has already existed in the list */
      to_include = 1;
      for (j = 0; j < i; j++) {
        if (NULL == spice->spice_models[j].model_netlist) {
          continue;
        }
        if (0 == strcmp(spice->spice_models[j].model_netlist, spice->spice_models[i].model_netlist)) {
          to_include = 0;
          break;
        }
      }
      /* Increamental */
      if (1 == to_include) {
        spice->include_netlists[cur].path = my_strdup(spice->spice_models[i].model_netlist); 
        spice->include_netlists[cur].included = 0;
        vpr_printf(TIO_MESSAGE_INFO, "[%d] %s\n", cur+1, spice->include_netlists[cur].path);
        cur++;
      }
    }
  }
  /* Check */
  assert(cur == (num_to_include + spice->num_include_netlist));
  /* Update */
  spice->num_include_netlist += num_to_include;
  
  return;
}

static 
void free_spice_tb_llist() {
  t_llist* temp = tb_head;

  while (temp) {
    my_free(((t_spicetb_info*)(temp->dptr))->tb_name);
    my_free(temp->dptr);
    temp->dptr = NULL;
    temp = temp->next;
  }
  free_llist(tb_head);

  return;
}

/***** Main Function *****/
void vpr_print_spice_netlists(t_vpr_setup vpr_setup,
                              t_arch Arch,
                              char* circuit_name) {
  clock_t t_start;
  clock_t t_end;
  float run_time_sec;

  int num_clocks = Arch.spice->spice_params.stimulate_params.num_clocks;
  int vpr_crit_path_delay = Arch.spice->spice_params.stimulate_params.vpr_crit_path_delay;

  char* spice_dir_formatted = NULL;
  char* include_dir_path = NULL;
  char* subckt_dir_path = NULL;
  char* top_netlist_path = NULL;
  char* include_dir_name = vpr_setup.FPGA_SPICE_Opts.SpiceOpts.include_dir;
  char* subckt_dir_name = vpr_setup.FPGA_SPICE_Opts.SpiceOpts.subckt_dir;
  char* chomped_circuit_name = NULL;
  char* chomped_spice_dir = NULL;
  char* top_testbench_dir_path = NULL;
  char* pb_mux_testbench_dir_path = NULL;
  char* cb_mux_testbench_dir_path = NULL;
  char* sb_mux_testbench_dir_path = NULL;
  char* cb_testbench_dir_path = NULL;
  char* sb_testbench_dir_path = NULL;
  char* grid_testbench_dir_path = NULL;
  char* lut_testbench_dir_path = NULL;
  char* dff_testbench_dir_path = NULL;
  char* top_testbench_file = NULL;

  /* Check if the routing architecture we support*/
  if (UNI_DIRECTIONAL != vpr_setup.RoutingArch.directionality) {
    vpr_printf(TIO_MESSAGE_ERROR, "FPGA SPICE netlists only support uni-directional routing architecture!\n");
    exit(1);
  }
  
  /* We don't support mrFPGA */
#ifdef MRFPGA_H
  if (is_mrFPGA) {
    vpr_printf(TIO_MESSAGE_ERROR, "FPGA SPICE netlists do not support mrFPGA!\n");
    exit(1);
  }
#endif  

  /* Initial Arch SPICE MODELS*/
  /* zero the counter of each spice_model */
  zero_spice_models_cnt(Arch.spice->num_spice_model, Arch.spice->spice_models);

  /* Move to the top-level function: vpr_fpga_spice_tool_suits */
  /* init_check_arch_spice_models(&Arch, &vpr_setup.RoutingArch); */
  init_list_include_netlists(Arch.spice); 

  /* assign the global variable of SRAM model */
  sram_spice_model = Arch.sram_inf.spice_model;
  sram_orgz_type = Arch.sram_inf.orgz_type;
  
  /* Initialize the number of configuration bits of all the grids */
  init_grids_num_conf_bits();
  init_grids_num_iopads();

  /* Add keyword checking */
  /* Move to the top-level function: vpr_fpga_spice_tool_suits */
  /* check_keywords_conflict(Arch); */

  /*Process the circuit name*/
  split_path_prog_name(circuit_name,'/',&chomped_spice_dir ,&chomped_circuit_name);
   
  /* FPGA-SPICE formally starts*/
  vpr_printf(TIO_MESSAGE_INFO, "\nFPGA-SPICE starts...\n");
  
  /* Start Clocking*/
  t_start = clock();

  /* Format the directory path */
  if (NULL != vpr_setup.FPGA_SPICE_Opts.SpiceOpts.spice_dir) {
    spice_dir_formatted = format_dir_path(vpr_setup.FPGA_SPICE_Opts.SpiceOpts.spice_dir);
  } else {
    spice_dir_formatted = format_dir_path(my_strcat(format_dir_path(chomped_spice_dir),
                                                    default_spice_dir_path));
  }

  /*Initial directory organization*/
  /* Process include directory */
  (include_dir_path) = my_strcat(spice_dir_formatted,include_dir_name); 
  /* Process subckt directory */
  (subckt_dir_path) = my_strcat(spice_dir_formatted,subckt_dir_name);

  /* Check the spice folders exists if not we create it.*/
  create_dir_path(spice_dir_formatted);
  create_dir_path(include_dir_path);
  create_dir_path(subckt_dir_path);

  /* determine the VPR clock frequency */
  /* Move to the top-level function: vpr_fpga_spice_tool_suits */
  /* 
  vpr_crit_path_delay = get_critical_path_delay()/1e9;
  assert(vpr_crit_path_delay > 0.);
  */
  /* if we don't have global clock, clock_freqency should be set to 0.*/
  /*
  num_clocks = count_netlist_clocks();
  if (0 == num_clocks) {
     vpr_clock_freq = 0.;
  } else { 
    assert(1 == num_clocks);
    vpr_clock_freq = 1. / vpr_crit_path_delay; 
  }
  */

  /* backannotation */  
  /* Move to the top-level function: vpr_fpga_spice_tool_suits */
  /*
  spice_backannotate_vpr_post_route_info(vpr_setup.RoutingArch,
                                         vpr_setup.SpiceOpts.fpga_spice_parasitic_net_estimation_off);
  */

  /* Auto check the density and recommend sim_num_clock_cylce */
  /* Move to the top-level function: vpr_fpga_spice_tool_suits */
  /* auto_select_num_sim_clock_cycle(Arch.spice); */

  /* Generate Header files */
  fprint_spice_headers(include_dir_path, vpr_crit_path_delay, num_clocks, *(Arch.spice));

  /* Generate sub circuits: Inverter, Buffer, Transmission Gate, LUT, DFF, SRAM, MUX*/
  generate_spice_subckts(subckt_dir_path, &Arch ,&vpr_setup.RoutingArch);

  /* Print MUX testbench if needed */
  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.print_spice_pb_mux_testbench) {
    pb_mux_testbench_dir_path = my_strcat(spice_dir_formatted, spice_pb_mux_tb_dir_name);
    create_dir_path(pb_mux_testbench_dir_path);
    fprint_spice_mux_testbench(pb_mux_testbench_dir_path, chomped_circuit_name, 
                               include_dir_path, subckt_dir_path,
                               rr_node_indices, num_clocks, Arch, 
                               SPICE_PB_MUX_TB, 
                               vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_leakage_only);
  }

  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.print_spice_cb_mux_testbench) {
    cb_mux_testbench_dir_path = my_strcat(spice_dir_formatted, spice_cb_mux_tb_dir_name);
    create_dir_path(cb_mux_testbench_dir_path);
    fprint_spice_mux_testbench(cb_mux_testbench_dir_path, chomped_circuit_name,
                               include_dir_path, subckt_dir_path,
                               rr_node_indices, num_clocks, Arch, SPICE_CB_MUX_TB, 
                               vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_leakage_only);
  }

  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.print_spice_sb_mux_testbench) {
    sb_mux_testbench_dir_path = my_strcat(spice_dir_formatted, spice_sb_mux_tb_dir_name);
    create_dir_path(sb_mux_testbench_dir_path);
    fprint_spice_mux_testbench(sb_mux_testbench_dir_path, chomped_circuit_name, 
                               include_dir_path, subckt_dir_path,
                               rr_node_indices, num_clocks, Arch, SPICE_SB_MUX_TB, 
                               vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_leakage_only);
  }

  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.print_spice_cb_testbench) {
    cb_testbench_dir_path = my_strcat(spice_dir_formatted, spice_cb_tb_dir_name);
    create_dir_path(cb_testbench_dir_path);
    fprint_spice_mux_testbench(cb_testbench_dir_path, chomped_circuit_name,
                               include_dir_path, subckt_dir_path,
                               rr_node_indices, num_clocks, Arch, SPICE_CB_TB, 
                               vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_leakage_only);
  }

  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.print_spice_sb_testbench) {
    sb_testbench_dir_path = my_strcat(spice_dir_formatted, spice_sb_tb_dir_name);
    create_dir_path(sb_testbench_dir_path);
    fprint_spice_mux_testbench(sb_testbench_dir_path, chomped_circuit_name, 
                               include_dir_path, subckt_dir_path,
                               rr_node_indices, num_clocks, Arch, SPICE_SB_TB, 
                               vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_leakage_only);
  }

  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.print_spice_lut_testbench) {
    lut_testbench_dir_path = my_strcat(spice_dir_formatted, spice_lut_tb_dir_name); 
    create_dir_path(lut_testbench_dir_path);
    fprint_spice_lut_testbench(lut_testbench_dir_path, chomped_circuit_name, include_dir_path, subckt_dir_path,
                               rr_node_indices, num_clocks, Arch, vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_leakage_only);
  }

  /* By pass dff testbench file if there is no clock */
  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.print_spice_dff_testbench) {
    dff_testbench_dir_path = my_strcat(spice_dir_formatted, spice_dff_tb_dir_name); 
    create_dir_path(dff_testbench_dir_path);
    fprint_spice_dff_testbench(dff_testbench_dir_path, chomped_circuit_name, include_dir_path, subckt_dir_path,
                               rr_node_indices, num_clocks, Arch, vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_leakage_only);
  }

  /* Print Grid testbench if needed */
  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.print_spice_grid_testbench) {
    grid_testbench_dir_path = my_strcat(spice_dir_formatted, spice_grid_tb_dir_name);
    create_dir_path(grid_testbench_dir_path);
    fprint_spice_grid_testbench(grid_testbench_dir_path, chomped_circuit_name, 
                                include_dir_path, subckt_dir_path,
                                rr_node_indices, num_clocks, Arch, 
                                vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_leakage_only);
  }

  /* Print Netlists of the given FPGA*/
  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.print_spice_top_testbench) {
    top_testbench_file = my_strcat(chomped_circuit_name, spice_top_testbench_postfix);
    /* Process top_netlist_path */
    top_testbench_dir_path = my_strcat(spice_dir_formatted, spice_top_tb_dir_name); 
    create_dir_path(top_testbench_dir_path);
    top_netlist_path = my_strcat(top_testbench_dir_path, top_testbench_file); 
    fprint_spice_top_netlist(chomped_circuit_name, top_netlist_path, 
                             include_dir_path, subckt_dir_path, 
                             num_rr_nodes, rr_node, rr_node_indices, num_clocks, *(Arch.spice), 
                             vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_leakage_only);
  }

  /* Generate a shell script for running HSPICE simulations */
  fprint_run_hspice_shell_script(*(Arch.spice), spice_dir_formatted, subckt_dir_path);

  /* END Clocking*/
  t_end = clock();

  run_time_sec = (float)(t_end - t_start) / CLOCKS_PER_SEC;
  vpr_printf(TIO_MESSAGE_INFO, "SPICE netlists dumping took %g seconds\n", run_time_sec);  

  /* Free index low and high */
  free_spice_model_grid_index_low_high(Arch.spice->num_spice_model, Arch.spice->spice_models);
  free_spice_model_routing_index_low_high(Arch.spice->num_spice_model, Arch.spice->spice_models);
  /* Free tb_llist */
  free_spice_tb_llist(); 
  /* Free */
  my_free(spice_dir_formatted);
  my_free(include_dir_path);
  my_free(subckt_dir_path);

  return;
}
