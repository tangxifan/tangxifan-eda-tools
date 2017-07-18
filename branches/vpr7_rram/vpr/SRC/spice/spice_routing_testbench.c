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
#include "spice_routing.h"
#include "spice_subckt.h"

/* Global parameters */
static int num_segments;
static t_segment_inf* segments;
static int testbench_load_cnt = 0;
static int upbound_sim_num_clock_cycles = 2;
static int max_sim_num_clock_cycles = 2;
static int auto_select_max_sim_num_clock_cycles = TRUE;

static void init_spice_routing_testbench_globals(t_spice spice) {
  auto_select_max_sim_num_clock_cycles = spice.spice_params.meas_params.auto_select_sim_num_clk_cycle;
  upbound_sim_num_clock_cycles = spice.spice_params.meas_params.sim_num_clock_cycle + 1;
  if (FALSE == auto_select_max_sim_num_clock_cycles) {
    max_sim_num_clock_cycles = spice.spice_params.meas_params.sim_num_clock_cycle + 1;
  } else {
    max_sim_num_clock_cycles = 2;
  }
}


static 
void fprint_spice_routing_testbench_global_ports(FILE* fp,
                                                 t_spice spice) {
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  /* Print generic global ports*/
  fprint_spice_generic_testbench_global_ports(fp, 
                                              sram_spice_orgz_info,
                                              global_ports_head); 

  return;
}

static 
void fprintf_spice_routing_testbench_generic_stimuli(FILE* fp,
                                                     int num_clocks) {

  /* Give global vdd, gnd, voltage sources*/
  /* A valid file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

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


  return;
}


/** Print input voltage pulses/output loads in a sb/cb testbench
 * For OPIN: generate input voltage pulses
 * For IPIN: generate output loads
 */
int fprint_grid_side_pins_voltage_pulses(FILE* fp,
                                         t_rr_type pin_type,
                                         int x, int y, int side,
                                         t_ivec*** LL_rr_node_indices,
                                         float* average_input_density) {
  int height, ipin, class_id, inode;
  t_type_ptr type = NULL;
  enum e_pin_type pin_class_type;

  char* outport_name = NULL;

  float input_density;
  float input_probability;
  int input_init_value;
  int avg_density_cnt = 0;
  
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 
  type = grid[x][y].type;
  assert(NULL != type);
 
  /* Assign the type of PIN*/ 
  switch (pin_type) {
  case IPIN:
  /* case SINK: */
    pin_class_type = RECEIVER; /* This is the end of a route path*/ 
    break;
  /* case SOURCE: */
  case OPIN:
    pin_class_type = DRIVER; /* This is the start of a route path */ 
    break;
  /* SINK and SOURCE are hypothesis nodes */
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid pin_type!\n", __FILE__, __LINE__);
    exit(1); 
  }

  /* Output the pins on the side*/ 
  height = grid[x][y].offset;
  for (ipin = 0; ipin < type->num_pins; ipin++) {
    class_id = type->pin_class[ipin];
    if ((1 == type->pinloc[height][side][ipin])&&(pin_class_type == type->class_inf[class_id].type)) {
      inode = get_rr_node_index(x, y, pin_type, ipin, LL_rr_node_indices);
      switch (pin_class_type) {
      case DRIVER:
        /* Add input voltage pulses*/
        input_density = get_rr_node_net_density(rr_node[inode]);
        input_probability = get_rr_node_net_probability(rr_node[inode]);
        input_init_value = get_rr_node_net_init_value(rr_node[inode]);
        fprintf(fp, "***** Signal grid[%d][%d]_pin[%d][%d][%d] density = %g, probability=%g.*****\n",
                x, y, height, side, ipin, input_density, input_probability);
        fprintf(fp, "Vgrid[%d][%d]_pin[%d][%d][%d] grid[%d][%d]_pin[%d][%d][%d] 0 \n", 
                x, y, height, side, ipin, x, y, height, side, ipin);
        fprint_voltage_pulse_params(fp, input_init_value, input_density, input_probability);
        /* Update statistics */
        (*average_input_density) += input_density;
        if (0. < input_density) {
          avg_density_cnt++;
        }
        break;
      case RECEIVER:
        /* Malloc */
        outport_name = (char*)my_malloc(sizeof(char)*(5 + strlen(my_itoa(x)) + 2 
                                        + strlen(my_itoa(y)) + 6 
                                        + strlen(my_itoa(height)) + 2 
                                        + strlen(my_itoa(side)) + 2 
                                        + strlen(my_itoa(ipin)) + 2));
        /* Add downstream components as loads*/
        fprintf(fp, "***** Downstream loads of grid[%d][%d]_pin[%d][%d][%d]*****\n",
                x, y, height, side, ipin);
        sprintf(outport_name, "grid[%d][%d]_pin[%d][%d][%d]", x, y, height, side, ipin);
        fprint_spice_testbench_one_cb_mux_loads(fp, &testbench_load_cnt, 
                                                &rr_node[inode], outport_name, 
                                                LL_rr_node_indices);
        my_free(outport_name);
        break;
      default:
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid pin_type!\n", __FILE__, __LINE__);
        exit(1); 
      }
    }
  } 
  
  return avg_density_cnt;
}


/** In a testbench, we call the subckt of defined connection box (cbx[x][y] or cby[x][y])
 *  For each input of connection box (channel track rr_nodes), 
 *  we find their activities and generate input voltage pulses.
 *  For each output of connection box, we add all the non-inverter downstream components as load.
 */
static 
int fprint_spice_routing_testbench_call_one_cb_tb(FILE* fp,
                                                  t_rr_type chan_type,
                                                  int x, int y,
                                                  t_ivec*** LL_rr_node_indices) {
  int itrack, inode, side, chan_width;
  int side_cnt = 0;
  int used = 0;

  float input_density;
  float input_probability;
  int input_init_value;
  float average_cb_input_density = 0.;
  int avg_density_cnt = 0;

  int num_sim_clock_cycles = 0;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 

  /* call the defined switch block sb[x][y]*/
  fprintf(fp, "***** Call defined Connection Box[%d][%d] *****\n", x, y);

  switch(chan_type) { 
  case CHANX:
    chan_width = chan_width_x[y];
    fprint_call_defined_one_connection_box(fp, cbx_info[x][y]);
    break;
  case CHANY:
    chan_width = chan_width_y[x];
    fprint_call_defined_one_connection_box(fp, cby_info[x][y]);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Print input voltage pulses */
  /* connect to the mid point of a track*/
  for (itrack = 0; itrack < chan_width; itrack++) {
    switch(chan_type) { 
    case CHANX:
      inode = get_rr_node_index(x, y, CHANX, itrack, LL_rr_node_indices);
      /* Add input voltage pulses*/
      input_density = get_rr_node_net_density(rr_node[inode]);
      input_probability = get_rr_node_net_probability(rr_node[inode]);
      input_init_value = get_rr_node_net_init_value(rr_node[inode]);
      fprintf(fp, "***** Signal chanx[%d][%d]_midout[%d] density = %g, probability=%g.*****\n",
              x, y, itrack, input_density, input_probability);
      fprintf(fp, "Vchanx[%d][%d]_midout[%d] chanx[%d][%d]_midout[%d] 0 \n", 
              x, y, itrack, x, y, itrack);
      fprint_voltage_pulse_params(fp, input_init_value, input_density, input_probability);
      /* Update statistics */
      average_cb_input_density += input_density;
      if (0. < input_density) {
        avg_density_cnt++;
      }
      break;
    case CHANY:
      inode = get_rr_node_index(x, y, CHANY, itrack, LL_rr_node_indices);
      /* Add input voltage pulses*/
      input_density = get_rr_node_net_density(rr_node[inode]);
      input_probability = get_rr_node_net_probability(rr_node[inode]);
      input_init_value = get_rr_node_net_init_value(rr_node[inode]);
      fprintf(fp, "***** Signal chany[%d][%d]_midout[%d] density = %g, probability=%g.*****\n",
              x, y, itrack, input_density, input_probability);
      fprintf(fp, "Vchany[%d][%d]_midout[%d] chany[%d][%d]_midout[%d] 0 \n", 
              x, y, itrack, x, y, itrack);
      fprint_voltage_pulse_params(fp, input_init_value, input_density, input_probability);
      /* Update statistics */
      average_cb_input_density += input_density;
      if (0. < input_density) {
        avg_density_cnt++;
      }
      break;
    default: 
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
      exit(1);
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "***** Global Force for all SRAMs *****\n");
  if (SPICE_SRAM_SCAN_CHAIN == sram_spice_orgz_type) {
    fprintf(fp, "Vsc_clk sc_clk 0 0\n");
    fprintf(fp, "Vsc_rst sc_rst 0 0\n");
    fprintf(fp, "Vsc_set sc_set 0 0\n");
    switch(chan_type) { 
    case CHANX:
      /* Connect to VDD supply */
      fprintf(fp, "***** Voltage supplies *****\n");
      fprintf(fp, "Vgvdd_cbx[%d][%d] gvdd_cbx[%d][%d] 0 vsp\n", x, y, x, y);
      fprintf(fp, "Vcbx[%d][%d]_sc_head cbx[%d][%d]_sc_head 0 0\n", x, y, x, y);
      fprintf(fp, ".nodeset V(cbx[%d][%d]_sc_head) 0\n", x, y);
      break;
    case CHANY:
      /* Connect to VDD supply */
      fprintf(fp, "***** Voltage supplies *****\n");
      fprintf(fp, "Vgvdd_cby[%d][%d] gvdd_cby[%d][%d] 0 vsp\n", x, y, x, y);
      fprintf(fp, "Vcby[%d][%d]_sc_head cby[%d][%d]_sc_head 0 0\n", x, y, x, y);
      fprintf(fp, ".nodeset V(cby[%d][%d]_sc_head) 0\n", x, y);
      break;
    default: 
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
      exit(1);
    }
  } else {
    fprintf(fp, "V%s->in %s->in 0 0\n", 
            sram_spice_model->prefix, sram_spice_model->prefix);
    fprintf(fp, ".nodeset V(%s->in) 0\n", sram_spice_model->prefix);
  }

  /* Calculate the num_sim_clock_cycle for this MUX, update global max_sim_clock_cycle in this testbench */
  if (0 < avg_density_cnt) {
    average_cb_input_density = average_cb_input_density/avg_density_cnt;
    num_sim_clock_cycles = (int)(1/average_cb_input_density) + 1;
    used = 1;
  } else {
    assert(0 == avg_density_cnt);
    average_cb_input_density = 0.;
    num_sim_clock_cycles = 2;
    used = 0;
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

  /* Measurements */
  /* Measure the delay of MUX */
  fprintf(fp, "***** Measurements *****\n");
  /* Measure the leakage power of MUX */
  fprintf(fp, "***** Leakage Power Measurement *****\n");
  fprintf(fp, ".meas tran cb[%d][%d]_leakage_power avg p(Vgvdd_cb[%d][%d]) from=0 to='clock_period'\n",
          x, y, x, y);
  /* Measure the dynamic power of MUX */
  fprintf(fp, "***** Dynamic Power Measurement *****\n");
  fprintf(fp, ".meas tran cb[%d][%d]_dynamic_power avg p(Vgvdd_cb[%d][%d]) from='clock_period' to='%d*clock_period'\n",
          x, y, x, y, num_sim_clock_cycles);
  fprintf(fp, ".meas tran cb[%d][%d]_energy_per_cycle param='cb[%d][%d]_dynamic_power*clock_period'\n",
          x, y, x, y);

  /* print average sb input density */
  switch(chan_type) { 
  case CHANX:
    vpr_printf(TIO_MESSAGE_INFO,"Average density of CBX[%d][%d] inputs is %.2g.\n", x, y, average_cb_input_density);
    break;
  case CHANY:
    vpr_printf(TIO_MESSAGE_INFO,"Average density of CBY[%d][%d] inputs is %.2g.\n", x, y, average_cb_input_density);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }

  return used;
}

/** In a testbench, we call the subckt of a defined switch block (sb[x][y])
 *  For each input of switch block, we find their activities and generate input voltage pulses.
 *  For each output of switch block, we add all the non-inverter downstream components as load.
 */
static 
int fprint_spice_routing_testbench_call_one_sb_tb(FILE* fp, 
                                                  int x, int y, 
                                                  t_ivec*** LL_rr_node_indices) {
  int itrack, inode, side, ix, iy;
  int* chan_width = (int*)my_malloc(sizeof(int)*4); /* 4 sides */
  int used = 0;

  char* outport_name = NULL;
  char* rr_node_outport_name = NULL;

  float input_density;
  float input_probability;
  int input_init_value;
  float average_sb_input_density = 0.;
  int avg_density_cnt = 0;

  int num_sim_clock_cycles = 0;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 

  /* call the defined switch block sb[x][y]*/
  fprintf(fp, "***** Call defined Switch Box[%d][%d] *****\n", x, y);
  fprint_call_defined_one_switch_box(fp, sb_info[x][y]);

  /* For each input of switch block, we generate a input voltage pulse
   * For each output of switch block, we generate downstream loads 
   */
  /* Find all rr_nodes of channels */
  for (side = 0; side < 4; side++) {
    switch (side) {
    case 0:
      /* For the bording, we should take special care */
      if (y == ny) {
        chan_width[side] = 0;
        break;
      }
      /* Routing channels*/
      ix = x; 
      iy = y + 1;
      /* Channel width */
      chan_width[side] = chan_width_y[ix];
      /* Malloc */
      outport_name = (char*)my_malloc(sizeof(char)*(6 + strlen(my_itoa(ix)) + 2 
                                      + strlen(my_itoa(iy)) + 6 
                                      + strlen(my_itoa(chan_width[side])) + 2));
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      /* Collect rr_nodes for Tracks for top: chany[x][y+1] */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANY, itrack, LL_rr_node_indices);
        /* Get direction of each rr_node and determine it is an input or output :*/
        switch (rr_node[inode].direction) {
        case INC_DIRECTION: /* Output */
          /* Add downstream components as loads*/
          sprintf(outport_name, "chany[%d][%d]_in[%d]", ix, iy, itrack);
          rr_node_outport_name = fprint_spice_testbench_rr_node_load_version(fp, &testbench_load_cnt,
                                                                             num_segments, 
                                                                             segments, 
                                                                             0, 
                                                                             rr_node[inode], 
                                                                             outport_name); 
          /* Free */
          my_free(rr_node_outport_name);
          break;
        case DEC_DIRECTION: /* Input */
          /* Add input voltage pulses*/
          input_density = get_rr_node_net_density(rr_node[inode]);
          input_probability = get_rr_node_net_probability(rr_node[inode]);
          input_init_value = get_rr_node_net_init_value(rr_node[inode]);
          fprintf(fp, "***** Signal chany[%d][%d]_out[%d] density = %g, probability=%g.*****\n",
                  ix, iy, itrack, input_density, input_probability);
          fprintf(fp, "Vchany[%d][%d]_out[%d] chany[%d][%d]_out[%d] 0 \n", 
                  ix, iy, itrack, ix, iy, itrack);
          fprint_voltage_pulse_params(fp, input_init_value, input_density, input_probability);
          /* Update statistics */
          average_sb_input_density += input_density;
          if (0. < input_density) {
           avg_density_cnt++;
          }
          break;
        default: /* Error Out */
          vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of chany[%d][%d]_track[%d]!\n",
                      __FILE__, __LINE__, ix, iy, itrack);
          exit(1);
        }
      }
      /* Free */
      my_free(outport_name);
      break;
    case 1:
      /* For the bording, we should take speical care */
      if (x == nx) {
        chan_width[side] = 0;
        break;
      }
      /* Routing channels*/
      ix = x + 1; 
      iy = y;
      /* Channel width */
      chan_width[side] = chan_width_x[iy];
      /* Malloc */
      outport_name = (char*)my_malloc(sizeof(char)*(6 + strlen(my_itoa(ix)) + 2 
                                      + strlen(my_itoa(iy)) + 6
                                      + strlen(my_itoa(chan_width[side])) + 2));
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANX, itrack, LL_rr_node_indices);
        /* Get direction of each rr_node and determine it is an input or output :*/
        switch (rr_node[inode].direction) {
        case INC_DIRECTION: /* Output */
          /* Add downstream components as loads*/
          sprintf(outport_name, "chanx[%d][%d]_in[%d]", ix, iy, itrack);
          rr_node_outport_name = fprint_spice_testbench_rr_node_load_version(fp, &testbench_load_cnt,
                                                                             num_segments, 
                                                                             segments, 
                                                                             0, 
                                                                             rr_node[inode], 
                                                                             outport_name); 
          /* Free */
          my_free(rr_node_outport_name);
          break;
        case DEC_DIRECTION: /* Input */
          /* Add input voltage pulses*/
          input_density = get_rr_node_net_density(rr_node[inode]);
          input_probability = get_rr_node_net_probability(rr_node[inode]);
          input_init_value = get_rr_node_net_init_value(rr_node[inode]);
          fprintf(fp, "***** Signal chanx[%d][%d]_out[%d] density = %g, probability=%g.*****\n",
                  ix, iy, itrack, input_density, input_probability);
          fprintf(fp, "Vchanx[%d][%d]_out[%d] chanx[%d][%d]_out[%d] 0 \n", 
                  ix, iy, itrack, ix, iy, itrack);
          fprint_voltage_pulse_params(fp, input_init_value, input_density, input_probability);
          /* Update statistics */
          average_sb_input_density += input_density;
          if (0. < input_density) {
           avg_density_cnt++;
          }
          break;
        default: /* Error Out */
          vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of chanx[%d][%d]_track[%d]!\n",
                      __FILE__, __LINE__, ix, iy, itrack);
          exit(1);
        }
      }
      /* Free */
      my_free(outport_name);
      break;
    case 2:
      /* For the bording, we should take speical care */
      if (y == 0) {
        chan_width[side] = 0;
        break;
      }
      /* Routing channels*/
      ix = x; 
      iy = y;
      /* Channel width */
      chan_width[side] = chan_width_y[ix];
      /* Malloc */
      outport_name = (char*)my_malloc(sizeof(char)*(6 + strlen(my_itoa(ix)) + 2 
                                      + strlen(my_itoa(iy)) + 6
                                      + strlen(my_itoa(chan_width[side])) + 2));
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANY, itrack, LL_rr_node_indices);
        /* Get direction of each rr_node and determine it is an input or output :*/
        switch (rr_node[inode].direction) {
        case INC_DIRECTION: /* Input */
          /* Add input voltage pulses*/
          input_density = get_rr_node_net_density(rr_node[inode]);
          input_probability = get_rr_node_net_probability(rr_node[inode]);
          input_init_value = get_rr_node_net_init_value(rr_node[inode]);
          fprintf(fp, "***** Signal chany[%d][%d]_in[%d] density = %g, probability=%g.*****\n",
                  ix, iy, itrack, input_density, input_probability);
          fprintf(fp, "Vchany[%d][%d]_in[%d] chany[%d][%d]_in[%d] 0 \n", 
                  ix, iy, itrack, ix, iy, itrack);
          fprint_voltage_pulse_params(fp, input_init_value, input_density, input_probability);
          /* Update statistics */
          average_sb_input_density += input_density;
          if (0. < input_density) {
           avg_density_cnt++;
          }
          break;
        case DEC_DIRECTION: /* Output */
          /* Add downstream components as loads*/
          sprintf(outport_name, "chany[%d][%d]_out[%d]", ix, iy, itrack);
          rr_node_outport_name = fprint_spice_testbench_rr_node_load_version(fp, &testbench_load_cnt,
                                                                             num_segments, 
                                                                             segments, 
                                                                             0, 
                                                                             rr_node[inode], 
                                                                             outport_name); 
          /* Free */
          my_free(rr_node_outport_name);
          break;
        default: /* Error Out */
          vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of chany[%d][%d]_track[%d]!\n",
                      __FILE__, __LINE__, ix, iy, itrack);
          exit(1);
        }
      }
      /* Free */
      my_free(outport_name);
      /* Print MUXes of BOTTOM side */
      break;
    case 3:
      /* For the bording, we should take speical care */
      if (x == 0) {
        chan_width[side] = 0;
        break;
      }
      /* Routing channels*/
      ix = x; 
      iy = y;
      /* Channel width */
      chan_width[side] = chan_width_x[iy];
      /* Malloc */
      outport_name = (char*)my_malloc(sizeof(char)*(6 + strlen(my_itoa(ix)) + 2 
                                      + strlen(my_itoa(iy)) + 6 
                                      + strlen(my_itoa(chan_width[side])) + 2));
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANX, itrack, LL_rr_node_indices);
        /* Get direction of each rr_node and determine it is an input or output :*/
        switch (rr_node[inode].direction) {
        case INC_DIRECTION: /* Input */
          /* Add input voltage pulses*/
          input_density = get_rr_node_net_density(rr_node[inode]);
          input_probability = get_rr_node_net_probability(rr_node[inode]);
          input_init_value = get_rr_node_net_init_value(rr_node[inode]);
          fprintf(fp, "***** Signal chanx[%d][%d]_in[%d] density = %g, probability=%g.*****\n",
                  ix, iy, itrack, input_density, input_probability);
          fprintf(fp, "Vchanx[%d][%d]_in[%d] chanx[%d][%d]_in[%d] 0 \n", 
                  ix, iy, itrack, ix, iy, itrack);
          fprint_voltage_pulse_params(fp, input_init_value, input_density, input_probability);
          /* Update statistics */
          average_sb_input_density += input_density;
          if (0. < input_density) {
           avg_density_cnt++;
          }
          break;
        case DEC_DIRECTION: /* Output */
          /* Add downstream components as loads*/
          sprintf(outport_name, "chanx[%d][%d]_out[%d]", ix, iy, itrack);
          rr_node_outport_name = fprint_spice_testbench_rr_node_load_version(fp, &testbench_load_cnt,
                                                                             num_segments, 
                                                                             segments, 
                                                                             0, 
                                                                             rr_node[inode], 
                                                                             outport_name); 
          /* Free */
          my_free(rr_node_outport_name);
          break;
        default: /* Error Out */
          vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of chanx[%d][%d]_track[%d]!\n",
                      __FILE__, __LINE__, ix, iy, itrack);
          exit(1);
        }
      }
      /* Free */
      my_free(outport_name);
      /* Print MUXes of LEFT side */
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid side index!\n", __FILE__, __LINE__);
      exit(1);
    }
  }

  /* Add input voltage pulses for CLB OPINs (input of a switch block) */
  /* Considering the border */
  if (ny != y) {
    /* 5. Grid[x][y+1] Right side outputs pins */
    avg_density_cnt += 
      fprint_grid_side_pins_voltage_pulses(fp, OPIN, x, y+1, 1, 
                                           LL_rr_node_indices, &average_sb_input_density);
  }
  if (0 != x) {
    /* 6. Grid[x][y+1] Bottom side outputs pins */
    avg_density_cnt += 
      fprint_grid_side_pins_voltage_pulses(fp, OPIN, x, y+1, 2,
                                           LL_rr_node_indices, &average_sb_input_density);
  }

  if (ny != y) {
    /* 7. Grid[x+1][y+1] Left side output pins */
    avg_density_cnt += 
      fprint_grid_side_pins_voltage_pulses(fp, OPIN, x+1, y+1, 3,
                                           LL_rr_node_indices, &average_sb_input_density);
  }
  if (nx != x) {
    /* 8. Grid[x+1][y+1] Bottom side output pins */
    avg_density_cnt += 
      fprint_grid_side_pins_voltage_pulses(fp, OPIN, x+1, y+1, 2,
                                           LL_rr_node_indices, &average_sb_input_density);
  }

  if (nx != x) {
    /* 9. Grid[x+1][y] Top side output pins */
    avg_density_cnt += 
      fprint_grid_side_pins_voltage_pulses(fp, OPIN, x+1, y, 0,
                                           LL_rr_node_indices, &average_sb_input_density);
  }
  if (0 != y) {
    /* 10. Grid[x+1][y] Left side output pins */
    avg_density_cnt += 
      fprint_grid_side_pins_voltage_pulses(fp, OPIN, x+1, y, 3,
                                           LL_rr_node_indices, &average_sb_input_density);
  }

  if (0 != y) {
    /* 11. Grid[x][y] Right side output pins */
    avg_density_cnt += 
      fprint_grid_side_pins_voltage_pulses(fp, OPIN, x, y, 1,
                                           LL_rr_node_indices, &average_sb_input_density);
  } 
  if (0 != x) {
    /* 12. Grid[x][y] Top side output pins */
    avg_density_cnt += 
      fprint_grid_side_pins_voltage_pulses(fp, OPIN, x, y, 0,
                                           LL_rr_node_indices, &average_sb_input_density);
  }

  /* Connect to VDD supply */
  fprintf(fp, "***** Voltage supplies *****\n");
  fprintf(fp, "Vgvdd_sb[%d][%d] gvdd_sb[%d][%d] 0 vsp\n", x, y, x, y);

  fprintf(fp, "***** Global Force for all SRAMs *****\n");
  if (SPICE_SRAM_SCAN_CHAIN == sram_spice_orgz_type) {
    fprintf(fp, "Vsc_clk sc_clk 0 0\n");
    fprintf(fp, "Vsc_rst sc_rst 0 0\n");
    fprintf(fp, "Vsc_set sc_set 0 0\n");
    fprintf(fp, "Vsb[%d][%d]_sc_head sb[%d][%d]_sc_head 0 0\n", 
            x, y, x, y);
    fprintf(fp, ".nodeset V(%s[0]->in) 0\n", sram_spice_model->prefix);
  } else {
    fprintf(fp, "V%s->in %s->in 0 0\n", 
            sram_spice_model->prefix, sram_spice_model->prefix);
    fprintf(fp, ".nodeset V(%s->in) 0\n", sram_spice_model->prefix);
  }

  /* Calculate the num_sim_clock_cycle for this MUX, update global max_sim_clock_cycle in this testbench */
  if (0 < avg_density_cnt) {
    average_sb_input_density = average_sb_input_density/avg_density_cnt;
    num_sim_clock_cycles = (int)(1/average_sb_input_density) + 1;
    used = 1;
  } else {
    assert(0 == avg_density_cnt);
    average_sb_input_density = 0.;
    num_sim_clock_cycles = 2;
    used = 0;
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

  /* Measurements */
  /* Measure the delay of MUX */
  fprintf(fp, "***** Measurements *****\n");
  /* Measure the leakage power of MUX */
  fprintf(fp, "***** Leakage Power Measurement *****\n");
  fprintf(fp, ".meas tran sb[%d][%d]_leakage_power avg p(Vgvdd_sb[%d][%d]) from=0 to='clock_period'\n",
          x, y, x, y);
  /* Measure the dynamic power of MUX */
  fprintf(fp, "***** Dynamic Power Measurement *****\n");
  fprintf(fp, ".meas tran sb[%d][%d]_dynamic_power avg p(Vgvdd_sb[%d][%d]) from='clock_period' to='%d*clock_period'\n",
          x, y, x, y, num_sim_clock_cycles);
  fprintf(fp, ".meas tran sb[%d][%d]_energy_per_cycle param='sb[%d][%d]_dynamic_power*clock_period'\n",
          x, y, x, y);

  /* print average sb input density */
  vpr_printf(TIO_MESSAGE_INFO,"Average density of SB[%d][%d] inputs is %.2g.\n", x, y, average_sb_input_density);

  /* Free */
  my_free(chan_width);
 
  return used;
}

int fprint_spice_one_cb_testbench(char* formatted_spice_dir,
                                  char* circuit_name,
                                  char* cb_testbench_name, 
                                  char* include_dir_path,
                                  char* subckt_dir_path,
                                  t_ivec*** LL_rr_node_indices,
                                  int num_clocks,
                                  t_arch arch,
                                  int grid_x, int grid_y, t_rr_type cb_type,
                                  boolean leakage_only) {
  FILE* fp = NULL;
  char* formatted_subckt_dir_path = format_dir_path(subckt_dir_path);
  char* title = my_strcat("FPGA SPICE Connection Box Testbench Bench for Design: ", circuit_name);
  char* cb_testbench_file_path = my_strcat(formatted_spice_dir, cb_testbench_name);
  char* cb_tb_name = NULL;
  int used = 0;
  char* temp_include_file_path = NULL;

  /* one cbx, one cby*/
  switch (cb_type) {
  case CHANX:
    cb_tb_name = "Connection Box X-channel ";
    break;
  case CHANY:
    cb_tb_name = "Connection Box Y-channel ";
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d]) Invalid connection_box_type!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Check if the path exists*/
  fp = fopen(cb_testbench_file_path,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create SPICE %s Test bench netlist %s!\n", 
               __FILE__, __LINE__, cb_tb_name, cb_testbench_file_path); 
    exit(1);
  }

  /* Load global vars in this source file */
  num_segments = arch.num_segments;
  segments = arch.Segments;
  testbench_load_cnt = 0;

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

  /* Print simulation temperature and other options for SPICE */
  fprint_spice_options(fp, arch.spice->spice_params);

  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprint_spice_routing_testbench_global_ports(fp, *(arch.spice));
 
  /* Quote defined Logic blocks subckts (Grids) */
  init_spice_routing_testbench_globals(*(arch.spice));

  fprintf(fp, "****** Include subckt netlists: Routing structures (Switch Boxes, Channels, Connection Boxes) *****\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, routing_spice_file_name);
  fprintf(fp, ".include \'%s\'\n", temp_include_file_path);
  my_free(temp_include_file_path);

  /* Generate SPICE routing testbench generic stimuli*/
  fprintf_spice_routing_testbench_generic_stimuli(fp, num_clocks);

  /* one cbx, one cby*/
  switch (cb_type) {
  case CHANX:
  case CHANY:
    used = fprint_spice_routing_testbench_call_one_cb_tb(fp, cb_type, grid_x, grid_y, LL_rr_node_indices);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d]) Invalid connection_box_type!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Push the testbench to the linked list */
  tb_head = add_one_spice_tb_info_to_llist(tb_head, cb_testbench_file_path, 
                                           max_sim_num_clock_cycles);
  used = 1;

  return used;
}

int fprint_spice_one_sb_testbench(char* formatted_spice_dir,
                                  char* circuit_name,
                                  char* sb_testbench_name, 
                                  char* include_dir_path,
                                  char* subckt_dir_path,
                                  t_ivec*** LL_rr_node_indices,
                                  int num_clocks,
                                  t_arch arch,
                                  int grid_x, int grid_y, 
                                  boolean leakage_only) {
  FILE* fp = NULL;
  char* formatted_subckt_dir_path = format_dir_path(subckt_dir_path);
  char* title = my_strcat("FPGA SPICE Switch Block Testbench Bench for Design: ", circuit_name);
  char* sb_testbench_file_path = my_strcat(formatted_spice_dir, sb_testbench_name);
  char* sb_tb_name = NULL;
  int used = 0;
  char* temp_include_file_path = NULL;

  sb_tb_name = "Switch Block ";

  /* Check if the path exists*/
  fp = fopen(sb_testbench_file_path,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create SPICE %s Test bench netlist %s!\n", 
               __FILE__, __LINE__, sb_tb_name, sb_testbench_file_path); 
    exit(1);
  }

  /* Load global vars in this source file */
  num_segments = arch.num_segments;
  segments = arch.Segments;
  testbench_load_cnt = 0;

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

  /* Print simulation temperature and other options for SPICE */
  fprint_spice_options(fp, arch.spice->spice_params);

  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprint_spice_routing_testbench_global_ports(fp, *(arch.spice));
 
  /* Quote defined Logic blocks subckts (Grids) */
  init_spice_routing_testbench_globals(*(arch.spice));

  fprintf(fp, "****** Include subckt netlists: Routing structures (Switch Boxes, Channels, Connection Boxes) *****\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, routing_spice_file_name);
  fprintf(fp, ".include \'%s\'\n", temp_include_file_path);
  my_free(temp_include_file_path);
  used = fprint_spice_routing_testbench_call_one_sb_tb(fp, grid_x, grid_y, LL_rr_node_indices);

  /* Push the testbench to the linked list */
  tb_head = add_one_spice_tb_info_to_llist(tb_head, sb_testbench_file_path, 
                                           max_sim_num_clock_cycles);
  used = 1;


  return used;
}

/* Top function: Generate testbenches for all Connection Boxes */
void fprint_spice_cb_testbench(char* formatted_spice_dir,
                               char* circuit_name,
                               char* include_dir_path,
                               char* subckt_dir_path,
                               t_ivec*** LL_rr_node_indices,
                               int num_clocks,
                               t_arch arch,
                               boolean leakage_only) {
  char* cb_testbench_name = NULL; 
  int ix, iy;
  int cnt = 0;
  int used = 0;

  for (iy = 0; iy < (ny+1); iy++) {
    for (ix = 1; ix < (nx+1); ix++) {
      cb_testbench_name = (char*)my_malloc(sizeof(char)*( strlen(circuit_name) 
                                            + 4 + strlen(my_itoa(ix)) + 2 + strlen(my_itoa(iy)) + 1
                                            + strlen(spice_cb_testbench_postfix)  + 1 ));
      sprintf(cb_testbench_name, "%s_cb%d_%d%s",
              circuit_name, ix, iy, spice_cb_testbench_postfix);
      used = fprint_spice_one_cb_testbench(formatted_spice_dir, circuit_name, cb_testbench_name, 
                                            include_dir_path, subckt_dir_path, LL_rr_node_indices,
                                            num_clocks, arch, ix, iy, CHANX, 
                                            leakage_only);
      if (1 == used) {
        cnt += used;
      }
      /* free */
      my_free(cb_testbench_name);
    }  
  } 
  for (ix = 0; ix < (nx+1); ix++) {
    for (iy = 1; iy < (ny+1); iy++) {
      cb_testbench_name = (char*)my_malloc(sizeof(char)*( strlen(circuit_name) 
                                            + 4 + strlen(my_itoa(ix)) + 2 + strlen(my_itoa(iy)) + 1
                                            + strlen(spice_cb_testbench_postfix)  + 1 ));
      sprintf(cb_testbench_name, "%s_cb%d_%d%s",
               circuit_name, ix, iy, spice_cb_testbench_postfix);
      used = fprint_spice_one_cb_testbench(formatted_spice_dir, circuit_name, cb_testbench_name, 
                                            include_dir_path, subckt_dir_path, LL_rr_node_indices,
                                            num_clocks, arch, ix, iy, CHANY,  
                                            leakage_only);
      if (1 == used) {
        cnt += used;
      }
      /* free */
      my_free(cb_testbench_name);
    }  
  } 
  /* Update the global counter */
  num_used_cb_tb = cnt;
  vpr_printf(TIO_MESSAGE_INFO,"No. of generated CB testbench = %d\n", num_used_cb_tb);

  
  return;
}

/* Top function: Generate testbenches for all Switch Blocks */
void fprint_spice_sb_testbench(char* formatted_spice_dir,
                               char* circuit_name,
                               char* include_dir_path,
                               char* subckt_dir_path,
                               t_ivec*** LL_rr_node_indices,
                               int num_clocks,
                               t_arch arch,
                               boolean leakage_only) {

  char* sb_testbench_name = NULL; 
  int ix, iy;
  int cnt = 0;
  int used = 0;

  for (ix = 0; ix < (nx+1); ix++) {
    for (iy = 0; iy < (ny+1); iy++) {
      sb_testbench_name = (char*)my_malloc(sizeof(char)*( strlen(circuit_name) 
                                            + 4 + strlen(my_itoa(ix)) + 2 + strlen(my_itoa(iy)) + 1
                                            + strlen(spice_sb_testbench_postfix)  + 1 ));
      sprintf(sb_testbench_name, "%s_sb%d_%d%s",
              circuit_name, ix, iy, spice_sb_testbench_postfix);
      used = fprint_spice_one_sb_testbench(formatted_spice_dir, circuit_name, sb_testbench_name, 
                                            include_dir_path, subckt_dir_path, LL_rr_node_indices,
                                            num_clocks, arch, ix, iy, 
                                            leakage_only);
      if (1 == used) {
        cnt += used;
      }
      /* free */
      my_free(sb_testbench_name);
    }  
  } 
  /* Update the global counter */
  num_used_sb_tb = cnt;
  vpr_printf(TIO_MESSAGE_INFO,"No. of generated SB testbench = %d\n", num_used_sb_tb);

  return;
}
