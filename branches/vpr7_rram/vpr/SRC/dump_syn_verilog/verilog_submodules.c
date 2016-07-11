/***********************************/
/*  Synthesizable Verilog Dumping  */
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
#include "path_delay.h"
#include "stats.h"

/* Include FPGA-SPICE utils */
#include "linkedlist.h"
#include "spice_utils.h"
#include "spice_mux.h"

/* Include verilog utils */
#include "verilog_utils.h"
#include "verilog_global.h"
#include "verilog_pbtypes.h"

/***** Subroutines *****/
t_llist* stats_verilog_muxes(int num_switch,
                             t_switch_inf* switches,
                             t_det_routing_arch* routing_arch) {
  /* Linked-list to store the information of Multiplexers*/
  t_llist* muxes_head = NULL; 
  int inode;
  int iedge;
  int itype;
  t_rr_node* node;
  t_spice_model* cb_switch_spice_model = NULL;
  t_spice_model* sb_switch_spice_model = NULL;

  /* Current Version: Support Uni-directional routing architecture only*/ 
  if (UNI_DIRECTIONAL != routing_arch->directionality) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Verilog Dumping Only support uni-directional routing architecture.\n",__FILE__, __LINE__);
    exit(1);
  }

  /* Step 1: We should check the multiplexer spice models defined in routing architecture.*/
  /* The routing path is. 
   *  OPIN ----> CHAN ----> ... ----> CHAN ----> IPIN
   *  Each edge is a switch, for IPIN, the switch is a connection block,
   *  for the rest is a switch box
   */
  /* Update the driver switch for each rr_node*/
  /* I can do a simple job here: 
   * just assign driver_switch from drive_switches[0]
   * which has been done in backannotation_vpr_post_route_info
   */
  /* update_rr_nodes_driver_switch(routing_arch->directionality); */
  for (inode = 0; inode < num_rr_nodes; inode++) {
    rr_node[inode].driver_switch = rr_node[inode].drive_switches[0];
    for (iedge = 0; iedge < rr_node[inode].num_drive_rr_nodes; iedge++) {
     assert (rr_node[inode].driver_switch == rr_node[inode].drive_switches[iedge]);
    }
  }
  /* Count the sizes of muliplexers in routing architecture */  
  /* Visit the global variable : num_rr_nodes, rr_node */
  for (inode = 0; inode < num_rr_nodes; inode++) {
    node = &rr_node[inode]; 
    switch (node->type) {
    case IPIN: 
      /* Have to consider the fan_in only, it is a connection box(multiplexer)*/
      assert((node->fan_in > 0)||(0 == node->fan_in));
      if (0 == node->fan_in) {
        break; 
      }
      /* Find the spice_model for multiplexers in connection blocks*/
      cb_switch_spice_model = switches[node->driver_switch].spice_model;
      /* we should select a spice model for the Switch box*/
      assert(NULL != cb_switch_spice_model);
      check_and_add_mux_to_linked_list(&muxes_head, node->fan_in,cb_switch_spice_model);
      break;
    case CHANX:
    case CHANY: 
      /* Channels are the same, have to consider the fan_in as well, 
       * it could be a switch box if previous rr_node is a channel
       * or it could be a connection box if previous rr_node is a IPIN or OPIN
       */
      assert((node->fan_in > 0)||(0 == node->fan_in));
      if (0 == node->fan_in) {
        break; 
      }
      /* Find the spice_model for multiplexers in switch blocks*/
      sb_switch_spice_model = switches[node->driver_switch].spice_model;
      /* we should select a spice model for the Switch box*/
      assert(NULL != sb_switch_spice_model);
      check_and_add_mux_to_linked_list(&muxes_head, node->fan_in,sb_switch_spice_model);
      break;
    case OPIN: 
      /* Actually, in single driver routing architecture, the OPIN, source of a routing path,
       * is directly connected to Switch Box multiplexers
       */
      break;
    default:
      break;
    }
  }

  /* Step 2: Count the sizes of multiplexers in complex logic blocks */  
  for (itype = 0; itype < num_types; itype++) {
    if (NULL != type_descriptors[itype].pb_type) {
      stats_mux_verilog_model_pb_type_rec(&muxes_head,type_descriptors[itype].pb_type);
    }
  }

  return muxes_head;
}

/**
 * Dump a verilog module for a MUXe
 */
void fprint_verilog_mux_module(FILE* fp, 
                               t_spice_model* spice_mux_model) {
  /** Act depends on the structure of MUX
   * 1. one-level: no need to print a submodule
   * 2. two/multi-level: we need to print a submodule
   */
  

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  return;
}


/*** Top-level function *****/

/* We should count how many multiplexers with different sizes are needed */
void dump_verilog_submodule_muxes(char* subckt_dir,
                                  int num_switch,
                                  t_switch_inf* switches,
                                  t_spice* spice,
                                  t_det_routing_arch* routing_arch) {

  return;
}

