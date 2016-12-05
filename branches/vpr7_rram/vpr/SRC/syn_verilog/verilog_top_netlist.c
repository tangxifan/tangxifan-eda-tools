/***********************************/
/*  Dump Synthesizable Veriolog    */
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

/* Include spice support headers*/
#include "read_xml_spice_util.h"
#include "linkedlist.h"
#include "fpga_spice_utils.h"
#include "spice_netlist_utils.h"
#include "fpga_spice_backannotate_utils.h"
#include "fpga_spice_globals.h"

/* Include verilog support headers*/
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_routing.h"
#include "verilog_pbtypes.h"
#include "verilog_bitstream.h"
#include "verilog_top_netlist.h"


/* Global varaiable only accessible in this source file*/
static int prog_clock_cycle_delay_unit = 2;
static int op_clock_cycle_delay_unit = 2;
static char* top_netlist_addr_bl_port_name = "addr_bl";
static char* top_netlist_addr_wl_port_name = "addr_wl";
static char* top_netlist_bl_port_name = "bl_bus";
static char* top_netlist_wl_port_name = "wl_bus";

/* Local Subroutines declaration */

/******** Subroutines ***********/
static 
void dump_verilog_top_netlist_ports(FILE* fp,
                                    int num_clocks,
                                    char* circuit_name,
                                    t_spice verilog) {
  int num_bl, num_wl, num_reserved_bl, num_reserved_wl;
  int bl_decoder_size, wl_decoder_size, num_mem_bit;
  t_spice_model* mem_model = NULL;

  /* A valid file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  fprintf(fp, "//----- Top-level Verilog Module -----\n");
  fprintf(fp, "module %s_top (\n", circuit_name);
  fprintf(fp, "\n");
  /* dump global ports */
  if (0 < dump_verilog_global_ports(fp, global_ports_head, TRUE)) {
    fprintf(fp, ",\n");
  }
  /* Inputs and outputs of I/O pads */
  /* Input Pads */
  assert(NULL != inpad_verilog_model);
  if ((NULL == inpad_verilog_model)
    &&(inpad_verilog_model->cnt > 0)) {
    fprintf(fp, " input wire [%d:0] %s%s, //---FPGA inputs \n", 
            inpad_verilog_model->cnt - 1,
            gio_input_prefix,
            inpad_verilog_model->prefix);
  }

  /* Output Pads */
  assert(NULL != outpad_verilog_model);
  if ((NULL == outpad_verilog_model)
   ||(outpad_verilog_model->cnt > 0)) {
    fprintf(fp, " output wire [%d:0] %s%s, //---- FPGA outputs \n", 
            outpad_verilog_model->cnt - 1,
            gio_output_prefix,
            outpad_verilog_model->prefix);
  }

  /* Inout Pads */
  assert(NULL != iopad_verilog_model);
  if ((NULL == iopad_verilog_model)
   ||(iopad_verilog_model->cnt > 0)) {
    fprintf(fp, " inout wire [%d:0] %s%s, //---- FPGA inouts \n", 
            iopad_verilog_model->cnt - 1,
            gio_inout_prefix,
            iopad_verilog_model->prefix);
  }

  /* Configuration ports depend on the organization of SRAMs */
  switch(sram_verilog_orgz_info->type) {
  case SPICE_SRAM_STANDALONE:
    fprintf(fp, "  input wire [%d:0] %s //---- SRAM outputs \n", 
            sram_verilog_model->cnt - 1,
            sram_verilog_model->prefix); 
    fprintf(fp, ");\n");
    /* Definition ends */
    break;
  case SPICE_SRAM_SCAN_CHAIN:
    /* TODO: currently, I use the same number of inputs as SRAM_STANDALONE
     * It will be changed to the number of scan-chain heads we want 
				*/
    fprintf(fp, "  input wire [%d:0] %s //---- Scan-chain outputs \n", 
            sram_verilog_model->cnt - 1,
            sram_verilog_model->prefix); 
    fprintf(fp, ");\n");
    /* Definition ends */
    break;
  case SPICE_SRAM_MEMORY_BANK:
    num_mem_bit = get_sram_orgz_info_num_mem_bit(sram_verilog_orgz_info);
    get_sram_orgz_info_num_blwl(sram_verilog_orgz_info, &num_bl, &num_wl);
    get_sram_orgz_info_reserved_blwl(sram_verilog_orgz_info, &num_reserved_bl, &num_reserved_wl);
    bl_decoder_size = determine_decoder_size(num_bl);
    wl_decoder_size = determine_decoder_size(num_wl);
    fprintf(fp, "  input en_bl, en_wl,\n");
    fprintf(fp, "  input wire [%d:0] %s, //--- Address of bit lines \n", 
                   bl_decoder_size - 1, top_netlist_addr_bl_port_name);
    fprintf(fp, "  input wire [%d:0] %s //--- Address of word lines \n", 
                   wl_decoder_size - 1, top_netlist_addr_wl_port_name);
    fprintf(fp, ");\n");
    /* Definition ends */
    /* I add all the Bit lines and Word lines here just for testbench usage */
    fprintf(fp, "  wire [%d:0] %s; //--- Bit lines bus \n", 
                   num_bl - 1, top_netlist_bl_port_name);
    fprintf(fp, "  wire [%d:0] %s; //--- Word lines bus \n", 
                   num_wl - 1, top_netlist_wl_port_name);
    fprintf(fp, "\n");
    /* Declare reserved and normal conf_bits ports  */
    mem_model = sram_verilog_orgz_info->mem_bank_info->mem_model;
    fprintf(fp, "  wire [%d:0] %s_%s; //---- Reserved Bit lines \n",
            num_reserved_bl - 1, mem_model->prefix, "reserved_bl");
    fprintf(fp, "  wire [%d:0] %s_%s; //---- Reserved Word lines \n",
            num_reserved_wl - 1, mem_model->prefix, "reserved_wl");
    fprintf(fp, "  wire [%d:%d] %s_%s; //---- Normal Bit lines \n",
            num_bl - 1, num_reserved_bl, mem_model->prefix, "bl");
    fprintf(fp, "  wire [%d:%d] %s_%s; //---- Normal Word lines \n",
            num_wl - 1, num_reserved_wl, mem_model->prefix, "wl");
    /* Connect reserved conf_bits and normal conf_bits to the bus */
    fprintf(fp, "  assign %s[%d:0] = %s_%s[%d:0];\n",
            top_netlist_bl_port_name, num_reserved_bl - 1,
            mem_model->prefix, "reserved_bl", num_reserved_bl - 1);
    fprintf(fp, "  assign %s[%d:0] = %s_%s[%d:0];\n",
            top_netlist_wl_port_name, num_reserved_wl - 1,
            mem_model->prefix, "reserved_wl", num_reserved_wl - 1);
    fprintf(fp, "  assign %s[%d:%d] = %s_%s[%d:%d];\n",
            top_netlist_bl_port_name, num_mem_bit - 1, num_reserved_bl,
            mem_model->prefix, "bl", num_mem_bit - 1, num_reserved_bl);
    fprintf(fp, "  assign %s[%d:%d] = %s_%s[%d:%d];\n",
            top_netlist_wl_port_name, num_mem_bit - 1, num_reserved_wl,
            mem_model->prefix, "wl", num_mem_bit - 1, num_reserved_wl);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  return; 
}

static 
void dump_verilog_defined_one_grid(FILE* fp,
                                   int ix, int iy) {
  /* Comment lines */
  fprintf(fp, "//----- BEGIN Call Grid[%d][%d] module -----\n", ix, iy);
  /* Print the Grid module */
  fprintf(fp, "grid_%d__%d_  ", ix, iy); /* Call the name of subckt */ 
  fprintf(fp, "grid_%d__%d__0_ ", ix, iy);
  fprintf(fp, "(");
  fprintf(fp, "\n");
  /* dump global ports */
  if (0 < dump_verilog_global_ports(fp, global_ports_head, FALSE)) {
    fprintf(fp, ",\n");
  }

  if (IO_TYPE == grid[ix][iy].type) {
    dump_verilog_io_grid_pins(fp, ix, iy, 1, FALSE, FALSE);
  } else {
    dump_verilog_grid_pins(fp, ix, iy, 1, FALSE, FALSE);
  }
  /* Print Input Pad and Output Pad */
  dump_verilog_grid_common_port(fp, inpad_verilog_model, gio_input_prefix,
                                inpad_verilog_model->grid_index_low[ix][iy],
                                inpad_verilog_model->grid_index_high[ix][iy] - 1,
                                FALSE); 
  dump_verilog_grid_common_port(fp, outpad_verilog_model, gio_output_prefix, 
                                outpad_verilog_model->grid_index_low[ix][iy],
                                outpad_verilog_model->grid_index_high[ix][iy] - 1,
                                FALSE); 
  /* IO PAD */
  dump_verilog_grid_common_port(fp, iopad_verilog_model, gio_inout_prefix, 
                                iopad_verilog_model->grid_index_low[ix][iy],
                                iopad_verilog_model->grid_index_high[ix][iy] - 1,
                                FALSE); 

  /* Print configuration ports */
  /* Reserved configuration ports */
  if (0 < sram_verilog_orgz_info->grid_reserved_conf_bits[ix][iy]) {
    fprintf(fp, ",\n");
  }
  dump_verilog_reserved_sram_ports(fp, sram_verilog_orgz_info,
                                   0, 
                                   sram_verilog_orgz_info->grid_reserved_conf_bits[ix][iy] - 1,
                                   FALSE);
  /* Normal configuration ports */
  if (0 < (sram_verilog_orgz_info->grid_conf_bits_msb[ix][iy]
           - sram_verilog_orgz_info->grid_conf_bits_lsb[ix][iy])) {
    fprintf(fp, ",\n");
  }
  dump_verilog_sram_ports(fp, sram_verilog_orgz_info,
                          sram_verilog_orgz_info->grid_conf_bits_lsb[ix][iy],
                          sram_verilog_orgz_info->grid_conf_bits_msb[ix][iy] - 1,
                          FALSE);
  fprintf(fp, ");\n");
  /* Comment lines */
  fprintf(fp, "//----- END call Grid[%d][%d] module -----\n\n", ix, iy);

  return;
}


/***** Print (call) the defined grids *****/
void dump_verilog_defined_grids(FILE* fp) {
  int ix, iy;

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Normal Grids */
  for (ix = 1; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      assert(IO_TYPE != grid[ix][iy].type);
      dump_verilog_defined_one_grid(fp, ix, iy);
    }
  } 

  /* IO Grids */
  /* LEFT side */
  ix = 0;
  for (iy = 1; iy < (ny + 1); iy++) {
    assert(IO_TYPE == grid[ix][iy].type);
    dump_verilog_defined_one_grid(fp, ix, iy);
  }

  /* RIGHT side */
  ix = nx + 1;
  for (iy = 1; iy < (ny + 1); iy++) {
    assert(IO_TYPE == grid[ix][iy].type);
    dump_verilog_defined_one_grid(fp, ix, iy);
  }

  /* BOTTOM side */
  iy = 0;
  for (ix = 1; ix < (nx + 1); ix++) {
    assert(IO_TYPE == grid[ix][iy].type);
    dump_verilog_defined_one_grid(fp, ix, iy);
  } 

  /* TOP side */
  iy = ny + 1;
  for (ix = 1; ix < (nx + 1); ix++) {
    assert(IO_TYPE == grid[ix][iy].type);
    dump_verilog_defined_one_grid(fp, ix, iy);
  } 

  return;
}

/* Call defined channels. 
 * Ensure the port name here is co-herent to other sub-circuits(SB,CB,grid)!!!
 */
void dump_verilog_defined_one_channel(FILE* fp,
                                      t_rr_type chan_type, int x, int y,
                                      int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                      t_ivec*** LL_rr_node_indices) {
  int itrack;
  int chan_width = 0;
  t_rr_node** chan_rr_nodes = NULL;

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Check */
  assert((CHANX == chan_type)||(CHANY == chan_type));
  /* check x*/
  assert((!(0 > x))&&(x < (nx + 1))); 
  /* check y*/
  assert((!(0 > y))&&(y < (ny + 1))); 

  /* Collect rr_nodes for Tracks for chanx[ix][iy] */
  chan_rr_nodes = get_chan_rr_nodes(&chan_width, chan_type, x, y,
                                    LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);
 
  /* Comment lines */
  switch (chan_type) {
  case CHANX:
    fprintf(fp, "//----- BEGIN Call Channel-X [%d][%d] module -----\n", x, y);
    break;
  case CHANY:
    fprintf(fp, "//----- BEGIN call Channel-Y [%d][%d] module -----\n\n", x, y);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid Channel Type!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Call the define sub-circuit */
  fprintf(fp, "%s_%d__%d_ ", 
          convert_chan_type_to_string(chan_type),
          x, y);
  fprintf(fp, "%s_%d__%d__0_ ", 
          convert_chan_type_to_string(chan_type),
          x, y);
  fprintf(fp, "(");
  fprintf(fp, "\n");
  /* dump global ports */
  if (0 < dump_verilog_global_ports(fp, global_ports_head, FALSE)) {
    fprintf(fp, ",\n");
  }

  /* LEFT/BOTTOM side port of CHANX/CHANY */
  /* We apply an opposite port naming rule than function: fprint_routing_chan_subckt 
   * In top-level netlists, we follow the same port name as switch blocks and connection blocks 
   * When a track is in INC_DIRECTION, the LEFT/BOTTOM port would be an output of a switch block
   * When a track is in DEC_DIRECTION, the LEFT/BOTTOM port would be an input of a switch block
   */
  for (itrack = 0; itrack < chan_width; itrack++) {
    switch (chan_rr_nodes[itrack]->direction) {
    case INC_DIRECTION:
      fprintf(fp, "%s_%d__%d__out_%d_, ", 
              convert_chan_type_to_string(chan_type),
              x, y, itrack);
      fprintf(fp, "\n");
      break;
    case DEC_DIRECTION:
      fprintf(fp, "%s_%d__%d__in_%d_, ", 
              convert_chan_type_to_string(chan_type),
              x, y, itrack);
      fprintf(fp, "\n");
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of %s[%d][%d]_track[%d]!\n",
                 __FILE__, __LINE__,
                 convert_chan_type_to_string(chan_type),
                 x, y, itrack);
      exit(1);
    }
  }
  /* RIGHT/TOP side port of CHANX/CHANY */
  /* We apply an opposite port naming rule than function: fprint_routing_chan_subckt 
   * In top-level netlists, we follow the same port name as switch blocks and connection blocks 
   * When a track is in INC_DIRECTION, the RIGHT/TOP port would be an input of a switch block
   * When a track is in DEC_DIRECTION, the RIGHT/TOP port would be an output of a switch block
   */
  for (itrack = 0; itrack < chan_width; itrack++) {
    switch (chan_rr_nodes[itrack]->direction) {
    case INC_DIRECTION:
      fprintf(fp, "%s_%d__%d__in_%d_, ", 
              convert_chan_type_to_string(chan_type),
              x, y, itrack);
      fprintf(fp, "\n");
      break;
    case DEC_DIRECTION:
      fprintf(fp, "%s_%d__%d__out_%d_, ", 
              convert_chan_type_to_string(chan_type),
              x, y, itrack);
      fprintf(fp, "\n");
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of %s[%d][%d]_track[%d]!\n",
                 __FILE__, __LINE__,
                 convert_chan_type_to_string(chan_type),
                 x, y, itrack);
      exit(1);
    }
  }

  /* output at middle point */
  for (itrack = 0; itrack < chan_width; itrack++) {
    fprintf(fp, "%s_%d__%d__midout_%d_ ", 
            convert_chan_type_to_string(chan_type),
            x, y, itrack);
    if (itrack < chan_width - 1) {
      fprintf(fp, ",");
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, ");\n");

  /* Comment lines */
  switch (chan_type) {
  case CHANX:
    fprintf(fp, "//----- END Call Channel-X [%d][%d] module -----\n", x, y);
    break;
  case CHANY:
    fprintf(fp, "//----- END call Channel-Y [%d][%d] module -----\n\n", x, y);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid Channel Type!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Free */
  my_free(chan_rr_nodes);

  return;
}

/* Call the sub-circuits for channels : Channel X and Channel Y*/
void dump_verilog_defined_channels(FILE* fp,
                                   int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                   t_ivec*** LL_rr_node_indices) {
  int ix, iy;

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Channel X */
  for (iy = 0; iy < (ny + 1); iy++) {
    for (ix = 1; ix < (nx + 1); ix++) {
      dump_verilog_defined_one_channel(fp, CHANX, ix, iy,
                                       LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);
    }
  }

  /* Channel Y */
  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      dump_verilog_defined_one_channel(fp, CHANY, ix, iy,
                                       LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);
    }
  }

  return;
}

/* Call the defined sub-circuit of connection box
 * TODO: actually most of this function is copied from
 * spice_routing.c : dump_verilog_conneciton_box_interc
 * Should be more clever to use the original function
 */
void dump_verilog_defined_one_connection_box(FILE* fp,
                                             t_cb cur_cb_info) {
  int itrack, inode, side, x, y;
  int side_cnt = 0;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > cur_cb_info.x))&&(!(cur_cb_info.x > (nx + 1)))); 
  assert((!(0 > cur_cb_info.y))&&(!(cur_cb_info.y > (ny + 1)))); 

  x = cur_cb_info.x;
  y = cur_cb_info.y;
  
  /* Print the definition of subckt*/
  /* Identify the type of connection box */
  switch(cur_cb_info.type) {
  case CHANX:
    /* Comment lines */
    fprintf(fp, "//----- BEGIN Call Connection Box-X direction [%d][%d] module -----\n", x, y);
    /* Print module */
    fprintf(fp, "cbx_%d__%d_ ", x, y);
    fprintf(fp, "cbx_%d__%d__0_ ", x, y);
    break;
  case CHANY:
    /* Comment lines */
    fprintf(fp, "//----- BEGIN Call Connection Box-Y direction [%d][%d] module -----\n", x, y);
    /* Print module */
    fprintf(fp, "cby_%d__%d_ ", x, y);
    fprintf(fp, "cby_%d__%d__0_ ", x, y);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }
 
  fprintf(fp, "(");
  fprintf(fp, "\n");
  /* dump global ports */
  if (0 < dump_verilog_global_ports(fp, global_ports_head, FALSE)) {
    fprintf(fp, ",\n");
  }

  /* Print the ports of channels*/
  /* connect to the mid point of a track*/
  side_cnt = 0;
  for (side = 0; side < cur_cb_info.num_sides; side++) {
    /* Bypass side with zero channel width */
    if (0 == cur_cb_info.chan_width[side]) {
      continue;
    }
    assert (0 < cur_cb_info.chan_width[side]);
    side_cnt++;
    fprintf(fp, "//----- %s side inputs: channel track middle outputs -----\n", convert_side_index_to_string(side));
    for (itrack = 0; itrack < cur_cb_info.chan_width[side]; itrack++) {
      fprintf(fp, "%s_%d__%d__midout_%d_, ",
              convert_chan_type_to_string(cur_cb_info.type),
              cur_cb_info.x, cur_cb_info.y, itrack);
      fprintf(fp, "\n");
    }
  }
  /*check side_cnt */
  assert(1 == side_cnt);
 
  side_cnt = 0;
  /* Print the ports of grids*/
  for (side = 0; side < cur_cb_info.num_sides; side++) {
    /* Bypass side with zero IPINs*/
    if (0 == cur_cb_info.num_ipin_rr_nodes[side]) {
      continue;
    }
    side_cnt++;
    assert(0 < cur_cb_info.num_ipin_rr_nodes[side]);
    assert(NULL != cur_cb_info.ipin_rr_node[side]);
    fprintf(fp, "//----- %s side outputs: CLB input pins -----\n", convert_side_index_to_string(side));
    for (inode = 0; inode < cur_cb_info.num_ipin_rr_nodes[side]; inode++) {
      /* Print each INPUT Pins of a grid */
      dump_verilog_grid_side_pin_with_given_index(fp, OPIN,
                                                 cur_cb_info.ipin_rr_node[side][inode]->ptc_num,
                                                 cur_cb_info.ipin_rr_node_grid_side[side][inode],
                                                 cur_cb_info.ipin_rr_node[side][inode]->xlow,
                                                 cur_cb_info.ipin_rr_node[side][inode]->ylow, 
                                                 FALSE); /* Do not specify direction of port */
      fprintf(fp, ", \n");
    }
  }
  /* Make sure only 2 sides of IPINs are printed */
  assert(2 == side_cnt);
 
  /* Configuration ports */
  /* Reserved sram ports */
  if (0 < cur_cb_info.num_reserved_conf_bits) {
  }
  dump_verilog_reserved_sram_ports(fp, sram_verilog_orgz_info, 
                                   0, cur_cb_info.num_reserved_conf_bits - 1,
                                   FALSE);
  /* Normal sram ports */
  if (0 < (cur_cb_info.conf_bits_msb - cur_cb_info.conf_bits_lsb)) {
    fprintf(fp, ",\n");
  }
  dump_verilog_sram_ports(fp, sram_verilog_orgz_info, 
                          cur_cb_info.conf_bits_lsb, cur_cb_info.conf_bits_msb - 1,
                          FALSE);
 
  fprintf(fp, ");\n");

  /* Comment lines */
  switch(cur_cb_info.type) {
  case CHANX:
    fprintf(fp, "//----- END call Connection Box-X direction [%d][%d] module -----\n\n", x, y);
    break;
  case CHANY:
    fprintf(fp, "//----- END call Connection Box-Y direction [%d][%d] module -----\n\n", x, y);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Check */
  assert(2 == side_cnt);
 
  return;
}

/* Call the sub-circuits for connection boxes */
void dump_verilog_defined_connection_boxes(FILE* fp) {
  int ix, iy;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* X - channels [1...nx][0..ny]*/
  for (iy = 0; iy < (ny + 1); iy++) {
    for (ix = 1; ix < (nx + 1); ix++) {
      dump_verilog_defined_one_connection_box(fp, cbx_info[ix][iy]);
    }
  }
  /* Y - channels [1...ny][0..nx]*/
  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      dump_verilog_defined_one_connection_box(fp, cby_info[ix][iy]);
    }
  }
 
  return; 
}

/* Call the defined switch box sub-circuit
 * TODO: This function is also copied from
 * spice_routing.c : dump_verilog_routing_switch_box_subckt
 */
void dump_verilog_defined_one_switch_box(FILE* fp,
                                         t_sb cur_sb_info) {
  int ix, iy, side, itrack, x, y, inode;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > cur_sb_info.x))&&(!(cur_sb_info.x > (nx + 1)))); 
  assert((!(0 > cur_sb_info.y))&&(!(cur_sb_info.y > (ny + 1)))); 

  x = cur_sb_info.x;
  y = cur_sb_info.y;
                 
  /* Comment lines */                 
  fprintf(fp, "//----- BEGIN call module Switch blocks [%d][%d] -----\n", 
          cur_sb_info.x, cur_sb_info.y);
  /* Print module*/
  fprintf(fp, "sb_%d__%d_ ", cur_sb_info.x, cur_sb_info.y);
  fprintf(fp, "sb_%d__%d__0_ ", cur_sb_info.x, cur_sb_info.y);
  fprintf(fp, "(");

  fprintf(fp, "\n");
  /* dump global ports */
  if (0 < dump_verilog_global_ports(fp, global_ports_head, FALSE)) {
    fprintf(fp, ",\n");
  }

  for (side = 0; side < cur_sb_info.num_sides; side++) {
    if (0 == side) {
      /* 1. Channel Y [x][y+1] inputs */
      ix = x;
      iy = y + 1;
    } else if (1 == side) {
      /* 2. Channel X [x+1][y] inputs */
      ix = x + 1;
      iy = y;
    } else if (2 == side) {
      /* 3. Channel Y [x][y] inputs */
      ix = x;
      iy = y;
    } else if (3 == side) {
      /* 4. Channel X [x][y] inputs */
      ix = x;
      iy = y;
    }
    fprintf(fp, "//----- %s side channel ports-----\n", convert_side_index_to_string(side));
    for (itrack = 0; itrack < cur_sb_info.chan_width[side]; itrack++) {
      switch (cur_sb_info.chan_rr_node_direction[side][itrack]) {
      case OUT_PORT:
        fprintf(fp, "%s_%d__%d__out_%d_, ", 
                convert_chan_type_to_string(cur_sb_info.chan_rr_node[side][itrack]->type), 
                ix, iy, itrack); 
        break;
      case IN_PORT:
        fprintf(fp, "%s_%d__%d__in_%d_, ",
                convert_chan_type_to_string(cur_sb_info.chan_rr_node[side][itrack]->type), 
                ix, iy, itrack); 
        break;
      default:
        vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of sb[%d][%d] side[%d] track[%d]!\n",
                   __FILE__, __LINE__, x, y, side, itrack);
        exit(1);
      }
    }
    fprintf(fp, "\n");
    fprintf(fp, "//----- %s side inputs: CLB output pins -----\n", convert_side_index_to_string(side));
    /* Dump OPINs of adjacent CLBs */
    for (inode = 0; inode < cur_sb_info.num_opin_rr_nodes[side]; inode++) {
      dump_verilog_grid_side_pin_with_given_index(fp, IPIN,
                                                  cur_sb_info.opin_rr_node[side][inode]->ptc_num,
                                                  cur_sb_info.opin_rr_node_grid_side[side][inode],
                                                  cur_sb_info.opin_rr_node[side][inode]->xlow,
                                                  cur_sb_info.opin_rr_node[side][inode]->ylow,
                                                  FALSE); /* Do not specify the direction of port */ 
      fprintf(fp, ",");
    } 
    fprintf(fp, "\n");
  }

  /* Configuration ports */
  /* output of each configuration bit */
  /* Reserved sram ports */
  if (0 < cur_sb_info.num_reserved_conf_bits) {
  }
  dump_verilog_reserved_sram_ports(fp, sram_verilog_orgz_info, 
                                   0, cur_sb_info.num_reserved_conf_bits - 1,
                                   FALSE);
  /* Normal sram ports */
  if (0 < (cur_sb_info.conf_bits_msb - cur_sb_info.conf_bits_lsb)) {
    fprintf(fp, ",\n");
  }
  dump_verilog_sram_ports(fp, sram_verilog_orgz_info, 
                          cur_sb_info.conf_bits_lsb, 
                          cur_sb_info.conf_bits_msb - 1,
                          FALSE);

  fprintf(fp, ");\n");

  /* Comment lines */                 
  fprintf(fp, "//----- END call module Switch blocks [%d][%d] -----\n\n", x, y);

  /* Free */

  return;
}

void dump_verilog_defined_switch_boxes(FILE* fp) {
  int ix, iy;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 0; iy < (ny + 1); iy++) {
      dump_verilog_defined_one_switch_box(fp, sb_info[ix][iy]);
    }
  }

  return;
}

/** Dump Standalone SRAMs 
 */
void dump_verilog_configuration_circuits_standalone_srams(FILE* fp) {
  int i;
  /* Check */
  assert(SPICE_SRAM_STANDALONE == sram_verilog_orgz_type);
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File handler!",__FILE__, __LINE__); 
    exit(1);
  } 

  /* Dump each SRAM */
  fprintf(fp, "//------ Standalone SRAMs -----\n");
  for (i = 0; i < sram_verilog_model->cnt; i++) {
    fprintf(fp, "%s %s_%d (\n", 
            sram_verilog_model->name, sram_verilog_model->prefix, i);
    /* Input and 2 outputs */
    fprintf(fp, "%s_in[%d] %s_out[%d] %s_outb[%d] ",
            sram_verilog_model->prefix, i,
            sram_verilog_model->prefix, i,
            sram_verilog_model->prefix, i);
    fprintf(fp, ");\n");
  }
  fprintf(fp, "//------ END Standalone SRAMs -----\n");

  return;
}

/** Dump scan-chains 
 */
void dump_verilog_configuration_circuits_scan_chains(FILE* fp) {
  int i;
  /* Check */
  assert(SPICE_SRAM_SCAN_CHAIN == sram_verilog_orgz_type);
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!",__FILE__, __LINE__); 
    exit(1);
  } 

  /* Dump each Scan-chain FF */
  fprintf(fp, "//------ Scan-chain FFs -----\n");
  for (i = 0; i < sram_verilog_model->cnt; i++) {
    fprintf(fp, "%s %s_%d (\n", 
            sram_verilog_model->name, sram_verilog_model->prefix, i);
    /* Port sequence: <D> <Q> <Qb> <CLK> <RESET> <SET> */
    fprintf(fp, "%s_in[%d], %s_out[%d], %s_outb[%d], ",
            sram_verilog_model->prefix, i,
            sram_verilog_model->prefix, i,
            sram_verilog_model->prefix, i);
    fprintf(fp, "prog_clk, prog_rst, prog_set ");
    fprintf(fp, ");\n");
    /* Connect the head of current scff to the tail of previous scff*/
    if (0 < i) {
      fprintf(fp, "assign %s_in[%d] <= %s_out[%d];\n",
              sram_verilog_model->prefix, i,
              sram_verilog_model->prefix, i - 1);
    }
  }
  fprintf(fp, "//------ END Scan-chain FFs -----\n");

  return;
}

/* Dump a memory bank to configure all the Bit lines and Word lines */
void dump_verilog_configuration_circuits_memory_bank(FILE* fp, 
                                                     t_sram_orgz_info* cur_sram_orgz_info) {
  int bl_decoder_size, wl_decoder_size;
  int num_bl, num_wl;

  /* Check */
  assert(SPICE_SRAM_MEMORY_BANK == cur_sram_orgz_info->type);
  assert(NULL != cur_sram_orgz_info->mem_bank_info);

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!",__FILE__, __LINE__); 
    exit(1);
  } 

  get_sram_orgz_info_num_blwl(cur_sram_orgz_info, &num_bl, &num_wl);
  bl_decoder_size = determine_decoder_size(num_bl);
  wl_decoder_size = determine_decoder_size(num_wl);

  /* Comment lines */
  fprintf(fp, "//----- BEGIN call decoders for memory bank controller -----\n");

  /* Dump Decoders for Bit lines and Word lines */
  /* Two huge decoders
   * TODO: divide to a number of small decoders ?
   */
  /* Bit lines decoder */
  fprintf(fp, "bl_decoder%dto%d mem_bank_bl_decoder (",
          bl_decoder_size, num_bl);
  /* Prefix of BL & WL is fixed, in order to simplify grouping nets */
  fprintf(fp, "en_bl, %s[%d:0], %s[%d:0]", 
          top_netlist_addr_bl_port_name, bl_decoder_size - 1, 
          top_netlist_bl_port_name, num_bl - 1);
  fprintf(fp, ");\n");

  /* Word lines decoder */
  fprintf(fp, "wl_decoder%dto%d mem_bank_wl_decoder (",
          wl_decoder_size, num_wl);
  fprintf(fp, "en_wl, %s[%d:0], %s[%d:0] ",
          top_netlist_addr_wl_port_name, wl_decoder_size - 1, 
          top_netlist_wl_port_name, num_wl - 1);
  fprintf(fp, ");\n");

  /* Comment lines */
  fprintf(fp, "//----- END call decoders for memory bank controller -----\n\n");

  return; 
}

/* Dump the configuration circuits in verilog according to user-specification
 * Supported styles of configuration circuits:
 * 1. Scan-chains
 * 2. Memory banks
 * 3. Standalone SRAMs
 */
void dump_verilog_configuration_circuits(FILE* fp) {
  switch(sram_verilog_orgz_info->type) {
  case SPICE_SRAM_STANDALONE:
    dump_verilog_configuration_circuits_standalone_srams(fp);
    break;
  case SPICE_SRAM_SCAN_CHAIN:
    dump_verilog_configuration_circuits_scan_chains(fp);
    break;
  case SPICE_SRAM_MEMORY_BANK:
    dump_verilog_configuration_circuits_memory_bank(fp, sram_verilog_orgz_info);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }
  return;
}

/* Dump all the global ports that are stored in the linked list */
static 
void dump_verilog_top_testbench_global_ports(FILE* fp, t_llist* head) {
  t_llist* temp = head;
  t_spice_model_port* cur_global_port = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
  }

  fprintf(fp, "//----- BEGIN Global ports -----\n");
  while(NULL != temp) {
    cur_global_port = (t_spice_model_port*)(temp->dptr); 
    fprintf(fp, "reg wire [0:%d] %s", 
            cur_global_port->size - 1, 
            cur_global_port->prefix); 

    /* if this is the tail, we do not dump a comma */
    if (NULL != temp->next) {
     fprintf(fp, ",\n");
    }
    /* Go to the next */
    temp = temp->next;
  }
  fprintf(fp, "//----- END Global ports -----\n");

  return;
}

static 
void dump_verilog_top_testbench_ports(FILE* fp,
                                      char* circuit_name) {
  int decoder_size = determine_decoder_size(sram_verilog_model->cnt);
  int iblock, iopad_idx;

  fprintf(fp, "module %s_top_tb;\n", circuit_name);
  /* Local wires */
  /* 1. reset, set, clock signals */
  /* 2. iopad signals */

  /* Connect to defined signals */
  /* set and reset signals */
  fprintf(fp, "\n");
  dump_verilog_top_testbench_global_ports(fp, global_ports_head);
  fprintf(fp, ",\n");

  /* TODO: dump each global signal as reg here */

  /* Inputs and outputs of I/O pads */
  /* Input Pads */
  assert(NULL != inpad_verilog_model);
  if ((NULL == inpad_verilog_model)
    &&(inpad_verilog_model->cnt > 0)) {
    fprintf(fp, " reg wire [%d:0] %s%s; //---FPGA inputs \n", 
            inpad_verilog_model->cnt - 1,
            gio_input_prefix,
            inpad_verilog_model->prefix);
  }

  /* Output Pads */
  assert(NULL != outpad_verilog_model);
  if ((NULL == outpad_verilog_model)
   ||(outpad_verilog_model->cnt > 0)) {
    fprintf(fp, " wire [%d:0] %s%s; //---- FPGA outputs \n", 
            outpad_verilog_model->cnt - 1,
            gio_output_prefix,
            outpad_verilog_model->prefix);
  }

  /* Inout Pads */
  assert(NULL != iopad_verilog_model);
  if ((NULL == iopad_verilog_model)
   ||(iopad_verilog_model->cnt > 0)) {
    fprintf(fp, " reg [%d:0] %s%s; //---- FPGA inouts \n", 
            iopad_verilog_model->cnt - 1,
            gio_inout_prefix,
            iopad_verilog_model->prefix);
  }

  /* Configuration ports depend on the organization of SRAMs */
  switch(sram_verilog_orgz_type) {
  case SPICE_SRAM_STANDALONE:
    fprintf(fp, " wire [%d:0] %s; //---- SRAM outputs \n", 
            sram_verilog_model->cnt - 1,
            sram_verilog_model->prefix); 
    break;
  case SPICE_SRAM_SCAN_CHAIN:
    /* TODO: currently, I use the same number of inputs as SRAM_STANDALONE
     * It will be changed to the number of scan-chain heads we want 
				*/
    fprintf(fp, "  wire [%d:0] %s; //---- Scan-chain outputs \n", 
            sram_verilog_model->cnt - 1,
            sram_verilog_model->prefix); 
    break;
  case SPICE_SRAM_MEMORY_BANK:
    fprintf(fp, "  wire en_bl, en_wl;\n");
    fprintf(fp, "  reg [%d:0] addr_bl; //--- Address of bit lines \n", 
                   decoder_size - 1);
    fprintf(fp, "  reg wire [%d:0] addr_wl; //--- Address of word lines \n", 
                   decoder_size - 1);
    /* I add all the Bit lines and Word lines here just for testbench usage */
    fprintf(fp, "  input wire [%d:0] %s_out; //--- Bit lines \n", 
                   sram_verilog_model->cnt - 1, sram_verilog_model->prefix);
    fprintf(fp, "  input wire [%d:0] %s_outb; //--- Word lines \n", 
                   sram_verilog_model->cnt - 1, sram_verilog_model->prefix);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Add a signal to identify the configuration phase is finished */
  fprintf(fp, "reg config_done;\n");
  /* Programming clock */
  fprintf(fp, "reg prog_clock;\n");

  /* Add signals from blif benchmark and short-wire them to FPGA I/O PADs
   * This brings convenience to checking functionality  
   */
  fprintf(fp, "//-----Link Blif Benchmark inputs to FPGA IOPADs -----\n");
  for (iblock = 0; iblock < num_logical_blocks; iblock++) {
    /* General INOUT*/
    if (iopad_verilog_model == logical_block[iblock].mapped_spice_model) {
      iopad_idx = logical_block[iblock].mapped_spice_model_index;
      /* Make sure We find the correct logical block !*/
      assert((VPACK_INPAD == logical_block[iblock].type)||(VPACK_OUTPAD == logical_block[iblock].type));
      fprintf(fp, "//----- Blif Benchmark inout %s is mapped to FPGA IOPAD %s[%d] -----\n", 
              logical_block[iblock].name, gio_inout_prefix, iopad_idx);
      fprintf(fp, "wire %s_%s[%d];\n",
              logical_block[iblock].name, gio_inout_prefix, iopad_idx);
      fprintf(fp, "assign %s_%s[%d] = %s[%d];\n",
              logical_block[iblock].name, gio_inout_prefix, iopad_idx,
              gio_inout_prefix, iopad_idx);
    }
    /* General INPUT*/
    if (inpad_verilog_model == logical_block[iblock].mapped_spice_model) {
      iopad_idx = logical_block[iblock].mapped_spice_model_index;
      /* Make sure We find the correct logical block !*/
      assert(VPACK_INPAD == logical_block[iblock].type);
      fprintf(fp, "//----- Blif Benchmark input %s is mapped to FPGA IOPAD %s[%d] -----\n", 
              logical_block[iblock].name, gio_input_prefix, iopad_idx);
      fprintf(fp, "wire %s_%s[%d];\n",
              logical_block[iblock].name, gio_input_prefix, iopad_idx);
      fprintf(fp, "assign %s_%s[%d] = %s[%d];\n",
              logical_block[iblock].name, gio_input_prefix, iopad_idx,
              gio_input_prefix, iopad_idx);
    }
    /* General INPUT*/
    if (outpad_verilog_model == logical_block[iblock].mapped_spice_model) {
      iopad_idx = logical_block[iblock].mapped_spice_model_index;
      /* Make sure We find the correct logical block !*/
      assert(VPACK_OUTPAD == logical_block[iblock].type);
      fprintf(fp, "//----- Blif Benchmark output %s is mapped to FPGA IOPAD %s[%d] -----\n", 
              logical_block[iblock].name, gio_output_prefix, iopad_idx);
      fprintf(fp, "wire %s_%s[%d];\n",
              logical_block[iblock].name, gio_output_prefix, iopad_idx);
      fprintf(fp, "assign %s_%s[%d] = %s[%d];\n",
              logical_block[iblock].name, gio_output_prefix, iopad_idx,
              gio_output_prefix, iopad_idx);
    }
  }

  return;
}

static 
void dump_verilog_top_testbench_call_top_module(FILE* fp,
                                                char* circuit_name) {
  int decoder_size = determine_decoder_size(sram_verilog_model->cnt);

  /* Include defined top-level module */
  fprintf(fp, "//----- Device Under Test (DUT) ----\n");
  fprintf(fp, "//------Call defined Top-level Verilog Module -----\n");
  fprintf(fp, "%s_top U0 (\n", circuit_name);

  /* Connect to defined signals */
  fprintf(fp, "\n");
  /* dump global ports */
  if (0 < dump_verilog_global_ports(fp, global_ports_head, FALSE)) {
    fprintf(fp, ",\n");
  }
  /* Inputs and outputs of I/O pads */
  /* Input Pads */
  assert(NULL != inpad_verilog_model);
  if ((NULL == inpad_verilog_model)
    &&(inpad_verilog_model->cnt > 0)) {
    fprintf(fp, " %s%s[%d:0], //---FPGA inputs \n", 
            gio_input_prefix,
            inpad_verilog_model->prefix,
            inpad_verilog_model->cnt - 1);
  }

  /* Output Pads */
  assert(NULL != outpad_verilog_model);
  if ((NULL == outpad_verilog_model)
   ||(outpad_verilog_model->cnt > 0)) {
    fprintf(fp, " %s%s[%d:0], //---- FPGA outputs \n", 
            gio_output_prefix,
            outpad_verilog_model->prefix,
            outpad_verilog_model->cnt - 1);
  }

  /* Inout Pads */
  assert(NULL != iopad_verilog_model);
  if ((NULL == iopad_verilog_model)
   ||(iopad_verilog_model->cnt > 0)) {
    fprintf(fp, " %s%s[%d:0], //---- FPGA inouts \n", 
            gio_inout_prefix,
            iopad_verilog_model->prefix,
            iopad_verilog_model->cnt - 1);
  }

  /* Configuration ports depend on the organization of SRAMs */
  switch(sram_verilog_orgz_type) {
  case SPICE_SRAM_STANDALONE:
    fprintf(fp, "  %s[%d:0], //---- SRAM outputs \n", 
            sram_verilog_model->prefix, 
            sram_verilog_model->cnt - 1);
    break;
  case SPICE_SRAM_SCAN_CHAIN:
    /* TODO: currently, I use the same number of inputs as SRAM_STANDALONE
     * It will be changed to the number of scan-chain heads we want 
				*/
    fprintf(fp, "  %s[%d:0]; //---- Scan-chain outputs \n", 
            sram_verilog_model->prefix, 
            sram_verilog_model->cnt - 1);
    break;
  case SPICE_SRAM_MEMORY_BANK:
    fprintf(fp, "  en_bl, en_wl,\n");
    fprintf(fp, "  addr_bl[%d:0], //--- Address of bit lines \n", 
                   decoder_size - 1);
    fprintf(fp, "  addr_wl[%d:0] //--- Address of word lines \n", 
                   decoder_size - 1);
    /* I add all the Bit lines and Word lines here just for testbench usage */
    fprintf(fp, "  %s_out[%d:0], //--- Bit lines \n", 
                   sram_verilog_model->prefix,
                   sram_verilog_model->cnt - 1); 
    fprintf(fp, "  %s_outb[%d:0], //--- Word lines \n", 
                   sram_verilog_model->prefix,
                   sram_verilog_model->cnt - 1); 
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  fprintf(fp, ");\n");

}

/* Find the number of configuration clock cycles for a top-level testbench
 * A valid configuration clock cycle is allocated for an element with 
 * (1) SRAM_val=1;
 * (2) BL = 1 && WL = 1;
 * (3) BL = 1 && WL = 0 with a paired conf_bit;
 */
int dump_verilog_top_testbench_find_num_config_clock_cycles(t_llist* head) {
  int cnt = 0;
  t_llist* temp = head; 
  t_conf_bit_info* temp_conf_bit_info = NULL; 

  while (NULL != temp) {
    /* Fetch the conf_bit_info */
    temp_conf_bit_info = (t_conf_bit_info*)(temp->dptr);
    /* Check if conf_bit_info needs a clock cycle*/
    switch (sram_verilog_orgz_type) {
    case SPICE_SRAM_STANDALONE:
    case SPICE_SRAM_SCAN_CHAIN:
      if (1 == temp_conf_bit_info->sram_bit->val) {
        cnt++;
        temp_conf_bit_info->index_in_top_tb = cnt;
      }
      break;
    case SPICE_SRAM_MEMORY_BANK:
      if ((1 == temp_conf_bit_info->bl->val)
         &&(1 == temp_conf_bit_info->wl->val)) {
        cnt++;
        temp_conf_bit_info->index_in_top_tb = cnt;
      } else if ((1 == temp_conf_bit_info->bl->val)
               &&(0 == temp_conf_bit_info->wl->val)
               &&(NULL != temp_conf_bit_info->pair_conf_bit)) {
        cnt++;
        temp_conf_bit_info->index_in_top_tb = cnt;
        temp_conf_bit_info->pair_conf_bit->index_in_top_tb = cnt;
      }
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
                 __FILE__, __LINE__);
      exit(1);
    }
    /* Go to the next */
    temp = temp->next;
  }
  
  return cnt;
}

/* Recursively dump configuration bits which are stored in the linked list 
 * We start dump configuration bit from the tail of the linked list
 * until the head of the linked list
 */
static 
void dump_verilog_top_testbench_conf_bits_parallel(FILE* fp, 
                                                   t_llist* head) {
  t_llist* temp = head; 
  t_conf_bit_info* cur_conf_bit_info = (t_conf_bit_info*)(temp->dptr);
  int bl_index = 0;
  int wl_index = 0;

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!",__FILE__, __LINE__); 
    exit(1);
  } 
						
  /* We alraedy touch the tail, start dump */
  switch (sram_verilog_orgz_type) {
  case SPICE_SRAM_STANDALONE:
  case SPICE_SRAM_SCAN_CHAIN:
    /* For scan chain, the last bit should go first!*/
    while (NULL != temp) {
      cur_conf_bit_info = (t_conf_bit_info*)(temp->dptr);
      /* Scan-chain only loads the SRAM values 
      */
      fprintf(fp, "Initial\n");
      fprintf(fp, "  begin: CONF_BIT_%d\n", cur_conf_bit_info->index);
      assert((0 == cur_conf_bit_info->sram_bit->val)||(1 == cur_conf_bit_info->sram_bit->val));
      fprintf(fp, "    %s[%d] = %d, ", 
              sram_verilog_model->prefix, cur_conf_bit_info->index, 
              cur_conf_bit_info->sram_bit->val),
      fprintf(fp, "  end\n");
      fprintf(fp, "//---- Configuration bit No.: %d, ", cur_conf_bit_info->index);
      fprintf(fp, " SRAM value: %d, ", cur_conf_bit_info->sram_bit->val);
      fprintf(fp, " SPICE model name: %s, ", cur_conf_bit_info->parent_spice_model->name);
      fprintf(fp, " SPICE model index: %d ", cur_conf_bit_info->parent_spice_model_index);
      fprintf(fp, "\n");
      /* This is not the tail, keep going */
      temp = temp->next;
    }
    break;
  case SPICE_SRAM_MEMORY_BANK:
    /* For memory bank, we do not care the sequence.
     * To be easy to understand, we go from the first to the last
     */
    while (NULL != temp) {
      cur_conf_bit_info = (t_conf_bit_info*)(temp->dptr);
      /* Memory bank requires the address to be given to the decoder*/
      /* If this WL is selected , we decode its index to address */
      bl_index = cur_conf_bit_info->index;
      wl_index = cur_conf_bit_info->index;
      assert((0 == cur_conf_bit_info->bl->val)||(1 == cur_conf_bit_info->bl->val));
      assert((0 == cur_conf_bit_info->wl->val)||(1 == cur_conf_bit_info->wl->val));
      /* The BL and WL will be keep its value since the first cycle */
      /* If this WL is selected, directly force its value for its signal*/
      fprintf(fp, "Initial\n");
      fprintf(fp, "  begin: BL_%d\n", bl_index);
      fprintf(fp, "    %s_out[%d] = 1'b%d;",
              sram_verilog_model->prefix, bl_index, cur_conf_bit_info->bl->val);
      /*
      fprintf(fp, "  input wire [%d:0] %s_out, //--- Bit lines \n", 
                     sram_verilog_model->cnt - 1, sram_verilog_model->prefix);
      fprintf(fp, "  input wire [%d:0] %s_outb //--- Word lines \n", 
                     sram_verilog_model->cnt - 1, sram_verilog_model->prefix);
      */
      /* Enable ADDR BL */
      fprintf(fp, "  end\n");
      fprintf(fp, "// Configuration bit No.: %d, ", cur_conf_bit_info->index);
      fprintf(fp, " Bit Line: %d, ", cur_conf_bit_info->bl->val);
      fprintf(fp, " SPICE model name: %s, ", cur_conf_bit_info->parent_spice_model->name);
      fprintf(fp, " SPICE model index: %d ", cur_conf_bit_info->parent_spice_model_index);
      fprintf(fp, "\n");
      /* Word line address */
      fprintf(fp, "Initial\n");
      fprintf(fp, "  begin: WL[%d]\n", wl_index);
      fprintf(fp, "    %s_outb[%d] = 1'b%d;", 
              sram_verilog_model->prefix, wl_index, cur_conf_bit_info->wl->val);
      fprintf(fp, "  end\n");
      fprintf(fp, "// Configuration bit No.: %d, ", cur_conf_bit_info->index);
      fprintf(fp, " Word Line: %d, ", cur_conf_bit_info->wl->val);
      fprintf(fp, " SPICE model name: %s, ", cur_conf_bit_info->parent_spice_model->name);
      fprintf(fp, " SPICE model index: %d ", cur_conf_bit_info->parent_spice_model_index);
      fprintf(fp, "\n");
      /* This is not the tail, keep going */
      temp = temp->next;
    }
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  return;
}

/* Recursively dump configuration bits which are stored in the linked list 
 * We start dump configuration bit from the tail of the linked list
 * until the head of the linked list
 */
static 
void rec_dump_verilog_top_testbench_conf_bits_serial(FILE* fp, 
                                                     t_llist* cur_conf_bit) {
  t_llist* temp = cur_conf_bit->next; 
  t_conf_bit_info* cur_conf_bit_info = (t_conf_bit_info*)(cur_conf_bit->dptr);
  int decoder_size = determine_decoder_size(sram_verilog_model->cnt);
  int bl_index = 0;
  int wl_index = 0;
  char* wl_addr = NULL;
  char* bl_addr = NULL; 
  int cur_prog_cycle = 0;
  int dump_cur_conf_bit = 0;

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!",__FILE__, __LINE__); 
    exit(1);
  } 
						
  /* We alraedy touch the tail, start dump */
  switch (sram_verilog_orgz_type) {
  case SPICE_SRAM_STANDALONE:
  case SPICE_SRAM_SCAN_CHAIN:
    /* TODO: Scan-chain only loads the SRAM values 
    fprintf(fp, "Initial\n");
    fprintf(fp, "  begin: CONF_BIT_%d\n", cur_conf_bit_info->index);
    assert((0 == cur_conf_bit_info->sram_val)||(1 == cur_conf_bit_info->sram_val));
    fprintf(fp, "    %s[%d] = %d, ", 
            sram_model_prefix, 
            1- cur_conf_bit_info->sram_val),
    fprintf(fp, "  end\n");
    fprintf(fp, "%d, ", cur_conf_bit_info->sram_val),
    fprintf(fp, "//---- Configuration bit No.: %d, ", cur_conf_bit_info->index);
    fprintf(fp, " SRAM value: %d, ", cur_conf_bit_info->sram_val);
    fprintf(fp, " SPICE model name: %s, ", cur_conf_bit_info->parent_spice_model->name);
    fprintf(fp, " SPICE model index: %d ", cur_conf_bit_info->parent_spice_model_index);
    fprintf(fp, "\n");
    */
    /* For scan chain, the last bit should go first!*/
    if (NULL != temp) {
      /* This is not the tail, keep going */
      rec_dump_verilog_top_testbench_conf_bits_serial(fp, temp);
    }
    break;
  case SPICE_SRAM_MEMORY_BANK:
    /* For memory bank, we do not care the sequence.
     * To be easy to understand, we go from the first to the last
     */
    if (NULL != temp) {
      /* This is not the tail, keep going */
      rec_dump_verilog_top_testbench_conf_bits_serial(fp, temp);
    }
    /* Memory bank requires the address to be given to the decoder*/
    /* If this WL is selected , we decode its index to address */
    if ((1 == cur_conf_bit_info->bl->val)
       &&(1 == cur_conf_bit_info->wl->val)) {
      bl_index = cur_conf_bit_info->index;
      wl_index = cur_conf_bit_info->index;
      cur_prog_cycle = cur_conf_bit_info->index_in_top_tb;
      /* Specify that we need dump this configuration bit */
      dump_cur_conf_bit = 1;
    } else if ((1 == cur_conf_bit_info->bl->val)
             &&(0 == cur_conf_bit_info->wl->val)) {
      /* Find the paired configuration bit */
      bl_index = cur_conf_bit_info->index;
      wl_index = cur_conf_bit_info->pair_conf_bit->index;
      cur_prog_cycle = cur_conf_bit_info->index_in_top_tb;
      assert(cur_prog_cycle == cur_conf_bit_info->pair_conf_bit->index_in_top_tb);
      /* Specify that we need dump this configuration bit */
      dump_cur_conf_bit = 1;
    }
    if (1 == dump_cur_conf_bit) { 
      /* Current programing cycle */
      cur_prog_cycle = cur_prog_cycle * prog_clock_cycle_delay_unit;
      /* Encode addresses */
      /* Bit line address */
      /* If this WL is selected , we decode its index to address */
      bl_addr = (char*)my_calloc(decoder_size + 1, sizeof(char));
      encode_decoder_addr(bl_index, decoder_size, bl_addr);
      fprintf(fp, "Initial\n");
      fprintf(fp, "  begin: CONF_BIT_%d\n", cur_conf_bit_info->index);
      fprintf(fp, "    addr_bl = {%d {1'b0}};", decoder_size);
      /* TODO: which programming cycle should it be ? */
      /* Enable ADDR BL */
      fprintf(fp, "    #%d addr_bl = %d'%s;\n", 
                       cur_prog_cycle, decoder_size, bl_addr);
      /* Disable ADDR BL after one programming clock cycle */
      fprintf(fp, "    #%d addr_bl = {%d {1'b0}};\n", 
                     cur_prog_cycle + 1*prog_clock_cycle_delay_unit, decoder_size);
      fprintf(fp, "  end\n");
      fprintf(fp, "// Configuration bit No.: %d, ", cur_conf_bit_info->index);
      fprintf(fp, " Bit Line: %d, ", cur_conf_bit_info->bl->val);
      fprintf(fp, " SPICE model name: %s, ", cur_conf_bit_info->parent_spice_model->name);
      fprintf(fp, " SPICE model index: %d ", cur_conf_bit_info->parent_spice_model_index);
      fprintf(fp, "\n");
      /* Word line address */
      wl_addr = (char*)my_calloc(decoder_size + 1, sizeof(char));
      encode_decoder_addr(wl_index, decoder_size, wl_addr);
      fprintf(fp, "Initial\n");
      fprintf(fp, "  begin: CONF_BIT_%d\n", cur_conf_bit_info->index);
      fprintf(fp, "    addr_wl = {%d {1'b0}};", decoder_size);
      /* TODO: which programming cycle should it be ? */
      /* Enable ADDR WL */
      fprintf(fp, "    #%d addr_wl = %d'%s;\n", 
                       cur_prog_cycle, decoder_size, bl_addr);
      /* Disable ADDR WL */
      fprintf(fp, "    #%d addr_wl = {%d'{1'b0}};\n", 
                       cur_prog_cycle + 1*prog_clock_cycle_delay_unit, decoder_size);
      fprintf(fp, "  end\n");
      fprintf(fp, "// Configuration bit No.: %d, ", cur_conf_bit_info->index);
      fprintf(fp, " Word Line: %d, ", cur_conf_bit_info->wl->val);
      fprintf(fp, " SPICE model name: %s, ", cur_conf_bit_info->parent_spice_model->name);
      fprintf(fp, " SPICE model index: %d ", cur_conf_bit_info->parent_spice_model_index);
      fprintf(fp, "\n");
    }
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Free */
  my_free(wl_addr);
  my_free(bl_addr);

  return;
}

/* This version contains a simplified configuration phase: 
 * All the configuration bits, i.e., RRAM and SRAMs, are programmed in one clock cycle
 * During the first clock cycle, we program all the configuration bits
 * and also meanwhile reset the whole FPGA
 */
static 
void dump_verilog_top_testbench_stimuli_parallel_version(FILE* fp,
                                                         int num_clock,
                                                         t_spice spice) {
  int inet, iblock, iopad_idx;
  int found_mapped_inpad = 0;
  int num_config_clock_cycles = 1;

  /* Find Input Pad Spice model */
  t_spice_net_info* cur_spice_net_info = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Configuration phase: configure each SRAM or BL/WL */
  fprintf(fp, "//----- Configuration phase -----\n");
  fprintf(fp, "//----- Number of clock cycles in configuration phase: %d -----\n", 
          num_config_clock_cycles);
  /* Test a simplified configuration phase: 
   * configuration bits are programming in one clock cycle
   */
  dump_verilog_top_testbench_conf_bits_parallel(fp, conf_bits_head);

  /* Generate stimilus of programming clock */
  fprintf(fp, "//----- Operation clock ----\n");
  fprintf(fp, "initial\n");
  fprintf(fp, "  begin: OP_CLOCK INITIALIZATION\n");
  fprintf(fp, "    op_clock = 1'b0;\n");
  fprintf(fp, "  end\n");
  /* Reset is enabled from the start and only holds for one clock cycle */
  fprintf(fp, "always\n"); /* TODO: */
  fprintf(fp, "  begin: OP_CLOCK GENERATOR\n");
  fprintf(fp, "    #%d op_clock = ~op_clock\n", op_clock_cycle_delay_unit);
  fprintf(fp, "  end\n");
  fprintf(fp, "\n");

  /* For each element in linked list, generate a voltage stimuli */
  
  /* Operating phase: configuration circuits are off. Stimuli IO pads */
  fprintf(fp, "//----- Operation phase: IO pads -----\n");

  /* reset signals */
  fprintf(fp, "//----- Reset Stimuli ----\n");
  fprintf(fp, "initial\n");
  fprintf(fp, "  begin: RESET GENERATOR\n");
  fprintf(fp, "reset = 1'b0;\n");
  /* Reset is enabled from the start and only holds for one clock cycle */
  fprintf(fp, "//----- Reset signal,only during the first operation clock cycle, is enabled ----\n");
  fprintf(fp, "#%d reset = 1'b0\n", 0 * op_clock_cycle_delay_unit);
  fprintf(fp, "#%d reset = 1'b1\n", 1 * op_clock_cycle_delay_unit);
  fprintf(fp, "#%d reset = 1'b0\n", 2 * op_clock_cycle_delay_unit);
  fprintf(fp, "end\n");
  fprintf(fp, "\n");

  /* set signals */
  fprintf(fp, "//----- Set Stimuli ----\n");
  fprintf(fp, "initial\n");
  fprintf(fp, "  begin: SET GENERATOR\n");
  fprintf(fp, "set = 1'b0;\n");
  fprintf(fp, "//----- Set signal is always disabled -----\n");
  fprintf(fp, "end\n");
  fprintf(fp, "\n");

  /* config_done signal: indicate when configuration is finished */
  fprintf(fp, "//----- config_done Stimuli ----\n");
  fprintf(fp, "initial\n");
  fprintf(fp, "  begin: CONFIG_DONE GENERATOR\n");
  fprintf(fp, "config_done = 1'b0;\n");
  /* Reset is enabled from the start and only holds for one clock cycle */
  fprintf(fp, "//----- config_done signal is enabled when all the configuration is finished ----\n");
  fprintf(fp, "#%d config_done = 1'b1\n", 1 * op_clock_cycle_delay_unit);
  fprintf(fp, "end\n");
  fprintf(fp, "\n");

  /* For each input_signal
   * TODO: this part is low-efficent for run-time concern... Need improve
   */
  assert(NULL != iopad_verilog_model);
  for (iopad_idx = 0; iopad_idx < iopad_verilog_model->cnt; iopad_idx++) {
    /* Find if this inpad is mapped to a logical block */
    found_mapped_inpad = 0;
    for (iblock = 0; iblock < num_logical_blocks; iblock++) {
      if ((iopad_verilog_model == logical_block[iblock].mapped_spice_model)
         &&(iopad_idx == logical_block[iblock].mapped_spice_model_index)) {
        /* Make sure We find the correct logical block !*/
        assert(VPACK_INPAD == logical_block[iblock].type);
        cur_spice_net_info = NULL;
        for (inet = 0; inet < num_nets; inet++) { 
          if (0 == strcmp(clb_net[inet].name, logical_block[iblock].name)) {
            cur_spice_net_info = clb_net[inet].spice_net_info;
            break;
          }
        }
        assert(NULL != cur_spice_net_info);
        assert(!(0 > cur_spice_net_info->density));
        assert(!(1 < cur_spice_net_info->density));
        assert(!(0 > cur_spice_net_info->probability));
        assert(!(1 < cur_spice_net_info->probability));
        /* Get the net information */
        /* TODO: Give the net name in the blif file !*/
        fprintf(fp, "//----- Input %s Stimuli ----\n", logical_block[iblock].name);
        fprintf(fp, "initial\n");
        fprintf(fp, "  begin: Input %s GENERATOR\n", logical_block[iblock].name);
        fprintf(fp, "    %s%s[%d] = 1'b%d;\n", 
                gio_inout_prefix, iopad_verilog_model->prefix, iopad_idx,
                cur_spice_net_info->init_val);
        fprintf(fp, "end\n");
        fprintf(fp, "always @(config_done)\n");
        fprintf(fp, "  begin \n");
        fprintf(fp, "    #%d %s%s[%d] = ~%s%s[%d];\n", 
                (int)(op_clock_cycle_delay_unit * cur_spice_net_info->density * 2. / cur_spice_net_info->probability), 
                gio_inout_prefix, iopad_verilog_model->prefix, iopad_idx,
                gio_inout_prefix, iopad_verilog_model->prefix, iopad_idx);
        fprintf(fp, "  end \n");
        fprintf(fp, "\n");
        found_mapped_inpad++;
      }
    } 
    assert((0 == found_mapped_inpad)||(1 == found_mapped_inpad));
    /* if we cannot find any mapped inpad from tech.-mapped netlist, give a default */
    if (0 == found_mapped_inpad) {
      /* TODO: Give the net name in the blif file !*/
      fprintf(fp, "//----- Input %s[%d] Stimuli ----\n", gio_input_prefix, iopad_idx);
      fprintf(fp, "initial\n");
      fprintf(fp, "  begin: Input %s[%d] GENERATOR\n", gio_input_prefix, iopad_idx);
      fprintf(fp, "    %s%s[%d] = 1'b%d;\n", 
              gio_inout_prefix, iopad_verilog_model->prefix, iopad_idx,
              verilog_default_signal_init_value);
      fprintf(fp, "end\n");
    }
  }

  /* Finish */
  
  return;
}


/* Print Stimulations for top-level netlist 
 * Task list:
 * 1. For clock signal, we should create voltage waveforms
 * 2. For Set/Reset, TODO: should we reset the chip in the first cycle ???
 * 3. For input/output clb nets (mapped to I/O grids), we should create voltage waveforms
 */
static 
void dump_verilog_top_testbench_stimuli_serial_version(FILE* fp,
                                                       int num_clock,
                                                       t_spice spice) {
  int inet, iblock, iopad_idx;
  int found_mapped_inpad = 0;
  int delay_cnt = 0;
  int num_config_clock_cycles = 0;

  /* Find Input Pad Spice model */
  t_spice_net_info* cur_spice_net_info = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Configuration phase: configure each SRAM or BL/WL */
  fprintf(fp, "//----- Configuration phase -----\n");

  /* Test a complete configuration phase: 
   * configuration bits are programming in serial (one by one)
   */
  /* Estimate the number of configuration clock cycles 
   * by traversing the linked-list and count the number of SRAM=1 or BL=1&WL=1 in it.
   */
  num_config_clock_cycles = dump_verilog_top_testbench_find_num_config_clock_cycles(conf_bits_head);
  fprintf(fp, "//----- Number of clock cycles in configuration phase: %d -----\n", 
          num_config_clock_cycles);
  /* Generate stimilus of programming clock */
  fprintf(fp, "//----- Programming clock ----\n");
  fprintf(fp, "initial\n");
  fprintf(fp, "  begin: PROG_CLOCK INITIALIZATION\n");
  fprintf(fp, "    prog_clock = 1'b0;\n");
  fprintf(fp, "  end\n");
  /* Reset is enabled from the start and only holds for one clock cycle */
  delay_cnt = prog_clock_cycle_delay_unit * 1;
  fprintf(fp, "always\n"); /* TODO: */
  fprintf(fp, "  begin: PROG_CLOCK GENERATOR\n");
  fprintf(fp, "    #%d prog_clock = ~prog_clock\n", delay_cnt);
  fprintf(fp, "  end\n");
  fprintf(fp, "\n");

  /* Generate stimilus of programming clock */
  fprintf(fp, "//----- Operation clock ----\n");
  fprintf(fp, "initial\n");
  fprintf(fp, "  begin: OP_CLOCK INITIALIZATION\n");
  fprintf(fp, "    op_clock = 1'b0;\n");
  fprintf(fp, "  end\n");
  /* Reset is enabled from the start and only holds for one clock cycle */
  delay_cnt = prog_clock_cycle_delay_unit * 1;
  fprintf(fp, "always\n"); /* TODO: */
  fprintf(fp, "  begin: OP_CLOCK GENERATOR\n");
  fprintf(fp, "    #%d op_clock = ~op_clock\n", delay_cnt);
  fprintf(fp, "  end\n");
  fprintf(fp, "\n");

  /* For each element in linked list, generate a voltage stimuli */
  rec_dump_verilog_top_testbench_conf_bits_serial(fp, conf_bits_head);
  
  /* Operating phase: configuration circuits are off. Stimuli IO pads */
  fprintf(fp, "//----- Operation phase: IO pads -----\n");
  /* determine the base of time slot where operation mode starts */
  delay_cnt = num_config_clock_cycles * prog_clock_cycle_delay_unit;

  /* reset signals */
  fprintf(fp, "//----- Reset Stimuli ----\n");
  fprintf(fp, "initial\n");
  fprintf(fp, "  begin: RESET GENERATOR\n");
  fprintf(fp, "reset = 1'b0;\n");
  /* Reset is enabled from the start and only holds for one clock cycle */
  fprintf(fp, "//----- Reset signal,only during the first operation clock cycle, is enabled ----\n");
  fprintf(fp, "#%d reset = 1'b0\n", delay_cnt + 0 * op_clock_cycle_delay_unit);
  fprintf(fp, "#%d reset = 1'b1\n", delay_cnt + 1 * op_clock_cycle_delay_unit);
  fprintf(fp, "#%d reset = 1'b0\n", delay_cnt + 2 * op_clock_cycle_delay_unit);
  fprintf(fp, "end\n");
  fprintf(fp, "\n");

  /* set signals */
  fprintf(fp, "//----- Set Stimuli ----\n");
  fprintf(fp, "initial\n");
  fprintf(fp, "  begin: SET GENERATOR\n");
  fprintf(fp, "set = 1'b0;\n");
  fprintf(fp, "//----- Set signal is always disabled -----\n");
  fprintf(fp, "end\n");
  fprintf(fp, "\n");

  /* config_done signal: indicate when configuration is finished */
  fprintf(fp, "//----- config_done Stimuli ----\n");
  fprintf(fp, "initial\n");
  fprintf(fp, "  begin: CONFIG_DONE GENERATOR\n");
  fprintf(fp, "config_done = 1'b0;\n");
  /* Reset is enabled from the start and only holds for one clock cycle */
  fprintf(fp, "//----- config_done signal is enabled when all the configuration is finished ----\n");
  fprintf(fp, "#%d config_done = 1'b1\n", delay_cnt + 1 * op_clock_cycle_delay_unit);
  fprintf(fp, "end\n");
  fprintf(fp, "\n");

  /* For each input_signal
   * TODO: this part is low-efficent for run-time concern... Need improve
   */
  assert(NULL != iopad_verilog_model);
  for (iopad_idx = 0; iopad_idx < iopad_verilog_model->cnt; iopad_idx++) {
    /* Find if this inpad is mapped to a logical block */
    found_mapped_inpad = 0;
    for (iblock = 0; iblock < num_logical_blocks; iblock++) {
      if ((iopad_verilog_model == logical_block[iblock].mapped_spice_model)
         &&(iopad_idx == logical_block[iblock].mapped_spice_model_index)) {
        /* Make sure We find the correct logical block !*/
        assert(VPACK_INPAD == logical_block[iblock].type);
        cur_spice_net_info = NULL;
        for (inet = 0; inet < num_nets; inet++) { 
          if (0 == strcmp(clb_net[inet].name, logical_block[iblock].name)) {
            cur_spice_net_info = clb_net[inet].spice_net_info;
            break;
          }
        }
        assert(NULL != cur_spice_net_info);
        assert(!(0 > cur_spice_net_info->density));
        assert(!(1 < cur_spice_net_info->density));
        assert(!(0 > cur_spice_net_info->probability));
        assert(!(1 < cur_spice_net_info->probability));
        /* Get the net information */
        /* TODO: Give the net name in the blif file !*/
        fprintf(fp, "//----- Input %s Stimuli ----\n", logical_block[iblock].name);
        fprintf(fp, "initial\n");
        fprintf(fp, "  begin: Input %s GENERATOR\n", logical_block[iblock].name);
        fprintf(fp, "    %s%s[%d] = 1'b%d;\n", 
                gio_inout_prefix, iopad_verilog_model->prefix, iopad_idx,
                cur_spice_net_info->init_val);
        fprintf(fp, "end\n");
        fprintf(fp, "always @(config_done)\n");
        fprintf(fp, "  begin \n");
        fprintf(fp, "    #%d %s%s[%d] = ~%s%s[%d];\n", 
                (int)(op_clock_cycle_delay_unit * cur_spice_net_info->density * 2. / cur_spice_net_info->probability), 
                gio_inout_prefix, iopad_verilog_model->prefix, iopad_idx,
                gio_inout_prefix, iopad_verilog_model->prefix, iopad_idx);
        fprintf(fp, "  end \n");
        fprintf(fp, "\n");
        found_mapped_inpad++;
      }
    } 
    assert((0 == found_mapped_inpad)||(1 == found_mapped_inpad));
    /* if we cannot find any mapped inpad from tech.-mapped netlist, give a default */
    if (0 == found_mapped_inpad) {
      /* TODO: Give the net name in the blif file !*/
      fprintf(fp, "//----- Input %s[%d] Stimuli ----\n", gio_input_prefix, iopad_idx);
      fprintf(fp, "initial\n");
      fprintf(fp, "  begin: Input %s[%d] GENERATOR\n", gio_input_prefix, iopad_idx);
      fprintf(fp, "    %s%s[%d] = 1'b%d;\n", 
              gio_inout_prefix, iopad_verilog_model->prefix, iopad_idx,
              verilog_default_signal_init_value);
      fprintf(fp, "end\n");
    }
  }

  /* Finish */
  
  return;
}

/* Generate the stimuli for the top-level testbench 
 * The simulation consists of two phases: configuration phase and operation phase
 * We have two choices for the configuration phase:
 * 1. Configuration bits are loaded serially. 
 *    This is actually what we do for a physical FPGA
 * 2. Configuration bits are loaded parallel. 
 *    This is only feasible in simulation, which is convenient to check functionality.
 */
void dump_verilog_top_testbench_stimuli(FILE* fp,
                                        int num_clock,
                                        t_syn_verilog_opts syn_verilog_opts,
                                        t_spice verilog) {

  if (TRUE == syn_verilog_opts.tb_serial_config_mode) {
    dump_verilog_top_testbench_stimuli_serial_version(fp, num_clock, verilog);
  } else {
    dump_verilog_top_testbench_stimuli_parallel_version(fp, num_clock, verilog);
  }

  return;
}


/***** Print Top-level SPICE netlist *****/
void dump_verilog_top_netlist(char* circuit_name,
                              char* top_netlist_name,
                              char* include_dir_path,
                              char* subckt_dir_path,
                              int LL_num_rr_nodes,
                              t_rr_node* LL_rr_node,
                              t_ivec*** LL_rr_node_indices,
                              int num_clock,
                              t_spice verilog) {
  FILE* fp = NULL;
  char* formatted_subckt_dir_path = format_dir_path(subckt_dir_path);
  char* temp_include_file_path = NULL;
  char* title = my_strcat("FPGA Verilog Netlist for Design: ", circuit_name);

  /* Check if the path exists*/
  fp = fopen(top_netlist_name,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create top Verilog netlist %s!",__FILE__, __LINE__, top_netlist_name); 
    exit(1);
  } 
  
  vpr_printf(TIO_MESSAGE_INFO, "Writing FPGA Top-level Verilog Netlist for %s...\n", circuit_name);
 
  /* Print the title */
  dump_verilog_file_header(fp, title);
  my_free(title);

  /* Include user-defined sub-circuit netlist */
  fprintf(fp, "//----- Include User-defined netlists -----\n");
  init_include_user_defined_verilog_netlists(verilog);
  dump_include_user_defined_verilog_netlists(fp, verilog);
  
  /* Special subckts for Top-level SPICE netlist */
  fprintf(fp, "//----- Include subckt netlists: Multiplexers -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, muxes_verilog_file_name);
  fprintf(fp, "// `include \"%s\"\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "//----- Include subckt netlists: Wires -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, wires_verilog_file_name);
  fprintf(fp, "// `include \"%s\"\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "//----- Include subckt netlists: Look-Up Tables (LUTs) -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, luts_verilog_file_name);
  fprintf(fp, "// `include \"%s\"\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "//------ Include subckt netlists: Logic Blocks -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, logic_block_verilog_file_name);
  fprintf(fp, "// `include \"%s\"\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "//----- Include subckt netlists: Routing structures (Switch Boxes, Channels, Connection Boxes) -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, routing_verilog_file_name);
  fprintf(fp, "// `include \"%s\"\n", temp_include_file_path);
  my_free(temp_include_file_path);
 
  /* Include decoders if required */ 
  switch(sram_verilog_orgz_type) {
  case SPICE_SRAM_STANDALONE:
  case SPICE_SRAM_SCAN_CHAIN:
    break;
  case SPICE_SRAM_MEMORY_BANK:
    /* Include verilog decoder */
    fprintf(fp, "//----- Include subckt netlists: Decoders (controller for memeory bank) -----\n");
    temp_include_file_path = my_strcat(formatted_subckt_dir_path, decoders_verilog_file_name);
    fprintf(fp, "// `include \"%s\"\n", temp_include_file_path);
    my_free(temp_include_file_path);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }
 
  /* Print all global wires*/
  dump_verilog_top_netlist_ports(fp, num_clock, circuit_name, verilog);

  /* Quote defined Logic blocks subckts (Grids) */
  dump_verilog_defined_grids(fp);

  /* Quote Routing structures: Channels */
  dump_verilog_defined_channels(fp, LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);

  /* Quote Routing structures: Conneciton Boxes */
  dump_verilog_defined_connection_boxes(fp); 
  
  /* Quote Routing structures: Switch Boxes */
  dump_verilog_defined_switch_boxes(fp); 

  /* Dump configuration circuits */
  dump_verilog_configuration_circuits(fp);

  /* verilog ends*/
  fprintf(fp, "endmodule\n");

  /* Close the file*/
  fclose(fp);

  return;
}

/** Top level function 2: Testbench for the top-level netlist
 * This testbench includes a top-level module of a mapped FPGA and voltage pulses
 */
void dump_verilog_top_testbench(char* circuit_name,
                                char* top_netlist_name,
                                int num_clock,
                                t_syn_verilog_opts syn_verilog_opts,
                                t_spice verilog) {
  FILE* fp = NULL;
  char* title = my_strcat("FPGA Verilog Testbench for Top-level netlist of Design: ", circuit_name);

  /* Check if the path exists*/
  fp = fopen(top_netlist_name,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create top Verilog testbench %s!",__FILE__, __LINE__, top_netlist_name); 
    exit(1);
  } 
  
  vpr_printf(TIO_MESSAGE_INFO, "Writing Testbench for FPGA Top-level Verilog netlist for  %s...\n", circuit_name);
 
  /* Print the title */
  dump_verilog_file_header(fp, title);
  my_free(title);

  /* Start of testbench */
  dump_verilog_top_testbench_ports(fp, circuit_name);

  /* Call defined top-level module */
  dump_verilog_top_testbench_call_top_module(fp, circuit_name);

  /* Add stimuli for reset, set, clock and iopad signals */
  dump_verilog_top_testbench_stimuli(fp, num_clock, syn_verilog_opts, verilog);

  /* Testbench ends*/
  fprintf(fp, "endmodule\n");

  /* Close the file*/
  fclose(fp);

  return;
}
