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
#include "spice_globals.h"
#include "spice_utils.h"
#include "spice_mux.h"
#include "spice_pbtypes.h"
#include "spice_subckt.h"
#include "spice_netlist_utils.h"

/* local global variables */
static int tb_num_luts = 0;

/* Subroutines in this source file*/
static 
void fprint_spice_lut_testbench_global_ports(FILE* fp, 
                                             int num_clock, 
                                             t_spice spice) {
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprintf(fp, ".global gvdd gset greset\n");
  fprintf(fp, ".global gvdd_local_interc gvdd_hardlogic\n");
  fprintf(fp, ".global gvdd_sram_local_routing gvdd_sram_luts\n");
  fprintf(fp, ".global %s->in\n", sram_spice_model->prefix);
  fprintf(fp, ".global gvdd_load\n");
  fprintf(fp, "***** Global Clock Signals *****\n");
  fprintf(fp, ".global gclock\n");

  /*Global Vdds for LUTs*/
  fprint_global_vdds_spice_model(fp, SPICE_MODEL_LUT, spice);

  return;
}

void fprint_spice_lut_testbench_one_lut(FILE* fp, 
                                        char* subckt_name, 
                                        int num_inputs, int num_outputs,
                                        float* input_density, 
                                        float* input_probability) {
  int ipin;
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Call defined subckt */
  fprintf(fp, "Xlut[%d] ", tb_num_luts);
  for (ipin = 0; ipin < num_inputs; ipin++) {
    fprintf(fp, "lut[%d]->in[%d] ", tb_num_luts, ipin);
  }
  fprintf(fp, "lut[%d]->out gvdd 0 %s\n", tb_num_luts, subckt_name);
  /* Stimulates */ 
  for (ipin = 0; ipin < num_inputs; ipin++) {
    fprintf(fp, "Vlut[%d]->in[%d] lut[%d]->in[%d] 0 \n",
            tb_num_luts, ipin, tb_num_luts, ipin);
    fprint_voltage_pulse_params(fp, 0, input_density[ipin], input_probability[ipin]);
  }
  return; 
}

void fprint_spice_lut_testbench_one_pb_graph_node_lut(FILE* fp, 
                                                      t_pb_graph_node* cur_pb_graph_node, 
                                                      char* prefix,
                                                      int x, int y) {
  int logical_block_index = OPEN;
  t_spice_model* pb_spice_model = NULL;
  t_pb_type* cur_pb_type = NULL;
  float* input_density = NULL;
  float* input_probability = NULL;
  int iport, ipin, iedge, cur_pin;
  int num_inputs, num_outputs, num_clock_pins, vpack_net_index;

  assert(NULL != cur_pb_graph_node);
  assert(NULL != prefix);

  cur_pb_type = cur_pb_graph_node->pb_type;
  assert(NULL != cur_pb_type);
  pb_spice_model = cur_pb_type->spice_model; 
 
  /* Try to find the mapped logic block index */
  logical_block_index = find_grid_mapped_logical_block(x, y, 
                                                       pb_spice_model, prefix);

  /* Allocate input_density and probability */
  stats_pb_graph_node_port_pin_numbers(cur_pb_graph_node,&num_inputs,&num_outputs, &num_clock_pins);
  assert(0 == num_clock_pins);
  assert(1 == num_outputs);
  assert(0 < num_inputs);

  input_density = (float*)my_malloc(sizeof(float)*num_inputs); 
  input_probability = (float*)my_malloc(sizeof(float)*num_inputs); 

  /* if we find a mapped logic block */
  if (OPEN != logical_block_index) {
    /* Get activity information */
    assert(1 == cur_pb_graph_node->num_input_ports);
    cur_pin = 0;
    for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
        vpack_net_index = logical_block[logical_block_index].input_nets[iport][ipin];
        /* Check this LUT pin has been used */
        if (cur_pin < logical_block[logical_block_index].used_input_pins) {
          assert(OPEN != vpack_net_index);
          input_density[cur_pin] = vpack_net[vpack_net_index].spice_net_info->density;
          input_probability[cur_pin] = vpack_net[vpack_net_index].spice_net_info->probability;
        } else {
          assert(OPEN == vpack_net_index);
          input_density[cur_pin] = 0.;
          input_probability[cur_pin] = 0.;
        }
        cur_pin++;
      }
    }
    assert(cur_pin == num_inputs);
  } else {
    /* We cannot find a mapped logic block, use default activity info*/
    for (cur_pin = 0; cur_pin < num_inputs; cur_pin++) {
      input_density[cur_pin] = 0.;
      input_probability[cur_pin] = 0.;
    } 
  }
 
  /* Call the subckt and give stimulates, measurements */
  fprint_spice_lut_testbench_one_lut(fp, prefix, num_inputs, num_outputs,
                                     input_density, input_probability);
  /* Add loads: two inverters */
  /* TODO: be more smart to idenity the loads */
  for (iedge = 0; iedge < 2; iedge++) {
  fprintf(fp, "Xinv[%d]_lut[%d]->out lut[%d]->out lut[%d]->out_load[%d] gvdd_load 0 inv\n",
          iedge, tb_num_luts, tb_num_luts, tb_num_luts, iedge);
  }

  tb_num_luts++;
  
  return; 
}

void fprint_spice_lut_testbench_rec_pb_graph_node_luts(FILE* fp,
                                                       t_pb_graph_node* cur_pb_graph_node, 
                                                       char* prefix,
                                                       int x, int y) {
  char* formatted_prefix = format_spice_node_prefix(prefix); 
  int ipb, jpb, mode_index; 
  t_pb_type* cur_pb_type = NULL;
  char* rec_prefix = NULL;
  
  assert(NULL != cur_pb_graph_node);
  cur_pb_type = cur_pb_graph_node->pb_type;
  assert(NULL != cur_pb_type);
  /* Until we reach a LUT */
  if (NULL != cur_pb_type->spice_model) {
    if (SPICE_MODEL_LUT != cur_pb_type->spice_model->type) {
      return;
    }
    /* Generate rec_prefix */
    rec_prefix = (char*)my_malloc(sizeof(char) * (strlen(formatted_prefix) 
                   + strlen(cur_pb_type->name) + 1 
                   + strlen(my_itoa(cur_pb_graph_node->placement_index))
                   + 1 + 1));
    sprintf(rec_prefix, "%s%s[%d]",
            formatted_prefix, cur_pb_type->name, cur_pb_graph_node->placement_index);
    /* Print a lut tb: call spice_model, stimulates */
    fprint_spice_lut_testbench_one_pb_graph_node_lut(fp, cur_pb_graph_node, rec_prefix, x, y);
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
      fprint_spice_lut_testbench_rec_pb_graph_node_luts(fp, &(cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb]), rec_prefix, x, y);
      my_free(rec_prefix);
    }
  }
  
  return;
}

void fprint_spice_lut_testbench_rec_pb_luts(FILE* fp, 
                                            t_pb* cur_pb, char* prefix, 
                                            int x, int y) {
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
    if (SPICE_MODEL_LUT != cur_pb->pb_graph_node->pb_type->spice_model->type) {
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
    fprint_spice_lut_testbench_one_pb_graph_node_lut(fp, cur_pb->pb_graph_node, rec_prefix, x, y);
    my_free(rec_prefix);
    return;
  }
  
  /* Go recursively ... */
  mode_index = cur_pb->mode;
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
        fprint_spice_lut_testbench_rec_pb_luts(fp, &(cur_pb->child_pbs[ipb][jpb]), rec_prefix, x, y);
      } else {
        /* Print idle graph_node muxes */
        fprint_spice_lut_testbench_rec_pb_graph_node_luts(fp, cur_pb->child_pbs[ipb][jpb].pb_graph_node, rec_prefix, x, y);
      }
    }
  }
  
  return;
}

void fprint_spice_lut_testbench_call_defined_luts(FILE* fp) {
  int ix, iy, iblk;
  char* prefix = NULL;
 
  for (ix = 1; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      if (NULL == grid[ix][iy].type) {
        continue; 
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
        fprint_spice_lut_testbench_rec_pb_luts(fp, block[grid[ix][iy].blocks[iblk]].pb, prefix, ix, iy);
        my_free(prefix);
      }
      for (iblk = grid[ix][iy].usage; iblk < grid[ix][iy].type->capacity; iblk++) {
        prefix = (char*)my_malloc(sizeof(char)* (5 + strlen(my_itoa(ix)) 
                                  + 2 + strlen(my_itoa(iy)) + 3 ));
        sprintf(prefix, "grid[%d][%d]_", ix, iy);
        assert(NULL != grid[ix][iy].type->pb_graph_head);
        fprint_spice_lut_testbench_rec_pb_graph_node_luts(fp, grid[ix][iy].type->pb_graph_head, prefix, ix, iy); 
        my_free(prefix);
      }
    }
  }

  return;
}

void fprint_spice_lut_testbench_stimulations(FILE* fp, 
                                             int num_clock, 
                                             t_spice spice, 
                                             t_ivec*** LL_rr_node_indices) {
  /* Global GND */
  fprintf(fp, "***** Global VDD port *****\n");
  fprintf(fp, "Vgvdd gvdd 0 vsp\n");
  fprintf(fp, "***** Global GND port *****\n");
  fprintf(fp, "*Rggnd ggnd 0 0\n");

  /* Global set and reset */
  fprintf(fp, "***** Global Net for reset signal *****\n");
  fprintf(fp, "Vgvreset greset 0 0\n");
  fprintf(fp, "***** Global Net for set signal *****\n");
  fprintf(fp, "Vgvset gset 0 0\n");
  /* Global vdd load */
  fprintf(fp, "***** Global Net for load vdd *****\n");
  fprintf(fp, "Vgvdd_load gvdd_load 0 vsp\n");

  /* Global Vdd ports */
  fprintf(fp, "***** Global VDD for LUTs SRAMs *****\n");
  fprintf(fp, "Vgvdd_sram_luts gvdd_sram_luts 0 vsp\n");

  /* Every LUT use an independent Voltage source */
  fprintf(fp, "***** Global VDD for Look-Up Tables (LUTs) *****\n");
  fprint_splited_vdds_spice_model(fp, SPICE_MODEL_LUT, spice);

  /* Every SRAM inputs should have a voltage source */
  fprintf(fp, "***** Global Inputs for SRAMs *****\n");
  /*
  for (i = 0; i < sram_spice_model->cnt; i++) {
    fprintf(fp, "V%s[%d]->in %s[%d]->in 0 0\n", 
            sram_spice_model->prefix, i, sram_spice_model->prefix, i);
  }
  */
  fprintf(fp, "V%s->in %s->in 0 0\n", 
          sram_spice_model->prefix, sram_spice_model->prefix);
  fprintf(fp, ".nodeset V(%s->in) 0\n", sram_spice_model->prefix);

  fprintf(fp, "***** Global Clock signal *****\n");
  if (0 < num_clock) {
    /* First cycle reserved for measuring leakage */
    fprintf(fp, "***** pulse(vlow vhigh tdelay trise tfall pulse_width period *****\n");
    fprintf(fp, "Vgclock gclock 0 pulse(0 vsp 'clock_period'\n");
    fprintf(fp, "+                      'clock_slew_pct_rise*clock_period' 'clock_slew_pct_fall*clock_period'\n");
    fprintf(fp, "+                      '0.5*(1-clock_slew_pct_rise-clock_slew_pct_fall)*clock_period' 'clock_period')\n");
  } else {
    assert(0 == num_clock);
    fprintf(fp, "***** clock off *****\n");
    fprintf(fp, "Vgclock gclock 0 0\n");
  }
  return;
}

void fprint_spice_lut_testbench_measurements(FILE* fp, 
                                             t_spice spice, 
                                             boolean leakage_only) {
 
  /* First cycle reserved for measuring leakage */
  int num_clock_cycle = spice.spice_params.meas_params.sim_num_clock_cycle + 1;
  
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  
  fprint_spice_netlist_transient_setting(fp, spice, leakage_only);

  /* TODO: Measure the delay of each mapped net and logical block */

  /* Measure the power */
  /* Leakage ( the first cycle is reserved for leakage measurement) */
  if (TRUE == leakage_only) {
    /* Leakage power of SRAMs */
    fprintf(fp, ".measure tran leakage_power_sram_luts find p(Vgvdd_sram_luts) at=0\n");
  } else {
    /* Leakage power of SRAMs */
    fprintf(fp, ".measure tran leakage_power_sram_luts avg p(Vgvdd_sram_luts) from=0 to='clock_period'\n");
  }
  /* Leakage power of LUTs*/
  fprint_measure_vdds_spice_model(fp, SPICE_MODEL_LUT, SPICE_MEASURE_LEAKAGE_POWER, num_clock_cycle, spice, leakage_only);

  if (TRUE == leakage_only) {
    return;
  }

  /* Dynamic power */
  /* Dynamic power of SRAMs */
  fprintf(fp, ".measure tran dynamic_power_sram_luts avg p(Vgvdd_sram_luts) from='clock_period' to='%d*clock_period'\n", num_clock_cycle);
  /* Dynamic power of LUTs */
  fprint_measure_vdds_spice_model(fp, SPICE_MODEL_LUT, SPICE_MEASURE_DYNAMIC_POWER, num_clock_cycle, spice, leakage_only);

  return;
}

/* Top-level function in this source file */
void fprint_spice_lut_testbench(char* formatted_spice_dir,
                                char* circuit_name,
                                char* lut_testbench_name,
                                char* include_dir_path,
                                char* subckt_dir_path,
                                t_ivec*** LL_rr_node_indices,
                                int num_clock,
                                t_arch arch,
                                boolean leakage_only) {
  FILE* fp = NULL;
  char* formatted_subckt_dir_path = format_dir_path(subckt_dir_path);
  char* temp_include_file_path = NULL;
  char* title = my_strcat("FPGA LUT Testbench for Design: ", circuit_name);
  char* lut_testbench_file_path = my_strcat(formatted_spice_dir, lut_testbench_name);

  /* Check if the path exists*/
  fp = fopen(lut_testbench_file_path,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create LUT Testbench SPICE netlist %s!",__FILE__, __LINE__, lut_testbench_file_path); 
    exit(1);
  } 
  
  vpr_printf(TIO_MESSAGE_INFO, "Writing LUT Testbench for %s...\n", circuit_name);
 
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
  fprint_spice_lut_testbench_global_ports(fp, num_clock, (*arch.spice));
 
  /* Quote defined Logic blocks subckts (Grids) */
  fprint_spice_lut_testbench_call_defined_luts(fp);

  /* Back-anotate activity information to each routing resource node 
   * (We should have activity of each Grid port) 
   */

  /* Add stimulations */
  fprint_spice_lut_testbench_stimulations(fp, num_clock, (*arch.spice), LL_rr_node_indices);

  /* Add measurements */  
  fprint_spice_lut_testbench_measurements(fp, (*arch.spice), leakage_only);

  /* SPICE ends*/
  fprintf(fp, ".end\n");

  /* Close the file*/
  fclose(fp);

  return;
}

