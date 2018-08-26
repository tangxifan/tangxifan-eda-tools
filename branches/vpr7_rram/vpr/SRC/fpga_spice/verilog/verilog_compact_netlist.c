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

/* Include SPICE support headers*/
#include "linkedlist.h"
#include "fpga_spice_utils.h"
#include "fpga_spice_mux_utils.h"
#include "fpga_spice_pbtypes_utils.h"
#include "fpga_spice_bitstream_utils.h"
#include "spice_mux.h"
#include "fpga_spice_globals.h"

/* Include Synthesizable Verilog headers */
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_primitives.h"
#include "verilog_pbtypes.h"
#include "verilog_top_netlist.h"
#include "verilog_compact_netlist_utils.h"

/* Create a Verilog file and dump a module consisting of a I/O block,
 * The pins appear in the port list will depend on the selected border side
 */
void dump_compact_verilog_one_physical_block(t_sram_orgz_info* cur_sram_orgz_info, 
                                             char* subckt_dir_path,
                                             t_type_ptr phy_block_type,
                                             int border_side,
                                             t_arch* arch) {
  int iz;
  int temp_reserved_conf_bits_msb;
  int temp_iopad_lsb, temp_iopad_msb;
  int temp_conf_bits_lsb, temp_conf_bits_msb;
  char* fname = NULL;  
  FILE* fp = NULL;
  char* title = my_strcat("FPGA Verilog Netlist for Design: ", phy_block_type->name);

  /* Check */
  if (IO_TYPE == phy_block_type) {
    assert( (-1 < border_side) && (border_side < 4));
  }

  /* Give a name to the Verilog netlist */
  fname = my_strcat(format_dir_path(subckt_dir_path), phy_block_type->name);
  fname = my_strcat(fname, verilog_netlist_file_postfix); 

  /* Create file handler */
  fp = fopen(fname, "w");
  /* Check if the path exists*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create top Verilog netlist %s!",__FILE__, __LINE__, fname); 
    exit(1);
  } 

  vpr_printf(TIO_MESSAGE_INFO, "Writing FPGA Verilog Netlist for logic block %s...\n",
             phy_block_type->name);

  /* Print the title */
  dump_verilog_file_header(fp, title);
  my_free(title);

  /* Dump all the submodules */
  for (iz = 0; iz < phy_block_type->capacity; iz++) {
    /* Comments: Grid [x][y]*/
    fprintf(fp, "//----- Submodule of type_descriptor: %s[%d] -----\n", phy_block_type->name, iz);
    /* Print a NULL logic block...*/
    dump_verilog_phy_pb_graph_node_rec(cur_sram_orgz_info, fp, phy_block_type->name, NULL, 
                                       phy_block_type->pb_graph_head, iz);
    fprintf(fp, "//----- END -----\n\n");
  }

  /* Create top module and call all the defined submodule */
  fprintf(fp, "//----- %s, Capactity: %d -----\n", phy_block_type->name, phy_block_type->capacity);
  fprintf(fp, "//----- Top Protocol -----\n");
  /* Definition */
  fprintf(fp, "module grid_%s_( \n", phy_block_type->name);
  fprintf(fp, "\n");
  /* dump global ports */
  if (0 < dump_verilog_global_ports(fp, global_ports_head, TRUE)) {
    fprintf(fp, ",\n");
  }

  /* Pins */
  if (IO_TYPE == phy_block_type) { 
    /* Generate a fake (x,y) coordinate that can be used for print pin names */
    /* verilog_compact_generate_fake_xy_for_io_border_side(border_side, &ix, &iy); */
    /* Special Care for I/O grid */
    dump_compact_verilog_io_grid_pins(fp, phy_block_type, border_side, TRUE, FALSE);
  } else {
    dump_compact_verilog_grid_pins(fp, phy_block_type, TRUE, FALSE);
  }

  /* I/O PAD */
  dump_verilog_grid_common_port(fp, iopad_verilog_model, gio_inout_prefix, 
                                0, phy_block_type->capacity - 1,
                                VERILOG_PORT_INPUT); 

  /* Print configuration ports */
  /* Reserved configuration ports */
  temp_reserved_conf_bits_msb = phy_block_type->pb_type->physical_mode_num_reserved_conf_bits; 
  if (0 < temp_reserved_conf_bits_msb) { 
    fprintf(fp, ",\n");
    dump_verilog_reserved_sram_ports(fp, cur_sram_orgz_info,
                                     0, temp_reserved_conf_bits_msb - 1,
                                     VERILOG_PORT_INPUT); 
  }
  /* Normal configuration ports */
  temp_conf_bits_msb = phy_block_type->capacity * phy_block_type->pb_type->physical_mode_num_conf_bits; 
  /* Get the number of sram bits in this submodule!!! */
  if (0 < temp_conf_bits_msb) { 
    fprintf(fp, ",\n");
    dump_verilog_sram_ports(fp, cur_sram_orgz_info,
                            0, temp_conf_bits_msb - 1, 
                            VERILOG_PORT_INPUT); 
  }
  fprintf(fp, ");\n");

  /* Initialize temporary counter */
  temp_conf_bits_lsb = 0;
  temp_iopad_lsb = 0;

  /* Quote all the sub blocks*/
  for (iz = 0; iz < phy_block_type->capacity; iz++) {
    /* Local Vdd and Gnd, subckt name*/
    fprintf(fp, "%s ", compact_verilog_get_grid_phy_block_subckt_name(phy_block_type, iz, phy_block_type->name));
    fprintf(fp, " grid_%s_%d_ (", phy_block_type->name, iz);
    fprintf(fp, "\n");
    /* dump global ports */
    if (0 < dump_verilog_global_ports(fp, global_ports_head, FALSE)) {
      fprintf(fp, ",\n");
    }
    /* Print all the pins */
    /* Special Care for I/O grid */
    if (IO_TYPE == phy_block_type) { 
      dump_compact_verilog_io_grid_block_subckt_pins(fp, phy_block_type, border_side, iz);
    } else {
      dump_verilog_grid_block_subckt_pins(fp, iz, phy_block_type);
    }

    /* Print configuration ports */
    temp_reserved_conf_bits_msb = phy_block_type->pb_type->physical_mode_num_reserved_conf_bits; 
    temp_conf_bits_msb = temp_conf_bits_lsb + phy_block_type->pb_type->physical_mode_num_conf_bits;
    temp_iopad_msb = temp_iopad_lsb + phy_block_type->pb_type->physical_mode_num_iopads;

    /* Print Input Pad and Output Pad */
    fprintf(fp, "\n//---- IOPAD ----\n");
    dump_verilog_grid_common_port(fp, iopad_verilog_model, gio_inout_prefix,
                                  temp_iopad_lsb,  temp_iopad_msb - 1,
                                  VERILOG_PORT_CONKT); 
    /* Reserved configuration ports */
    if (0 < temp_reserved_conf_bits_msb) { 
      fprintf(fp, ",\n");
      dump_verilog_reserved_sram_ports(fp, cur_sram_orgz_info,
                                       0, temp_reserved_conf_bits_msb - 1,
                                       VERILOG_PORT_CONKT); 
    }
    /* Normal configuration ports */
    if (0 < (temp_conf_bits_msb - temp_conf_bits_lsb)) { 
      fprintf(fp, ",\n");
      fprintf(fp, "//---- SRAM ----\n");
      dump_verilog_sram_ports(fp, cur_sram_orgz_info,
                              temp_conf_bits_lsb, temp_conf_bits_msb - 1, 
                              VERILOG_PORT_CONKT); 
    }
    /* Update temp_sram_lsb */
    temp_conf_bits_lsb = temp_conf_bits_msb;
    temp_iopad_lsb = temp_iopad_msb;
    fprintf(fp, ");\n");
  }

  fprintf(fp, "endmodule\n");
  fprintf(fp, "//----- END Top Protocol -----\n");
  fprintf(fp, "//----- END Grid %s, Capactity: %d -----\n\n", phy_block_type->name, phy_block_type->capacity);

  /* Check flags */
  assert( temp_conf_bits_msb == phy_block_type->capacity * phy_block_type->pb_type->physical_mode_num_conf_bits ); 

  /* Close file handler */
  fclose(fp); 

  /* Free */
  my_free(fname);

  return;
}

/** Create logic block modules in a compact way:
 * 1. Only one module for each I/O on each border side (IO_TYPE)
 * 2. Only one module for each CLB (FILL_TYPE)
 * 3. Only one module for each heterogeneous block
 */
void dump_compact_verilog_logic_blocks(t_sram_orgz_info* cur_sram_orgz_info,
                                       char* subckt_dir,
                                       t_arch* arch) {
  int itype, iside, num_sides;
 
  num_sides = 4;

  /* Enumerate the types, dump one Verilog module for each */
  for (itype = 0; itype > num_types; itype++) {
    if (EMPTY_TYPE == &type_descriptors[itype]) {
    /* Bypass empty type or NULL */
      continue;
    } else if (IO_TYPE == &type_descriptors[itype]) {
    /* Special for I/O block, generate one module for each border side */
      for (iside = 0; iside < num_sides; iside++) {
        dump_compact_verilog_one_physical_block(cur_sram_orgz_info, subckt_dir, 
                                                &type_descriptors[itype], iside, arch);
      } 
      continue;
    } else if (FILL_TYPE == &type_descriptors[itype]) {
    /* For CLB */
      dump_compact_verilog_one_physical_block(cur_sram_orgz_info, subckt_dir, 
                                              &type_descriptors[itype], -1, arch);
      continue;
    } else {
    /* For heterogenenous blocks */
      dump_compact_verilog_one_physical_block(cur_sram_orgz_info, subckt_dir, 
                                              &type_descriptors[itype], -1, arch);

    }
  }

  return;
}

/* Call defined grid 
 * Instance unique submodules (I/O, CLB, Heterogeneous block) for the full grids
 */
static 
void dump_compact_verilog_defined_one_grid(t_sram_orgz_info* cur_sram_orgz_info,
                                           FILE* fp,
                                           int ix, int iy) {
   
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }
  
  if ((NULL == grid[ix][iy].type)
    || (EMPTY_TYPE == grid[ix][iy].type) 
    ||(0 != grid[ix][iy].offset)) {
    return;
  }
 
  /* Comment lines */
  fprintf(fp, "//----- BEGIN Call Grid[%d][%d] module -----\n", ix, iy);
  /* Print the Grid module */
  fprintf(fp, "grid_%s  ", grid[ix][iy].type->name); /* Call the name of subckt */ 
  fprintf(fp, "grid_%d__%d_ ", ix, iy);
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
 
  /* IO PAD */
  dump_verilog_grid_common_port(fp, iopad_verilog_model, gio_inout_prefix, 
                                iopad_verilog_model->grid_index_low[ix][iy],
                                iopad_verilog_model->grid_index_high[ix][iy] - 1,
                                VERILOG_PORT_CONKT); 

  /* Print configuration ports */
  /* Reserved configuration ports */
  if (0 < cur_sram_orgz_info->grid_reserved_conf_bits[ix][iy]) {
    fprintf(fp, ",\n");
  }
  dump_verilog_reserved_sram_ports(fp, cur_sram_orgz_info,
                                   0, 
                                   cur_sram_orgz_info->grid_reserved_conf_bits[ix][iy] - 1,
                                   VERILOG_PORT_CONKT);
  /* Normal configuration ports */
  if (0 < (cur_sram_orgz_info->grid_conf_bits_msb[ix][iy]
           - cur_sram_orgz_info->grid_conf_bits_lsb[ix][iy])) {
    fprintf(fp, ",\n");
  }
  dump_verilog_sram_ports(fp, cur_sram_orgz_info,
                          cur_sram_orgz_info->grid_conf_bits_lsb[ix][iy],
                          cur_sram_orgz_info->grid_conf_bits_msb[ix][iy] - 1,
                          VERILOG_PORT_CONKT);
  fprintf(fp, ");\n");
  /* Comment lines */
  fprintf(fp, "//----- END call Grid[%d][%d] module -----\n\n", ix, iy);

  return;
}

/* Call defined grid 
 * Instance unique submodules (I/O, CLB, Heterogeneous block) for the full grids
 */
void dump_compact_verilog_defined_grids(t_sram_orgz_info* cur_sram_orgz_info,
                                        FILE* fp) {
  int ix, iy;

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Normal Grids */
  for (ix = 1; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      /* Bypass EMPTY grid */
      if (EMPTY_TYPE == grid[ix][iy].type) {
        continue;
      }
      assert(IO_TYPE != grid[ix][iy].type);
      dump_compact_verilog_defined_one_grid(cur_sram_orgz_info, fp, ix, iy);
    }
  } 

  /* IO Grids */
  /* LEFT side */
  ix = 0;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Bypass EMPTY grid */
    if (EMPTY_TYPE == grid[ix][iy].type) {
      continue;
    }
    assert(IO_TYPE == grid[ix][iy].type);
    dump_compact_verilog_defined_one_grid(cur_sram_orgz_info, fp, ix, iy);
  }

  /* RIGHT side */
  ix = nx + 1;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Bypass EMPTY grid */
    if (EMPTY_TYPE == grid[ix][iy].type) {
      continue;
    }
    assert(IO_TYPE == grid[ix][iy].type);
    dump_compact_verilog_defined_one_grid(cur_sram_orgz_info, fp, ix, iy);
  }

  /* BOTTOM side */
  iy = 0;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Bypass EMPTY grid */
    if (EMPTY_TYPE == grid[ix][iy].type) {
      continue;
    }
    assert(IO_TYPE == grid[ix][iy].type);
    dump_compact_verilog_defined_one_grid(cur_sram_orgz_info, fp, ix, iy);
  } 

  /* TOP side */
  iy = ny + 1;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Bypass EMPTY grid */
    if (EMPTY_TYPE == grid[ix][iy].type) {
      continue;
    }
    assert(IO_TYPE == grid[ix][iy].type);
    dump_compact_verilog_defined_one_grid(cur_sram_orgz_info, fp, ix, iy);
  } 

  return;
}

/** Print Top-level SPICE netlist in a compact way
 * Instance unique submodules (I/O, CLB, Heterogeneous block) for the full grids
 */
void dump_compact_verilog_top_netlist(t_sram_orgz_info* cur_sram_orgz_info,
                                      char* circuit_name,
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
  switch(cur_sram_orgz_info->type) {
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
  dump_verilog_top_netlist_ports(cur_sram_orgz_info, fp, num_clock, circuit_name, verilog);

  dump_verilog_top_netlist_internal_wires(cur_sram_orgz_info, fp);

  /* Quote defined Logic blocks subckts (Grids) */
  dump_compact_verilog_defined_grids(cur_sram_orgz_info, fp);

  /* Quote Routing structures: Channels */
  dump_verilog_defined_channels(fp, LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);

  /* Quote Routing structures: Connection Boxes */
  dump_verilog_defined_connection_boxes(cur_sram_orgz_info, fp); 
  
  /* Quote Routing structures: Switch Boxes */
  dump_verilog_defined_switch_boxes(cur_sram_orgz_info, fp); 

  /* Apply CLB to CLB direct connections */
  dump_verilog_clb2clb_directs(fp, num_clb2clb_directs, clb2clb_direct);

  /* Dump configuration circuits */
  dump_verilog_configuration_circuits(cur_sram_orgz_info, fp);

  /* verilog ends*/
  fprintf(fp, "endmodule\n");

  /* Close the file*/
  fclose(fp);

  return;
}


