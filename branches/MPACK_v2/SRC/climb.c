#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "mpack_types.h"
#include "mpack_conf.h"
#include "blearch_parser.h"
#include "mpack_util.h"
#include "check_ble_arch.h"
#include "stats.h"
#include "lgkntwk_common.h"
#include "sat_api.h"
#include "sat_solver.h"
#include "packing.h"
#include "climb.h"

/**
 * Climb Pack. Merge BLEs
 *  We have 2 cases to consider:
 *  1. Mapped logic node has two input. Predecessors are in the same BLE.
 *     We try to fill predecessors' BLE.
 *  2. If we cannot implement case 1. Search the rest BLEs, fill current BLE until we cannot. 
 */
int climb_pack(t_lgkntwk* lgkntwk,
               t_stats* stats,
               t_ble_arch* ble_arch,
               t_ble_info* ble_info_head,
               int verbose)
{
  t_ble_info* head = ble_info_head->next;
  t_ble_info* rm_node = ble_info_head;
  int cell_used_num = 0;
  int cell_idx = -1;
  int im;
  t_lgknd* cur;

  printf("Info: Start Climbing...\n");
  /*1st Traversal, handle 1st case*/
  if (1 == verbose) {
    printf("Info: Start Merging BLEs...\n");
  } 

  head = ble_info_head->next;
  while(head) {
    cell_used_num = 0;
    /*Count if there is only one cell used*/
    for (im=0; im<head->blend_num; im++) {
      if (1 == head->blend_used[im]) {
        cell_used_num++;
        cell_idx = im;
      }
    }
    /*Simple Check number of used cell*/
    if (0 == cell_used_num) {
      printf("Error: Blank BLE found in Climbing!\n");
      exit(1);
    }
    else if (1 == cell_used_num) {
      /*Find a 1-cell used BLE*/
      cur = head->blend_lgknds[cell_idx];
      if ((1 < cur->input_num)&&(1 == join_preds_ble(lgkntwk,ble_arch,cur,head,cell_idx))) {
        if (1 == verbose) {
          //printf("Merge success!\n");
        }
        free_ble_info(head);
        remove_ble_list_node(rm_node);
        head = rm_node->next; 
      }      
      else {
        rm_node = head;
        head = head->next;
      }
    }
    else {
      rm_node = head;
      head = head->next;
    }
  }

  head = ble_info_head->next;
  while(head) {
    cell_used_num = 0;
    /*Count if there is only one cell used*/
    for (im=0; im<head->blend_num; im++) {
      if (1 == head->blend_used[im]) {
        cell_used_num++;
        cell_idx = im;
      }
    }
    /*Simple Check number of used cell*/
    if (0 == cell_used_num) {
      printf("Error: Blank BLE found in Climbing!\n");
      exit(1);
    }
    else if (1 == cell_used_num) { 
      /*Find a 1-cell used BLE*/
      cur = head->blend_lgknds[cell_idx];
      if (1 == join_preds_ble(lgkntwk,ble_arch,cur,head,cell_idx)) {
        if (1 == verbose) {
          //printf("Merge success!\n");
        }
        free_ble_info(head);
        remove_ble_list_node(rm_node);
        head = rm_node->next; 
      }      
      else {
        rm_node = head;
        head = head->next;
      }
    }
    else {
      rm_node = head;
      head = head->next;
    }
  }

  if (1 == verbose) {
    printf("Info: Start Filling BLEs...\n");
  } 
  head = ble_info_head->next;
  /*2nd Traversal, handle 2nd case*/
  while(head) {
    cell_used_num = 0;
    /*Count if there is only one cell used*/
    for (im=0; im<head->blend_num; im++) {
      if (1 == head->blend_used[im]) {
        cell_used_num++;
        cell_idx = im;
      }
    }
    /*Simple Check number of used cell*/
    if (0 == cell_used_num) {
      printf("Error: Blank BLE found in Climbing!\n");
      exit(1);
    }
    //else if (1 == cell_used_num) { 
    else if (0 < cell_used_num) { 
      /*Find a 1-cell used BLE*/
      //fill_ble(lgkntwk,ble_arch,head);
      new_fill_ble(lgkntwk,ble_arch,head,verbose);
    }
    head = head->next;
  }

  printf("Info: After climbing, %d BLE are used.\n",stats_ble_num(ble_arch,ble_info_head,verbose));

  return 1;
}

/**
 * Join Predecessor's BLE.
 * If logic node's predecessors are in the same BLE, try to join them.
 */
int join_preds_ble(t_lgkntwk* lgkntwk,
                   t_ble_arch* ble_arch,
                   t_lgknd* cur,
                   t_ble_info* ble_info,
                   int cell_idx)
{
  int im;
  int ip;
  int ret = 1;
  t_ble_info* pred_ble = NULL;
  int* pred_idxes = (int*)my_malloc(sizeof(int)*cur->input_num);
  t_ble_info pred_ble_mirror;
  int join_fail;
  int latch_mapped;
  t_ble_info* origin = cur->ble_info;

  init_ble_info(ble_arch,&pred_ble_mirror);

  for (im=0; im<cur->input_num; im++) {
    pred_idxes[im] = -1;
  }

  /*Step 1: Check predecessors are in the same BLE*/
  for (im=0; im<cur->input_num; im++) {
    if ((ND_PI == cur->inputs[im]->type)||(ND_LATCH == cur->inputs[im]->type)) {
      return 0;
    }
    if (0 == im) { 
      pred_ble = (t_ble_info*)cur->inputs[im]->ble_info;
    }
    else if (pred_ble != (t_ble_info*)cur->inputs[im]->ble_info) {
      ret = 0; 
      break;
    }
  }
  if (1 != ret) {
    return 0;
  }
  /*Record predecessors' cell index*/
  for (im=0; im<cur->input_num; im++) {
    for (ip=0; ip<ble_arch->blend_num; ip++) {
      if ((1 == pred_ble->blend_used[ip])&&(pred_ble->blend_lgknds[ip] == cur->inputs[im])) {
        pred_idxes[im] = ip;
      }
    }
  }
  for (im=0; im<cur->input_num; im++) {
    if (-1 == pred_idxes[im]) {
      printf("Error: pre_idxes has empty element!(Join Preds BLE)\n");
      exit(1);
    }
  }
  /*Temporary change ble_info pointer*/
  cur->ble_info = pred_ble; 

  /*Step 2: Try to join them, try each un-used cell*/
  for (im=0; im<ble_arch->blend_num; im++) {
    copy_ble_info(ble_arch,pred_ble,&pred_ble_mirror);
    if (0 == pred_ble_mirror.blend_used[im]) {
      /*Generate a copy of pred_ble_info*/
      join_fail = 0;
      /*TODO: We should check if there is conflicts*/
      /*Try map BLE output*/
      if (0 == sat_cell_output(ble_arch,&pred_ble_mirror,im,cur,&latch_mapped)) {
        continue;
      }
      /*Try each inputs*/
      for (ip=0; ip<cur->input_num; ip++) {
        if (0 == sat_cell_in_cell_out_conkt(ble_arch,&pred_ble_mirror,im,pred_idxes[ip],cur)) {
          continue;
        }
        remap_cell_ble_output(ble_arch,&pred_ble_mirror,cur,pred_idxes[ip]);
      }
      /*Free BLE output of predecessors if we need*/
      if (0 == mark_lgknd_outputs_mapped(lgkntwk,cur,pred_ble->idx,0,pred_ble)) {
        continue;
      }
      /*Reaching here means success, copy results and return*/
      cur->ble_idx = pred_ble->idx;
      copy_ble_info(ble_arch,&pred_ble_mirror,pred_ble);
      free_ble_info(&pred_ble_mirror);
      return 1;
    }
  }
 
  /*Reach here means we failed*/
  /*Recover*/
  cur->ble_info = origin; 
  //free_ble_info(&pred_ble_mirror);

  return 0;
}


int fill_ble(t_lgkntwk* lgkntwk,
             t_ble_arch* ble_arch,
             t_ble_info* ble_info)
{
  int ible;
  int ble_used;
  t_lgknd* cur;
  t_ble_info* tmp = ble_info->next;
  t_ble_info* rm_node = ble_info;
  t_ble_info ble_info_mirror;
  int done = 0;
  int full = 0; 
  int idx_backup;

  init_ble_info(ble_arch,&ble_info_mirror);

  while(tmp) {
    done = 0;
    /*Check if tmp(BLE_info) has only one Cell used*/
    ble_used = 0;
    for (ible=0; ible<tmp->blend_num; ible++) {
      if (1 == tmp->blend_used[ible]) {
        ble_used++;
        cur = tmp->blend_lgknds[ible];
      }
    }
    if (0 == ble_used) {
      printf("Error: Zero Cell used in a BLE!(fill_ble)\n");
      exit(1);
    }
    else if (1 == ble_used) {
      /*Check we have available position*/
      full = 1;
      for (ible=0; ible<ble_info->blend_num; ible++) {
        if ((0 == ble_info->blend_used[ible])) {
          full = 0;
        }
      }
      if (1 == full) {
        return 1;
      }
      /*Try to fill it*/
      for (ible=0; ible<ble_arch->blend_num; ible++) {
        done = 0;
        if ((0 == ble_info->blend_used[ible])) {
          copy_ble_info(ble_arch,ble_info,&ble_info_mirror);
          idx_backup = ble_info->idx;
//&&(1 == check_independent_cell(ble_arch,ble_info,ible,cur->input_num))) {
          /*It seems we find a proper place, try to map it with found ble in tmp*/ 
          if (1 == map_one_cell(ble_arch,&ble_info_mirror,ible,cur)) {
            /*Success, assign ble_info pointer, free tmp ble_info, remove it from linked list*/
            if (1 == mark_lgknd_outputs_mapped(lgkntwk,cur,idx_backup,0,&ble_info_mirror)) {
              done = 1;
              copy_ble_info(ble_arch,&ble_info_mirror,ble_info);
              cur->ble_idx = idx_backup;
              cur->ble_info = ble_info;
              free_ble_info(tmp);
              remove_ble_list_node(rm_node);
              tmp = rm_node->next;
              break;
            }
          }
        }
      }
    }
    if (0 == done) {
      rm_node = tmp;
      tmp = tmp->next;
    }
  }
  free_ble_info(&ble_info_mirror);

  return 1;
}



/**
 * Count the number of used cell in a BLE
 * Return the number of used cell,
 * If there is only one cell used, its index is stored in cell_idx
 */
int count_cell_used(t_ble_info* ble_info,
                    int* cell_idx)
{
  int cell_used_num = 0;
  int im = 0;

  /*Count if there is only one cell used*/
  for (im=0; im<ble_info->blend_num; im++) {
    if (1 == ble_info->blend_used[im]) {
      cell_used_num++;
      (*cell_idx) = im;
    }
  }
  /*Simple Check number of used cell*/
  if (0 == cell_used_num) {
    printf("Error: Blank BLE found in Climbing!\n");
    exit(1);
  }
  return cell_used_num;
}

/**
 * TODO: Try to join a BLE 
 */
int try_join_ble(t_lgkntwk* lgkntwk,
                 t_ble_arch* ble_arch,
                 t_lgknd* cur,
                 t_ble_info* ble_info)
{
  int ret = 0;
  t_ble_info ble_info_mirror;
  int im = 0;

  init_ble_info(ble_arch,&ble_info_mirror);
  /*Try each un-used cell*/
  for (im=0; im<ble_arch->blend_num; im++) {
    if (0 == ble_info->blend_used[im]) {
      /*Generate a copy of pred_ble_info*/
      copy_ble_info(ble_arch,ble_info,&ble_info_mirror);
      if (0 == map_one_cell(ble_arch,&ble_info_mirror,im,cur)) {
        //free_ble_info(&ble_info_mirror);
        continue;
      }
      if (0 == mark_lgknd_outputs_mapped(lgkntwk,cur,ble_info->idx,0,&ble_info_mirror)) {
        //free_ble_info(&ble_info_mirror);
        continue;
      }
      /*Reaching here means success, copy results and return*/
      //cur->ble_idx = ble_info->idx;
      //cur->ble_info = ble_info;
      copy_ble_info(ble_arch,&ble_info_mirror,ble_info);
      free_ble_info(&ble_info_mirror);
      //free_ble_info(&ble_info_mirror);
      return 1;
    }
  }

  free_ble_info(&ble_info_mirror);
  return ret;
}

/**
 * A similar implementation of hill climbing as VPACK does
 * We try to replace it with fill_ble
 */
int new_fill_ble_once(t_lgkntwk* lgkntwk,
                      t_ble_arch* ble_arch,
                      t_ble_info* ble_info,
                      int verbose)
{
  t_ble_info* tmp = ble_info->next;
  t_ble_info* pred_tmp = ble_info;
  t_ble_info* rm_node = ble_info;
  t_lgknd* cur;
  t_lgknd* last_cur = NULL;
  t_ble_info* last_cur_ble = NULL;
  int used_cell_num;
  int cell_idx;
  t_ble_info ble_info_mirror;
  t_ble_info ble_info_final;
  float attract = -1.0;
  float tmp_attract = -1.0;
  int ret = 0;

  //copy_ble_info(ble_arch,ble_info,&ble_info_mirror);
  //copy_ble_info(ble_arch,&ble_info_mirror,&ble_info_final);
  init_ble_info(ble_arch,&ble_info_mirror);
  init_ble_info(ble_arch,&ble_info_final);

  while(tmp) {
    /*Check each BLE has more than one cell used*/ 
    used_cell_num = count_cell_used(tmp,&cell_idx);
    if (0 == used_cell_num) {
      printf("Error: Blank BLE found in Climbing!\n");
      exit(1);
    } 
    else if (1 == used_cell_num) {
      /*Calculate Attractness*/
      cur = tmp->blend_lgknds[cell_idx];
      tmp_attract = ble_lgknd_attract(ble_info,cur);
      if ((-1.0 == attract)||(tmp_attract > attract)) { 
        //free_ble_info(&ble_info_mirror);
        copy_ble_info(ble_arch,ble_info,&ble_info_mirror);
        if (1 == try_join_ble(lgkntwk,ble_arch,cur,&ble_info_mirror)) {
          if (1 == verbose) {
            printf("Join: cur(%d) from BLE(%d) to BLE(%d)...\n",cur->idx,cur->ble_idx,ble_info->idx);
          }
          if (NULL != last_cur) {
            if (1 == verbose) {
              printf("Recover: last_cur(%d) to last_cur_ble(%d)...\n",last_cur->idx,last_cur_ble->idx);
            }
            last_cur->ble_info = last_cur_ble;
            last_cur->ble_idx = last_cur_ble->idx;
            if (0 == mark_lgknd_outputs_mapped(lgkntwk,last_cur,last_cur_ble->idx,0,last_cur_ble)) {
              printf("Error: Fail to recover last_cur BLE!\n");
              exit(1);
            }
          }
          //free_ble_info(&ble_info_final);
          copy_ble_info(ble_arch,&ble_info_mirror,&ble_info_final);
          attract = tmp_attract;
          rm_node = pred_tmp; 
          last_cur = cur;
          last_cur_ble = cur->ble_info;
          ret = 1;
        }
      }
    }
    pred_tmp = tmp;
    tmp = tmp->next;
  }

  if (1 == ret) {
    if (1 == verbose) {
      printf("Libgate(%d) join BLE(%d)...Success(Attract:%f)\n",last_cur->idx,ble_info->idx,attract);
    }
    copy_ble_info(ble_arch,&ble_info_final,ble_info);
    last_cur->ble_info = ble_info;
    last_cur->ble_idx = ble_info->idx;
    remove_ble_list_node(rm_node);
  }  
  
  free_ble_info(&ble_info_final);
  free_ble_info(&ble_info_mirror);
  //free_ble_info(&ble_info_mirror);

  return ret;
}


int new_fill_ble(t_lgkntwk* lgkntwk,
                 t_ble_arch* ble_arch,
                 t_ble_info* ble_info,
                 int verbose)
{
  int full = 0;
  int tmp_cell_idx;

  if (1 == verbose) {
    printf("Info: Climb BLE(%d) Begin...\n",ble_info->idx);
  } 

  while(1) { 
    /*Check we have available position*/
    full = 1;
    if (ble_info->blend_num != count_cell_used(ble_info,&tmp_cell_idx)) {
      full = 0;
    }
    if (1 == full) {
      break;
    }
    else if (0 == new_fill_ble_once(lgkntwk,ble_arch,ble_info,verbose)) {
      break;
    }
  }

  if (1 == verbose) {
    printf("Info: Climb BLE(%d) Over...\n",ble_info->idx);
  } 

  return 1;
}
