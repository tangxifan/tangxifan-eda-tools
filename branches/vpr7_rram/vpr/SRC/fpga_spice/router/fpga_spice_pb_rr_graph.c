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
#include "route_common.h"

/* Include SPICE support headers*/
#include "linkedlist.h"
#include "fpga_spice_types.h"
#include "fpga_spice_utils.h"
#include "fpga_spice_rr_graph_utils.h"
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
                                                   int cur_rr_node_index,
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
    local_rr_graph->rr_node[cur_rr_node_index].pack_intrinsic_cost = 1 + (float) local_rr_graph->rr_node[cur_rr_node_index].num_edges / 5; 
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, 
               "(File:%s, [LINE%d]) Invalid rr_node_type (%d)! \n",
               __FILE__, __LINE__, port_type); 
    exit(1);
  }
}


/* Override the fan-in and fan-out for a top/primitive pb_graph_node */
void override_one_rr_node_for_top_primitive_phy_pb_graph_node(INP t_pb_graph_pin* cur_pb_graph_pin,
                                                              INOUTP t_rr_graph* local_rr_graph,
                                                              int cur_rr_node_index,
                                                              boolean is_top_pb_graph_node,
                                                              boolean is_primitive_pb_graph_node) {
  /* check : must be either a top_pb_graph_node or a primitive_pb_graph_node */
  if ((FALSE == is_top_pb_graph_node) && (FALSE == is_primitive_pb_graph_node)) {
    return;
  }

  /* depends on the port type  */
  switch (cur_pb_graph_pin->port->type) {
  case IN_PORT:
    if (TRUE == is_top_pb_graph_node) {
      /* Top-level IN_PORT should only has 1 fan-in */
      local_rr_graph->rr_node[cur_rr_node_index].fan_in = 1;
      return;
    }
    if (TRUE == is_primitive_pb_graph_node) {
      /* Primitive-level IN_PORT should only has 1 output edge */
      local_rr_graph->rr_node[cur_rr_node_index].num_edges = 1;
      return;
    }
    break;
  case OUT_PORT:
    if (TRUE == is_top_pb_graph_node) {
      /* Top-level OUT_PORT should only has 1 output edge */
      local_rr_graph->rr_node[cur_rr_node_index].num_edges = 1;
      return;
    }
    if (TRUE == is_primitive_pb_graph_node) {
      /* Primitive-level OUT_PORT should only has 1 fan-in */
      local_rr_graph->rr_node[cur_rr_node_index].fan_in = 1;
      return;
    }
    break;    
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, 
               "(File:%s, [LINE%d]) Invalid rr_node_type (%d)! \n",
               __FILE__, __LINE__, cur_pb_graph_pin->port->type); 
    exit(1);
  }

  return;
}

/* initialize a rr_node in a rr_graph of phyical pb_graph_node */
void init_one_rr_node_for_phy_pb_graph_node(INP t_pb_graph_pin* cur_pb_graph_pin,
                                            INOUTP t_rr_graph* local_rr_graph,
                                            int cur_rr_node_index,
                                            int fan_in_phy_mode_index, 
                                            int fan_out_phy_mode_index, 
                                            t_rr_type rr_node_type,
                                            boolean is_top_pb_graph_node,
                                            boolean is_primitive_pb_graph_node) {

  switch (rr_node_type) {
  case INTRA_CLUSTER_EDGE: 
    /* Link the rr_node to the pb_graph_pin*/
    cur_pb_graph_pin->rr_node_index_physical_pb = cur_rr_node_index;
    local_rr_graph->rr_node[cur_rr_node_index].pb_graph_pin = cur_pb_graph_pin;
    /* Get the number of input edges that belong to physical mode only! */
    local_rr_graph->rr_node[cur_rr_node_index].fan_in = count_pb_graph_node_input_edge_in_phy_mode(cur_pb_graph_pin, fan_in_phy_mode_index);
    /* Get the number of output edges that belong to physical mode only! */
    local_rr_graph->rr_node[cur_rr_node_index].num_edges = count_pb_graph_node_output_edge_in_phy_mode(cur_pb_graph_pin, fan_out_phy_mode_index);
    /* Override for special rr_nodes : at top-level and primitive nodes */
    override_one_rr_node_for_top_primitive_phy_pb_graph_node(cur_pb_graph_pin, local_rr_graph, cur_rr_node_index, 
                                                             is_top_pb_graph_node, is_primitive_pb_graph_node);
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
                                               int cur_rr_node_index,
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
    assert (0 == local_rr_graph->rr_node[cur_rr_node_index].fan_in);
    assert (1 == local_rr_graph->rr_node[cur_rr_node_index].num_edges);
    local_rr_graph->rr_node[cur_rr_node_index].edges[0] = cur_pb_graph_pin->rr_node_index_physical_pb;
    local_rr_graph->rr_node[cur_rr_node_index].switches[0] = local_rr_graph->delayless_switch_index;
    break;
  case SINK:
    /* Connect the rr_node of cur_pb_graph_pin to the SINK */
    assert (1 == local_rr_graph->rr_node[cur_rr_node_index].fan_in);
    assert (0 == local_rr_graph->rr_node[cur_rr_node_index].num_edges);
    if (1 != local_rr_graph->rr_node[cur_pb_graph_pin->rr_node_index_physical_pb].num_edges) {
    assert (1 == local_rr_graph->rr_node[cur_pb_graph_pin->rr_node_index_physical_pb].num_edges);
    }
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
void rec_init_rr_graph_for_phy_pb_graph_node(INP t_pb_graph_node* cur_pb_graph_node, 
                                            INOUTP t_rr_graph* local_rr_graph,
                                            int* cur_rr_node_index) {
  boolean is_top_pb_graph_node = (boolean) (NULL == cur_pb_graph_node->parent_pb_graph_node);
  boolean is_primitive_pb_graph_node = (boolean) (NULL != cur_pb_graph_node->pb_type->spice_model);
  int phy_mode_idx = -1;
  int parent_phy_mode_idx = -1;
  int iport, ipin;
  int ichild, ipb;

  /* Find the physical mode in the next level! */
  phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->pb_type));
  /* There is no parent mode index for top-level node */
  if (FALSE == is_top_pb_graph_node) { 
    parent_phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->parent_pb_graph_node->pb_type));
  } else {
    parent_phy_mode_idx = phy_mode_idx;
  }

  /* Configure rr_nodes with the information of pb_graph_pin  */
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
      init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                             *cur_rr_node_index, parent_phy_mode_idx, phy_mode_idx, INTRA_CLUSTER_EDGE,
                                             is_top_pb_graph_node, is_primitive_pb_graph_node);
      (*cur_rr_node_index)++;
    }
  }

  /* Importantly, the interconnection for output ports belong to the parent pb_graph_node */
  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                             *cur_rr_node_index, phy_mode_idx, parent_phy_mode_idx, INTRA_CLUSTER_EDGE,
                                             is_top_pb_graph_node, is_primitive_pb_graph_node);
      (*cur_rr_node_index)++;
    }
  }

  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
      init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->clock_pins[iport][ipin], local_rr_graph,  
                                             *cur_rr_node_index, parent_phy_mode_idx, phy_mode_idx, INTRA_CLUSTER_EDGE,
                                             is_top_pb_graph_node, is_primitive_pb_graph_node);
      (*cur_rr_node_index)++;
    }
  }

  /* check if this is a top pb_graph_node */
  if ((TRUE == is_top_pb_graph_node)) {
    /* Configure SOURCE and SINK rr_node: 
     * input_pins should have a SOURCE node
     */
    for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                               *cur_rr_node_index, phy_mode_idx, phy_mode_idx, SOURCE,
                                               is_top_pb_graph_node, is_primitive_pb_graph_node);

        (*cur_rr_node_index)++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * clock_pins should have a SOURCE node
     */
    for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->clock_pins[iport][ipin], local_rr_graph,  
                                               *cur_rr_node_index, phy_mode_idx, phy_mode_idx, SOURCE,
                                               is_top_pb_graph_node, is_primitive_pb_graph_node);

        (*cur_rr_node_index)++;
      }
    }

    /* Configure SOURCE and SINK rr_node: 
     * output_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                               *cur_rr_node_index, phy_mode_idx, phy_mode_idx, SINK,
                                               is_top_pb_graph_node, is_primitive_pb_graph_node);

        (*cur_rr_node_index)++;
      }
    }
    /* Finish adding SOURCE and SINKs */
  }

  /* Return when this is a primitive node */
  if (TRUE == is_primitive_pb_graph_node) {
    /* Configure SOURCE and SINK rr_node: 
     * output_pins should have a SOURCE node
     */
    for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                               *cur_rr_node_index, phy_mode_idx, parent_phy_mode_idx,  SOURCE,
                                               is_top_pb_graph_node, is_primitive_pb_graph_node);

        (*cur_rr_node_index)++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * input_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                               *cur_rr_node_index, parent_phy_mode_idx, phy_mode_idx, SINK,
                                               is_top_pb_graph_node, is_primitive_pb_graph_node);

        (*cur_rr_node_index)++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * clock_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
        init_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->clock_pins[iport][ipin], local_rr_graph,  
                                               *cur_rr_node_index, parent_phy_mode_idx, phy_mode_idx, SINK,
                                               is_top_pb_graph_node, is_primitive_pb_graph_node);

        (*cur_rr_node_index)++;
      }
    }
    /* Finish adding SOURCE and SINKs */

    return;
  }

  /* Go recursively to the lower levels */
  for (ichild = 0; ichild < cur_pb_graph_node->pb_type->modes[phy_mode_idx].num_pb_type_children; ichild++) {
    /* num_pb is the number of such pb_type in a physical mode*/
    for (ipb = 0; ipb < cur_pb_graph_node->pb_type->modes[phy_mode_idx].pb_type_children[ichild].num_pb; ipb++) {
      rec_init_rr_graph_for_phy_pb_graph_node(&cur_pb_graph_node->child_pb_graph_nodes[phy_mode_idx][ichild][ipb],
                                              local_rr_graph, cur_rr_node_index);
    }
  }

  return;
}

/* Recursively connect all the rr_nodes in the rr_graph
 * output_edges, output_switches 
 */
void rec_connect_rr_graph_for_phy_pb_graph_node(INP t_pb_graph_node* cur_pb_graph_node, 
                                                INOUTP t_rr_graph* local_rr_graph,
                                                int* cur_rr_node_index) {
  boolean is_top_pb_graph_node = (boolean) (NULL == cur_pb_graph_node->parent_pb_graph_node);
  boolean is_primitive_pb_graph_node = (boolean) (NULL != cur_pb_graph_node->pb_type->spice_model);
  int phy_mode_idx = -1;
  int parent_phy_mode_idx = -1;
  int iport, ipin;
  int ichild, ipb;

  /* Find the physical mode in the next level! */
  phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->pb_type));
  /* There is no parent mode index for top-level node */
  if (FALSE == is_top_pb_graph_node) { 
    parent_phy_mode_idx = find_pb_type_physical_mode_index(*(cur_pb_graph_node->parent_pb_graph_node->pb_type));
  } else {
    parent_phy_mode_idx = phy_mode_idx;
  }

  /* Configure rr_nodes with the information of pb_graph_pin  */
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
      connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                                *cur_rr_node_index, phy_mode_idx, INTRA_CLUSTER_EDGE);
      (*cur_rr_node_index)++;
    }
  }

  /* Importantly, the interconnection for output ports belong to the parent pb_graph_node */
  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                                *cur_rr_node_index, parent_phy_mode_idx, INTRA_CLUSTER_EDGE);
      (*cur_rr_node_index)++;
    }
  }

  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
      connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->clock_pins[iport][ipin], local_rr_graph,  
                                                *cur_rr_node_index, phy_mode_idx, INTRA_CLUSTER_EDGE);
      (*cur_rr_node_index)++;
    }
  }

  /* check if this is a top pb_graph_node */
  if ((TRUE == is_top_pb_graph_node)) {
    /* Configure SOURCE and SINK rr_node: 
     * input_pins should have a SOURCE node
     */
    for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                                  *cur_rr_node_index, phy_mode_idx, SOURCE);

        (*cur_rr_node_index)++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * clock_pins should have a SOURCE node
     */
    for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->clock_pins[iport][ipin], local_rr_graph,  
                                                  *cur_rr_node_index, phy_mode_idx, SOURCE);

        (*cur_rr_node_index)++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * output_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                                  *cur_rr_node_index, parent_phy_mode_idx, SINK);

        (*cur_rr_node_index)++;
      }
    }
    /* Finish adding SOURCE and SINKs */
  }

  /* Return when this is a primitive node */
  if (TRUE == is_primitive_pb_graph_node) {
    /* Configure SOURCE and SINK rr_node: 
     * output_pins should have a SOURCE node
     */
    for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->output_pins[iport][ipin], local_rr_graph,  
                                                  *cur_rr_node_index, parent_phy_mode_idx, SOURCE);

        (*cur_rr_node_index)++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * input_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->input_pins[iport][ipin], local_rr_graph,  
                                                  *cur_rr_node_index, phy_mode_idx, SINK);

        (*cur_rr_node_index)++;
      }
    }
    /* Configure SOURCE and SINK rr_node: 
     * clock_pins should have a SINK node 
     */
    for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
        connect_one_rr_node_for_phy_pb_graph_node(&cur_pb_graph_node->clock_pins[iport][ipin], local_rr_graph,  
                                                  *cur_rr_node_index, phy_mode_idx, SINK);

        (*cur_rr_node_index)++;
      }
    }
    /* Finish adding SOURCE and SINKs */

    return;
  }

  /* Go recursively to the lower levels */
  for (ichild = 0; ichild < cur_pb_graph_node->pb_type->modes[phy_mode_idx].num_pb_type_children; ichild++) {
    /* num_pb is the number of such pb_type in a physical mode*/
    for (ipb = 0; ipb < cur_pb_graph_node->pb_type->modes[phy_mode_idx].pb_type_children[ichild].num_pb; ipb++) {
      rec_connect_rr_graph_for_phy_pb_graph_node(&cur_pb_graph_node->child_pb_graph_nodes[phy_mode_idx][ichild][ipb],
                                                 local_rr_graph, cur_rr_node_index);
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
                                                  OUTP t_rr_graph* local_rr_graph) {

  int phy_pb_num_rr_nodes, check_point;

  /* Count the number of rr_nodes that are required */
  phy_pb_num_rr_nodes = rec_count_rr_graph_nodes_for_phy_pb_graph_node(top_pb_graph_node);

  /* Allocate rr_graph */
  alloc_and_load_rr_graph_rr_node(local_rr_graph, phy_pb_num_rr_nodes);

  /* Fill basic information for the rr_graph  */ 
  check_point = 0;
  rec_init_rr_graph_for_phy_pb_graph_node(top_pb_graph_node, local_rr_graph, &check_point);
  assert (check_point == local_rr_graph->num_rr_nodes); 

  /* Fill edges and switches for the rr_graph */
  check_point = 0;
  rec_connect_rr_graph_for_phy_pb_graph_node(top_pb_graph_node, local_rr_graph, &check_point);
  assert (check_point == local_rr_graph->num_rr_nodes); 

  return;
}

/* Find the vpack_nets used by this pb
 * And allocate an array for those nets and load it to pb_rr_graph
 */
void alloc_and_load_phy_pb_rr_graph_nets(INP t_pb* cur_op_pb,
                                         t_rr_graph* local_rr_graph,
                                         int L_num_vpack_nets, t_net* L_vpack_net) {
  /* Create an array labeling which vpack_net is used in this pb */
  int num_vpack_net_used_in_pb = 0;
  boolean* vpack_net_used_in_pb = NULL; 
  int inet, inode, net_index;
  
  /* Allocate */ 
  vpack_net_used_in_pb = (boolean*)my_malloc(sizeof(boolean) * L_num_vpack_nets);
  /* Initial to FALSE */
  for (inet = 0; inet < L_num_vpack_nets; inet++) { 
    vpack_net_used_in_pb[inet] = FALSE;
  }

  /* Check each net_name in the rr_graph of cur_op_pb */
  for (inode = 0; inode < cur_op_pb->num_rr_nodes; inode++) {
    /* only consider those has been mapped to a pb_graph_pin */
    if (NULL == cur_op_pb->rr_graph[inode].pb_graph_pin) {
      continue;
    }
    /* bypass unmapped rr_node */
    if (OPEN == cur_op_pb->rr_graph[inode].vpack_net_num) {
      continue;
    }
    /* Reach here, it means this net is used in this pb */
    assert(   (-1 < cur_op_pb->rr_graph[inode].vpack_net_num)
           && ( cur_op_pb->rr_graph[inode].vpack_net_num < L_num_vpack_nets));
    vpack_net_used_in_pb[cur_op_pb->rr_graph[inode].vpack_net_num] = TRUE; 
  } 

  /* Count the number of vpack_net used in this pb  */
  num_vpack_net_used_in_pb = 0;
  for (inet = 0; inet < L_num_vpack_nets; inet++) { 
    if (TRUE == vpack_net_used_in_pb[inet]) {
      num_vpack_net_used_in_pb++;
    } 
  }

  /* Allocate net for rr_graph */
  local_rr_graph->num_nets = num_vpack_net_used_in_pb;
  local_rr_graph->net = (t_net**) my_malloc(sizeof(t_net*) * local_rr_graph->num_nets);
  local_rr_graph->net_to_vpack_net_mapping = (int*) my_malloc(sizeof(int) * local_rr_graph->num_nets);
  
  /* Fill the net array and net_to_net_mapping */
  net_index = 0;
  for (inet = 0; inet < L_num_vpack_nets; inet++) { 
    if (TRUE == vpack_net_used_in_pb[inet]) {
      local_rr_graph->net[net_index] = &L_vpack_net[inet]; 
      local_rr_graph->net_to_vpack_net_mapping[net_index] = inet; 
      net_index++;
    }
  }
  assert( net_index == local_rr_graph->num_nets );

  return;
}

t_rr_node* get_rr_node_in_pb_rr_graph(t_pb* cur_op_pb,
                                      int net_index, t_rr_type rr_node_type) {
  int inode;
  t_rr_node* found_rr_node = NULL;
  int num_found = 0;
   
  for (inode = 0; inode < cur_op_pb->num_rr_nodes; inode++) {
    if ((rr_node_type == cur_op_pb->rr_graph[inode].type) 
       &&(net_index == cur_op_pb->rr_graph[inode].vpack_net_num)) {
      found_rr_node = &(cur_op_pb->rr_graph[inode]);
      num_found++;
    }
  }

  assert(1 == num_found); /* There should be only one results*/
  
  return found_rr_node;
} 

/* Load mapping information from an op_pb to the net_rr_terminals of a phy_pb rr_graph 
 * This function should do the following tasks:
 * 1. Find mapped pb_graph_pin in the rr_graph of cur_op_pb 
 * 2. Locate the pin-to-pin annotation in the pb_graph_pin of cur_op_pb
 *    and find the corresponding pb_graph_pin in local_rr_graph (phy_pb)
 * 3. Find the SOURCE and SINK rr_nodes related to the pb_graph_pin 
 * 4. Configure the net_rr_terminals with the SINK/SOURCE rr_nodes  
 */
void load_phy_pb_rr_graph_net_rr_terminals(INP t_pb* cur_op_pb,
                                           t_rr_graph* local_rr_graph) {
  int inet, isink, rr_node_vpack_net_name;
  t_rr_node* pb_rr_node = NULL;
  t_rr_type pb_rr_node_type;
  
  /* Check each net in the local_rr_graph,
   * Find the routing resource node in the pb_rr_graph of the cur_op_pb
   * and annotate in the local_rr_graph
   * assign the net_rr_terminal with the node index in the local_rr_graph
   */ 
  for (inet = 0; inet < local_rr_graph->num_nets; inet++) {
    /* SINK 0 is the SOURCE nodes  
     * The rest sink are SINK nodes 
     */
    for (isink = 0; isink < local_rr_graph->net[inet]->num_sinks + 1; isink++) {
      pb_rr_node_type = (0 == isink) ? SOURCE : SINK;
      rr_node_vpack_net_name = get_rr_graph_net_vpack_net_index(local_rr_graph, inet);
      pb_rr_node = get_rr_node_in_pb_rr_graph(cur_op_pb, rr_node_vpack_net_name, pb_rr_node_type); 
      /* If there is rr_node_index_physical_pb, this is also a pin in the physical mode */
      if (OPEN != pb_rr_node->pb_graph_pin->rr_node_index_physical_pb) {
        local_rr_graph->net_rr_terminals[inet][0] = pb_rr_node->pb_graph_pin->rr_node_index_physical_pb;
      } else {
        /* This is not a pin in the physical mode, find the mapped physical pb_graph_pin */
        local_rr_graph->net_rr_terminals[inet][0] = pb_rr_node->pb_graph_pin->physical_pb_graph_pin->rr_node_index_physical_pb; 
      }
    }
  }

  return;
}

/* Allocate and load a local rr_graph for a pb
 * 1. Allocate the rr_graph nodes and configure with pb_graph_node connectivity
 * 2. load all the routing statisitics required by the router
 * 3. load the net to be routed into the rr_graph  
 */
void alloc_and_load_rr_graph_for_phy_pb(INP t_pb* cur_op_pb,
                                        INP t_phy_pb* cur_phy_pb,
                                        int L_num_vpack_nets, t_net* L_vpack_net) {

  /* Allocate rr_graph*/
  cur_phy_pb->rr_graph = (t_rr_graph*) my_calloc(1, sizeof(t_rr_graph));

  /* Create rr_graph */
  alloc_and_load_rr_graph_for_phy_pb_graph_node(cur_phy_pb->pb_graph_node, cur_phy_pb->rr_graph);

  /* Build prev nodes list for rr_nodes */
  alloc_and_load_prev_node_list_rr_graph_rr_nodes(cur_phy_pb->rr_graph);

  /* Allocate structs routing information */
  alloc_and_load_rr_graph_route_structs(cur_phy_pb->rr_graph);

  /* Find the nets inside the pb and initialize the rr_graph */
  alloc_and_load_phy_pb_rr_graph_nets(cur_op_pb, cur_phy_pb->rr_graph,
                                      L_num_vpack_nets, L_vpack_net); 

  /* Allocate net_rr_terminals */
  alloc_rr_graph_net_rr_terminals(cur_phy_pb->rr_graph);
  /* Fill the net_rr_terminals with 
   * 1. pin-to-pin mapping in pb_graph_node in cur_op_pb
   * 2. rr_graph in the cur_op_pb
   */ 
  load_phy_pb_rr_graph_net_rr_terminals(cur_op_pb, cur_phy_pb->rr_graph); 

  return;
}

