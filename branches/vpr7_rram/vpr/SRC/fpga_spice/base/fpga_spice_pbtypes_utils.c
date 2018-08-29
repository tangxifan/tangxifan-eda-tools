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
#include "fpga_spice_types.h"
#include "fpga_spice_utils.h"
#include "fpga_spice_utils.h"
#include "fpga_spice_bitstream_utils.h"
#include "fpga_spice_pbtypes_utils.h"
#include "fpga_spice_globals.h"

/* Make sure the edge has only one input pin and output pin*/
void check_pb_graph_edge(t_pb_graph_edge pb_graph_edge) {
  assert(1 == pb_graph_edge.num_input_pins);
  assert(1 == pb_graph_edge.num_output_pins);

  return;
}

/* Check all the edges for a given pb_graph_pin*/
void check_pb_graph_pin_edges(t_pb_graph_pin pb_graph_pin) {
  int iedge;
 
  for (iedge = 0; iedge < pb_graph_pin.num_input_edges; iedge++) {
    check_pb_graph_edge(*(pb_graph_pin.input_edges[iedge]));
  }
  
  for (iedge = 0; iedge < pb_graph_pin.num_output_edges; iedge++) {
    check_pb_graph_edge(*(pb_graph_pin.output_edges[iedge]));
  }
 
  return;
}

void backup_one_pb_rr_node_pack_prev_node_edge(t_rr_node* pb_rr_node) {

  pb_rr_node->prev_node_in_pack = pb_rr_node->prev_node; 
  pb_rr_node->prev_edge_in_pack = pb_rr_node->prev_edge; 
  pb_rr_node->net_num_in_pack = pb_rr_node->net_num; 
  pb_rr_node->prev_node = OPEN; 
  pb_rr_node->prev_edge = OPEN; 

  return;
}

int find_parent_pb_type_child_index(t_pb_type* parent_pb_type,
                                    int mode_index,
                                    t_pb_type* child_pb_type) {
  int i;

  assert(NULL != parent_pb_type);
  assert(NULL != child_pb_type);
  assert((!(0 > mode_index))&&(mode_index < parent_pb_type->num_modes));

  for (i = 0; i < parent_pb_type->modes[mode_index].num_pb_type_children; i++) {
    if (child_pb_type == &(parent_pb_type->modes[mode_index].pb_type_children[i])) {
      assert(0 == strcmp(child_pb_type->name, parent_pb_type->modes[mode_index].pb_type_children[i].name));
      return i;
    }
  }
  
  return -1;
}

/* Rule in generating a unique name: 
 * name of current pb =  <parent_pb_name_tag>_<cur_pb_graph_node>[index]
 */
void gen_spice_name_tag_pb_rec(t_pb* cur_pb,
                               char* prefix) {
  char* prefix_rec = NULL; 
  int ipb, jpb, mode_index; 

  mode_index = cur_pb->mode;

  /* Free previous name_tag if there is */
  /* my_free(cur_pb->spice_name_tag); */

  /* Generate the name_tag */
  if ((0 < cur_pb->pb_graph_node->pb_type->num_modes)
    &&(NULL == cur_pb->pb_graph_node->pb_type->spice_model_name)) {
    prefix_rec = (char*)my_malloc(sizeof(char)*(strlen(prefix) + 1 + strlen(cur_pb->pb_graph_node->pb_type->name) + 1
                                              + strlen(my_itoa(cur_pb->pb_graph_node->placement_index)) + 7 + strlen(cur_pb->pb_graph_node->pb_type->modes[mode_index].name) + 2 ));
    sprintf(prefix_rec, "%s_%s[%d]_mode[%s]", 
            prefix, cur_pb->pb_graph_node->pb_type->name, cur_pb->pb_graph_node->placement_index, cur_pb->pb_graph_node->pb_type->modes[mode_index].name);
    cur_pb->spice_name_tag = my_strdup(prefix_rec);
  } else {
    assert((0 == cur_pb->pb_graph_node->pb_type->num_modes)
          ||(NULL != cur_pb->pb_graph_node->pb_type->spice_model_name));
    prefix_rec = (char*)my_malloc(sizeof(char)*(strlen(prefix) + 1 + strlen(cur_pb->pb_graph_node->pb_type->name) + 1
                                              + strlen(my_itoa(cur_pb->pb_graph_node->placement_index)) + 2 ));
    sprintf(prefix_rec, "%s_%s[%d]", 
            prefix, cur_pb->pb_graph_node->pb_type->name, cur_pb->pb_graph_node->placement_index);
    cur_pb->spice_name_tag = my_strdup(prefix_rec);
  }

  /* When reach the leaf, we directly return */
  /* Recursive until reach the leaf */
  if ((0 == cur_pb->pb_graph_node->pb_type->num_modes)
     ||(NULL == cur_pb->child_pbs)) {
    return;
  }
  for (ipb = 0; ipb < cur_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb->pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Refer to pack/output_clustering.c [LINE 392] */
      //if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) { 
        /* Try to simplify the name tag... to avoid exceeding the length of SPICE name (up to 1024 chars) */
        /* gen_spice_name_tag_pb_rec(&(cur_pb->child_pbs[ipb][jpb]),prefix); */
        gen_spice_name_tag_pb_rec(&(cur_pb->child_pbs[ipb][jpb]),prefix_rec); 
      //}
    }
  }
 
  my_free(prefix_rec);
 
  return;
}

/* Generate a unique name tag for each pb, 
 * to identify it in both SPICE netlist and Power Modeling.
 */
void gen_spice_name_tags_all_pbs() {
  int iblk;
  char* prefix = NULL;

  for (iblk = 0; iblk < num_blocks; iblk++) {
    prefix = (char*)my_malloc(sizeof(char)*(5 + strlen(my_itoa(block[iblk].x)) + 2 + strlen(my_itoa(block[iblk].y)) + 2));
    sprintf(prefix, "grid[%d][%d]", block[iblk].x, block[iblk].y);
    gen_spice_name_tag_pb_rec(block[iblk].pb, prefix);
    my_free(prefix);
  }

  return;
}

int find_pb_mapped_logical_block_rec(t_pb* cur_pb,
                                     t_spice_model* pb_spice_model, 
                                     char* pb_spice_name_tag) {
  int logical_block_index = OPEN;
  int mode_index, ipb, jpb;

  assert(NULL != cur_pb);

  if ((pb_spice_model == cur_pb->pb_graph_node->pb_type->spice_model)
    &&(0 == strcmp(cur_pb->spice_name_tag, pb_spice_name_tag))) {
    /* Return the logic block we may find */
    switch (pb_spice_model->type) {
    case SPICE_MODEL_LUT :
      /* Special for LUT... They have sub modes!!!*/
      assert(NULL != cur_pb->child_pbs);
      return cur_pb->child_pbs[0][0].logical_block; 
    case SPICE_MODEL_FF:
      assert(pb_spice_model == logical_block[cur_pb->logical_block].mapped_spice_model);
      return cur_pb->logical_block;
    case SPICE_MODEL_HARDLOGIC:
      if (NULL != cur_pb->child_pbs) {
        return cur_pb->child_pbs[0][0].logical_block; 
      } else {
        assert(pb_spice_model == logical_block[cur_pb->logical_block].mapped_spice_model);
        return cur_pb->logical_block;
      } 
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid spice model type!\n",
                 __FILE__, __LINE__);
      exit(1);
    }
  }
  
  /* Go recursively ... */
  mode_index = cur_pb->mode;
  if (0 == cur_pb->pb_graph_node->pb_type->num_modes) {
    return logical_block_index;
  }
  for (ipb = 0; ipb < cur_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb->pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Refer to pack/output_clustering.c [LINE 392] */
      if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
        logical_block_index = 
        find_pb_mapped_logical_block_rec(&(cur_pb->child_pbs[ipb][jpb]), pb_spice_model, pb_spice_name_tag);
        if (OPEN != logical_block_index) {
          return logical_block_index;
        }
      }
    }
  }
  
  return logical_block_index;
}

int find_grid_mapped_logical_block(int x, int y,
                                   t_spice_model* pb_spice_model,
                                   char* pb_spice_name_tag) {
  int logical_block_index = OPEN;
  int iblk;

  /* Find the grid usage */
  if (0 == grid[x][y].usage) { 
    return logical_block_index;
  } else {
    assert(0 < grid[x][y].usage);
    /* search each block */
    for (iblk = 0; iblk < grid[x][y].usage; iblk++) {
      /* Get the pb */
      logical_block_index = find_pb_mapped_logical_block_rec(block[grid[x][y].blocks[iblk]].pb,
                                                             pb_spice_model, pb_spice_name_tag);
      if (OPEN != logical_block_index) {
        return logical_block_index;
      }
    }
  }
  
  return logical_block_index;
}

void stats_pb_graph_node_port_pin_numbers(t_pb_graph_node* cur_pb_graph_node,
                                          int* num_inputs,
                                          int* num_outputs,
                                          int* num_clock_pins) {
  int iport;

  assert(NULL != cur_pb_graph_node);

  (*num_inputs) = 0;
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
    (*num_inputs) += cur_pb_graph_node->num_input_pins[iport];
  }
  (*num_outputs) = 0;
  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    (*num_outputs) += cur_pb_graph_node->num_output_pins[iport];
  }
  (*num_clock_pins) = 0;
  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
    (*num_clock_pins) += cur_pb_graph_node->num_clock_pins[iport];
  }

  return;
}

int find_pb_type_idle_mode_index(t_pb_type cur_pb_type) {
  int idle_mode_index = 0;
  int imode = 0;
  int num_idle_mode = 0;

  /* if we touch the leaf node */
  if (NULL != cur_pb_type.blif_model) {
    return 0;
  }

  if (0 == cur_pb_type.num_modes) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Intend to find the idle mode while cur_pb_type has 0 modes!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Corner case:
   * If there is only 1 mode available, it will be the idle_mode by default! 
   */
  if (1 == cur_pb_type.num_modes) {
    idle_mode_index = 0;
    num_idle_mode++;
    return idle_mode_index;
  } 
 
  /* Normal Condition: */ 
  for (imode = 0; imode < cur_pb_type.num_modes; imode++) {
    if (1 == cur_pb_type.modes[imode].define_idle_mode) {
      idle_mode_index = imode;
      num_idle_mode++;
    }
  } 

  assert(1 == num_idle_mode); 

  return idle_mode_index;
}

/* Find the physical mode index */
int find_pb_type_physical_mode_index(t_pb_type cur_pb_type) {
  int phy_mode_index = 0;
  int imode = 0;
  int num_phy_mode = 0;

  /* if we touch the leaf node */
  if (NULL != cur_pb_type.blif_model) {
    return 0;
  }

  if (0 == cur_pb_type.num_modes) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Intend to find the physical mode while cur_pb_type has 0 modes!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Corner case:
   * If there is only 1 mode available, it will be the physical_mode by default! 
   */
  if (1 == cur_pb_type.num_modes) {
    phy_mode_index = 0;
    num_phy_mode++;
    return phy_mode_index;
  } 
 
  /* Normal Condition: */ 
  for (imode = 0; imode < cur_pb_type.num_modes; imode++) {
    if (1 == cur_pb_type.modes[imode].define_physical_mode) {
      phy_mode_index = imode;
      num_phy_mode++;
    }
  } 
  assert(1 == num_phy_mode); 

  return phy_mode_index;
}

void mark_grid_type_pb_graph_node_pins_temp_net_num(int x, int y) {
  int iport, ipin, type_pin_index, class_id, pin_global_rr_node_id; 
  t_type_ptr type = NULL;
  t_pb_graph_node* top_pb_graph_node = NULL;
  int mode_index, ipb, jpb;

  /* Assert */
  assert((!(x < 0))&&(x < (nx + 2)));  
  assert((!(y < 0))&&(y < (ny + 2)));  

  type = grid[x][y].type;

  if (EMPTY_TYPE == type) {
    return; /* Bypass empty grid */
  }

  top_pb_graph_node = type->pb_graph_head;
  /* Input ports */
  for (iport = 0; iport < top_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_input_pins[iport]; ipin++) {
      top_pb_graph_node->input_pins[iport][ipin].temp_net_num = OPEN;
      type_pin_index = top_pb_graph_node->input_pins[iport][ipin].pin_count_in_cluster;
      class_id = type->pin_class[type_pin_index];
      assert(RECEIVER == type->class_inf[class_id].type);
      /* Find the pb net_num and update OPIN net_num */
      pin_global_rr_node_id = get_rr_node_index(x, y, IPIN, type_pin_index, rr_node_indices);
      if (OPEN == rr_node[pin_global_rr_node_id].net_num) {
        top_pb_graph_node->input_pins[iport][ipin].temp_net_num = OPEN;
        continue;
      } 
      top_pb_graph_node->input_pins[iport][ipin].temp_net_num = clb_to_vpack_net_mapping[rr_node[pin_global_rr_node_id].net_num];
    } 
  }
  /* clock ports */
  for (iport = 0; iport < top_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_clock_pins[iport]; ipin++) {
      top_pb_graph_node->clock_pins[iport][ipin].temp_net_num = OPEN;
      type_pin_index = top_pb_graph_node->clock_pins[iport][ipin].pin_count_in_cluster;
      class_id = type->pin_class[type_pin_index];
      assert(RECEIVER == type->class_inf[class_id].type);
      /* Find the pb net_num and update OPIN net_num */
      pin_global_rr_node_id = get_rr_node_index(x, y, IPIN, type_pin_index, rr_node_indices);
      if (OPEN == rr_node[pin_global_rr_node_id].net_num) {
        top_pb_graph_node->clock_pins[iport][ipin].temp_net_num = OPEN;
        continue;
      } 
      top_pb_graph_node->clock_pins[iport][ipin].temp_net_num = clb_to_vpack_net_mapping[rr_node[pin_global_rr_node_id].net_num];
    } 
  }

  /* Go recursively ... */
  mode_index = find_pb_type_idle_mode_index(*(top_pb_graph_node->pb_type));
  for (ipb = 0; ipb < top_pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < top_pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Mark pb_graph_node temp_net_num */
      rec_mark_pb_graph_node_temp_net_num(&(top_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb]));
    }
  }

  /* Output ports */
  for (iport = 0; iport < top_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_output_pins[iport]; ipin++) {
      top_pb_graph_node->output_pins[iport][ipin].temp_net_num = OPEN;
      type_pin_index = top_pb_graph_node->output_pins[iport][ipin].pin_count_in_cluster;
      class_id = type->pin_class[type_pin_index];
      assert(DRIVER == type->class_inf[class_id].type);
      /* Find the pb net_num and update OPIN net_num */
      pin_global_rr_node_id = get_rr_node_index(x, y, OPIN, type_pin_index, rr_node_indices);
      if (OPEN == rr_node[pin_global_rr_node_id].net_num) {
        top_pb_graph_node->output_pins[iport][ipin].temp_net_num = OPEN;
        continue;
      } 
      top_pb_graph_node->output_pins[iport][ipin].temp_net_num = clb_to_vpack_net_mapping[rr_node[pin_global_rr_node_id].net_num];
    } 
  }

  /* Run again to handle feedback loop */
  /* Input ports */
  for (iport = 0; iport < top_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_input_pins[iport]; ipin++) {
      top_pb_graph_node->input_pins[iport][ipin].temp_net_num = OPEN;
      type_pin_index = top_pb_graph_node->input_pins[iport][ipin].pin_count_in_cluster;
      class_id = type->pin_class[type_pin_index];
      assert(RECEIVER == type->class_inf[class_id].type);
      /* Find the pb net_num and update OPIN net_num */
      pin_global_rr_node_id = get_rr_node_index(x, y, IPIN, type_pin_index, rr_node_indices);
      if (OPEN == rr_node[pin_global_rr_node_id].net_num) {
        top_pb_graph_node->input_pins[iport][ipin].temp_net_num = OPEN;
        continue;
      } 
      top_pb_graph_node->input_pins[iport][ipin].temp_net_num = clb_to_vpack_net_mapping[rr_node[pin_global_rr_node_id].net_num];
    } 
  }
  /* clock ports */
  for (iport = 0; iport < top_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_clock_pins[iport]; ipin++) {
      top_pb_graph_node->clock_pins[iport][ipin].temp_net_num = OPEN;
      type_pin_index = top_pb_graph_node->clock_pins[iport][ipin].pin_count_in_cluster;
      class_id = type->pin_class[type_pin_index];
      assert(RECEIVER == type->class_inf[class_id].type);
      /* Find the pb net_num and update OPIN net_num */
      pin_global_rr_node_id = get_rr_node_index(x, y, IPIN, type_pin_index, rr_node_indices);
      if (OPEN == rr_node[pin_global_rr_node_id].net_num) {
        top_pb_graph_node->clock_pins[iport][ipin].temp_net_num = OPEN;
        continue;
      } 
      top_pb_graph_node->clock_pins[iport][ipin].temp_net_num = clb_to_vpack_net_mapping[rr_node[pin_global_rr_node_id].net_num];
    } 
  }

  return; 
}

/* Assign the temp_net_num by considering the first incoming edge that belongs to the correct operating mode */
void assign_pb_graph_node_pin_temp_net_num_by_mode_index(t_pb_graph_pin* cur_pb_graph_pin,
                                                         int mode_index) {
  int iedge;

  /* IMPORTANT: I assume by default the index of selected edge is 0 
   * Make sure this input edge comes from the default mode     
   */
  for (iedge = 0; iedge < cur_pb_graph_pin->num_input_edges; iedge++) {
    if (mode_index != cur_pb_graph_pin->input_edges[iedge]->interconnect->parent_mode_index) {
      continue;
    }
    cur_pb_graph_pin->temp_net_num = cur_pb_graph_pin->input_edges[iedge]->input_pins[0]->temp_net_num;
    break;
  }

  return;  
}

void mark_pb_graph_node_input_pins_temp_net_num(t_pb_graph_node* cur_pb_graph_node,
                                                int mode_index) {
  int iport, ipin;

  assert(NULL != cur_pb_graph_node);

  /* Input ports */
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
      cur_pb_graph_node->input_pins[iport][ipin].temp_net_num = OPEN;
      /* IMPORTANT: I assume by default the index of selected edge is 0 
       * Make sure this input edge comes from the default mode     
       */
       assign_pb_graph_node_pin_temp_net_num_by_mode_index(&(cur_pb_graph_node->input_pins[iport][ipin]), mode_index);
    }
  }

  return;
}

void mark_pb_graph_node_clock_pins_temp_net_num(t_pb_graph_node* cur_pb_graph_node,
                                                int mode_index) {
  int iport, ipin;

  assert(NULL != cur_pb_graph_node);

  /* Clock ports */
  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
      cur_pb_graph_node->clock_pins[iport][ipin].temp_net_num = OPEN;
      assign_pb_graph_node_pin_temp_net_num_by_mode_index(&(cur_pb_graph_node->clock_pins[iport][ipin]), mode_index);
    }
  }

  return;
}

void mark_pb_graph_node_output_pins_temp_net_num(t_pb_graph_node* cur_pb_graph_node,
                                                int mode_index) {
  int iport, ipin;

  assert(NULL != cur_pb_graph_node);

  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      cur_pb_graph_node->output_pins[iport][ipin].temp_net_num = OPEN;
      /* IMPORTANT: I assume by default the index of selected edge is 0 
       * Make sure this input edge comes from the default mode     
       */
      assign_pb_graph_node_pin_temp_net_num_by_mode_index(&(cur_pb_graph_node->output_pins[iport][ipin]), mode_index);
    }
  }

  return;
}

/* Mark temp_net_num in current pb_graph_node from the parent pb_graph_node */
void rec_mark_pb_graph_node_temp_net_num(t_pb_graph_node* cur_pb_graph_node) {
  int mode_index, ipb, jpb;

  assert(NULL != cur_pb_graph_node);

  /* Find the default mode */
  mode_index = find_pb_type_idle_mode_index(*(cur_pb_graph_node->pb_type));

  mark_pb_graph_node_input_pins_temp_net_num(cur_pb_graph_node, mode_index);

  mark_pb_graph_node_clock_pins_temp_net_num(cur_pb_graph_node, mode_index);

  if (NULL != cur_pb_graph_node->pb_type->spice_model) {
    return;
  }

  /* Go recursively ... */
  mode_index = find_pb_type_idle_mode_index(*(cur_pb_graph_node->pb_type));
  for (ipb = 0; ipb < cur_pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Mark pb_graph_node temp_net_num */
      rec_mark_pb_graph_node_temp_net_num(&(cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb]));
    }
  }

  /* IMPORTANT: update the temp_net of Output ports after recursion is done!
   * the outputs of sub pb_graph_node should be updated first 
   */
  mark_pb_graph_node_output_pins_temp_net_num(cur_pb_graph_node, mode_index);

  /* Do this again to handle feedback loops ! */
  mark_pb_graph_node_input_pins_temp_net_num(cur_pb_graph_node, mode_index);

  mark_pb_graph_node_clock_pins_temp_net_num(cur_pb_graph_node, mode_index);

  return;
}

void load_one_pb_graph_pin_temp_net_num_from_pb(t_pb* cur_pb,
                                                t_pb_graph_pin* cur_pb_graph_pin) {
  int node_index;
  t_rr_node* pb_rr_nodes = NULL;

  assert(NULL != cur_pb);
  assert(NULL != cur_pb->pb_graph_node);

  /* Get the selected edge of current pin*/
  pb_rr_nodes = cur_pb->rr_graph;
  node_index = cur_pb_graph_pin->pin_count_in_cluster;
  cur_pb_graph_pin->temp_net_num = pb_rr_nodes[node_index].vpack_net_num;

  return;
}

/* According to the vpack_net_num in cur_pb
 * assign it to the corresponding pb_graph_pins 
 */
void load_pb_graph_node_temp_net_num_from_pb(t_pb* cur_pb) {
  int iport, ipin;

  assert(NULL != cur_pb);
  assert(NULL != cur_pb->pb_graph_node);

  /* Input ports */
  for (iport = 0; iport < cur_pb->pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < cur_pb->pb_graph_node->num_input_pins[iport]; ipin++) {
      load_one_pb_graph_pin_temp_net_num_from_pb(cur_pb,
                                                 &(cur_pb->pb_graph_node->input_pins[iport][ipin]));
    }
  }

  /* Clock ports */
  for (iport = 0; iport < cur_pb->pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < cur_pb->pb_graph_node->num_clock_pins[iport]; ipin++) {
      load_one_pb_graph_pin_temp_net_num_from_pb(cur_pb,
                                                 &(cur_pb->pb_graph_node->clock_pins[iport][ipin]));
    }
  }

  /* Output ports */
  for (iport = 0; iport < cur_pb->pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < cur_pb->pb_graph_node->num_output_pins[iport]; ipin++) {
      load_one_pb_graph_pin_temp_net_num_from_pb(cur_pb,
                                                 &(cur_pb->pb_graph_node->output_pins[iport][ipin]));
    }
  }
 
  return;
}

/* Recursively traverse the hierachy of a pb, 
 * store parasitic nets in the temp_net_num of the assoicated pb_graph_node 
 */
void rec_mark_one_pb_unused_pb_graph_node_temp_net_num(t_pb* cur_pb) {
  int ipb, jpb;
  int mode_index;

  /* Check */
  assert(NULL != cur_pb);

  if (NULL != cur_pb->pb_graph_node->pb_type->spice_model) {
    return;
  }
  /* Go recursively ... */
  mode_index = cur_pb->mode;
  if (!(0 < cur_pb->pb_graph_node->pb_type->num_modes)) {
    return;
  }
  for (ipb = 0; ipb < cur_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb->pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Refer to pack/output_clustering.c [LINE 392] */
      if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
        rec_mark_one_pb_unused_pb_graph_node_temp_net_num(&(cur_pb->child_pbs[ipb][jpb]));
      } else {
        /* Print idle graph_node muxes */
        load_pb_graph_node_temp_net_num_from_pb(cur_pb);
        /* We should update the net_num  */
        rec_mark_pb_graph_node_temp_net_num(cur_pb->child_pbs[ipb][jpb].pb_graph_node);
      }
    }
  }
  
  return;
}

void update_pb_vpack_net_num_from_temp_net_num(t_pb* cur_pb, 
                                               t_pb_graph_pin* cur_pb_graph_pin) {
  int node_index;
  t_rr_node* pb_rr_nodes = NULL;

  assert(NULL != cur_pb);
  assert(NULL != cur_pb->pb_graph_node);

  /* Get the selected edge of current pin*/
  pb_rr_nodes = cur_pb->rr_graph;
  node_index = cur_pb_graph_pin->pin_count_in_cluster;
  
  /* Avoid mistakenly modification */
  if (OPEN != pb_rr_nodes[node_index].vpack_net_num) {
    return;
  }
  /* Only modify when original vpack_net_num is open!!! */
  pb_rr_nodes[node_index].vpack_net_num = cur_pb_graph_pin->temp_net_num;

  return;
} 

void update_pb_graph_node_temp_net_num_to_pb(t_pb_graph_node* cur_pb_graph_node,
                                             t_pb* cur_pb) {
  int iport, ipin;
  t_rr_node* pb_rr_nodes = NULL;

  assert(NULL != cur_pb->pb_graph_node);
  assert(NULL != cur_pb);

  pb_rr_nodes = cur_pb->rr_graph;

  /* Input ports */
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
      update_pb_vpack_net_num_from_temp_net_num(cur_pb,
                                                &(cur_pb_graph_node->input_pins[iport][ipin]));
    }
  }

  /* Clock ports */
  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
      update_pb_vpack_net_num_from_temp_net_num(cur_pb,
                                                &(cur_pb_graph_node->clock_pins[iport][ipin]));
    }
  }

  /* Output ports */
  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      update_pb_vpack_net_num_from_temp_net_num(cur_pb,
                                                &(cur_pb_graph_node->output_pins[iport][ipin]));
    }
  }

  return;
}

void rec_load_unused_pb_graph_node_temp_net_num_to_pb(t_pb* cur_pb) {
  int ipb, jpb;
  int mode_index;

  /* Check */
  assert(NULL != cur_pb);

  if (NULL != cur_pb->pb_graph_node->pb_type->spice_model) {
    return;
  }
  /* Go recursively ... */
  mode_index = cur_pb->mode;
  if (!(0 < cur_pb->pb_graph_node->pb_type->num_modes)) {
    return;
  }
  for (ipb = 0; ipb < cur_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb->pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Refer to pack/output_clustering.c [LINE 392] */
      if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
        rec_load_unused_pb_graph_node_temp_net_num_to_pb(&(cur_pb->child_pbs[ipb][jpb]));
      } else {
        update_pb_graph_node_temp_net_num_to_pb(cur_pb->child_pbs[ipb][jpb].pb_graph_node, 
                                                cur_pb);
      }
    }
  }
  
  return;
}

void mark_one_pb_parasitic_nets(t_pb* cur_pb) {

  /* By go recursively, parasitic net num are stored in the temp_net_num in pb_graph_node */
  rec_mark_one_pb_unused_pb_graph_node_temp_net_num(cur_pb);

  /* Load the temp_net_num to vpack_net_num in the current pb! */
  rec_load_unused_pb_graph_node_temp_net_num_to_pb(cur_pb);

  return;
}

int count_num_conf_bit_one_interc(t_interconnect* cur_interc,
                                  enum e_sram_orgz cur_sram_orgz_type) {
  int fan_in = 0;
  enum e_interconnect spice_interc_type = DIRECT_INTERC;

  int num_conf_bits = 0;

  /* 1. identify pin interconnection type, 
   * 2. Identify the number of fan-in (Consider interconnection edges of only selected mode)
   * 3. Select and print the SPICE netlist
   */
  if (NULL == cur_interc) { 
    return num_conf_bits;
  } else {
    fan_in = cur_interc->fan_in;
    if (0 == fan_in) {
      return num_conf_bits;
    }
  }
  /* Initialize the interconnection type that will be implemented in SPICE netlist*/
  switch (cur_interc->type) {
  case DIRECT_INTERC:
    assert(cur_interc->fan_out == fan_in);
    spice_interc_type = DIRECT_INTERC;
    break;
  case COMPLETE_INTERC:
    if (1 == fan_in) {
      spice_interc_type = DIRECT_INTERC;
    } else {
      assert((2 == fan_in)||(2 < fan_in));
      spice_interc_type = MUX_INTERC;
    }
    break;
  case MUX_INTERC:
    assert((2 == fan_in)||(2 < fan_in));
    spice_interc_type = MUX_INTERC;
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid interconnection type for %s (Arch[LINE%d])!\n",
               __FILE__, __LINE__, cur_interc->name, cur_interc->line_num);
    exit(1);
  }
  /* This time, (2nd round), count the number of configuration bits, according to interc type*/ 
  switch (spice_interc_type) {
  case DIRECT_INTERC:
    /* Check : 
     * 1. Direct interc has only one fan-in!
     */
    assert((cur_interc->fan_out == fan_in)
         ||((COMPLETE_INTERC == cur_interc->type)&&(1 == fan_in)));
    break;
  case COMPLETE_INTERC:
  case MUX_INTERC:
    /* Check : 
     * MUX should have at least 2 fan_in
     */
    assert((2 == fan_in)||(2 < fan_in));
    assert((1 == cur_interc->fan_out)||(1 < cur_interc->fan_out));
    /* 2. spice_model is a wire */ 
    assert(NULL != cur_interc->spice_model);
    assert(SPICE_MODEL_MUX == cur_interc->spice_model->type);
    num_conf_bits = count_num_conf_bits_one_spice_model(cur_interc->spice_model, 
                                                        cur_sram_orgz_type, fan_in);
    /* FOR COMPLETE_INTERC: we should consider fan_out number ! */
    num_conf_bits = num_conf_bits * cur_interc->fan_out;
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid interconnection type for %s (Arch[LINE%d])!\n",
               __FILE__, __LINE__, cur_interc->name, cur_interc->line_num);
    exit(1);
  }
  return num_conf_bits;
}

/* Count the number of configuration bits of interconnection inside a pb_type in its default mode */
int count_num_conf_bits_pb_type_mode_interc(t_mode* cur_pb_type_mode,
                                            enum e_sram_orgz cur_sram_orgz_type) {
  int num_conf_bits = 0;
  int jinterc = 0;

  for (jinterc = 0; jinterc < cur_pb_type_mode->num_interconnect; jinterc++) {
    num_conf_bits += count_num_conf_bit_one_interc(&(cur_pb_type_mode->interconnect[jinterc]),
                                                   cur_sram_orgz_type); 
  }
  
  return num_conf_bits;
} 

/* Count the number of configuration bits of interconnection inside a pb_type in its default mode */
int count_num_reserved_conf_bits_pb_type_mode_interc(t_mode* cur_pb_type_mode,
                                                     enum e_sram_orgz cur_sram_orgz_type) {
  int num_reserved_conf_bits = 0;
  int temp_num_reserved_conf_bits = 0;
  int jinterc = 0;

  for (jinterc = 0; jinterc < cur_pb_type_mode->num_interconnect; jinterc++) {
    /* num of reserved configuration bits is determined by the largest one */
    temp_num_reserved_conf_bits = 
                     count_num_reserved_conf_bit_one_interc(&(cur_pb_type_mode->interconnect[jinterc]),
                                                            cur_sram_orgz_type); 
    if (temp_num_reserved_conf_bits > num_reserved_conf_bits) {
      num_reserved_conf_bits = temp_num_reserved_conf_bits;
    }
  }
  
  return num_reserved_conf_bits;
}

/* Count the number of configuration bits of a grid (type_descriptor) in default mode */
int rec_count_num_conf_bits_pb_type_default_mode(t_pb_type* cur_pb_type,
                                                 t_sram_orgz_info* cur_sram_orgz_info) {
  int mode_index, ipb, jpb;
  int sum_num_conf_bits = 0;
  int num_reserved_conf_bits = 0;
  int temp_num_reserved_conf_bits = 0;

  cur_pb_type->default_mode_num_conf_bits = 0;

  /* Recursively finish all the child pb_types*/
  if (NULL == cur_pb_type->spice_model) { 
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_idle_mode_index((*cur_pb_type));
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) { 
        rec_count_num_conf_bits_pb_type_default_mode(&(cur_pb_type->modes[mode_index].pb_type_children[ipb]), cur_sram_orgz_info);
      }
    }
  }

  /* Check if this has defined a spice_model*/
  if (NULL != cur_pb_type->spice_model) {
    sum_num_conf_bits = count_num_conf_bits_one_spice_model(cur_pb_type->spice_model, cur_sram_orgz_info->type, 0);
    cur_pb_type->default_mode_num_conf_bits = sum_num_conf_bits;
    /* calculate the number of reserved configuration bits */
    cur_pb_type->default_mode_num_reserved_conf_bits = 
         count_num_reserved_conf_bits_one_spice_model(cur_pb_type->spice_model,
                                                      cur_sram_orgz_info->type, 0);
  } else { /* Count the sum of configuration bits of all the children pb_types */
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_idle_mode_index((*cur_pb_type));
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
        /* Count in the number of configuration bits of on child pb_type */
        sum_num_conf_bits += cur_pb_type->modes[mode_index].pb_type_children[ipb].default_mode_num_conf_bits;
        temp_num_reserved_conf_bits = 
                            cur_pb_type->modes[mode_index].pb_type_children[ipb].default_mode_num_reserved_conf_bits;
        /* number of reserved conf. bits is deteremined by the largest number of reserved conf. bits !*/
        if (temp_num_reserved_conf_bits > num_reserved_conf_bits) {
          num_reserved_conf_bits = temp_num_reserved_conf_bits;
        }
      }
    }
    /* Count the number of configuration bits of interconnection */
    sum_num_conf_bits += count_num_conf_bits_pb_type_mode_interc(&(cur_pb_type->modes[mode_index]), cur_sram_orgz_info->type); 
    /* Count the number of reserved_configuration bits of interconnection */
    temp_num_reserved_conf_bits = 
                count_num_reserved_conf_bits_pb_type_mode_interc(&(cur_pb_type->modes[mode_index]),
                                                                 cur_sram_orgz_info->type); 
    /* number of reserved conf. bits is deteremined by the largest number of reserved conf. bits !*/
    if (temp_num_reserved_conf_bits > num_reserved_conf_bits) {
      num_reserved_conf_bits = temp_num_reserved_conf_bits;
    }
    /* Update the info in pb_type */
    cur_pb_type->default_mode_num_reserved_conf_bits = num_reserved_conf_bits;
    cur_pb_type->default_mode_num_conf_bits = sum_num_conf_bits;
  }

  return sum_num_conf_bits;
}

/* Count the number of configuration bits of a grid (type_descriptor) in default mode */
int rec_count_num_conf_bits_pb_type_physical_mode(t_pb_type* cur_pb_type,
                                                  t_sram_orgz_info* cur_sram_orgz_info) {
  int mode_index, ipb, jpb;
  int sum_num_conf_bits = 0;
  int num_reserved_conf_bits = 0;
  int temp_num_reserved_conf_bits = 0;

  cur_pb_type->physical_mode_num_conf_bits = 0;

  /* Recursively finish all the child pb_types*/
  if (NULL == cur_pb_type->spice_model) { 
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_physical_mode_index((*cur_pb_type));
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) { 
        rec_count_num_conf_bits_pb_type_physical_mode(&(cur_pb_type->modes[mode_index].pb_type_children[ipb]), cur_sram_orgz_info);
      }
    }
  }

  /* Check if this has defined a spice_model*/
  if (NULL != cur_pb_type->spice_model) {
    sum_num_conf_bits = count_num_conf_bits_one_spice_model(cur_pb_type->spice_model, cur_sram_orgz_info->type, 0);
    cur_pb_type->physical_mode_num_conf_bits = sum_num_conf_bits;
    /* calculate the number of reserved configuration bits */
    cur_pb_type->physical_mode_num_reserved_conf_bits = 
         count_num_reserved_conf_bits_one_spice_model(cur_pb_type->spice_model,
                                                      cur_sram_orgz_info->type, 0);
  } else { /* Count the sum of configuration bits of all the children pb_types */
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_physical_mode_index((*cur_pb_type));
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
        /* Count in the number of configuration bits of on child pb_type */
        sum_num_conf_bits += cur_pb_type->modes[mode_index].pb_type_children[ipb].physical_mode_num_conf_bits;
        temp_num_reserved_conf_bits = cur_pb_type->modes[mode_index].pb_type_children[ipb].physical_mode_num_reserved_conf_bits;

        /* number of reserved conf. bits is deteremined by the largest number of reserved conf. bits !*/
        if (temp_num_reserved_conf_bits > num_reserved_conf_bits) {
          num_reserved_conf_bits = temp_num_reserved_conf_bits;
        }
      }
    }
    /* Count the number of configuration bits of interconnection */
    sum_num_conf_bits += count_num_conf_bits_pb_type_mode_interc(&(cur_pb_type->modes[mode_index]), 
                                                                 cur_sram_orgz_info->type); 
    /* Count the number of reserved_configuration bits of interconnection */
    temp_num_reserved_conf_bits = 
                count_num_reserved_conf_bits_pb_type_mode_interc(&(cur_pb_type->modes[mode_index]),
                                                                 cur_sram_orgz_info->type); 
    /* number of reserved conf. bits is deteremined by the largest number of reserved conf. bits !*/
    if (temp_num_reserved_conf_bits > num_reserved_conf_bits) {
      num_reserved_conf_bits = temp_num_reserved_conf_bits;
    }
    /* Update the info in pb_type */
    cur_pb_type->physical_mode_num_reserved_conf_bits = num_reserved_conf_bits;
    cur_pb_type->physical_mode_num_conf_bits = sum_num_conf_bits;
  }

  return sum_num_conf_bits;
}


/* Count the number of configuration bits of a pb_graph_node */
int rec_count_num_conf_bits_pb(t_pb* cur_pb, 
                               t_sram_orgz_info* cur_sram_orgz_info) {
  int mode_index, ipb, jpb;
  t_pb_type* cur_pb_type = NULL;
  t_pb_graph_node* cur_pb_graph_node = NULL;
  int sum_num_conf_bits = 0;
  int num_reserved_conf_bits = 0;
  int temp_num_reserved_conf_bits = 0;

  /* Check cur_pb_graph_node*/
  if (NULL == cur_pb) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid cur_pb.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  cur_pb_graph_node = cur_pb->pb_graph_node;
  cur_pb_type = cur_pb_graph_node->pb_type;
  mode_index = cur_pb->mode; 

  cur_pb->num_conf_bits = 0;

  /* Recursively finish all the child pb_types*/
  if (NULL == cur_pb_type->spice_model) { 
    /* recursive for the child_pbs*/
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) { 
        /* Recursive*/
        /* Refer to pack/output_clustering.c [LINE 392] */
        if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
          rec_count_num_conf_bits_pb(&(cur_pb->child_pbs[ipb][jpb]), cur_sram_orgz_info);
        } else {
          /* Check if this pb has no children, no children mean idle*/
          rec_count_num_conf_bits_pb_type_default_mode(cur_pb->child_pbs[ipb][jpb].pb_graph_node->pb_type,
                                                       cur_sram_orgz_info);
        }
      } 
    }
  }

  /* Check if this has defined a spice_model*/
  if (NULL != cur_pb_type->spice_model) {
    sum_num_conf_bits += count_num_conf_bits_one_spice_model(cur_pb_type->spice_model, 
                                                             cur_sram_orgz_info->type, 0);
    cur_pb->num_conf_bits = sum_num_conf_bits;
    /* calculate the number of reserved configuration bits */
    cur_pb->num_reserved_conf_bits = 
         count_num_reserved_conf_bits_one_spice_model(cur_pb_type->spice_model,
                                                      cur_sram_orgz_info->type, 0);
 
  } else {
    /* Definition ends*/
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
        /* we should make sure this placement index == child_pb_type[jpb]*/
        assert(jpb == cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb].placement_index);
        if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
          /* Count in the number of configuration bits of on child pb */
          sum_num_conf_bits += cur_pb->child_pbs[ipb][jpb].num_conf_bits;
          temp_num_reserved_conf_bits = cur_pb->child_pbs[ipb][jpb].num_reserved_conf_bits;
        } else {
          /* Count in the number of configuration bits of on child pb_type */
          sum_num_conf_bits += cur_pb_type->modes[mode_index].pb_type_children[ipb].default_mode_num_conf_bits;
          temp_num_reserved_conf_bits = 
                            cur_pb_type->modes[mode_index].pb_type_children[ipb].default_mode_num_reserved_conf_bits;
        }
        /* number of reserved conf. bits is deteremined by the largest number of reserved conf. bits !*/
        if (temp_num_reserved_conf_bits > num_reserved_conf_bits) {
          num_reserved_conf_bits = temp_num_reserved_conf_bits;
        }
      }
    }
    /* Count the number of configuration bits of interconnection */
    sum_num_conf_bits += count_num_conf_bits_pb_type_mode_interc(&(cur_pb_type->modes[mode_index]),
                                                                 cur_sram_orgz_info->type); 
    /* Count the number of reserved_configuration bits of interconnection */
    temp_num_reserved_conf_bits = 
                count_num_reserved_conf_bits_pb_type_mode_interc(&(cur_pb_type->modes[mode_index]),
                                                                 cur_sram_orgz_info->type); 
    /* number of reserved conf. bits is deteremined by the largest number of reserved conf. bits !*/
    if (temp_num_reserved_conf_bits > num_reserved_conf_bits) {
      num_reserved_conf_bits = temp_num_reserved_conf_bits;
    }
    /* Update the info in pb_type */
    cur_pb->num_reserved_conf_bits = num_reserved_conf_bits;
    cur_pb->num_conf_bits = sum_num_conf_bits;
  }

  return sum_num_conf_bits;
}

/* Initialize the number of configuraion bits for one grid */
void init_one_grid_num_conf_bits(int ix, int iy, 
                                 t_sram_orgz_info* cur_sram_orgz_info) {
  t_block* mapped_block = NULL;
  int iz;
  int cur_block_index = 0;
  int capacity; 

  /* Check */
  assert((!(0 > ix))&&(!(ix > (nx + 1)))); 
  assert((!(0 > iy))&&(!(iy > (ny + 1)))); 
 
  if ((NULL == grid[ix][iy].type)
     ||(EMPTY_TYPE == grid[ix][iy].type)
     ||(0 != grid[ix][iy].offset)) {
    /* Empty grid, directly return */
    return; 
  }
  capacity= grid[ix][iy].type->capacity;
  assert(0 < capacity);

  /* check capacity and if this has been mapped */
  for (iz = 0; iz < capacity; iz++) {
    /* Check in all the blocks(clustered logic block), there is a match x,y,z*/
    mapped_block = search_mapped_block(ix, iy, iz); 
    rec_count_num_conf_bits_pb_type_physical_mode(grid[ix][iy].type->pb_type, cur_sram_orgz_info);
    /* Comments: Grid [x][y]*/
    if (NULL == mapped_block) {
      /* Print a consider a idle pb_type ...*/
      rec_count_num_conf_bits_pb_type_default_mode(grid[ix][iy].type->pb_type, cur_sram_orgz_info);
    } else {
      if (iz == mapped_block->z) {
        // assert(mapped_block == &(block[grid[ix][iy].blocks[cur_block_index]]));
        cur_block_index++;
      }
      rec_count_num_conf_bits_pb(mapped_block->pb, cur_sram_orgz_info);
    }
  } 

  assert(cur_block_index == grid[ix][iy].usage);

  return;
}

/* Initialize the number of configuraion bits for all grids */
void init_grids_num_conf_bits(t_sram_orgz_info* cur_sram_orgz_info) {
  int ix, iy; 

  /* Core grid */
  vpr_printf(TIO_MESSAGE_INFO, "INFO: Initializing number of configuration bits of Core grids...\n");
  for (ix = 1; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      init_one_grid_num_conf_bits(ix, iy, cur_sram_orgz_info);
    }
  }
  
  /* Consider the IO pads */
  vpr_printf(TIO_MESSAGE_INFO, "INFO: Initializing number of configuration bits of I/O grids...\n");
  /* Left side: x = 0, y = 1 .. ny*/
  ix = 0;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_conf_bits(ix, iy, cur_sram_orgz_info);
  }
  /* Right side : x = nx + 1, y = 1 .. ny*/
  ix = nx + 1;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_conf_bits(ix, iy, cur_sram_orgz_info);
  }
  /* Bottom  side : x = 1 .. nx + 1, y = 0 */
  iy = 0;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_conf_bits(ix, iy, cur_sram_orgz_info);
  }
  /* Top side : x = 1 .. nx + 1, y = nx + 1  */
  iy = ny + 1;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_conf_bits(ix, iy, cur_sram_orgz_info);
  }
 
  return;
}

/* With given spice_model_port, find the pb_type port with same name and type*/
t_port* find_pb_type_port_match_spice_model_port(t_pb_type* pb_type,
                                                 t_spice_model_port* spice_model_port) {
  int iport;
  t_port* ret = NULL;

  /* Search ports */
  for (iport = 0; iport < pb_type->num_ports; iport++) {
    /* Match the name and port size*/
    if ((0 == strcmp(pb_type->ports[iport].name, spice_model_port->prefix)) 
      &&(pb_type->ports[iport].num_pins == spice_model_port->size)) {
      /* Match the type*/
      switch (spice_model_port->type) {
      case SPICE_MODEL_PORT_INPUT:
        if ((IN_PORT == pb_type->ports[iport].type)
          &&(0 == pb_type->ports[iport].is_clock)) {
          if (NULL != ret) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])More than 1 pb_type(%s) port match spice_model_port(%s)!\n",
                       __FILE__, __LINE__, pb_type->name, spice_model_port->prefix);
            exit(1);
          }
          ret = &(pb_type->ports[iport]);
        }
        break;
      case SPICE_MODEL_PORT_OUTPUT:
        if (OUT_PORT == pb_type->ports[iport].type) {
          if (NULL != ret) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])More than 1 pb_type(%s) port match spice_model_port(%s)!\n",
                       __FILE__, __LINE__, pb_type->name, spice_model_port->prefix);
            exit(1);
          }
          ret = &(pb_type->ports[iport]);
        }
        break;
      case SPICE_MODEL_PORT_CLOCK:
        if ((IN_PORT == pb_type->ports[iport].type)&&(1 == pb_type->ports[iport].is_clock)) {
          if (NULL != ret) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])More than 1 pb_type(%s) port match spice_model_port(%s)!\n",
                       __FILE__, __LINE__, pb_type->name, spice_model_port->prefix);
            exit(1);
          }
          ret = &(pb_type->ports[iport]);
        }
        break;
      case SPICE_MODEL_PORT_INOUT : 
        if ((INOUT_PORT == pb_type->ports[iport].type)&&(0 == pb_type->ports[iport].is_clock)) {
          if (NULL != ret) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])More than 1 pb_type(%s) port match spice_model_port(%s)!\n",
                       __FILE__, __LINE__, pb_type->name, spice_model_port->prefix);
            exit(1);
          }
          ret = &(pb_type->ports[iport]);
        }
        break;
      case SPICE_MODEL_PORT_SRAM:
        break;
      default:
        vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid type for spice_model_port(%s)!\n",
                   __FILE__, __LINE__, spice_model_port->prefix);
        exit(1);
      }
    }
  }

  return ret;
}

t_port** find_pb_type_ports_match_spice_model_port_type(t_pb_type* pb_type,
                                                        enum e_spice_model_port_type port_type,
                                                        int* port_num) {
  int iport, cur;
  t_port** ret = NULL;

  /* Check codes*/
  assert(NULL != port_num);
  assert(NULL != pb_type);

  /* Count the number of ports that match*/
  (*port_num) = 0;
  for (iport = 0; iport < pb_type->num_ports; iport++) {
    switch (port_type) {
    case SPICE_MODEL_PORT_INPUT: /* TODO: support is_non_clock_global*/ 
      if ((IN_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        (*port_num)++;
      }
      break;
    case SPICE_MODEL_PORT_OUTPUT: 
      if ((OUT_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        (*port_num)++;
      }
      break;
    case SPICE_MODEL_PORT_INOUT: 
      if ((INOUT_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        (*port_num)++;
      }
      break;
    case SPICE_MODEL_PORT_CLOCK: 
      if ((IN_PORT == pb_type->ports[iport].type)
        &&(1 == pb_type->ports[iport].is_clock)) {
        (*port_num)++;
      }
      break;
    case SPICE_MODEL_PORT_SRAM:
      /* Original VPR don't support this*/
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid type for port!\n",
                 __FILE__, __LINE__);
      exit(1);
    }
  }
  
  /* Initial the return pointers*/
  ret = (t_port**)my_malloc(sizeof(t_port*)*(*port_num));
  memset(ret, 0 , sizeof(t_port*)*(*port_num));
  
  /* Fill the return pointers*/
  cur = 0;

  for (iport = 0; iport < pb_type->num_ports; iport++) {
    switch (port_type) {
    case SPICE_MODEL_PORT_INPUT : /* TODO: support is_non_clock_global*/ 
      if ((IN_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        ret[cur] = &(pb_type->ports[iport]);
        cur++;
      }
      break;
    case SPICE_MODEL_PORT_OUTPUT: 
      if ((OUT_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        ret[cur] = &(pb_type->ports[iport]);
        cur++;
      }
      break;
    case SPICE_MODEL_PORT_INOUT: 
      if ((INOUT_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        ret[cur] = &(pb_type->ports[iport]);
        cur++;
      }
      break;
    case SPICE_MODEL_PORT_CLOCK: 
      if ((IN_PORT == pb_type->ports[iport].type)
        &&(1 == pb_type->ports[iport].is_clock)) {
        ret[cur] = &(pb_type->ports[iport]);
        cur++;
      }
      break;
    case SPICE_MODEL_PORT_SRAM: 
      /* Original VPR don't support this*/
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid type for port!\n",
                 __FILE__, __LINE__);
      exit(1);
    }
  }
 
  /* Check correctness*/
  assert(cur == (*port_num));
  
  return ret;
}


int find_path_id_between_pb_rr_nodes(t_rr_node* local_rr_graph,
                                                int src_node,
                                                int des_node) {
  int path_id = -1;
  int prev_edge = -1;
  int path_count = 0;
  int iedge;
  t_interconnect* cur_interc = NULL;

  /* Check */
  assert(NULL != local_rr_graph);
  assert((0 == src_node)||(0 < src_node));
  assert((0 == des_node)||(0 < des_node));

  prev_edge = local_rr_graph[des_node].prev_edge;
  check_pb_graph_edge(*(local_rr_graph[src_node].pb_graph_pin->output_edges[prev_edge]));
  assert(local_rr_graph[src_node].pb_graph_pin->output_edges[prev_edge]->output_pins[0] == local_rr_graph[des_node].pb_graph_pin);
 
  cur_interc = local_rr_graph[src_node].pb_graph_pin->output_edges[prev_edge]->interconnect;
  /* Search des_node input edges */ 
  for (iedge = 0; iedge < local_rr_graph[des_node].pb_graph_pin->num_input_edges; iedge++) {
    if (local_rr_graph[des_node].pb_graph_pin->input_edges[iedge]->input_pins[0] 
        == local_rr_graph[src_node].pb_graph_pin) {
      /* Strict check */
      assert(local_rr_graph[src_node].pb_graph_pin->output_edges[prev_edge]
              == local_rr_graph[des_node].pb_graph_pin->input_edges[iedge]);
      path_id = path_count;
      break;
    }
    if (cur_interc == local_rr_graph[des_node].pb_graph_pin->input_edges[iedge]->interconnect) {
      path_count++;
    }
  }

  return path_id; 
}

/* Find the interconnection type of pb_graph_pin edges*/
enum e_interconnect find_pb_graph_pin_in_edges_interc_type(t_pb_graph_pin pb_graph_pin) {
  enum e_interconnect interc_type;
  int def_interc_type = 0;
  int iedge;

  for (iedge = 0; iedge < pb_graph_pin.num_input_edges; iedge++) {
    /* Make sure all edges are legal: 1 input_pin, 1 output_pin*/
    check_pb_graph_edge(*(pb_graph_pin.input_edges[iedge]));
    /* Make sure all the edges interconnect type is the same*/
    if (0 == def_interc_type) {
      interc_type = pb_graph_pin.input_edges[iedge]->interconnect->type;
    } else if (interc_type != pb_graph_pin.input_edges[iedge]->interconnect->type) {
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d])Interconnection type are not same for port(%s),pin(%d).\n",
                 __FILE__, __LINE__, pb_graph_pin.port->name,pb_graph_pin.pin_number);
      exit(1);
    }
  }

  return interc_type;  
}

/* Find the interconnection type of pb_graph_pin edges*/
t_spice_model* find_pb_graph_pin_in_edges_interc_spice_model(t_pb_graph_pin pb_graph_pin) {
  t_spice_model* interc_spice_model;
  int def_interc_model = 0;
  int iedge;

  for (iedge = 0; iedge < pb_graph_pin.num_input_edges; iedge++) {
    /* Make sure all edges are legal: 1 input_pin, 1 output_pin*/
    check_pb_graph_edge(*(pb_graph_pin.input_edges[iedge]));
    /* Make sure all the edges interconnect type is the same*/
    if (0 == def_interc_model) {
      interc_spice_model= pb_graph_pin.input_edges[iedge]->interconnect->spice_model;
    } else if (interc_spice_model != pb_graph_pin.input_edges[iedge]->interconnect->spice_model) {
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d])Interconnection spice_model are not same for port(%s),pin(%d).\n",
                 __FILE__, __LINE__, pb_graph_pin.port->name, pb_graph_pin.pin_number);
      exit(1);
    }
  }

  return interc_spice_model;  
}

void find_interc_fan_in_des_pb_graph_pin(t_pb_graph_pin* des_pb_graph_pin,
                                         t_mode* cur_mode,
                                         t_interconnect** cur_interc,
                                         int* fan_in) { 
  int iedge;
  
  (*cur_interc) = NULL;
  (*fan_in) = 0;  

  /* Search the input edges only, stats on the size of MUX we may need (fan-in) */
  for (iedge = 0; iedge < des_pb_graph_pin->num_input_edges; iedge++) {
    /* 1. First, we should make sure this interconnect is in the selected mode!!!*/
    if (cur_mode == des_pb_graph_pin->input_edges[iedge]->interconnect->parent_mode) {
      /* Check this edge*/
      check_pb_graph_edge(*(des_pb_graph_pin->input_edges[iedge]));
      /* Record the interconnection*/
      if (NULL == (*cur_interc)) {
        (*cur_interc) = des_pb_graph_pin->input_edges[iedge]->interconnect;
      } else { /* Make sure the interconnections for this pin is the same!*/
        assert((*cur_interc) == des_pb_graph_pin->input_edges[iedge]->interconnect);
      }
      /* Search the input_pins of input_edges only*/
      (*fan_in) += des_pb_graph_pin->input_edges[iedge]->num_input_pins;
    }
  }

  return;
}

/* Return a child_pb if it is mapped.*/
t_pb* get_child_pb_for_phy_pb_graph_node(t_pb* cur_pb, int ipb, int jpb) {
  t_pb* child_pb = NULL;
  
  /* TODO: more check ? */

  if (NULL == cur_pb) {
    return NULL;
  }

  if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
    child_pb = &(cur_pb->child_pbs[ipb][jpb]);
  }
  
  return child_pb;
}

/* Count the number of inpad and outpad of a grid (type_descriptor) in default mode */
void rec_count_num_iopads_pb_type_physical_mode(t_pb_type* cur_pb_type) {
  int mode_index, ipb, jpb;
  int sum_num_iopads = 0;

  cur_pb_type->physical_mode_num_iopads = 0;

  /* Recursively finish all the child pb_types*/
  if (NULL == cur_pb_type->spice_model) { 
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_physical_mode_index((*cur_pb_type));
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) { 
        rec_count_num_iopads_pb_type_physical_mode(&(cur_pb_type->modes[mode_index].pb_type_children[ipb]));
      }
    }
  }

  /* Check if this has defined a spice_model*/
  if (NULL != cur_pb_type->spice_model) {
    if (SPICE_MODEL_IOPAD == cur_pb_type->spice_model->type) {
      sum_num_iopads = 1;
      cur_pb_type->physical_mode_num_iopads = sum_num_iopads;
    }
  } else { /* Count the sum of configuration bits of all the children pb_types */
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_physical_mode_index((*cur_pb_type));
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
        /* Count in the number of configuration bits of on child pb_type */
        sum_num_iopads += cur_pb_type->modes[mode_index].pb_type_children[ipb].physical_mode_num_iopads;
      }
    }
    /* Count the number of configuration bits of interconnection */
    /* Update the info in pb_type */
    cur_pb_type->physical_mode_num_iopads = sum_num_iopads;
  }

  return;
}

/* Count the number of inpad and outpad of a grid (type_descriptor) in default mode */
void rec_count_num_iopads_pb_type_default_mode(t_pb_type* cur_pb_type) {
  int mode_index, ipb, jpb;
  int sum_num_iopads = 0;

  cur_pb_type->default_mode_num_iopads = 0;

  /* Recursively finish all the child pb_types*/
  if (NULL == cur_pb_type->spice_model) { 
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_idle_mode_index((*cur_pb_type));
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) { 
        rec_count_num_iopads_pb_type_default_mode(&(cur_pb_type->modes[mode_index].pb_type_children[ipb]));
      }
    }
  }

  /* Check if this has defined a spice_model*/
  if (NULL != cur_pb_type->spice_model) {
    if (SPICE_MODEL_IOPAD == cur_pb_type->spice_model->type) {
      sum_num_iopads = 1;
      cur_pb_type->default_mode_num_iopads = sum_num_iopads;
    }
  } else { /* Count the sum of configuration bits of all the children pb_types */
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_idle_mode_index((*cur_pb_type));
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
        /* Count in the number of configuration bits of on child pb_type */
        sum_num_iopads += cur_pb_type->modes[mode_index].pb_type_children[ipb].default_mode_num_iopads;
      }
    }
    /* Count the number of configuration bits of interconnection */
    /* Update the info in pb_type */
    cur_pb_type->default_mode_num_iopads = sum_num_iopads;
  }

  return;
}

/* Count the number of configuration bits of a pb_graph_node */
void rec_count_num_iopads_pb(t_pb* cur_pb) {
  int mode_index, ipb, jpb;
  t_pb_type* cur_pb_type = NULL;
  t_pb_graph_node* cur_pb_graph_node = NULL;

  int sum_num_iopads = 0;

  /* Check cur_pb_graph_node*/
  if (NULL == cur_pb) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid cur_pb.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  cur_pb_graph_node = cur_pb->pb_graph_node;
  cur_pb_type = cur_pb_graph_node->pb_type;
  mode_index = cur_pb->mode; 

  cur_pb->num_iopads = 0;

  /* Recursively finish all the child pb_types*/
  if (NULL == cur_pb_type->spice_model) { 
    /* recursive for the child_pbs*/
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) { 
        /* Recursive*/
        /* Refer to pack/output_clustering.c [LINE 392] */
        if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
          rec_count_num_iopads_pb(&(cur_pb->child_pbs[ipb][jpb]));
        } else {
          /* Check if this pb has no children, no children mean idle*/
          rec_count_num_iopads_pb_type_default_mode(cur_pb->child_pbs[ipb][jpb].pb_graph_node->pb_type);
        }
      } 
    }
  }

  /* Check if this has defined a spice_model*/
  if (NULL != cur_pb_type->spice_model) {
    if (SPICE_MODEL_IOPAD == cur_pb_type->spice_model->type) {
      sum_num_iopads = 1;
      cur_pb->num_iopads = sum_num_iopads;
    }
  } else {
    /* Definition ends*/
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
        /* we should make sure this placement index == child_pb_type[jpb]*/
        assert(jpb == cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb].placement_index);
        if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
          /* Count in the number of configuration bits of on child pb */
          sum_num_iopads += cur_pb->child_pbs[ipb][jpb].num_iopads;
        } else {
          /* Count in the number of configuration bits of on child pb_type */
          sum_num_iopads += cur_pb_type->modes[mode_index].pb_type_children[ipb].default_mode_num_iopads;
        }
      }
    }
    /* Count the number of configuration bits of interconnection */
    /* Update the info in pb_type */
    cur_pb->num_iopads = sum_num_iopads;
  }

  return;
}

/* Initialize the number of configuraion bits for one grid */
void init_one_grid_num_iopads(int ix, int iy) {
  t_block* mapped_block = NULL;
  int iz;
  int cur_block_index = 0;
  int capacity; 

  /* Check */
  assert((!(0 > ix))&&(!(ix > (nx + 1)))); 
  assert((!(0 > iy))&&(!(iy > (ny + 1)))); 
 
  if ((NULL == grid[ix][iy].type)
     ||(EMPTY_TYPE == grid[ix][iy].type)
     ||(0 != grid[ix][iy].offset)) {
    /* Empty grid, directly return */
    return; 
  }
  capacity= grid[ix][iy].type->capacity;
  assert(0 < capacity);

  /* check capacity and if this has been mapped */
  for (iz = 0; iz < capacity; iz++) {
    /* Check in all the blocks(clustered logic block), there is a match x,y,z*/
    mapped_block = search_mapped_block(ix, iy, iz); 
    rec_count_num_iopads_pb_type_physical_mode(grid[ix][iy].type->pb_type);
    /* Comments: Grid [x][y]*/
    if (NULL == mapped_block) {
      /* Print a consider a idle pb_type ...*/
      rec_count_num_iopads_pb_type_default_mode(grid[ix][iy].type->pb_type);
    } else {
      if (iz == mapped_block->z) {
        // assert(mapped_block == &(block[grid[ix][iy].blocks[cur_block_index]]));
        cur_block_index++;
      }
      rec_count_num_iopads_pb(mapped_block->pb);
    }
  } 

  assert(cur_block_index == grid[ix][iy].usage);

  return;
}

/* Initialize the number of configuraion bits for all grids */
void init_grids_num_iopads() {
  int ix, iy; 

  /* Core grid */
  vpr_printf(TIO_MESSAGE_INFO, "INFO: Initializing number of I/O pads in Core grids...\n");
  for (ix = 1; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      init_one_grid_num_iopads(ix, iy);
    }
  }
  
  /* Consider the IO pads */
  vpr_printf(TIO_MESSAGE_INFO, "INFO: Initializing number of I/O pads in I/O grids...\n");
  /* Left side: x = 0, y = 1 .. ny*/
  ix = 0;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_iopads(ix, iy);
  }
  /* Right side : x = nx + 1, y = 1 .. ny*/
  ix = nx + 1;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_iopads(ix, iy);
  }
  /* Bottom  side : x = 1 .. nx + 1, y = 0 */
  iy = 0;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_iopads(ix, iy);
  }
  /* Top side : x = 1 .. nx + 1, y = nx + 1  */
  iy = ny + 1;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_iopads(ix, iy);
  }
 
  return;
}

/* Count the number of mode configuration bits of a grid (type_descriptor) in default mode */
void rec_count_num_mode_bits_pb_type_default_mode(t_pb_type* cur_pb_type) {
  int mode_index, ipb, jpb;
  int sum_num_mode_bits = 0;

  cur_pb_type->default_mode_num_mode_bits = 0;

  /* Recursively finish all the child pb_types*/
  if (NULL == cur_pb_type->spice_model) { 
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_idle_mode_index((*cur_pb_type));
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) { 
        rec_count_num_mode_bits_pb_type_default_mode(&(cur_pb_type->modes[mode_index].pb_type_children[ipb]));
      }
    }
  }

  /* Check if this has defined a spice_model*/
  if (NULL != cur_pb_type->spice_model) {
    if (SPICE_MODEL_IOPAD == cur_pb_type->spice_model->type) {
      sum_num_mode_bits = 1;
      cur_pb_type->default_mode_num_mode_bits = sum_num_mode_bits;
    }
  } else { /* Count the sum of configuration bits of all the children pb_types */
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_idle_mode_index((*cur_pb_type));
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
        /* Count in the number of configuration bits of on child pb_type */
        sum_num_mode_bits += cur_pb_type->modes[mode_index].pb_type_children[ipb].default_mode_num_mode_bits;
      }
    }
    /* Count the number of configuration bits of interconnection */
    /* Update the info in pb_type */
    cur_pb_type->default_mode_num_mode_bits = sum_num_mode_bits;
  }

  return;
}

/* Count the number of configuration bits of a pb_graph_node */
void rec_count_num_mode_bits_pb(t_pb* cur_pb) {
  int mode_index, ipb, jpb;
  t_pb_type* cur_pb_type = NULL;
  t_pb_graph_node* cur_pb_graph_node = NULL;

  int sum_num_mode_bits = 0;

  /* Check cur_pb_graph_node*/
  if (NULL == cur_pb) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid cur_pb.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  cur_pb_graph_node = cur_pb->pb_graph_node;
  cur_pb_type = cur_pb_graph_node->pb_type;
  mode_index = cur_pb->mode; 

  cur_pb->num_mode_bits = 0;

  /* Recursively finish all the child pb_types*/
  if (NULL == cur_pb_type->spice_model) { 
    /* recursive for the child_pbs*/
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) { 
        /* Recursive*/
        /* Refer to pack/output_clustering.c [LINE 392] */
        if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
          rec_count_num_mode_bits_pb(&(cur_pb->child_pbs[ipb][jpb]));
        } else {
          /* Check if this pb has no children, no children mean idle*/
          rec_count_num_mode_bits_pb_type_default_mode(cur_pb->child_pbs[ipb][jpb].pb_graph_node->pb_type);
        }
      } 
    }
  }

  /* Check if this has defined a spice_model*/
  if (NULL != cur_pb_type->spice_model) {
    if (SPICE_MODEL_IOPAD == cur_pb_type->spice_model->type) {
      sum_num_mode_bits = 1;
      cur_pb->num_mode_bits = sum_num_mode_bits;
    }
  } else {
    /* Definition ends*/
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
        /* we should make sure this placement index == child_pb_type[jpb]*/
        assert(jpb == cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb].placement_index);
        if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
          /* Count in the number of configuration bits of on child pb */
          sum_num_mode_bits += cur_pb->child_pbs[ipb][jpb].num_mode_bits;
        } else {
          /* Count in the number of configuration bits of on child pb_type */
          sum_num_mode_bits += cur_pb_type->modes[mode_index].pb_type_children[ipb].default_mode_num_mode_bits;
        }
      }
    }
    /* Count the number of configuration bits of interconnection */
    /* Update the info in pb_type */
    cur_pb->num_mode_bits = sum_num_mode_bits;
  }

  return;
}

/* Initialize the number of configuraion bits for one grid */
void init_one_grid_num_mode_bits(int ix, int iy) {
  t_block* mapped_block = NULL;
  int iz;
  int cur_block_index = 0;
  int capacity; 

  /* Check */
  assert((!(0 > ix))&&(!(ix > (nx + 1)))); 
  assert((!(0 > iy))&&(!(iy > (ny + 1)))); 
 
  if ((NULL == grid[ix][iy].type)||(0 != grid[ix][iy].offset)) {
    /* Empty grid, directly return */
    return; 
  }
  capacity= grid[ix][iy].type->capacity;
  assert(0 < capacity);

  /* check capacity and if this has been mapped */
  for (iz = 0; iz < capacity; iz++) {
    /* Check in all the blocks(clustered logic block), there is a match x,y,z*/
    mapped_block = search_mapped_block(ix, iy, iz); 
    /* Comments: Grid [x][y]*/
    if (NULL == mapped_block) {
      /* Print a consider a idle pb_type ...*/
      rec_count_num_mode_bits_pb_type_default_mode(grid[ix][iy].type->pb_type);
    } else {
      if (iz == mapped_block->z) {
        // assert(mapped_block == &(block[grid[ix][iy].blocks[cur_block_index]]));
        cur_block_index++;
      }
      rec_count_num_mode_bits_pb(mapped_block->pb);
    }
  } 

  assert(cur_block_index == grid[ix][iy].usage);

  return;
}

/* Initialize the number of configuraion bits for all grids */
void init_grids_num_mode_bits() {
  int ix, iy; 

  /* Core grid */
  vpr_printf(TIO_MESSAGE_INFO, "INFO: Initializing number of mode configuraiton bits of Core grids...\n");
  for (ix = 1; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      init_one_grid_num_mode_bits(ix, iy);
    }
  }
  
  /* Consider the IO pads */
  vpr_printf(TIO_MESSAGE_INFO, "INFO: Initializing number of mode configuration bits of I/O grids...\n");
  /* Left side: x = 0, y = 1 .. ny*/
  ix = 0;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_mode_bits(ix, iy);
  }
  /* Right side : x = nx + 1, y = 1 .. ny*/
  ix = nx + 1;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_mode_bits(ix, iy);
  }
  /* Bottom  side : x = 1 .. nx + 1, y = 0 */
  iy = 0;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_mode_bits(ix, iy);
  }
  /* Top side : x = 1 .. nx + 1, y = nx + 1  */
  iy = ny + 1;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    init_one_grid_num_mode_bits(ix, iy);
  }
 
  return;
}

/* Return the child_pb of a LUT pb
 * Because the mapping information is stored in the child_pb!!!
 */
t_pb* get_lut_child_pb(t_pb* cur_lut_pb,
                       int mode_index) {

  assert(SPICE_MODEL_LUT == cur_lut_pb->pb_graph_node->pb_type->spice_model->type);

  assert(1 == cur_lut_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children);
  assert(1 == cur_lut_pb->pb_graph_node->pb_type->num_pb);

  return (&(cur_lut_pb->child_pbs[0][0])); 
}

/* Return the child_pb of a hardlogic  pb
 * Because the mapping information is stored in the child_pb!!!
 */
t_pb* get_hardlogic_child_pb(t_pb* cur_hardlogic_pb,
                             int mode_index) {

  assert(SPICE_MODEL_HARDLOGIC == cur_hardlogic_pb->pb_graph_node->pb_type->spice_model->type);

  assert(1 == cur_hardlogic_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children);
  assert(1 == cur_hardlogic_pb->pb_graph_node->pb_type->num_pb);

  return (&(cur_hardlogic_pb->child_pbs[0][0])); 
}

int get_grid_pin_height(int grid_x, int grid_y, int pin_index) {
  int pin_height;
  t_type_ptr grid_type = NULL;

  /* Get type */
  grid_type = grid[grid_x][grid_y].type;

  /* Return if this is an empty type */
  if ((NULL == grid_type)
     ||(EMPTY_TYPE == grid_type)) {
    pin_height = 0;
    return pin_height;
  }

  /* Check if the pin index is in the range */
  assert ( ((0 == pin_index) || (0 < pin_index))
          &&(pin_index < grid_type->num_pins) );

  /* Find the pin_height */
  pin_height = grid_type->pin_height[pin_index];
  
  return pin_height;
}

int get_grid_pin_side(int grid_x, int grid_y, int pin_index) {
  int pin_height, side, pin_side;
  t_type_ptr grid_type = NULL;

  /* Get type */
  grid_type = grid[grid_x][grid_y].type;

  /* Return if this is an empty type */
  if ((NULL == grid_type)
     ||(EMPTY_TYPE == grid_type)) {
    return -1;
  }

  /* Check if the pin index is in the range */
  assert ( ((0 == pin_index) || (0 < pin_index))
          &&(pin_index < grid_type->num_pins) );

  /* Find the pin_height */
  pin_height = get_grid_pin_height(grid_x, grid_y, pin_index);

  pin_side = -1;
  for (side = 0; side < 4; side++) {
    /* Bypass corner cases */
    /* Pin can only locate on BOTTOM side, when grid is on TOP border */
    if ((ny == grid_y)&&(2 != side)) {
      continue;
    }
    /* Pin can only locate on LEFT side, when grid is on RIGHT border */
    if ((nx == grid_x)&&(3 != side)) {
      continue;
    }
    /* Pin can only locate on the TOP side, when grid is on BOTTOM border */
    if ((0 == grid_y)&&(0 != side)) {
      continue;
    }
    /* Pin can only locate on the RIGHT side, when grid is on LEFT border */
    if ((0 == grid_x)&&(1 != side)) {
      continue;
    }
    if (1 == grid_type->pinloc[pin_height][side][pin_index]) {
      if (-1 != pin_side) { 
        vpr_printf(TIO_MESSAGE_ERROR, "(%s, [LINE%d]) Duplicated pin(index:%d) on two sides: %s and %s of type (name=%s)!\n",
                   __FILE__, __LINE__, 
                   pin_index, 
                   convert_side_index_to_string(pin_side),
                   convert_side_index_to_string(side),
                   grid_type->name);
        exit(1);
      }
      pin_side = side;
    }
  }
  
  return pin_side;
}

/* Decode mode bits "01..." to a SRAM bits array */
int* decode_mode_bits(char* mode_bits, int* num_sram_bits) {
  int* sram_bits = NULL;
  int i;
  
  assert(NULL != mode_bits);
  (*num_sram_bits) = strlen(mode_bits);
 
  sram_bits = (int*)my_calloc((*num_sram_bits), sizeof(int));

  for (i = 0; i < (*num_sram_bits); i++) {
    switch(mode_bits[i]) {
    case '1':
      sram_bits[i] = 1;
      break;
    case '0':
      sram_bits[i] = 0;
      break;
    default: 
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid mode_bits(%s)!\n",
                 __FILE__, __LINE__, mode_bits);
      exit(1);
    }
  }

  return sram_bits;
}

enum e_interconnect determine_actual_pb_interc_type(t_interconnect* def_interc, 
                                                    int fan_in) {
  enum e_interconnect actual_interc_type = DIRECT_INTERC;

  /* Check */
  assert ((NULL != def_interc) && (0 != fan_in)); 

  /* Initialize the interconnection type that will be implemented in SPICE netlist*/
  switch (def_interc->type) {
    case DIRECT_INTERC:
      assert(1 == fan_in);
      actual_interc_type = DIRECT_INTERC;
      break;
    case COMPLETE_INTERC:
      if (1 == fan_in) {
        actual_interc_type = DIRECT_INTERC;
      } else {
        assert((2 == fan_in)||(2 < fan_in));
        actual_interc_type = MUX_INTERC;
      }
      break;
    case MUX_INTERC:
      assert((2 == fan_in)||(2 < fan_in));
      actual_interc_type = MUX_INTERC;
      break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid interconnection type for %s (Arch[LINE%d])!\n",
               __FILE__, __LINE__, def_interc->name, def_interc->line_num);
    exit(1);
  }

  return actual_interc_type;
} 


/* Count the number of pins in a pb_graph node */
int count_pin_number_one_port_pb_graph_node(int num_ports, int* num_pins) {
  int total_pins = 0;
  int iport;

  for (iport = 0; iport < num_ports; iport++) {
    total_pins += num_pins[iport];
  }

  return total_pins;
}

/* Count the number of pins in a pb_graph node */
int count_pin_number_one_pb_graph_node(t_pb_graph_node* cur_pb_graph_node) {
  int total_pin = 0;
  int iport;

  /* INPUT port */
  total_pins += count_pin_number_one_port_pb_graph_node(cur_pb_graph_node->num_input_ports,
                                                        cur_pb_graph_node->num_input_pins);

  /* OUTPUT port */
  total_pins += count_pin_number_one_port_pb_graph_node(cur_pb_graph_node->num_output_ports,
                                                        cur_pb_graph_node->num_output_pins);

  /* CLOCK port */
  total_pins += count_pin_number_one_port_pb_graph_node(cur_pb_graph_node->num_clock_ports,
                                                        cur_pb_graph_node->num_clock_pins);
  

  return total_pin;
}

/* find the number of pb_graph edges with a given physical mode index */
int count_pb_graph_node_input_edge_in_phy_mode(t_pb_graph_pin* cur_pb_graph_pin,
                                               int phy_mode_index) {
  int cnt = 0;
  int iedge;

  for (iedge = 0; iedge < cur_pb_graph_pin->num_input_edges; iedge++) {
    if (phy_mode_index == cur_pb_graph_pin->input_edges[iedge]->interconnect->parent_mode_index) {
      cnt++;
    }
  }
  return cnt;
}

int count_pb_graph_node_output_edge_in_phy_mode(t_pb_graph_pin* cur_pb_graph_pin,
                                                int phy_mode_index) {
  int cnt = 0;
  int iedge;

  for (iedge = 0; iedge < cur_pb_graph_pin->num_output_edges; iedge++) {
    if (phy_mode_index == cur_pb_graph_pin->output_edges[iedge]->interconnect->parent_mode_index) {
      cnt++;
    }
  }
  return cnt;
}

/* With a given name, find the pb_type by recursively traversing the pb_type_tree */
t_pb_type* rec_get_pb_type_by_name(t_pb_type* cur_pb_type, 
                                   char* pb_type_name) {
  t_pb_type* ret_pb_type = NULL;
  t_pb_type* found_pb_type = NULL;
 
  /* Check the name of this pb_type */ 
  if (0 == strcmp(cur_pb_type->name, pb_type_name)) {
    ret_pb_type = cur_pb_type;
  }

  /* We cannot find what we want this level, go recursively */
  /* Check each mode*/
  for (imode = 0; imode < cur_pb_type->num_modes; imode++) {
    /* bypass physical modes */
    if (imode == find_pb_type_physical_mode_index((*cur_pb_type))) {
      continue;
    }
     /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[imode].num_pb_type_children; ipb++) {
      /* we should make sure this placement index == child_pb_type[jpb]*/
      found_pb_type = rec_get_pb_type_by_name(&(cur_pb_type->modes[imode].pb_type_children[ipb]), pb_type_name);
      if (NULL == found_pb_type) { /* See if we have found anything*/
        continue;
      }
      /* We find something, check if we have a overlap in naming */
      if (NULL != ret_pb_type) {
        vpr_printf(TIO_MESSAGE_ERROR,
                   "(File:%s,[LINE%d])Duplicated pb_type name(%s) is not allowed in pb_types!\n",
                   __FILE__, __LINE__, pb_type_name);
        exit(1);
      } else { /* We are free of naming conflict, assign the return value */
        ret_pb_type = found_pb_type;
      }
    }
  }

  return ret_pb_type;
}

/* Decode an annotation (a string): <port_name>[<msb>:<lsb>] */
void decode_physical_mode_pin_annotation(char* phy_mode_pin,
                                         char** port_name,
                                         int* pin_msb, int* pin_lsb) {
  int itoken;
  int num_tokens = 0;
  char* token = NULL;

  token = my_strtok(phy_mode_pin, "[:]", &num_tokens);

  /* 1 == num_token */
  switch (num_tokens) {
  case 1:
    (*port_name) = my_strdup(phy_mode_pin);
    (*pin_msb) = 0;
    (*pin_lsb) = 0;
    break;
  case 2:
    (*port_name) = my_strdup(token[0]);
    (*pin_msb) = my_atoi(token[1]);
    (*pin_lsb) = (*pin_msb);
    break;
  case 3:
    (*port_name) = my_strdup(token[0]);
    (*pin_msb) = my_atoi(token[1]);
    (*pin_lsb) = my_atoi(token[2]);
    /* Identify which is larger: pin_msb and pin_lsb */
    (*pin_msb) = ((*pin_msb) > (*pin_lsb)) ? (*pin_msb) : (*pin_lsb);
    (*pin_msb) = ((*pin_msb) > (*pin_lsb)) ? (*pin_lsb) : (*pin_msb);
    break;
  default:
    /* Error out! */
    vpr_printf(TIO_MESSAGE_ERROR,
                 "(File:%s,[LINE%d])Invalid physical_mode_pin: %s!\n",
                  __FILE__, __LINE__, phy_mode_pin);
    exit(1);
  }

  /* Free tokens */
  for (itoken = 0; itoken < num_tokens; itoken++) {
    my_free(token[itoken]);
  }
  my_free(token);
 
  return;
}

/* Decode the physical_mode_pin definition in cur_pb_type_port
 * Annotate it to the ports of phy_pb_type
 */
void annotate_physical_mode_pin_to_pb_type(t_port* cur_pb_type_port,
                                           t_pb_type* phy_pb_type) {
  int iport; 
  char* phy_port_name = NULL;
  int msb, lsb;
  int port_matched = 0;

  /* Check */
  assert ( TRUE == phy_pb_type->parent_mode->define_physical_mode );

  /* Decode the physical_mode_pin */
  decode_physical_mode_pin_annotation(phy_mode_pin, &phy_port_name, &msb, &lsb);

  /* Search phy_pb_port ports */
  for (iport = 0; iport < phy_pb_type->num_ports; iport) { 
    if (0 == strcmp(phy_port_name, phy_pb_type->ports[iport].name)) {
      /* We got a match! Give the lsb, msb and create a link */
      cur_pb_type_port->phy_pb_type_port = &(phy_pb_type->ports[iport]);
      cur_pb_type_port->phy_pb_type_port_msb = msb;      
      cur_pb_type_port->phy_pb_type_port_lsb = lsb;      
      port_matched++;
    }
  }

  /* Check if the port is unique */
  assert (1 == port_matched);
  /* Check if the pin number match */
  assert(!(cur_pb_type_type->phy_pb_type_port->num_pins < (msb - lsb)));

  /* Free */  
  my_free(phy_port_name);

  return;
}

/* Annotate the port-to-port definition from cur_pb_type to a physical pb_type */
void annotate_pb_type_port_to_phy_pb_type(t_pb_type* cur_pb_type, 
                                          t_pb_type* phy_pb_type) {
  int iport; 

  /* Check */
  assert ( TRUE == phy_pb_type->parent_mode->define_physical_mode );
  assert ( NULL != cur_pb_type->phy_pb_type );

  /* Check each port of cur_pb_type */
  for (iport = 0; iport < cur_pb_type->num_ports; iport) { 
    annotate_physical_mode_pin_to_pb_type(cur_pb_type->ports[iport], phy_pb_type);
  }

  return;
}

/* Find a pb_graph_node with a given pb_type_name in placement_index_in_top */
t_pb_graph_node* rec_get_pb_graph_node_by_pb_type_and_placement_index_in_top_node(t_pb_graph_node* cur_pb_graph_node, 
                                                                                  t_pb_type* target_pb_type,
                                                                                  int target_placement_index) {
  t_pb_graph_node* ret_pb_graph_node = NULL;
  t_pb_graph_node* found_pb_graph_node = NULL;
 
  /* Check if pb_type matches and also the placement_index */ 
  if (( target_pb_type == cur_pb_graph_node->pb_type )
     &&( target_placement_index == cur_pb_graph_node->placement_index_in_top_node )) {
    ret_pb_graph_node = cur_pb_graph_node;
  }

  /* We cannot find what we want this level, go recursively */
  /* Check each mode*/
  for (imode = 0; imode < cur_pb_type->num_modes; imode++) {
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[imode].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[imode].pb_type_children[ipb].num_pb; jpb++) {
        found_pb_graph_node = rec_get_pb_graph_node_by_pb_type_and_placement_index_in_top_node(&(cur_pb_graph_node->child_pb_graph_nodes[imode][ipb][jpb])i,
                                                                                               target_pb_type, target_placement_index);
      
        if (NULL == found_pb_graph_node) { /* See if we have found anything*/
          continue;
        }
        /* We find something, check if we have a overlap in naming */
        if (NULL != ret_pb_graph_node) {
          vpr_printf(TIO_MESSAGE_ERROR,
                     "(File:%s,[LINE%d])Duplicated pb_graph_node name %s[%d] is not allowed in pb_graph_node!\n",
                     __FILE__, __LINE__, target_pb_type->name, target_placement_index);
          exit(1);
        } else { /* We are free of naming conflict, assign the return value */
          ret_pb_graph_node = found_pb_graph_node;
        }
      }
    }
  }

  return ret_pb_graph_node;
}

/* Check if the pin_number of cur_pb_graph_pin matches the phycial pb_graph_pin */
boolean check_pin_number_match_phy_pb_graph_pin(t_pb_graph_pin* cur_pb_graph_pin, 
                                                t_pb_graph_pin* phy_pb_graph_pin) {
  boolean pin_number_match = FALSE;

  if ( (cur_pb_graph_pin->port->phy_pb_type_port == phy_pb_graph_pin->port)
     &&(cur_pb_graph_pin->pin_number + cur_pb_graph_pin->port->phy_pb_type_port_lsb == phy_pb_graph_pin->pin_number)) {
    pin_number_match = TRUE;
  }
  return pin_number_match; 
}

/* Link a port in the pb_graph_node to its physical_pb_graph_node port */
void link_one_pb_graph_node_pin_to_phy_pb_graph_pin(t_pb_graph_pin* cur_pb_graph_pin, 
                                                    t_pb_graph_node* phy_pb_graph_node) {
  t_pb_graph_pin* phy_pb_graph_pin = NULL;
  int iport, ipin;

  /* Get the name match pin in the phy_graph_node */
  for (iport = 0; iport < phy_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < phy_pb_graph_node->num_input_pins[iport]; ipin++) {
      if (FALSE == check_pin_number_match_phy_pb_graph_pin(cur_pb_graph_pin, &(phy_pb_graph_node->input_pins[iport][ipin]))) {
        continue;
      }
      if (NULL == phy_pb_graph_pin) {
        phy_pb_graph_pin = phy_pb_graph_node->input_pins[iport][ipin];
      } else {
        vpr_printf(TIO_MESSAGE_ERROR,
                   "(File:%s,[LINE%d]) More than one matched pin number found for %s[%d] in %s!\n",
                   __FILE__, __LINE__, cur_pb_graph_pin->port->name, cur_pb_graph_pin->pin_number, phy_pb_graph_node->pb_type->name);
        exit(1);
      }
    }
  }

  for (iport = 0; iport < phy_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < phy_pb_graph_node->num_output_pins[iport]; ipin++) {
      if (FALSE == check_pin_number_match_phy_pb_graph_pin(cur_pb_graph_pin, &(phy_pb_graph_node->output_pins[iport][ipin]))) {
        continue;
      }
      if (NULL == phy_pb_graph_pin) {
        phy_pb_graph_pin = phy_pb_graph_node->output_pins[iport][ipin];
      } else {
        vpr_printf(TIO_MESSAGE_ERROR,
                   "(File:%s,[LINE%d]) More than one matched pin number found for %s[%d] in %s!\n",
                   __FILE__, __LINE__, cur_pb_graph_pin->port->name, cur_pb_graph_pin->pin_number, phy_pb_graph_node->pb_type->name);
        exit(1);
      }

    }
  }

  for (iport = 0; iport < phy_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < phy_pb_graph_node->num_clock_pins[iport]; ipin++) {
      if (FALSE == check_pin_number_match_phy_pb_graph_pin(cur_pb_graph_pin, &(phy_pb_graph_node->clock_pins[iport][ipin]))) {
        continue;
      }
      if (NULL == phy_pb_graph_pin) {
        phy_pb_graph_pin = phy_pb_graph_node->clock_pins[iport][ipin];
      } else {
        vpr_printf(TIO_MESSAGE_ERROR,
                   "(File:%s,[LINE%d]) More than one matched pin number found for %s[%d] in %s!\n",
                   __FILE__, __LINE__, cur_pb_graph_pin->port->name, cur_pb_graph_pin->pin_number, phy_pb_graph_node->pb_type->name);
        exit(1);
      }

    }
  }

  /* We should find one! */
  if (NULL == phy_pb_graph_pin) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "(File:%s,[LINE%d]) No matched pin number found for %s[%d] in %s!\n",
                __FILE__, __LINE__, cur_pb_graph_pin->port->name, cur_pb_graph_pin->pin_number, phy_pb_graph_node->pb_type->name);
    exit(1);
  }
  /* Create the link */
  cur_pb_graph_pin->physical_pb_graph_pin = phy_pb_graph_pin;

  return;
}

/* Link the pb_graph_pins of a pb_graph_node to its physical pb_graph_node by the annotation in pb_type ports 
 * pb_graph_node A contains the annotation, while pb_graph_node B is the physical_pb_graph_node */
void link_pb_graph_node_pins_to_phy_pb_graph_pins(t_pb_graph_node* cur_pb_graph_node, 
                                                  t_pb_graph_node* phy_pb_graph_node) {
  int iport, ipin;

  /* Search each port of cur_pb_graph_node and 
   * check matched port name in phy_pb_graph_node
   */
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
      link_one_pb_graph_node_pin_to_phy_pb_graph_pin(&(cur_pb_graph_node->input_pins[iport][ipin]), 
                                                     phy_pb_graph_node);
    }
  }

  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      link_one_pb_graph_node_pin_to_phy_pb_graph_pin(&(cur_pb_graph_node->output_pins[iport][ipin]), 
                                                     phy_pb_graph_node);
    }
  }

  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
      link_one_pb_graph_node_pin_to_phy_pb_graph_pin(&(cur_pb_graph_node->clock_pins[iport][ipin]), 
                                                     phy_pb_graph_node);
    }
  }

  return;
}

void rec_reset_pb_graph_node_rr_node_index_physical_pb(t_pb_graph_node* cur_pb_graph_node) {
  int imode, ipb, jpb;
  t_pb_type* cur_pb_type = cur_pb_graph_node->pb_type;

  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
      cur_pb_graph_node->input_pins[iport][ipin].rr_node_index_physical_pb = OPEN;
    }
  }

  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
      cur_pb_graph_node->output_pins[iport][ipin].rr_node_index_physical_pb = OPEN;
    }
  }

  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
      cur_pb_graph_node->clock_pins[iport][ipin].rr_node_index_physical_pb = OPEN;
    }
  }

  /* END until primitive node */
  if (NULL != cur_pb_type->spice_model) {
    return;
  }

  /* We cannot find what we want this level, go recursively */
  /* Check each mode*/
  for (imode = 0; imode < cur_pb_type->num_modes; imode++) {
    /* Quote all child pb_types */
    for (ipb = 0; ipb < cur_pb_type->modes[imode].num_pb_type_children; ipb++) {
      /* Each child may exist multiple times in the hierarchy*/
      for (jpb = 0; jpb < cur_pb_type->modes[imode].pb_type_children[ipb].num_pb; jpb++) {
        rec_reset_pb_graph_node_rr_node_index_physical_pb(&(cur_pb_graph_node->child_pb_graph_nodes[imode][ipb][jpb]));
      }
    }
  }

  return;
}

/* Allocate empty child_phy_pbs according to a pb_graph_node */
void rec_alloc_phy_pb_children(t_pb_graph_node* cur_pb_graph_node, 
                               t_phy_pb* cur_phy_pb) {
  int phy_mode_index;
  t_pb_type* cur_pb_type = cur_pb_graph_node->pb_type;
  char* phy_pb_name = NULL;

  phy_pb_name = (char*) my_malloc(sizeof(char) * (strlen(cur_pb_graph_node->pb_type->name)
                                                  + 1 + strlen(my_itoa(cur_pb_graph_node->placement_index_in_top_node)) 
                                                  + 2));
  sprintf(phy_pb_name, "%s[%d]", 
          cur_pb_graph_node->pb_type->name, cur_pb_graph_node->placement_index_in_top_node);

  /* Initialize */
  cur_phy_pb->pb_graph_node = cur_pb_graph_node;
  cur_phy_pb->name = phy_pb_name;
  cur_phy_pb->num_logical_blocks = 0;
  cur_phy_pb->logical_block = NULL;

  /* Return if we reach the primitive node */
  if (NULL != cur_pb_type->spice_model) {
    return;
  }

  /* Contine recursively */
  phy_mode_index = find_pb_type_physical_mode_index((*cur_pb_type));
  /* Quote all child pb_types */
  for (ipb = 0; ipb < cur_pb_type->modes[phy_mode_index].num_pb_type_children; ipb++) {
    /* Each child may exist multiple times in the hierarchy*/
    for (jpb = 0; jpb < cur_pb_type->modes[phy_mode_index].pb_type_children[ipb].num_pb; jpb++) {
      rec_alloc_phy_pb_children(&(cur_pb_graph_node->child_pb_graph_nodes[phy_mode_index][ipb][jpb]),
                                cur_phy_pb->child_pbs[ipb][jpb]);
      /* Assign parent_pb */
      cur_phy_pb->child_pbs[ipb][jpb].parent_pb = cur_phy_pb;
      cur_phy_pb->child_pbs[ipb][jpb].mode = phy_mode_index;
    }
  }

  /* Allocate */
  cur_phy_pb->child_pbs = (t_phy_pb**) my_calloc(cur_pb_type->modes[phy_mode_index].pb_type_children, sizeof(t_phy_pb*));
  for (ipb = 0; ipb < cur_pb_type->modes[phy_mode_index].num_pb_type_children; ipb++) {
    cur_phy_pb->child_pbs[ipb] = (t_phy_pb*) my_calloc(cur_pb_type->modes[phy_mode_index].pb_type_children[ipb].num_pb, sizeof(t_phy_pb));
  }

  return;
}

/* With a given name, find the pb_type by recursively traversing the pb_type_tree */
t_phy_pb* rec_get_phy_pb_by_name(t_phy_pb* cur_phy_pb, 
                                 char* phy_pb_name) {
  t_phy_pb* ret_phy_pb = NULL;
  t_phy_pb* found_phy_pb = NULL;
 
  /* Check the name of this pb_type */ 
  if (0 == strcmp(cur_phy_pb->name, phy_pb_name)) {
    ret_phy_pb = cur_phy_pb;
  }

  for (ipb = 0; ipb < cur_phy_pb->pb_graph_node->pb_type->modes[cur_phy_pb->mode].num_pb_type_children; ipb++) {
    /* Each child may exist multiple times in the hierarchy*/
    for (jpb = 0; jpb < cur_phy_pb->pb_graph_node->pb_type->modes[cur_phy_pb->mode].pb_type_children[ipb].num_pb; jpb++) {
      /* we should make sure this placement index == child_pb_type[jpb]*/
      found_phy_pb = rec_get_phy_pb_by_name(&(cur_pb_pb->child_pbs[ipb][jpb]), phy_pb_name);
      if (NULL == found_phy_pb) { /* See if we have found anything*/
        continue;
      }
      /* We find something, check if we have a overlap in naming */
      if (NULL != ret_phy_pb) {
        vpr_printf(TIO_MESSAGE_ERROR,
                   "(File:%s,[LINE%d])Duplicated phy_pb name(%s) is not allowed in phy_pb!\n",
                   __FILE__, __LINE__, phy_pb_name);
        exit(1);
      } else { /* We are free of naming conflict, assign the return value */
        ret_phy_pb = found_phy_pb;
      }
    }
  }

  return ret_phy_pb;
}


/* Synchronize the mapped information from operating pb cur_pb to phy_pb */
void sync_op_pb_mapping_to_phy_pb_children(t_pb* cur_op_pb, 
                                           t_phy_pb* cur_phy_pb) {
  int ipb, jpb;
  t_pb_graph_node* cur_pb_graph_node = cur_op_pb->pb_graph_node;
  t_pb_type* cur_pb_type = cur_pb_graph_node->pb_type;
  int mode_index = cur_pb->mode; 
  t_pb* child_pb = NULL;
  char* phy_pb_name = NULL;
  t_phy_pb* phy_pb_to_sync = NULL;

  /* Return if we reach the primitive node */
  if (NULL != cur_pb_type->spice_model) {
    /* Check */
    assert(NULL != cur_pb_type->phy_pb_type);
    assert(NULL != cur_pb_graph_node->physical_pb_graph_node);
    /* Generate the name */
    phy_pb_name = (char*) my_malloc(sizeof(char) * (strlen(cur_pb_type->phy_pb_type->name)
                                                    + 1 + strlen(my_itoa(cur_pb_graph_node->physical_pb_graph_node->placement_index_in_top_node)) 
                                                    + 2));
    sprintf(phy_pb_name, "%s[%d]", 
            cur_pb_type->phy_pb_type->name, cur_pb_graph_node->physical_pb_graph_node->placement_index_in_top_node);
    /* find the child_pb in the current physical pb (cur_phy_pb) */
    phy_pb_to_sync = rec_get_phy_pb_by_name(cur_phy_pb, phy_pb_name);
    /* Check */
    assert (phy_pb_to_sync->spice_model == cur_pb_type->spice_model);
    /* Re-allocate logical_block array mapped to this pb */
    phy_pb_to_sync->num_logical_blocks++;
    phy_pb_to_sync->logical_block = (int*) my_realloc(phy_pb_to_sync->logical_block, sizeof(int) * phy_pb_to_sync->num_logical_blocks);
    /* Synchronize the logic block information */
    switch (cur_pb_type->class_type) {
    case LUT_CLASS: 
      child_pb = get_lut_child_pb(cur_op_pb, mode_index);
      phy_pb_to_sync->logical_block[phy_pb_to_syn->num_logical_blocks - 1] = child_pb->logical_block;
      break;
    case LATCH_CLASS:
      phy_pb_to_sync->logical_block[phy_pb_to_syn->num_logical_blocks - 1] = cur_op_pb->logical_block;
      break;
    case MEMORY_CLASS:
      child_pb = get_hardlogic_child_pb(cur_op_pb, mode_index); 
      phy_pb_to_sync->logical_block[phy_pb_to_syn->num_logical_blocks - 1] = child_pb->logical_block;
      break;  
    case UNKNOWN_CLASS:
      phy_pb_to_sync->logical_block[phy_pb_to_syn->num_logical_blocks - 1] = cur_op_pb->logical_block;
      break;  
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Unknown class type of pb_type(%s)!\n",
                 __FILE__, __LINE__, cur_pb_type->name);
      exit(1);
    }
 
    /* Free */
    my_free(phy_pb_name);
    return;
  }

  /* Recursive*/
  for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Refer to pack/output_clustering.c [LINE 392] */
      if ((NULL != cur_op_pb->child_pbs[ipb])&&(NULL != cur_op_pb->child_pbs[ipb][jpb].name)) {
        sync_op_pb_mapping_to_phy_pb_children(&(cur_op_pb->child_pbs[ipb][jpb], cur_phy_pb);
      }
    }
  }

  return;
}


/* Allocate pb children for a physical pb, according to the results in cur_pb*/
void alloc_and_load_phy_pb_children_for_one_mapped_block(t_pb* cur_pb,
                                                         t_phy_pb* cur_phy_pb) {

  /* allocate empty child pbs according to pb_graph_node  */
  rec_alloc_phy_pb_children(cur_phy_pb->pb_graph_node, cur_phy_pb);

  /* Synchronize the cur_pb to cur_phy_pb */
  sync_op_pb_mapping_to_phy_pb_children(cur_op_pb, cur_phy_pb);

  return;
}
