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

void init_one_rr_node_pack_cost_for_phy_graph_node(INP t_pb_graph_pin* cur_pb_graph_pin,
                                                   INOUTP t_rr_graph* local_rr_graph,
                                                   IN int cur_rr_node_index,
                                                   enum PORTS port_type) {
  switch (port_type) {
  case IN_PORT:
    /* Routing costs : INPUT pins 
     * need to normalize better than 5 and 10, bias router to use earlier inputs pins 
     */
    local_rr_graph->rr_node[cur_rr_node_index].pack_intrinsic_cost = 1 + (float) local_rr_graph->rr_node[cur_rr_node_index].num_edges / 5 
                                                                       + ((float)cur_pb_graph_pin->pin_number / (float)cur_pb_graph_pin->port->num_pins) / (float)10; 
    break;
  case OUT_PORT:
    /* Routing costs : OUTPUT pins  
     * need to normalize better than 5 
     */
    local_rr_graph->rr_node[cur_rr_node_index].pack_intrinsic_cost = 1 + (float) local_rr_graph->_rr_node[cur_rr_node_index].num_edges / 5; 
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, 
               "(File:%s, [LINE%d]) Invalid rr_node_type (%d)! \n",
               __FILE__, __LINE__, pb_graph_pin_type); 
    exit(1);
  }
}

/* initialize a rr_node in a rr_graph of phyical pb_graph_node */
void init_one_rr_node_for_phy_pb_graph_node(INP t_pb_graph_pin* cur_pb_graph_pin,
                                            INOUTP t_rr_graph* local_rr_graph,
                                            IN int cur_rr_node_index,
                                            int phy_mode_index, 
                                            t_rr_type rr_node_type) {

  switch (rr_node_type) {
  case INTRA_CLUSTER_EDGE: 
    /* Link the rr_node to the pb_graph_pin*/
    cur_pb_graph_pin->rr_node_index_physical_pb = cur_rr_node_index;
    local_rr_graph->rr_node[cur_rr_node_index].pb_graph_pin = cur_pb_graph_pin;
    /* Get the number of input edges that belong to physical mode only! */
    local_rr_graph->rr_node[cur_rr_node_index].fan_in = count_pb_graph_node_input_edge_in_phy_mode(cur_pb_graph_pin, phy_mode_index);
    /* Get the number of output edges that belong to physical mode only! */
    local_rr_graph->rr_node[cur_rr_node_index].num_edges = count_pb_graph_node_output_edge_in_phy_mode(cur_pb_graph_pin, phy_mode_index);
    /* Routing costs */ 
    init_one_rr_node_pack_cost_for_phy_graph_node(cur_pb_graph_pin, local_rr_graph, cur_rr_node_index, cur_pb_graph_pin->port->type);
    break;
  case SOURCE:
    /* SOURCE only has one output and zero inputs */
    local_rr_graph->rr_node[cur_rr_node_index].pb_graph_pin = cur_pb_graph_pin;
    local_rr_graph->rr_node[cur_rr_node_index].fan_in = 0;
    local_rr_graph->rr_node[cur_rr_node_index].num_edges = 1;
    /* Routing costs : SOURCE */
    init_one_rr_node_pack_cost_for_phy_graph_node(cur_pb_graph_pin, local_rr_graph, cur_rr_node_index, OUT_PORT);
    break;
  case SINK:
    /* SINK only has one input and zero outputs */
    local_rr_graph->rr_node[cur_rr_node_index].pb_graph_pin = cur_pb_graph_pin;
    local_rr_graph->rr_node[cur_rr_node_index].fan_in = 1;
    local_rr_graph->rr_node[cur_rr_node_index].num_edges = 0;
    /* Routing costs : SINK */
    local_rr_graph->rr_node[cur_rr_node_index].pack_intrinsic_cost = 1;
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, 
               "(File:%s, [LINE%d]) Invalid rr_node_type (%d)! \n",
               __FILE__, __LINE__, rr_node_type); 
    exit(1);
  }

  /* Routing connectivity */
  local_rr_graph->rr_node[cur_rr_node_index].edges = (int *) my_calloc(local_rr_graph->rr_node[cur_rr_node_index].num_edges, sizeof(int));
  local_rr_graph->rr_node[cur_rr_node_index].switches = (short *) my_calloc(local_rr_graph->rr_node[cur_rr_node_index].num_edges,  sizeof(short));
  local_rr_graph->rr_node[cur_rr_node_index].net_num = OPEN;
  local_rr_graph->rr_node[cur_rr_node_index].prev_node = OPEN;
  local_rr_graph->rr_node[cur_rr_node_index].prev_edge = OPEN;

  local_rr_graph->rr_node[cur_rr_node_index].capacity = 1;
  local_rr_graph->rr_node[cur_rr_node_index].type = rr_node_type;
  
  return;
} 

/* Connect a rr_node in a rr_graph of phyical pb_graph_node,
 * Assign the edges and switches 
 */
void connect_one_rr_node_for_phy_pb_graph_node(INP t_pb_graph_pin* cur_pb_graph_pin,
                                               INOUTP t_rr_graph* local_rr_graph,
                                               IN int cur_rr_node_index,
                                               int phy_mode_index, 
                                               t_rr_type rr_node_type) {
  int iedge;

  /* Check: if type matches! */
  assert(rr_node_type == local_rr_graph->rr_node[cur_rr_node_index].type);

  switch (rr_node_type) {
  case INTRA_CLUSTER_EDGE: 
    /* Check out all the output_edges belonging to the same physical mode */
    for (iedge = 0; iedge < cur_pb_graph_pin->num_output_edges; iedge++) {
      check_pb_graph_edge(*(cur_pb_graph_pin->output_edges[iedge]));
      if (phy_mode_index == cur_pb_graph_pin->output_edges[iedge]->interconnect->parent_mode_index) {
        local_rr_graph->rr_node[cur_rr_node_index].edges[iedge] = cur_pb_graph_pin->output_edges[iedge]->output_pins[0]->rr_node_index_physical_pb;   
        local_rr_graph->rr_node[cur_rr_node_index].switches[iedge] = local_rr_graph->delayless_switch_index;   
      }
    }
    break;
  case SOURCE:
    /* Connect the SOURCE nodes to the rr_node of cur_pb_graph_pin */
    assert (1 == local_rr_graph->rr_node[cur_rr_node_index].num_edges);
    local_rr_graph->rr_node[cur_rr_node_index].edges[0] = cur_pb_graph_pin->rr_node_index_physical_pb;
    assert (1 == local_rr_graph->rr_node[cur_rr_node_index].num_switches);
    local_rr_graph->rr_node[cur_rr_node_index].switches[0] = local_rr_graph->delayless_switch_index;
    break;
  case SINK:
    /* Connect the rr_node of cur_pb_graph_pin to the SINK */
    assert (1 == local_rr_graph->rr_node[cur_rr_node_index].fan_in);
    assert (0 == local_rr_graph->rr_node[cur_rr_node_index].num_edges);
    assert (1 == local_rr_graph->rr_node[cur_pb_graph_pin->rr_node_index_physical_pb].num_edges);
    local_rr_graph->rr_node[cur_pb_graph_pin->rr_node_index_physical_pb].edges[0] = cur_rr_node_index;
    local_rr_graph->rr_node[cur_pb_graph_pin->rr_node_index_physical_pb].switches[0] = local_rr_graph->delayless_switch_index;
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, 
               "(File:%s, [LINE%d]) Invalid rr_node_type (%d)! \n",
               __FILE__, __LINE__, rr_node_type); 
    exit(1);
  }
  
  return;
} 


/* Recursively configure all the rr_nodes in the rr_graph
 * Initialize the routing cost, fan-in rr_nodes and fan-out rr_nodes, and switches  
 */
int rec_init_rr_graph_for_phy_pb_graph_node(INP t_pb_graph_node* cur_pb_graph_node, 
                                            INOUTP t_rr_graph* local_rr_graph,
                                            int cur_rr_node_index) {
  boolean is_top_pb_graph_node = (boolean) (NULL == cur_pb_graph_node->parent_pb_graph_node);
  boolean is_primitive_pb_graph_node = (boolean) (NULL != cur_pb_graph_node->pb_type->spice_model);
  int phy_mode_idx = -1;
  int parent_phy_mode_idx = -1;
  int iport, ipin;
  int ichild, ipb;

  /* Find the physical mode in the next level! */
  phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->pb_type));
  parent_phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->parent_pb_graph_node->pb_type));

  /* Configure rr_nodes with the information of pb_graph_pin  */
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; ipin++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
      init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                             cur_rr_node_index, phy_mode_index, INTRA_CLUSTER_EDGE);
      cur_rr_node_index++;
    }
  }

  /* Importantly, the interconnection for output ports belong to the parent pb_graph_node */
  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; ipin++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                             cur_rr_node_index, parent_phy_mode_index, INTRA_CLUSTER_EDGE);
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
                                               cur_rr_node_index, parent_phy_mode_index, SINK);

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
                                               cur_rr_node_index, parent_phy_mode_index, SOURCE);

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

    return cur_rr_node_index;
  }

  /* Go recursively to the lower levels */
  for (ichild = 0; ichild < cur_pb_graph_node->pb_type->modes[phy_mode_idx].num_pb_type_children; ichild++) {
    /* num_pb is the number of such pb_type in a physical mode*/
    for (ipb = 0; ipb < cur_pb_graph_node->pb_type->modes[phy_mode_idx].pb_type_children[ichild].num_pb; ipb++) {
      cur_num_rr_node_index += rec_init_rr_graph_for_phy_pb_graph_node(&cur_pb_graph_node->child_pb_graph_nodes[phy_mode_idx][ichild][ipb],
                                                                       local_rr_graph, cur_num_rr_node_index);
    }
  }

  return cur_rr_node_index;
}

/* Recursively connect all the rr_nodes in the rr_graph
 * output_edges, output_switches 
 */
int rec_connect_rr_graph_for_phy_pb_graph_node(INP t_pb_graph_node* cur_pb_graph_node, 
                                               INOUTP t_rr_graph* local_rr_graph,
                                               int cur_rr_node_index) {
  boolean is_top_pb_graph_node = (boolean) (NULL == cur_pb_graph_node->parent_pb_graph_node);
  boolean is_primitive_pb_graph_node = (boolean) (NULL != cur_pb_graph_node->pb_type->spice_model);
  int phy_mode_idx = -1;
  int parent_phy_mode_idx = -1;
  int iport, ipin;
  int ichild, ipb;

  /* Find the physical mode in the next level! */
  phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->pb_type));
  parent_phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->parent_pb_graph_node->pb_type));

  /* Configure rr_nodes with the information of pb_graph_pin  */
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; ipin++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
     connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                             cur_rr_node_index, phy_mode_index, INTRA_CLUSTER_EDGE);
      cur_rr_node_index++;
    }
  }

  /* Importantly, the interconnection for output ports belong to the parent pb_graph_node */
  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; ipin++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                             cur_rr_node_index, parent_phy_mode_index, INTRA_CLUSTER_EDGE);
      cur_rr_node_index++;
    }
  }

  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; ipin++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
      connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->clock_pins[iport][ipin], local_rr_graph,  
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
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                               cur_rr_node_index, phy_mode_index, SOURCE);

        cur_rr_node_index++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * output_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_output_ports; ipin++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                                  cur_rr_node_index, parent_phy_mode_index, SINK);

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
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                               cur_rr_node_index, parent_phy_mode_index, SOURCE);

        cur_rr_node_index++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * input_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_input_ports; ipin++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                               cur_rr_node_index, phy_mode_index, SINK);

        cur_rr_node_index++;
      }
    }
    /* Finish adding SOURCE and SINKs */

    return cur_rr_node_index;
  }

  /* Go recursively to the lower levels */
  for (ichild = 0; ichild < cur_pb_graph_node->pb_type->modes[phy_mode_idx].num_pb_type_children; ichild++) {
    /* num_pb is the number of such pb_type in a physical mode*/
    for (ipb = 0; ipb < cur_pb_graph_node->pb_type->modes[phy_mode_idx].pb_type_children[ichild].num_pb; ipb++) {
      cur_num_rr_node_index += rec_connect_rr_graph_for_phy_pb_graph_node(&cur_pb_graph_node->child_pb_graph_nodes[phy_mode_idx][ichild][ipb],
                                                                          local_rr_graph, cur_num_rr_node_index);
    }
  }

  return cur_rr_node_index;
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
                                                  OUTP t_rr_graph* local_rr_graph) {

  int index, phy_pb_num_rr_nodes;

  /* Count the number of rr_nodes that are required */
  phy_pb_num_rr_nodes = rec_count_rr_graph_nodes_for_phy_pb_graph_node(top_pb_graph_node);
   
  /* Allocate rr_graph */
  alloc_and_load_rr_graph_rr_node(local_rr_graph, phy_pb_num_rr_nodes);

  /* Fill basic information for the rr_graph  */ 
  index = rec_init_rr_graph_for_phy_pb_graph_node(top_pb_graph_node, local_rr_graph, 0);
  assert (index == local_rr_graph->num_rr_nodes); 

  /* Fill edges and switches for the rr_graph */
  index = rec_connect_rr_graph_for_phy_pb_graph_node(top_pb_graph_node, local_rr_graph, 0);
  assert (index == local_rr_graph->num_rr_nodes); 

  return;
}



/* Allocate and load a local rr_graph for a pb
 * 1. Find 
 */
void alloc_and_load_rr_graph_for_phy_pb(INP t_phy_pb* cur_phy_pb) {

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

  /* Allocate rr_graph*/
  cur_phy_pb->rr_graph = (t_rr_graph*) my_calloc(1, sizeof(t_rr_graph));

  /* Create rr_graph */
  alloc_and_load_rr_graph_for_phy_pb_graph_node(cur_phy_pb->pb_graph_head, cur_phy_pb->rr_graph);

  /* Allocate structs routing information */
  alloc_and_load_rr_graph_route_structs(cur_phy_pb->rr_graph);

  return;
}

