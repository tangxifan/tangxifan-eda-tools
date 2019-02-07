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
#include "fpga_x2p_globals.h"

/* Include Verilog support headers*/
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_routing.h"

void dump_verilog_sdc_file_header(FILE* fp,
                                  char* usage) {
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d]) FileHandle is NULL!\n",__FILE__,__LINE__); 
    exit(1);
  } 
  fprintf(fp,"#############################################\n");
  fprintf(fp,"#     Synopsys Design Constraints (SDC)    # \n");
  fprintf(fp,"#    FPGA Synthesizable Verilog Netlist    # \n");
  fprintf(fp,"#    Description: %s \n",usage);
  fprintf(fp,"#           Author: Xifan TANG             # \n");
  fprintf(fp,"#        Organization: EPFL/IC/LSI         # \n");
  fprintf(fp,"#    Date: %s \n", my_gettime());
  fprintf(fp,"#############################################\n");
  fprintf(fp,"\n");

  return;
}

/* Report timing for a routing wire,
 * Support uni-directional routing architecture
 * Each routing wire start from an OPIN 
 * We check each fan-out to find all possible ending point:
 * An ending point is supposed to be an OPIN or CHANX or CHANY
 */
void verilog_generate_one_routing_wire_report_timing(FILE* fp, 
                                                     t_sb* cur_sb_info,
                                                     t_rr_node* wire_rr_node,
                                                     int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                                     t_ivec*** LL_rr_node_indices) {
  int iedge, itrack, inode, pin_side;
  int next_sb_x, next_sb_y;

  /* Find the starting points */
  for (iedge = 0; iedge < wire_rr_node->num_drive_rr_nodes; iedge++) {
    /* Driver could be OPIN or CHANX or CHANY,
     * and it must be in the cur_sb_info
     */
    /* Get the top-level pin name and print it out */

    /* Find the ending points*/
    for (iedge = 0; iedge < wire_rr_node->num_edges; iedge++) {
      /* Reciever could be IPIN or CHANX or CHANY */
      inode = wire_rr_node->edges[iedge];
      /* Find the SB/CB block that it belongs to */
      switch (LL_rr_node[inode].type) {
      case IPIN:
        /* IPIN indicates an CLB input */
        /* Get the top-level pin name and print it out */
        /* Print each INPUT Pins of a grid */
        dump_verilog_grid_side_pin_with_given_index(fp, OPIN,
                                                    LL_rr_node[inode].ptc_num,
                                                    pin_side,
                                                    LL_rr_node[inode].xlow,
                                                    LL_rr_node[inode].ylow, 
                                                    FALSE); /* Do not specify direction of port */
  
        break;
      case CHANX:
      case CHANY:
        /* xlow and ylow should be the coordinate of the ending SB */
        /* the ending rr_node should be an output of the ending SB*/
        /* find the wire_rr_node in the ending SB*/
        dump_verilog_routing_channel_one_pin(fp, &(LL_rr_node[inode]),
                                             next_sb_x, next_sb_y, itrack, 
                                             IN_PORT);
 
        break;
      default:
       vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid type of ending point rr_node!\n",
                   __FILE__, __LINE__);
 
        exit(1);
      }
      /* Get the user-constrained delay of this routing wire */
      /* Find the pins/ports of SBs that this wire may across */
      /* Output the Report Timing commands */
    }
  }

  return;
}

/* Generate report timing for each routing wires/segments */
void verilog_generate_routing_wires_report_timing(FILE* fp, 
                                                  int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                                  t_ivec*** LL_rr_node_indices) {
  int ix, iy;
  int side, itrack;
  t_sb* cur_sb_info = NULL;

  /* Create a file*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "(FILE:%s,LINE[%d])Invalid file handler for SDC generation",
               __FILE__, __LINE__); 
    exit(1);
  } 

  /* We start from a SB[x][y] */
  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 0; iy < (ny + 1); iy++) {
      cur_sb_info = &(sb_info[ix][iy]);
      for (side = 0; side < cur_sb_info->num_sides; side++) {
        for (itrack = 0; itrack < cur_sb_info->chan_width[side]; itrack++) {
          assert((CHANX == cur_sb_info->chan_rr_node[side][itrack]->type)
               ||(CHANY == cur_sb_info->chan_rr_node[side][itrack]->type));
          /* We only care the output port and it should indicate a SB mux */
          if ( (OUT_PORT != cur_sb_info->chan_rr_node_direction[side][itrack]) 
             || (FALSE != check_drive_rr_node_imply_short(*cur_sb_info, cur_sb_info->chan_rr_node[side][itrack], side))) {
            continue; 
          }
          /* Bypass if we have only 1 driving node */
          if (1 == cur_sb_info->chan_rr_node[side][itrack]->num_drive_rr_nodes) {
            continue; 
          }
          /* Reach here, it means a valid starting point of a routing wire */
          verilog_generate_one_routing_wire_report_timing(fp, &(sb_info[ix][iy]),
                                                          cur_sb_info->chan_rr_node[side][itrack],
                                                          LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);
        }
      } 
    }
  }

  return;
}

/* Output a log file to guide routing report_timing */
void verilog_generate_routing_report_timing(t_sram_orgz_info* cur_sram_orgz_info,
                                            char* sdc_dir,
                                            t_arch arch,
                                            t_det_routing_arch* routing_arch,
                                            int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                            t_ivec*** LL_rr_node_indices,
                                            t_syn_verilog_opts fpga_verilog_opts) {
  char* sdc_fname = NULL;
  FILE* fp = NULL;
 
  /* Create the file handler */
  sdc_fname = my_strcat(format_dir_path(sdc_dir), routing_sdc_file_name);

  /* Create a file*/
  fp = fopen(sdc_fname, "w");

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "(FILE:%s,LINE[%d])Failure in create SDC constraints %s",
               __FILE__, __LINE__, sdc_fname); 
    exit(1);
  } 

  /* Generate SDC header */
  dump_verilog_sdc_file_header(fp, "Report Timing for Global Routing");

  /* Part 1. Output routing constraints for routing tracks */

  /* Part 2. Output routing constraints for Switch Blocks */

  /* Part 3. Output routing constraints for Connection Blocks */

  /* Part 4. Report timing for routing wires */
  verilog_generate_routing_wires_report_timing(fp, LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);

  /* close file*/
  fclose(fp);

  return;
}

