#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "mpack_types.h"
#include "mpack_conf.h"
#include "blearch_parser.h"
#include "mpack_util.h"
#include "global.h"
#include "check_ble_arch.h"
#include "stats.h"
#include "lgkntwk_common.h"

/**
 * Remove 0-output Primary Inputs from Logic Network
 */
int remove_isolated_pi_lgkntwk(t_lgkntwk* lgkntwk,
                                t_stats* stats,
                                int verbose)
{
  t_lgknd* rm_node = lgkntwk->pi_head;
  t_lgknd* head = lgkntwk->pi_head->next;
  int new_pi_num = lgkntwk->pi_num;
  int im;

  /*Delete nodes from linked list*/
  while(head) {
    /*Check if this is a 0-output PI*/
    if (0 == head->output_num) {
      /*Remove it*/
      remove_lgknd_list_node(rm_node);
      new_pi_num--;
      head = rm_node->next;
    }
    else {
      rm_node = head;
      head = head->next;
    }
  }
  /*Echo*/
  if (0 != (lgkntwk->pi_num - new_pi_num)) {
    printf("Info: Synthesis away %d isolated PI\n",lgkntwk->pi_num-new_pi_num);
    /*Rebuild PI Array*/
    lgkntwk->pi_ptrs = (t_lgknd**)my_realloc(lgkntwk->pi_ptrs,new_pi_num*sizeof(t_lgknd*));
    lgkntwk->pi_num = new_pi_num;
    head = lgkntwk->pi_head->next;
    im = 0;
    while(head) { 
      lgkntwk->pi_ptrs[im] = head;
      im++; 
      head = head->next; 
    }
  }

  return 1;
}

/**
 * Remove 0-input Primary Outputs from Logic Network
 */
int remove_isolated_po_lgkntwk(t_lgkntwk* lgkntwk,
                               t_stats* stats,
                               int verbose)
{
  t_lgknd* rm_node = lgkntwk->po_head;
  t_lgknd* head = lgkntwk->po_head->next;
  int new_po_num = lgkntwk->po_num;
  int im;

  /*Delete nodes from linked list*/
  while(head) {
    /*Check if PO has inputs...This is stupid but some netlist do have some...
     *Check if this is a 0-input PO
     */
    if (0 == head->input_num) {
      /*Remove it*/
      remove_lgknd_list_node(rm_node);
      new_po_num--;
      head = rm_node->next;
    }
    else {
      rm_node = head;
      head = head->next;
    }
  }
  /*Echo*/
  if (0 != (lgkntwk->po_num - new_po_num)) {
    printf("Info: Synthesis away %d isolated PO\n",lgkntwk->po_num-new_po_num);
    /*Rebuild PI Array*/
    lgkntwk->po_ptrs = (t_lgknd**)my_realloc(lgkntwk->po_ptrs,new_po_num*sizeof(t_lgknd*));
    lgkntwk->po_num = new_po_num;
    head = lgkntwk->po_head->next;
    im = 0;
    while(head) { 
      lgkntwk->po_ptrs[im] = head;
      im++; 
      head = head->next; 
    }
  }

  return 1;
}

/** Remove all the hanging latches in logic network
 *  Hanging latches are those have 0-fanout
 */
int remove_hanging_latch_lgkntwk(t_lgkntwk* lgkntwk,
                                 t_stats* stats,
                                 int verbose) {
  t_lgknd* rm_node = lgkntwk->latch_head;
  t_lgknd* head = lgkntwk->latch_head->next;
  int new_latch_num = lgkntwk->latch_num;
  int im,cnt;
  t_lgknd* swap;

  /*Delete nodes from linked list*/
  while(head) {
    // Check 0-fanout
    if (0 == head->output_num) {
      // Modify its predecessor
      assert(head->input_num == 1);
      cnt = 0;
      for (im=0; im<head->inputs[0]->output_num; im++) {
        if (head->inputs[0]->outputs[im] == head) {
          swap = head->inputs[0]->outputs[im];
          head->inputs[0]->outputs[im] = head->inputs[0]->outputs[head->inputs[0]->output_num-1];
          head->inputs[0]->outputs[head->inputs[0]->output_num-1] = swap;
          head->inputs[0]->output_num--; 
          cnt++;
        }
      } 
      if (cnt != 1) {
        printf("Warning: (remove_hanging_latch_lgkntwk)Duplicated outputs for logic node(idx:%d)!\n",head->inputs[0]->idx);
      } 
      head->inputs[0]->outputs = (t_lgknd**)my_realloc(head->inputs[0]->outputs,head->inputs[0]->output_num*sizeof(t_lgknd*));
      /*Remove it*/
      remove_lgknd_list_node(rm_node);
      new_latch_num--;
      head = rm_node->next;
    }
    else {
      rm_node = head;
      head = head->next;
    }
  }
  /*Echo*/
  if (0 != (lgkntwk->latch_num - new_latch_num)) {
    printf("Info: Synthesis away %d Hanging Latches\n",lgkntwk->latch_num-new_latch_num);
    /*Rebuild PI Array*/
    if (0 != new_latch_num) {
      lgkntwk->latch_ptrs = (t_lgknd**)my_realloc(lgkntwk->latch_ptrs,new_latch_num*sizeof(t_lgknd*));
    }
    lgkntwk->latch_num = new_latch_num;
    head = lgkntwk->latch_head->next;
    im = 0;
    while(head) { 
      lgkntwk->latch_ptrs[im] = head;
      im++; 
      head = head->next; 
    }
  }
  return 1;
}

/**
 * Remove all buffers in the logic network.
 * Buffers are useless in FPGA re-synthesis, 
 * since there are fixed buffers in FPGAs.
 * Step 1:
 * Remove the buffer, outputs to its predecessor.
 * Special applied to Primary Outputs:
 * Romove buffer, but store the net name in po_ptr->io_name
 * Step 2:
 * Regenerate the libgate_ptrs
 */
int remove_buf_lgkntwk(t_lgkntwk* lgkntwk,
                       t_stats* stats,
                       int verbose)
{
  t_lgknd* tmp = lgkntwk->libgate_head->next;
  t_lgknd* pre = NULL;
  t_lgknd* rm_node = lgkntwk->libgate_head;
  int preout_no = 0;
  int buf_cnt = 0;
  int im = 0;
  int igate = 0;
  int new_len = 0;
  int ip = 0;
  //int skip = 0;
 
  /*Determine the length of list without buffer*/
  for (im=0; im<13; im++) {
    if (ND_BUF != im) {
      new_len += stats->stdgate_stats[im];
    }
  }
  //printf("New_len: %d\n",new_len);

  if (new_len != (lgkntwk->libgate_num - stats->stdgate_stats[ND_BUF]))
  {
    printf("Error: Libgate number != BUF + Rest gates!\n");
    exit(1);
  }

  /*Free libgate_ptrs*/
  free(lgkntwk->libgate_ptrs);
  lgkntwk->libgate_ptrs = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*new_len); 
  lgkntwk->libgate_num = new_len;

  /*Re-build libgate_ptrs*/
  while(tmp) {
    /*Identify BUFFER*/
    if (ND_BUF == tmp->type) {
      /*Check input number*/
      if (1 != tmp->input_num) {
        printf("Error: Buffer has multiple inputs!\n");
        exit(1);
      } 
      /*Check output number*/
      if (0 == tmp->output_num) {
        printf("Warning: Buffer has 0 outputs!Just remove it...\n");
      }
      /**
       * Two situation that BUFFER should not be removed:
       * 1. Buffer between a Primary Input and Latch
      skip = 0;
      for (im=0; im < tmp->output_num; im++) {
        if ((ND_LATCH == tmp->outputs[im]->type)&&(ND_PI == tmp->inputs[0]->type)) {
          skip = 1;
          break;
        }
      } 
      if (1 == skip) {
        rm_node = tmp;
        tmp = tmp->next;
        continue;
      }     
      */
 
      /*Get predecessor*/
      pre = tmp->inputs[0];
      /*Copy outputs*/
      preout_no = pre->output_num;
      pre->output_num += tmp->output_num - 1;
      /*Swap connections of buffer from pre*/
      for (im=0; im<preout_no; im++) {
        if (tmp == pre->outputs[im]) {
          pre->outputs[im] = pre->outputs[preout_no-1];
          break;
        }
      } 
      pre->outputs = (t_lgknd**)my_realloc(pre->outputs,pre->output_num*sizeof(t_lgknd*));
      /*Copy the exist and delete buffer connections*/
      for (im=preout_no-1; im<pre->output_num; im++) {
        pre->outputs[im] = tmp->outputs[im-preout_no+1]; 
      } 
      /*Configure buffer outputs' inputs*/
      for (im=0; im<tmp->output_num; im++) {
        for (ip=0; ip<tmp->outputs[im]->input_num; ip++) {
          if (tmp == tmp->outputs[im]->inputs[ip]) {
            tmp->outputs[im]->inputs[ip] = pre;
          }
        }
      }

      //printf("Node %s 's output changes from %d to %d.\n",typepair_list[pre->type].type_name,preout_no,pre->output_num);
      /*Remove buffer*/
      remove_lgknd_list_node(rm_node);
      tmp = rm_node->next;
      /*Update counter*/
      buf_cnt++;
    }
    else {
      /*Check overflow*/
      if (igate > (new_len-1)) {
        printf("Error: Overflow of stdgate's length!\n");
        exit(1);
      }
      lgkntwk->libgate_ptrs[igate] = tmp;
      igate++; 

      rm_node = tmp;
      tmp = tmp->next;
    }
  }
  
  stats->stdgate_stats[ND_BUF] = stats->stdgate_stats[ND_BUF] - buf_cnt;

  /*Print some info*/
  printf("%d Buffers are removed! %d libgates in total...\n",buf_cnt,new_len);

  return 1;
}

/**
 * Check Logic Network
 * 1. Check PI legal (No input!)
 * 2. Check PO legal (1 input!)
 */
int check_lgkntwk(t_lgkntwk* lgkntwk)
{
  int im;

  printf("Checking Logic Network after removing buffers...");

  /*Check PI*/
  for (im=0; im<lgkntwk->pi_num; im++)
  {
    if (lgkntwk->pi_ptrs[im]->type != ND_PI)
    {
      printf("\nError: find a none-PI node in PI list!\n");
      exit(1);
    }
    if (lgkntwk->pi_ptrs[im]->input_num != 0)
    {
      printf("\nError: PI(%d) has none-zero input number! This is illegal.\n",im);
      exit(1);
    }
    if (0 == lgkntwk->pi_ptrs[im]->output_num)
    {
      printf("\nWarning: PI(%d) has zero outputs! This is illegal.\n",im);
     // exit(1);
    }
    
  }

  /*Check PO*/
  for (im=0; im<lgkntwk->po_num; im++)
  {
    if (lgkntwk->po_ptrs[im]->type != ND_PO)
    {
      printf("\nError: find a none-PO node in PO list!\n");
      exit(1);
    }
    if (lgkntwk->po_ptrs[im]->output_num != 0)
    {
      printf("\nError: PO in Logic Network has none zero output.\n");
      exit(1);
    }
  }

  /*Check libgates*/
  for (im=0; im<lgkntwk->libgate_num; im++)
  {
    /*Check type*/
    if (ND_BUF == lgkntwk->libgate_ptrs[im]->type)
    {
      printf("\nError: Find Buffer in Logic Network!(Libgate index: %d)\n",im);
      exit(1);
    }
    /*Check inputs and outputs*/ 
    if (0 == lgkntwk->libgate_ptrs[im]->input_num) {
      if ((ND_LUT == lgkntwk->libgate_ptrs[im]->type)||(ND_ZERO == lgkntwk->libgate_ptrs[im]->type)||(ND_ONE == lgkntwk->libgate_ptrs[im]->type)) {
        printf("\nWarning: constant generator logic block#%d(%s) founded!\n",im,lgkntwk->libgate_ptrs[im]->net_name);
      } else {
        printf("\nError: logic block#%d(%s) has 0 input!\n",im,lgkntwk->libgate_ptrs[im]->net_name);
        exit(1);
      }
    }
    if (1 > lgkntwk->libgate_ptrs[im]->output_num)
    {
      printf("\nError: Libgate(Index:%d) has less than 1 output!\n",im);
      exit(1);
    }
  }

  /*Check latches*/
  for (im=0; im<lgkntwk->latch_num; im++)
  {
    /*Check type*/
    if (ND_LATCH != lgkntwk->latch_ptrs[im]->type)
    {
      printf("\nError: element(index:%d) 's type is not latch but it is in latch list\n",im);
      exit(1);
    }
    /*Check inputs and outputs*/ 
    if (1 > lgkntwk->latch_ptrs[im]->input_num)
    {
      printf("\nError: Latch(Index:%d) has less than 1 input!\n",im);
      exit(1);
    }
    if (1 > lgkntwk->latch_ptrs[im]->output_num)
    {
      printf("\nError: Latch(Index:%d) has less than 1 output!\n",im);
      exit(1);
    }
  }

  printf("Complete\n");

  return 1;
}

/**
 * DFS(Depth-First Search) the tree of logic nodes
 * Recursive algorithm
 */
int dfs_lgknd_tree_depth(t_lgknd* root,
                         int timing_analysis,
                         int verbose)
{
  int im;
   

  root->dfsed = 1;

  for(im=0; im<root->output_num; im++) {
    /*Check if PI*/
    if (root->outputs[im]->type == ND_PI) {
      printf("PI has input!\n");
      exit(1);
    }
    /*Update Depth if needed*/
    if ((0 == root->outputs[im]->dfsed)&&(root->outputs[im]->depth < (root->depth+1))) {
      /*Print Debug*/
      //printf("Update depth from %d to %d\n",root->outputs[im]->depth,root->depth+1);
      if (ND_LATCH == root->outputs[im]->type) {
        root->outputs[im]->depth_source = 0;
      } else {
        root->outputs[im]->depth_source += root->depth_source + 1;
      }
      root->outputs[im]->depth = root->depth + 1;

      /* Update timing information*/
      /*
      if (ND_LATCH == root->type) {
        root->outputs[im]->acc_delay = root->outputs[im]->delay + root->delay;
      }
      else if (ND_LATCH == root->outputs[im]->type) {
        root->outputs[im]->acc_delay = dff_tsu + root->acc_delay;
      }
      else {
        root->outputs[im]->acc_delay = root->outputs[im]->delay + root->acc_delay;
      }
      */

      /*Recursive if needed*/ 
      if ((root->outputs[im]->type != ND_PO)&&(0 == root->outputs[im]->dfsed))
      {
        if ((1 > root->outputs[im]->output_num)&&(ND_PO != root->outputs[im]->type)) {
            printf("Error: Zero output for logic node!\n");
            exit(1);
        }
        dfs_lgknd_tree_depth(root->outputs[im],timing_analysis,verbose);
      }
    }
  }  

  /*clear dfsed*/
  root->dfsed = 0;

  return 1;
}



/**
 * DFS (Depth First Search) logic network
 * Mark depth for each node
 * Latches do not increase depth, they are considered as followers.
 */
int dfs_lgkntwk_depth(t_lgkntwk* lgkntwk,
                      int timing_analysis,
                      int verbose)
{
  int im;
  int max_depth = 0;

  for (im=0; im<lgkntwk->pi_num; im++) {
    dfs_lgknd_tree_depth(lgkntwk->pi_ptrs[im],timing_analysis,verbose);
  }  
  // For constant generator
  for (im=0; im<lgkntwk->libgate_num; im++) {
    if (0 == lgkntwk->libgate_ptrs[im]->input_num) {
      dfs_lgknd_tree_depth(lgkntwk->libgate_ptrs[im],timing_analysis,verbose);
    }
  }

  for (im=0; im<lgkntwk->po_num; im++) {
    /*Get maximum depth*/
    if (max_depth < lgkntwk->po_ptrs[im]->depth) {
      max_depth = lgkntwk->po_ptrs[im]->depth;
    }
    /*DEBUG*/
    //printf("PO(index:%d) Depth: %d.\n",im,lgkntwk->po_ptrs[im]->depth);
  }

  printf("MAX PO Depth: %d.\n",max_depth);

  return 1;
}

// Calculate the combinational output num of a logic node,
int lgknd_fanout_comb_num(t_lgknd* lgknd) {
  int comb_num = 0;
  int i;
  for (i=0; i<lgknd->output_num; i++) {
    if (ND_LATCH != lgknd->outputs[i]->type) {
      comb_num++;
    }     
  }
  return comb_num;
}

/*
 * Search all latches, add a buffer if the latch's input is a PI
 */
int add_buf_latches(t_lgkntwk* lgkntwk,
                    t_ble_arch* ble_arch,
                    t_stats* stats,
                    int strict, // add buffer in any case
                    int verbose) {
  int im;
  int buf_num = 0;

  /*Add buffers to logic network*/
  for (im=0; im<lgkntwk->latch_num; im++) {
    assert(1 == lgkntwk->latch_ptrs[im]->input_num);
    /*Update: In benchmark, 38584.1 has cascaded latches*/
    if (ND_LATCH == lgkntwk->latch_ptrs[im]->inputs[0]->type) {
      // Definitely, we need a buffer
      add_one_buf_lgknd(lgkntwk->latch_ptrs[im],0);
      add_input_bufs_lgknd_lgkntwk(lgkntwk,lgkntwk->latch_ptrs[im]);
      buf_num++;
    } else if (ND_PI == lgkntwk->latch_ptrs[im]->inputs[0]->type) {
      // For PI, when have different strategies.
      if (ble_arch->blend_num < lgknd_fanout_comb_num(lgkntwk->latch_ptrs[im]->inputs[0])) {
        // We need a buffer
        add_one_buf_lgknd(lgkntwk->latch_ptrs[im],0);
        add_input_bufs_lgknd_lgkntwk(lgkntwk,lgkntwk->latch_ptrs[im]);
        buf_num++;
      } else {
        // We need a buffer, but all the fanouts of pi should be moved to this buffer....
        //add_pi_latch_buf(lgkntwk->latch_ptrs[im],0);
        add_one_buf_lgknd(lgkntwk->latch_ptrs[im],0);
        add_input_bufs_lgknd_lgkntwk(lgkntwk,lgkntwk->latch_ptrs[im]);
        buf_num++;
      }
    } else { // a LUT we should check the fanouts
      if ((1 == strict)&&(1 < lgkntwk->latch_ptrs[im]->inputs[0]->output_num)) {
        add_one_buf_lgknd(lgkntwk->latch_ptrs[im],0);
        add_input_bufs_lgknd_lgkntwk(lgkntwk,lgkntwk->latch_ptrs[im]);
        buf_num++;
      } else if ((0 == strict)&&(ble_arch->blend_num < lgknd_fanout_comb_num(lgkntwk->latch_ptrs[im]->inputs[0]))) {
        // We need a buffer
        add_one_buf_lgknd(lgkntwk->latch_ptrs[im],0);
        add_input_bufs_lgknd_lgkntwk(lgkntwk,lgkntwk->latch_ptrs[im]);
        buf_num++;
      }
    }
  }
  /*update statistics in stats*/
  stats->libgate_num += buf_num;
  stats->stdgate_stats[5] += buf_num;
  
  //if (1 == verbose) {
  printf("Info: add buffer to %d latch input.\n",buf_num);
  //}

  return 1;
}

/*Bubble Descend Sort, local DFS lgk nodes*/
int bubble_sort_depth_lgk_dfsnds(int num_dfsnd,
                                 t_lgknd** dfsnds,
                                 int* cur_depths,
                                 float* dfsnds_weight)
{
  int i = num_dfsnd;
  int j;
  int tmp;
  t_lgknd* tmpnd;

  while(i > 0) {
    for (j=0; j<(i - 1); j++) {
      // Consider the depth in logic network when we have the same.
      if (cur_depths[j] > cur_depths[j+1]) {
        /*Swap*/
        tmp = cur_depths[j];
        tmpnd = dfsnds[j];
        cur_depths[j] = cur_depths[j+1];
        dfsnds[j] = dfsnds[j+1];
        cur_depths[j+1] = tmp;
        dfsnds[j+1] = tmpnd;
      //} else if ((cur_depths[j] == cur_depths[j+1])&&(dfsnds[j]->depth > dfsnds[j+1]->depth)) {
      } else if ((cur_depths[j] == cur_depths[j+1])&&(dfsnds_weight[j] > dfsnds_weight[j+1])) {
        /*Swap*/
        tmp = cur_depths[j];
        tmpnd = dfsnds[j];
        cur_depths[j] = cur_depths[j+1];
        dfsnds[j] = dfsnds[j+1];
        cur_depths[j+1] = tmp;
        dfsnds[j+1] = tmpnd;
      }
    }
    i--;
  }
  return 1;
}


/**
 * Local DFS logic nodes
 */
int* local_dfs_lgknds(int num_dfsnds,
                      t_lgknd** dfsnds,
                      float* dfsnds_weight,
                      int verbose)
{
  int ind;
  int im;
  int idfsnd;
  int* cur_depths = (int*)my_malloc(num_dfsnds*sizeof(int));
  int* pre_depths = (int*)my_malloc(num_dfsnds*sizeof(int));
  int* dfsed = (int*)my_malloc(num_dfsnds*sizeof(int));
  int pre_match = 0;


  /*Initial*/
  for (im=0; im<num_dfsnds; im++) {
    cur_depths[im] = 0;
    pre_depths[im] = 0;
    dfsed[im] = 0;
  }

  while(1) {
    for (ind=0; ind<num_dfsnds; ind++) {
      for (im=0; im<dfsnds[ind]->input_num; im++) {
        for (idfsnd=0; idfsnd<num_dfsnds; idfsnd++) {
          if ((dfsnds[ind]->inputs[im] == dfsnds[idfsnd])&&(0 == dfsed[ind])) {
            if (cur_depths[ind] < (cur_depths[idfsnd] + 1)) {
              cur_depths[ind] = cur_depths[idfsnd] + 1;
              dfsed[ind] = 1;
            }
          }
        }
      }
      dfsed[ind] = 1; 
    }
    /*Check pre_depths ?= cur_depths*/
    pre_match = 0;
    for (im=0; im<num_dfsnds; im++) {
      if (pre_depths[im] == cur_depths[im]) {
        pre_match++;
      }
    }
    /*If match, break*/
    if (num_dfsnds == pre_match) {
      break;
    }
    /*Clear and update pre_depths*/
    for (im=0; im<num_dfsnds; im++) {
      pre_depths[im] = cur_depths[im];
      dfsed[im] = 0;
    }
  }

  bubble_sort_depth_lgk_dfsnds(num_dfsnds,dfsnds,cur_depths,dfsnds_weight);
             
  //if (1 == verbose) {
  //  for (im=0; im<num_dfsnds; im++) {
  //    printf("Info: Local dfs,lgknd[%d].depth=%d\n",im,cur_depths[im]);
  //  }
  //}

  /*Don't forget free resources*/
  free(pre_depths);
  free(dfsed);

  return cur_depths;
}

/**
 * Add a buffer between PI and Latch
 * IMPORTANT: in this function, we DON'T UPDATE lgkntwk!!!
 *            it will be left to another function update_bufs_lgkntwk
 */
int add_pi_latch_buf(t_lgknd* cur,
                     int input_idx) {
  char BUF[BUFSIZE];

  assert(ND_LATCH == cur->type);
  assert(0 == input_idx);
  assert(ND_PI == cur->inputs[input_idx]->type);
  //assert(cur->in_edges == NULL);
  //assert(cur->out_edges == NULL);
  //assert(cur->inputs[input_idx]->in_edges == NULL);
  //assert(cur->inputs[input_idx]->out_edges == NULL);
  /*Malloc a new logic node*/ 
  t_lgknd* buf = (t_lgknd*)my_malloc(sizeof(t_lgknd));
  /*Initial*/
  init_lgknd(buf);
  /*Assign buffer information*/
  buf->type = ND_BUF;
  buf->input_num = 1;
  buf->inputs = (t_lgknd**)my_malloc(sizeof(t_lgknd*));
  buf->output_num = cur->inputs[input_idx]->output_num;
  // Allocate edges
  buf->in_edges = (t_edge*)my_malloc(sizeof(t_edge));
  //buf->out_edges = (t_edge**)my_malloc(sizeof(t_edge*));
  init_edge(&buf->in_edges[0]);
  // next pointer
  buf->next = NULL;
  /*Create Net name _buf\0*/
  snprintf(BUF,BUFSIZE,"%d",cur->inputs[input_idx]->fanout_buf_num); //TODO: should use my_atoi
  buf->net_name = (char*)my_malloc(sizeof(char)*(strlen(cur->inputs[input_idx]->net_name)+4+strlen(BUF)+1));
  strcpy(buf->net_name,cur->inputs[input_idx]->net_name);
  strcat(buf->net_name,"_buf");
  strcat(buf->net_name,BUF);
  cur->inputs[input_idx]->fanout_buf_num++;
  /*Modify current logic node's predecessor*/
  // Move all the fanouts of predecessor to the buffer
  // Exchange the outputs between PI and buffer
  buf->outputs = cur->inputs[input_idx]->outputs;
  buf->inputs[0] = cur->inputs[input_idx];
  cur->inputs[input_idx]->output_num = 1;
  cur->inputs[input_idx]->outputs = NULL;
  cur->inputs[input_idx]->outputs = (t_lgknd**)my_malloc(sizeof(t_lgknd*));
  cur->inputs[input_idx]->outputs[0] = buf;
  /*Make Depth*/
  buf->depth = cur->inputs[input_idx]->depth;
  /*Make connections for new buffer*/
  buf->in_edges[0].weight = 1;
  buf->out_edges = cur->inputs[input_idx]->out_edges;
  cur->inputs[input_idx]->out_edges = NULL;
  cur->inputs[input_idx]->out_edges = (t_edge**)my_malloc(sizeof(t_edge*));
  cur->inputs[input_idx]->out_edges[0] = &(buf->in_edges[0]);
  //buf->in_edges[0].weight = 1/buf->inputs[0]->output_num;
  /*Modify current logic node*/
  cur->in_edges[input_idx].weight = 1;
  //cur->in_edges[input_idx].weight = 1/buf->output_num;  
  cur->inputs[input_idx]->outputs[0] = buf;
  cur->inputs[input_idx] = buf;

  return 1;
}


/**
 * Add a buffer to input of logic node
 * IMPORTANT: in this function, we DON'T UPDATE lgkntwk!!!
 *            it will be left to another function update_bufs_lgkntwk
 */
int add_one_buf_lgknd(t_lgknd* cur,
                      int input_idx)
{
  int im;
  char BUF[BUFSIZE];

  /*Malloc a new logic node*/ 
  t_lgknd* buf = (t_lgknd*)my_malloc(sizeof(t_lgknd));
  /*Initial*/
  init_lgknd(buf);
  /*Assign buffer information*/
  buf->type = ND_BUF;
  buf->input_num = 1;
  buf->inputs = (t_lgknd**)my_malloc(sizeof(t_lgknd*));
  buf->output_num = 1;
  buf->outputs = (t_lgknd**)my_malloc(sizeof(t_lgknd*));
  // Allocate edges
  buf->in_edges = (t_edge*)my_malloc(sizeof(t_edge));
  buf->out_edges = (t_edge**)my_malloc(sizeof(t_edge*));
  init_edge(&buf->in_edges[0]);
  // next pointer
  buf->next = NULL;
  /*Create Net name _buf\0*/
  snprintf(BUF,BUFSIZE,"%d",cur->inputs[input_idx]->fanout_buf_num); //TODO: should use my_atoi
  buf->net_name = (char*)my_malloc(sizeof(char)*(strlen(cur->inputs[input_idx]->net_name)+4+strlen(BUF)+1));
  strcpy(buf->net_name,cur->inputs[input_idx]->net_name);
  strcat(buf->net_name,"_buf");
  strcat(buf->net_name,BUF);
  cur->inputs[input_idx]->fanout_buf_num++;
  /*Modify current logic node's predecessor*/
  for (im=0; im<cur->inputs[input_idx]->output_num; im++) { 
    if (cur == cur->inputs[input_idx]->outputs[im]) {
      cur->inputs[input_idx]->outputs[im] = buf;
      // Modify the edges
      cur->inputs[input_idx]->out_edges[im] = &(buf->in_edges[0]); 
      break;
    }
  } 
  /*Make Depth*/
  buf->depth = cur->inputs[input_idx]->depth;
  /*Make connections for new buffer*/
  buf->inputs[0] = cur->inputs[input_idx];
  //buf->in_edges[0].weight = 1;
  //buf->in_edges[0].depth_weight = 2/buf->inputs[0]->output_num;
  buf->outputs[0] = cur;
  buf->out_edges[0] = &(cur->in_edges[input_idx]);
  /*Modify current logic node*/
  cur->inputs[input_idx] = buf;
  //cur->in_edges[input_idx].weight = 1;
  //cur->in_edges[input_idx].weight = 1/buf->output_num;  

  return 1;
}

/**
 * Remove a buffer from input of logic node 
 * IMPORTANT: in this function, we DON'T UPDATE lgkntwk!!!
 *            it will be left to another function update_bufs_lgkntwk
 */
int remove_one_buf_lgknd(t_lgknd* cur,
                         int input_idx)
{
  int im;
  /*Get buffer pointer*/
  t_lgknd* buf = cur->inputs[input_idx];
  if (ND_BUF != buf->type) {
    printf("Error: Try to remove a non-buf node!\n");
    exit(1);
  }
  /*Modify buffer's predecessor*/
  for (im=0; im<buf->inputs[0]->output_num; im++) { 
    if (buf == buf->inputs[0]->outputs[im]) {
      buf->inputs[0]->outputs[im] = cur;
      break;
    }
  } 
  /*Modify current logic node*/
  cur->inputs[input_idx] = buf->inputs[0];
  
  /*Free buffer node*/
  free_lgknd(buf);
  free(buf);

  return 1;
}

/**
 * Add input buffers of given logic node into lgkntwk
 */
int add_input_bufs_lgknd_lgkntwk(t_lgkntwk* lgkntwk,
                                 t_lgknd* cur)
{
  int im;
  int buf_num = 0;
  t_lgknd* buf = NULL;
  int cnt;
  int buf_idx_base = lgkntwk->libgate_ptrs[lgkntwk->libgate_num-1]->idx+1;
  
  /*Add buffers into lgkntwk->libgate_head*/
  for (im=0; im<cur->input_num; im++) {
    if (ND_BUF == cur->inputs[im]->type) {
      buf = cur->inputs[im];
      buf->idx = buf_idx_base + buf_num;
      buf->depth = cur->depth - 1;
      /*Insert into libgate_head*/
      buf->next = lgkntwk->libgate_head->next;
      lgkntwk->libgate_head->next = buf;
      buf_num++;
      //lgkntwk->libgate_num++;
    }
  }

  /*Re-alloc lgkntwk->libgate_ptrs*/
  cnt = lgkntwk->libgate_num;
  lgkntwk->libgate_num += buf_num;
  lgkntwk->libgate_ptrs = (t_lgknd**)my_realloc(lgkntwk->libgate_ptrs,lgkntwk->libgate_num*sizeof(t_lgknd*));
  for (im=0; im<cur->input_num; im++) {
    if (ND_BUF == cur->inputs[im]->type) {
      lgkntwk->libgate_ptrs[cnt] = cur->inputs[im];
      cnt++;
    }
  } 

  return 1;
}


/*
 * Split the combinational output and sequential output of logic node
 * Simliar to VPACK, a buffer is added before sequential output
 * The created buffer is added to logic network
 */
int split_lgknd_output(t_lgkntwk* lgkntwk,
                       t_lgknd* lgknd)
{
  int im = 0;
  int comb_num = 0;
  int seq_num = 0;
 
  /* First, check there is a need to split the outputs.
   * Check if there is two types outputs
   */
  for (im=0; im<lgknd->output_num; im++) {
    if (ND_LATCH == lgknd->outputs[im]->type) {
      seq_num++;
    }
    else {
      comb_num++;
    }
  } 

  if (seq_num > 1) {
    printf("Warning: (split_lgknd_output)Logic node has more than 1 latch as outputs!\n");
  }
  /*There is no need...*/
  if ((0 == seq_num)||(0 == comb_num)) {
    return 1;
  }
  /*Add buffer before latch if comb_num > 0*/
  if (comb_num > 0) {
    for (im=0; im<lgknd->output_num; im++) {
      if (ND_LATCH == lgknd->outputs[im]->type) {
        add_one_buf_lgknd(lgknd->outputs[im],0);
        add_input_bufs_lgknd_lgkntwk(lgkntwk,lgknd->outputs[im]);
      }
    }
  }

  return 1; 
}

/* Split the combinational outputs and sequential outputs
 * of certain logic node in logic network.
 * What kind of logic nodes need split? Those have both
 * Seqential outputs and more than 1 combinational outputs
 */
int split_lgkntwk_lgknds_outputs(t_lgkntwk* lgkntwk,
                                 int output_num_upbound,
                                 int verbose)
{
  int im;
  int cnt = 0;
  t_lgknd* lgknd = NULL;

  for (im=0; im<lgkntwk->latch_num; im++) {
    lgknd = lgkntwk->latch_ptrs[im]->inputs[0];
    /*Check*/  
    if ((ND_PI != lgknd->type)&&(ND_PO != lgknd->type)&&(ND_LATCH != lgknd->type)) {
      if (lgknd->output_num > output_num_upbound) {
        split_lgknd_output(lgkntwk,lgknd);
        cnt++;
      }
    }
    else {
      printf("Error:(split_lgkntwk_lgknds_outputs)Latch input is a [PI|PO|LATCH]!\n");
      exit(1);
    }
  } 

  printf("Info: Split outputs for %d logic gates.\n",cnt);

  return 1;
}
            
/*Reset try_sat to 0 for all libgates in lgkntwk*/
void reset_lgkntwk_try_sat(t_lgkntwk* lgkntwk)
{
  int im;
  
  for (im=0; im<lgkntwk->libgate_num; im++) {
    lgkntwk->libgate_ptrs[im]->try_sat = 0;
  } 
}

/*Find the predecessor with minimum-fanout, return input_idx*/
int find_min_fanout_pred(t_lgknd* lgknd,
                         int verbose)
{
  int ret,min,i;

  min = -1;
  for (i=0; i < lgknd->input_num; i++) {
    if ((-1 == min)||(min > (lgknd->inputs[i]->output_num+lgknd->inputs[i]->redund_output_num+lgknd->inputs[i]->tmp_redund_output_num))) {
      ret = i;
      min = lgknd->inputs[i]->output_num 
          + lgknd->inputs[i]->redund_output_num
          + lgknd->inputs[i]->tmp_redund_output_num;
    }
  }
  
  return ret;
}

/* Clear redundant output number as 0
 * IMPORTANT: it clears lgknds predecessor's temporary
 *            redundant output number!
 */
int clear_tmp_redund_output_num(int lgknd_num,
                                t_lgknd** lgknds,
                                int verbose)
{
  int i,j;
    
  for (i = 0; i < lgknd_num; i++) {
    for (j = 0; j <lgknds[i]->input_num; j++) {
      lgknds[i]->inputs[j]->tmp_redund_output_num = 0;
    }
  }

  return 1;
}

/* Update redundant output number by adding tmp_redund_output_num
 * IMPORTANT: it updates lgknds predecessor's
 *            redundant output number!
 */
int update_redund_output_num(int lgknd_num,
                             t_lgknd** lgknds,
                             int verbose)
{
  int i,j;
    
  for (i = 0; i < lgknd_num; i++) {
    for (j = 0; j <lgknds[i]->input_num; j++) {
      lgknds[i]->inputs[j]->redund_output_num += lgknds[i]->inputs[j]->tmp_redund_output_num;
    }
  }

  return 1;
}

/** Initial Edges
 */
int init_lgkntwk_edges(t_lgkntwk* lgkntwk,
                       int verbose) {
  int iedge,edge_num;
  int i;
  t_lgknd* head;
  /** We alway allocate those edges for lgknd inputs only. 
   *  Every lgknd input has only one driver! 
   *  Then we can connect these edges with their predecessors
   */
  /*For PI*/
  head = lgkntwk->pi_head->next;
  while(head != NULL) {
    // Each PI has 0 input
    assert(0 == head->input_num);
    head->out_edges = (t_edge**)my_malloc(head->output_num*sizeof(t_edge*));
    head = head->next;
  }
  /*For PO*/
  edge_num = 0;
  head = lgkntwk->po_head->next;
  while(head != NULL) {
    // Each PO has only one input, and 0 output
    assert(1 == head->input_num);
    assert(0 == head->output_num);
    // Alloc edges
    edge_num += head->input_num;
    head->in_edges = (t_edge*)my_malloc(head->input_num*sizeof(t_edge));
    for (iedge=0; iedge<head->input_num; iedge++) {
      init_edge(&head->in_edges[iedge]);
    }
    //lgkntwk->po_ptrs[i]->out_edges = (t_edge**)my_malloc(lgkntwk->po_ptrs[i]->output_num*sizeof(t_edges*));
    head = head->next;
  }
  /*For Latch*/
  head = lgkntwk->latch_head->next;
  while(head != NULL) {
    // Each Latch has only one input, D.
    assert(1 == head->input_num);
    // Alloc edges
    edge_num += head->input_num;
    head->in_edges = (t_edge*)my_malloc(head->input_num*sizeof(t_edge));
    for (iedge=0; iedge<head->input_num; iedge++) {
      init_edge(&head->in_edges[iedge]);
    }
    head->out_edges = (t_edge**)my_malloc(head->output_num*sizeof(t_edge*));
    for (iedge=0; iedge<head->output_num; iedge++) {
      head->out_edges[iedge] = NULL;
    }
    head = head->next;
  }
  /*For Libgate*/
  head = lgkntwk->libgate_head->next;
  while(head != NULL) {
    // Alloc edges
    edge_num += head->input_num;
    if (0 < head->input_num) {
      head->in_edges = (t_edge*)my_malloc(head->input_num*sizeof(t_edge));
      for (iedge=0; iedge<head->input_num; iedge++) {
        init_edge(&head->in_edges[iedge]);
      }
    }
    head->out_edges = (t_edge**)my_malloc(head->output_num*sizeof(t_edge*));
    for (iedge=0; iedge<head->output_num; iedge++) {
      head->out_edges[iedge] = NULL;
    }
    head = head->next;
  }
  printf("Info: Number of Edges = %d.\n",edge_num);

  // Alloc completed, then fill the out_edges
  /*For PO*/
  head = lgkntwk->po_head->next;
  while(head != NULL) {
    assert(1 == head->input_num);
    assert(0 == head->output_num);
    // Setup out_edges
    for (iedge=0; iedge<head->input_num; iedge++) {
      for (i=0; i<head->inputs[iedge]->output_num; i++) {
        if (head == head->inputs[iedge]->outputs[i]) {
          head->inputs[iedge]->out_edges[i] = &head->in_edges[iedge];
          break;
        }
      }
    }
    head = head->next;
  }
  /*For Latch*/
  head = lgkntwk->latch_head->next;
  while(head != NULL) {
    // Each latch has only 1 input
    assert(1 == head->input_num);
    // Setup out_edges
    for (iedge=0; iedge<head->input_num; iedge++) {
      for (i=0; i<head->inputs[iedge]->output_num; i++) {
        if (head == head->inputs[iedge]->outputs[i]) {
          head->inputs[iedge]->out_edges[i] = &head->in_edges[iedge];
          break;
        }
      }
    }
    head = head->next;
  }
  /*For libgate*/
  head = lgkntwk->libgate_head->next;
  while(head != NULL) {
    // Setup out_edges
    for (iedge=0; iedge<head->input_num; iedge++) {
      for (i=0; i<head->inputs[iedge]->output_num; i++) {
        if (head == head->inputs[iedge]->outputs[i]) {
          head->inputs[iedge]->out_edges[i] = &head->in_edges[iedge];
          break;
        }
      }
    }
    head = head->next;
  }
  // Check each out_edge_num match the output_num, and inital the weight
  /*For PI*/
  head = lgkntwk->pi_head->next;
  while(head != NULL) {
    for (iedge=0; iedge<head->output_num; iedge++) {
      assert(NULL != head->out_edges[iedge]);
      //head->out_edges[iedge]->weight = 1;
      //head->out_edges[iedge]->weight = 1/head->output_num;
    }
    head = head->next;
  }
  /*For PO*/
  head = lgkntwk->po_head->next;
  while(head != NULL) {
    for (iedge=0; iedge<head->output_num; iedge++) {
      assert(NULL != head->out_edges[iedge]);
      //head->out_edges[iedge]->weight = 1;
      //head->out_edges[iedge]->weight = 1/head->output_num;
    }
    head = head->next;
  }
  /*For latch*/
  head = lgkntwk->latch_head->next;
  while(head != NULL) {
    for (iedge=0; iedge<head->output_num; iedge++) {
      assert(NULL != head->out_edges[iedge]);
      //head->out_edges[iedge]->weight = 1;
      //head->out_edges[iedge]->weight = 1/head->output_num;
    }
    head = head->next;
  }
  /*For libgate*/
  head = lgkntwk->libgate_head->next;
  while(head != NULL) {
    for (iedge=0; iedge<head->output_num; iedge++) {
      assert(NULL != head->out_edges[iedge]);
      //head->out_edges[iedge]->weight = 1;
      //head->out_edges[iedge]->weight = 1/head->output_num;
    }
    head = head->next;
  }
  
  return 1;
}

// Give weight according to the depth delta
int depth_weighting_lgkntwk(t_lgkntwk* lgkntwk,
                            int verbose) {
  // Only those have inputs have in_edges
  t_lgknd* head;
  int iedge;
  int max_depth_delta;
  int depth_delta;

  // Start from All libgates
  head = lgkntwk->libgate_head->next;  
  while(head != NULL) {
    max_depth_delta = 1;
    for (iedge=0; iedge<head->input_num; iedge++) {
      if (max_depth_delta < (head->depth - head->inputs[iedge]->depth)) {
        max_depth_delta = (head->depth - head->inputs[iedge]->depth);
      }
    }
    for (iedge=0; iedge<head->input_num; iedge++) {
      depth_delta = head->depth - head->inputs[iedge]->depth;
      if (0 == depth_delta) {
        depth_delta = 1;
      }
      //head->in_edges[iedge].depth_weight = depth_delta/max_depth_delta;
      //head->in_edges[iedge].depth_weight = 1/depth_delta;
      head->in_edges[iedge].depth_weight = 1;
      //head->in_edges[iedge].depth_weight = 2/head->inputs[iedge]->output_num;
    }
    head = head->next;
  }
  // Latches
  head = lgkntwk->latch_head->next;  
  while(head != NULL) {
    assert(1 == head->input_num);
    max_depth_delta = 1;
    for (iedge=0; iedge<head->input_num; iedge++) {
      if (max_depth_delta < (head->depth - head->inputs[iedge]->depth)) {
        max_depth_delta = (head->depth - head->inputs[iedge]->depth);
      }
    }
    for (iedge=0; iedge<head->input_num; iedge++) {
      depth_delta = head->depth - head->inputs[iedge]->depth;
      if (0 == depth_delta) {
        depth_delta = 1;
      }
      //head->in_edges[iedge].depth_weight = depth_delta/max_depth_delta;
      //head->in_edges[iedge].depth_weight = 1/depth_delta;
      head->in_edges[iedge].depth_weight = 1;
      //head->in_edges[iedge].depth_weight = 2/head->inputs[iedge]->output_num;
    } 
    head = head->next;
  }
  // POs
  head = lgkntwk->po_head->next;  
  while(head != NULL) {
    assert(1 == head->input_num);
    assert(0 == head->output_num);
    max_depth_delta = 1;
    for (iedge=0; iedge<head->input_num; iedge++) {
      if (max_depth_delta < (head->depth - head->inputs[iedge]->depth)) {
        max_depth_delta = (head->depth - head->inputs[iedge]->depth);
      }
    }
    for (iedge=0; iedge<head->input_num; iedge++) {
      depth_delta = head->depth - head->inputs[iedge]->depth;
      if (0 == depth_delta) {
        depth_delta = 1;
      }
      //head->in_edges[iedge].depth_weight = depth_delta/max_depth_delta;
      //head->in_edges[iedge].depth_weight = 1/depth_delta;
      head->in_edges[iedge].depth_weight = 1;
      //head->in_edges[iedge].depth_weight = 2/head->inputs[iedge]->output_num;
    } 
    head = head->next;
  }
  
  return 1;
}

// Initial delay information for logic network
int init_timing_lgkntwk(t_lgkntwk* lgkntwk,
                       t_delay_info* delay_info,
                       int verbose) {
  t_lgknd* head;
  int iedge;
  // Start from All libgates
  head = lgkntwk->libgate_head->next;  
  while(head != NULL) {
    head->delay = delay_info->gate_delay;
    head->t_arrival = 0.0;
    head->t_required = -1.0;
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].delay = delay_info->inter_cluster_delay + delay_info->inner_cluster_delay + delay_info->ff_mux_delay;
      head->in_edges[iedge].t_arrival = 0.0;
      // Special, if we find the latch output is connected to its input logic cell
      if (ND_LATCH == head->inputs[iedge]->type) {
        assert(1 == head->inputs[iedge]->input_num);
        if (head->inputs[iedge]->inputs[0] == head) {
          head->in_edges[iedge].delay = delay_info->inner_cluster_delay + delay_info->ff_mux_delay;
        }
      } else if ((1 == head->mapped)&&(1 == head->inputs[iedge]->mapped)&&(head->ble_info == head->inputs[iedge]->ble_info)) {
        head->in_edges[iedge].delay = delay_info->inner_cluster_delay;
      }
    }
    head = head->next;
  }
  // Latches
  head = lgkntwk->latch_head->next;  
  while(head != NULL) {
    assert(1 == head->input_num);
    head->delay = delay_info->ff_delay;
    head->t_arrival = 0.0;
    head->t_required = -1.0;
    head->in_edges[0].delay = delay_info->ff_tsu;
    head->in_edges[0].t_arrival = 0.0;
    head = head->next;
  }
  // PO
  head = lgkntwk->po_head->next;  
  while(head != NULL) {
    assert(1 == head->input_num);
    assert(0 == head->output_num);
    head->delay = delay_info->outpad_delay;
    head->t_arrival = 0.0;
    head->t_required = -1.0;
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].delay = delay_info->inter_cluster_delay + delay_info->ff_mux_delay;
      head->in_edges[iedge].t_arrival = 0.0;
    }
    head = head->next;
  }
  // PI
  head = lgkntwk->pi_head->next;  
  while(head != NULL) {
    assert(0 == head->input_num);
    head->delay = delay_info->inpad_delay;
    head->t_arrival = 0.0;
    head->t_required = -1.0;
    head = head->next;
  }
  return 1;
}

// Reset DFS tags
int reset_dfs_tags_lgkntwk(t_lgkntwk* lgkntwk,
                           int verbose) {
  t_lgknd* head; 
  
  // Reset the dfs tags
  head = lgkntwk->pi_head->next;
  while(NULL != head) {
    head->dfsed = 0;
    head = head->next;
  } 
  head = lgkntwk->po_head->next;
  while(NULL != head) {
    head->dfsed = 0;
    head = head->next;
  } 
  head = lgkntwk->libgate_head->next;
  while(NULL != head) {
    head->dfsed = 0;
    head = head->next;
  } 
  head = lgkntwk->latch_head->next;
  while(NULL != head) {
    head->dfsed = 0;
    head = head->next;
  } 

  return 1;
}

int timing_analysis_lgknd(t_lgknd* root,
                          int verbose) {
  int i;

  // A source node...
  // FOR LATCH NODE: We start a new path or constant generator
  for (i=0; i<root->input_num; i++) {
    if ((ND_LATCH == root->inputs[i]->type)||(0 == root->inputs[0]->input_num)) {
      root->in_edges[i].t_arrival = root->in_edges[i].delay+root->inputs[i]->delay;
    } else {
      root->in_edges[i].t_arrival = root->in_edges[i].delay+root->inputs[i]->t_arrival;
    }
    if (root->t_arrival < (root->in_edges[i].t_arrival+root->delay)) {
      root->t_arrival = root->in_edges[i].t_arrival+root->delay;
    }
  }
  root->dfsed = 1;
  // Analysis the fanouts if we have...
  for (i=0; i<root->output_num; i++) {
    // Avoid Loop
    if (0 == root->outputs[i]->dfsed) {
      timing_analysis_lgknd(root->outputs[i],verbose);
    }
  }
   
  return 1;
}

// Timing analysis the logic network
int timing_analysis_lgkntwk(t_lgkntwk* lgkntwk,
                            int verbose) {
  t_lgknd* head = lgkntwk->pi_head->next;
  
  // Initial all the latch acc_delay to deal with loop
  head = lgkntwk->latch_head->next;
  while(NULL != head) {
    assert(head->type == ND_LATCH);
    head->t_arrival = 0;
    head = head->next;
  }
  // Initial all the constant generator 
  head = lgkntwk->libgate_head->next;
  while(NULL != head) {
    if (0 == head->input_num) {
      head->t_arrival = 0;
    }
    head = head->next;
  }
  // We start each Primary Input
  head = lgkntwk->pi_head->next;
  while(NULL != head) {
    assert(head->type == ND_PI);
    reset_dfs_tags_lgkntwk(lgkntwk,verbose);
    timing_analysis_lgknd(head,verbose);
    head = head->next;
  }
  // If we have a constant generator, we should do...
  head = lgkntwk->libgate_head->next;
  while(NULL != head) {
    if (0 == head->input_num) {
      reset_dfs_tags_lgkntwk(lgkntwk,verbose);
      timing_analysis_lgknd(head,verbose);
    }
    head = head->next;
  } 

  return 1;
}

// The critical delay is actually the longest acc_delay of a latch or PO. 
float critical_delay_lgkntwk(t_lgkntwk* lgkntwk,
                            int verbose) {
  t_lgknd* head;
  float crit_delay = -1.0;
  // search latch
  head = lgkntwk->latch_head->next;
  while (NULL != head) {
    assert(head->type == ND_LATCH);
    if ((-1.0 == crit_delay)||(crit_delay < head->t_arrival)) {
      crit_delay = head->t_arrival;
    }
    head = head->next;
  }
  // search PO
  head = lgkntwk->po_head->next;
  while (NULL != head) {
    assert(head->type == ND_PO);
    if ((-1.0 == crit_delay)||(crit_delay < head->t_arrival)) {
      crit_delay = head->t_arrival;
    }
    head = head->next;
  }
  // Return
  if (-1.0 == crit_delay) {
    printf("Error: (critical_delay_lgkntwk)Getting critical path fails!\n");
  }
  return crit_delay;
}

int timing_required_lgknd(t_lgknd* root,
                          int verbose) {
  int i;

  // A source node...
  // FOR LATCH NODE: We start a new path or constant generator
  for (i=0; i<root->input_num; i++) {
    if ((ND_LATCH != root->inputs[i]->type)&&(ND_PO != root->inputs[i]->type)) {
      if ((-1.0 == root->inputs[i]->t_required)||(root->inputs[i]->t_required > (root->t_required - root->in_edges[i].delay))) {
        root->inputs[i]->t_required = (root->t_required - root->in_edges[i].delay);
      }
    }
  }
  root->dfsed = 1;
  // Analysis the fanouts if we have...
  for (i=0; i<root->input_num; i++) {
    // Avoid Loop
    if (0 == root->inputs[i]->dfsed) {
      timing_required_lgknd(root->inputs[i],verbose);
    }
  }
   
  return 1;
}

 
// For latches and PO, we set the t_required to critical_delay
int timing_required_lgkntwk(t_lgkntwk* lgkntwk,
                            float crit_delay,
                            int verbose) {
  t_lgknd* head;
  // search PO
  head = lgkntwk->po_head->next;
  while (NULL != head) {
    assert(head->type == ND_PO);
    head->t_required = crit_delay;
    head = head->next;
  }
  // search latches
  head = lgkntwk->latch_head->next;
  while (NULL != head) {
    assert(head->type == ND_LATCH);
    head->t_required = crit_delay;
    head = head->next;
  }
  // We start each Primary Output
  head = lgkntwk->po_head->next;
  while(NULL != head) {
    assert(head->type == ND_PO);
    reset_dfs_tags_lgkntwk(lgkntwk,verbose);
    timing_required_lgknd(head,verbose);
    head = head->next;
  }

  return 1;
}

// Get max slack
float timing_max_slack_lgkntwk(t_lgkntwk* lgkntwk,
                               int verbose) {
  t_lgknd* head;
  int iedge;
  float max_slack = -1.0;

  // Start from All libgates
  head = lgkntwk->libgate_head->next;  
  while(head != NULL) {
    for (iedge=0; iedge<head->input_num; iedge++) {
      if ((-1.0 == max_slack)||(max_slack < head->in_edges[iedge].slack)) {
        max_slack = head->in_edges[iedge].slack;
      }
    }
    head = head->next;
  }
  // Latches
  head = lgkntwk->latch_head->next;  
  while(head != NULL) {
    assert(1 == head->input_num);
    for (iedge=0; iedge<head->input_num; iedge++) {
      if ((-1.0 == max_slack)||(max_slack < head->in_edges[iedge].slack)) {
        max_slack = head->in_edges[iedge].slack;
      }
    }
    head = head->next;
  }
  // POs
  head = lgkntwk->po_head->next;  
  while(head != NULL) {
    assert(1 == head->input_num);
    assert(0 == head->output_num);
    for (iedge=0; iedge<head->input_num; iedge++) {
      if ((-1.0 == max_slack)||(max_slack < head->in_edges[iedge].slack)) {
        max_slack = head->in_edges[iedge].slack;
      }
    }
    head = head->next;
  }
  
  // Return
  if (-1.0 == max_slack) {
    printf("Error: (timing_max_slack_lgkntwk)Getting max slack fails!\n");
  }
  return max_slack;
}

int timing_slack_lgkntwk(t_lgkntwk* lgkntwk,
                         int verbose) {
  t_lgknd* head;
  int iedge;
  // search PO
  head = lgkntwk->po_head->next;
  while (NULL != head) {
    assert(head->type == ND_PO);
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].slack = head->t_required - head->inputs[iedge]->t_arrival - head->in_edges[iedge].delay;
      // Find the minimum slack
      if ((0 == iedge)||(head->slack > head->in_edges[iedge].slack)) {
        head->slack = head->in_edges[iedge].slack;
      }
    }
    head = head->next;
  }
  // search libgates
  head = lgkntwk->libgate_head->next;
  while (NULL != head) {
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].slack = head->t_required - head->inputs[iedge]->t_arrival - head->in_edges[iedge].delay;
      // Find the minimum slack
      if ((0 == iedge)||(head->slack > head->in_edges[iedge].slack)) {
        head->slack = head->in_edges[iedge].slack;
      }
    }
    head = head->next;
  }
  // search latches
  head = lgkntwk->latch_head->next;
  while (NULL != head) {
    assert(head->type == ND_LATCH);
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].slack = head->t_required - head->inputs[iedge]->t_arrival - head->in_edges[iedge].delay;
      // Find the minimum slack
      if ((0 == iedge)||(head->slack > head->in_edges[iedge].slack)) {
        head->slack = head->in_edges[iedge].slack;
      }
    }
    head = head->next;
  }
  // Speical for those 0 fanin node
  // search PI
  head = lgkntwk->pi_head->next;
  while (NULL != head) {
    assert(head->type == ND_PI);
    for (iedge=0; iedge<head->output_num; iedge++) {
      // Find the minimum slack
      if ((0 == iedge)||(head->slack > head->out_edges[iedge]->slack)) {
        head->slack = head->out_edges[iedge]->slack;
      }
    }
    head = head->next;
  }
  // search libgates: 0 fanin
  head = lgkntwk->libgate_head->next;
  while (NULL != head) {
    if (head->input_num == 0) {
      for (iedge=0; iedge<head->output_num; iedge++) {
        // Find the minimum slack
        if ((0 == iedge)||(head->slack > head->out_edges[iedge]->slack)) {
          head->slack = head->out_edges[iedge]->slack;
        }
      }
    }
    head = head->next;
  }
  return 1; 
}

// Timing Slack for lgkntwk
int timing_weighting_lgkntwk(t_lgkntwk* lgkntwk,
                             float max_slack,
                             int verbose) {
  t_lgknd* head;
  int iedge;
  // search PO
  head = lgkntwk->po_head->next;
  while (NULL != head) {
    assert(head->type == ND_PO);
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].timing_weight = 1 - (head->in_edges[iedge].slack/max_slack);
    }
    head = head->next;
  }
  // search libgates
  head = lgkntwk->libgate_head->next;
  while (NULL != head) {
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].timing_weight = 1 - (head->in_edges[iedge].slack/max_slack);
    }
    head = head->next;
  }
  // search latches
  head = lgkntwk->latch_head->next;
  while (NULL != head) {
    assert(head->type == ND_LATCH);
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].timing_weight = 1 - (head->in_edges[iedge].slack/max_slack);
    }
    head = head->next;
  }
  // Update PI slacks
  head = lgkntwk->pi_head->next;
  while (NULL != head) {
    assert(head->type == ND_PI);
    for (iedge=0; iedge<head->output_num; iedge++) {
      if ((0 == iedge)||(head->slack > head->out_edges[iedge]->slack)) {
        head->slack = head->out_edges[iedge]->slack;
      }
    }
    head = head->next;
  }

  return 1; 
}

// Merge the weight of timing and depth
int merge_weights_lgkntwk(t_lgkntwk* lgkntwk,
                          float alpha,
                          int verbose) {
  t_lgknd* head;
  int iedge;
  // search PO
  head = lgkntwk->po_head->next;
  while (NULL != head) {
    assert(head->type == ND_PO);
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].weight = (1-alpha)*head->in_edges[iedge].depth_weight + alpha*head->in_edges[iedge].timing_weight;
    }
    head = head->next;
  }
  // search libgates
  head = lgkntwk->libgate_head->next;
  while (NULL != head) {
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].weight = (1-alpha)*head->in_edges[iedge].depth_weight + alpha*head->in_edges[iedge].timing_weight;
    }
    head = head->next;
  }
  // search latches
  head = lgkntwk->latch_head->next;
  while (NULL != head) {
    assert(head->type == ND_LATCH);
    for (iedge=0; iedge<head->input_num; iedge++) {
      head->in_edges[iedge].weight = (1-alpha)*head->in_edges[iedge].depth_weight + alpha*head->in_edges[iedge].timing_weight;
    }
    head = head->next;
  }
  return 1; 
}

int critical_input_paths_affect_lgknd(t_lgknd* lgknd,
                                      int verbose) {
  int i;
  
  // Update the input_paths_affect
  if (ND_LATCH != lgknd->type) {
    for (i=0; i<lgknd->input_num; i++) {
      if (0.0 == lgknd->in_edges[i].slack) {
        lgknd->input_paths_affect += lgknd->inputs[i]->input_paths_affect;
      }
    }
  } else if (0.0  == lgknd->slack) {
    lgknd->input_paths_affect = 1.0;
  }
      
  lgknd->dfsed = 1;
  for (i=0; i<lgknd->output_num; i++) {
    if (0 == lgknd->outputs[i]->dfsed) {
      lgknd->outputs[i]->input_paths_affect = 0.0;
      critical_input_paths_affect_lgknd(lgknd->outputs[i],verbose);
    }
  }

  return 1;
}

int critical_output_paths_affect_lgknd(t_lgknd* lgknd,
                                       int verbose) {
  int i;
  
  // Update the output_paths_affect
  if (ND_LATCH != lgknd->type) {
    for (i=0; i<lgknd->output_num; i++) {
      if (0.0 == lgknd->out_edges[i]->slack) {
        lgknd->output_paths_affect += lgknd->outputs[i]->output_paths_affect;
      }
    }
  } else if (0.0 == lgknd->slack) {
    lgknd->output_paths_affect = 1.0;
  }
      
  lgknd->dfsed = 1;
  for (i=0; i<lgknd->input_num; i++) {
    if (0 == lgknd->inputs[i]->dfsed) {
      lgknd->inputs[i]->output_paths_affect = 0.0;
      critical_output_paths_affect_lgknd(lgknd->inputs[i],verbose);
    }
  }

  return 1;
}

// Mark the Critical Paths affects(input and output)
// Should be called after slack the logic network
int critical_paths_affect_lgkntwk(t_lgkntwk* lgkntwk,
                                  int verbose) {
  t_lgknd* head;
  // Input Path Affect, Start from Source 
  // search PI
  head = lgkntwk->pi_head->next;
  while (NULL != head) {
    assert(head->type == ND_PI);
    if (0.0 == head->slack) { // On Critical Path
      head->input_paths_affect = 1.0;
    } else {
      head->input_paths_affect = 0.0;
    }
    reset_dfs_tags_lgkntwk(lgkntwk,verbose);
    critical_input_paths_affect_lgknd(head,verbose); 
    head = head->next;
  }
  // In case we have a constant generator
  head = lgkntwk->libgate_head->next;
  while (NULL != head) {
    if ((0 == head->input_num)&&(0.0 == head->slack)) { // On Critical Path
      head->input_paths_affect = 1.0;
    } else {
      head->input_paths_affect = 0.0;
    }
    reset_dfs_tags_lgkntwk(lgkntwk,verbose);
    critical_input_paths_affect_lgknd(head,verbose); 
    head = head->next;
  }
  // In case we ha
  // search PO
  head = lgkntwk->po_head->next;
  while (NULL != head) {
    assert(head->type == ND_PO);
    if (0.0 == head->slack) { // On Critical Path
      head->output_paths_affect = 1.0;
    } else {
      head->output_paths_affect = 0.0;
    }
    reset_dfs_tags_lgkntwk(lgkntwk,verbose);
    critical_output_paths_affect_lgknd(head,verbose); 
    head = head->next;
  }
  return 1; 
}

int initial_lgkntwk_try_pattern_types(t_lgkntwk* lgkntwk) {
  t_lgknd* head;
  // Libgates
  head = lgkntwk->libgate_head->next;
  while (NULL != head) {
    head->try_pattern_types = (int*)my_malloc((head->input_num+1)*sizeof(int));
    memset(head->try_pattern_types,0,(head->input_num+1)*sizeof(int)); 
    head = head->next;
  }
  
  return 1;
}

int reset_lgkntwk_try_pattern_types(t_lgkntwk* lgkntwk) {
  t_lgknd* head;
  // Libgates
  head = lgkntwk->libgate_head->next;
  while (NULL != head) {
    // memset should be consider the size!!!
    memset(head->try_pattern_types,0,(head->input_num+1)*sizeof(int)); 
    head = head->next;
  }
  
  return 1;
}


