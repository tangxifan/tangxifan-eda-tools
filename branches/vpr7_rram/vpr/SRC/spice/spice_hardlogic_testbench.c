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
#include "rr_graph_util.h"
#include "rr_graph.h"
#include "rr_graph2.h"
#include "vpr_utils.h"

/* Include spice support headers*/
#include "linkedlist.h"
#include "fpga_spice_globals.h"
#include "spice_globals.h"
#include "fpga_spice_utils.h"
#include "spice_utils.h"
#include "spice_mux.h"
#include "spice_pbtypes.h"
#include "spice_subckt.h"

/* local global variables */
static int tb_num_dffs = 0;
static int testbench_load_cnt = 0;
static int upbound_sim_num_clock_cycles = 2;
static int max_sim_num_clock_cycles = 2;
static int auto_select_max_sim_num_clock_cycles = TRUE;

/* Subroutines in this source file*/
static void init_spice_dff_testbench_globals(t_spice spice) {
  tb_num_dffs = 0;
  auto_select_max_sim_num_clock_cycles = spice.spice_params.meas_params.auto_select_sim_num_clk_cycle;
  upbound_sim_num_clock_cycles = spice.spice_params.meas_params.sim_num_clock_cycle + 1;
  if (FALSE == auto_select_max_sim_num_clock_cycles) {
    max_sim_num_clock_cycles = spice.spice_params.meas_params.sim_num_clock_cycle + 1;
  } else {
    max_sim_num_clock_cycles = 2;
  }
}

static 
void fprint_spice_dff_testbench_global_ports(FILE* fp, int grid_x, int grid_y, 
                                             int num_clock, 
                                             t_spice spice) {
  /* int i; */
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  /* Print generic global ports*/
  fprint_spice_generic_testbench_global_ports(fp, 
                                              sram_spice_orgz_info,
                                              global_ports_head); 
  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  /* Print generic global ports*/
  fprint_spice_generic_testbench_global_ports(fp, 
                                              sram_spice_orgz_info, 
                                              global_ports_head); 
  /* VDD Load port name */
  fprintf(fp, ".global %s\n",
               spice_tb_global_vdd_load_port_name);

  /*Global Vdds for FFs*/
  fprint_grid_global_vdds_spice_model(fp, grid_x, grid_y, SPICE_MODEL_FF, spice);

  /*Global Vdds for Hardlogics*/
  fprint_grid_global_vdds_spice_model(fp, grid_x, grid_y, SPICE_MODEL_HARDLOGIC, spice);

  /*Global Vdds for IOPADs*/
  fprint_grid_global_vdds_spice_model(fp, grid_x, grid_y, SPICE_MODEL_IOPAD, spice);

  /* Global VDDs for SRAMs of IOPADs */
  fprintf(fp, ".global %s\n",
               spice_tb_global_vdd_io_sram_port_name);

  return;
}

void fprint_spice_dff_testbench_one_dff(FILE* fp, 
                                        char* subckt_name, 
                                        t_spice_model* dff_spice_model,
                                        int num_inputs, int num_outputs,
                                        int* input_init_value, 
                                        float* input_density, 
                                        float* input_probability) {
  int ipin;
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Call defined subckt */
  fprintf(fp, "Xdff[%d] ", tb_num_dffs);

  /* Global ports */
  if (0 < rec_fprint_spice_model_global_ports(fp, dff_spice_model, FALSE)) { 
    fprintf(fp, "+ ");
  }

  assert(1 == num_inputs);
  for (ipin = 0; ipin < num_inputs; ipin++) {
    fprintf(fp, "dff[%d]->in[%d] ", tb_num_dffs, ipin);
  }
  fprintf(fp, "dff[%d]->out gvdd 0 ", tb_num_dffs);
  fprintf(fp, "%s\n", dff_spice_model->name);
  /* Stimulates */ 
  for (ipin = 0; ipin < num_inputs; ipin++) {
    fprintf(fp, "Vdff[%d]->in[%d] dff[%d]->in[%d] 0 \n",
            tb_num_dffs, ipin, tb_num_dffs, ipin);
    fprint_voltage_pulse_params(fp, input_init_value[ipin], input_density[ipin], input_probability[ipin]);
  }
  return; 
}

void fprint_spice_dff_testbench_one_pb_graph_node_dff(FILE* fp, 
                                                      t_pb_graph_node* cur_pb_graph_node, 
                                                      char* prefix,
                                                      int x, int y,
                                                      t_ivec*** LL_rr_node_indices) {
  int logical_block_index = OPEN;
  t_spice_model* pb_spice_model = NULL;
  t_pb_type* cur_pb_type = NULL;
  float* input_density = NULL;
  float* input_probability = NULL;
  int* input_init_value = NULL;
  int* input_net_num = NULL;
  int iport, ipin, cur_pin;
  int num_inputs, num_outputs, num_clock_pins;
  char* outport_name = NULL;
  t_rr_node* local_rr_graph = NULL;
  float average_density = 0.;
  int avg_density_cnt = 0;
  int num_sim_clock_cycles = 0;

  assert(NULL != cur_pb_graph_node);
  assert(NULL != prefix);

  cur_pb_type = cur_pb_graph_node->pb_type;
  assert(NULL != cur_pb_type);
  pb_spice_model = cur_pb_type->spice_model; 
 
  /* Try to find the mapped logic block index */
  logical_block_index = find_grid_mapped_logical_block(x, y, 
                                                       pb_spice_model, prefix);

  /*
  if (OPEN == logical_block_index) {
    return;
  }
  */

  /* Allocate input_density and probability */
  stats_pb_graph_node_port_pin_numbers(cur_pb_graph_node,&num_inputs,&num_outputs, &num_clock_pins);
  assert(1 == num_clock_pins);
  assert(1 == num_outputs);
  assert(1 == num_inputs);

  input_density = (float*)my_malloc(sizeof(float)*num_inputs); 
  input_probability = (float*)my_malloc(sizeof(float)*num_inputs); 
  input_init_value = (int*)my_malloc(sizeof(int)*num_inputs); 
  input_net_num = (int*)my_malloc(sizeof(int)*num_inputs); 

  /* Get activity information */
  assert(1 == cur_pb_graph_node->num_input_ports);
  cur_pin = 0;
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
      /* if we find a mapped logic block */
      if (OPEN != logical_block_index) {
        local_rr_graph = logical_block[logical_block_index].pb->parent_pb->rr_graph;
      } else {
        local_rr_graph = NULL;
      }
      input_net_num[cur_pin] = pb_pin_net_num(local_rr_graph, &(cur_pb_graph_node->input_pins[iport][ipin]));
      input_density[cur_pin] = pb_pin_density(local_rr_graph, &(cur_pb_graph_node->input_pins[iport][ipin]));
      input_probability[cur_pin] = pb_pin_probability(local_rr_graph, &(cur_pb_graph_node->input_pins[iport][ipin]));
      input_init_value[cur_pin] =  pb_pin_init_value(local_rr_graph, &(cur_pb_graph_node->input_pins[iport][ipin]));
      cur_pin++;
    } 
    assert(cur_pin == num_inputs);
  }
  /* Check lut pin net num consistency */
  if (OPEN != logical_block_index) {
    if (0 == check_consistency_logical_block_net_num(&(logical_block[logical_block_index]), num_inputs, input_net_num)) {
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])FF(name:%s) consistency check fail!\n",
                 __FILE__, __LINE__, logical_block[logical_block_index].name);
      exit(1);
    }
  }
 
  /* Call the subckt and give stimulates, measurements */
  if (OPEN != logical_block_index) {
    fprintf(fp,"***** DFF[%d]: logical_block_index[%d], gvdd_index[%d]*****\n", 
            tb_num_dffs, logical_block_index, logical_block[logical_block_index].mapped_spice_model_index);
  } else {
    fprintf(fp,"***** DFF[%d]: logical_block_index[%d], gvdd_index[%d]*****\n", 
            tb_num_dffs, -1, -1);
  }

  fprint_spice_dff_testbench_one_dff(fp, prefix, pb_spice_model, num_inputs, num_outputs,
                                     input_init_value, input_density, input_probability);

  /* Add loads: Recursively */
  outport_name = (char*)my_malloc(sizeof(char)*( 4 + strlen(my_itoa(tb_num_dffs)) 
                                  + 6 + 1 ));
  sprintf(outport_name, "dff[%d]->out",
                         tb_num_dffs);
  if (OPEN != logical_block_index) {
    fprint_spice_testbench_pb_graph_pin_inv_loads_rec(fp, &testbench_load_cnt,
                                                      x, y, 
                                                      &(cur_pb_graph_node->output_pins[0][0]), 
                                                      logical_block[logical_block_index].pb, 
                                                      outport_name, 
                                                      FALSE, 
                                                      LL_rr_node_indices); 
  } else {
    fprint_spice_testbench_pb_graph_pin_inv_loads_rec(fp, &testbench_load_cnt,
                                                      x, y, 
                                                      &(cur_pb_graph_node->output_pins[0][0]), 
                                                      NULL, 
                                                      outport_name, 
                                                      FALSE, 
                                                      LL_rr_node_indices); 
  }

  /* Calculate average density of this MUX */
  average_density = 0.;
  avg_density_cnt = 0;
  for (ipin = 0; ipin < num_inputs; ipin++) {
    assert(!(0 > input_density[ipin]));
    if (0. < input_density[ipin]) {
      average_density += input_density[ipin];
      avg_density_cnt++;
    }
  }
  /* Calculate the num_sim_clock_cycle for this MUX, update global max_sim_clock_cycle in this testbench */
  if (0 < avg_density_cnt) {
    average_density = average_density/avg_density_cnt;
  } else {
    assert(0 == avg_density_cnt);
    average_density = 0.;
  }
  if (0. == average_density) {
    num_sim_clock_cycles = 2;
  } else {
    assert(0. < average_density);
    num_sim_clock_cycles = (int)(1/average_density) + 1;
  }
  if (TRUE == auto_select_max_sim_num_clock_cycles) {
    /* for idle blocks, 2 clock cycle is well enough... */
    if (2 < num_sim_clock_cycles) {
      num_sim_clock_cycles = upbound_sim_num_clock_cycles;
    } else {
      num_sim_clock_cycles = 2;
    }
    if (max_sim_num_clock_cycles < num_sim_clock_cycles) {
      max_sim_num_clock_cycles = num_sim_clock_cycles;
    }
  } else {
    num_sim_clock_cycles = max_sim_num_clock_cycles;
  }

  /* Mark temporary used */
  if (OPEN != logical_block_index) {
    logical_block[logical_block_index].temp_used = 1;
  }
  tb_num_dffs++;

  /* Free */
  my_free(input_net_num);
  my_free(input_init_value);
  my_free(input_density);
  my_free(input_probability);
  
  return; 
}

void fprint_spice_dff_testbench_rec_pb_graph_node_dffs(FILE* fp,
                                                       t_pb_graph_node* cur_pb_graph_node, 
                                                       char* prefix,
                                                       int x, int y,
                                                       t_ivec*** LL_rr_node_indices) {
  char* formatted_prefix = format_spice_node_prefix(prefix); 
  int ipb, jpb, mode_index; 
  t_pb_type* cur_pb_type = NULL;
  char* rec_prefix = NULL;
  
  assert(NULL != cur_pb_graph_node);
  cur_pb_type = cur_pb_graph_node->pb_type;
  assert(NULL != cur_pb_type);
  /* Until we reach a FF */
  if (NULL != cur_pb_type->spice_model) {
    if (SPICE_MODEL_FF != cur_pb_type->spice_model->type) {
      return;
    }
    /* Generate rec_prefix */
    rec_prefix = (char*)my_malloc(sizeof(char) * (strlen(formatted_prefix) 
                   + strlen(cur_pb_type->name) + 1 
                   + strlen(my_itoa(cur_pb_graph_node->placement_index))
                   + 1 + 1));
    sprintf(rec_prefix, "%s%s[%d]",
            formatted_prefix, cur_pb_type->name, cur_pb_graph_node->placement_index);
    /* Print a dff tb: call spice_model, stimulates */
    fprint_spice_dff_testbench_one_pb_graph_node_dff(fp, cur_pb_graph_node, rec_prefix, x, y, LL_rr_node_indices);
    my_free(rec_prefix);
    return;
  }

  /* Go recursively ... */
  mode_index = find_pb_type_idle_mode_index(*(cur_pb_type));
  for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Generate rec_prefix */
      rec_prefix = (char*)my_malloc(sizeof(char) * (strlen(formatted_prefix) 
                     + strlen(cur_pb_type->name) + 1 
                     + strlen(my_itoa(cur_pb_graph_node->placement_index)) + 7 
                     + strlen(cur_pb_type->modes[mode_index].name) + 1 + 1));
      sprintf(rec_prefix, "%s%s[%d]_mode[%s]",
              formatted_prefix, cur_pb_type->name, cur_pb_graph_node->placement_index,
              cur_pb_type->modes[mode_index].name);
      /* Go recursively */
      fprint_spice_dff_testbench_rec_pb_graph_node_dffs(fp, &(cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb]),
                                                        rec_prefix, x, y, LL_rr_node_indices);
      my_free(rec_prefix);
    }
  }
  
  return;
}

void fprint_spice_dff_testbench_rec_pb_dffs(FILE* fp, 
                                            t_pb* cur_pb, char* prefix, 
                                            int x, int y,
                                            t_ivec*** LL_rr_node_indices) {
  char* formatted_prefix = format_spice_node_prefix(prefix); 
  int ipb, jpb;
  int mode_index;
  char* rec_prefix = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert(NULL != cur_pb);

  /* If we touch the leaf, there is no need print interc*/
  if (NULL != cur_pb->pb_graph_node->pb_type->spice_model) {
    if (SPICE_MODEL_FF != cur_pb->pb_graph_node->pb_type->spice_model->type) {
      return;
    }
    /* Generate rec_prefix */
    rec_prefix = (char*)my_malloc(sizeof(char) * (strlen(formatted_prefix) 
                   + strlen(cur_pb->pb_graph_node->pb_type->name) + 1 
                   + strlen(my_itoa(cur_pb->pb_graph_node->placement_index))
                   + 1 + 1));
    sprintf(rec_prefix, "%s%s[%d]",
            formatted_prefix, cur_pb->pb_graph_node->pb_type->name, cur_pb->pb_graph_node->placement_index);
    /* Print a lut tb: call spice_model, stimulates */
    fprint_spice_dff_testbench_one_pb_graph_node_dff(fp, cur_pb->pb_graph_node, rec_prefix, x, y, LL_rr_node_indices);
    my_free(rec_prefix);
    return;
  }
  
  /* Go recursively ... */
  mode_index = cur_pb->mode;
  if (!(0 < cur_pb->pb_graph_node->pb_type->num_modes)) {
    return;
  }
  for (ipb = 0; ipb < cur_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb->pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Generate rec_prefix */
      rec_prefix = (char*)my_malloc(sizeof(char) * (strlen(formatted_prefix) 
                     + strlen(cur_pb->pb_graph_node->pb_type->name) + 1 
                     + strlen(my_itoa(cur_pb->pb_graph_node->placement_index)) + 7 
                     + strlen(cur_pb->pb_graph_node->pb_type->modes[mode_index].name) + 1 + 1));
      sprintf(rec_prefix, "%s%s[%d]_mode[%s]",
              formatted_prefix, cur_pb->pb_graph_node->pb_type->name, 
              cur_pb->pb_graph_node->placement_index,
              cur_pb->pb_graph_node->pb_type->modes[mode_index].name);
      /* Refer to pack/output_clustering.c [LINE 392] */
      if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
        fprint_spice_dff_testbench_rec_pb_dffs(fp, &(cur_pb->child_pbs[ipb][jpb]), rec_prefix, x, y, LL_rr_node_indices);
      } else {
        /* Print idle graph_node muxes */
        /* Bypass unused blocks */
        fprint_spice_dff_testbench_rec_pb_graph_node_dffs(fp, cur_pb->child_pbs[ipb][jpb].pb_graph_node, 
                                                          rec_prefix, x, y, LL_rr_node_indices);
      }
    }
  }
  
  return;
}

void fprint_spice_dff_testbench_call_one_grid_defined_dffs(FILE* fp,
                                                           int ix, int iy,
                                                           t_ivec*** LL_rr_node_indices) {
  int iblk;
  char* prefix = NULL;
 
  if (NULL == grid[ix][iy].type) {
    return; 
  }

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  for (iblk = 0; iblk < grid[ix][iy].usage; iblk++) {
    prefix = (char*)my_malloc(sizeof(char)* (5 
                    + strlen(my_itoa(block[grid[ix][iy].blocks[iblk]].x)) 
                    + 2 + strlen(my_itoa(block[grid[ix][iy].blocks[iblk]].y)) 
                    + 3 ));
    sprintf(prefix, "grid[%d][%d]_", 
            block[grid[ix][iy].blocks[iblk]].x,
            block[grid[ix][iy].blocks[iblk]].y);
    /* Only for mapped block */
    assert(NULL != block[grid[ix][iy].blocks[iblk]].pb);
    /* Mark the temporary net_num for the type pins*/
    mark_grid_type_pb_graph_node_pins_temp_net_num(ix, iy);
    fprint_spice_dff_testbench_rec_pb_dffs(fp, block[grid[ix][iy].blocks[iblk]].pb, prefix, ix, iy, LL_rr_node_indices);
    my_free(prefix);
  }
  /* Bypass unused blocks */
  for (iblk = grid[ix][iy].usage; iblk < grid[ix][iy].type->capacity; iblk++) {
    prefix = (char*)my_malloc(sizeof(char)* (5 + strlen(my_itoa(ix)) 
                              + 2 + strlen(my_itoa(iy)) + 3 ));
    sprintf(prefix, "grid[%d][%d]_", ix, iy);
    assert(NULL != grid[ix][iy].type->pb_graph_head);
    /* Mark the temporary net_num for the type pins*/
    mark_grid_type_pb_graph_node_pins_temp_net_num(ix, iy);
    fprint_spice_dff_testbench_rec_pb_graph_node_dffs(fp, grid[ix][iy].type->pb_graph_head, prefix, ix, iy, LL_rr_node_indices); 
    my_free(prefix);
  }

  return;
}

void fprint_spice_dff_testbench_call_defined_dffs(FILE* fp, t_ivec*** LL_rr_node_indices) {
  int ix, iy;
 
  for (ix = 1; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      fprint_spice_dff_testbench_call_one_grid_defined_dffs(fp, ix, iy, LL_rr_node_indices);
    }
  }

  return;
}

static 
void fprint_spice_hardlogic_testbench_stimulations(FILE* fp, int grid_x, int grid_y, 
                                                   int num_clocks, 
                                                   t_spice spice, 
                                                   t_ivec*** LL_rr_node_indices) {
  /* Print generic stimuli */
  fprint_spice_testbench_generic_global_ports_stimuli(fp, num_clocks);
  
  /* Generate global ports stimuli */
  fprint_spice_testbench_global_ports_stimuli(fp, global_ports_head);

  /* SRAM ports */
  fprintf(fp, "***** Global Inputs for SRAMs *****\n");
  fprint_spice_testbench_global_sram_inport_stimuli(fp, sram_spice_orgz_info);

  fprintf(fp, "***** Global VDD for SRAMs *****\n");
  fprint_spice_testbench_global_vdd_port_stimuli(fp,
                                                 spice_tb_global_vdd_sram_port_name,
                                                 "vsp");

  fprintf(fp, "***** Global VDD for load inverters *****\n");
  fprint_spice_testbench_global_vdd_port_stimuli(fp,
                                                 spice_tb_global_vdd_load_port_name,
                                                 "vsp");

  fprintf(fp, "***** Global VDD for IOPADs *****\n");
  fprint_spice_testbench_global_vdd_port_stimuli(fp,
                                                 spice_tb_global_vdd_io_port_name,
                                                 "vsp");

  fprintf(fp, "***** Global VDD for IOPAD SRAMs *****\n");
  fprint_spice_testbench_global_vdd_port_stimuli(fp,
                                                 spice_tb_global_vdd_io_sram_port_name,
                                                 "vsp");

  /* Every LUT use an independent Voltage source */
  fprintf(fp, "***** Global VDD for FFs *****\n");
  fprint_grid_splited_vdds_spice_model(fp, grid_x, grid_y, SPICE_MODEL_FF, spice);
 
  return;
}

void fprint_spice_hardlogic_testbench_measurements(FILE* fp, int grid_x, int grid_y, 
                                                   t_spice spice, 
                                                   boolean leakage_only) {
 
  /* int i; */
  /* First cycle reserved for measuring leakage */
  int num_clock_cycle = max_sim_num_clock_cycles;
  
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  
  fprint_spice_netlist_transient_setting(fp, spice, num_clock_cycle, leakage_only);

  /* TODO: Measure the delay of each mapped net and logical block */

  /* Measure the power */
  /* Leakage ( the first cycle is reserved for leakage measurement) */
  /* Leakage power of FFs*/
  fprint_measure_grid_vdds_spice_model(fp, grid_x, grid_y, SPICE_MODEL_FF, SPICE_MEASURE_LEAKAGE_POWER, num_clock_cycle, spice, leakage_only);

  if (TRUE == leakage_only) {
    return;
  }

  /* Dynamic power */
  /* Dynamic power of FFs */
  fprint_measure_grid_vdds_spice_model(fp, grid_x, grid_y, SPICE_MODEL_FF, SPICE_MEASURE_DYNAMIC_POWER, num_clock_cycle, spice, leakage_only);

  /* Dynamic power of Hardlogics */
  fprint_measure_grid_vdds_spice_model(fp, grid_x, grid_y, SPICE_MODEL_HARDLOGIC, SPICE_MEASURE_DYNAMIC_POWER, num_clock_cycle, spice, leakage_only);

  /* Dynamic power of IOPAD */
  fprint_measure_grid_vdds_spice_model(fp, grid_x, grid_y, SPICE_MODEL_IOPAD, SPICE_MEASURE_DYNAMIC_POWER, num_clock_cycle, spice, leakage_only);

  /* TODO: Dynamic power of SRAMs of IOPAD */

  return;
}

/* Top-level function in this source file */
int fprint_spice_one_dff_testbench(char* formatted_spice_dir,
                                   char* circuit_name,
                                   char* dff_testbench_name,
                                   char* include_dir_path,
                                   char* subckt_dir_path,
                                   t_ivec*** LL_rr_node_indices,
                                   int num_clock,
                                   t_arch arch,
                                   int grid_x, int grid_y,
                                   boolean leakage_only) {
  FILE* fp = NULL;
  char* formatted_subckt_dir_path = format_dir_path(subckt_dir_path);
  char* temp_include_file_path = NULL;
  char* title = my_strcat("FPGA DFF Testbench for Design: ", circuit_name);
  char* dff_testbench_file_path = my_strcat(formatted_spice_dir, dff_testbench_name);
  int used;

  /* Check if the path exists*/
  fp = fopen(dff_testbench_file_path,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create DFF Testbench SPICE netlist %s!",__FILE__, __LINE__, dff_testbench_file_path); 
    exit(1);
  } 
  
  /* vpr_printf(TIO_MESSAGE_INFO, "Writing DFF Testbench for %s...\n", circuit_name); */
  testbench_load_cnt = 0;
 
  /* Print the title */
  fprint_spice_head(fp, title);
  my_free(title);

  /* print technology library and design parameters*/
  fprint_tech_lib(fp, arch.spice->tech_lib);

  /* Include parameter header files */
  fprint_spice_include_param_headers(fp, include_dir_path);

  /* Include Key subckts */
  fprint_spice_include_key_subckts(fp, subckt_dir_path);

  /* Include user-defined sub-circuit netlist */
  init_include_user_defined_netlists(*(arch.spice));
  fprint_include_user_defined_netlists(fp, *(arch.spice));
  
  /* Special subckts for Top-level SPICE netlist */
  fprintf(fp, "****** Include subckt netlists: Look-Up Tables (LUTs) *****\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, luts_spice_file_name);
  fprintf(fp, ".include \'%s\'\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "****** Include subckt netlists: Logic Blocks *****\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, logic_block_spice_file_name);
  fprintf(fp, ".include \'%s\'\n", temp_include_file_path);
  my_free(temp_include_file_path);

  /* Print simulation temperature and other options for SPICE */
  fprint_spice_options(fp, arch.spice->spice_params);

  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprint_spice_dff_testbench_global_ports(fp, grid_x, grid_y, num_clock, (*arch.spice));
 
  /* Quote defined Logic blocks subckts (Grids) */
  init_spice_dff_testbench_globals(*(arch.spice));
  init_logical_block_spice_model_type_temp_used(arch.spice->num_spice_model, arch.spice->spice_models, SPICE_MODEL_FF);
  fprint_spice_dff_testbench_call_one_grid_defined_dffs(fp, grid_x, grid_y, LL_rr_node_indices);

  /* Back-anotate activity information to each routing resource node 
   * (We should have activity of each Grid port) 
   */

  /* Add stimulations */
  fprint_spice_hardlogic_testbench_stimulations(fp, grid_x, grid_y, num_clock, (*arch.spice), LL_rr_node_indices);

  /* Add measurements */  
  fprint_spice_hardlogic_testbench_measurements(fp, grid_x, grid_y, (*arch.spice), leakage_only);

  /* SPICE ends*/
  fprintf(fp, ".end\n");

  /* Close the file*/
  fclose(fp);

  if (0 < tb_num_dffs) {
    vpr_printf(TIO_MESSAGE_INFO, "Writing Grid[%d][%d] SPICE DFF Testbench for %s...\n",
               grid_x, grid_y, circuit_name);
    /* Push the testbench to the linked list */
    tb_head = add_one_spice_tb_info_to_llist(tb_head, dff_testbench_file_path, 
                                             max_sim_num_clock_cycles);
    used = 1;
  } else {
    /* Remove the file generated */
    my_remove_file(dff_testbench_file_path);
    used = 0;
  }

  return used;
}

/* Top-level function in this source file */
void fprint_spice_dff_testbench(char* formatted_spice_dir,
                                char* circuit_name,
                                char* include_dir_path,
                                char* subckt_dir_path,
                                t_ivec*** LL_rr_node_indices,
                                int num_clock,
                                t_arch arch,
                                boolean leakage_only) {
  char* dff_testbench_name = NULL;
  int ix, iy;
  int cnt = 0;
  int used = 0;

  for (ix = 1; ix < (nx+1); ix++) {
    for (iy = 1; iy < (ny+1); iy++) {
      dff_testbench_name = (char*)my_malloc(sizeof(char)*( strlen(circuit_name) 
                                            + 6 + strlen(my_itoa(ix)) + 1
                                            + strlen(my_itoa(iy)) + 1
                                            + strlen(spice_dff_testbench_postfix)  + 1 ));
      sprintf(dff_testbench_name, "%s_grid%d_%d%s",
              circuit_name, ix, iy, spice_dff_testbench_postfix);
      used = fprint_spice_one_dff_testbench(formatted_spice_dir, circuit_name, dff_testbench_name, 
                                            include_dir_path, subckt_dir_path, LL_rr_node_indices,
                                            num_clock, arch, ix, iy, 
                                            leakage_only);
      if (1 == used) {
        cnt += used;
      }
      /* free */
      my_free(dff_testbench_name);
    }  
  } 
  /* Update the global counter */
  num_used_dff_tb = cnt;
  vpr_printf(TIO_MESSAGE_INFO,"No. of generated FF testbench = %d\n", num_used_dff_tb);

  return;
}
