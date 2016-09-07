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
#include "spice_utils.h"
#include "spice_netlist_utils.h"

/* Include verilog support headers*/
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_routing.h"
#include "verilog_pbtypes.h"
#include "verilog_top_netlist.h"

/* Local Subroutines declaration */

/******** Subroutines ***********/
static 
void dump_verilog_top_netlist_ports(FILE* fp,
                                    int num_clocks,
                                    char* circuit_name,
                                    t_spice verilog) {

  int decoder_size = determine_decoder_size(sram_verilog_model->cnt);

  /* A valid file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  fprintf(fp, "//----- Top-level Verilog Module -----\n");
  fprintf(fp, "module %s_top (\n", circuit_name);

  /* Inputs and outputs of I/O pads */
  /* Input Pads */
  assert(NULL != inpad_verilog_model);
  fprintf(fp, " input wire [%d:0] gfpga_input_%s, //---FPGA inputs \n", 
          inpad_verilog_model->cnt - 1,
          inpad_verilog_model->prefix);

  /* Output Pads */
  assert(NULL != outpad_verilog_model);
  fprintf(fp, " output wire [%d:0] gfpga_output_%s, //---- FPGA outputs \n", 
          outpad_verilog_model->cnt - 1,
          outpad_verilog_model->prefix);


  /* Configuration ports depend on the organization of SRAMs */
  switch(sram_verilog_orgz_type) {
  case SPICE_SRAM_STANDALONE:
    fprintf(fp, "  input wire [%d:0] %s //---- SRAM outputs \n", 
            sram_verilog_model->cnt - 1,
            sram_verilog_model->prefix); 
    break;
  case SPICE_SRAM_SCAN_CHAIN:
    /* TODO: currently, I use the same number of inputs as SRAM_STANDALONE
     * It will be changed to the number of scan-chain heads we want 
				*/
    fprintf(fp, "  input wire [%d:0] %s //---- Scan-chain outputs \n", 
            sram_verilog_model->cnt - 1,
            sram_verilog_model->prefix); 
    break;
  case SPICE_SRAM_MEMORY_BANK:
    fprintf(fp, "  input en_bl, en_wl,\n");
    fprintf(fp, "  input wire [%d:0] addr_bl, //--- Address of bit lines \n", 
                   decoder_size - 1);
    fprintf(fp, "  input wire [%d:0] addr_wl, //--- Address of word lines \n", 
                   decoder_size - 1);
    /* I add all the Bit lines and Word lines here just for testbench usage */
    fprintf(fp, "  input wire [%d:0] %s_out, //--- Bit lines \n", 
                   sram_verilog_model->cnt - 1, sram_verilog_model->prefix);
    fprintf(fp, "  input wire [%d:0] %s_outb //--- Word lines \n", 
                   sram_verilog_model->cnt - 1, sram_verilog_model->prefix);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  fprintf(fp, ");\n");

  /* Print wires of SRAM outputs */

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
      /* Comment lines */
      fprintf(fp, "//----- BEGIN Call Grid[%d][%d] module -----\n", ix, iy);
      /* Print the Grid module */
      fprintf(fp, "grid_%d__%d_  ", ix, iy); /* Call the name of subckt */ 
      fprintf(fp, "grid_%d__%d_ ", ix, iy);
      fprintf(fp, "(");
      dump_verilog_grid_pins(fp, ix, iy, 1, FALSE, FALSE);
      /* Print Input Pad and Output Pad */
      assert(!(0 > (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])));
      if (0 < (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])) {
        fprintf(fp, ",");
        fprintf(fp, "  gfpga_input_%s[%d:%d] \n", 
                inpad_verilog_model->prefix, 
                inpad_verilog_model->grid_index_high[ix][iy] - 1, 
                inpad_verilog_model->grid_index_low[ix][iy]);
      }
      assert(!(0 > (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])));
      if (0 < (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])) {
        fprintf(fp, ",");
        fprintf(fp, " gfpga_output_%s[%d:%d] \n", 
                outpad_verilog_model->prefix, 
                outpad_verilog_model->grid_index_high[ix][iy] - 1, 
                outpad_verilog_model->grid_index_low[ix][iy]);
      }
      /* Configuration ports */
      assert(!(0 > sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]));
      if (0 < sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]) {
        fprintf(fp, ",");
        fprintf(fp, "  %s_out[%d:%d], \n", 
                sram_verilog_model->prefix, 
                sram_verilog_model->grid_index_high[ix][iy] - 1,
                sram_verilog_model->grid_index_low[ix][iy]);
        /* inverted output of each configuration bit */
        fprintf(fp, "  %s_outb[%d:%d] \n", 
                sram_verilog_model->prefix, 
                sram_verilog_model->grid_index_high[ix][iy] - 1,
                sram_verilog_model->grid_index_low[ix][iy]);
        
      }
      fprintf(fp, ");\n");
      /* Comment lines */
      fprintf(fp, "//----- END call Grid[%d][%d] module -----\n\n", ix, iy);
    }
  } 

  /* IO Grids */
  /* LEFT side */
  ix = 0;
  for (iy = 1; iy < (ny + 1); iy++) {
    assert(IO_TYPE == grid[ix][iy].type);
    fprintf(fp, "grid_%d__%d_ ", ix, iy); /* Call the name of subckt */ 
    fprintf(fp, "grid_%d__%d_ ", ix, iy);
    fprintf(fp, "(");
    dump_verilog_io_grid_pins(fp, ix, iy, 1, FALSE, FALSE);
    /* Print Input Pad and Output Pad */
    assert(!(0 > (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])));
    if (0 < (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])) {
      fprintf(fp, ",");
      fprintf(fp, "  gfpga_input_%s[%d:%d] \n", 
              inpad_verilog_model->prefix, 
              inpad_verilog_model->grid_index_high[ix][iy] - 1, 
              inpad_verilog_model->grid_index_low[ix][iy]);
    }
    assert(!(0 > (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])));
    if (0 < (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])) {
      fprintf(fp, ",");
      fprintf(fp, " gfpga_output_%s[%d:%d] \n", 
              outpad_verilog_model->prefix, 
              outpad_verilog_model->grid_index_high[ix][iy] - 1, 
              outpad_verilog_model->grid_index_low[ix][iy]);
    }
    /* Configuration ports */
    assert(!(0 > sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]));
    if (0 < sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]) {
      fprintf(fp, ",");
      fprintf(fp, "  %s_out[%d:%d], \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->grid_index_low[ix][iy],
              sram_verilog_model->grid_index_high[ix][iy] - 1);
      /* inverted output of each configuration bit */
      fprintf(fp, "  %s_outb[%d:%d] \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->grid_index_low[ix][iy],
              sram_verilog_model->grid_index_high[ix][iy] - 1);
        
    }
    fprintf(fp, ");\n");
  }

  /* RIGHT side */
  ix = nx + 1;
  for (iy = 1; iy < (ny + 1); iy++) {
    assert(IO_TYPE == grid[ix][iy].type);
    fprintf(fp, "grid_%d__%d_ ", ix, iy); /* Call the name of subckt */ 
    fprintf(fp, "grid_%d__%d_ ", ix, iy);
    fprintf(fp, "(");
    dump_verilog_io_grid_pins(fp, ix, iy, 1, FALSE, FALSE);
    /* Print Input Pad and Output Pad */
    assert(!(0 > (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])));
    if (0 < (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])) {
      fprintf(fp, ",");
      fprintf(fp, "  gfpga_input_%s[%d:%d] \n", 
              inpad_verilog_model->prefix, 
              inpad_verilog_model->grid_index_high[ix][iy] - 1, 
              inpad_verilog_model->grid_index_low[ix][iy]);
    }
    assert(!(0 > (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])));
    if (0 < (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])) {
      fprintf(fp, ",");
      fprintf(fp, " gfpga_output_%s[%d:%d] \n", 
              outpad_verilog_model->prefix, 
              outpad_verilog_model->grid_index_high[ix][iy] - 1, 
              outpad_verilog_model->grid_index_low[ix][iy]);
    }
    /* Configuration ports */
    assert(!(0 > sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]));
    if (0 < sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]) {
      fprintf(fp, ",");
      fprintf(fp, "  %s_out[%d:%d], \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->grid_index_low[ix][iy],
              sram_verilog_model->grid_index_high[ix][iy] - 1);
      /* inverted output of each configuration bit */
      fprintf(fp, "  %s_outb[%d:%d] \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->grid_index_low[ix][iy],
              sram_verilog_model->grid_index_high[ix][iy] - 1);
        
    }
    fprintf(fp, ");\n");
  }

  /* BOTTOM side */
  iy = 0;
  for (ix = 1; ix < (nx + 1); ix++) {
    assert(IO_TYPE == grid[ix][iy].type);
    fprintf(fp, "grid_%d__%d_ ", ix, iy); /* Call the name of subckt */ 
    fprintf(fp, "grid_%d__%d_ ", ix, iy);
    fprintf(fp, "(");
    dump_verilog_io_grid_pins(fp, ix, iy, 1, FALSE, FALSE);
    /* Print Input Pad and Output Pad */
    assert(!(0 > (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])));
    if (0 < (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])) {
      fprintf(fp, ",");
      fprintf(fp, "  gfpga_input_%s[%d:%d] \n", 
              inpad_verilog_model->prefix, 
              inpad_verilog_model->grid_index_high[ix][iy] - 1, 
              inpad_verilog_model->grid_index_low[ix][iy]);
    }
    assert(!(0 > (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])));
    if (0 < (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])) {
      fprintf(fp, ",");
      fprintf(fp, " gfpga_output_%s[%d:%d] \n", 
              outpad_verilog_model->prefix, 
              outpad_verilog_model->grid_index_high[ix][iy] - 1, 
              outpad_verilog_model->grid_index_low[ix][iy]);
    }
    /* Configuration ports */
    assert(!(0 > sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]));
    if (0 < sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]) {
      fprintf(fp, ",");
      fprintf(fp, "  %s_out[%d:%d], \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->grid_index_low[ix][iy],
              sram_verilog_model->grid_index_high[ix][iy] - 1);
      /* inverted output of each configuration bit */
      fprintf(fp, "  %s_outb[%d:%d] \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->grid_index_low[ix][iy],
              sram_verilog_model->grid_index_high[ix][iy] - 1);
        
    }
    fprintf(fp, ");\n");
  } 

  /* TOP side */
  iy = ny + 1;
  for (ix = 1; ix < (nx + 1); ix++) {
    assert(IO_TYPE == grid[ix][iy].type);
    fprintf(fp, "grid_%d__%d_ ", ix, iy); /* Call the name of subckt */ 
    fprintf(fp, "grid_%d__%d_ ", ix, iy);
    fprintf(fp, "(");
    dump_verilog_io_grid_pins(fp, ix, iy, 1, FALSE, FALSE);
    /* Print Input Pad and Output Pad */
    assert(!(0 > (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])));
    if (0 < (inpad_verilog_model->grid_index_high[ix][iy] - inpad_verilog_model->grid_index_low[ix][iy])) {
      fprintf(fp, ",");
      fprintf(fp, "  gfpga_input_%s[%d:%d] \n", 
              inpad_verilog_model->prefix, 
              inpad_verilog_model->grid_index_high[ix][iy] - 1, 
              inpad_verilog_model->grid_index_low[ix][iy]);
    }
    assert(!(0 > (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])));
    if (0 < (outpad_verilog_model->grid_index_high[ix][iy] - outpad_verilog_model->grid_index_low[ix][iy])) {
      fprintf(fp, ",");
      fprintf(fp, " gfpga_output_%s[%d:%d] \n", 
              outpad_verilog_model->prefix, 
              outpad_verilog_model->grid_index_high[ix][iy] - 1, 
              outpad_verilog_model->grid_index_low[ix][iy]);
    }
    /* Configuration ports */
    assert(!(0 > sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]));
    if (0 < sram_verilog_model->grid_index_high[ix][iy] - sram_verilog_model->grid_index_low[ix][iy]) {
      fprintf(fp, ",");
      fprintf(fp, "  %s_out[%d:%d], \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->grid_index_low[ix][iy],
              sram_verilog_model->grid_index_high[ix][iy] - 1);
      /* inverted output of each configuration bit */
      fprintf(fp, "  %s_outb[%d:%d] \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->grid_index_low[ix][iy],
              sram_verilog_model->grid_index_high[ix][iy] - 1);
        
    }
    fprintf(fp, ");\n");
    /* Connect to a speical vdd port for statistics power */
  } 

  return;
}

/* Call defined channels. 
 * Ensure the port name here is co-herent to other sub-circuits(SB,CB,grid)!!!
 */
void dump_verilog_defined_chan(FILE* fp,
                             t_rr_type chan_type,
                             int x,
                             int y,
                             int chan_width) {
  int itrack;

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }
 
  /* Check */
  switch (chan_type) {
  case CHANX:
    /* check x*/
    assert((0 < x)&&(x < (nx + 1))); 
    /* check y*/
    assert((!(0 > y))&&(y < (ny + 1))); 
    /* Comment lines */
    fprintf(fp, "//----- BEGIN Call Channel-X [%d][%d] module -----\n", x, y);
    /* Call the define sub-circuit */
    fprintf(fp, "chanx_%d__%d_ ", x, y);
    fprintf(fp, "chanx_%d__%d_ ", x, y);
    fprintf(fp, "(");
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chanx_%d__%d__in_%d_, ", x, y, itrack);
      fprintf(fp, "\n");
    }
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chanx_%d__%d__out_%d_, ", x, y, itrack);
      fprintf(fp, "\n");
    }
    /* output at middle point */
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chanx_%d__%d__midout_%d_, ", x, y, itrack);
      fprintf(fp, "\n");
    }
    fprintf(fp, ");\n");
    /* Comment lines */
    fprintf(fp, "//----- END call Channel-X [%d][%d] module -----\n\n", x, y);
    break;
  case CHANY:
    /* check x*/
    assert((!(0 > x))&&(x < (nx + 1))); 
    /* check y*/
    assert((0 < y)&&(y < (ny + 1))); 
    /* Comment lines */
    fprintf(fp, "//----- BEGIN call Call Channel-Y [%d][%d] module -----\n", x, y);
    /* Call the define sub-circuit */
    fprintf(fp, "chany_%d__%d_ ", x, y);
    fprintf(fp, "chany_%d__%d_ ", x, y);
    fprintf(fp, "(");
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chany_%d__%d__in_%d_, ", x, y, itrack);
      fprintf(fp, "\n");
    }
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chany_%d__%d__out_%d_, ", x, y, itrack);
      fprintf(fp, "\n");
    }
    /* output at middle point */
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chany_%d__%d__midout_%d_, ", x, y, itrack);
      fprintf(fp, "\n");
    }
    fprintf(fp, ");\n");
    /* Comment lines */
    fprintf(fp, "//----- END call Channel-Y [%d][%d] module -----\n\n", x, y);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid Channel Type!\n", __FILE__, __LINE__);
    exit(1);
  }


  return;
}

/* Call the sub-circuits for channels : Channel X and Channel Y*/
void dump_verilog_defined_channels(FILE* fp) {
  int ix, iy;

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Channel X */
  for (iy = 0; iy < (ny + 1); iy++) {
    for (ix = 1; ix < (nx + 1); ix++) {
      dump_verilog_defined_chan(fp, CHANX, ix, iy, chan_width_x[iy]);
    }
  }

  /* Channel Y */
  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      dump_verilog_defined_chan(fp, CHANY, ix, iy, chan_width_y[ix]);
    }
  }

  return;
}

/* Call the defined sub-circuit of connection box
 * TODO: actually most of this function is copied from
 * spice_routing.c : dump_verilog_conneciton_box_interc
 * Should be more clever to use the original function
 */
void dump_verilog_defined_connection_box(FILE* fp,
                                        t_rr_type chan_type,
                                        int x,
                                        int y,
                                        int chan_width,
                                        t_ivec*** LL_rr_node_indices) {
  int itrack, side;
  int side_cnt = 0;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 
  
  /* Print the definition of subckt*/
  /* Identify the type of connection box */
  switch(chan_type) {
  case CHANX:
    /* Comment lines */
    fprintf(fp, "//----- BEGIN Call Connection Box-X direction [%d][%d] module -----\n", x, y);
    /* Print module */
    fprintf(fp, "cbx_%d__%d_ ", x, y);
    fprintf(fp, "cbx_%d__%d_ ", x, y);
    break;
  case CHANY:
    /* Comment lines */
    fprintf(fp, "//----- BEGIN Call Connection Box-Y direction [%d][%d] module -----\n", x, y);
    /* Print module */
    fprintf(fp, "cby_%d__%d_ ", x, y);
    fprintf(fp, "cby_%d__%d_ ", x, y);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }
 
  fprintf(fp, "(");
  /* Print the ports of channels*/
  /* connect to the mid point of a track*/
  for (itrack = 0; itrack < chan_width; itrack++) {
    switch(chan_type) { 
    case CHANX:
      fprintf(fp, "chanx_%d__%d__midout_%d_, ", x, y, itrack);
      break;
    case CHANY:
      fprintf(fp, "chany_%d__%d__midout_%d_, ", x, y, itrack);
      break;
    default: 
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
      exit(1);
    }
    fprintf(fp, "\n");
  }
  /* Print the ports of grids*/
  side_cnt = 0;
  for (side = 0; side < 4; side++) {
    switch (side) {
    case 0: /* TOP */
      switch(chan_type) { 
      case CHANX:
        /* BOTTOM INPUT Pins of Grid[x][y+1] */
        dump_verilog_grid_side_pins(fp, IPIN, x, y + 1, 2, FALSE);
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
        /* LEFT INPUT Pins of Grid[x+1][y] */
        dump_verilog_grid_side_pins(fp, IPIN, x + 1, y, 3, FALSE);
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
        /* TOP INPUT Pins of Grid[x][y] */
        dump_verilog_grid_side_pins(fp, IPIN, x, y, 0, FALSE);
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
        /* RIGHT INPUT Pins of Grid[x][y] */
        dump_verilog_grid_side_pins(fp, IPIN, x, y, 1, FALSE);
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
    fprintf(fp, "\n");
  }
  /* Configuration ports */
  switch(chan_type) { 
  case CHANX:
    /* Print Input Pad and Output Pad */
    assert(!(0 > (inpad_verilog_model->cbx_index_high[x][y] - inpad_verilog_model->cbx_index_low[x][y])));
    if (0 < (inpad_verilog_model->cbx_index_high[x][y] - inpad_verilog_model->cbx_index_low[x][y])) {
      fprintf(fp, "  gfpga_input_%s[%d:%d], \n", 
              inpad_verilog_model->prefix, 
              inpad_verilog_model->cbx_index_high[x][y] - 1, 
              inpad_verilog_model->cbx_index_low[x][y]);
    }
    assert(!(0 > (outpad_verilog_model->cbx_index_high[x][y] - outpad_verilog_model->cbx_index_low[x][y])));
    if (0 < (outpad_verilog_model->cbx_index_high[x][y] - outpad_verilog_model->cbx_index_low[x][y])) {
      fprintf(fp, " gfpga_output_%s[%d:%d], \n", 
              outpad_verilog_model->prefix, 
              outpad_verilog_model->cbx_index_high[x][y] - 1, 
              outpad_verilog_model->cbx_index_low[x][y]);
    }
    assert(!(0 > sram_verilog_model->cbx_index_high[x][y] - sram_verilog_model->cbx_index_low[x][y]));
    if (0 < sram_verilog_model->cbx_index_high[x][y] - sram_verilog_model->cbx_index_low[x][y]) {
      fprintf(fp, "  %s_out[%d:%d], \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->cbx_index_high[x][y] - 1,
              sram_verilog_model->cbx_index_low[x][y]);
      /* inverted output of each configuration bit */
      fprintf(fp, "  %s_outb[%d:%d] \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->cbx_index_high[x][y] - 1,
              sram_verilog_model->cbx_index_low[x][y]);
        
    }
    break;
  case CHANY:
    /* Print Input Pad and Output Pad */
    assert(!(0 > (inpad_verilog_model->cby_index_high[x][y] - inpad_verilog_model->cby_index_low[x][y])));
    if (0 < (inpad_verilog_model->cby_index_high[x][y] - inpad_verilog_model->cby_index_low[x][y])) {
      fprintf(fp, "  gfpga_input_%s[%d:%d], \n", 
              inpad_verilog_model->prefix, 
              inpad_verilog_model->cby_index_high[x][y] - 1, 
              inpad_verilog_model->cby_index_low[x][y]);
    }
    assert(!(0 > (outpad_verilog_model->cby_index_high[x][y] - outpad_verilog_model->cby_index_low[x][y])));
    if (0 < (outpad_verilog_model->cby_index_high[x][y] - outpad_verilog_model->cby_index_low[x][y])) {
      fprintf(fp, " gfpga_output_%s[%d:%d], \n", 
              outpad_verilog_model->prefix, 
              outpad_verilog_model->cby_index_high[x][y] - 1, 
              outpad_verilog_model->cby_index_low[x][y]);
    }
    assert(!(0 > sram_verilog_model->cby_index_high[x][y] - sram_verilog_model->cby_index_low[x][y]));
    if (0 < sram_verilog_model->cby_index_high[x][y] - sram_verilog_model->cby_index_low[x][y]) {
      fprintf(fp, "  %s_out[%d:%d], \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->cby_index_high[x][y] - 1,
              sram_verilog_model->cby_index_low[x][y]);
      /* inverted output of each configuration bit */
      fprintf(fp, "  %s_outb[%d:%d] \n", 
              sram_verilog_model->prefix, 
              sram_verilog_model->cby_index_high[x][y] - 1,
              sram_verilog_model->cby_index_low[x][y]);
        
    }
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }
  fprintf(fp, ");\n");

  /* Comment lines */
  switch(chan_type) {
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
void dump_verilog_defined_connection_boxes(FILE* fp,
                                          t_ivec*** LL_rr_node_indices) {
  int ix, iy, chan_width;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* X - channels [1...nx][0..ny]*/
  for (iy = 0; iy < (ny + 1); iy++) {
    for (ix = 1; ix < (nx + 1); ix++) {
      chan_width = chan_width_x[iy];
      dump_verilog_defined_connection_box(fp, CHANX, ix, iy, chan_width, LL_rr_node_indices);
    }
  }
  /* Y - channels [1...ny][0..nx]*/
  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      chan_width = chan_width_y[ix];
      dump_verilog_defined_connection_box(fp, CHANY, ix, iy, chan_width, LL_rr_node_indices);
    }
  }
 
  return; 
}

/* Call the defined switch box sub-circuit
 * TODO: This function is also copied from
 * spice_routing.c : dump_verilog_routing_switch_box_subckt
 */
void dump_verilog_defined_switch_box(FILE* fp,
                                    int x, 
                                    int y) {
  int side, itrack;
  int* chan_width = (int*)my_malloc(sizeof(int)*4);

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 

  /* find Chan width for each side */
  for (side = 0; side < 4; side++) {
    switch (side) {
    case 0:
      /* For the bording, we should take special care */
      if (y == ny) { 
        chan_width[side] = 0;
        break;
      }
      chan_width[side] = chan_width_y[x];
      break;
    case 1:
      /* For the bording, we should take special care */
      if (x == nx) { 
        chan_width[side] = 0;
        break;
      }
      chan_width[side] = chan_width_x[y];
      break;
    case 2:
      /* For the bording, we should take special care */
      if (0 == y) { 
        chan_width[side] = 0;
        break;
      }
      chan_width[side] = chan_width_y[x];
      break;
    case 3:
      /* For the bording, we should take special care */
      if (0 == x) { 
        chan_width[side] = 0;
        break;
      }
      chan_width[side] = chan_width_x[y];
      break;
    default: 
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid side index.\n", 
                 __FILE__, __LINE__); 
      exit(1);
    } 
  } 
                 
  /* Comment lines */                 
  fprintf(fp, "//----- BEGIN call module Switch blocks [%d][%d] -----\n", x, y);
  /* Print module*/
  fprintf(fp, "sb_%d__%d_ ", x, y);
  fprintf(fp, "sb_%d__%d_ ", x, y);
  fprintf(fp, "(");
  /* 1. Channel Y [x][y+1] inputs */
  for (itrack = 0; itrack < chan_width[0]; itrack++) {
    fprintf(fp, "chany_%d__%d__in_%d_, ", x, y + 1, itrack);
    fprintf(fp, "\n");
  }
  /* 2. Channel X [x+1][y] inputs */
  for (itrack = 0; itrack < chan_width[1]; itrack++) {
    fprintf(fp, "chanx_%d__%d__in_%d_, ", x + 1, y, itrack);
    fprintf(fp, "\n");
  }
  /* 3. Channel Y [x][y] outputs */
  for (itrack = 0; itrack < chan_width[2]; itrack++) {
    fprintf(fp, "chany_%d__%d__out_%d_, ", x, y, itrack);
    fprintf(fp, "\n");
  }
  /* 4. Channel X [x][y] outputs */
  for (itrack = 0; itrack < chan_width[3]; itrack++) {
    fprintf(fp, "chanx_%d__%d__out_%d_, ", x, y, itrack);
    fprintf(fp, "\n");
  }

  /* Considering the border */
  if (ny != y) {
    /* 5. Grid[x][y+1] Right side outputs pins */
    dump_verilog_grid_side_pins(fp, OPIN, x, y+1, 1, FALSE);
  }
  if (0 != x) {
    /* 6. Grid[x][y+1] Bottom side outputs pins */
    dump_verilog_grid_side_pins(fp, OPIN, x, y+1, 2, FALSE);
  }

  if (ny != y) {
    /* 7. Grid[x+1][y+1] Left side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x+1, y+1, 3, FALSE);
  }
  if (nx != x) {
    /* 8. Grid[x+1][y+1] Bottom side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x+1, y+1, 2, FALSE);
  }

  if (nx != x) {
    /* 9. Grid[x+1][y] Top side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x+1, y, 0, FALSE);
  }
  if (0 != y) {
    /* 10. Grid[x+1][y] Left side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x+1, y, 3, FALSE);
  }

  if (0 != y) {
    /* 11. Grid[x][y] Right side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x, y, 1, FALSE);
  } 
  if (0 != x) {
    /* 12. Grid[x][y] Top side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x, y, 0, FALSE);
  }
  /* Print Input Pad and Output Pad */
  assert(!(0 > (inpad_verilog_model->sb_index_high[x][y] - inpad_verilog_model->sb_index_low[x][y])));
  if (0 < (inpad_verilog_model->sb_index_high[x][y] - inpad_verilog_model->sb_index_low[x][y])) {
    fprintf(fp, "  gfpga_input_%s[%d:%d], \n", 
            inpad_verilog_model->prefix, 
            inpad_verilog_model->sb_index_high[x][y] - 1, 
            inpad_verilog_model->sb_index_low[x][y]);
  }
  assert(!(0 > (outpad_verilog_model->sb_index_high[x][y] - outpad_verilog_model->sb_index_low[x][y])));
  if (0 < (outpad_verilog_model->sb_index_high[x][y] - outpad_verilog_model->sb_index_low[x][y])) {
    fprintf(fp, " gfpga_output_%s[%d:%d], \n", 
            outpad_verilog_model->prefix, 
            outpad_verilog_model->sb_index_high[x][y] - 1, 
            outpad_verilog_model->sb_index_low[x][y]);
  }
  /* Configuration ports */
  assert(!(0 > sram_verilog_model->sb_index_high[x][y] - sram_verilog_model->sb_index_low[x][y]));
  if (0 < sram_verilog_model->sb_index_high[x][y] - sram_verilog_model->sb_index_low[x][y]) {
    fprintf(fp, "  %s_out[%d:%d], \n", 
            sram_verilog_model->prefix, 
            sram_verilog_model->sb_index_high[x][y] - 1,
            sram_verilog_model->sb_index_low[x][y]);
    /* inverted output of each configuration bit */
    fprintf(fp, "  %s_outb[%d:%d] \n", 
            sram_verilog_model->prefix, 
            sram_verilog_model->sb_index_high[x][y] - 1,
            sram_verilog_model->sb_index_low[x][y]);
  }

  fprintf(fp, ");\n");

  /* Comment lines */                 
  fprintf(fp, "//----- END call module Switch blocks [%d][%d] -----\n\n", x, y);

  /* Free */
  my_free(chan_width);

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
      dump_verilog_defined_switch_box(fp, ix, iy);
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
void dump_verilog_configuration_circuits_memory_bank(FILE* fp) {
  int decoder_size = determine_decoder_size(sram_verilog_model->cnt);
  /* Check */
  assert(SPICE_SRAM_MEMORY_BANK == sram_verilog_orgz_type);

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!",__FILE__, __LINE__); 
    exit(1);
  } 

  /* Comment lines */
  fprintf(fp, "//----- BEGIN call decoders for memory bank controller -----\n");

  /* Dump Decoders for Bit lines and Word lines */
  /* Two huge decoders
   * TODO: divide to a number of small decoders ?
   */
  /* Bit lines decoder */
  fprintf(fp, "decoder%dto%d mem_bank_bl_decoder (",
          decoder_size, sram_verilog_model->cnt);
  fprintf(fp, "en_bl, addr_bl[%d:0], %s_out[%d:0] ",
          decoder_size - 1, sram_verilog_model->prefix, sram_verilog_model->cnt - 1);
  fprintf(fp, ");\n");

  /* Word lines decoder */
  fprintf(fp, "decoder%dto%d mem_bank_wl_decoder (",
          decoder_size, sram_verilog_model->cnt);
  fprintf(fp, "en_wl, addr_wl[%d:0], %s_outb[%d:0] ",
          decoder_size - 1, sram_verilog_model->prefix, sram_verilog_model->cnt - 1);
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
  switch(sram_verilog_orgz_type) {
  case SPICE_SRAM_STANDALONE:
    dump_verilog_configuration_circuits_standalone_srams(fp);
    break;
  case SPICE_SRAM_SCAN_CHAIN:
    dump_verilog_configuration_circuits_scan_chains(fp);
    break;
  case SPICE_SRAM_MEMORY_BANK:
    dump_verilog_configuration_circuits_memory_bank(fp);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }
  return;
}

/***** Print Top-level SPICE netlist *****/
void dump_verilog_top_netlist(char* circuit_name,
                              char* top_netlist_name,
                              char* include_dir_path,
                              char* subckt_dir_path,
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
  init_include_user_defined_netlists(verilog);
  //dump_verilog_include_user_defined_netlists(fp, verilog);
  
  /* Special subckts for Top-level SPICE netlist */
  fprintf(fp, "//----- Include subckt netlists: Look-Up Tables (LUTs) -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, luts_verilog_file_name);
  fprintf(fp, "//`include \"%s\"\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "//------ Include subckt netlists: Logic Blocks -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, logic_block_verilog_file_name);
  fprintf(fp, "//`include \"%s\"\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "//----- Include subckt netlists: Routing structures (Switch Boxes, Channels, Connection Boxes) -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, routing_verilog_file_name);
  fprintf(fp, "//`include \"%s\"\n", temp_include_file_path);
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
    fprintf(fp, "//`include \"%s\"\n", temp_include_file_path);
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
  dump_verilog_defined_channels(fp);

  /* Quote Routing structures: Conneciton Boxes */
  dump_verilog_defined_connection_boxes(fp, LL_rr_node_indices);
  
  /* Quote Routing structures: Switch Boxes */
  dump_verilog_defined_switch_boxes(fp); 

  /* Dump configuration circuits */
  dump_verilog_configuration_circuits(fp);

  /* SPICE ends*/
  fprintf(fp, "endmodule\n");

  /* Close the file*/
  fclose(fp);

  return;
}

