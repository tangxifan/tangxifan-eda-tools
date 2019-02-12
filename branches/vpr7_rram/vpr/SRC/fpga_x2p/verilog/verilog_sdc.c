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

/* options for report timing */
typedef struct s_sdc_trpt_opts t_sdc_trpt_opts;
struct s_sdc_trpt_opts {
  char* sdc_dir;
  boolean longest_path_only;
  boolean report_cb_timing;
  boolean report_sb_timing;
};

/***** Subroutine Functions *****/
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

/* Output the pin name of a routing wire in a SB */
void dump_verilog_one_sb_routing_pin(FILE* fp,
                                     t_sb* cur_sb_info,
                                     t_rr_node* cur_rr_node) {
  int track_idx, side;
  int x_start, y_start;
  t_rr_type chan_rr_type;

  /* Check the file handler */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "(FILE:%s,LINE[%d])Invalid file handler for SDC generation",
               __FILE__, __LINE__); 
    exit(1);
  } 

  /* Get the top-level pin name and print it out */
  /* Depends on the type of node */
  switch (cur_rr_node->type) {
  case OPIN:
    /* Identify the side of OPIN on a grid */
    side = get_grid_pin_side(cur_rr_node->xlow, cur_rr_node->ylow, cur_rr_node->ptc_num);
    assert (OPEN != side);
    dump_verilog_grid_side_pin_with_given_index(fp, OPIN,
                                                cur_rr_node->ptc_num,
                                                side,
                                                cur_rr_node->xlow,
                                                cur_rr_node->ylow, 
                                                FALSE); /* Do not specify direction of port */
    break; 
  case CHANX:
  case CHANY:
    /* Get the coordinate of chanx or chany*/
    /* Find the coordinate of the cur_rr_node */  
    get_rr_node_side_and_index_in_sb_info(cur_rr_node, 
                                          *cur_sb_info,
                                          IN_PORT, &side, &track_idx); 
    get_chan_rr_node_coorindate_in_sb_info(*cur_sb_info, side, 
                                           &(chan_rr_type),
                                           &x_start, &y_start);
    assert (chan_rr_type == cur_rr_node->type); 
    /* Print the pin of the cur_rr_node */  
    fprintf(fp, "%s",
            gen_verilog_routing_channel_one_pin_name(cur_rr_node,
                                                     x_start, y_start, track_idx, 
                                                     IN_PORT));
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid type of ending point rr_node!\n",
               __FILE__, __LINE__);
 
    exit(1);
  }

  return;
}

/** Given a starting rr_node (CHANX or CHANY) 
 *  and a ending rr_node (IPIN) 
 *  return the cb contains both (the ending CB of the routing wire)
 */
t_cb* get_chan_rr_node_ending_cb(t_rr_node* src_rr_node, 
                                t_rr_node* end_rr_node) {
  int num_ipin_sides = 2;
  int* ipin_side = (int*)my_calloc(num_ipin_sides, sizeof(int));
  int num_chan_sides = 2;
  int* chan_side = (int*)my_calloc(num_chan_sides, sizeof(int));
  int iside, next_cb_x, next_cb_y;
  int node_exist;
  t_cb* next_cb = NULL;
 
  /* Type of connection block depends on the src_rr_node */
  switch (src_rr_node->type) {
  case CHANX:
    /* the x of CB is same as end_rr_node,
     * the y of CB should be same as src_rr_node
     */
    assert (end_rr_node->xlow == end_rr_node->xhigh);
    next_cb_x = end_rr_node->xlow;
    assert (src_rr_node->ylow == src_rr_node->yhigh);
    next_cb_y = src_rr_node->ylow;
    /* Side will be either on TOP or BOTTOM */
    ipin_side[0] = TOP;
    ipin_side[1] = BOTTOM;
    chan_side[0] = RIGHT;
    chan_side[1] = LEFT;
    next_cb = &(cbx_info[next_cb_x][next_cb_y]); 
    break;
  case CHANY:
    /* the x of CB is same as src_rr_node,
     * the y of CB should be same as end_rr_node
     */
    assert (src_rr_node->xlow == src_rr_node->xhigh);
    next_cb_x = src_rr_node->xlow;
    assert (end_rr_node->ylow == end_rr_node->yhigh);
    next_cb_y = end_rr_node->ylow;
    /* Side will be either on RIGHT or LEFT */
    ipin_side[0] = LEFT;
    ipin_side[1] = RIGHT;
    chan_side[0] = BOTTOM;
    chan_side[1] = TOP;
    next_cb = &(cby_info[next_cb_x][next_cb_y]); 
    break;
  default:
   vpr_printf(TIO_MESSAGE_ERROR, 
              "(File: %s [LINE%d]) Invalid type of src_rr_node!\n",
               __FILE__, __LINE__);
 
    exit(1);
  }

  /* Double check if src_rr_node is in the IN_PORT list */
  node_exist = 0;
  for (iside = 0; iside < num_chan_sides; iside++) {
    if (OPEN != get_rr_node_index_in_cb_info( src_rr_node,
                                              *next_cb, 
                                              chan_side[iside], IN_PORT)) {   
      node_exist++;
    }
  }
  assert (0 < node_exist);

  /* Double check if end_rr_node is in the OUT_PORT list */
  node_exist = 0;
  for (iside = 0; iside < num_ipin_sides; iside++) {
    if (OPEN != get_rr_node_index_in_cb_info( end_rr_node,
                                              *next_cb, 
                                              ipin_side[iside], OUT_PORT)) {
      node_exist++;
    }
  }
  assert (0 < node_exist);

  return next_cb;
}

/** Given a starting rr_node (CHANX or CHANY) 
 *  and a ending rr_node (IPIN) 
 *  return the sb contains both (the ending CB of the routing wire)
 */
t_sb* get_chan_rr_node_ending_sb(t_rr_node* src_rr_node, 
                                 t_rr_node* end_rr_node) {
  int side;
  int x_start, y_start;
  int x_end, y_end;
  int next_sb_x, next_sb_y;
  int node_exist;
  t_sb* next_sb = NULL;

  get_chan_rr_node_start_coordinate(src_rr_node, &x_start, &y_start);
  get_chan_rr_node_start_coordinate(end_rr_node, &x_end, &y_end);

  /* Case 1:                       
   *                     end_rr_node(chany[x][y+1]) 
   *                        /|\ 
   *                         |  
   *                     ---------
   *                    |         | 
   * src_rr_node ------>| next_sb |-------> end_rr_node
   * (chanx[x][y])      |  [x][y] |        (chanx[x+1][y]
   *                     ---------
   *                         |
   *                        \|/
   *                     end_rr_node(chany[x][y])
   */
  /* Case 2                            
   *                     end_rr_node(chany[x][y+1]) 
   *                        /|\ 
   *                         |  
   *                     ---------
   *                    |         | 
   * end_rr_node <------| next_sb |<-------- src_rr_node
   * (chanx[x][y])      |  [x][y] |        (chanx[x+1][y]
   *                     ---------
   *                         |
   *                        \|/
   *                     end_rr_node(chany[x][y])
   */
  /* Case 3                            
   *                     end_rr_node(chany[x][y+1]) 
   *                        /|\ 
   *                         |  
   *                     ---------
   *                    |         | 
   * end_rr_node <------| next_sb |-------> src_rr_node
   * (chanx[x][y])      |  [x][y] |        (chanx[x+1][y]
   *                     ---------
   *                        /|\
   *                         |
   *                     src_rr_node(chany[x][y])
   */
  /* Case 4                            
   *                     src_rr_node(chany[x][y+1]) 
   *                         | 
   *                        \|/  
   *                     ---------
   *                    |         | 
   * end_rr_node <------| next_sb |--------> end_rr_node
   * (chanx[x][y])      |  [x][y] |        (chanx[x+1][y]
   *                     ---------
   *                         |
   *                        \|/
   *                     end_rr_node(chany[x][y])
   */

 
  /* Try the xlow, ylow of ending rr_node */
  switch (src_rr_node->type) {
  case CHANX:
    next_sb_x = x_end;
    next_sb_y = y_start;
    break;
  case CHANY:
    next_sb_x = x_start;
    next_sb_y = y_end;
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid type of rr_node!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  switch (src_rr_node->direction) {
  case INC_DIRECTION:
    get_chan_rr_node_end_coordinate(src_rr_node, &x_end, &y_end);
    if (next_sb_x > x_end) {
      next_sb_x = x_end;
    }
    if (next_sb_y > y_end) {
      next_sb_y = y_end;
    }
    break;
  case DEC_DIRECTION:
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid type of rr_node!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Double check if src_rr_node is in the list */
  node_exist = 0;
  for (side = 0; side < 4; side++) {
    if( OPEN != get_rr_node_index_in_sb_info(src_rr_node, 
                                             sb_info[next_sb_x][next_sb_y],
                                             side, IN_PORT)) {
      node_exist++;
    }
  }
  assert (1 == node_exist);                                        

  /* Double check if end_rr_node is in the list */
  node_exist = 0;
  for (side = 0; side < 4; side++) {
    if (OPEN != get_rr_node_index_in_sb_info(end_rr_node, 
                                             sb_info[next_sb_x][next_sb_y],
                                             side, OUT_PORT)) {
      node_exist++;
    }
  }
  if (1 != node_exist) {
    assert (1 == node_exist);                                        
  }

  /* Passing the check, assign ending sb */
  next_sb = &(sb_info[next_sb_x][next_sb_y]);

  return next_sb;
}

/* Report timing for a routing wire,
 * Support uni-directional routing architecture
 * Each routing wire start from an OPIN 
 * We check each fan-out to find all possible ending point:
 * An ending point is supposed to be an OPIN or CHANX or CHANY
 */
void verilog_generate_one_routing_wire_report_timing(FILE* fp, 
                                                     t_sdc_trpt_opts sdc_opts,
                                                     int L_wire,
                                                     t_sb* cur_sb_info,
                                                     t_rr_node* wire_rr_node,
                                                     int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                                     t_ivec*** LL_rr_node_indices) {
  int iedge, jedge, inode;
  int track_idx, side;
  int path_cnt = 0;
  t_sb* next_sb = NULL; 
  t_cb* next_cb = NULL; 
  int x_end, y_end;
  t_rr_type end_chan_rr_type;

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
    /* Find the ending points*/
    for (jedge = 0; jedge < wire_rr_node->num_edges; jedge++) {
      /* Find where the destination pin belongs to */
      get_chan_rr_node_start_coordinate(wire_rr_node, &x_end, &y_end);
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
        fprintf(fp, " -unconstrained -point_to_point\n"); 
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
        get_rr_node_side_and_index_in_sb_info(wire_rr_node, *next_sb,
                                              IN_PORT, &side, &track_idx); 
        get_chan_rr_node_coorindate_in_sb_info(*next_sb, side, 
                                               &(end_chan_rr_type),
                                               &x_end, &y_end);
        assert (end_chan_rr_type == wire_rr_node->type); 
        /* output pin name */
        fprintf(fp, "%s",
                gen_verilog_routing_channel_one_pin_name(  wire_rr_node,
                                                           x_end, y_end, track_idx, 
                                                           IN_PORT));
        fprintf(fp, " -unconstrained -point_to_point\n"); 
        path_cnt++;
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
                                                  t_sdc_trpt_opts sdc_opts,
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
                                       t_sdc_trpt_opts sdc_opts,
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
  sdc_fname = my_strcat(format_dir_path(sdc_opts.sdc_dir), routing_sdc_file_name);

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

/* Output a log file to guide routing report_timing */
void verilog_generate_routing_report_timing(t_sram_orgz_info* cur_sram_orgz_info,
                                            char* sdc_dir,
                                            t_arch arch,
                                            t_det_routing_arch* routing_arch,
                                            int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                            t_ivec*** LL_rr_node_indices,
                                            t_syn_verilog_opts fpga_verilog_opts) {
  t_sdc_trpt_opts sdc_trpt_opts;

  /* Initialize */
  sdc_trpt_opts.report_sb_timing = TRUE;
  sdc_trpt_opts.report_cb_timing = TRUE;
  sdc_trpt_opts.longest_path_only = TRUE;
  sdc_trpt_opts.sdc_dir = my_strdup(sdc_dir);
 

  /* Part 1. Output routing constraints for routing tracks */

  /* Part 2. Output routing constraints for Switch Blocks */

  /* Part 3. Output routing constraints for Connection Blocks */

  /* Part 4. Report timing for routing wires */
  if (TRUE == sdc_trpt_opts.report_sb_timing) {
    verilog_generate_sb_report_timing(cur_sram_orgz_info, sdc_trpt_opts,
                                      arch, routing_arch,
                                      LL_num_rr_nodes, LL_rr_node, LL_rr_node_indices,
                                      fpga_verilog_opts);
  }

  return;
}

