#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "blif_parser.h"
#include "blearch_parser.h"
#include "mpack_types.h"
#include "mpack_conf.h"
#include "mpack_util.h"


/*****Included Subroutines*****/
void free_blif_arch(t_blif_arch* blif_arch);

void free_ble_arch(t_ble_arch* ble_arch);


/*Initial logic node network*/
void init_lgkntwk(t_lgkntwk* lgkntwk)
{
  lgkntwk->pi_head = NULL;
  lgkntwk->po_head = NULL;
  lgkntwk->libgate_head = NULL;
  lgkntwk->latch_head = NULL;

  lgkntwk->libgate_num = 0;
  lgkntwk->pi_num = 0;
  lgkntwk->po_num = 0;
  lgkntwk->latch_num = 0;

  lgkntwk->pi_ptrs = NULL;
  lgkntwk->po_ptrs = NULL;
  lgkntwk->libgate_ptrs = NULL;
  lgkntwk->latch_ptrs = NULL;

  lgkntwk->critical_delay = 0.0;
  lgkntwk->max_slack = 0.0;
}

/* Initialize stats*/
void init_stats(t_stats* stats)
{
  int i;  
  /*
  stats->libgate_stats.total = 0;
  stats->libgate_stats.and_num = 0;
  stats->libgate_stats.xnor_num = 0;
  stats->libgate_stats.add_num = 0;
  stats->libgate_stats.carry_num = 0;
  stats->libgate_stats.inv_num = 0;
  stats->libgate_stats.buf_num = 0;
  */
  stats->libgate_num = 0;
  
  for(i=0; i<13; i++)
  {stats->stdgate_stats[i] = 0;}

  stats->max_lut_size = 0;
  stats->lut_stats = NULL;
  
  stats->pi_num = 0;
  stats->po_num = 0;
  stats->latch_num = 0;
  stats->ble_num = 0;
  stats->ble_fill_rate = 0.0;

  stats->libgate_latch_fanout_avg = 0.0;
  stats->avg_lt10 = 0.0;
}

void init_edge(t_edge* edge) {
  edge->delay = 0.0;
  edge->t_arrival = 0.0;
  edge->t_required = 0.0;
  edge->weight = 0.0;
  edge->timing_weight = 0.0;
  edge->depth_weight = 0.0;
  edge->slack = 0.0;
}

/* Initialize logic node*/
void init_lgknd(t_lgknd* lgknd)
{
  lgknd->idx = -1;
  lgknd->type = -1; // Imply undefined
  lgknd->input_num = 0;
  lgknd->inputs = NULL;
  lgknd->output_num = 0;
  lgknd->outputs = NULL;
  
  lgknd->in_edges = NULL; 
  lgknd->out_edges = NULL; 
  //lgknd->input_conkt = NULL;
  //lgknd->output_conkt = NULL;

  lgknd->init_val = -1;

  lgknd->delay = 0.0;
  lgknd->t_arrival = 0.0;
  lgknd->slack = 0.0;
  lgknd->t_required = 0.0;

  lgknd->mapped = 0;
  lgknd->depth = 0;
  lgknd->dfsed = 0;
  lgknd->map_climb = 0;
  lgknd->try_sat = 0;
  lgknd->try_pattern_types = NULL;
  lgknd->redund_output_num = 0;  
  lgknd->tmp_redund_output_num = 0;  
  lgknd->fanout_buf_num = 0;
  lgknd->cur_edge = 0;
  lgknd->comb_num = 0;
  lgknd->logic_equivalent = 1;
  lgknd->strict = 1;
  lgknd->input_paths_affect = 0.0;
  lgknd->output_paths_affect = 0.0;
  lgknd->depth_source = 0.0;

  lgknd->ble_idx = -1;
  lgknd->ble_cell_idx = -1;
  lgknd->ble_info = NULL;
 
  lgknd->output_conkt = -1;
  lgknd->pio_conkt = -1;
  lgknd->latch_conkt = -1;
  lgknd->net_name = NULL;
  lgknd->io_name = NULL;

  //lgknd->next = NULL;
}

/**
 * Create fixed length of Linked list
 * Node struct must have a pointor named after "next"!
 * The head of this linked list is a scout. Formal start is the second element. 
 */
t_lgknd* create_lgknd_list(int len,
                           size_t size_node)
{
  t_lgknd* head;
  t_lgknd* tmp_head;
  int ind;

  /* Create a scout, this is a blank node*/
  head = (t_lgknd*)my_malloc(size_node);
  head->next = NULL;
  
  for (ind=0; ind<len; ind++)
  {
    /* Create a node ahead of the current*/
    tmp_head = (t_lgknd*)my_malloc(size_node); 
    tmp_head->next = head;
    head = tmp_head;
  }
  
  return head; 
}

/**
 * Insert a node inside the linked list
 * Cur is pointer which a new node will be insertedafter.
 */
t_lgknd* insert_lgknd_list_node(t_lgknd* cur,
                                size_t size_node)
{
  t_lgknd* cur_next; 
  /* Store the current next*/  
  cur_next = cur->next;
  /* Allocate new node*/
  cur->next = (t_lgknd*)my_malloc(size_node);
  init_lgknd(cur->next);
  /* Configure the new node*/
  cur->next->next = cur_next;

  return cur_next;
}

/**
 * Romove a node from linked list
 * cur is the node whose next node is to be removed
 */
void remove_lgknd_list_node(t_lgknd* cur)
{ 
  t_lgknd* rm_node = cur->next;
  /* Connect the next next node*/ 
  cur->next = cur->next->next;
  /* free the node*/
  free_lgknd(rm_node);
  free(rm_node);
}

/**
 * Cat the linked list
 * head2 is connected to the tail of head1
 * return head1
 */
t_lgknd* cat_lgknd_lists(t_lgknd* head1,
                         t_lgknd* head2)
{
  t_lgknd* tmp = head1;

  /* Reach the tail of head1*/ 
  while(tmp->next != NULL)
  {
    tmp = tmp->next;
  }
  /* Cat*/
  tmp->next = head2->next;
  /* Free head2*/
  free(head2);
  
  return head1;
}

/*Struct ble_info utilizations*/

/**
 *  Initial the BLE info from BLE_arch
 */
int init_ble_info(t_ble_arch* ble_arch,
                  t_ble_info* ble_info)
{
  int im;
  int j;

  ble_info->idx = -1;
  /*Inter-BLE connection initialization*/
  /*Inputs*/
  ble_info->input_num = ble_arch->input_num;
  ble_info->input_conkts = (int*)my_malloc(sizeof(int)*ble_arch->input_num);
  ble_info->input_conkts_type = (int*)my_malloc(sizeof(int)*ble_arch->input_num);
  ble_info->input_used = (int*)my_malloc(sizeof(int)*ble_arch->input_num);
  ble_info->input_lgknds = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*ble_arch->input_num);
  ble_info->input_lgknds_input_idx = (int*)my_malloc(sizeof(int)*ble_arch->input_num);
  
  for (im=0; im<ble_info->input_num; im++)
  {
    ble_info->input_conkts[im] = -1;
    ble_info->input_conkts_type[im] = -1;
    ble_info->input_used[im] = 0;
    ble_info->input_lgknds[im] = NULL;
    ble_info->input_lgknds_input_idx[im] = -1;
  }

  /*Outputs*/
  ble_info->output_num = ble_arch->output_num;
  ble_info->output_conkts = (int*)my_malloc(sizeof(int)*ble_arch->output_num);
  ble_info->output_conkts_type = (int*)my_malloc(sizeof(int)*ble_arch->output_num);
  ble_info->output_used = (int*)my_malloc(sizeof(int)*ble_arch->output_num);
  ble_info->output_muxes = (int*)my_malloc(sizeof(int)*ble_arch->output_num);
  ble_info->output_types = (int*)my_malloc(sizeof(int)*ble_arch->output_num);
  ble_info->output_lgknds = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*ble_arch->output_num);
  
  for (im=0; im<ble_info->output_num; im++)
  {
    ble_info->output_conkts[im] = -1;
    ble_info->output_conkts_type[im] = -1;
    ble_info->output_used[im] = 0;
    ble_info->output_muxes[im] = -1;
    ble_info->output_types[im] = -1;
    ble_info->output_lgknds[im] = NULL;
  }

  /*Inside-BLE connections Initialization*/
  ble_info->blend_num = ble_arch->blend_num;
  ble_info->blend_muxes = (int**)my_malloc(sizeof(int*)*ble_arch->blend_num);
  ble_info->blend_port_used = (int**)my_malloc(sizeof(int*)*ble_arch->blend_num);
  ble_info->blend_used = (int*)my_malloc(sizeof(int)*ble_arch->blend_num);
  ble_info->blend_lgknds = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*ble_arch->blend_num);
  ble_info->blend_lgknd_input_idxes = (int**)my_malloc(sizeof(int*)*ble_arch->blend_num);
  

  /*Initial each input mux*/
  for (im=0; im<ble_info->blend_num; im++) {
    ble_info->blend_muxes[im] = (int*)my_malloc(sizeof(int)*ble_arch->blends[im].input_num); 
    ble_info->blend_port_used[im] = (int*)my_malloc(sizeof(int)*ble_arch->blends[im].input_num); 
    ble_info->blend_lgknds[im] = NULL;
    ble_info->blend_lgknd_input_idxes[im] = (int*)my_malloc(sizeof(int)*ble_arch->blends[im].input_num); 
    for (j=0; j<ble_arch->blends[im].input_num; j++) { 
	  ble_info->blend_muxes[im][j] = -1;
      ble_info->blend_port_used[im][j] = 0;
      ble_info->blend_lgknd_input_idxes[im][j] = -1;
    }
    ble_info->blend_used[im] = 0;
  }

  return 1;
}

/**
 *  Initial the BLE info from BLE_arch
 */
int blank_ble_info(t_ble_arch* ble_arch,
                   t_ble_info* ble_info)
{
  int im;
  int j;

  /*Inter-BLE connection initialization*/
  /*Inputs*/
  //ble_info->input_num = -1;
  for (im=0; im<ble_info->input_num; im++)
  {
    ble_info->input_conkts[im] = -1;
    ble_info->input_conkts_type[im] = -1;
    ble_info->input_used[im] = 0;
    ble_info->input_lgknds[im] = NULL;
    ble_info->input_lgknds_input_idx[im] = -1;
  }

  /*Outputs*/
  //ble_info->output_num = -1;
  for (im=0; im<ble_info->output_num; im++)
  {
    ble_info->output_conkts[im] = -1;
    ble_info->output_conkts_type[im] = -1;
    ble_info->output_used[im] = 0;
    ble_info->output_muxes[im] = -1;
    ble_info->output_types[im] = -1;
    ble_info->output_lgknds[im] = NULL;
  }

  /*Inside-BLE connections Initialization*/
  /*Initial each input mux*/
  //ble_info->blend_num = -1;
  for (im=0; im<ble_info->blend_num; im++) {
    ble_info->blend_lgknds[im] = NULL;
    for (j=0; j<ble_arch->blends[im].input_num; j++) { 
	  ble_info->blend_muxes[im][j] = -1;
      ble_info->blend_port_used[im][j] = 0;
      ble_info->blend_lgknd_input_idxes[im][j] = -1;
    }
    ble_info->blend_used[im] = 0;
  }

  return 1;
}

/*Create a mirror of BLE info*/
int copy_ble_info(t_ble_arch* ble_arch,
                  t_ble_info* src,
                  t_ble_info* des)
{
  int im;
  int ip;

  /*Initial the destination*/
  blank_ble_info(ble_arch,des);
  
  des->idx = src->idx;
  /*Copy information*/
  /*BLE inputs*/
  des->input_num = src->input_num;
  for (im=0; im<src->input_num; im++) {
    des->input_conkts[im] = src->input_conkts[im];
    des->input_conkts_type[im] = src->input_conkts_type[im];
    des->input_used[im] = src->input_used[im];
    des->input_lgknds[im] = src->input_lgknds[im];
    des->input_lgknds_input_idx[im] = src->input_lgknds_input_idx[im];
  }
  /*Inside-BLE connections*/
  des->blend_num = src->blend_num;
  for (im=0; im<src->blend_num; im++) {
    des->blend_used[im] = src->blend_used[im];
    des->blend_lgknds[im] = src->blend_lgknds[im];
    for (ip=0; ip<ble_arch->blends[im].input_num; ip++) {
      des->blend_port_used[im][ip] = src->blend_port_used[im][ip];
      des->blend_muxes[im][ip] = src->blend_muxes[im][ip];
      des->blend_lgknd_input_idxes[im][ip] = src->blend_lgknd_input_idxes[im][ip];
    }
  }
       
  des->output_num = src->output_num;
  /*BLE outputs*/
  for (im=0; im<src->output_num; im++) {
    des->output_conkts[im] = src->output_conkts[im];
    des->output_conkts_type[im] = src->output_conkts_type[im];
    des->output_used[im] = src->output_used[im];
    des->output_muxes[im] = src->output_muxes[im];
    des->output_types[im] = src->output_types[im];
    des->output_lgknds[im] = src->output_lgknds[im];
  }
  /*Linked-list information*/
  des->next = src->next;

  return 1;
}

/**
 * Create fixed length of Linked list
 * Node struct must have a pointor named after "next"!
 * The head of this linked list is a scout. Formal start is the second element. 
 */
t_ble_info* create_ble_list(int len,
                            size_t size_node)
{
  t_ble_info* head;
  t_ble_info* tmp_head;
  int ind;

  /* Create a scout, this is a blank node*/
  head = (t_ble_info*)my_malloc(size_node);
  head->next = NULL;
  
  for (ind=0; ind<len; ind++)
  {
    /* Create a node ahead of the current*/
    tmp_head = (t_ble_info*)my_malloc(size_node); 
    tmp_head->next = head;
    head = tmp_head;
  }
  
  return head; 
}

/**
 * Insert a node inside the linked list
 * Cur is pointer which a new node will be insertedafter.
 */
t_ble_info* insert_ble_list_node(t_ble_info* cur,
                                 size_t size_node)
{
  t_ble_info* cur_next; 
  /* Store the current next*/  
  cur_next = cur->next;
  /* Allocate new node*/
  cur->next = (t_ble_info*)my_malloc(size_node);
  /* Configure the new node*/
  cur->next->next = cur_next;
  return cur_next;
}

/**
 * Romove a node from linked list
 * cur is the node whose next node is to be removed
 */
void remove_ble_list_node(t_ble_info* cur)
{ 
  t_ble_info* rm_node = cur->next;
  /* Connect the next next node*/ 
  cur->next = cur->next->next;
  /* free the node*/
  free(rm_node);
}

/**
 * Cat the linked list
 * head2 is connected to the tail of head1
 * return head1
 */
t_ble_info* cat_ble_lists(t_ble_info* head1,
                          t_ble_info* head2)
{
  t_ble_info* tmp = head1;

  /* Reach the tail of head1*/ 
  while(tmp->next != NULL)
  {
    tmp = tmp->next;
  }
  /* Cat*/
  tmp->next = head2->next;
  /* Free head2*/
  free(head2);
  
  return head1;
}

void free_edge(t_edge* edge) {
  return;
}

/**
 * Free lgknd
 */
void free_lgknd(t_lgknd* lgknd)
{
  int i;
 
  /*Free inputs*/ 
  free(lgknd->inputs);

  /*Free outputs*/ 
  free(lgknd->outputs);

  /*Free edges*/
  for (i=0; i<lgknd->input_num; i++) {
    free_edge(&lgknd->in_edges[i]);
  }
  free(lgknd->in_edges);
  free(lgknd->out_edges); 

  if (NULL != lgknd->try_pattern_types) {
    free(lgknd->try_pattern_types);
  }
  
}

/**
 * Free all the resources we have used
 */

void free_lgkntwk(t_lgkntwk* lgkntwk)
{
  int i;

  /*Free logic network*/
  /*Free PI*/
  for (i=0; i<lgkntwk->pi_num; i++) {
    free(lgkntwk->pi_ptrs[i]->outputs);
    free(lgkntwk->pi_ptrs[i]);
  }   
  free(lgkntwk->pi_head);
  /*Free PO*/
  for (i=0; i<lgkntwk->po_num; i++) {
    free(lgkntwk->po_ptrs[i]->inputs);
    free(lgkntwk->po_ptrs[i]);
  }   
  free(lgkntwk->po_head);
  /*Free libgates*/
  for (i=0; i<lgkntwk->libgate_num; i++) {
    free_lgknd(lgkntwk->libgate_ptrs[i]);
    free(lgkntwk->libgate_ptrs[i]);
  }   
  free(lgkntwk->libgate_head);
  /*Free latches*/
  for (i=0; i<lgkntwk->latch_num; i++) {
    free_lgknd(lgkntwk->latch_ptrs[i]);
    free(lgkntwk->latch_ptrs[i]);
  }   
  free(lgkntwk->latch_head);
}

/**
 * Free a single BLE info
 */
void free_ble_info(t_ble_info* ble_info)
{
  int i;
  /*Free BLE inputs*/
  free(ble_info->input_conkts);
  free(ble_info->input_conkts_type);
  free(ble_info->input_used);
  free(ble_info->input_lgknds);
  free(ble_info->input_lgknds_input_idx);
  /*Free BLE outputs*/
  free(ble_info->output_conkts);
  free(ble_info->output_conkts_type);
  free(ble_info->output_used);
  free(ble_info->output_muxes);
  free(ble_info->output_types);
  free(ble_info->output_lgknds);
  /*Free BLE Cells*/
  for (i=0; i<ble_info->blend_num; i++) {
    free(ble_info->blend_port_used[i]);
    free(ble_info->blend_muxes[i]);
    free(ble_info->blend_lgknd_input_idxes[i]);
  }
  free(ble_info->blend_lgknds);
  free(ble_info->blend_used);
  
}

/**
 * Free ble info list
 */
void free_ble_info_list(t_ble_info* head)
{
  if (NULL != head->next) {
    free_ble_info(head);
    free_ble_info_list(head->next);
  }
  free(head);
}

/**
 * Remove a BLE info from Linked list
 */
void remove_ble_info_from_llist(t_ble_info* head,
                                t_ble_info* target)
{
  t_ble_info* tmp = head;

  while(tmp->next) {
    if (tmp->next == target) {
      /*Remove*/
      remove_ble_list_node(tmp);
      break;
    }
    tmp = tmp->next;
  }
      
}

/*Initial integer array*/
void my_init_int_ary(int ary_len, int* ary, int init_val)
{
  int i;
  
  if (NULL == ary) {
    printf("Error: intend to initial a NULL array pointer!\n");
    exit(1);
  }
  
  for (i=0; i<ary_len; i++) {
    ary[i] = init_val;
  }
}

/*Find the subject index in a lgknd array*/
int spot_index_lgknd_array(int lgknd_num,
                           t_lgknd** lgknds,
                           t_lgknd* subject)
{
  int ret = -1;
  int im;

  for (im=0; im<lgknd_num; im++) {
    if (lgknds[im] == subject) {
      return im;
    }
  }
  
  return ret;
}

/* Check if a logic node's outputs can be
 * absorbed in given BLE.
 * Return 1 if absorbed
 * Return 0 if not absorbed
 */
int check_ble_absorb_lgknd_output(t_ble_info* ble_info,
                                  t_lgknd* lgknd,
                                  int verbose)
{
  //int absorbed = 1;
  int i,j;
  int* absorbed = (int*)my_malloc(sizeof(int)*lgknd->output_num);

  /* Old version
  for (im = 0; im<lgknd->output_num; im++) {
    if (ble_info != lgknd->outputs[im]->ble_info) {
      absorbed = 0;
      break;
    }
  }
  */
  /* Updated version: check all blend_lgknds,output_lgknds inside BLE,
   * which makes this function more useful in mapping stage
   */
  my_init_int_ary(lgknd->output_num,absorbed,0);

  for (i=0; i<lgknd->output_num; i++) {
    /*Check blend_lgknds*/
    for (j=0; j<ble_info->blend_num; j++) {
      if (lgknd->outputs[i] == ble_info->blend_lgknds[j]) {
        absorbed[i] = 1;
      } 
    }
    /*Check output_lgknds*/
    for (j=0; j<ble_info->output_num; j++) {
      if (lgknd->outputs[i] == ble_info->output_lgknds[j]) {
        absorbed[i] = 1;
      } 
    }
  }

  /*Check all absorbed*/
  for (i=0; i<lgknd->output_num; i++) {
    if (0 == absorbed[i]) {
      return 0;
    }
  }

  return 1;
  //return absorbed;
} 

/**
 *  Check the BLE info is blank or not.
 *  Return 1 if blank
 */
int check_blank_ble_info(t_ble_arch* ble_arch,
                         t_ble_info* ble_info)
{
  int im;
  int j;

  /*Inter-BLE connection initialization*/
  /*Inputs*/
  for (im=0; im<ble_info->input_num; im++) {
    if (-1 != ble_info->input_conkts[im]) {
      return 0;
    }
    if (-1 != ble_info->input_conkts_type[im]) {
      return 0;
    }
    if (0 != ble_info->input_used[im]) {
      return 0;
    }
    if (NULL != ble_info->input_lgknds[im]) {
      return 0;
    }
    if (-1 != ble_info->input_lgknds_input_idx[im]) {
      return 0;
    }
  }

  /*Outputs*/
  for (im=0; im<ble_info->output_num; im++)
  {
    if (-1 != ble_info->output_conkts[im]) {
      return 0;
    }
    if (-1 != ble_info->output_conkts_type[im]) {
      return 0;
    }
    if (0 != ble_info->output_used[im]) {
      return 0;
    }
    if (-1 != ble_info->output_muxes[im]) {
      return 0;
    }
    if (-1 != ble_info->output_types[im]) {
      return 0;
    }
    if (NULL != ble_info->output_lgknds[im]) {
      return 0;
    }
  }

  /*Inside-BLE connections Initialization*/
  /*Initial each input mux*/
  for (im=0; im<ble_info->blend_num; im++) {
    if (NULL != ble_info->blend_lgknds[im]) {
      return 0;
    }
    for (j=0; j<ble_arch->blends[im].input_num; j++) { 
	  if (-1 != ble_info->blend_muxes[im][j]) {
        return 0;
      }
      if (0 != ble_info->blend_port_used[im][j]) {
        return 0;
      }
      if (-1 != ble_info->blend_lgknd_input_idxes[im][j]) {
        return 0;
      }
    }
    if (0 != ble_info->blend_used[im]) {
      return 0;
    }
  }

  return 1;
}

/*Check the number of connections to a certain ble port*/
int check_bleport_conkt(t_ble_arch* ble_arch,
                        t_ble_info* ble_info,
                        t_bleport* port,
                        int verbose)
{
  int ret = 0;
  int i,j,mux;  

  for (i = 0; i<ble_arch->blend_num; i++) {
    if (1 == ble_info->blend_used[i]) {
      for (j = 0; j<ble_arch->blends[i].input_num; j++) {
        mux = ble_info->blend_muxes[i][j];
        if (port == ble_arch->blends[i].inputs[j].cands[mux]) {
          ret++;
        }
      }
    }
  }

  return ret;
}

// Initial delay_info
void init_delay_info(t_delay_info* delay_info,
                     t_mpack_conf mpack_opts,
                     t_ble_arch ble_arch) {
  delay_info->inpad_delay = 4.243e-11;
  delay_info->outpad_delay = 1.394e-11;
  delay_info->gate_delay = ble_arch.cell_delay;
  delay_info->ff_tsu = 6.6e-11;
  delay_info->ff_delay = 1.24e-10;
  delay_info->ff_mux_delay = 2.5e-11;
  delay_info->inner_cluster_delay = ble_arch.inner_cluster_delay;
  delay_info->inter_cluster_delay_per_unit = mpack_opts.inter_cluster_delay;
  delay_info->inter_cluster_delay = mpack_opts.inter_cluster_delay;
}
