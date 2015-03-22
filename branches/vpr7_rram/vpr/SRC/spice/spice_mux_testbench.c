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
#include "spice_routing.h"
#include "spice_subckt.h"
#include "spice_netlist_utils.h"

/** In this test bench. 
 * All the multiplexers (Local routing, Switch Boxes, Connection Blocks) in the FPGA are examined 
 * All the multiplexers are hanged with equivalent capactive loads in their context.
 */

/* Global variables in this C-source file */
static int testbench_mux_cnt = 0;
static int testbench_sram_cnt = 0;
static int testbench_load_cnt = 0;
static int testbench_pb_mux_cnt = 0;
static int testbench_cb_mux_cnt = 0;
static int testbench_sb_mux_cnt = 0;
static t_llist* testbench_muxes_head = NULL; 
static int num_segments = 0;
static t_segment_inf* segments = NULL;

/***** Local Subroutines Declaration *****/
static 
void fprint_spice_mux_testbench_global_ports(FILE* fp,
                                             t_spice spice);

static 
int find_spice_mux_testbench_pb_pin_mux_load_inv_size(t_spice_model* fan_out_spice_model);

static 
void fprint_spice_mux_testbench_one_mux(FILE* fp,
                                        char* meas_tag,
                                        t_spice_model* mux_spice_model,
                                        int mux_size,
                                        float* input_density,
                                        float* input_probability,
                                        int path_id);

static 
void fprint_spice_mux_testbench_pb_pin_mux(FILE* fp,
                                           t_rr_node* pb_rr_nodes,
                                           t_pb* des_pb,
                                           t_mode* cur_mode,
                                           t_pb_graph_pin* des_pb_graph_pin,
                                           t_interconnect* cur_interc,
                                           int fan_in,
                                           int select_edge);

static 
void fprint_spice_mux_testbench_pb_pin_interc(FILE* fp,
                                              t_rr_node* pb_rr_nodes,
                                              t_pb* des_pb,
                                              enum e_pin2pin_interc_type pin2pin_interc_type,
                                              t_pb_graph_pin* des_pb_graph_pin,
                                              t_mode* cur_mode,
                                              int select_path_id);

static 
void fprint_spice_mux_testbench_pb_interc(FILE* fp,
                                          t_pb* cur_pb);

static 
void fprint_spice_mux_testbench_pb_muxes_rec(FILE* fp,
                                             t_pb* cur_pb);

static 
float find_spice_mux_testbench_rr_mux_load_inv_size(t_rr_node* load_rr_node,
                                                    int switch_index);

static 
void fprint_spice_mux_testbench_cb_one_mux(FILE* fp,
                                           t_rr_type chan_type,
                                           int cb_x,
                                           int cb_y,
                                           t_rr_node* src_rr_node);

static 
void fprint_spice_mux_testbench_cb_interc(FILE* fp, 
                                          t_rr_type chan_type,
                                          int cb_x, int cb_y,
                                          t_rr_node* src_rr_node);

static 
void fprint_spice_mux_testbench_cb_muxes(FILE* fp, 
                                         t_rr_type chan_type,
                                         int x, int y,
                                         t_ivec*** LL_rr_node_indices);

static 
void fprint_spice_mux_testbench_sb_one_mux(FILE* fp,
                                           int switch_box_x, int switch_box_y,
                                           int chan_side,
                                           t_rr_node* src_rr_node);

static 
void fprint_spice_mux_testbench_sb_muxes(FILE* fp, 
                                         int x, int y, 
                                         t_ivec*** LL_rr_node_indices);

static 
void fprint_spice_mux_testbench_call_defined_muxes(FILE* fp,
                                                   t_ivec*** LL_rr_node_indices);

static 
void fprint_spice_mux_testbench_stimulations(FILE* fp, 
                                             t_spice spice);

static 
void fprint_spice_mux_testbench_measurements(FILE* fp, 
                                             t_spice spice);

/***** Local Subroutines *****/
static void init_spice_mux_testbench_globals() {
  testbench_mux_cnt = 0;
  testbench_sram_cnt = 0;
  testbench_load_cnt = 0;
  testbench_muxes_head = NULL; 
  testbench_pb_mux_cnt = 0;
  testbench_cb_mux_cnt = 0;
  testbench_sb_mux_cnt = 0;
}

static 
void fprint_spice_mux_testbench_global_ports(FILE* fp,
                                             t_spice spice) {
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprintf(fp, ".global gvdd gset greset\n");
  fprintf(fp, ".global gvdd_sram\n");
  fprintf(fp, ".global gvdd_load\n");
  fprintf(fp, ".global %s->in\n", sram_spice_model->prefix);

  return;
}

static 
int find_spice_mux_testbench_pb_pin_mux_load_inv_size(t_spice_model* fan_out_spice_model) {
  float load_inv_size = 0;

  /* Check */
  assert(NULL != fan_out_spice_model);
  assert(NULL != fan_out_spice_model->input_buffer);

  /* depend on the input_buffer type */
  if (1 == fan_out_spice_model->input_buffer->exist) {
    switch(fan_out_spice_model->input_buffer->type) {
    case SPICE_MODEL_BUF_INV:
      load_inv_size = fan_out_spice_model->input_buffer->size;
      break;
    case SPICE_MODEL_BUF_BUF:
      load_inv_size = 1;
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid fanout spice_model input_buffer type!\n",
                 __FILE__, __LINE__);
      exit(1);
    }
  } else {
    /* TODO: If there is no inv/buffer at input, we should traversal until there is one 
     * However, now we just simply give a minimum sized inverter
     */
    load_inv_size = 1;
  }
 
  return load_inv_size;
}

static 
void fprint_spice_mux_testbench_pb_mux_meas(FILE* fp,
                                            char* meas_tag) {
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  if (0 == testbench_pb_mux_cnt) {
    fprintf(fp, ".meas tran sum_leakage_power_pb_mux[0to%d] \n", testbench_pb_mux_cnt);
    fprintf(fp, "+          param=\'leakage_%s\'\n", meas_tag);
    fprintf(fp, ".meas tran sum_dynamic_power_pb_mux[0to%d] \n", testbench_pb_mux_cnt);
    fprintf(fp, "+          param=\'dynamic_%s\'\n", meas_tag);
  } else {
    fprintf(fp, ".meas tran sum_leakage_power_pb_mux[0to%d] \n", testbench_pb_mux_cnt);
    fprintf(fp, "+          param=\'sum_leakage_power_pb_mux[0to%d]+leakage_%s\'\n", testbench_pb_mux_cnt-1, meas_tag);
    fprintf(fp, ".meas tran sum_dynamic_power_pb_mux[0to%d] \n", testbench_pb_mux_cnt);
    fprintf(fp, "+          param=\'sum_dynamic_power_pb_mux[0to%d]+dynamic_%s\'\n", testbench_pb_mux_cnt-1, meas_tag);
  }

  /* Update the counter */
  testbench_pb_mux_cnt++;

  return;
}

static 
void fprint_spice_mux_testbench_cb_mux_meas(FILE* fp,
                                            char* meas_tag) {
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  if (0 == testbench_cb_mux_cnt) {
    fprintf(fp, ".meas tran sum_leakage_power_cb_mux[0to%d] \n", testbench_cb_mux_cnt);
    fprintf(fp, "+          param=\'leakage_%s\'\n", meas_tag);
    fprintf(fp, ".meas tran sum_dynamic_power_cb_mux[0to%d] \n", testbench_cb_mux_cnt);
    fprintf(fp, "+          param=\'dynamic_%s\'\n", meas_tag);
  } else {
    fprintf(fp, ".meas tran sum_leakage_power_cb_mux[0to%d] \n", testbench_cb_mux_cnt);
    fprintf(fp, "+          param=\'sum_leakage_power_cb_mux[0to%d]+leakage_%s\'\n", testbench_cb_mux_cnt-1, meas_tag);
    fprintf(fp, ".meas tran sum_dynamic_power_cb_mux[0to%d] \n", testbench_cb_mux_cnt);
    fprintf(fp, "+          param=\'sum_dynamic_power_cb_mux[0to%d]+dynamic_%s\'\n", testbench_cb_mux_cnt-1, meas_tag);
  }

  /* Update the counter */
  testbench_cb_mux_cnt++;

  return;
}

static 
void fprint_spice_mux_testbench_sb_mux_meas(FILE* fp,
                                            char* meas_tag) {
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  if (0 == testbench_sb_mux_cnt) {
    fprintf(fp, ".meas tran sum_leakage_power_sb_mux[0to%d] \n", testbench_sb_mux_cnt);
    fprintf(fp, "+          param=\'leakage_%s\'\n", meas_tag);
    fprintf(fp, ".meas tran sum_dynamic_power_sb_mux[0to%d] \n", testbench_sb_mux_cnt);
    fprintf(fp, "+          param=\'dynamic_%s\'\n", meas_tag);
  } else {
    fprintf(fp, ".meas tran sum_leakage_power_sb_mux[0to%d] \n", testbench_sb_mux_cnt);
    fprintf(fp, "+          param=\'sum_leakage_power_sb_mux[0to%d]+leakage_%s\'\n", testbench_sb_mux_cnt-1, meas_tag);
    fprintf(fp, ".meas tran sum_dynamic_power_sb_mux[0to%d] \n", testbench_sb_mux_cnt);
    fprintf(fp, "+          param=\'sum_dynamic_power_sb_mux[0to%d]+dynamic_%s\'\n", testbench_sb_mux_cnt-1, meas_tag);
  }

  /* Update the counter */
  testbench_sb_mux_cnt++;

  return;
}

static 
void fprint_spice_mux_testbench_one_mux(FILE* fp,
                                        char* meas_tag,
                                        t_spice_model* mux_spice_model,
                                        int mux_size,
                                        float* input_density,
                                        float* input_probability,
                                        int path_id) {
  int inode, mux_level, ilevel, cur_num_sram;
  int* mux_sram_bits = NULL; 
  t_llist* found_mux_node = NULL;
  t_spice_mux_model* cur_mux = NULL;

  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  /* Check */
  assert(NULL != mux_spice_model);
  assert((2 < mux_size)||(2 == mux_size));
  assert(NULL != input_density);
  assert(NULL != input_probability);

  /* Add to linked list */
  check_and_add_mux_to_linked_list(&(testbench_muxes_head), mux_size, mux_spice_model);
  found_mux_node = search_mux_linked_list(testbench_muxes_head, mux_size, mux_spice_model);
  /* Check */
  assert(NULL != found_mux_node);
  cur_mux = (t_spice_mux_model*)(found_mux_node->dptr);
  assert(mux_spice_model == cur_mux->spice_model);

  /* Call the subckt that has already been defined before */
  fprintf(fp, "X%s_size%d[%d] ", mux_spice_model->prefix, mux_size, testbench_mux_cnt);
  /* input port*/
  for (inode = 0; inode < mux_size; inode++) {
    fprintf(fp, "%s_size%d[%d]->in[%d] ", 
            mux_spice_model->prefix, mux_size, testbench_mux_cnt, inode);
  }
  /* Output port */
  fprintf(fp, "%s_size%d[%d]->out ", 
          mux_spice_model->prefix, mux_size, testbench_mux_cnt);

  /* SRAMs */
  /* 1. Get the mux level*/
  mux_level = determine_mux_level(mux_size);
  /* Print SRAM configurations, 
   * we should have a global SRAM vdd, AND it should be connected to a real sram subckt !!!
   */
  /* Configuration bits for MUX*/
  assert((-1 != path_id)&&(path_id < mux_size));
  mux_sram_bits = decode_mux_sram_bits(mux_size, mux_level, path_id); 

  /* Print SRAMs that configure this MUX */
  /* TODO: What about RRAM-based MUX? */
  cur_num_sram = testbench_sram_cnt;
  for (ilevel = 0; ilevel < mux_level; ilevel++) {
    /* Pull Up/Down the SRAM outputs*/
    switch (mux_sram_bits[mux_level - ilevel - 1]) {
    case 0:
      /* Pull down power is considered as a part of subckt (CB or SB)*/
      fprintf(fp, "%s[%d]->out ", sram_spice_model->prefix, cur_num_sram);
      fprintf(fp, "%s[%d]->outb ", sram_spice_model->prefix, cur_num_sram);
      break;
    case 1:
      /* Pull down power is considered as a part of subckt (CB or SB)*/
      fprintf(fp, "%s[%d]->outb ", sram_spice_model->prefix, cur_num_sram);
      fprintf(fp, "%s[%d]->out ", sram_spice_model->prefix, cur_num_sram);
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File: %s,[LINE%d])Invalid sram_bit(=%d)! Should be [0|1].\n",
                 __FILE__, __LINE__, mux_sram_bits[ilevel]);
      exit(1);
    }
    cur_num_sram++;
  }
  /* End with svdd and sgnd, subckt name*/
  /* Local vdd and gnd, we should have an independent VDD for all local interconnections*/
  fprintf(fp, "gvdd_%s_size%d[%d] 0 ", mux_spice_model->prefix, mux_size, testbench_mux_cnt);
  /* End with spice_model name */
  fprintf(fp, "%s_size%d\n", mux_spice_model->name, mux_size);


  /* Print the encoding in SPICE netlist for debugging */
  fprintf(fp, "***** SRAM bits for MUX[%d], level=%d, select_path_id=%d. *****\n", 
          mux_spice_model->cnt, mux_level, path_id);
  fprintf(fp, "*****");
  for (ilevel = 0; ilevel < mux_level; ilevel++) {
    fprintf(fp, "%d", mux_sram_bits[ilevel]);
  }
  fprintf(fp, "*****\n");
  
  /* Force SRAM bits */
  /* cur_num_sram = testbench_sram_cnt; */
  /* 
  for (ilevel = 0; ilevel < mux_level; ilevel++) {
    fprintf(fp,"V%s[%d]->in %s[%d]->in 0 ", 
          sram_spice_model->prefix, cur_num_sram, sram_spice_model->prefix, cur_num_sram);
    fprintf(fp, "0\n");
    cur_num_sram++;
  }
  */

  /* Call SRAM subckts*/
  for (ilevel = 0; ilevel < mux_level; ilevel++) {
    fprintf(fp, "X%s[%d] ", sram_spice_model->prefix, testbench_sram_cnt);
    /* fprintf(fp, "%s[%d]->in ", sram_spice_model->prefix, testbench_sram_cnt); */
    fprintf(fp, "%s->in ", sram_spice_model->prefix);
    fprintf(fp,"%s[%d]->out %s[%d]->outb ", 
          sram_spice_model->prefix, testbench_sram_cnt, sram_spice_model->prefix, testbench_sram_cnt);
    /* Configure the SRAMs*/
    fprintf(fp, "gvdd_sram 0 %s\n", sram_spice_model->name);
    /* Add nodeset to help convergence */ 
    fprintf(fp, ".nodeset V(%s[%d]->out) 0\n", sram_spice_model->prefix, testbench_sram_cnt);
    fprintf(fp, ".nodeset V(%s[%d]->outb) vsp\n", sram_spice_model->prefix, testbench_sram_cnt);
    testbench_sram_cnt++;
  }

  /* Check SRAM counters */
  assert(cur_num_sram == testbench_sram_cnt);

  /* Test bench : Add voltage sources */
  for (inode = 0; inode < mux_size; inode++) {
    /* Print voltage source */
    fprintf(fp, "***** Signal %s_size%d[%d]->in[%d] density = %g, probability=%g.*****\n",
            mux_spice_model->prefix, mux_size, testbench_mux_cnt, inode, input_density[inode], input_probability[inode]);
    fprintf(fp, "V%s_size%d[%d]->in[%d] %s_size%d[%d]->in[%d] 0 \n", 
            mux_spice_model->prefix, mux_size, testbench_mux_cnt, inode,
            mux_spice_model->prefix, mux_size, testbench_mux_cnt, inode);
    fprint_voltage_pulse_params(fp, 0, input_density[inode], input_probability[inode]);
  }
  /* global voltage supply */
  fprintf(fp, "Vgvdd_%s_size%d[%d] gvdd_%s_size%d[%d] 0 vsp\n",
          mux_spice_model->prefix, mux_size, testbench_mux_cnt,
          mux_spice_model->prefix, mux_size, testbench_mux_cnt);

  /* Measurements */
  /* Measure the delay of MUX */
  fprintf(fp, "***** Measurements *****\n");
  /* Rise delay */
  fprintf(fp, "***** Rise delay *****\n");
  fprintf(fp, ".meas tran delay_rise_%s trig v(%s_size%d[%d]->in[%d]) val='input_thres_pct_rise*vsp' rise=1 td='clock_period'\n", meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt, path_id);
  fprintf(fp, "+          targ v(%s_size%d[%d]->out) val='output_thres_pct_rise*vsp' rise=1 td='clock_period'\n",
          mux_spice_model->prefix, mux_size, testbench_mux_cnt);
  /* Fall delay */
  fprintf(fp, "***** Fall delay *****\n");
  fprintf(fp, ".meas tran delay_fall_%s trig v(%s_size%d[%d]->in[%d]) val='input_thres_pct_fall*vsp' fall=1 td='clock_period'\n", meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt, path_id);
  fprintf(fp, "+          targ v(%s_size%d[%d]->out) val='output_thres_pct_fall*vsp' fall=1 td='clock_period'\n",
          mux_spice_model->prefix, mux_size, testbench_mux_cnt);
  /* Measure timing period of MUX switching */
  /* Rise */
  fprintf(fp, "***** Rise timing period *****\n");
  fprintf(fp, ".meas start_rise_%s when v(%s_size%d[%d]->in[%d])='slew_lower_thres_pct_rise*vsp' rise=1 td='clock_period'\n",
              meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt, path_id);
  fprintf(fp, ".meas tran switch_rise_%s trig v(%s_size%d[%d]->in[%d]) val='slew_lower_thres_pct_rise*vsp' rise=1 td='clock_period'\n", meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt, path_id);
  fprintf(fp, "+          targ v(%s_size%d[%d]->out) val='slew_upper_thres_pct_rise*vsp' rise=1 td='clock_period'\n",
          mux_spice_model->prefix, mux_size, testbench_mux_cnt);
  /* Fall */
  fprintf(fp, "***** Fall timing period *****\n");
  fprintf(fp, ".meas start_fall_%s when v(%s_size%d[%d]->in[%d])='slew_lower_thres_pct_fall*vsp' fall=1 td='clock_period'\n",
              meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt, path_id);
  fprintf(fp, ".meas tran switch_fall_%s trig v(%s_size%d[%d]->in[%d]) val='slew_lower_thres_pct_fall*vsp' fall=1 td='clock_period'\n", meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt, path_id);
  fprintf(fp, "+          targ v(%s_size%d[%d]->out) val='slew_upper_thres_pct_fall*vsp' fall=1 td='clock_period'\n",
          mux_spice_model->prefix, mux_size, testbench_mux_cnt);
  /* Measure the leakage power of MUX */
  fprintf(fp, ".meas tran leakage_%s avg p(Vgvdd_%s_size%d[%d]) from=0 to='clock_period'\n",
          meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt);
  fprintf(fp, ".meas tran %s_size%d[%d]_leakage_power param='leakage_%s'\n",
          mux_spice_model->prefix, mux_size, testbench_mux_cnt, meas_tag);
  /* Measure the dynamic power of MUX */
  fprintf(fp, ".meas tran dynamic_%s avg p(Vgvdd_%s_size%d[%d]) from='clock_period' to='2*clock_period'\n",
          meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt);
  fprintf(fp, ".meas tran %s_size%d[%d]_dynamic_power param='dynamic_%s'\n",
          mux_spice_model->prefix, mux_size, testbench_mux_cnt, meas_tag);
  fprintf(fp, ".meas tran dynamic_rise_%s avg p(Vgvdd_%s_size%d[%d]) from='start_rise_%s' to='start_rise_%s+switch_rise_%s'\n",
          meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt, meas_tag, meas_tag, meas_tag);
  fprintf(fp, ".meas tran dynamic_fall_%s avg p(Vgvdd_%s_size%d[%d]) from='start_fall_%s' to='start_fall_%s+switch_fall_%s'\n",
          meas_tag, mux_spice_model->prefix, mux_size, testbench_mux_cnt, meas_tag, meas_tag, meas_tag);
  if (0 == testbench_mux_cnt) {
    fprintf(fp, ".meas tran sum_leakage_power_mux[0to%d] \n", testbench_mux_cnt);
    fprintf(fp, "+          param=\'leakage_%s\'\n", meas_tag);
    fprintf(fp, ".meas tran sum_dynamic_power_mux[0to%d] \n", testbench_mux_cnt);
    fprintf(fp, "+          param=\'dynamic_%s\'\n", meas_tag);
  } else {
    fprintf(fp, ".meas tran sum_leakage_power_mux[0to%d] \n", testbench_mux_cnt);
    fprintf(fp, "+          param=\'sum_leakage_power_mux[0to%d]+leakage_%s\'\n", testbench_mux_cnt-1, meas_tag);
    fprintf(fp, ".meas tran sum_dynamic_power_mux[0to%d] \n", testbench_mux_cnt);
    fprintf(fp, "+          param=\'sum_dynamic_power_mux[0to%d]+dynamic_%s\'\n", testbench_mux_cnt-1, meas_tag);
  }

  /* Update the counter */
  cur_mux->cnt++;

  /* Free */
  my_free(mux_sram_bits);

  return;
}

/** Print a mulitplexer testbench of a given pb pin
 * --|---|
 * --| M |
 * ..| U |--- des_pb_graph_in
 * --| X |
 * --|---|
 */
static 
void fprint_spice_mux_testbench_pb_graph_node_pin_mux(FILE* fp,
                                                      t_mode* cur_mode,
                                                      t_pb_graph_pin* des_pb_graph_pin,
                                                      t_interconnect* cur_interc,
                                                      int fan_in,
                                                      int select_edge) {
  int cur_input = 0;  
  float* input_density = NULL;
  float* input_probability = NULL;
  int iedge;
  int* sram_bits = NULL; 
  t_spice_model* fan_out_spice_model = NULL;
  float load_inv_size = 0.;
  char* meas_tag = NULL;

  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Check : 
   * MUX should have at least 2 fan_in
   */
  assert((2 == fan_in)||(2 < fan_in));
  /* 2. spice_model is a wire */ 
  assert(NULL != cur_interc->spice_model);
  assert(SPICE_MODEL_MUX == cur_interc->spice_model->type);
 
  /* Test bench : Add voltage sources */
  cur_input = 0;
  input_density = (float*)my_malloc(sizeof(float)*fan_in);
  input_probability = (float*)my_malloc(sizeof(float)*fan_in);
  for (iedge = 0; iedge < des_pb_graph_pin->num_input_edges; iedge++) {
    if (cur_mode != des_pb_graph_pin->input_edges[iedge]->interconnect->parent_mode) {
       continue;
    }
    check_pb_graph_edge(*(des_pb_graph_pin->input_edges[iedge]));
    /* Find activity information */
    input_density[cur_input] = 0; 
    input_probability[cur_input] = 0; 
    cur_input++;
  }
  /* Check fan-in number is correct */
  assert(fan_in == cur_input);

  /* Build a measurement tag: <des_pb:spice_name_tag>_<des_port>[pin_index]_<interc_name> */
  meas_tag = (char*)my_malloc(sizeof(char)* (10 + strlen(my_itoa(fan_in)) + strlen(my_itoa(testbench_mux_cnt)) + 2
                                            + strlen(des_pb_graph_pin->port->name) + 1 
                                            + strlen(my_itoa(des_pb_graph_pin->pin_number)) + 2
                                            + strlen(cur_interc->name) + 1)); /* Add '0'*/
  sprintf(meas_tag, "idle_mux%d[%d]_%s[%d]_%s", 
          fan_in, testbench_mux_cnt, des_pb_graph_pin->port->name, des_pb_graph_pin->pin_number, cur_interc->name);
  /* Print the main part of a single MUX testbench */
  fprint_spice_mux_testbench_one_mux(fp, meas_tag, cur_interc->spice_model,
                                     fan_in, input_density, input_probability, select_edge);

  /* Test bench : Capactive load */
  /* Search all the fan-outs of des_pb_graph_pin */
  for (iedge = 0; iedge < des_pb_graph_pin->num_output_edges; iedge++) {
    /* Bypass fan-out not in this mode */
    if (cur_mode != des_pb_graph_pin->output_edges[iedge]->interconnect->parent_mode) {
       continue;
    }
    check_pb_graph_edge(*(des_pb_graph_pin->output_edges[iedge]));
    /* For each fan-out, give inv/buf loads depending on its fanout interc. */
    if (0 == des_pb_graph_pin->num_output_edges) { /* If there is zero fan-out, this could be a primitive block */
      fan_out_spice_model = des_pb_graph_pin->output_edges[iedge]->output_pins[0]->parent_node->pb_type->spice_model; 
      assert(NULL != fan_out_spice_model);
      /* Detect its input buffers */
      load_inv_size = find_spice_mux_testbench_pb_pin_mux_load_inv_size(fan_out_spice_model);
      /* Print an inverter */
      fprintf(fp, "Xload_inv[%d] %s_size%d[%d]->out load_inv[%d]_out gvdd_load 0 inv size=%g\n",
              testbench_load_cnt, cur_interc->spice_model->prefix, fan_in, testbench_mux_cnt, testbench_load_cnt, load_inv_size);
      testbench_load_cnt++;
    }
  }

  fprint_spice_mux_testbench_pb_mux_meas(fp, meas_tag);
  /* Update the counter */
  testbench_mux_cnt++;

  /* Free */
  my_free(sram_bits);
  my_free(input_density);
  my_free(input_probability);

  return;
}


/** Print a mulitplexer testbench of a given pb pin
 * --|---|
 * --| M |
 * ..| U |--- des_pb_graph_in
 * --| X |
 * --|---|
 */
static 
void fprint_spice_mux_testbench_pb_pin_mux(FILE* fp,
                                           t_rr_node* pb_rr_graph,
                                           t_pb* des_pb,
                                           t_mode* cur_mode,
                                           t_pb_graph_pin* des_pb_graph_pin,
                                           t_interconnect* cur_interc,
                                           int fan_in,
                                           int select_edge) {
  int cur_input = 0;  
  float* input_density = NULL;
  float* input_probability = NULL;
  int iedge;
  int* sram_bits = NULL; 
  t_spice_model* fan_out_spice_model = NULL;
  float load_inv_size = 0.;
  char* meas_tag = NULL;

  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Check : 
   * MUX should have at least 2 fan_in
   */
  assert((2 == fan_in)||(2 < fan_in));
  /* 2. spice_model is a wire */ 
  assert(NULL != cur_interc->spice_model);
  assert(SPICE_MODEL_MUX == cur_interc->spice_model->type);
 
  /* Test bench : Add voltage sources */
  cur_input = 0;
  input_density = (float*)my_malloc(sizeof(float)*fan_in);
  input_probability = (float*)my_malloc(sizeof(float)*fan_in);
  for (iedge = 0; iedge < des_pb_graph_pin->num_input_edges; iedge++) {
    if (cur_mode != des_pb_graph_pin->input_edges[iedge]->interconnect->parent_mode) {
       continue;
    }
    check_pb_graph_edge(*(des_pb_graph_pin->input_edges[iedge]));
    /* Find activity information */
    input_density[cur_input] = pb_pin_density(pb_rr_graph, des_pb_graph_pin->input_edges[iedge]->input_pins[0]); 
    input_probability[cur_input] = pb_pin_probability(pb_rr_graph, des_pb_graph_pin->input_edges[iedge]->input_pins[0]); 
    cur_input++;
  }
  /* Check fan-in number is correct */
  assert(fan_in == cur_input);

  /* Build a measurement tag: <des_pb:spice_name_tag>_<des_port>[pin_index]_<interc_name> */
  meas_tag = (char*)my_malloc(sizeof(char)* (strlen(des_pb->spice_name_tag) + 1
                                            + strlen(des_pb_graph_pin->port->name) + 1 
                                            + strlen(my_itoa(des_pb_graph_pin->pin_number)) + 2
                                            + strlen(cur_interc->name) + 1)); /* Add '0'*/
  sprintf(meas_tag, "%s_%s[%d]_%s", 
          des_pb->spice_name_tag, des_pb_graph_pin->port->name, des_pb_graph_pin->pin_number, cur_interc->name);
  /* Print the main part of a single MUX testbench */
  fprint_spice_mux_testbench_one_mux(fp, meas_tag, cur_interc->spice_model,
                                     fan_in, input_density, input_probability, select_edge);

  /* Test bench : Capactive load */
  /* Search all the fan-outs of des_pb_graph_pin */
  for (iedge = 0; iedge < des_pb_graph_pin->num_output_edges; iedge++) {
    /* Bypass fan-out not in this mode */
    if (cur_mode != des_pb_graph_pin->output_edges[iedge]->interconnect->parent_mode) {
       continue;
    }
    check_pb_graph_edge(*(des_pb_graph_pin->output_edges[iedge]));
    /* For each fan-out, give inv/buf loads depending on its fanout interc. */
    if (0 == des_pb_graph_pin->num_output_edges) { /* If there is zero fan-out, this could be a primitive block */
      fan_out_spice_model = des_pb_graph_pin->output_edges[iedge]->output_pins[0]->parent_node->pb_type->spice_model; 
      assert(NULL != fan_out_spice_model);
      /* Detect its input buffers */
      load_inv_size = find_spice_mux_testbench_pb_pin_mux_load_inv_size(fan_out_spice_model);
      /* Print an inverter */
      fprintf(fp, "Xload_inv[%d] %s_size%d[%d]->out load_inv[%d]_out gvdd_load 0 inv size=%g\n",
              testbench_load_cnt, cur_interc->spice_model->prefix, fan_in, testbench_mux_cnt, testbench_load_cnt, load_inv_size);
      testbench_load_cnt++;
    }
  }

  fprint_spice_mux_testbench_pb_mux_meas(fp, meas_tag);

  /* Update the counter */
  testbench_mux_cnt++;

  /* Free */
  my_free(sram_bits);
  my_free(input_density);
  my_free(input_probability);

  return;
}

static 
void fprint_spice_mux_testbench_pb_graph_node_pin_interc(FILE* fp,
                                                         enum e_pin2pin_interc_type pin2pin_interc_type,
                                                         t_pb_graph_pin* des_pb_graph_pin,
                                                         t_mode* cur_mode,
                                                         int select_path_id) {
  int fan_in;
  int iedge;
  t_interconnect* cur_interc = NULL;
  enum e_interconnect spice_interc_type;

  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  /* 1. identify pin interconnection type, 
   * 2. Identify the number of fan-in (Consider interconnection edges of only selected mode)
   * 3. Select and print the SPICE netlist
   */
  fan_in = 0;
  cur_interc = NULL;
  /* Search the input edges only, stats on the size of MUX we may need (fan-in) */
  for (iedge = 0; iedge < des_pb_graph_pin->num_input_edges; iedge++) {
    /* 1. First, we should make sure this interconnect is in the selected mode!!!*/
    if (cur_mode == des_pb_graph_pin->input_edges[iedge]->interconnect->parent_mode) {
      /* Check this edge*/
      check_pb_graph_edge(*(des_pb_graph_pin->input_edges[iedge]));
      /* Record the interconnection*/
      if (NULL == cur_interc) {
        cur_interc = des_pb_graph_pin->input_edges[iedge]->interconnect;
      } else { /* Make sure the interconnections for this pin is the same!*/
        assert(cur_interc == des_pb_graph_pin->input_edges[iedge]->interconnect);
      }
      /* Search the input_pins of input_edges only*/
      fan_in += des_pb_graph_pin->input_edges[iedge]->num_input_pins;
    }
  }
  if (NULL == cur_interc) { 
    /* No interconnection matched */
    return;
  }
  /* Initialize the interconnection type that will be implemented in SPICE netlist*/
  switch (cur_interc->type) {
    case DIRECT_INTERC:
      assert(1 == fan_in);
      spice_interc_type = DIRECT_INTERC;
      break;
    case COMPLETE_INTERC:
      if (1 == fan_in) {
        spice_interc_type = DIRECT_INTERC;
      } else {
        assert((2 == fan_in)||(2 < fan_in));
        spice_interc_type = MUX_INTERC;
      }
      break;
    case MUX_INTERC:
      assert((2 == fan_in)||(2 < fan_in));
      spice_interc_type = MUX_INTERC;
      break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid interconnection type for %s (Arch[LINE%d])!\n",
               __FILE__, __LINE__, cur_interc->name, cur_interc->line_num);
    exit(1);
  }

  /* Print all the multiplexers at current level */
  switch (spice_interc_type) {
  case DIRECT_INTERC:
    break;
  case MUX_INTERC:
    assert(SPICE_MODEL_MUX == cur_interc->spice_model->type);
    fprint_spice_mux_testbench_pb_graph_node_pin_mux(fp, cur_mode, des_pb_graph_pin, 
                                                     cur_interc, fan_in, select_path_id); 
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid interconnection type!\n", 
               __FILE__, __LINE__);
    exit(1);
  }

  return;
}


static 
void fprint_spice_mux_testbench_pb_pin_interc(FILE* fp,
                                              t_rr_node* pb_rr_graph,
                                              t_pb* des_pb,
                                              enum e_pin2pin_interc_type pin2pin_interc_type,
                                              t_pb_graph_pin* des_pb_graph_pin,
                                              t_mode* cur_mode,
                                              int select_path_id) {
  int fan_in;
  int iedge;
  t_interconnect* cur_interc = NULL;
  enum e_interconnect spice_interc_type;

  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  /* 1. identify pin interconnection type, 
   * 2. Identify the number of fan-in (Consider interconnection edges of only selected mode)
   * 3. Select and print the SPICE netlist
   */
  fan_in = 0;
  cur_interc = NULL;
  /* Search the input edges only, stats on the size of MUX we may need (fan-in) */
  for (iedge = 0; iedge < des_pb_graph_pin->num_input_edges; iedge++) {
    /* 1. First, we should make sure this interconnect is in the selected mode!!!*/
    if (cur_mode == des_pb_graph_pin->input_edges[iedge]->interconnect->parent_mode) {
      /* Check this edge*/
      check_pb_graph_edge(*(des_pb_graph_pin->input_edges[iedge]));
      /* Record the interconnection*/
      if (NULL == cur_interc) {
        cur_interc = des_pb_graph_pin->input_edges[iedge]->interconnect;
      } else { /* Make sure the interconnections for this pin is the same!*/
        assert(cur_interc == des_pb_graph_pin->input_edges[iedge]->interconnect);
      }
      /* Search the input_pins of input_edges only*/
      fan_in += des_pb_graph_pin->input_edges[iedge]->num_input_pins;
    }
  }
  if (NULL == cur_interc) { 
    /* No interconnection matched */
    return;
  }
  /* Initialize the interconnection type that will be implemented in SPICE netlist*/
  switch (cur_interc->type) {
    case DIRECT_INTERC:
      assert(1 == fan_in);
      spice_interc_type = DIRECT_INTERC;
      break;
    case COMPLETE_INTERC:
      if (1 == fan_in) {
        spice_interc_type = DIRECT_INTERC;
      } else {
        assert((2 == fan_in)||(2 < fan_in));
        spice_interc_type = MUX_INTERC;
      }
      break;
    case MUX_INTERC:
      assert((2 == fan_in)||(2 < fan_in));
      spice_interc_type = MUX_INTERC;
      break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid interconnection type for %s (Arch[LINE%d])!\n",
               __FILE__, __LINE__, cur_interc->name, cur_interc->line_num);
    exit(1);
  }

  /* Print all the multiplexers at current level */
  switch (spice_interc_type) {
  case DIRECT_INTERC:
    break;
  case MUX_INTERC:
    assert(SPICE_MODEL_MUX == cur_interc->spice_model->type);
    fprint_spice_mux_testbench_pb_pin_mux(fp, pb_rr_graph, des_pb, cur_mode, des_pb_graph_pin, 
                                          cur_interc, fan_in, select_path_id); 
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid interconnection type!\n", 
               __FILE__, __LINE__);
    exit(1);
  }

  return;
}

/* For each pb, we search the input pins and output pins for local interconnections */
static 
void fprint_spice_mux_testbench_pb_graph_node_interc(FILE* fp,
                                                     t_pb_graph_node* cur_pb_graph_node) {
  int iport, ipin;
  int ipb, jpb;
  t_pb_type* cur_pb_type = NULL;
  t_mode* cur_mode = NULL;
  t_pb_graph_node* child_pb_graph_node = NULL;
  int select_mode_index = -1;

  int path_id = -1;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  
  assert(NULL != cur_pb_graph_node);
  cur_pb_type = cur_pb_graph_node->pb_type;
  assert(NULL != cur_pb_type);
  select_mode_index = find_pb_type_idle_mode_index(*(cur_pb_type));
  cur_mode = &(cur_pb_type->modes[select_mode_index]);
  assert(NULL != cur_mode);

  /* We check output_pins of cur_pb_graph_node and its the input_edges
   * Built the interconnections between outputs of cur_pb_graph_node and outputs of child_pb_graph_node
   *   child_pb_graph_node.output_pins -----------------> cur_pb_graph_node.outpins
   *                                        /|\
   *                                         |
   *                         input_pins,   edges,       output_pins
   */ 
  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      path_id = 0; 
      fprint_spice_mux_testbench_pb_graph_node_pin_interc(fp, 
                                                          OUTPUT2OUTPUT_INTERC,
                                                          &(cur_pb_graph_node->output_pins[iport][ipin]),
                                                          cur_mode,
                                                          path_id);
    }
  }
  
  /* We check input_pins of child_pb_graph_node and its the input_edges
   * Built the interconnections between inputs of cur_pb_graph_node and inputs of child_pb_graph_node
   *   cur_pb_graph_node.input_pins -----------------> child_pb_graph_node.input_pins
   *                                        /|\
   *                                         |
   *                         input_pins,   edges,       output_pins
   */ 
  for (ipb = 0; ipb < cur_pb_type->modes[select_mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb_type->modes[select_mode_index].pb_type_children[ipb].num_pb; jpb++) {
      child_pb_graph_node = &(cur_pb_graph_node->child_pb_graph_nodes[select_mode_index][ipb][jpb]);
      /* For each child_pb_graph_node input pins*/
      for (iport = 0; iport < child_pb_graph_node->num_input_ports; iport++) {
        for (ipin = 0; ipin < child_pb_graph_node->num_input_pins[iport]; ipin++) {
          path_id = 0;
          /* Write the interconnection*/
          fprint_spice_mux_testbench_pb_graph_node_pin_interc(fp, 
                                                              INPUT2INPUT_INTERC,
                                                              &(child_pb_graph_node->input_pins[iport][ipin]),
                                                              cur_mode,
                                                              path_id);
        }
      }
      /* TODO: for clock pins, we should do the same work */
      for (iport = 0; iport < child_pb_graph_node->num_clock_ports; iport++) {
        for (ipin = 0; ipin < child_pb_graph_node->num_clock_pins[iport]; ipin++) {
          path_id = 0;
          /* Write the interconnection*/
          fprint_spice_mux_testbench_pb_graph_node_pin_interc(fp, 
                                                              INPUT2INPUT_INTERC,
                                                              &(child_pb_graph_node->clock_pins[iport][ipin]),
                                                              cur_mode,
                                                              path_id);
        }
      }
    }
  }

  return; 
}

void fprint_spice_mux_testbench_idle_pb_graph_node_muxes_rec(FILE* fp, 
                                                             t_pb_graph_node* cur_pb_graph_node) {
  int ipb, jpb;
  int mode_index;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert(NULL != cur_pb_graph_node);

  /* If we touch the leaf, there is no need print interc*/
  if (NULL == cur_pb_graph_node->pb_type->spice_model) {
    /* Print MUX interc at current-level pb*/
    fprint_spice_mux_testbench_pb_graph_node_interc(fp, cur_pb_graph_node);
  } else {
    return;
  }
  
  /* Go recursively ... */
  mode_index = find_pb_type_idle_mode_index(*(cur_pb_graph_node->pb_type));
  for (ipb = 0; ipb < cur_pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Print idle muxes */
      fprint_spice_mux_testbench_idle_pb_graph_node_muxes_rec(fp, &(cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb]));
    }
  }
  
  return;
}

/* For each pb, we search the input pins and output pins for local interconnections */
static 
void fprint_spice_mux_testbench_pb_interc(FILE* fp,
                                          t_pb* cur_pb) {
  int iport, ipin;
  int ipb, jpb;
  t_pb_graph_node* cur_pb_graph_node = NULL;
  t_pb_type* cur_pb_type = NULL;
  t_mode* cur_mode = NULL;
  t_pb_graph_node* child_pb_graph_node = NULL;
  t_pb* child_pb = NULL;
  int select_mode_index = -1;

  int node_index = -1;
  int prev_node = -1;
  int prev_edge = -1;
  int path_id = -1;
  t_rr_node* pb_rr_nodes = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  
  assert(NULL != cur_pb);
  cur_pb_graph_node = cur_pb->pb_graph_node;
  assert(NULL != cur_pb_graph_node);
  cur_pb_type = cur_pb_graph_node->pb_type;
  assert(NULL != cur_pb_type);
  select_mode_index = cur_pb->mode;
  cur_mode = &(cur_pb_type->modes[select_mode_index]);
  assert(NULL != cur_mode);

  /* We check output_pins of cur_pb_graph_node and its the input_edges
   * Built the interconnections between outputs of cur_pb_graph_node and outputs of child_pb_graph_node
   *   child_pb_graph_node.output_pins -----------------> cur_pb_graph_node.outpins
   *                                        /|\
   *                                         |
   *                         input_pins,   edges,       output_pins
   */ 
  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      /* Get the selected edge of current pin*/
      assert(NULL != cur_pb);
      pb_rr_nodes = cur_pb->rr_graph;
      node_index = cur_pb_graph_node->output_pins[iport][ipin].pin_count_in_cluster;
      /* Bypass unmapped interc */
      if (OPEN == pb_rr_nodes[node_index].net_num) {
        continue;
      }
      prev_node = pb_rr_nodes[node_index].prev_node;
      /* prev_edge is the index of edge of prev_node !!! */
      prev_edge = pb_rr_nodes[node_index].prev_edge;
      /* Make sure this pb_rr_node is not OPEN and is not a primitive output*/
      if (OPEN == prev_node) {
        path_id = 0; 
        /* Determine the child_pb */
      } else {
        /* Find the path_id */
        path_id = find_path_id_between_pb_rr_nodes(pb_rr_nodes, prev_node, node_index);
        assert(-1 != path_id);
      } 
      fprint_spice_mux_testbench_pb_pin_interc(fp, pb_rr_nodes ,cur_pb, /* TODO: find out the child_pb*/
                                               OUTPUT2OUTPUT_INTERC,
                                               &(cur_pb_graph_node->output_pins[iport][ipin]),
                                               cur_mode,
                                               path_id);
    }
  }
  
  /* We check input_pins of child_pb_graph_node and its the input_edges
   * Built the interconnections between inputs of cur_pb_graph_node and inputs of child_pb_graph_node
   *   cur_pb_graph_node.input_pins -----------------> child_pb_graph_node.input_pins
   *                                        /|\
   *                                         |
   *                         input_pins,   edges,       output_pins
   */ 
  for (ipb = 0; ipb < cur_pb_type->modes[select_mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb_type->modes[select_mode_index].pb_type_children[ipb].num_pb; jpb++) {
      child_pb_graph_node = &(cur_pb_graph_node->child_pb_graph_nodes[select_mode_index][ipb][jpb]);
      child_pb = &(cur_pb->child_pbs[ipb][jpb]);
      /* Check if child_pb is empty */
      if (NULL == child_pb->name) { 
        /* fprint_spice_mux_testbench_idle_pb_graph_node_muxes_rec(fp, child_pb->pb_graph_node, jpb); */
        continue; /* by pass*/
        /* For each child_pb_graph_node input pins*/
        for (iport = 0; iport < child_pb_graph_node->num_input_ports; iport++) {
          for (ipin = 0; ipin < child_pb_graph_node->num_input_pins[iport]; ipin++) {
            path_id = 0;
            /* Write the interconnection*/
            fprint_spice_mux_testbench_pb_graph_node_pin_interc(fp, 
                                                                INPUT2INPUT_INTERC,
                                                                &(child_pb_graph_node->input_pins[iport][ipin]),
                                                                cur_mode,
                                                                path_id);
          }
        }
        /* TODO: for clock pins, we should do the same work */
        for (iport = 0; iport < child_pb_graph_node->num_clock_ports; iport++) {
          for (ipin = 0; ipin < child_pb_graph_node->num_clock_pins[iport]; ipin++) {
            path_id = 0;
            /* Write the interconnection*/
            fprint_spice_mux_testbench_pb_graph_node_pin_interc(fp, 
                                                                INPUT2INPUT_INTERC,
                                                                &(child_pb_graph_node->clock_pins[iport][ipin]),
                                                                cur_mode,
                                                                path_id);
          }  
        }
        break;
      }
      /* Get pb_rr_graph of current pb*/
      pb_rr_nodes = child_pb->rr_graph;
      /* For each child_pb_graph_node input pins*/
      for (iport = 0; iport < child_pb_graph_node->num_input_ports; iport++) {
        for (ipin = 0; ipin < child_pb_graph_node->num_input_pins[iport]; ipin++) {
          /* Get the index of the edge that are selected to pass signal*/
          node_index = child_pb_graph_node->input_pins[iport][ipin].pin_count_in_cluster;
          prev_node = pb_rr_nodes[node_index].prev_node;
          prev_edge = pb_rr_nodes[node_index].prev_edge;
          /* Bypass unmapped interc */
          if (OPEN == pb_rr_nodes[node_index].net_num) {
            continue;
          }
          /* Make sure this pb_rr_node is not OPEN and is not a primitive output*/
          if (OPEN == prev_node) {
            path_id = 0;
            //break; /* TODO: if there exist parasitic input waveforms, we should print the interc */
          } else {
            /* Find the path_id */
            path_id = find_path_id_between_pb_rr_nodes(pb_rr_nodes, prev_node, node_index);
            assert(-1 != path_id);
          }
          /* Write the interconnection*/
          fprint_spice_mux_testbench_pb_pin_interc(fp, pb_rr_nodes, child_pb,
                                                   INPUT2INPUT_INTERC,
                                                   &(child_pb_graph_node->input_pins[iport][ipin]),
                                                   cur_mode,
                                                   path_id);
        }
      }
      /* TODO: for clock pins, we should do the same work */
      for (iport = 0; iport < child_pb_graph_node->num_clock_ports; iport++) {
        for (ipin = 0; ipin < child_pb_graph_node->num_clock_pins[iport]; ipin++) {
          /* Get the index of the edge that are selected to pass signal*/
          node_index = child_pb_graph_node->input_pins[iport][ipin].pin_count_in_cluster;
          prev_node = pb_rr_nodes[node_index].prev_node;
          prev_edge = pb_rr_nodes[node_index].prev_edge;
          /* Bypass unmapped interc */
          if (OPEN == pb_rr_nodes[node_index].net_num) {
            continue;
          }
          /* Make sure this pb_rr_node is not OPEN and is not a primitive output*/
          if (OPEN == prev_node) {
            path_id = 0;
            //break; /* TODO: if there exist parasitic input waveforms, we should print the interc */
          } else {
            /* Find the path_id */
            path_id = find_path_id_between_pb_rr_nodes(pb_rr_nodes, prev_node, node_index);
            assert(-1 != path_id);
          }
          /* Write the interconnection*/
          fprint_spice_mux_testbench_pb_pin_interc(fp, pb_rr_nodes, child_pb,
                                                   INPUT2INPUT_INTERC,
                                                   &(child_pb_graph_node->clock_pins[iport][ipin]),
                                                   cur_mode,
                                                   path_id);
        }
      }
    }
  }

  return; 
}

static 
void fprint_spice_mux_testbench_pb_muxes_rec(FILE* fp,
                                             t_pb* cur_pb) {
  int ipb, jpb;
  int mode_index;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert(NULL != cur_pb);

  /* If we touch the leaf, there is no need print interc*/
  if (OPEN == cur_pb->logical_block) {
    /* Print MUX interc at current-level pb*/
    fprint_spice_mux_testbench_pb_interc(fp, cur_pb);
  } else {
    return;
  }
  
  /* Go recursively ... */
  mode_index = cur_pb->mode;
  for (ipb = 0; ipb < cur_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb->pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Refer to pack/output_clustering.c [LINE 392] */
      if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
        fprint_spice_mux_testbench_pb_muxes_rec(fp, &(cur_pb->child_pbs[ipb][jpb]));
      } else {
        /* Print idle muxes */
        /* Bypass idle muxes */
        /*
        fprint_spice_mux_testbench_idle_pb_graph_node_muxes_rec(fp, cur_pb->child_pbs[ipb][jpb].pb_graph_node);
        */
      }
    }
  }
  
  return;
}

static 
float find_spice_mux_testbench_rr_mux_load_inv_size(t_rr_node* load_rr_node,
                                                    int switch_index) {
  float load_inv_size = 0;
  t_spice_model* fan_out_spice_model = NULL;

  fan_out_spice_model = switch_inf[switch_index].spice_model;

  /* Check */
  assert(NULL != fan_out_spice_model);
  assert(NULL != fan_out_spice_model->input_buffer);

  /* depend on the input_buffer type */
  if (1 == fan_out_spice_model->input_buffer->exist) {
    switch(fan_out_spice_model->input_buffer->type) {
    case SPICE_MODEL_BUF_INV:
      load_inv_size = fan_out_spice_model->input_buffer->size;
      break;
    case SPICE_MODEL_BUF_BUF:
      load_inv_size = 1;
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid fanout spice_model input_buffer type!\n",
                 __FILE__, __LINE__);
      exit(1);
    }
  } else {
    /* TODO: If there is no inv/buffer at input, we should traversal until there is one 
     * However, now we just simply give a minimum sized inverter
     */
    load_inv_size = 1;
  }
 
  return load_inv_size;

}

static 
void fprint_spice_mux_testbench_cb_one_mux(FILE* fp,
                                           t_rr_type chan_type,
                                           int cb_x,
                                           int cb_y,
                                           t_rr_node* src_rr_node) {
  int mux_size;
  int iedge, inode, path_id, switch_index;
  t_rr_node** drive_rr_nodes = NULL;
  t_spice_model* mux_spice_model = NULL;
  int* mux_sram_bits = NULL;
  float* input_density = NULL;
  float* input_probability = NULL;
  float load_inv_size = 0;
  char* meas_tag = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > cb_x))&&(!(cb_x > (nx + 1)))); 
  assert((!(0 > cb_y))&&(!(cb_y > (ny + 1)))); 

  /* Find drive_rr_nodes*/
  mux_size = src_rr_node->num_drive_rr_nodes;
  assert(mux_size == src_rr_node->fan_in);
  drive_rr_nodes = src_rr_node->drive_rr_nodes; 

  /* Find path_id */
  path_id = -1;
  for (inode = 0; inode < mux_size; inode++) {
    if (drive_rr_nodes[inode] == &(rr_node[src_rr_node->prev_node])) { 
      path_id = inode;
      break;
    } 
  }
  assert((-1 != path_id)&&(path_id < mux_size));  

  switch_index = src_rr_node->drive_switches[path_id];

  mux_spice_model = switch_inf[switch_index].spice_model;

  /* Check : 
   * MUX should have at least 2 fan_in
   */
  assert((2 == mux_size)||(2 < mux_size));
  /* 2. spice_model is a wire */ 
  assert(SPICE_MODEL_MUX == mux_spice_model->type);

  input_density = (float*)my_malloc(sizeof(float)*mux_size);
  input_probability = (float*)my_malloc(sizeof(float)*mux_size);
  for (inode = 0; inode < mux_size; inode++) {
    /* Find activity information */
    input_density[inode] = get_rr_node_net_density(*(drive_rr_nodes[inode]));
    input_probability[inode] = get_rr_node_net_probability(*(drive_rr_nodes[inode]));
  }

  /* Build meas_tag: cb_mux[cb_x][cb_y]_rrnode[node]*/
  meas_tag = (char*)my_malloc(sizeof(char)*(7 + strlen(my_itoa(cb_x)) + 2
                                            + strlen(my_itoa(cb_y)) + 9 
                                            + strlen(my_itoa(src_rr_node-rr_node)) + 2)); /* Add '0'*/
  sprintf(meas_tag, "cb_mux[%d][%d]_rrnode[%ld]", cb_x, cb_y, src_rr_node-rr_node);
  /* Print the main part of a single MUX testbench */
  fprint_spice_mux_testbench_one_mux(fp, meas_tag, mux_spice_model,
                                     src_rr_node->fan_in, input_density, input_probability, path_id);

  /* add a load representing each fan-out */ 
  for (iedge = 0; iedge < src_rr_node->num_edges; iedge++) {
    /* Detect its input buffers */
    load_inv_size = find_spice_mux_testbench_rr_mux_load_inv_size(&(rr_node[src_rr_node->edges[iedge]]), src_rr_node->switches[iedge]);
    /* Print an inverter */
    fprintf(fp, "Xload_inv[%d] %s_size%d[%d]->out load_inv[%d]_out gvdd_load 0 inv size=%g\n",
            testbench_load_cnt, mux_spice_model->prefix, mux_size, testbench_mux_cnt, testbench_load_cnt, load_inv_size);
    testbench_load_cnt++;
  }

  fprint_spice_mux_testbench_cb_mux_meas(fp, meas_tag);
  /* Update the counter */
  testbench_mux_cnt++;

  /* Free */
  my_free(mux_sram_bits);

  return;
}

void fprint_spice_mux_testbench_cb_interc(FILE* fp, 
                                          t_rr_type chan_type,
                                          int cb_x, int cb_y,
                                          t_rr_node* src_rr_node) {
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > cb_x))&&(!(cb_x > (nx + 1)))); 
  assert((!(0 > cb_y))&&(!(cb_y > (ny + 1)))); 
  /* Skip non-mapped CB MUX */
  if (OPEN == src_rr_node->net_num) {
    return;
  }

  if (1 == src_rr_node->fan_in) {
    /* By-pass a direct connection*/
    return;
  } else if (!(2 > src_rr_node->fan_in)) {
    /* Print a MUX */
    fprint_spice_mux_testbench_cb_one_mux(fp, chan_type, cb_x, cb_y, src_rr_node);
  } 
   
  return;
}

static 
void fprint_spice_mux_testbench_cb_muxes(FILE* fp, 
                                         t_rr_type chan_type,
                                         int x, int y,
                                         t_ivec*** LL_rr_node_indices) {
  int inode, side;
  int side_cnt = 0;
  int num_ipin_rr_node = 0;
  t_rr_node** ipin_rr_nodes = NULL;
  int num_temp_rr_node = 0;
  t_rr_node** temp_rr_nodes = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 

  /* Print the ports of grids*/
  side_cnt = 0;
  num_ipin_rr_node = 0;  
  for (side = 0; side < 4; side++) {
    switch (side) {
    case 0: /* TOP */
      switch(chan_type) { 
      case CHANX:
        /* Collect IPIN rr_nodes*/ 
        temp_rr_nodes = get_grid_side_pin_rr_nodes(&num_temp_rr_node, IPIN, x, y + 1, 2, LL_rr_node_indices);
        /* Update the ipin_rr_nodes, if ipin_rr_nodes is NULL, realloc will do a pure malloc */
        ipin_rr_nodes = (t_rr_node**)realloc(ipin_rr_nodes, sizeof(t_rr_node*)*(num_ipin_rr_node + num_temp_rr_node));
        for (inode = num_ipin_rr_node; inode < (num_ipin_rr_node + num_temp_rr_node); inode++) {
          ipin_rr_nodes[inode] = temp_rr_nodes[inode - num_ipin_rr_node];
        } 
        /* Count in the new members*/
        num_ipin_rr_node += num_temp_rr_node; 
        /* Free the temp_ipin_rr_node */
        my_free(temp_rr_nodes);
        num_temp_rr_node = 0;
        side_cnt++;
        break; 
      case CHANY:
        /* Nothing should be done */
        break;
      default: 
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
        exit(1);
      }
      break;
    case 1: /* RIGHT */
      switch(chan_type) { 
      case CHANX:
        /* Nothing should be done */
        break; 
      case CHANY:
        /* Collect IPIN rr_nodes*/ 
        temp_rr_nodes = get_grid_side_pin_rr_nodes(&num_temp_rr_node, IPIN, x + 1, y, 3, LL_rr_node_indices);
        /* Update the ipin_rr_nodes, if ipin_rr_nodes is NULL, realloc will do a pure malloc */
        ipin_rr_nodes = (t_rr_node**)realloc(ipin_rr_nodes, sizeof(t_rr_node*)*(num_ipin_rr_node + num_temp_rr_node));
        for (inode = num_ipin_rr_node; inode < (num_ipin_rr_node + num_temp_rr_node); inode++) {
          ipin_rr_nodes[inode] = temp_rr_nodes[inode - num_ipin_rr_node];
        } 
        /* Count in the new members*/
        num_ipin_rr_node += num_temp_rr_node; 
        /* Free the temp_ipin_rr_node */
        my_free(temp_rr_nodes);
        num_temp_rr_node = 0;
        side_cnt++;
        break;
      default: 
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
        exit(1);
      }
      break;
    case 2: /* BOTTOM */
      switch(chan_type) { 
      case CHANX:
        /* Collect IPIN rr_nodes*/ 
        temp_rr_nodes = get_grid_side_pin_rr_nodes(&num_temp_rr_node, IPIN, x, y, 0, LL_rr_node_indices);
        /* Update the ipin_rr_nodes, if ipin_rr_nodes is NULL, realloc will do a pure malloc */
        ipin_rr_nodes = (t_rr_node**)realloc(ipin_rr_nodes, sizeof(t_rr_node*)*(num_ipin_rr_node + num_temp_rr_node));
        for (inode = num_ipin_rr_node; inode < (num_ipin_rr_node + num_temp_rr_node); inode++) {
          ipin_rr_nodes[inode] = temp_rr_nodes[inode - num_ipin_rr_node];
        } 
        /* Count in the new members*/
        num_ipin_rr_node += num_temp_rr_node; 
        /* Free the temp_ipin_rr_node */
        my_free(temp_rr_nodes);
        num_temp_rr_node = 0;
        side_cnt++;
        break; 
      case CHANY:
        /* Nothing should be done */
        break;
      default: 
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
        exit(1);
      }
      break;
    case 3: /* LEFT */
      switch(chan_type) { 
      case CHANX:
        /* Nothing should be done */
        break; 
      case CHANY:
        /* Collect IPIN rr_nodes*/ 
        temp_rr_nodes = get_grid_side_pin_rr_nodes(&num_temp_rr_node, IPIN, x, y, 1, LL_rr_node_indices);
        /* Update the ipin_rr_nodes, if ipin_rr_nodes is NULL, realloc will do a pure malloc */
        ipin_rr_nodes = (t_rr_node**)realloc(ipin_rr_nodes, sizeof(t_rr_node*)*(num_ipin_rr_node + num_temp_rr_node));
        for (inode = num_ipin_rr_node; inode < (num_ipin_rr_node + num_temp_rr_node); inode++) {
          ipin_rr_nodes[inode] = temp_rr_nodes[inode - num_ipin_rr_node];
        } 
        /* Count in the new members*/
        num_ipin_rr_node += num_temp_rr_node; 
        /* Free the temp_ipin_rr_node */
        my_free(temp_rr_nodes);
        num_temp_rr_node = 0;
        side_cnt++;
        break;
      default: 
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
        exit(1);
      }
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid side index!\n", __FILE__, __LINE__);
      exit(1);
    }
  }
  /* Check */
  assert(2 == side_cnt);

  /* Print multiplexers */
  for (inode = 0; inode < num_ipin_rr_node; inode++) {
    fprint_spice_mux_testbench_cb_interc(fp, chan_type, x, y, ipin_rr_nodes[inode]);
  } 

  /* Free */
  my_free(ipin_rr_nodes);

  return;
}

static 
char* fprint_spice_mux_testbench_rr_node_load_version(FILE* fp,
                                                      int load_level, 
                                                      int fanout_index,
                                                      t_rr_node cur_rr_node, 
                                                      char* outport_name) {
  char* ret_outport_name = NULL;
  int cost_index;
  int iseg, i, chan_wire_length;
  t_spice_model* wire_spice_model = NULL;

  switch(cur_rr_node.type) {
  case CHANX:
  case CHANY:
    cost_index = cur_rr_node.cost_index;
    iseg = rr_indexed_data[cost_index].seg_index;
    assert((!(iseg < 0))&&(iseg < num_segments));
    wire_spice_model = segments[iseg].spice_model;
    assert(SPICE_MODEL_CHAN_WIRE == wire_spice_model->type);
    ret_outport_name = (char*)my_malloc(sizeof(char)*( strlen(outport_name)
                       + 9 + strlen(my_itoa(load_level)) + 6
                       + strlen(my_itoa(fanout_index)) + 2 ));
    chan_wire_length = cur_rr_node.xhigh - cur_rr_node.xlow 
                     + cur_rr_node.yhigh - cur_rr_node.ylow;
    assert((0 == cur_rr_node.xhigh - cur_rr_node.xlow)
          ||(0 == cur_rr_node.yhigh - cur_rr_node.ylow));
    for (i = 0; i < chan_wire_length + 1; i++) { 
      sprintf(ret_outport_name,"%s_loadlvl[%d]_out[%d]",
              outport_name, load_level + i, fanout_index);
      if (0 == i) {
        fprintf(fp, "X%s %s %s 0 gvdd_load 0 %s_seg%d\n",
                ret_outport_name, outport_name, ret_outport_name, 
                wire_spice_model->name, iseg); 
      } else {
        fprintf(fp, "X%s %s_loadlvl[%d]_out[%d] %s 0 gvdd_load 0 %s_seg%d\n",
                ret_outport_name, outport_name, load_level + i -1,
                fanout_index, ret_outport_name, 
                wire_spice_model->name, iseg); 
      }
    }
    break;
  case SOURCE:
  case SINK:
  case IPIN:
  case OPIN:
  case INTRA_CLUSTER_EDGE:
  case NUM_RR_TYPES:
    ret_outport_name = my_strdup(outport_name);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid rr_node type!\n", __FILE__, __LINE__);
    exit(1);
  }

  return ret_outport_name;
} 

static 
void fprint_spice_mux_testbench_sb_one_mux(FILE* fp,
                                           int switch_box_x, int switch_box_y,
                                           int chan_side,
                                           t_rr_node* src_rr_node) {
  int iedge, inode, switch_index, mux_size;
  t_spice_model* mux_spice_model = NULL;
  float* input_density = NULL;
  float* input_probability = NULL;
  int path_id = -1;
  char* meas_tag = NULL;
  int num_drive_rr_nodes = 0;  
  t_rr_node** drive_rr_nodes = NULL;
  float load_inv_size = 0.;
  char* outport_name = NULL;
  char* rr_node_outport_name = NULL;

  /* Check */
  assert((!(0 > switch_box_x))&&(!(switch_box_x > (nx + 1)))); 
  assert((!(0 > switch_box_y))&&(!(switch_box_y > (ny + 1)))); 

  /* ignore idle sb mux */
  if (OPEN == src_rr_node->net_num) {
    return;
  }

  find_drive_rr_nodes_switch_box(switch_box_x, switch_box_y, src_rr_node, chan_side, 0, 
                                 &num_drive_rr_nodes, &drive_rr_nodes, &switch_index);

  /* Print MUX only when fan-in >= 2 */
  if (2 > num_drive_rr_nodes) {
    return;
  }

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }


  /* Find mux_spice_model, mux_size */
  mux_size = num_drive_rr_nodes;

  mux_spice_model = switch_inf[switch_index].spice_model;
  assert(NULL != mux_spice_model);

  /* input_density, input_probability */
  input_density = (float*)my_malloc(sizeof(float)*mux_size);
  input_probability = (float*)my_malloc(sizeof(float)*mux_size);
  for (inode = 0; inode < mux_size; inode++) {
    input_density[inode] = get_rr_node_net_density(*(drive_rr_nodes[inode]));
    input_probability[inode] = get_rr_node_net_probability(*(drive_rr_nodes[inode]));
  }

  /* Find path_id */
  path_id = -1;
  for (inode = 0; inode < mux_size; inode++) {
    if (drive_rr_nodes[inode] == &(rr_node[src_rr_node->prev_node])) { 
      path_id = inode;
      break;
    } 
  }
  assert((-1 != path_id)&&(path_id < mux_size));  

  /* Build meas_tag: sb_mux[sb_x][sb_y]_rrnode[node]*/
  meas_tag = (char*)my_malloc(sizeof(char)*(7 + strlen(my_itoa(switch_box_x)) + 2
                                            + strlen(my_itoa(switch_box_y)) + 9 
                                            + strlen(my_itoa(src_rr_node-rr_node)) + 2)); /* Add '0'*/
  sprintf(meas_tag, "sb_mux[%d][%d]_rrnode[%ld]", switch_box_x, switch_box_y, src_rr_node-rr_node);
  /* Print MUX */
  fprint_spice_mux_testbench_one_mux(fp, meas_tag, mux_spice_model, mux_size,
                                     input_density, input_probability, path_id);

  /* Print a channel wire !*/
  outport_name = (char*)my_malloc(sizeof(char)*( strlen(mux_spice_model->prefix)
                                 + 5 + strlen(my_itoa(mux_size)) + 1 
                                 + strlen(my_itoa(testbench_mux_cnt))
                                 + 6 + 1 ));
  sprintf(outport_name, "%s_size%d[%d]->out", mux_spice_model->prefix, mux_size, testbench_mux_cnt);
  rr_node_outport_name = fprint_spice_mux_testbench_rr_node_load_version(fp, 0, 0, (*src_rr_node), outport_name); 
  /* add a load representing each fan-out */ 
  for (iedge = 0; iedge < src_rr_node->num_edges; iedge++) {
    /* Detect its input buffers */
    load_inv_size = find_spice_mux_testbench_rr_mux_load_inv_size(&rr_node[src_rr_node->edges[iedge]], src_rr_node->switches[iedge]);
    /* Print an inverter */
    fprintf(fp, "Xload_inv[%d] %s load_inv[%d]_out gvdd_load 0 inv size=%g\n",
            testbench_load_cnt, rr_node_outport_name, testbench_load_cnt, load_inv_size);
    testbench_load_cnt++;
  }

  fprint_spice_mux_testbench_sb_mux_meas(fp, meas_tag);

  /* Update the counter */
  testbench_mux_cnt++;

  /* Free */
  my_free(drive_rr_nodes);
  my_free(input_density);
  my_free(input_probability);

  return;
}

static 
void fprint_spice_mux_testbench_sb_muxes(FILE* fp, 
                                         int x, int y, 
                                         t_ivec*** LL_rr_node_indices) {
  int itrack, inode, side, ix, iy;
  t_rr_node*** chan_rr_nodes = (t_rr_node***)my_malloc(sizeof(t_rr_node**)*4); /* 4 sides*/
  int* chan_width = (int*)my_malloc(sizeof(int)*4); /* 4 sides */

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 

  /* Find all rr_nodes of channels */
  for (side = 0; side < 4; side++) {
    switch (side) {
    case 0:
      /* For the bording, we should take special care */
      if (y == ny) {
        chan_width[side] = 0;
        chan_rr_nodes[side] = NULL;
        break;
      }
      /* Routing channels*/
      ix = x; 
      iy = y + 1;
      /* Channel width */
      chan_width[side] = chan_width_y[ix];
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      chan_rr_nodes[side] = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*chan_width[side]);
      /* Collect rr_nodes for Tracks for top: chany[x][y+1] */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANY, itrack, LL_rr_node_indices);
        chan_rr_nodes[0][itrack] = &(rr_node[inode]);
        if (INC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          fprint_spice_mux_testbench_sb_one_mux(fp, ix, iy, side, chan_rr_nodes[side][itrack]);
        }
      }
      break;
    case 1:
      /* For the bording, we should take speical care */
      if (x == nx) {
        chan_width[side] = 0;
        chan_rr_nodes[side] = NULL;
        break;
      }
      /* Routing channels*/
      ix = x + 1; 
      iy = y;
      /* Channel width */
      chan_width[side] = chan_width_x[iy];
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      chan_rr_nodes[side] = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*chan_width[side]);
      /* Collect rr_nodes for Tracks for right: chanX[x+1][y] */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANX, itrack, LL_rr_node_indices);
        chan_rr_nodes[1][itrack] = &(rr_node[inode]);
        if (INC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          fprint_spice_mux_testbench_sb_one_mux(fp, ix, iy, side, chan_rr_nodes[side][itrack]);
        }
      }
      /* Print MUXes of RIGHT side */
      break;
    case 2:
      /* For the bording, we should take speical care */
      if (y == 0) {
        chan_width[side] = 0;
        chan_rr_nodes[side] = NULL;
        break;
      }
      /* Routing channels*/
      ix = x; 
      iy = y;
      /* Channel width */
      chan_width[side] = chan_width_y[ix];
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      chan_rr_nodes[side] = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*chan_width[side]);
      /* Collect rr_nodes for Tracks for bottom: chany[x][y] */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANY, itrack, LL_rr_node_indices);
        chan_rr_nodes[2][itrack] = &(rr_node[inode]);
        if (DEC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          fprint_spice_mux_testbench_sb_one_mux(fp, ix, iy, side, chan_rr_nodes[side][itrack]);
        }
      }
      /* Print MUXes of BOTTOM side */
      break;
    case 3:
      /* For the bording, we should take speical care */
      if (x == 0) {
        chan_width[side] = 0;
        chan_rr_nodes[side] = NULL;
        break;
      }
      /* Routing channels*/
      ix = x; 
      iy = y;
      /* Channel width */
      chan_width[side] = chan_width_x[iy];
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      chan_rr_nodes[side] = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*chan_width[side]);
      /* Collect rr_nodes for Tracks for left: chanx[x][y] */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANX, itrack, LL_rr_node_indices);
        chan_rr_nodes[3][itrack] = &(rr_node[inode]);
        if (DEC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          fprint_spice_mux_testbench_sb_one_mux(fp, ix, iy, side, chan_rr_nodes[side][itrack]);
        }
      }
      /* Print MUXes of LEFT side */
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid side index!\n", __FILE__, __LINE__);
      exit(1);
    }
  }

  /* Free */
  my_free(chan_width);
  for (side = 0; side < 4; side++) {
    my_free(chan_rr_nodes[side]);
  }
  my_free(chan_rr_nodes);
}

static 
void fprint_spice_mux_testbench_call_routing_muxes(FILE* fp,
                                                   t_ivec*** LL_rr_node_indices) {
  int ix, iy;

  /* Find all routing Switch Boxes and Connection Blocks */
  /* Print Connection Block MUXes */
  for (iy = 0; iy < (ny + 1); iy++) { 
    for (ix = 1; ix < (nx + 1); ix++) {
      fprint_spice_mux_testbench_cb_muxes(fp, CHANX, ix, iy, LL_rr_node_indices);
    }
  }

  for (ix = 0; ix < (nx + 1); ix++) { 
    for (iy = 1; iy < (ny + 1); iy++) {
      fprint_spice_mux_testbench_cb_muxes(fp, CHANY, ix, iy, LL_rr_node_indices);
    }
  }

  /* Print Switch Box MUXes */
  for (iy = 0; iy < (ny + 1); iy++) { 
    for (ix = 0; ix < (nx + 1); ix++) {
      fprint_spice_mux_testbench_sb_muxes(fp, ix, iy, LL_rr_node_indices);
    }
  }

  return;
}

static 
void fprint_spice_mux_testbench_call_defined_muxes(FILE* fp,
                                                   t_ivec*** LL_rr_node_indices) {
  int iblk, ix, iy;

  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Print all the grid */
  for (iy = 1; iy < (ny + 1); iy++) { 
    for (ix = 1; ix < (nx + 1); ix++) {
      if (NULL == grid[ix][iy].type) {
        continue;
      }
      /* Used blocks */
      for (iblk = 0; iblk < grid[ix][iy].usage; iblk++) {
        /* Only for mapped block */
        assert(NULL != block[grid[ix][iy].blocks[iblk]].pb);
        fprint_spice_mux_testbench_pb_muxes_rec(fp, block[grid[ix][iy].blocks[iblk]].pb); 
      }  
      continue;
      /* By pass Unused blocks */
      /*
      for (iblk = grid[ix][iy].usage; iblk < grid[ix][iy].type->capacity; iblk++) {
        fprint_spice_mux_testbench_idle_pb_graph_node_muxes_rec(fp, grid[ix][iy].type->pb_graph_head);
      } 
      */
    }
  }
  /* Find all routing Switch Boxes and Connection Blocks */
  /* Print Connection Block MUXes */
  for (iy = 0; iy < (ny + 1); iy++) { 
    for (ix = 1; ix < (nx + 1); ix++) {
      fprint_spice_mux_testbench_cb_muxes(fp, CHANX, ix, iy, LL_rr_node_indices);
    }
  }

  for (ix = 0; ix < (nx + 1); ix++) { 
    for (iy = 1; iy < (ny + 1); iy++) {
      fprint_spice_mux_testbench_cb_muxes(fp, CHANY, ix, iy, LL_rr_node_indices);
    }
  }

  /* Print Switch Box MUXes */
  for (iy = 0; iy < (ny + 1); iy++) { 
    for (ix = 0; ix < (nx + 1); ix++) {
      fprint_spice_mux_testbench_sb_muxes(fp, ix, iy, LL_rr_node_indices);
    }
  }

  return;
}

static 
void fprint_spice_mux_testbench_stimulations(FILE* fp, 
                                             t_spice spice) {
  /* Voltage sources of Multiplexers are already generated during printing the netlist 
   * We just need global stimulations Here.
   */

  /* Give global vdd, gnd, voltage sources*/
  /* A valid file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }
  /* Global GND */
  fprintf(fp, "***** Global GND port *****\n");
  fprintf(fp, "Rggnd ggnd 0 0\n");

  /* Global set and reset */
  fprintf(fp, "***** Global Net for reset signal *****\n");
  fprintf(fp, "Vgvreset greset 0 0\n");
  fprintf(fp, "***** Global Net for set signal *****\n");
  fprintf(fp, "Vgvset gset 0 0\n");

  /* Global Vdd ports */
  fprintf(fp, "***** Global VDD ports *****\n");
  fprintf(fp, "Vgvdd gvdd 0 vsp\n");
  fprintf(fp, "***** Global VDD for SRAMs *****\n");
  fprintf(fp, "Vgvdd_sram gvdd_sram 0 vsp\n");
  fprintf(fp, "***** Global VDD for load inverters *****\n");
  fprintf(fp, "Vgvdd_load gvdd_load 0 vsp\n");
  fprintf(fp, "***** Global Force for all SRAMs *****\n");
  fprintf(fp, "V%s->in %s->in 0 0\n", 
          sram_spice_model->prefix, sram_spice_model->prefix);
  fprintf(fp, ".nodeset V(%s->in) 0\n", sram_spice_model->prefix);

  return;
}

static 
void fprint_spice_mux_testbench_measurements(FILE* fp, 
                                             t_spice spice) {
  int num_clock_cycle = spice.spice_params.meas_params.sim_num_clock_cycle + 1;
  /*
  int i;
  t_llist* head = NULL;
  t_spice_mux_model* spice_mux_model = NULL;  
  */

  /* Give global vdd, gnd, voltage sources*/
  /* A valid file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  fprint_spice_netlist_transient_setting(fp, spice, FALSE);
  /* Measure the leakage and dynamic power of SRAMs*/
  fprintf(fp, ".meas tran total_leakage_srams avg p(Vgvdd_sram) from=0 to=\'clock_period\'\n");
  fprintf(fp, ".meas tran total_dynamic_srams avg p(Vgvdd_sram) from=\'clock_period\' to=\'%d*clock_period\'\n", num_clock_cycle);

  /* Measure the total leakage and dynamic power */
  fprintf(fp, ".meas tran total_leakage_power_mux[0to%d] \n", testbench_mux_cnt - 1);
  fprintf(fp, "+          param=\'sum_leakage_power_mux[0to%d]\'\n", testbench_mux_cnt-1);
  fprintf(fp, ".meas tran total_dynamic_power_mux[0to%d] \n", testbench_mux_cnt - 1);
  fprintf(fp, "+          param=\'sum_dynamic_power_mux[0to%d]\'\n", testbench_mux_cnt-1);
  /* pb_muxes */
  fprintf(fp, ".meas tran total_leakage_power_pb_mux \n");
  fprintf(fp, "+          param=\'sum_leakage_power_pb_mux[0to%d]\'\n", testbench_pb_mux_cnt-1);
  fprintf(fp, ".meas tran total_dynamic_power_pb_mux \n");
  fprintf(fp, "+          param=\'sum_dynamic_power_pb_mux[0to%d]\'\n", testbench_pb_mux_cnt-1);
  /* cb_muxes */
  fprintf(fp, ".meas tran total_leakage_power_cb_mux \n");
  fprintf(fp, "+          param=\'sum_leakage_power_cb_mux[0to%d]\'\n", testbench_cb_mux_cnt-1);
  fprintf(fp, ".meas tran total_dynamic_power_cb_mux \n");
  fprintf(fp, "+          param=\'sum_dynamic_power_cb_mux[0to%d]\'\n", testbench_cb_mux_cnt-1);
  /* sb_muxes */
  fprintf(fp, ".meas tran total_leakage_power_sb_mux \n");
  fprintf(fp, "+          param=\'sum_leakage_power_sb_mux[0to%d]\'\n", testbench_sb_mux_cnt-1);
  fprintf(fp, ".meas tran total_dynamic_power_sb_mux \n");
  fprintf(fp, "+          param=\'sum_dynamic_power_sb_mux[0to%d]\'\n", testbench_sb_mux_cnt-1);

  /* We don not measure the power of SRAM and load !*/
  /* Sum the total MUX leakage power and dynamic power */
  /*
  fprintf(fp, ".measure tran total_leakage_power param='0+\n");
  head = testbench_muxes_head; 
  while(head) {
    spice_mux_model = (t_spice_mux_model*)(head->dptr);
    for (i = 0; i < spice_mux_model->cnt; i++) {
      fprintf(fp, "+ %s_size%d[%d]_leakage_power+\n",
              spice_mux_model->spice_model->prefix, spice_mux_model->size, i);
    } 
    head = head->next;
  }
  fprintf(fp, "+ 0'\n");

  fprintf(fp, ".measure tran total_dynamic_power param='0+\n");
  head = testbench_muxes_head; 
  while(head) {
    spice_mux_model = (t_spice_mux_model*)(head->dptr);
    for (i = 0; i < spice_mux_model->cnt; i++) {
      fprintf(fp, "+ %s_size%d[%d]_dynamic_power+\n",
              spice_mux_model->spice_model->prefix, spice_mux_model->size, i);
    } 
    head = head->next;
  }
  fprintf(fp, "+ 0'\n");
  */

  return;
}

/* Top-level function in this source file */
void fprint_spice_mux_testbench(char* formatted_spice_dir,
                                char* circuit_name,
                                char* mux_testbench_name,
                                char* include_dir_path,
                                char* subckt_dir_path,
                                t_ivec*** LL_rr_node_indices,
                                int num_clocks,
                                t_arch arch,
                                boolean leakage_only) {
  FILE* fp = NULL;
  char* formatted_subckt_dir_path = format_dir_path(subckt_dir_path);
  char* title = my_strcat("FPGA SPICE MUX Test Bench for Design: ", circuit_name);
  char* mux_testbench_file_path = my_strcat(formatted_spice_dir, mux_testbench_name);

  /* Check if the path exists*/
  fp = fopen(mux_testbench_file_path,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create SPICE MUX Test bench netlist %s!\n",__FILE__, __LINE__, mux_testbench_file_path); 
    exit(1);
  } 
  
  vpr_printf(TIO_MESSAGE_INFO, "Writing SPICE MUX Test Bench for %s...\n", circuit_name);
 
  /* Load global vars in this source file */
  num_segments = arch.num_segments;
  segments = arch.Segments;

  /* Print the title */
  fprint_spice_head(fp, title);
  my_free(title);

  /* print technology library and design parameters*/
  fprint_tech_lib(fp, arch.spice->tech_lib);

  /* Include parameter header files */
  fprint_spice_include_param_headers(fp, include_dir_path);

  /* Include Key subckts */
  fprint_spice_include_key_subckts(fp, formatted_subckt_dir_path);

  /* Include user-defined sub-circuit netlist */
  init_include_user_defined_netlists(*(arch.spice));
  fprint_include_user_defined_netlists(fp, *(arch.spice));
  
  /* Special subckts for Top-level SPICE netlist */
  /*
  fprintf(fp, "****** Include subckt netlists: Look-Up Tables (LUTs) *****\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, luts_spice_file_name);
  fprintf(fp, ".include %s\n", temp_include_file_path);
  my_free(temp_include_file_path);
  */

  /* Print simulation temperature and other options for SPICE */
  fprint_spice_options(fp, arch.spice->spice_params);

  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprint_spice_mux_testbench_global_ports(fp, *(arch.spice));
 
  /* Quote defined Logic blocks subckts (Grids) */
  init_spice_mux_testbench_globals();
  fprint_spice_mux_testbench_call_defined_muxes(fp, LL_rr_node_indices);

  /* Add stimulations */
  fprint_spice_mux_testbench_stimulations(fp, *(arch.spice));

  /* Add measurements */  
  fprint_spice_mux_testbench_measurements(fp, *(arch.spice));

  /* SPICE ends*/
  fprintf(fp, ".end\n");

  /* Close the file*/
  fclose(fp);

  /* Free */
  //my_free(formatted_subckt_dir_path);
  //my_free(mux_testbench_file_path);
  //my_free(title);

  return;
}

/* Top-level function in this source file */
void fprint_spice_routing_mux_testbench(char* formatted_spice_dir,
                                        char* circuit_name,
                                        char* mux_testbench_name,
                                        char* include_dir_path,
                                        char* subckt_dir_path,
                                        t_ivec*** LL_rr_node_indices,
                                        int num_clocks,
                                        t_arch arch,
                                        boolean leakage_only) {
  FILE* fp = NULL;
  char* formatted_subckt_dir_path = format_dir_path(subckt_dir_path);
  char* title = my_strcat("FPGA SPICE Routing MUX Test Bench for Design: ", circuit_name);
  char* mux_testbench_file_path = my_strcat(formatted_spice_dir, mux_testbench_name);

  /* Check if the path exists*/
  fp = fopen(mux_testbench_file_path,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create SPICE MUX Test bench netlist %s!\n",__FILE__, __LINE__, mux_testbench_file_path); 
    exit(1);
  } 
  
  vpr_printf(TIO_MESSAGE_INFO, "Writing SPICE Routing MUX Test Bench for %s...\n", circuit_name);

  /* Load global vars in this source file */
  num_segments = arch.num_segments;
  segments = arch.Segments;
 
  /* Print the title */
  fprint_spice_head(fp, title);
  my_free(title);

  /* print technology library and design parameters*/
  fprint_tech_lib(fp, arch.spice->tech_lib);

  /* Include parameter header files */
  fprint_spice_include_param_headers(fp, include_dir_path);

  /* Include Key subckts */
  fprint_spice_include_key_subckts(fp, formatted_subckt_dir_path);

  /* Include user-defined sub-circuit netlist */
  init_include_user_defined_netlists(*(arch.spice));
  fprint_include_user_defined_netlists(fp, *(arch.spice));
  
  /* Special subckts for Top-level SPICE netlist */
  /*
  fprintf(fp, "****** Include subckt netlists: Look-Up Tables (LUTs) *****\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, luts_spice_file_name);
  fprintf(fp, ".include %s\n", temp_include_file_path);
  my_free(temp_include_file_path);
  */

  /* Print simulation temperature and other options for SPICE */
  fprint_spice_options(fp, arch.spice->spice_params);

  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprint_spice_mux_testbench_global_ports(fp, (*arch.spice));
 
  /* Quote defined Logic blocks subckts (Grids) */
  init_spice_mux_testbench_globals();
  fprint_spice_mux_testbench_call_routing_muxes(fp, LL_rr_node_indices);

  /* Add stimulations */
  fprint_spice_mux_testbench_stimulations(fp, *(arch.spice));

  /* Add measurements */  
  fprint_spice_mux_testbench_measurements(fp, *(arch.spice));

  /* SPICE ends*/
  fprintf(fp, ".end\n");

  /* Close the file*/
  fclose(fp);

  /* Free */
  //my_free(formatted_subckt_dir_path);
  //my_free(mux_testbench_file_path);
  //my_free(title);

  return;
}
