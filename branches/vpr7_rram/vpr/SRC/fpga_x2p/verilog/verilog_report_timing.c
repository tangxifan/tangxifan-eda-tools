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
#include "fpga_x2p_rr_graph_utils.h"
#include "fpga_x2p_globals.h"

/* Include Verilog support headers*/
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_routing.h"
#include "verilog_tcl_utils.h"

/* options for report timing */
typedef struct s_trpt_opts t_trpt_opts;
struct s_trpt_opts {
  char* sdc_dir;
  boolean longest_path_only;
  boolean report_pb_timing;
  boolean report_cb_timing;
  boolean report_sb_timing;
  boolean report_routing_timing;
  boolean print_thru_pins;
};

/* Print the pins of SBs that a routing wire will go through 
 * from the src_rr_node to the des_rr_node 
 */
void dump_verilog_sb_through_routing_pins(FILE* fp,
                                          t_sb* src_sb_info,
                                          t_rr_node* src_rr_node, 
                                          t_rr_node* des_rr_node) {
  int ix, iy;
  int cur_sb_x, cur_sb_y;
  int end_sb_x, end_sb_y;
  t_cb* next_cb;
  t_sb* next_sb;

  /* Check the file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "(FILE:%s,LINE[%d])Invalid file handler for SDC generation",
               __FILE__, __LINE__); 
    exit(1);
  } 

  /* Check */
  assert ((INC_DIRECTION == src_rr_node->direction)
        ||(DEC_DIRECTION == src_rr_node->direction));
  assert ((CHANX == src_rr_node->type)
        ||(CHANY == src_rr_node->type));

  /* Switch depends on the type of des_rr_node  */
  switch(des_rr_node->type) {
  /* Range of SBs that on the path                        
   *                             ---------
   *                            |         |        
   *                            |  des_sb |
   *                            | [x][y]  |        
   *                             ---------
   *                                /|\ 
   *                                 |  
   *                             ---------
   *                            |         |        
   *                            | thru_cb |
   *                            |         |        
   *                             ---------
   *                                /|\ 
   *                                 |  
   *  --------      -------      ---------      -------      --------
   * |        |    |       |    |         |    |       |    |        |
   * | des_sb |<---|thru_cb|<---| src_sb  |--->|thru_cb|--->| des_sb |
   * |[x-1][y]|    | [x][y]|    |         |    | [x][y]|    |[x][y]  |
   *  --------      -------      ---------      -------      --------
   *                                 |
   *                                \|/
   *                             ---------
   *                            |         |        
   *                            | thru_cb |
   *                            |         |        
   *                             ---------
   *                                 |
   *                                \|/
   *                             ---------
   *                            |         |        
   *                            |  des_sb |
   *                            | [x][y-1]|        
   *                             ---------
   */
  case IPIN: 
    /* Get the coordinate of ending CB */
    next_cb = get_chan_rr_node_ending_cb(src_rr_node, des_rr_node);
    assert(next_cb->type == src_rr_node->type);
    /* 4 cases: */
    if ((INC_DIRECTION == src_rr_node->direction) 
       &&(CHANX == src_rr_node->type)) {
      end_sb_x = next_cb->x; 
      end_sb_y = next_cb->y;
    } else if ((INC_DIRECTION == src_rr_node->direction) 
       &&(CHANY == src_rr_node->type)) {
      end_sb_x = next_cb->x; 
      end_sb_y = next_cb->y;
    } else if ((DEC_DIRECTION == src_rr_node->direction) 
       &&(CHANX == src_rr_node->type)) {
      end_sb_x = next_cb->x - 1; 
      end_sb_y = next_cb->y;
    } else if ((DEC_DIRECTION == src_rr_node->direction) 
       &&(CHANY == src_rr_node->type)) {
      end_sb_x = next_cb->x; 
      end_sb_y = next_cb->y - 1;
    }
    break;
  /* Range of SBs that on the path                        
   *                             ---------
   *                            |         |        
   *                            |  des_sb |
   *                            | [x][y+1]|        
   *                             ---------
   *                                /|\ 
   *                                 |  
   *                             ---------
   *                            |         |        
   *                            | thru_sb |
   *                            |         |        
   *                             ---------
   *                                /|\ 
   *                                 |  
   *  --------      -------      ---------      -------      --------
   * |        |    |       |    |         |    |       |    |        |
   * | des_sb |<---|thru_sb|<---| src_sb  |--->|thru_sb|--->| des_sb |
   * |[x-1][y]|    | [x][y]|    |         |    | [x][y]|    |[x+1][y]|
   *  --------      -------      ---------      -------      --------
   *                                 |
   *                                \|/
   *                             ---------
   *                            |         |        
   *                            | thru_sb |
   *                            |         |        
   *                             ---------
   *                                 |
   *                                \|/
   *                             ---------
   *                            |         |        
   *                            |  des_sb |
   *                            | [x][y-1]|        
   *                             ---------
   */
  case CHANX:
  case CHANY:
    /* Get the coordinate of ending CB */
    next_sb = get_chan_rr_node_ending_sb(src_rr_node, des_rr_node);
    end_sb_x = next_sb->x; 
    end_sb_y = next_sb->y;
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid type of rr_node!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Get the base coordinate of src_sb */
  cur_sb_x = src_sb_info->x;
  cur_sb_y = src_sb_info->y;
  /* 4 cases: */
  if ((INC_DIRECTION == src_rr_node->direction) 
     &&(CHANX == src_rr_node->type)) {
    /* Follow the graph above, go through X channel */
    for (ix = src_sb_info->x + 1; ix < end_sb_x; ix++) {
      /* Print an IN_PORT*/ 
      fprintf(fp, " ");
      /* output instance name */
      fprintf(fp, "%s/", 
              gen_verilog_one_sb_instance_name(&sb_info[ix][cur_sb_y])); 
      dump_verilog_one_sb_chan_pin(fp, &(sb_info[ix][cur_sb_y]), src_rr_node, IN_PORT); 
      /* Print an OUT_PORT*/ 
      fprintf(fp, " ");
      /* output instance name */
      fprintf(fp, "%s/", 
              gen_verilog_one_sb_instance_name(&sb_info[ix][cur_sb_y])); 
      dump_verilog_one_sb_chan_pin(fp, &(sb_info[ix][cur_sb_y]), src_rr_node, OUT_PORT); 
    }
  } else if ((INC_DIRECTION == src_rr_node->direction) 
     &&(CHANY == src_rr_node->type)) {
    /* Follow the graph above, go through Y channel */
    for (iy = src_sb_info->y + 1; iy < end_sb_y; iy++) {
      /* Print an IN_PORT*/ 
      fprintf(fp, " ");
      /* output instance name */
      fprintf(fp, "%s/", 
              gen_verilog_one_sb_instance_name(&sb_info[cur_sb_x][iy])); 
      dump_verilog_one_sb_chan_pin(fp, &(sb_info[cur_sb_x][iy]), src_rr_node, IN_PORT); 
      /* Print an OUT_PORT*/ 
      fprintf(fp, " ");
      /* output instance name */
      fprintf(fp, "%s/", 
              gen_verilog_one_sb_instance_name(&sb_info[cur_sb_x][iy])); 
      dump_verilog_one_sb_chan_pin(fp, &(sb_info[cur_sb_x][iy]), src_rr_node, OUT_PORT); 
    }
  } else if ((DEC_DIRECTION == src_rr_node->direction) 
     &&(CHANX == src_rr_node->type)) {
    /* Follow the graph above, go through X channel */
    for (ix = src_sb_info->x - 1; ix > end_sb_x; ix--) {
      /* Print an IN_PORT*/ 
      fprintf(fp, " ");
      /* output instance name */
      fprintf(fp, "%s/", 
              gen_verilog_one_sb_instance_name(&sb_info[ix][cur_sb_y])); 
      dump_verilog_one_sb_chan_pin(fp, &(sb_info[ix][cur_sb_y]), src_rr_node, IN_PORT); 
      /* Print an OUT_PORT*/ 
      fprintf(fp, " ");
      /* output instance name */
      fprintf(fp, "%s/", 
              gen_verilog_one_sb_instance_name(&sb_info[ix][cur_sb_y])); 
      dump_verilog_one_sb_chan_pin(fp, &(sb_info[ix][cur_sb_y]), src_rr_node, OUT_PORT); 
    }
  } else if ((DEC_DIRECTION == src_rr_node->direction) 
     &&(CHANY == src_rr_node->type)) {
    /* Follow the graph above, go through Y channel */
    for (iy = src_sb_info->y - 1; iy > end_sb_y; iy--) {
      /* Print an IN_PORT*/ 
      fprintf(fp, " ");
      /* output instance name */
      fprintf(fp, "%s/", 
              gen_verilog_one_sb_instance_name(&sb_info[cur_sb_x][iy])); 
      dump_verilog_one_sb_chan_pin(fp, &(sb_info[cur_sb_x][iy]), src_rr_node, IN_PORT); 
      /* Print an OUT_PORT*/ 
      fprintf(fp, " ");
      /* output instance name */
      fprintf(fp, "%s/", 
              gen_verilog_one_sb_instance_name(&sb_info[cur_sb_x][iy])); 
      dump_verilog_one_sb_chan_pin(fp, &(sb_info[cur_sb_x][iy]), src_rr_node, OUT_PORT); 
    }
  }

  return;
}

/* Report timing for a routing wire,
 * Support uni-directional routing architecture
 * Each routing wire start from an OPIN 
 * We check each fan-out to find all possible ending point:
 * An ending point is supposed to be an OPIN or CHANX or CHANY
 */
void verilog_generate_one_routing_wire_report_timing(FILE* fp, 
                                                     t_trpt_opts sdc_opts,
                                                     int L_wire,
                                                     t_sb* cur_sb_info,
                                                     t_rr_node* wire_rr_node,
                                                     int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                                     t_ivec*** LL_rr_node_indices) {
  int iedge, jedge, inode;
  int track_idx;
  int path_cnt = 0;
  t_sb* next_sb = NULL; 
  t_cb* next_cb = NULL; 
  int x_end, y_end;
  boolean sb_dumped = FALSE;

  /* Check the file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "(FILE:%s,LINE[%d])Invalid file handler for SDC generation",
               __FILE__, __LINE__); 
    exit(1);
  } 

  assert(  ( CHANX == wire_rr_node->type )
        || ( CHANY == wire_rr_node->type ));
  track_idx = wire_rr_node->ptc_num;

  /* We only care a specific length of wires */
  if (L_wire != (abs(wire_rr_node->xlow - wire_rr_node->xhigh + wire_rr_node->ylow - wire_rr_node->yhigh) + 1)) {
    return; 
  }

  /* Find the starting points */
  for (iedge = 0; iedge < wire_rr_node->num_drive_rr_nodes; iedge++) {
    sb_dumped = FALSE;
    /* Find the ending points*/
    for (jedge = 0; jedge < wire_rr_node->num_edges; jedge++) {
      /* Find where the destination pin belongs to */
      get_chan_rr_node_end_coordinate(wire_rr_node, &x_end, &y_end);
      /* Reciever could be IPIN or CHANX or CHANY */
      inode = wire_rr_node->edges[jedge];
      /* Find the SB/CB block that it belongs to */
      switch (LL_rr_node[inode].type) {
      case IPIN:
        /* Get the coordinate of ending CB */
        next_cb = get_chan_rr_node_ending_cb(wire_rr_node, &(LL_rr_node[inode]));
        /* This will not be the longest path unless the cb is close to the ending SB */
        if ((TRUE == sdc_opts.longest_path_only)
           && ((next_cb->x != x_end) || (next_cb->y != y_end))) {
          continue;
        }
        /* Driver could be OPIN or CHANX or CHANY,
          * and it must be in the cur_sb_info
          */
        /* Start printing report timing info  */
        fprintf(fp, "# L%d wire, Path %d\n", 
                abs(wire_rr_node->xlow - wire_rr_node->xhigh + wire_rr_node->ylow - wire_rr_node->yhigh) + 1,
                path_cnt); 
        fprintf(fp, "report_timing -from "); 
        /* output instance name */
        fprintf(fp, "%s/", 
                    gen_verilog_one_sb_instance_name(cur_sb_info)); 
        /* output pin name */
        dump_verilog_one_sb_routing_pin(fp, cur_sb_info, 
                                        wire_rr_node->drive_rr_nodes[iedge]);
        fprintf(fp, " -to "); 
        /* output instance name */
        fprintf(fp, "%s/",
                gen_verilog_one_cb_instance_name(next_cb));
        /* output pin name */
        fprintf(fp, "%s",
                gen_verilog_routing_channel_one_midout_name( next_cb,
                                                             track_idx));
        /* Print through pins */
        if (TRUE == sdc_opts.print_thru_pins) { 
          fprintf(fp, " -through_pins "); 
          dump_verilog_sb_through_routing_pins(fp, cur_sb_info, wire_rr_node, &(LL_rr_node[inode]));
          fprintf(fp, " -point_to_point\n"); 
        } else {
          fprintf(fp, " -unconstrained\n"); 
        }
        path_cnt++;
        break;
      case CHANX:
      case CHANY:
        /* Get the coordinate of ending SB */
        next_sb = get_chan_rr_node_ending_sb(wire_rr_node, &(LL_rr_node[inode]));
        /* This will not be the longest path unless the cb is close to the ending SB */
        if ((TRUE == sdc_opts.longest_path_only)
           && ((next_sb->x != x_end) || (next_sb->y != y_end))) {
          continue;
        }
        if (TRUE == sb_dumped) {
          continue;
        }
        /* Driver could be OPIN or CHANX or CHANY,
          * and it must be in the cur_sb_info
          */
        /* Start printing report timing info  */
        fprintf(fp, "# L%d wire, Path %d\n", 
                abs(wire_rr_node->xlow - wire_rr_node->xhigh + wire_rr_node->ylow - wire_rr_node->yhigh) + 1,
                path_cnt); 
        fprintf(fp, "report_timing -from "); 
        /* output instance name */
        fprintf(fp, "%s/", 
                    gen_verilog_one_sb_instance_name(cur_sb_info)); 
        /* output pin name */
        dump_verilog_one_sb_routing_pin(fp, cur_sb_info, 
                                        wire_rr_node->drive_rr_nodes[iedge]);
        fprintf(fp, " -to "); 
        /* output instance name */
        fprintf(fp, "%s/",
                gen_verilog_one_sb_instance_name(next_sb));
        /* Find which side the ending pin locates, and determine the coordinate */
        dump_verilog_one_sb_routing_pin(fp, next_sb, 
                                        wire_rr_node);
        /* Print through pins */
        if (TRUE == sdc_opts.print_thru_pins) { 
          fprintf(fp, " -through_pins "); 
          dump_verilog_sb_through_routing_pins(fp, cur_sb_info, 
                                               wire_rr_node, &(LL_rr_node[inode]));
          fprintf(fp, " -point_to_point\n"); 
        } else {
          fprintf(fp, " -unconstrained\n"); 
        }
        path_cnt++;
        /* Set the flag */
        sb_dumped = TRUE;
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
                                                  t_trpt_opts sdc_opts,
                                                  int L_wire, 
                                                  int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                                  t_ivec*** LL_rr_node_indices) {
  int ix, iy;
  int side, itrack;
  t_sb* cur_sb_info = NULL;

  /* Check the file handler */
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
          verilog_generate_one_routing_wire_report_timing(fp, sdc_opts, L_wire, &(sb_info[ix][iy]),
                                                          cur_sb_info->chan_rr_node[side][itrack],
                                                          LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);
        }
      } 
    }
  }

  return;
}

void verilog_generate_sb_report_timing(t_sram_orgz_info* cur_sram_orgz_info,
                                       t_trpt_opts sdc_opts,
                                       t_arch arch,
                                       t_det_routing_arch* routing_arch,
                                       int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                       t_ivec*** LL_rr_node_indices,
                                       t_syn_verilog_opts fpga_verilog_opts) {
  char* sdc_fname = NULL;
  FILE* fp = NULL;
  int iseg;
  int L_max = OPEN;

  /* Create the file handler */
  sdc_fname = my_strcat(format_dir_path(sdc_opts.sdc_dir), trpt_sb_file_name);

  /* Create a file*/
  fp = fopen(sdc_fname, "w");

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "(FILE:%s,LINE[%d])Failure in create SDC constraints %s",
               __FILE__, __LINE__, sdc_fname); 
    exit(1);
  } 

  /* Generate SDC header */
  dump_verilog_sdc_file_header(fp, "Report Timing for Switch blocks");

  vpr_printf(TIO_MESSAGE_INFO,
             "Generating TCL script to report timing for Switch Blocks: %s\n",
             sdc_fname);
  /* Find the longest wires: we only care defined length of wires? */
  for (iseg = 0; iseg < arch.num_segments; iseg++) {
    /* Bypass zero frequency sgements */
    if (0 == arch.Segments[iseg].frequency) {
      continue;
    }
    if ((OPEN == L_max) || (L_max < arch.Segments[iseg].length)) {
      L_max = arch.Segments[iseg].length;
    }
  }
  /* In some case, FPGA array size is smaller than any segments.
   * Therefore, to be strict non-segment timing will be reported
   * We added the FPGA array size for report timing 
   */
  if ((L_max > nx) && (L_max > ny)) { 
    if (nx != ny) {
      verilog_generate_routing_wires_report_timing(fp, sdc_opts,
                                                   nx, LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);
      verilog_generate_routing_wires_report_timing(fp, sdc_opts,
                                                   ny, LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);
    } else {
      verilog_generate_routing_wires_report_timing(fp, sdc_opts,
                                                   nx, LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);
    }
  } else {
    /* We only care defined length of wires? */
    for (iseg = 0; iseg < arch.num_segments; iseg++) {
      /* Bypass zero frequency sgements */
      if (0 == arch.Segments[iseg].frequency) {
        continue;
      }
      verilog_generate_routing_wires_report_timing(fp, sdc_opts, arch.Segments[iseg].length, 
                                                   LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices);
    }
  }

  /* close file*/
  fclose(fp);

  return;
}

/* Report timing for a routing wire divided in segements,
 * Support uni-directional routing architecture
 * Each routing wire start from an OPIN 
 * We check each fan-out to find all possible ending point:
 * An ending point is supposed to be an OPIN or CHANX or CHANY
 * We consider the farest ending point and report timing for each segements on the path
 * We output TCL commands to sum up the segmental delay 
 */
void verilog_generate_one_routing_segmental_report_timing(FILE* fp, 
                                                          t_trpt_opts sdc_opts,
                                                          t_sb* cur_sb_info,
                                                          t_rr_node* wire_rr_node,
                                                          int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                                          t_ivec*** LL_rr_node_indices) {
  int iedge, jedge, inode;
  int track_idx;
  int path_cnt = 0;
  t_sb* next_sb = NULL; 
  t_cb* next_cb = NULL; 
  int x_end, y_end;
  boolean sb_dumped = FALSE;
  int num_end_rr_nodes = 0;
  t_rr_node** end_rr_node = NULL;

  /* Check the file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "(FILE:%s,LINE[%d])Invalid file handler for SDC generation",
               __FILE__, __LINE__); 
    exit(1);
  } 

  assert(  ( CHANX == wire_rr_node->type )
        || ( CHANY == wire_rr_node->type ));
  track_idx = wire_rr_node->ptc_num;
 
  /* Find where the destination pin belongs to */
  get_chan_rr_node_end_coordinate(wire_rr_node, &x_end, &y_end);

  /* Find the farest ending points!*/
  for (jedge = 0; jedge < wire_rr_node->num_edges; jedge++) {
    /* Build a list of ending rr_node we care */
  }

  /* Find the starting points */
  for (iedge = 0; iedge < wire_rr_node->num_drive_rr_nodes; iedge++) {
    sb_dumped = FALSE;
    /* Find the ending points*/
    for (jedge = 0; jedge < wire_rr_node->num_edges; jedge++) {
      /* Find where the destination pin belongs to */
      /* Reciever could be IPIN or CHANX or CHANY */
    }
  }

  return;
}

void verilog_generate_routing_report_timing(t_sram_orgz_info* cur_sram_orgz_info,
                                            t_trpt_opts sdc_opts,
                                            t_arch arch,
                                            t_det_routing_arch* routing_arch,
                                            int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                            t_ivec*** LL_rr_node_indices,
                                            t_syn_verilog_opts fpga_verilog_opts) {
  char* sdc_fname = NULL;
  FILE* fp = NULL;
  int ix, iy;
  int side, itrack;
  t_sb* cur_sb_info = NULL;

  /* Create the file handler */
  sdc_fname = my_strcat(format_dir_path(sdc_opts.sdc_dir), trpt_routing_file_name);

  /* Create a file*/
  fp = fopen(sdc_fname, "w");

  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "(FILE:%s,LINE[%d])Failure in create SDC constraints %s",
               __FILE__, __LINE__, sdc_fname); 
    exit(1);
  } 

  /* Generate SDC header */
  dump_verilog_sdc_file_header(fp, "Report Timing for Switch blocks");

  vpr_printf(TIO_MESSAGE_INFO,
             "Generating TCL script to report timing for Switch Blocks: %s\n",
             sdc_fname);

  /* Check the file handler */
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
          verilog_generate_one_routing_segmental_report_timing(fp, sdc_opts, cur_sb_info, 
                                                               cur_sb_info->chan_rr_node[side][itrack], 
                                                               LL_num_rr_nodes, LL_rr_node, 
                                                               LL_rr_node_indices);
        }
      }
    }
  }

  /* close file*/
  fclose(fp);

  return;
}

/* Output a log file to guide routing report_timing */
void verilog_generate_report_timing(t_sram_orgz_info* cur_sram_orgz_info,
                                    char* sdc_dir,
                                    t_arch arch,
                                    t_det_routing_arch* routing_arch,
                                    int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                    t_ivec*** LL_rr_node_indices,
                                    t_syn_verilog_opts fpga_verilog_opts) {
  t_trpt_opts trpt_opts;

  /* Initialize */
  trpt_opts.report_pb_timing = TRUE;
  trpt_opts.report_sb_timing = TRUE;
  trpt_opts.report_cb_timing = TRUE;
  trpt_opts.report_routing_timing = TRUE;
  trpt_opts.longest_path_only = TRUE;
  trpt_opts.print_thru_pins = TRUE;
  trpt_opts.sdc_dir = my_strdup(sdc_dir);

  /* Part 1. Report timing for Programmable Logic Blocks */

  /* Part 2. Report timing for Connection Blocks */

  /* Part 3. Report timing for Switch Blocks */
  if (TRUE == trpt_opts.report_sb_timing) {
    verilog_generate_sb_report_timing(cur_sram_orgz_info, trpt_opts,
                                      arch, routing_arch,
                                      LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices,
                                      fpga_verilog_opts);
  }

  /* Part 3. */
  if (TRUE == trpt_opts.report_routing_timing) {
    verilog_generate_routing_report_timing(cur_sram_orgz_info, trpt_opts,
                                           arch, routing_arch,
                                           LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices,
                                           fpga_verilog_opts);
  }

  return;
}

