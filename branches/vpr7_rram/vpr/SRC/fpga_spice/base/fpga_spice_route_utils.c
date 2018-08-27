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

/* Include SPICE support headers*/
#include "linkedlist.h"
#include "fpga_spice_utils.h"
#include "fpga_spice_utils.h"
#include "fpga_spice_bitstream_utils.h"
#include "fpga_spice_pbtypes_utils.h"
#include "fpga_spice_globals.h"

/* Count the number of rr_graph nodes that should be allocated
 *   (a) INPUT pins at the top-level pb_graph_node should be a local_rr_node and plus a SOURCE
 *   (b) CLOCK pins at the top-level pb_graph_node should be a local_rr_node and plus a SOURCE
 *   (c) OUTPUT pins at the top-level pb_graph_node should be a local_rr_node and plus a SINK
 *   (e) INPUT pins at a primitive pb_graph_node should be a local_rr_node and plus a SINK
 *   (f) CLOCK pins at a primitive pb_graph_node should be a local_rr_node and plus a SINK
 *   (g) OUTPUT pins at a primitive pb_graph_node should be a local_rr_node and plus a SOURCE
 *   (h) all the other pins should be a local_rr_node 
 */
int rec_count_rr_graph_nodes_for_phy_pb_graph_node(t_pb_graph_node* cur_pb_graph_node) {
  boolean is_top_pb_graph_node = (boolean) (NULL == cur_pb_graph_node->parent_pb_graph_node);
  boolean is_primitive_pb_graph_node = (boolean) (NULL != cur_pb_graph_node->pb_type->spice_model);
  int phy_mode_idx = -1;
  int cur_num_rr_nodes = 0;
  int ichild, ipb;

  /* Count in INPUT/OUTPUT/CLOCK pins as regular rr_nodes */
  cur_num_rr_nodes = count_pin_number_one_pb_graph_node(cur_pb_graph_node);

  /* check if this is a top pb_graph_node or a primitive node */
  if ((TRUE == is_top_pb_graph_node)||(TRUE == is_primitive_pb_graph_node)) {
    /* Count in INPUT/OUTPUT/CLOCK pins as SINK/SOURCE rr_nodes */
    cur_num_rr_nodes += cur_num_rr_nodes;
  }

  /* Return when this is a primitive node */
  if (TRUE == is_primitive_pb_graph_node) {
    return cur_num_rr_nodes;
  }

  /* Go recursively to the lower levels */
  /* Find the physical mode in the next level! */
  phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->pb_type));
  for (ichild = 0; ichild < cur_pb_graph_node->pb_type->modes[phy_mode_idx].num_pb_type_children; ichild++) {
    /* num_pb is the number of such pb_type in a physical mode*/
    for (ipb = 0; ipb < cur_pb_graph_node->pb_type->modes[phy_mode_idx].pb_type_children[ichild].num_pb; ipb++) {
      cur_num_rr_nodes += rec_count_rr_graph_nodes_for_phy_pb_graph_node(&cur_pb_graph_node->child_pb_graph_nodes[phy_mode_idx][ichild][ipb]);
    }
  }

  return cur_num_rr_nodes;
}

/* initialize a rr_node in a rr_graph of phyical pb_graph_node */
void init_one_rr_node_for_phy_pb_graph_node(INP t_pb_graph_pin* cur_pb_graph_pin,
                                            INOUTP t_rr_graph* local_rr_graph,
                                            IN int cur_rr_node_index,
                                            int phy_mode_index, 
                                            t_rr_type rr_node_type) {
  /* Link the rr_node to the pb_graph_pin*/
  cur_pb_graph_pin->rr_node_index_physical_pb = cur_rr_node_index;
  local_rr_graph->rr_node[cur_rr_node_index].pb_graph_pin = cur_pb_graph_pin;

  switch (rr_node_type) {
  case INTRA_CLUSTER_EDGE: 
    /* Get the number of input edges that belong to physical mode only! */
    local_rr_graph->rr_node[cur_rr_node_index].fan_in = count_pb_graph_node_input_edge_in_phy_mode(cur_pb_graph_pin, phy_mode_index);
    /* Get the number of output edges that belong to physical mode only! */
    local_rr_graph->rr_node[cur_rr_node_index].num_edges = count_pb_graph_node_output_edge_in_phy_mode(cur_pb_graph_pin, phy_mode_index);
    break;
  case SOURCE:
    /* SOURCE only has one output and zero inputs */
    local_rr_graph->rr_node[cur_rr_node_index].fan_in = 0;
    local_rr_graph->rr_node[cur_rr_node_index].num_edges = 1;
    break;
  case SINK:
    /* SINK only has one input and zero outputs */
    local_rr_graph->rr_node[cur_rr_node_index].fan_in = 1;
    local_rr_graph->rr_node[cur_rr_node_index].num_edges = 0;
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, 
               "(File:%s, [LINE%d]) Invalid rr_node_type (%d)! \n",
               __FILE__, __LINE__, rr_node_type); 
    exit(1);
  }

  /* Routing connectivity */
  local_rr_graph->rr_node[cur_rr_node_index].edges = (int *) my_calloc(cur_rr_node->num_edges, sizeof(int));
  local_rr_graph->rr_node[cur_rr_node_index].switches = (short *) my_calloc(cur_rr_node->num_edges,  sizeof(short));
  local_rr_graph->rr_node[cur_rr_node_index].net_num = OPEN;
  local_rr_graph->rr_node[cur_rr_node_index].prev_node = OPEN;
  local_rr_graph->rr_node[cur_rr_node_index].prev_edge = OPEN;

  local_rr_graph->rr_node[cur_rr_node_index].capacity = 1;
  local_rr_graph->rr_node[cur_rr_node_index].type = rr_node_type;
  
  return;
} 

/* Recursively configure all the rr_nodes in the rr_graph
 * Initialize the routing cost, fan-in rr_nodes and fan-out rr_nodes, and switches  
 */
void rec_init_rr_graph_for_phy_pb_graph_node(INP t_graph_node* cur_pb_graph_node, 
                                             INOUTP t_rr_graph* local_rr_graph,
                                             int cur_rr_node_index) {
  boolean is_top_pb_graph_node = (boolean) (NULL == cur_pb_graph_node->parent_pb_graph_node);
  boolean is_primitive_pb_graph_node = (boolean) (NULL != cur_pb_graph_node->pb_type->spice_model);
  int phy_mode_idx = -1;
  int iport, ipin;
  int ichild, ipb;

  /* Find the physical mode in the next level! */
  phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->pb_type));

  /* Configure rr_nodes with the information of pb_graph_pin  */
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; ipin++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
      init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                             cur_rr_node_index, phy_mode_index, INTRA_CLUSTER_EDGE);
      cur_rr_node_index++;
    }
  }

  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; ipin++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                             cur_rr_node_index, phy_mode_index, INTRA_CLUSTER_EDGE);
      cur_rr_node_index++;
    }
  }

  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; ipin++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
      init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->clock_pins[iport][ipin], local_rr_graph,  
                                             cur_rr_node_index, phy_mode_index, INTRA_CLUSTER_EDGE);
      cur_rr_node_index++;
    }
  }

  /* check if this is a top pb_graph_node */
  if ((TRUE == is_top_pb_graph_node)) {
    /* Configure SOURCE and SINK rr_node: 
     * input_pins should have a SOURCE node
     */
    for (iport = 0; iport < cur_pb_graph_node->num_input_ports; ipin++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                               cur_rr_node_index, phy_mode_index, SOURCE);

        cur_rr_node_index++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * output_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_output_ports; ipin++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                               cur_rr_node_index, phy_mode_index, SINK);

        cur_rr_node_index++;
      }
    }
    /* Finish adding SOURCE and SINKs */
  }

  /* Return when this is a primitive node */
  if (TRUE == is_primitive_pb_graph_node) {
    /* Configure SOURCE and SINK rr_node: 
     * output_pins should have a SOURCE node
     */
    for (iport = 0; iport < cur_pb_graph_node->num_output_ports; ipin++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                               cur_rr_node_index, phy_mode_index, SOURCE);

        cur_rr_node_index++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * input_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_input_ports; ipin++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                               cur_rr_node_index, phy_mode_index, SINK);

        cur_rr_node_index++;
      }
    }
    /* Finish adding SOURCE and SINKs */

    return;
  }

  /* Go recursively to the lower levels */
  for (ichild = 0; ichild < cur_pb_graph_node->pb_type->modes[phy_mode_idx].num_pb_type_children; ichild++) {
    /* num_pb is the number of such pb_type in a physical mode*/
    for (ipb = 0; ipb < cur_pb_graph_node->pb_type->modes[phy_mode_idx].pb_type_children[ichild].num_pb; ipb++) {
      cur_num_rr_nodes += rec_init_rr_graph_for_phy__pb_graph_node(&cur_pb_graph_node->child_pb_graph_nodes[phy_mode_idx][ichild][ipb]);
    }
  }

  return;
}

/* Allocate a rr_graph for a given pb_graph node 
 * This is function is a copy of alloc_and_load_rr_graph_for_pb_graph_node
 * The major difference lies in removing the use of global variables 
 * This function does the following tasks:
 * 1. Count all the pins in pb_graph_node (only those in physical modes) that can be a rr_node in the local rr_graph
 *   (a) INPUT pins at the top-level pb_graph_node should be a local_rr_node and plus a SOURCE
 *   (b) CLOCK pins at the top-level pb_graph_node should be a local_rr_node and plus a SOURCE
 *   (c) OUTPUT pins at the top-level pb_graph_node should be a local_rr_node and plus a SINK
 *   (e) INPUT pins at a primitive pb_graph_node should be a local_rr_node and plus a SINK
 *   (f) CLOCK pins at a primitive pb_graph_node should be a local_rr_node and plus a SINK
 *   (g) OUTPUT pins at a primitive pb_graph_node should be a local_rr_node and plus a SOURCE
 *   (h) all the other pins should be a local_rr_node 
 * 2. Allocate the rr_graph and initialize its properties according to the pb_graph_pin interconnections
 *   capacity, mapped net name,  edges, switches, routing cost information
 * 3. Synchronize mapped_net_name from a mapped pb:
 *   (a) give mapped_net_name to INPUT/OUTPUT/CLOCK pins of the top-level pb_graph_node
 *   (b) give mapped_net_name to INPUT/OUTPUT/CLOCK pins of the primitive pb_graph_nodes
 */
void alloc_and_load_rr_graph_for_phy_pb_graph_node(INP t_pb_graph_node* top_pb_graph_node, 
                                                  INP const t_arch* arch, 
                                                  OUTP t_rr_graph* local_rr_graph) {

  int i, j, k, index;

  /* Count the number of rr_nodes that are required */
  local_rr_graph->num_rr_nodes = rec_count_rr_graph_nodes_for_phy_pb_graph_node(top_pb_graph_node);
   
  /* Allocate rr_graph */
  local_rr_graph->rr_node = (t_rr_node*) my_calloc(local_rr_graph->num_rr_nodes, sizeof(t_rr_node));

  /* Fill basic information for the rr_graph  */ 
  rec_init_rr_graph_for_phy_pb_graph_node(top_pb_graph_node, local_rr_graph);

  /* Fill edges and switches for the rr_graph */

  for (i = 0; i < pb_graph_node->num_input_ports; i++) {
    for (j = 0; j < pb_graph_node->num_input_pins[i]; j++) {
      index = pb_graph_node->input_pins[i][j].pin_count_in_cluster;
      LL_rr_node[index].pb_graph_pin = &pb_graph_node->input_pins[i][j];
      LL_rr_node[index].fan_in =
          pb_graph_node->input_pins[i][j].num_input_edges;
      LL_rr_node[index].num_edges =
          pb_graph_node->input_pins[i][j].num_output_edges;
      LL_rr_node[index].pack_intrinsic_cost = 1
          + (float) LL_rr_node[index].num_edges / 5 + ((float)j/(float)pb_graph_node->num_input_pins[i])/(float)10; /* need to normalize better than 5 and 10, bias router to use earlier inputs pins */
      LL_rr_node[index].edges = (int *) my_malloc(
          LL_rr_node[index].num_edges * sizeof(int));
      LL_rr_node[index].switches = (short *) my_calloc(rr_node[index].num_edges,
          sizeof(short));
      LL_rr_node[index].net_num = OPEN;
      LL_rr_node[index].prev_node = OPEN;
      LL_rr_node[index].prev_edge = OPEN;
      if (mode == 0) { /* default mode is the first mode */
        LL_rr_node[index].capacity = 1;
      } else {
        LL_rr_node[index].capacity = 0;
      }
      for (k = 0; k < pb_graph_node->input_pins[i][j].num_output_edges;
          k++) {
        /* TODO: Intention was to do bus-based implementation here */
        LL_rr_node[index].edges[k] =
            pb_graph_node->input_pins[i][j].output_edges[k]->output_pins[0]->pin_count_in_cluster;
        LL_rr_node[index].switches[k] = arch->num_switches - 1; /* last switch in arch switch properties is a delayless switch */
        assert(
            pb_graph_node->input_pins[i][j].output_edges[k]->num_output_pins == 1);
      }
      LL_rr_node[index].type = INTRA_CLUSTER_EDGE;
      if (is_primitive) {
        /* This is a terminating pin, add SINK node */
        assert(rr_node[index].num_edges == 0);
        LL_rr_node[index].num_edges = 1;
        LL_rr_node[index].edges = (int *) my_calloc(rr_node[index].num_edges, sizeof(int));
        LL_rr_node[index].switches = (short *) my_calloc(rr_node[index].num_edges, sizeof(short));
        LL_rr_node[index].edges[0] = num_rr_nodes;
        
        /* Create SINK node */
        LL_rr_node[num_rr_nodes].pb_graph_pin = NULL;
        LL_rr_node[num_rr_nodes].fan_in = 1;
        LL_rr_node[num_rr_nodes].num_edges = 0;
        LL_rr_node[num_rr_nodes].pack_intrinsic_cost = 1;
        LL_rr_node[num_rr_nodes].edges = NULL;
        LL_rr_node[num_rr_nodes].switches = NULL;
        LL_rr_node[num_rr_nodes].net_num = OPEN;
        LL_rr_node[num_rr_nodes].prev_node = OPEN;
        LL_rr_node[num_rr_nodes].prev_edge = OPEN;
        LL_rr_node[num_rr_nodes].capacity = 1;
        LL_rr_node[num_rr_nodes].type = SINK;          
        num_rr_nodes++;

        if(pb_graph_node->pb_type->class_type == LUT_CLASS) {
          /* LUTs are special, they have logical equivalence at inputs, logical equivalence is represented by a single high capacity sink instead of multiple single capacity sinks */
          LL_rr_node[num_rr_nodes - 1].capacity = pb_graph_node->num_input_pins[i];          
          if(j != 0) {
            num_rr_nodes--;
            LL_rr_node[index].edges[0] = num_rr_nodes - 1;
          }          
        }        
      }
    }
  }

  for (i = 0; i < pb_graph_node->num_output_ports; i++) {
    for (j = 0; j < pb_graph_node->num_output_pins[i]; j++) {
      index = pb_graph_node->output_pins[i][j].pin_count_in_cluster;
      LL_rr_node[index].pb_graph_pin = &pb_graph_node->output_pins[i][j];
      LL_rr_node[index].fan_in =
          pb_graph_node->output_pins[i][j].num_input_edges;
      LL_rr_node[index].num_edges =
          pb_graph_node->output_pins[i][j].num_output_edges;
      LL_rr_node[index].pack_intrinsic_cost = 1
          + (float) LL_rr_node[index].num_edges / 5; /* need to normalize better than 5 */
      LL_rr_node[index].edges = (int *) my_malloc(
          LL_rr_node[index].num_edges * sizeof(int));
      LL_rr_node[index].switches = (short *) my_calloc(rr_node[index].num_edges,
          sizeof(short));
      LL_rr_node[index].net_num = OPEN;
      LL_rr_node[index].prev_node = OPEN;
      LL_rr_node[index].prev_edge = OPEN;
      if (mode == 0) { /* Default mode is the first mode */
        LL_rr_node[index].capacity = 1;
      } else {
        LL_rr_node[index].capacity = 0;
      }
      for (k = 0; k < pb_graph_node->output_pins[i][j].num_output_edges;
          k++) {
        /* TODO: Intention was to do bus-based implementation here */
        LL_rr_node[index].edges[k] =
            pb_graph_node->output_pins[i][j].output_edges[k]->output_pins[0]->pin_count_in_cluster;
        LL_rr_node[index].switches[k] = arch->num_switches - 1;
        assert(
            pb_graph_node->output_pins[i][j].output_edges[k]->num_output_pins == 1);
      }
      LL_rr_node[index].type = INTRA_CLUSTER_EDGE;
      if (is_primitive) {
        LL_rr_node[index].type = SOURCE;
      }
    }
  }

  for (i = 0; i < pb_graph_node->num_clock_ports; i++) {
    for (j = 0; j < pb_graph_node->num_clock_pins[i]; j++) {
      index = pb_graph_node->clock_pins[i][j].pin_count_in_cluster;
      LL_rr_node[index].pb_graph_pin = &pb_graph_node->clock_pins[i][j];
      LL_rr_node[index].fan_in =
          pb_graph_node->clock_pins[i][j].num_input_edges;
      LL_rr_node[index].num_edges =
          pb_graph_node->clock_pins[i][j].num_output_edges;
      LL_rr_node[index].pack_intrinsic_cost = 1
          + (float) LL_rr_node[index].num_edges / 5; /* need to normalize better than 5 */
      LL_rr_node[index].edges = (int *) my_malloc(
          LL_rr_node[index].num_edges * sizeof(int));
      LL_rr_node[index].switches = (short *) my_calloc(rr_node[index].num_edges,
          sizeof(short));
      LL_rr_node[index].net_num = OPEN;
      LL_rr_node[index].prev_node = OPEN;
      LL_rr_node[index].prev_edge = OPEN;
      if (mode == 0) { /* default mode is the first mode (useful for routing */
        LL_rr_node[index].capacity = 1;
      } else {
        LL_rr_node[index].capacity = 0;
      }
      for (k = 0; k < pb_graph_node->clock_pins[i][j].num_output_edges;
          k++) {
        /* TODO: Intention was to do bus-based implementation here */
        LL_rr_node[index].edges[k] =
            pb_graph_node->clock_pins[i][j].output_edges[k]->output_pins[0]->pin_count_in_cluster;
        LL_rr_node[index].switches[k] = arch->num_switches - 1;
        assert(
            pb_graph_node->clock_pins[i][j].output_edges[k]->num_output_pins == 1);
      }
      LL_rr_node[index].type = INTRA_CLUSTER_EDGE;
      if (is_primitive) {
        /* This is a terminating pin, add SINK node */
        assert(rr_node[index].num_edges == 0);
        LL_rr_node[index].num_edges = 1;
        LL_rr_node[index].edges = (int *) my_calloc(rr_node[index].num_edges, sizeof(int));
        LL_rr_node[index].switches = (short *) my_calloc(rr_node[index].num_edges, sizeof(short));
        LL_rr_node[index].edges[0] = num_rr_nodes;
        
        /* Create SINK node */
        LL_rr_node[num_rr_nodes].pb_graph_pin = NULL;
        LL_rr_node[num_rr_nodes].fan_in = 1;
        LL_rr_node[num_rr_nodes].num_edges = 0;
        LL_rr_node[num_rr_nodes].pack_intrinsic_cost = 1;
        LL_rr_node[num_rr_nodes].edges = NULL;
        LL_rr_node[num_rr_nodes].switches = NULL;
        LL_rr_node[num_rr_nodes].net_num = OPEN;
        LL_rr_node[num_rr_nodes].prev_node = OPEN;
        LL_rr_node[num_rr_nodes].prev_edge = OPEN;
        LL_rr_node[num_rr_nodes].capacity = 1;
        LL_rr_node[num_rr_nodes].type = SINK;  
        num_rr_nodes++;
      }
    }
  }

  for (i = 0; i < pb_graph_node->pb_type->num_modes; i++) {
    for (j = 0; j < pb_graph_node->pb_type->modes[i].num_pb_type_children;
        j++) {
      for (k = 0;
          k
              < pb_graph_node->pb_type->modes[i].pb_type_children[j].num_pb;
          k++) {
        alloc_and_load_rr_graph_for_pb_graph_node(
            &pb_graph_node->child_pb_graph_nodes[i][j][k], arch, i);
      }
    }
  }

}

/* Allocate a LL_rr_node route structs for a given rr_graph
 * This is function is a copy of alloc_and_load_rr_node_route_structs
 * The major difference lies in removing the use of global variables 
 */
t_rr_node_route_inf*
 alloc_and_load_LL_rr_node_route_structs(int LL_num_rr_nodes) {

  t_rr_node_route_inf* LL_rr_node_route_inf = NULL;

  /* Allocates some extra information about each LL_rr_node that is used only   *
   * during routing.                                                         */

  int inode;

  LL_rr_node_route_inf = (t_rr_node_route_inf *) my_malloc(num_rr_nodes * sizeof(t_rr_node_route_inf));

  for (inode = 0; inode < num_rr_nodes; inode++) {
    LL_rr_node_route_inf[inode].prev_node = NO_PREVIOUS;
    LL_rr_node_route_inf[inode].prev_edge = NO_PREVIOUS;
    LL_rr_node_route_inf[inode].pres_cost = 1.;
    LL_rr_node_route_inf[inode].acc_cost = 1.;
    LL_rr_node_route_inf[inode].path_cost = HUGE_POSITIVE_FLOAT;
    LL_rr_node_route_inf[inode].target_flag = 0;
  }

  return LL_rr_node_route_inf;
}


/* Allocate and load a local rr_graph for a pb
 * 1. Find 
 */
void alloc_and_load_rr_graph_for_pb(INP t_pb* cur_pb, 
                                    INP const t_arch *arch) {

  /**
   * Structure: Model external routing and internal routing
   * 
   * 1.  Model external routing
   *   num input pins == num external sources for input pins, fully connect them to input pins (simulates external routing)
   *   num output pins == num external sinks for output pins, fully connect them to output pins (simulates external routing)
   *   num clock pins == num external sources for clock pins, fully connect them to clock pins (simulates external routing)
   * 2.  Model internal routing
   * 
   */
  /* make each LL_rr_node one correspond with pin and correspond with pin's index pin_count_in_cluster */
  int i, j, k, m, index, pb_graph_rr_index;
  int count_pins;
  t_pb_type * pb_type;
  t_pb_graph_node *pb_graph_node;
  int ipin;
  int LL_num_rr_nodes;
  t_rr_node* LL_rr_node = NULL;

  /* Create rr_graph */
  pb_type = cur_pb->pb_type;
  pb_graph_node = cur_pb->pb_graph_head;


  num_rr_nodes = pb_graph_node->total_pb_pins + pb_type->num_input_pins
      + pb_type->num_output_pins + pb_type->num_clock_pins;

  /* allocate memory for LL_rr_node resources + additional memory for any additional sources/sinks, 2x is an overallocation but guarantees that there will be enough sources/sinks available */
  LL_rr_node = (t_rr_node *) my_calloc(num_rr_nodes * 2, sizeof(t_rr_node));
  clb->pb->rr_graph = LL_rr_node;

  alloc_and_load_rr_graph_for_pb_graph_node(pb_graph_node, arch, 0);

  /*   Alloc and load rr_graph external sources and sinks */
  ext_input_rr_node_index = pb_graph_node->total_pb_pins;
  ext_output_rr_node_index = pb_type->num_input_pins
      + pb_graph_node->total_pb_pins;
  ext_clock_rr_node_index = pb_type->num_input_pins + pb_type->num_output_pins
      + pb_graph_node->total_pb_pins;
  max_ext_index = pb_type->num_input_pins + pb_type->num_output_pins
      + pb_type->num_clock_pins + pb_graph_node->total_pb_pins;

  /* Create source and sinks for the rr_graph */
  for (i = 0; i < pb_type->num_input_pins; i++) {
    index = i + pb_graph_node->total_pb_pins;
    LL_rr_node[index].type = SOURCE;
    LL_rr_node[index].fan_in = 0;
    LL_rr_node[index].num_edges = pb_type->num_input_pins;
    LL_rr_node[index].pack_intrinsic_cost = 1
        + (float) LL_rr_node[index].num_edges / 5; /* need to normalize better than 5 */
    LL_rr_node[index].edges = (int *) my_malloc(
        LL_rr_node[index].num_edges * sizeof(int));
    LL_rr_node[index].switches = (short *) my_calloc(rr_node[index].num_edges,
        sizeof(int));
    LL_rr_node[index].capacity = 1;
  }

  for (i = 0; i < pb_type->num_output_pins; i++) {
    index = i + pb_type->num_input_pins + pb_graph_node->total_pb_pins;
    LL_rr_node[index].type = SINK;
    LL_rr_node[index].fan_in = pb_type->num_output_pins;
    LL_rr_node[index].num_edges = 0;
    LL_rr_node[index].pack_intrinsic_cost = 1
        + (float) LL_rr_node[index].num_edges / 5; /* need to normalize better than 5 */
    LL_rr_node[index].capacity = 1;
  }

  for (i = 0; i < pb_type->num_clock_pins; i++) {
    index = i + pb_type->num_input_pins + pb_type->num_output_pins
        + pb_graph_node->total_pb_pins;
    LL_rr_node[index].type = SOURCE;
    LL_rr_node[index].fan_in = 0;
    LL_rr_node[index].num_edges = pb_type->num_clock_pins;
    LL_rr_node[index].pack_intrinsic_cost = 1
        + (float) LL_rr_node[index].num_edges / 5; /* need to normalize better than 5 */
    LL_rr_node[index].edges = (int *) my_malloc(
        LL_rr_node[index].num_edges * sizeof(int));
    LL_rr_node[index].switches = (short *) my_calloc(rr_node[index].num_edges,
        sizeof(int));
    LL_rr_node[index].capacity = 1;
  }

  /* Fill the edges */
  ipin = 0;
  for (i = 0; i < pb_graph_node->num_input_ports; i++) {
    for (j = 0; j < pb_graph_node->num_input_pins[i]; j++) {
      pb_graph_rr_index =
          pb_graph_node->input_pins[i][j].pin_count_in_cluster;
      for (k = 0; k < pb_type->num_input_pins; k++) {
        index = k + pb_graph_node->total_pb_pins;
        LL_rr_node[index].edges[ipin] = pb_graph_rr_index;
        LL_rr_node[index].switches[ipin] = arch->num_switches - 1;
      }
      LL_rr_node[pb_graph_rr_index].pack_intrinsic_cost = MAX_SHORT; /* using an input pin should be made costly */
      ipin++;
    }
  }

  /* Must attach output pins to input pins because if a connection cannot fit using intra-cluster routing, it can also use external routing */
  for (i = 0; i < pb_graph_node->num_output_ports; i++) {
    for (j = 0; j < pb_graph_node->num_output_pins[i]; j++) {
      count_pins = pb_graph_node->output_pins[i][j].num_output_edges
          + pb_type->num_output_pins + pb_type->num_input_pins;
      pb_graph_rr_index =
          pb_graph_node->output_pins[i][j].pin_count_in_cluster;
      LL_rr_node[pb_graph_rr_index].edges = (int *) my_realloc(
          LL_rr_node[pb_graph_rr_index].edges,
          (count_pins) * sizeof(int));
      LL_rr_node[pb_graph_rr_index].switches = (short *) my_realloc(
          LL_rr_node[pb_graph_rr_index].switches,
          (count_pins) * sizeof(int));

      ipin = 0;
      for (k = 0; k < pb_graph_node->num_input_ports; k++) {
        for (m = 0; m < pb_graph_node->num_input_pins[k]; m++) {
          index =
              pb_graph_node->input_pins[k][m].pin_count_in_cluster;
          LL_rr_node[pb_graph_rr_index].edges[ipin
              + pb_graph_node->output_pins[i][j].num_output_edges] =
              index;
          LL_rr_node[pb_graph_rr_index].switches[ipin
              + pb_graph_node->output_pins[i][j].num_output_edges] =
              arch->num_switches - 1;
          ipin++;
        }
      }
      for (k = 0; k < pb_type->num_output_pins; k++) {
        index = k + pb_type->num_input_pins
            + pb_graph_node->total_pb_pins;
        LL_rr_node[pb_graph_rr_index].edges[k + pb_type->num_input_pins
            + pb_graph_node->output_pins[i][j].num_output_edges] =
            index;
        LL_rr_node[pb_graph_rr_index].switches[k + pb_type->num_input_pins
            + pb_graph_node->output_pins[i][j].num_output_edges] =
            arch->num_switches - 1;
      }
      LL_rr_node[pb_graph_rr_index].num_edges += pb_type->num_output_pins
          + pb_type->num_input_pins;
      LL_rr_node[pb_graph_rr_index].pack_intrinsic_cost = 1
          + (float) LL_rr_node[pb_graph_rr_index].num_edges / 5; /* need to normalize better than 5 */
    }
  }

  ipin = 0;
  for (i = 0; i < pb_graph_node->num_clock_ports; i++) {
    for (j = 0; j < pb_graph_node->num_clock_pins[i]; j++) {
      for (k = 0; k < pb_type->num_clock_pins; k++) {
        index = k + pb_type->num_input_pins + pb_type->num_output_pins
            + pb_graph_node->total_pb_pins;
        pb_graph_rr_index =
            pb_graph_node->clock_pins[i][j].pin_count_in_cluster;
        LL_rr_node[index].edges[ipin] = pb_graph_rr_index;
        LL_rr_node[index].switches[ipin] = arch->num_switches - 1;
      }
      ipin++;
    }
  }

  alloc_and_load_LL_rr_node_route_structs();
  num_nets_in_cluster = 0;

  return;
}

void free_LL_rr_node_route_structs(t_rr_node_route_inf *LL_rr_node_route_inf) { /* [0..num_rr_nodes-1] */

  /* Frees the extra information about each LL_rr_node that is needed only      *
   * during routing.                                                         */

  free(LL_rr_node_route_inf);
  LL_rr_node_route_inf = NULL; /* Mark as free */
}


void free_LL_rr_graph_for_one_pb(INP t_pb* cur_pb,
                                 IN t_rr_node_route_inf *LL_rr_node_route_inf,
                                 boolean free_local_rr_graph) {
  int i;

  free_LL_rr_node_route_structs(LL_rr_node_route_inf);
  if(free_local_rr_graph == TRUE) {
    for (i = 0; i < cur_pb->num_rr_nodes; i++) {
      if (cur_pb->rr_graph[i].edges != NULL) {
        free(cur_pb->rr_graph[i].edges);
      }
      if (cur_pb->rr_graph[i].switches != NULL) {
        free(cur_pb->rr_graph[i].switches);
      }
    }
    free(cur_pb->rr_graph);
  }

  return;
}


