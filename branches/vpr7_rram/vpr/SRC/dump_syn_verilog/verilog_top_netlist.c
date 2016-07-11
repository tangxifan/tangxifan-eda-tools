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
#include "verilog_pbtypes.h"
#include "verilog_routing.h"
#include "verilog_top_netlist.h"

/* Local Subroutines declaration */
static 
void dump_verilog_top_netlist_global_ports(FILE* fp,
                                     int num_clock,
                                     t_spice verilog);

/******** Subroutines ***********/
static 
void dump_verilog_top_netlist_global_ports(FILE* fp,
                                     int num_clock,
                                     t_spice verilog) {

  /* A valid file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprintf(fp, ".global gset greset\n");

  /*Global ports for INPUTs of I/O PADS, SRAMs */
  //dump_verilog_global_pad_ports_verilog_model(fp, verilog);


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
      fprintf(fp, "grid[%d][%d]  ", ix, iy); /* Call the name of subckt */ 
      fprintf(fp, "grid[%d][%d] ", ix, iy);
    fprintf(fp, "(");
      dump_verilog_grid_pins(fp, ix, iy, 1, FALSE);
      fprintf(fp, ");\n");
    }
  } 

  /* IO Grids */
  /* LEFT side */
  ix = 0;
  for (iy = 1; iy < (ny + 1); iy++) {
    assert(IO_TYPE == grid[ix][iy].type);
    fprintf(fp, "grid[%d][%d] ", ix, iy); /* Call the name of subckt */ 
    fprintf(fp, "grid[%d][%d] ", ix, iy);
    fprintf(fp, "(");
    dump_verilog_io_grid_pins(fp, ix, iy, 1, FALSE);
    fprintf(fp, ");\n");
  }

  /* RIGHT side */
  ix = nx + 1;
  for (iy = 1; iy < (ny + 1); iy++) {
    assert(IO_TYPE == grid[ix][iy].type);
    fprintf(fp, "grid[%d][%d] ", ix, iy); /* Call the name of subckt */ 
    fprintf(fp, "grid[%d][%d] ", ix, iy);
    fprintf(fp, "(");
    dump_verilog_io_grid_pins(fp, ix, iy, 1, FALSE);
    fprintf(fp, ");\n");
  }

  /* BOTTOM side */
  iy = 0;
  for (ix = 1; ix < (nx + 1); ix++) {
    assert(IO_TYPE == grid[ix][iy].type);
    fprintf(fp, "grid[%d][%d] ", ix, iy); /* Call the name of subckt */ 
    fprintf(fp, "grid[%d][%d] ", ix, iy);
    fprintf(fp, "(");
    dump_verilog_io_grid_pins(fp, ix, iy, 1, FALSE);
    fprintf(fp, ");\n");
  } 

  /* TOP side */
  iy = ny + 1;
  for (ix = 1; ix < (nx + 1); ix++) {
    assert(IO_TYPE == grid[ix][iy].type);
    fprintf(fp, "grid[%d][%d] ", ix, iy); /* Call the name of subckt */ 
    fprintf(fp, "grid[%d][%d] ", ix, iy);
    fprintf(fp, "(");
    dump_verilog_io_grid_pins(fp, ix, iy, 1, FALSE);
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
    /* Call the define sub-circuit */
    fprintf(fp, "chanx[%d][%d] ", x, y);
    fprintf(fp, "chanx[%d][%d] ", x, y);
    fprintf(fp, "(");
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chanx[%d][%d]_in[%d], ", x, y, itrack);
      fprintf(fp, "\n");
    }
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chanx[%d][%d]_out[%d], ", x, y, itrack);
      fprintf(fp, "\n");
    }
    /* output at middle point */
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chanx[%d][%d]_midout[%d], ", x, y, itrack);
      fprintf(fp, "\n");
    }
    fprintf(fp, ");\n");
    break;
  case CHANY:
    /* check x*/
    assert((!(0 > x))&&(x < (nx + 1))); 
    /* check y*/
    assert((0 < y)&&(y < (ny + 1))); 
    /* Call the define sub-circuit */
    fprintf(fp, "chany[%d][%d] ", x, y);
    fprintf(fp, "chany[%d][%d] ", x, y);
    fprintf(fp, "(");
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chany[%d][%d]_in[%d], ", x, y, itrack);
      fprintf(fp, "\n");
    }
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chany[%d][%d]_out[%d], ", x, y, itrack);
      fprintf(fp, "\n");
    }
    /* output at middle point */
    for (itrack = 0; itrack < chan_width; itrack++) {
      fprintf(fp, "chany[%d][%d]_midout[%d], ", x, y, itrack);
      fprintf(fp, "\n");
    }
    fprintf(fp, ");\n");
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
    fprintf(fp, "cbx[%d][%d] ", x, y);
    fprintf(fp, "cbx[%d][%d] ", x, y);
    break;
  case CHANY:
    fprintf(fp, "cby[%d][%d] ", x, y);
    fprintf(fp, "cby[%d][%d] ", x, y);
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
      fprintf(fp, "chanx[%d][%d]_midout[%d], ", x, y, itrack);
      break;
    case CHANY:
      fprintf(fp, "chany[%d][%d]_midout[%d], ", x, y, itrack);
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
                                  
  fprintf(fp, "sb[%d][%d] ", x, y);
  fprintf(fp, "sb[%d][%d] ", x, y);
  fprintf(fp, "(");
  /* 1. Channel Y [x][y+1] inputs */
  for (itrack = 0; itrack < chan_width[0]; itrack++) {
    fprintf(fp, "chany[%d][%d]_in[%d], ", x, y + 1, itrack);
    fprintf(fp, "\n");
  }
  /* 2. Channel X [x+1][y] inputs */
  for (itrack = 0; itrack < chan_width[1]; itrack++) {
    fprintf(fp, "chanx[%d][%d]_in[%d], ", x + 1, y, itrack);
    fprintf(fp, "\n");
  }
  /* 3. Channel Y [x][y] outputs */
  for (itrack = 0; itrack < chan_width[2]; itrack++) {
    fprintf(fp, "chany[%d][%d]_out[%d], ", x, y, itrack);
    fprintf(fp, "\n");
  }
  /* 4. Channel X [x][y] outputs */
  for (itrack = 0; itrack < chan_width[3]; itrack++) {
    fprintf(fp, "chanx[%d][%d]_out[%d], ", x, y, itrack);
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

  fprintf(fp, ");\n");

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
  t_llist* temp = NULL;

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
  fprintf(fp, "`include \'%s\'\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "//------ Include subckt netlists: Logic Blocks -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, logic_block_verilog_file_name);
  fprintf(fp, "`include \'%s\'\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "//----- Include subckt netlists: Routing structures (Switch Boxes, Channels, Connection Boxes) -----\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, routing_verilog_file_name);
  fprintf(fp, "`include \'%s\'\n", temp_include_file_path);
  my_free(temp_include_file_path);
 
  /* Print all global wires*/
  dump_verilog_top_netlist_global_ports(fp, num_clock, verilog);

  /* Quote defined Logic blocks subckts (Grids) */
  dump_verilog_defined_grids(fp);

  /* Quote Routing structures: Channels */
  dump_verilog_defined_channels(fp);

  /* Quote Routing structures: Conneciton Boxes */
  dump_verilog_defined_connection_boxes(fp, LL_rr_node_indices);
  
  /* Quote Routing structures: Switch Boxes */
  dump_verilog_defined_switch_boxes(fp); 

  /* SPICE ends*/
  fprintf(fp, "endmodule\n");

  /* Close the file*/
  fclose(fp);

  return;
}

