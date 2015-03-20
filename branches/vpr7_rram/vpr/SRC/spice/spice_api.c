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
#include "path_delay.h"
#include "stats.h"

/* Include spice support headers*/
#include "read_xml_spice_util.h"
#include "linkedlist.h"
#include "spice_globals.h"
#include "spice_utils.h"
#include "spice_subckt.h"
#include "spice_pbtypes.h"
#include "spice_heads.h"
#include "spice_top_netlist.h"
#include "spice_mux_testbench.h"
#include "spice_grid_testbench.h"

/* For mrFPGA */
#ifdef MRFPGA_H
#include "mrfpga_globals.h"
#endif

/***** Subroutines Declarations *****/
static 
void back_annotate_rr_node_map_info();

static 
void backannotate_clb_nets_act_info();

static 
void free_clb_nets_spice_net_info();

void init_list_include_netlists(t_spice* spice); 

static 
void rec_stat_pb_type_keywords(t_pb_type* cur_pb_type,
                               int* num_keyword);

static 
void rec_add_pb_type_keywords_to_list(t_pb_type* cur_pb_type,
                                      int* cur,
                                      char** keywords,
                                      char* prefix);

void check_keywords_conflict(t_arch Arch);

void rec_backannotate_rr_node_net_num(int LL_num_rr_nodes,
                                      t_rr_node* LL_rr_node,
                                      int src_node_index);

static 
void build_prev_node_list_rr_nodes(int LL_num_rr_nodes,
                                   t_rr_node* LL_rr_nodes);

static 
void fprint_run_hspice_shell_script(char* spice_dir_path,
                                    char* subckt_dir_path,
                                    char* top_netlist_file, 
                                    char* grid_testbench_file, 
                                    char* routing_testbench_file, 
                                    char* mux_testbench_file);

/***** Subroutines *****/
/* Initialize and check spice models in architecture
 * Tasks:
 * 1. Link the spice model defined in pb_types and routing switches
 * 2. Add default spice model (MUX) if needed
 */
void init_check_arch_spice_models(t_arch* arch,
                                  t_det_routing_arch* routing_arch) {
  int i;

  vpr_printf(TIO_MESSAGE_INFO,"Initializing and checking SPICE models...\n");
  /* Check Spice models first*/
  assert(NULL != arch);
  assert(NULL != arch->spice);
  if ((0 == arch->spice->num_spice_model)||(0 > arch->spice->num_spice_model)) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])SPICE models are not defined! Miss this part in architecture file.\n",__FILE__,__LINE__);
    exit(1);
  }
  assert(NULL != arch->spice->spice_models);
  
  /* Find default spice model*/
  /* MUX */
  if (NULL == get_default_spice_model(SPICE_MODEL_MUX,
                                      arch->spice->num_spice_model, 
                                      arch->spice->spice_models)) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Fail to find the default MUX SPICE Model! Should define it in architecture file\n",__FILE__,__LINE__);
    exit(1);
  }

  /* Channel Wire */
  if (NULL == get_default_spice_model(SPICE_MODEL_CHAN_WIRE,
                                      arch->spice->num_spice_model, 
                                      arch->spice->spice_models)) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Fail to find the default Channel Wire SPICE Model! Should define it in architecture file\n",__FILE__,__LINE__);
    exit(1);
  }

  /* Wire */
  if (NULL == get_default_spice_model(SPICE_MODEL_WIRE,
                                      arch->spice->num_spice_model, 
                                      arch->spice->spice_models)) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Fail to find the default Wire SPICE Model! Should define it in architecture file\n",__FILE__,__LINE__);
    exit(1);
  }

  /* 1. Link the spice model defined in pb_types and routing switches */
  /* Step A:  Check routing switches, connection blocks*/
  if ((0 == arch->num_cb_switch)||(0 > arch->num_cb_switch)) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d]) Define Switches for Connection Blocks is mandatory in SPICE model support! Miss this part in architecture file.\n",__FILE__,__LINE__);
    exit(1);
  }
  
  for (i = 0; i < arch->num_cb_switch; i++) {
    arch->cb_switches[i].spice_model = 
      find_name_matched_spice_model(arch->cb_switches[i].spice_model_name,
                                    arch->spice->num_spice_model, 
                                    arch->spice->spice_models); 
    if (NULL == arch->cb_switches[i].spice_model) {
      vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Invalid SPICE model name(%s) of Switch(%s) is undefined in SPICE models!\n",__FILE__ ,__LINE__, arch->cb_switches[i].spice_model_name, arch->cb_switches[i].name);
      exit(1);
    }
  } 
 
  /* Step B: Check switch list: Switch Box*/
  if ((0 == arch->num_switches)||(0 > arch->num_switches)) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d]) Define Switches for Switch Boxes is mandatory in SPICE model support! Miss this part in architecture file.\n",__FILE__,__LINE__);
    exit(1);
  }
  
  for (i = 0; i < arch->num_switches; i++) {
    arch->Switches[i].spice_model = 
      find_name_matched_spice_model(arch->Switches[i].spice_model_name,
                                    arch->spice->num_spice_model, 
                                    arch->spice->spice_models); 
    if (NULL == arch->Switches[i].spice_model) {
      vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Invalid SPICE model name(%s) of Switch(%s) is undefined in SPICE models!\n",__FILE__ ,__LINE__, arch->Switches[i].spice_model_name, arch->Switches[i].name);
      exit(1);
    }
  } 

  /* Update the switches in detailed routing architecture settings*/
  for (i = 0; i < routing_arch->num_switch; i++) {
    if (NULL == switch_inf[i].spice_model_name) { 
      switch_inf[i].spice_model = get_default_spice_model(SPICE_MODEL_MUX,
                                                          arch->spice->num_spice_model, 
                                                          arch->spice->spice_models);
      continue;
    }
    switch_inf[i].spice_model = 
      find_name_matched_spice_model(switch_inf[i].spice_model_name,
                                    arch->spice->num_spice_model, 
                                    arch->spice->spice_models); 
    if (NULL == switch_inf[i].spice_model) {
      vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Invalid SPICE model name(%s) of Switch(%s) is undefined in SPICE models!\n",__FILE__ ,__LINE__, switch_inf[i].spice_model_name, switch_inf[i].name);
      exit(1);
    }
  }

  /* Step C: Find SRAM Model*/
  if (NULL == arch->sram_inf.spice_model_name) { 
      arch->sram_inf.spice_model = get_default_spice_model(SPICE_MODEL_SRAM,
                                                           arch->spice->num_spice_model, 
                                                           arch->spice->spice_models);
  } else {
    arch->sram_inf.spice_model = 
        find_name_matched_spice_model(arch->sram_inf.spice_model_name,
                                      arch->spice->num_spice_model, 
                                      arch->spice->spice_models); 
  }
  if (NULL == arch->sram_inf.spice_model) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Invalid SPICE model name(%s) of SRAM is undefined in SPICE models!\n",__FILE__ ,__LINE__, arch->Switches[i].spice_model_name, arch->Switches[i].name);
    exit(1);
  }
  /* Find the sram model and assign the global variable*/
  sram_spice_model = arch->sram_inf.spice_model;

  /* Step D: Find the segment spice_model*/
  for (i = 0; i < arch->num_segments; i++) {
    if (NULL == arch->Segments[i].spice_model_name) {
      arch->Segments[i].spice_model = 
        get_default_spice_model(SPICE_MODEL_CHAN_WIRE,
                                arch->spice->num_spice_model, 
                                arch->spice->spice_models); 
      continue;
    } else {
      arch->Segments[i].spice_model = 
        find_name_matched_spice_model(arch->Segments[i].spice_model_name,
                                      arch->spice->num_spice_model, 
                                      arch->spice->spice_models); 
    }
    if (NULL == arch->Segments[i].spice_model) {
      vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Invalid SPICE model name(%s) of Segment(Length:%d) is undefined in SPICE models!\n",__FILE__ ,__LINE__, arch->Segments[i].spice_model_name, arch->Segments[i].length);
      exit(1);
    } else if (SPICE_MODEL_CHAN_WIRE != arch->Segments[i].spice_model->type) {
      vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d])Invalid SPICE model(%s) type of Segment(Length:%d)! Should be chan_wire!\n",__FILE__ ,__LINE__, arch->Segments[i].spice_model_name, arch->Segments[i].length);
      exit(1);
    }
  } 

  /* 2. Search Complex Blocks (Pb_Types), Link spice_model according to the spice_model_name*/
  for (i = 0; i < num_types; i++) {
    if (type_descriptors[i].pb_type) {
       match_pb_types_spice_model_rec(type_descriptors[i].pb_type,
                                      arch->spice->num_spice_model,
                                      arch->spice->spice_models);
    }
  }

  return;
}

/***** Recursively Backannotate parasitic_net_num for a rr_node*****/
void rec_backannotate_rr_node_net_num(int LL_num_rr_nodes,
                                      t_rr_node* LL_rr_node,
                                      int src_node_index) {
  int iedge, to_node;
  
  /* Traversal until 
   * 1. we meet a sink
   * 2. None of the edges propagates this net_num 
   */
  for (iedge = 0; iedge < LL_rr_node[src_node_index].num_edges; iedge++) {
    to_node = LL_rr_node[src_node_index].edges[iedge];
    if (src_node_index == LL_rr_node[to_node].prev_node) {
      assert(iedge == LL_rr_node[to_node].prev_edge);
      /* assert(LL_rr_node[src_node_index].net_num == LL_rr_node[to_node].net_num); */
      /* Propagate the net_num */
      LL_rr_node[to_node].net_num = LL_rr_node[src_node_index].net_num; 
      /* Go recursively */ 
      rec_backannotate_rr_node_net_num(LL_num_rr_nodes, LL_rr_node, to_node);
    }
  }
  
  return;
} 

/***** Backannotate activity information to nets *****/
static 
void backannotate_rr_nodes_net_info() {
  int inode, inet;
  t_trace* head = NULL;

  /* Initialize the net_num */
  for (inode = 0; inode < num_rr_nodes; inode++) {
    rr_node[inode].net_num = OPEN;
  }
  
  /* Mark mapped rr_nodes with net_num*/
  for (inet = 0; inet < num_nets; inet++) {
    /* Skip Global nets and dangling nets */
    if ((TRUE == clb_net[inet].is_global)||(FALSE == clb_net[inet].num_sinks)) {
      continue;
    }
    /* Find the trace_head */
    head = trace_head[inet];
    /* Loop util we reach the tail */
    while (head) {
      rr_node[head->index].net_num = inet; 
      /* Map parastic rr_nodes net_num */
      rec_backannotate_rr_node_net_num(num_rr_nodes, rr_node, head->index); 
      head = head->next;
    }
  }
 
  return;
}

static 
void backannotate_clb_nets_act_info() {
  int inet;
  
  /* Free all spice_net_info and reallocate */
  for (inet = 0; inet < num_nets; inet++) {
    if (NULL == clb_net[inet].spice_net_info) {
      /* Allocate */
      clb_net[inet].spice_net_info = (t_spice_net_info*)my_malloc(sizeof(t_spice_net_info));
    } 
    /* Initialize to zero */
    init_spice_net_info(clb_net[inet].spice_net_info);
    /* Load activity info */
    clb_net[inet].spice_net_info->probability = vpack_net[clb_to_vpack_net_mapping[inet]].net_power->probability;
    clb_net[inet].spice_net_info->density = vpack_net[clb_to_vpack_net_mapping[inet]].net_power->density;
  }

  /* Free all spice_net_info and reallocate */
  for (inet = 0; inet < num_logical_nets; inet++) {
    if (NULL == vpack_net[inet].spice_net_info) {
      /* Allocate */
      vpack_net[inet].spice_net_info = (t_spice_net_info*)my_malloc(sizeof(t_spice_net_info));
    } 
    /* Initialize to zero */
    init_spice_net_info(vpack_net[inet].spice_net_info);
    /* Load activity info */
    vpack_net[inet].spice_net_info->probability = vpack_net[inet].net_power->probability;
    vpack_net[inet].spice_net_info->density = vpack_net[inet].net_power->density;
  }

  return;
}

static 
void free_clb_nets_spice_net_info() {
  int inet;
  
  /* Free all spice_net_info and reallocate */
  for (inet = 0; inet < num_nets; inet++) {
    my_free(clb_net[inet].spice_net_info);
  }

  for (inet = 0; inet < num_logical_nets; inet++) {
    my_free(vpack_net[inet].spice_net_info);
  }

  return;
}

void init_list_include_netlists(t_spice* spice) { 
  int i, j, cur;
  int to_include = 0;
  int num_to_include = 0;

  /* Initialize */
  for (i = 0; i < spice->num_include_netlist; i++) { 
    FreeSpiceModelNetlist(&(spice->include_netlists[i]));
  }
  my_free(spice->include_netlists);
  spice->include_netlists = NULL;
  spice->num_include_netlist = 0;

  /* Generate include netlist list */
  vpr_printf(TIO_MESSAGE_INFO, "Listing Netlist Names to be included...\n");
  for (i = 0; i < spice->num_spice_model; i++) {
    if (NULL != spice->spice_models[i].model_netlist) {
      /* Check if this netlist name has already existed in the list */
      to_include = 1;
      for (j = 0; j < i; j++) {
        if (NULL == spice->spice_models[j].model_netlist) {
          continue;
        }
        if (0 == strcmp(spice->spice_models[j].model_netlist, spice->spice_models[i].model_netlist)) {
          to_include = 0;
          break;
        }
      }
      /* Increamental */
      if (1 == to_include) {
        num_to_include++;
      }
    }
  }

  /* realloc */
  spice->include_netlists = (t_spice_model_netlist*)my_realloc(spice->include_netlists, 
                              sizeof(t_spice_model_netlist)*(num_to_include + spice->num_include_netlist));

  /* Fill the new included netlists */
  cur = spice->num_include_netlist;
  for (i = 0; i < spice->num_spice_model; i++) {
    if (NULL != spice->spice_models[i].model_netlist) {
      /* Check if this netlist name has already existed in the list */
      to_include = 1;
      for (j = 0; j < i; j++) {
        if (NULL == spice->spice_models[j].model_netlist) {
          continue;
        }
        if (0 == strcmp(spice->spice_models[j].model_netlist, spice->spice_models[i].model_netlist)) {
          to_include = 0;
          break;
        }
      }
      /* Increamental */
      if (1 == to_include) {
        spice->include_netlists[cur].path = my_strdup(spice->spice_models[i].model_netlist); 
        spice->include_netlists[cur].included = 0;
        vpr_printf(TIO_MESSAGE_INFO, "[%d] %s\n", cur+1, spice->include_netlists[cur].path);
        cur++;
      }
    }
  }
  /* Check */
  assert(cur == (num_to_include + spice->num_include_netlist));
  /* Update */
  spice->num_include_netlist += num_to_include;
  
  return;
}

/* Statistics reserved names in pb_types to the list*/
static 
void rec_stat_pb_type_keywords(t_pb_type* cur_pb_type,
                               int* num_keyword) {
  int imode, ipb, jpb;

  assert((0 == (*num_keyword))||(0 < (*num_keyword)));
  assert(NULL != num_keyword);
  assert(NULL != cur_pb_type);

  for (ipb = 0; ipb < cur_pb_type->num_pb; ipb++) {
    for (imode = 0; imode < cur_pb_type->num_modes; imode++) {
      /* pb_type_name[num_pb]_mode[mode_name]*/
      (*num_keyword) += 1;
      for (jpb = 0; jpb < cur_pb_type->modes[imode].num_pb_type_children; jpb++) {
        if (NULL == cur_pb_type->modes[imode].pb_type_children[jpb].spice_model) {
          rec_stat_pb_type_keywords(&(cur_pb_type->modes[imode].pb_type_children[jpb]),
                                    num_keyword);
        }
      }
    }
  }

  return;
}

/* Add reserved names in pb_types to the list*/
static 
void rec_add_pb_type_keywords_to_list(t_pb_type* cur_pb_type,
                                      int* cur,
                                      char** keywords,
                                      char* prefix) {
  int imode, ipb, jpb;
  char* formatted_prefix = format_spice_node_prefix(prefix);
  char* pass_on_prefix = NULL;

  assert(NULL != cur);
  assert((0 == (*cur))||(0 < (*cur)));
  assert(NULL != keywords);
  assert(NULL != cur_pb_type);

  /* pb_type_name[num_pb]_mode[mode_name]*/
  // num_keyword += cur_pb_type->num_pb * cur_pb_type->num_modes;
  for (ipb = 0; ipb < cur_pb_type->num_pb; ipb++) {
    for (imode = 0; imode < cur_pb_type->num_modes; imode++) {
      keywords[(*cur)] = (char*)my_malloc(sizeof(char)*
                                         (strlen(formatted_prefix) + strlen(cur_pb_type->name) + 1 + strlen(my_itoa(ipb)) + 7 
                                          + strlen(cur_pb_type->modes[imode].name) + 2));
      sprintf(keywords[(*cur)], "%s%s[%d]_mode[%s]", formatted_prefix, cur_pb_type->name, ipb, cur_pb_type->modes[imode].name);
      pass_on_prefix = my_strdup(keywords[(*cur)]);
      (*cur)++;
      for (jpb = 0; jpb < cur_pb_type->modes[imode].num_pb_type_children; jpb++) {
        if (NULL == cur_pb_type->modes[imode].pb_type_children[jpb].spice_model) {
          rec_add_pb_type_keywords_to_list(&(cur_pb_type->modes[imode].pb_type_children[jpb]),
                                             cur, keywords, pass_on_prefix);
          my_free(pass_on_prefix);
        }
      }
    }
  }

  my_free(formatted_prefix);

  return;
}

/* This function checks conflicts between
 * 1. SPICE model names and reserved sub-circuit names
 */
void check_keywords_conflict(t_arch Arch) {
  int num_keyword = 0;
  char**keywords;
  int conflict = 0;

  int num_keyword_per_grid = 0;
  int cur, iseg, imodel, i;
  int ix, iy, iz;
  t_pb_type* cur_pb_type = NULL;
  char* prefix = NULL;

  /* Generate the list of reserved names */
  num_keyword = 0;
  keywords = NULL;

  /* Reserved names: grid names */
  /* Reserved names: pb_type names */
  for (ix = 0; ix < (nx + 2); ix++) { 
    for (iy = 0; iy < (ny + 2); iy++) { 
      /* by_pass the empty */
      if (EMPTY_TYPE != grid[ix][iy].type) {
        num_keyword += 1; /* plus grid[ix][iy]*/ 
        for (iz = 0; iz < grid[ix][iy].type->capacity; iz++) {
          num_keyword_per_grid = 0;
          /* Per grid, type_descriptor.name[i]*/
          /* Go recursive pb_graph_node, until the leaf which defines a spice_model */
          cur_pb_type = grid[ix][iy].type->pb_type;
          rec_stat_pb_type_keywords(cur_pb_type, &num_keyword_per_grid);
          num_keyword += num_keyword_per_grid; 
        }
      }
    }
  }

  /* Reserved names: switch boxes, connection boxes, channels */
  /* Channels -X */
  num_keyword += (ny+1) * nx;
  /* Channels -Y */
  num_keyword += (nx+1) * ny;
  /* Switch Boxes */
  /* sb[ix][iy]*/
  num_keyword += (nx + 1)*(ny + 1); 
  /* Connection Boxes */
  /* cbx[ix][iy] */
  num_keyword += (ny+1) * nx;
  /* cby[ix][iy] */
  num_keyword += (nx+1) * ny;

  /* internal names: inv, buf, cpt, vpr_nmos, vpr_pmos, wire_segments */
  num_keyword += 5 + Arch.num_segments;

  /* Malloc */
  keywords = (char**)my_malloc(sizeof(char*)*num_keyword);
  
  /* Add reserved names to the list */
  cur = 0;
  for (i = 0; i < num_keyword; i++) {
    keywords[i] = NULL;
  }
  /* internal names: inv, buf, cpt, vpr_nmos, vpr_pmos, wire_segments */
  keywords[cur] = "inv"; cur++;
  keywords[cur] = "buf"; cur++;
  keywords[cur] = "cpt"; cur++;
  keywords[cur] = "vpr_nmos"; cur++;
  keywords[cur] = "vpr_pmos"; cur++;
  for (iseg = 0; iseg < Arch.num_segments; iseg++) {
    keywords[cur] = (char*)my_malloc(sizeof(char)*
                                    (strlen(Arch.Segments[iseg].spice_model->name) + 4 + strlen(my_itoa(iseg)) + 1));
    sprintf(keywords[cur], "%s_seg%d", Arch.Segments[iseg].spice_model->name, iseg);
    cur++;
  }
  /* Reserved names: switch boxes, connection boxes, channels */
  /* Channels -X */
  for (iy = 0; iy < (ny + 1); iy++) { 
    for (ix = 1; ix < (nx + 1); ix++) { 
      /* chanx[ix][iy]*/
      keywords[cur] = (char*)my_malloc(sizeof(char)* (6 + strlen(my_itoa(ix)) + 2 + strlen(my_itoa(iy)) + 2));
      sprintf(keywords[cur], "chanx[%d][%d]", ix, iy);
      cur++;
    }
  }
  /* Channels -Y */
  for (ix = 0; ix < (nx + 1); ix++) { 
    for (iy = 1; iy < (ny + 1); iy++) { 
      /* chany[ix][iy]*/
      keywords[cur] = (char*)my_malloc(sizeof(char)* (6 + strlen(my_itoa(ix)) + 2 + strlen(my_itoa(iy)) + 2));
      sprintf(keywords[cur], "chany[%d][%d]", ix, iy);
      cur++;
    }
  }
  /* Connection Box */
  /* cbx[ix][iy]*/
  for (iy = 0; iy < (ny + 1); iy++) { 
    for (ix = 1; ix < (nx + 1); ix++) { 
      /* cbx[ix][iy]*/
      keywords[cur] = (char*)my_malloc(sizeof(char)* (4 + strlen(my_itoa(ix)) + 2 + strlen(my_itoa(iy)) + 2));
      sprintf(keywords[cur], "cbx[%d][%d]", ix, iy);
      cur++;
    }
  }
  /* cby[ix][iy]*/
  for (ix = 0; ix < (nx + 1); ix++) { 
    for (iy = 1; iy < (ny + 1); iy++) { 
      /* cby[ix][iy]*/
      keywords[cur] = (char*)my_malloc(sizeof(char)* (4 + strlen(my_itoa(ix)) + 2 + strlen(my_itoa(iy)) + 2));
      sprintf(keywords[cur], "cby[%d][%d]", ix, iy);
      cur++;
    }
  }
  /* Switch Boxes */
  for (ix = 0; ix < (nx + 1); ix++) { 
    for (iy = 0; iy < (ny + 1); iy++) { 
      /* sb[ix][iy]*/
      keywords[cur] = (char*)my_malloc(sizeof(char)* (3 + strlen(my_itoa(ix)) + 2 + strlen(my_itoa(iy)) + 2));
      sprintf(keywords[cur], "sb[%d][%d]", ix, iy);
      cur++;
    }
  }
  /* Reserved names: grid names */
  /* Reserved names: pb_type names */
  for (ix = 0; ix < (nx + 2); ix++) { 
    for (iy = 0; iy < (ny + 2); iy++) { 
      /* by_pass the empty */
      if (EMPTY_TYPE != grid[ix][iy].type) {
        prefix = (char*)my_malloc(sizeof(char)* (5 + strlen(my_itoa(ix)) + 2 + strlen(my_itoa(iy)) + 2));
        sprintf(prefix, "grid[%d][%d]", ix, iy);
        /* plus grid[ix][iy]*/ 
        keywords[cur] = my_strdup(prefix); 
        cur++; 
        for (iz = 0; iz < grid[ix][iy].type->capacity; iz++) {
          /* Per grid, type_descriptor.name[i]*/
          /* Go recursive pb_graph_node, until the leaf which defines a spice_model */
          cur_pb_type = grid[ix][iy].type->pb_type;
          rec_add_pb_type_keywords_to_list(cur_pb_type, &cur, keywords, prefix);
        }
        my_free(prefix);
      }
    }
  }
  /* assert */
  assert(cur == num_keyword);
  
  /* Check the keywords conflicted with defined spice_model names */
  for (imodel = 0; imodel < Arch.spice->num_spice_model; imodel++) {
    for (i = 0; i < num_keyword; i++) {
      if (0 == strcmp(Arch.spice->spice_models[imodel].name, keywords[i])) {
        vpr_printf(TIO_MESSAGE_ERROR, "Keyword Conflicted! Spice Model Name: %s\n", keywords[i]);
        conflict++;
      }
    }
  } 

  assert((0 == conflict)||(0 < conflict));
  if (0 < conflict) {
    vpr_printf(TIO_MESSAGE_ERROR, "Found %d conflicted keywords!\n", conflict);
    exit(1);
  }
  
  return; 
}

static 
void build_prev_node_list_rr_nodes(int LL_num_rr_nodes,
                                   t_rr_node* LL_rr_node) {
  int inode, iedge, to_node, cur;
  int* cur_index = (int*)my_malloc(sizeof(int)*LL_num_rr_nodes);
  
  /* This function is not timing-efficient, I comment it */
  /*
  for (inode = 0; inode < LL_num_rr_nodes; inode++) {
    find_prev_rr_nodes_with_src(&(LL_rr_nodes[inode]), 
                                &(LL_rr_nodes[inode].num_drive_rr_nodes),
                                &(LL_rr_nodes[inode].drive_rr_nodes),
                                &(LL_rr_nodes[inode].drive_switches));
  }
  */
  for (inode = 0; inode < LL_num_rr_nodes; inode++) {
    /* Malloc */
    LL_rr_node[inode].num_drive_rr_nodes = LL_rr_node[inode].fan_in;
    LL_rr_node[inode].drive_rr_nodes = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*LL_rr_node[inode].num_drive_rr_nodes);
    LL_rr_node[inode].drive_switches = (int*)my_malloc(sizeof(int)*LL_rr_node[inode].num_drive_rr_nodes);
  }
  /* Initialize */
  for (inode = 0; inode < LL_num_rr_nodes; inode++) {
    cur_index[inode] = 0;
    for (iedge = 0; iedge < LL_rr_node[inode].num_drive_rr_nodes; iedge++) {
      LL_rr_node[inode].drive_rr_nodes[iedge] = NULL;
      LL_rr_node[inode].drive_switches[iedge] = -1;
    }
  }
  /* Fill */
  for (inode = 0; inode < LL_num_rr_nodes; inode++) {
    for (iedge = 0; iedge < LL_rr_node[inode].num_edges; iedge++) {
      to_node = LL_rr_node[inode].edges[iedge]; 
      cur = cur_index[to_node];
      LL_rr_node[to_node].drive_rr_nodes[cur] = &(LL_rr_node[inode]);
      LL_rr_node[to_node].drive_switches[cur] = LL_rr_node[inode].switches[iedge];
      /* Update cur_index[to_node]*/
      cur_index[to_node]++;
    }
  }
  /* Check */
  for (inode = 0; inode < LL_num_rr_nodes; inode++) {
    assert(cur_index[inode] == LL_rr_node[inode].num_drive_rr_nodes);
  }

  return;
}

static 
void back_annotate_rr_node_map_info() {
  int inode, jnode, inet, xlow, ylow;
  int next_node, iedge;
  t_trace* tptr;
  t_rr_type rr_type;

  /* 1st step: Set all the configurations to default.
   * rr_nodes select edge[0]
   */
  for (inode = 0; inode < num_rr_nodes; inode++) {
    rr_node[inode].prev_node = OPEN;
    rr_node[inode].prev_edge = 0;
  }
  for (inode = 0; inode < num_rr_nodes; inode++) {
    if (0 == rr_node[inode].num_edges) {
      continue;
    }
    assert(0 < rr_node[inode].num_edges);
    for (iedge = 0; iedge < rr_node[inode].num_edges; iedge++) {
      jnode = rr_node[inode].edges[iedge];
      if (&(rr_node[inode]) == rr_node[jnode].drive_rr_nodes[0]) {
        rr_node[jnode].prev_node = inode;
        rr_node[jnode].prev_edge = iedge;
      }
    }
  }

  /* 2nd step: With the help of trace, we back-annotate */
  for (inet = 0; inet < num_nets; inet++) {
    if (FALSE == clb_net[inet].is_global) {
      if (FALSE == clb_net[inet].num_sinks) {
        /* Net absorbed by CLB */
      } else {
        tptr = trace_head[inet];
        while (tptr != NULL) {
          inode = tptr->index;
          rr_type = rr_node[inode].type;
          xlow = rr_node[inode].xlow;
          ylow = rr_node[inode].ylow;
          switch (rr_type) {
          case SINK: 
          case IPIN: 
            /* Nothing should be done. This supposed to the end of a trace*/
            break;
          case CHANX: 
          case CHANY: 
          case OPIN: 
          case SOURCE: 
            /* SINK(IO/Pad) is the end of a routing path. Should configure its prev_edge and prev_node*/
            /* We care the next rr_node, this one is driving, which we have to configure 
             */
            assert(NULL != tptr->next);
            next_node = tptr->next->index;
            assert((!(0 > next_node))&&(next_node < num_rr_nodes));
            /* Prev_node */
            rr_node[next_node].prev_node = inode;
            /* Prev_edge */
            for (iedge = 0; iedge < rr_node[inode].num_edges; iedge++) {
              if (next_node == rr_node[inode].edges[iedge]) {
                rr_node[next_node].prev_edge = iedge;
                break;
              }
            }
            break;
          default:
            vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid traceback element type.\n");
            exit(1);
          }
          tptr = tptr->next;
        }
      }
    //} else {
      /* Global net never routed*/
    }
  }

  return;
}

static 
void fprint_run_hspice_shell_script(char* spice_dir_path,
                                    char* subckt_dir_path,
                                    char* top_netlist_file, 
                                    char* grid_testbench_file, 
                                    char* routing_mux_testbench_file, 
                                    char* mux_testbench_file) {
  FILE* fp = NULL;
  /* Format the directory path */
  char* spice_dir_formatted = format_dir_path(spice_dir_path);
  char* shell_script_path = my_strcat(spice_dir_path, run_hspice_shell_script_name);
  char* chomped_top_netlist_file = NULL;
  char* chomped_mux_testbench_file = NULL; 
  char* chomped_routing_mux_testbench_file = NULL; 
  char* chomped_grid_testbench_file = NULL; 

  /* Check if the path exists*/
  fp = fopen(shell_script_path,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create Shell Script for running HSPICE %s!",__FILE__, __LINE__, shell_script_path); 
    exit(1);
  } 

  /* Go to the subckt dir for HSPICE VerilogA sim*/
  fprintf(fp, "cd %s\n", subckt_dir_path);

  /* For VerilogA initilization */
  fprintf(fp, "source /softs/synopsys/hspice/2013.12/hspice/bin/cshrc.meta\n");
 
  /* Run hspice Top Netlist */
  if (NULL != top_netlist_file) {
    chomped_top_netlist_file = chomp_file_name_postfix(top_netlist_file);
    fprintf(fp, "hspice64 -mt 10 -i ../%s -o ../%s%s.lis -hdlpath /softs/synopsys/hspice/2013.12/hspice/include\n",
            /*spice_dir_formatted, */top_netlist_file, /*spice_dir_formatted,*/ sim_results_dir_path,
            chomped_top_netlist_file);
  }
  /* Run hspice Mux Testbench Netlist */
  if (NULL != mux_testbench_file) {
    chomped_mux_testbench_file = chomp_file_name_postfix(mux_testbench_file);
    fprintf(fp, "hspice64 -mt 10 -i ../%s -o ../%s%s.lis -hdlpath /softs/synopsys/hspice/2013.12/hspice/include\n",
            /*spice_dir_formatted,*/ mux_testbench_file, /*spice_dir_formatted,*/ sim_results_dir_path,
            chomped_mux_testbench_file);
  }
  /* Run hspice grid testbench netlist */
  if (NULL != grid_testbench_file) {
    chomped_grid_testbench_file = chomp_file_name_postfix(grid_testbench_file);
    fprintf(fp, "hspice64 -mt 10 -i ../%s -o ../%s%s.lis -hdlpath /softs/synopsys/hspice/2013.12/hspice/include\n",
            /*spice_dir_formatted,*/ grid_testbench_file, /*spice_dir_formatted,*/ sim_results_dir_path,
            chomped_grid_testbench_file);
  }
  
  /* Run hspice routing mux testbench netlist */
  if (NULL != routing_mux_testbench_file) {
    chomped_routing_mux_testbench_file = chomp_file_name_postfix(routing_mux_testbench_file);
    fprintf(fp, "hspice64 -mt 10 -i ../%s -o ../%s%s.lis -hdlpath /softs/synopsys/hspice/2013.12/hspice/include\n",
            /*spice_dir_formatted,*/ routing_mux_testbench_file, /*spice_dir_formatted,*/ sim_results_dir_path,
            chomped_routing_mux_testbench_file);
  }
  
  fprintf(fp, "cd %s\n", spice_dir_path);

  /* close fp */
  fclose(fp);

  /* Free */  
  my_free(spice_dir_formatted);
  my_free(chomped_top_netlist_file);
  my_free(chomped_mux_testbench_file);

  return;
}

/***** Main Function *****/
void vpr_print_spice_netlists(t_vpr_setup vpr_setup,
                              t_arch Arch,
                              char* circuit_name) {
  clock_t t_start;
  clock_t t_end;
  float run_time_sec;

  int num_clocks = 0;
  float vpr_crit_path_delay = 0.; 
  float vpr_clock_freq = 0.; 

  char* spice_dir_formatted = NULL;
  char* include_dir_path = NULL;
  char* subckt_dir_path = NULL;
  char* top_netlist_path = NULL;
  char* include_dir_name = vpr_setup.SpiceOpts.include_dir;
  char* subckt_dir_name = vpr_setup.SpiceOpts.subckt_dir;
  char* top_netlist_name = NULL;
  char* chomped_circuit_name = NULL;
  char* chomped_spice_dir = NULL;
  char* mux_testbench_file = NULL;
  char* grid_testbench_file = NULL;
  char* routing_mux_testbench_file = NULL;

  /* Check if the routing architecture we support*/
  if (UNI_DIRECTIONAL != vpr_setup.RoutingArch.directionality) {
    vpr_printf(TIO_MESSAGE_ERROR, "FPGA SPICE netlists only support uni-directional routing architecture!\n");
    exit(1);
  }
  
  /* We don't support mrFPGA */
#ifdef MRFPGA_H
  if (is_mrFPGA) {
    vpr_printf(TIO_MESSAGE_ERROR, "FPGA SPICE netlists do not support mrFPGA!\n");
    exit(1);
  }
#endif  

  /* Initial Arch SPICE MODELS*/
  init_check_arch_spice_models(&Arch, &vpr_setup.RoutingArch);
  init_list_include_netlists(Arch.spice); 

  /* Add keyword checking */
  check_keywords_conflict(Arch);

  /*Process the circuit name*/
  split_path_prog_name(circuit_name,'/',&chomped_spice_dir ,&chomped_circuit_name);
  
  /* Start Clocking*/
  t_start = clock();

  /* Format the directory path */
  if (NULL != vpr_setup.SpiceOpts.spice_dir) {
    spice_dir_formatted = format_dir_path(vpr_setup.SpiceOpts.spice_dir);
  } else {
    spice_dir_formatted = format_dir_path(my_strcat(format_dir_path(chomped_spice_dir),default_spice_dir_path));
  }

  /*Initial directory organization*/
  /* Process include directory */
  (include_dir_path) = my_strcat(spice_dir_formatted,include_dir_name); 
  /* Process subckt directory */
  (subckt_dir_path) = my_strcat(spice_dir_formatted,subckt_dir_name);

  /* Check the spice folders exists if not we create it.*/
  create_dir_path(spice_dir_formatted);
  create_dir_path(include_dir_path);
  create_dir_path(subckt_dir_path);
  create_dir_path(my_strcat(spice_dir_formatted, sim_results_dir_path));

  /* determine the VPR clock frequency */
  vpr_crit_path_delay = get_critical_path_delay()/1e9;
  assert(vpr_crit_path_delay > 0.);
  /* if we don't have global clock, clock_freqency should be set to 0.*/
  num_clocks = count_netlist_clocks();
  if (0 == num_clocks) {
     vpr_clock_freq = 0.;
  } else { 
    assert(1 == num_clocks);
    vpr_clock_freq = 1. / vpr_crit_path_delay; 
  }

  /* Build previous node lists for each rr_node */
  vpr_printf(TIO_MESSAGE_INFO, "Building previous node list for all Routing Resource Nodes...\n");
  build_prev_node_list_rr_nodes(num_rr_nodes, rr_node);
  vpr_printf(TIO_MESSAGE_INFO,"Back annotating mapping information to routing resource nodes...\n");
  back_annotate_rr_node_map_info();

  /* Backannotate activity information, initialize the waveform information */
  vpr_printf(TIO_MESSAGE_INFO, "Backannoating Net Activities...\n");
  backannotate_rr_nodes_net_info();
  backannotate_clb_nets_act_info();
  /* Give spice_name_tag for each pb*/
  gen_spice_name_tags_all_pbs();
  /* Update local_rr_graphs to match post-route results*/
  vpr_printf(TIO_MESSAGE_INFO, "Update CLB local routing graph to match post-route results...\n");
  update_grid_pbs_post_route_rr_graph();
  
  /* Generate Header files */
  fprint_spice_headers(include_dir_path, vpr_crit_path_delay, num_clocks, *(Arch.spice));

  /* Generate sub circuits: Inverter, Buffer, Transmission Gate, LUT, DFF, SRAM, MUX*/
  generate_spice_subckts(subckt_dir_path, Arch ,&vpr_setup.RoutingArch);

  /* Print Netlists of the given FPGA*/
  if (vpr_setup.SpiceOpts.print_spice_top_testbench) {
    top_netlist_name = my_strcat(chomped_circuit_name, top_netlist_postfix);
    /* Process top_netlist_path */
    (top_netlist_path) = my_strcat(spice_dir_formatted,top_netlist_name); 
    fprint_spice_top_netlist(circuit_name, top_netlist_path, include_dir_path, subckt_dir_path, 
                             rr_node_indices, num_clocks, *(Arch.spice), vpr_setup.SpiceOpts.fpga_spice_leakage_only);
  }

  /* Print Grid testbench if needed */
  if (vpr_setup.SpiceOpts.print_spice_grid_testbench) {
    grid_testbench_file = my_strcat(chomped_circuit_name, spice_grid_testbench_postfix); 
    fprint_spice_grid_testbench(spice_dir_formatted, circuit_name, grid_testbench_file, include_dir_path, subckt_dir_path,
                                rr_node_indices, num_clocks, Arch, vpr_setup.SpiceOpts.fpga_spice_leakage_only);
  }

  /* Print MUX testbench if needed */
  if (vpr_setup.SpiceOpts.print_spice_mux_testbench) {
    mux_testbench_file = my_strcat(chomped_circuit_name, spice_mux_testbench_postfix); 
    fprint_spice_mux_testbench(spice_dir_formatted, circuit_name, mux_testbench_file, include_dir_path, subckt_dir_path,
                               rr_node_indices, num_clocks, Arch, vpr_setup.SpiceOpts.fpga_spice_leakage_only);
  }

  if (vpr_setup.SpiceOpts.print_spice_routing_mux_testbench) {
    routing_mux_testbench_file = my_strcat(chomped_circuit_name, spice_routing_mux_testbench_postfix); 
    fprint_spice_routing_mux_testbench(spice_dir_formatted, circuit_name, routing_mux_testbench_file, include_dir_path, subckt_dir_path,
                                       rr_node_indices, num_clocks, Arch, vpr_setup.SpiceOpts.fpga_spice_leakage_only);
  }

  /* Generate a shell script for running HSPICE simulations */
  fprint_run_hspice_shell_script(spice_dir_formatted, subckt_dir_path, 
                                 top_netlist_name, grid_testbench_file, routing_mux_testbench_file,
                                 mux_testbench_file);

  /* END Clocking*/
  t_end = clock();

  run_time_sec = (float)(t_end - t_start) / CLOCKS_PER_SEC;
  vpr_printf(TIO_MESSAGE_INFO, "SPICE netlists generation took %g seconds\n", run_time_sec);  

  /* Free spice_net_info */
  free_clb_nets_spice_net_info();
  /* Free */
  my_free(spice_dir_formatted);
  my_free(include_dir_path);
  my_free(subckt_dir_path);

  return;
}
