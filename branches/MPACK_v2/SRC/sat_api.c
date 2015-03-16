#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "mpack_types.h"
#include "mpack_conf.h"
#include "blearch_parser.h"
#include "mpack_util.h"
#include "lgkntwk_common.h"
#include "global.h"
#include "check_ble_arch.h"
#include "stats.h"
#include "sat_api.h"

/*Return the number of used BLE outputs*/
int count_used_ble_outputs(t_ble_info* ble_info,
                           int verbose)
{
  int im;
  int ret = 0;

  for (im=0; im<ble_info->output_num; im++) {
    if (1 == ble_info->output_used[im]) {
      ret++;
    }
  }

  return ret;
}

/* Return the number of used BLE cell
 */
int count_used_blend(t_ble_info* ble_info,
                     int verbose)
{
  int im;
  int ret = 0;

  for (im=0; im<ble_info->blend_num; im++) {
    if (1 == ble_info->blend_used[im]) {
      ret++;
    }
  }

  return ret;
}

/**
 * Get the cell index of given logic node in a BLE
 */
int spot_lgknd_ble_cell_idx(t_ble_info* ble_info,
                            t_lgknd* cur)
{
  int ret = -1;
  int im; 
 
  for (im=0; im < ble_info->blend_num; im++) {
    if (cur == ble_info->blend_lgknds[im]) {
      ret = im;
      return ret;
    }
  }

  return ret;
}

/**
 * Get the output latch index of given logic node in a BLE
 * Mode latch_only: only search sequential outputs of BLE
 */
int spot_lgknd_ble_output_idx(t_ble_info* ble_info,
                              t_lgknd* cur,
                              int latch_only)
{
  int ret = -1;
  int im; 
 
  if ((1 == latch_only)&&(ND_LATCH != cur->type)) {
    //printf("Warning: Logic node type in spot_latch_ble_output_idx should be LATCH!\n");
    return ret;
  } 

  for (im=0; im < ble_info->output_num; im++) {
    if (cur == ble_info->output_lgknds[im]) {
      if ((1 == latch_only)&&(0 != ble_info->output_types[im])) {
        printf("Error: Output types is combinational but corresponding logic node is a latch!\n");
        exit(1);
      }
      ret = im;
      return ret;
    }
  }

  return ret;
}

/* Check if the lgknd's inputs require remapping
 * Check outputs(index = output_idx) of sat_lgknd if it has
 * connection to mapped lgknds in ble_info.
 * Return 1 if need remapping
 * Return 0 if do not need
 */
int check_lgknd_input_remap(t_ble_arch* ble_arch,
                            t_ble_info* ble_info,
                            t_lgknd* sat_lgknd,
                            int output_idx,
                            int* remap_cells,
                            int verbose)
{
  int ret = 0;
  int i,j,tmp_mux,input_idx;
  
  /*Check other BLE cell share inputs to sat_lgknds*/
  for (i=0; i<ble_info->blend_num; i++) {
    if (1 == ble_info->blend_used[i]) {
      for (j=0; j<ble_arch->blends[i].input_num; j++) {
        tmp_mux = ble_info->blend_muxes[i][j];
        if ((1 == ble_info->blend_port_used[i][j])&&(BLE_IN == ble_arch->blends[i].inputs[j].cands[tmp_mux]->type)) {
          input_idx = ble_arch->blends[i].inputs[j].cands[tmp_mux]->idx;
          if (sat_lgknd == ble_info->input_lgknds[input_idx]->inputs[ble_info->input_lgknds_input_idx[input_idx]]) {
            remap_cells[i] = 1;
          }
        }
      }
    }
  }

  if (ND_LATCH == sat_lgknd->outputs[output_idx]->type) {
    for (i=0; i < sat_lgknd->outputs[output_idx]->output_num; i++) {
      check_lgknd_input_remap(ble_arch,ble_info,sat_lgknd->outputs[output_idx],i,remap_cells,verbose);
    }
  }
   
  /*
  if (ND_LATCH != sat_lgknd->outputs[output_idx]->type) {
    if ((ND_PO != sat_lgknd->outputs[output_idx]->type)
      &&(ble_info == sat_lgknd->outputs[output_idx]->ble_info)
      &&(sat_lgknd->outputs[output_idx] == 
         ble_info->blend_lgknds[sat_lgknd->outputs[output_idx]->ble_cell_idx])) {
      remap_cells[sat_lgknd->outputs[output_idx]->ble_cell_idx] = 1;
    }
  }
  else if (ND_LATCH == sat_lgknd->outputs[output_idx]->type) {
  */
    /*Search the outputs of latch*/
  /*
    for (i = 0; i < sat_lgknd->outputs[output_idx]->output_num; i++) {
      check_lgknd_input_remap(ble_arch,ble_info,sat_lgknd->outputs[output_idx],i,remap_cells,verbose);
    }
  }
  */

  return ret;
}

/**
 * Allocate an output in BLE for the cell chosen
 * 1. For a combinational request we can allocate a combinational or sequential output
 * 2. For a sequential request, we have only one choice
 * In this function, we mark flags: blend_used, blend_lgknds, output_lgknds,
 * output_used, output_type, output_muxes
 * Return 0 if failed
 * Return 1 if success
 */
int sat_ble_out(t_ble_arch* ble_arch,
                t_ble_info* ble_info,
                int is_comb,
                int cell_idx,
                t_lgknd* cur,
                t_lgknd* latch)
{
  int im;
  int icnd;
  int out_idx = -1;
  int conkt = -1;
  int cost = -1;
  int tmpcost = -1;
  int penalty = 2; // Penalty for using sequential output as combinational

  //if (1 == ble_info->blend_used[cell_idx]) {
  //  printf("Error: Intend to re-connect output of used BLE cell%d!\n",cell_idx);
  //  exit(1);
  //}
  
  if (DEBUG) {
    printf("DEBUG: Try to find ble_out(comb:%d) for cell%d.\n",is_comb,cell_idx);}

  for (im=0; im<ble_arch->output_num; im++) {
    for (icnd=0; icnd<ble_arch->outputs[im].cand_num; icnd++) {
      /*Match: 1. Index 2. Type(is_comb)*/
      if ((0 == ble_info->output_used[im])&&(cell_idx == ble_arch->outputs[im].cands[icnd]->blend_idx)) {
        tmpcost = ble_arch->outputs[im].cand_num;
        if (is_comb != ble_arch->outputs[im].is_comb) {
          tmpcost = tmpcost*penalty;
        }
        if (DEBUG) {
          printf("DEBUG: map_BLE_out,Update out[%d] tmpcost(%d)\n",im,tmpcost);}
        /*Update connection and cost*/
        if ((-1 == cost)||(cost > tmpcost)) {
          if (DEBUG) {
            printf("DEBUG: map_BLE_out,Update cost(from:%d,to:%d)\n",cost,tmpcost);}
          cost = tmpcost;
          conkt = icnd; 
          out_idx = im;
        }
      }
    } 
  }

  /*Find a connection!*/
  if (conkt != -1) {
    if (DEBUG) {
      printf("DEBUG: find ble_output[%d],for cell%d.\n",out_idx,cell_idx);
    }
    ble_info->output_used[out_idx] = 1; 
    ble_info->output_muxes[out_idx] = conkt; 
    ble_info->output_types[out_idx] = is_comb;
    if (1 == is_comb) {
      ble_info->output_lgknds[out_idx] = cur; 
    } else {
      ble_info->output_lgknds[out_idx] = latch; 
    }  
    return 1;
  }

  return 0;
}

/**
 * SAT output connections of a cell in BLE
 * Allocate an output port of BLE if necessry
 * If a sequential output cannot be satisfied, a combinational output is allocated instead.
 * In ble_info, output_lgknds[i] depicts which logic node is mapped to this output.
 * For example, output_lgknds[i] is a latch when a seqential output is allocated
 * Special parameter: comb_num 
 * The number of internal combinational output we already have. In most case, it equals to 1.
 * Return 1 if satisfied
 * Return 0 if failed
 */
int sat_lgknd_outputs(t_ble_arch* ble_arch,
                     t_ble_info* ble_info,
                     int cell_idx,
                     t_lgknd* cur,
                     int comb_num,
                     int verbose)
{
  int is_comb = 0;
  int is_seq = 0;
  int im;
  int ret = 1;
  t_lgknd* latch = NULL;

  /* Check if cell has combinational output, 
   * sequential output, or primary output
   * Primary output or combinational output requires a combinational output.
   * A latch requires a sequential output
   */ 
  for (im=0; im<cur->output_num; im++) {
    if (ND_LATCH != cur->outputs[im]->type) {
      is_comb++;
    }
    else if ((ND_LATCH == cur->outputs[im]->type)&&(0 == cur->outputs[im]->mapped)) {
      if (1 == is_seq) {
        /*TODO: This should be checked before packing.*/
        printf("Warning: Fanout includes two latches! Mpack merge them...\n");
      } else {
        is_seq = 1;
        latch = cur->outputs[im];
      }
    }
  }
 
  /*Find outputs available in BLE*/
  if (0 == (is_comb + is_seq)) {
    printf("Warning: Logic node has no output! None BLE output will be assigned.\n");
  }

  /*We should consider internal combinational connection*/
  is_comb = is_comb - comb_num;

  if (0 > is_comb) {
    printf("Error:(sat_lgknd_outputs)Invalid is_comb = %d!\n",is_comb);
    exit(1);
  }

  /* Updates: even there is no need, we allocate a combinational output for routability
   *          redundant ports
  */

  if ((0 == is_seq)&&(0 == is_comb)) {
    if ((0 == sat_ble_out(ble_arch,ble_info,1,cell_idx,cur,NULL))) {
      return 0;
    } else {
      ble_info->blend_used[cell_idx] = 1;
      ble_info->blend_lgknds[cell_idx] = cur;
      return 1;
    }
  }

  // Assert that buf should have zero combinational output
  if (ND_BUF == cur->type) {
    assert(0 == is_comb);
  }

  if ((0 < is_comb)&&(0 == sat_ble_out(ble_arch,ble_info,1,cell_idx,cur,NULL))) {
    ret = 0;
  }
  if ((0 < is_seq)&&(0 == sat_ble_out(ble_arch,ble_info,0,cell_idx,cur,latch))) {
    /*Try to allocate a seqential output*/
    // For buffer, if latch cannot be mapped, we should fail it. Or it creates additional buffers
    if (ND_BUF == cur->type) {
      ret = 0;
    } else if ((1 == ret)&&(0 < is_comb)) { // we have alloced combinational output already.
      ret = 1;
    } else if (0 == sat_ble_out(ble_arch,ble_info,1,cell_idx,cur,NULL)) {
      /*Try to allocate a combinational output*/
      ret = 0;
    }
  }

  if (1 == ret) {
    ble_info->blend_used[cell_idx] = 1;
    ble_info->blend_lgknds[cell_idx] = cur;
  }
  
  return ret;
}

/**
 * Establish a connection from source bleport to des bleport
 * Return index of MUX inputs if success
 * Return -1 if fail
 */
int sat_src_port_des_port(t_ble_arch* ble_arch,
                          t_ble_info* ble_info,
                          t_bleport* src_port,
                          t_bleport* des_port)
{
  int ret = -1;
  int icnd = 0;

  if (src_port == des_port) {
    printf("Warning: Intend to conduct self connection! (src_port == des_port)\n");
    return ret;
  }
  //if (-1 == src_port->blend_idx) {
  //  printf("Warning: Source port does not belong to a BLE cell!\n");
  //  return ret;
  //}
  for (icnd=0; icnd<src_port->cand_num; icnd++) {
    if (des_port == src_port->cands[icnd]) {
      ret = icnd;
      return ret;
    }
  }

  return ret;
}


/**
 * Route a cell's port to given target logic node 
 * Return the index of MUX inputs if success
 * Return -1 if failed
 */
int sat_cell_port_target_lgknd(t_ble_arch* ble_arch,
                               t_ble_info* ble_info,
                               t_bleport* src_port,
                               t_lgknd* targ)
{
  int ret = -1;
  int im = 0;
  int pred_idx = spot_lgknd_ble_cell_idx(ble_info,targ);
  int out_idx = spot_lgknd_ble_output_idx(ble_info,targ,0);
  t_bleport* targ_port = NULL;
  
  /*When the target logic node is inside BLE*/      
  if (-1 != pred_idx) {
    targ_port = &ble_arch->blends[pred_idx].inputs[0];
    ret = sat_src_port_des_port(ble_arch,ble_info,src_port,targ_port);
    if (-1 != ret) {
      return ret;
    }
  }
  if (-1 != out_idx) {
    targ_port = &ble_arch->outputs[out_idx];
    ret = sat_src_port_des_port(ble_arch,ble_info,src_port,targ_port);
    if (-1 != ret) {
      return ret;
    }
  }
  if (((-1 != pred_idx)||(-1 != out_idx))&&(-1 == ret)) {
    return ret;
  }
  /*Target node is not inside BLE, a BLE input should be allocated*/
  /*First we try to share a used BLE input*/
  for (im=0; im<ble_info->input_num; im++) {
    if ((1 == ble_info->input_used[im])&&(targ == ble_info->input_lgknds[im]->inputs[ble_info->input_lgknds_input_idx[im]])) {
      ret = sat_src_port_des_port(ble_arch,ble_info,src_port,&ble_arch->inputs[im]);
      if (-1 != ret) {
        return ret;
      }
    }
  }
  /*If we cannot share, allocate a new input*/
  for (im=0; im<ble_info->input_num; im++) {
    if (0 == ble_info->input_used[im]) {
      ret = sat_src_port_des_port(ble_arch,ble_info,src_port,&ble_arch->inputs[im]);
      if (-1 != ret) {
        return ret;
      }
    }
  }

  /*If we still cannot find, return fail*/
  ret = -1;
 
  return ret;
}

/**
 * Route a logic node input on a cell port
 * Search all inputs of the cell if all inputs are logical equivalent
 * Or only corresponding cell input is routed. 
 * Return 1 if success
 */
int sat_lgknd_input_cell_port(t_ble_arch* ble_arch, 
                              t_ble_info* ble_info,
                              int cell_idx,
                              t_lgknd* lgknd,
                              int lgknd_input_idx,
                              int logic_equivalent)
{
  int ret = 0;
  int im = 0;
  
  int cell_input_idx = -1;
  int tmp_cell_input_idx = -1;
  int rst = -1;
  int tmp_rst = -1;
  int cost = -1;
  int tmp_cost = -1;

  t_lgknd* targ = NULL;
  t_bleport* src_port = NULL;
 
  if ((lgknd_input_idx < lgknd->input_num)&&(-1 < lgknd_input_idx)) {
    targ = lgknd->inputs[lgknd_input_idx];
  }
  else {
    printf("Warning: Invalid lgknd_input_idx(%d)!\n",lgknd_input_idx);
  }
  
  if (1 == logic_equivalent) {
    for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
      if (0 == ble_info->blend_port_used[cell_idx][im]) {
        src_port = &ble_arch->blends[cell_idx].inputs[im]; 
        /*Find the port connection*/
        tmp_rst = sat_cell_port_target_lgknd(ble_arch,ble_info,src_port,targ);
        if (-1 != tmp_rst) {
          tmp_cell_input_idx = im;
          tmp_cost = src_port->cand_num;
          /*Saving cost for internal connections between BLE cells*/
          if (BLE_ND == src_port->cands[tmp_rst]->type) {
            tmp_cost = 0.1*tmp_cost;
          }
          if ((-1 == cost)||(tmp_cost < cost)) {
            ret = 1;
            rst = tmp_rst;
            cost = tmp_cost;
            cell_input_idx = tmp_cell_input_idx;
          }
        } 
      }
    }
  }
  else {
    src_port = &ble_arch->blends[cell_idx].inputs[lgknd_input_idx]; 
    /*Find the port connection*/
    tmp_rst = sat_cell_port_target_lgknd(ble_arch,ble_info,src_port,targ);
    if (-1 != tmp_rst) {
      ret = 1;
      rst = tmp_rst;
      cell_input_idx = lgknd_input_idx;
    }
  }    
  /*Record the results and route*/
  if (1 == ret) {
    ble_info->blend_port_used[cell_idx][cell_input_idx] = 1;
    ble_info->blend_muxes[cell_idx][cell_input_idx] = rst;
    ble_info->blend_lgknd_input_idxes[cell_idx][cell_input_idx] = lgknd_input_idx;
    /*If a new input is allocated*/
    src_port = &ble_arch->blends[cell_idx].inputs[cell_input_idx]; 
    if ((BLE_IN == src_port->cands[rst]->type)&&(0 == ble_info->input_used[src_port->cands[rst]->idx])) {
      im = src_port->cands[rst]->idx;
      ble_info->input_used[im] = 1;
      ble_info->input_lgknds[im] = lgknd;
      ble_info->input_lgknds_input_idx[im] = lgknd_input_idx;
    }
  }

  return ret;
}

/**
 * Route redundant cell port
 * Rules:
 * Redundant cell port should be connected to
 * 1. A sequential output
 * 2. The same source of a regular logic node input
 * Mode Strict: redundant ports should share the same inputs
 */
int sat_cell_redundant_ports(t_ble_arch* ble_arch, 
                             t_ble_info* ble_info,
                             int cell_idx,
                             t_lgknd* lgknd,
                             int strict,
                             int clb_logic_equal,
                             int verbose)
{
  int ret = 0;
  int im,j = 0;
  int ilgknd = 0;
  int rst = -1;
  t_bleport* des_port = NULL;
  /*We have a list of candidate ports which redundant port can be connected to*/
  int cand_lgknd_num = 0;
  /*Critical Updates: we don't construct a bleport array, instead target lgknds are used*/
  t_lgknd** cand_lgknds = NULL;
  int redundant_port_num = ble_arch->blends[cell_idx].input_num - lgknd->input_num;
  int* ports_muxes = NULL;
  int cur_redund = 0;
  int input_idx = -1;
 
  /*DEBUG*/
  //if ((ble_info->idx == 1)&&(lgknd->idx == 5)) {
  //  printf("DEBUG\n");
  //} 

  /*Check if there is a need*/
  if (0 == redundant_port_num) {
    //printf("Info: no redundant ports require to be routed(BLE idx:%d, BLE cell:%d).\n",ble_info->idx,cell_idx);
    return 1;
  }
  if (0 > redundant_port_num) {
    printf("Error: Logic node input number is less than BLE cell input number(BLE idx:%d, BLE cell:%d, logic node input number:%d).\n",ble_info->idx,cell_idx,lgknd->input_num);
    exit(1);
  }

  cand_lgknd_num = 0;
  /*Count the length of candidate ports*/
  /*1. Count the number of used input by un-redundant ports */
  for (im=0; im<ble_info->input_num; im++) {
    if ((1 == ble_info->input_used[im])&&(lgknd == ble_info->input_lgknds[im])) {
      cand_lgknd_num++;
    }
  }
  /*2. Count the number of used sequential outputs*/
  for (im=0; im<ble_info->output_num; im++) {
    if ((0 == ble_info->output_types[im])&&(NULL != ble_info->output_lgknds[im])) {
      cand_lgknd_num++;
    }
  }
  /*3. Count the number of output_lgknds that equals to lgknd's inputs*/
  for (im=0; im<ble_info->output_num; im++) {
    for (j=0; j<lgknd->input_num; j++) {
      if (lgknd->inputs[j] == ble_info->output_lgknds[im]) {
        cand_lgknd_num++;
      }
    }
  }
  /*4. Count the number of BLE_ND ports that equals to lgknd's inputs*/
  for (im=0; im<ble_info->blend_num; im++) {
    for (j=0; j<lgknd->input_num; j++) {
      if (lgknd->inputs[j] == ble_info->blend_lgknds[im]) {
        cand_lgknd_num++;
      }
    }
  }

  /*Allocate the candidate port list*/
  cand_lgknds = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*cand_lgknd_num);
  /*Allocate the muxes for redundant ports*/
  ports_muxes = (int*)my_malloc(sizeof(int)*redundant_port_num);
  /* Fill the lgknds list, that redundant ports can connect to.
   * 1. First part is the ports which non-redundant ports connected to.
   * 2. Second part is the sequential outputs.  
   */
  ilgknd = 0;
  /*Priority: 1. Search for used sequential output*/
  for (im=0; im<ble_info->output_num; im++) {
    if ((0 == ble_info->output_types[im])&&(NULL != ble_info->output_lgknds[im])) {
      cand_lgknds[ilgknd] = ble_info->output_lgknds[im];
      ilgknd++;
    }
  }
  /*Priority: 2. Search any inputs of current logic node*/
  for (im=0; im<ble_info->input_num; im++) {
    if ((1 == ble_info->input_used[im])&&(lgknd == ble_info->input_lgknds[im])) {
      cand_lgknds[ilgknd] = lgknd->inputs[ble_info->input_lgknds_input_idx[im]]; 
      ilgknd++;
    }
  }
  /*3. Count the number of output_lgknds that equals to lgknd's inputs*/
  for (im=0; im<ble_info->output_num; im++) {
    for (j=0; j<lgknd->input_num; j++) {
      if (lgknd->inputs[j] == ble_info->output_lgknds[im]) {
        cand_lgknds[ilgknd] = ble_info->output_lgknds[im];
        ilgknd++;
      }
    }
  }
  /*4. Count the number of BLE_ND ports that equals to lgknd's inputs*/
  for (im=0; im<ble_info->blend_num; im++) {
    for (j=0; j<lgknd->input_num; j++) {
      if (lgknd->inputs[j] == ble_info->blend_lgknds[im]) {
        cand_lgknds[ilgknd] = ble_info->blend_lgknds[im];
        ilgknd++;
      }
    }
  }  

  my_init_int_ary(redundant_port_num,ports_muxes,-1); 
  
  /*Mode Strict off: Solve redundant ports one by one*/ 
  if (1 != strict) {
    for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
      if (0 == ble_info->blend_port_used[cell_idx][im]) {
        for (ilgknd=0; ilgknd<cand_lgknd_num; ilgknd++) {
          des_port = &ble_arch->blends[cell_idx].inputs[im];
          /*Try to find an legal routing solution*/
          rst = sat_cell_port_target_lgknd(ble_arch,ble_info,des_port,cand_lgknds[ilgknd]);
          if (-1 != rst) {
            /*Ensure it does not give a new input!!!*/
            if (BLE_IN == ble_arch->blends[cell_idx].inputs[im].cands[rst]->type) {
              input_idx = ble_arch->blends[cell_idx].inputs[im].cands[rst]->idx;
              if (1 == ble_info->input_used[input_idx]) {
                ble_info->blend_port_used[cell_idx][im] = 1;
                ble_info->blend_muxes[cell_idx][im] = rst;
                ble_info->blend_lgknd_input_idxes[cell_idx][im] = -1;
                break;
              }
            }
            else {
              ble_info->blend_port_used[cell_idx][im] = 1;
              ble_info->blend_muxes[cell_idx][im] = rst;
              ble_info->blend_lgknd_input_idxes[cell_idx][im] = -1;
              break;
            }
          }
        } 
      }
    }
    ret = 1;
    for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
      if ((0 == ble_info->blend_port_used[cell_idx][im])||(-1 == ble_info->blend_muxes[cell_idx][im])) {
        ret = 0;
      }
    } 
    if (1 == ret) {
      /*Don't forget free resources!!!*/
      free(ports_muxes);
      free(cand_lgknds);
      return ret;
    }
  }
  /*Mode Strict on: Solve redundant ports one by one*/ 
  else {
    for (ilgknd=0; ilgknd<cand_lgknd_num; ilgknd++) {
      cur_redund = 0;
      for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
        if (0 == ble_info->blend_port_used[cell_idx][im]) {
          des_port = &ble_arch->blends[cell_idx].inputs[im];
          /*Try to find an legal routing solution*/
          rst = sat_cell_port_target_lgknd(ble_arch,ble_info,des_port,cand_lgknds[ilgknd]);
          if (-1 == rst) {
            my_init_int_ary(redundant_port_num,ports_muxes,-1); 
            break;
          }
          /*Ensure it does not give a new input!!!*/
          else if (BLE_IN == ble_arch->blends[cell_idx].inputs[im].cands[rst]->type) {
            input_idx = ble_arch->blends[cell_idx].inputs[im].cands[rst]->idx;
            if (0 == ble_info->input_used[input_idx]) {
              my_init_int_ary(redundant_port_num,ports_muxes,-1); 
              break;
            }
            else {
              ports_muxes[cur_redund] = rst;
              cur_redund++;
            }
          }
          else {
            ports_muxes[cur_redund] = rst;
            cur_redund++;
          }
          ret = 1;
          for (j=0; j<redundant_port_num; j++) {
            if (-1 == ports_muxes[j]) {
              ret = 0;
            }
          } 
          if (1 == ret) {
            cur_redund = 0;
            for (j=0; j<ble_arch->blends[cell_idx].input_num; j++) {
              if (0 == ble_info->blend_port_used[cell_idx][j]) {
                ble_info->blend_port_used[cell_idx][j] = 1;
                ble_info->blend_muxes[cell_idx][j] = ports_muxes[cur_redund];
                ble_info->blend_lgknd_input_idxes[cell_idx][j] = -1;
                cur_redund++;
              }
            }
            /*Don't forget free resources!!!*/
            free(ports_muxes);
            free(cand_lgknds);
            return ret;
          }
        }
      }
    }
    ret = 0;
  }

  /*We still cannot find a solution here, we have to create a input for BLE*/
  /*Strict off: create redundant inputs one by one but using least fanout inputs*/
  if (1 == clb_logic_equal) {
  if (1 != strict) {
    for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
      if (0 == ble_info->blend_port_used[cell_idx][im]) {
        for (ilgknd = 0; ilgknd<lgknd->input_num; ilgknd++) {
          /* Check if this predecessor's output has been absorbed,
           * if absorbed, we should not consider it!
           */
          if (1 == check_ble_absorb_lgknd_output(ble_info,lgknd,verbose)) {
            continue;
          }
          des_port = &ble_arch->blends[cell_idx].inputs[im];
          /*Try to find an legal routing solution*/
          rst = sat_cell_port_target_lgknd(ble_arch,ble_info,des_port,lgknd->inputs[ilgknd]);
          if ((-1 != rst)&&(BLE_IN == ble_arch->blends[cell_idx].inputs[im].cands[rst]->type)) {
            input_idx = ble_arch->blends[cell_idx].inputs[im].cands[rst]->idx;
            if (0 == ble_info->input_used[input_idx]) {
              ble_info->blend_port_used[cell_idx][im] = 1;
              ble_info->blend_muxes[cell_idx][im] = rst;
              ble_info->blend_lgknd_input_idxes[cell_idx][im] = -1;
              ble_info->input_used[input_idx] = 1;
              ble_info->input_lgknds[input_idx] = lgknd;
              ble_info->input_lgknds_input_idx[input_idx] = ilgknd;
              break;
            }
          }
        }
      }
    }
    ret = 1;
    for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
      if ((0 == ble_info->blend_port_used[cell_idx][im])||(-1 == ble_info->blend_muxes[cell_idx][im])) {
        ret = 0;
      }
    } 
    if (1 == ret) {
      /*Don't forget free resources!!!*/
      free(ports_muxes);
      free(cand_lgknds);
      return ret;
    }
  }
  else { 
    /*Strict mode: all redundant inputs should share the same predecessor*/
    for (ilgknd = 0; ilgknd<lgknd->input_num; ilgknd++) {
      cur_redund = 0;
      /* Check if this predecessor's output has been absorbed,
       * if absorbed, we should not consider it!
       */
      if (1 == check_ble_absorb_lgknd_output(ble_info,lgknd,verbose)) {
        continue;
      }
      for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
        if (0 == ble_info->blend_port_used[cell_idx][im]) {
          des_port = &ble_arch->blends[cell_idx].inputs[im];
          /*Try to find an legal routing solution*/
          rst = sat_cell_port_target_lgknd(ble_arch,ble_info,des_port,lgknd->inputs[ilgknd]);
          if ((-1 != rst)&&(BLE_IN == ble_arch->blends[cell_idx].inputs[im].cands[rst]->type)) {
            input_idx = ble_arch->blends[cell_idx].inputs[im].cands[rst]->idx;
            if (0 == ble_info->input_used[input_idx]) {
              ports_muxes[cur_redund] = rst;
              cur_redund++;
            }
            else {
              my_init_int_ary(redundant_port_num,ports_muxes,-1); 
              break;
            }
          }
          else {
            my_init_int_ary(redundant_port_num,ports_muxes,-1); 
            break;
          }
          ret = 1;
          for (j=0; j<redundant_port_num; j++) {
            if (-1 == ports_muxes[j]) {
              ret = 0;
            }
          } 
          if (1 == ret) {
            cur_redund = 0;
            for (j=0; j<ble_arch->blends[cell_idx].input_num; j++) {
              if (0 == ble_info->blend_port_used[cell_idx][j]) {
                ble_info->blend_port_used[cell_idx][j] = 1;
                ble_info->blend_muxes[cell_idx][j] = rst;
                ble_info->blend_lgknd_input_idxes[cell_idx][j] = -1;

                input_idx = ble_arch->blends[cell_idx].inputs[im].cands[ports_muxes[cur_redund]]->idx;
                ble_info->input_used[input_idx] = 1;
                ble_info->input_lgknds[input_idx] = lgknd;
                ble_info->input_lgknds_input_idx[input_idx] = ilgknd;
                cur_redund++;
              }
            }
            /*Don't forget free resources!!!*/
            free(ports_muxes);
            free(cand_lgknds);
            return ret;
          }
        }
      }
    }
    ret = 0;
  }
  }

  if (1 == ret) {
    printf("Error:(sat_redundant_port)Ret should be 0 here!\n");
    exit(1);
  }
  /*Don't forget free resources!!!*/
  free(ports_muxes);
  free(cand_lgknds);
  /*Reach here means fail*/
  return ret;
}
                              
/**
 * Route logic node's inputs on cell
 * Redundant inputs of the cell is left open
 */
int sat_lgknd_inputs(t_ble_arch* ble_arch, 
                     t_ble_info* ble_info,
                     int cell_idx,
                     t_lgknd* lgknd,
                     int logic_equivalent,
                     int strict,
                     int verbose)
{
  int ret = 1;
  int im,tmp_cell_idx,max_cell_idx,input_idx;
  int lgknd_input_all_mapped = 0;
  int* lgknd_input_status = NULL;

  // Constant generator
  if (0 == lgknd->input_num) {
    return 1;
  }
 
  lgknd_input_status = (int*)my_malloc(sizeof(int)*lgknd->input_num);
  my_init_int_ary(lgknd->input_num,lgknd_input_status,0);
  /*Each time,we map the inputs which has the max cell idx in this BLE in priority*/
  while(0 == lgknd_input_all_mapped) {
    max_cell_idx = -1;
    input_idx = -1;
    for (im=0; im<lgknd->input_num; im++) {
      if (0 == lgknd_input_status[im]) {
        /*Check if it is in this BLE*/
        tmp_cell_idx = spot_lgknd_ble_cell_idx(ble_info,lgknd->inputs[im]);
        //if (-1 == tmp_cell_idx) {
        //  continue;
        //}
        if ((-1 == max_cell_idx)||(max_cell_idx < tmp_cell_idx)) {
        //||(ble_arch->blends[max_cell_idx].priority < ble_arch->blends[tmp_cell_idx].priority)) { 
          max_cell_idx = tmp_cell_idx; 
          input_idx = im;
        }
      } 
    }
  
    /*Route regular inputs*/ 
    //if (-1 == input_idx) {
    //  free(lgknd_input_status);
    //  return 0;
    //}
    if (1 != sat_lgknd_input_cell_port(ble_arch,ble_info,cell_idx,lgknd,input_idx,logic_equivalent)) {
      ret = 0;
      //printf("Info:(sat_lgknd_inputs)Fail in sat_lgknd_input_cell_port(lgknd index:%d, Cell_idx:%d)\n",lgknd->idx,cell_idx);
      free(lgknd_input_status);
      return ret;
    }
    lgknd_input_status[input_idx] = 1;

    lgknd_input_all_mapped = 1;
    /*Check if all inputs has been mapped*/
    for (im=0; im<lgknd->input_num; im++) {
      if (0 == lgknd_input_status[im]) {
        lgknd_input_all_mapped = 0;
        break;
      }
    }
  }
  /*I kill this part because in AAPack, VPack, T-VPack, they just leave it open!!!*/
  /*Route Redundant inputs*/
  //if (1 != sat_cell_redundant_ports(ble_arch,ble_info,cell_idx,lgknd,strict,0,verbose)) {
  //  ret = 0;
    //printf("Info:(sat_lgknd_inputs)Fail in sat_cell_redundant_ports(lgknd index:%d, Cell_idx:%d)\n",lgknd->idx,cell_idx);
  //  free(lgknd_input_status);
  //  return ret;
  //}

  free(lgknd_input_status);
  return ret;
}

/* Mark output latches mapped
 * This function should be called after each SAT solver, 
 * it checks latches of each node is mapped or not.
 * When add_buf is set as 1, a buffer is added before latch that hasn't been mapped.
 */
int mark_lgknd_outputs_mapped(t_lgkntwk* lgkntwk,
                              t_lgknd* cur,
                              int ble_num,
                              int add_buf,
                              t_ble_info* ble_info)
{
  int im;
  int io;
  int latched = 0;
  int need_buf = add_buf;
  t_lgknd* latch_ptr = NULL;

  // We don't want duplicated buffer!
  if (ND_BUF == cur->type) {
    need_buf = 0;
  }
  
  for(im=0; im<cur->output_num; im++) {
    if ((ND_LATCH == cur->outputs[im]->type)&&(0 == cur->outputs[im]->mapped)) {
      latched = 1;
      latch_ptr = cur->outputs[im];
      for (io=0; io<ble_info->output_num; io++) {
        if (latch_ptr == ble_info->output_lgknds[io]) {
          latch_ptr->mapped = 1;
          latch_ptr->map_climb = 0;
          latch_ptr->ble_info = ble_info;
          latch_ptr->ble_idx = ble_num;
          if (DEBUG) {
            printf("Output latch mapped!(Cur idx: %d)\n",cur->idx);
          }
          return 1;
        }
      }
    }
  }
  
  /*Reach here,it means we haven't found a latch or latch has not been mapped*/
  if ((1 == latched)&&(1 == need_buf)) {
    // Check if this lgknd has one buffer already!
    for (im=0; im<cur->output_num; im++) {
      if (ND_BUF == cur->outputs[im]->type) {
        need_buf = 0;
        return 1;
      }
    }
    if ((1 == need_buf)&&(0 == latch_ptr->mapped)) {
      /*Add a buffer between latch and cur*/
      add_one_buf_lgknd(latch_ptr,0);
      add_input_bufs_lgknd_lgkntwk(lgkntwk,latch_ptr);
      latch_ptr->inputs[0]->try_pattern_types = (int*)my_malloc(sizeof(int)*(latch_ptr->inputs[0]->input_num+1));
      memset(latch_ptr->inputs[0]->try_pattern_types,0,(latch_ptr->inputs[0]->input_num+1)*sizeof(int));
      //printf("Add_one_new_buffer!\n");
    }
    return 0;
  }

  return 1;
}

/** Depth SAT solver
 * Given the sat_type and sat_lgknds, we combine them with the mapped logic nodes
 * in ble_info. Mark their depth in local network, and remap everyone.
 * FOR LATCH: 
 *   DON'T INSERT BUFFER UNTIL THE BLE_INFO IS FULL OR NONE CAN BE INSERTED(sat_lgknd_num == 0,
 *   sat_lgknds == NULL)
 */
int depth_local_router(int sat_lgknd_num,
                       t_lgknd** sat_lgknds,
                       t_lgkntwk* lgkntwk,
                       t_ble_arch* ble_arch,
                       t_ble_info* ble_info,
                       int verbose) {
  int i,j,cur_lgknd;
  int lgknd_num; // Number of all lgknds to be mapped
  t_lgknd** lgknds;
  t_lgknd** sorted_lgknds;
  t_lgknd* swap_lgknd;
  int* sorted_cells;
  int swap_cell;
  /*Create a local copy*/
  t_ble_info* ble_info_mirror = (t_ble_info*)my_malloc(sizeof(t_ble_info));

  /*Initial local copy*/
  init_ble_info(ble_arch,ble_info_mirror);

  if (0 == sat_lgknd_num) {
    assert(NULL == sat_lgknds);
    // Add BUFFER
    for (i=0; i<ble_info->blend_num; i++) {
      if (1 == ble_info->blend_used[i]) {
        mark_lgknd_outputs_mapped(lgkntwk,ble_info->blend_lgknds[i],ble_info->idx,1,ble_info);
      }
    }
    return 1;
  }

  lgknd_num = sat_lgknd_num;
  /*Calculate the number of lgknd*/
  for (i=0; i<ble_info->blend_num; i++) {
    if (1 == ble_info->blend_used[i]) {
      assert(NULL != ble_info->blend_lgknds[i]);
      lgknd_num++;
    }
  }
  // Check if lgknd_num exceeds the blend_num, if exceeds, return failure
  if (lgknd_num > ble_info->blend_num) {
    return 0;
  }
  // Alloc lgknds, and setup
  cur_lgknd = 0;
  lgknds = (t_lgknd**)my_malloc(lgknd_num*sizeof(t_lgknd*)); 
  for (i=0; i<sat_lgknd_num; i++) {
    lgknds[cur_lgknd] = sat_lgknds[i];
    cur_lgknd++;
  }
  for (i=0; i<ble_info->blend_num; i++) {
    if (1 == ble_info->blend_used[i]) {
      assert(NULL != ble_info->blend_lgknds[i]);
      lgknds[cur_lgknd] = ble_info->blend_lgknds[i];
      cur_lgknd++;
    }
  }
  assert(cur_lgknd == lgknd_num);
  /*Determine comb_num for lgknd*/
  for (i=0; i<lgknd_num; i++) {
    lgknds[i]->comb_num = 0;
  }
  for (i=0; i<lgknd_num; i++) {
    for (j=0; j<lgknds[i]->output_num; j++) {
      if (ND_LATCH != lgknds[i]->outputs[j]->type) {
        for (cur_lgknd=0; cur_lgknd<lgknd_num; cur_lgknd++) {
          if (cur_lgknd == i) {
            continue;
          }
          if (lgknds[i]->outputs[j] == lgknds[cur_lgknd]) {
            lgknds[i]->comb_num++;
          }
        }
      }
    }
  }
  // Sort the lgknds according to comb_num, ascend
  sorted_lgknds = (t_lgknd**)my_malloc(lgknd_num*sizeof(t_lgknd*));
  sorted_lgknds[0] = lgknds[0];
  for (i=1; i<lgknd_num; i++) {
    sorted_lgknds[i] = lgknds[i];
    for (j=i; j>0; j--) {
      if (sorted_lgknds[j]->comb_num < sorted_lgknds[j-1]->comb_num) {
        swap_lgknd = sorted_lgknds[j];
        sorted_lgknds[j] = sorted_lgknds[j-1];
        sorted_lgknds[j-1] = swap_lgknd; 
      } 
    } 
  } 
  // give each node a cell index according to their depth.
  // Sort blends according to depth, ascend
  assert(ble_arch->blend_num == ble_info->blend_num);
  sorted_cells = (int*)my_malloc(ble_info->blend_num*sizeof(int));
  sorted_cells[0] = ble_arch->blends[0].depth;
  for (i=1; i<ble_arch->blend_num; i++) {
    sorted_cells[i] = i;
    for (j=i; j>0; j--) {
      if (ble_arch->blends[sorted_cells[j]].depth < ble_arch->blends[sorted_cells[j-1]].depth) {
        swap_cell = sorted_cells[j-1];
        sorted_cells[j] =  sorted_cells[j-1];
        sorted_cells[j-1] = swap_cell;
      }
    }
  }
  // We should start from the bottom, the node with max depth 
  // Outputs
  for (i=0; i<lgknd_num; i++) {
    for (j=0; j<ble_arch->blend_num; j++) {
      assert(0 == ble_info_mirror->blend_used[sorted_cells[j]]);
      if (0 == sat_lgknd_outputs(ble_arch,ble_info_mirror,sorted_cells[j],sorted_lgknds[i],sorted_lgknds[i]->comb_num,verbose)) {
        free(lgknds);
        free(sorted_lgknds);
        free(sorted_cells);
        return 0;
      } 
    }
  }
  // Inputs
  for (i=0; i<lgknd_num; i++) {
    for (j=0; j<ble_arch->blend_num; j++) {
      assert(0 == ble_info_mirror->blend_used[sorted_cells[j]]);
      if (0 == sat_lgknd_inputs(ble_arch,ble_info_mirror,sorted_cells[j],sorted_lgknds[i],sorted_lgknds[i]->logic_equivalent,sorted_lgknds[i]->strict,verbose)) { 
        free(lgknds);
        free(sorted_lgknds);
        free(sorted_cells);
        return 0;
      } 
    }
  }
  // Reach here, it is a success...
  copy_ble_info(ble_arch,ble_info_mirror,ble_info);  
  for (i=0; i<ble_info->blend_num; i++) {
    if (1 == ble_info->blend_used[i]) {
      mark_lgknd_outputs_mapped(lgkntwk,ble_info->blend_lgknds[i],ble_info->idx,0,ble_info);
    }
  }
  for (i=0; i<sat_lgknd_num; i++) {
    sat_lgknds[i]->mapped = 1;
    sat_lgknds[i]->map_climb = 0;
    sat_lgknds[i]->ble_info = ble_info;
    sat_lgknds[i]->ble_idx = ble_info->idx;
  }
  free(lgknds);
  free(sorted_lgknds);
  free(sorted_cells);
  
  return 1;
}
                     


/* A mini SAT solver, sat_type >= 0
 * ATTENTION: All used BLE cell' outputs/inputs will be mapped as well. This is a killer for run time.
 *            But it can increase resource utilization. 
 * In this SAT solver, buffer won't be created for predecessors or root, they will be added somewhere else.
 * 1. Length of array sat_lgknds should not be less than sat_type+1.
 *    The last element of array sat_lgknds should be the root node.
 * 2. Array cell_idx contains the index of cell where each logic node is supposed to be mapped.
 *    Array index range should be [0..sat_type]
 * 3. Array logic equivalent and strict select the mapping mode for logic node's inputs.
 *    Array index range should be [0..sat_type]
 * TODO: Some mapped logic node's inputs should be remapped when a new
 *       one whose outputs connect to
 * Return 1 if success,
 * Return 0 if fail
 */
int mini_sat_solver(int sat_type,
                    t_lgkntwk* lgkntwk,
                    t_lgknd** sat_lgknds,
                    int* sat_idxes,
                    t_ble_arch* ble_arch,
                    t_ble_info* ble_info,
                    int* logic_equivalent,
                    int* strict,
                    int verbose)
{
  int im = 0;
  //int ip=0;
  /* UPDATE: Array comb_nums contains the number of absorbed combinational outputs 
   *         inside BLE. Array include each node to be mapped or already mapped in BLE.
   */
  int* comb_nums = NULL;
  /* UPDATE: array to flag which cell require remapping inputs
   */
  int* remap_cells = NULL;
  /*Create a local copy*/
  t_ble_info* ble_info_mirror = (t_ble_info*)my_malloc(sizeof(t_ble_info));

  /*Initial local copy*/
  init_ble_info(ble_arch,ble_info_mirror);
  copy_ble_info(ble_arch,ble_info,ble_info_mirror);

  comb_nums = (int*)my_malloc(sizeof(int)*(sat_type+1));
  my_init_int_ary(sat_type+1,comb_nums,0);

  /*Determine comb_num*/
  count_lgknds_comb_num_ble(ble_info,sat_type+1,sat_lgknds,comb_nums,verbose);

  /*Map root outputs, root is the last element of sat_lgknds,
   *Root should not have any combinational outputs to the rest, or it is illegal.
   */
  if (0 == sat_lgknd_outputs(ble_arch,ble_info_mirror,sat_idxes[sat_type],sat_lgknds[sat_type],comb_nums[sat_type],verbose)) {
    if (1 == verbose) {
      printf("Info: (mini_sat_solver)Fail sat_lgknd_output(Cell idx:%d,Logic node idx: %d).\n",sat_idxes[sat_type],sat_lgknds[sat_type]->idx);
    }
    /*Don't forget free resources*/
    free(comb_nums);
    free_ble_info(ble_info_mirror);
    free(ble_info_mirror);
    return 0;
  }

  
  /* The rest of sat_lgknds should be DFSed before this function called, 
   * so we map it one by one. Here we assume the depth of each inputs of BLE cell 
   * is sorted in defining BLE architecture 
   */
  for (im=0; im<sat_type; im++) {
    if (0 == sat_lgknd_outputs(ble_arch,ble_info_mirror,sat_idxes[im],sat_lgknds[im],comb_nums[im],verbose)) {
      if (1 == verbose) {
        printf("Info: (mini_sat_solver)Fail sat_lgknd_output(Cell idx:%d,Logic node idx: %d).\n",sat_idxes[im],sat_lgknds[im]->idx);
      }
      /*Don't forget free resources*/
      free(comb_nums);
      free_ble_info(ble_info_mirror);
      free(ble_info_mirror);
      return 0;
    }
  }

  /* Some input(latches DON'T) need be remapped due to new joined logic node
   * Check inputs of each mapped logic node excluding sat_lgknds
   * Mark remap needs in remap_cells
   */
  /*Initial remap cell*/
  //remap_cells = (int*)my_malloc(sizeof(int)*(ble_arch->blend_num));
  //my_init_int_ary(ble_arch->blend_num,remap_cells,0);

  //for (im=0; im<sat_type+1; im++) {
  //  for (ip=0; ip<sat_lgknds[im]->output_num; ip++) {
     /*Check conflicts*/
  //    check_lgknd_input_remap(ble_arch,ble_info,sat_lgknds[im],ip,remap_cells,verbose);
  //  }
  //}
        
  /*Remap the cell in remap_cells*/
  for (im = 0; im<ble_arch->blend_num; im++) {
  //  if (1 == remap_cells[im]) {
      /*Free used inputs*/
    if (1 == ble_info_mirror->blend_used[im]) {
      assert(NULL != ble_info_mirror->blend_lgknds[im]);
      free_lgknd_inputs_in_ble(ble_arch,ble_info_mirror,im,ble_info_mirror->blend_lgknds[im],verbose);
    }
  }
  /*Free BLE inputs*/
  for (im=0; im<ble_info_mirror->input_num; im++) {
    if (NULL != ble_info_mirror->input_lgknds[im]) {
      ble_info_mirror->input_used[im] = 0;
      ble_info_mirror->input_lgknds[im] = NULL;
      ble_info_mirror->input_lgknds_input_idx[im] = -1;
    }
  }
  for (im = 0; im<ble_arch->blend_num; im++) {
   // if (1 == remap_cells[im]) {
      /*Everything match.(BLE info, BLE cell index), remap inputs*/
    if (1 == ble_info_mirror->blend_used[im]) {
      assert(NULL != ble_info_mirror->blend_lgknds[im]);
      if (0 == sat_lgknd_inputs(ble_arch,ble_info_mirror,im,ble_info_mirror->blend_lgknds[im],ble_info_mirror->blend_lgknds[im]->logic_equivalent,strict[sat_type],verbose)) { 
        if (1 == verbose) {
          printf("Info: (mini_sat_solver)Fail remap sat_lgknd_inputs(Cell idx:%d,Logic node idx: %d).\n",im,ble_info->blend_lgknds[im]->idx);
        }
        /*Don't forget free resources*/
        free(remap_cells);
        free(comb_nums);
        free_ble_info(ble_info_mirror);
        free(ble_info_mirror);
        return 0;
      }
      else {
        if (1 ==verbose) {
          printf("Info: (mini_sat_solver)Success remap sat_lgknd_inputs(Cell idx:%d,Logic node idx: %d).\n",im,ble_info_mirror->blend_lgknds[im]->idx);
        }
      }
    }
  }
        
  /* Now we can map the inputs for each logic node, the root goes first,
   * the rest follow.
   */
  //if (0 == sat_lgknd_inputs(ble_arch,ble_info_mirror,sat_idxes[sat_type],sat_lgknds[sat_type],logic_equivalent[sat_type],strict[sat_type],verbose)) { 
  //  if (1 == verbose) {
  //    printf("Info: (mini_sat_solver)Fail sat_lgknd_inputs(Cell idx:%d,Logic node idx: %d).\n",sat_idxes[sat_type],sat_lgknds[sat_type]->idx);
  //  }
    /*Don't forget free resources*/
  //  free(remap_cells);
  //  free(comb_nums);
  //  free_ble_info(ble_info_mirror);
  //  free(ble_info_mirror);
  //  return 0;
  //}
  
  //for (im=0; im<sat_type; im++) {
  //  if (0 == sat_lgknd_inputs(ble_arch,ble_info_mirror,sat_idxes[im],sat_lgknds[im],logic_equivalent[im],strict[im],verbose)) { 
  //    if (1 == verbose) {
  //      printf("Info: (mini_sat_solver)Fail sat_lgknd_inputs(Cell idx:%d,Logic node idx: %d).\n",sat_idxes[im],sat_lgknds[im]->idx);
  //    }
      /*Don't forget free resources*/
  //    free(remap_cells);
  //    free(comb_nums);
  //    free_ble_info(ble_info_mirror);
  //    free(ble_info_mirror);
  //    return 0;
  //  }
  //}
  
  /*Reach here, mapping is a success, copy mirror to orginal*/ 
  copy_ble_info(ble_arch,ble_info_mirror,ble_info);  
 
  /*Don't forget free resources*/
  free(remap_cells);
  free(comb_nums);
  free_ble_info(ble_info_mirror);
  free(ble_info_mirror);
  
  return 1;
}

/* Run SAT solver, map (sat_type+1) logic nodes into a BLE. The position of last logic node
 * is fixed on cell_idx. 
 * Three steps:
 * A. Check BLE architecture has enough resources.
 * B. Prepare necessary information for mini_sat_solver
 *   1. sat_type, which depends on the chosen BLE cell.
 *   2. comb_nums, the number of combinational connections which will be aborbed inside BLE.
 *   3. cell_idx, the index of cell where root will be mapped.
 *   4. sat_lgknds, logic nodes to be SAT mapped. Array length should be sat_type+1. 
 *                  And root node should be the last one.
 *   5. logic_equivalent, actually it is determined by user. TODO
 *   6. strict, it should be determined by circuits. For non-LUT circuits, it should be strict. TODO
 * C. Free related BLE inputs/outputs, Run Mini_sat_solver, and complete notes
 *   1. Free related BLE inputs/outputs, if they can be absorbed   
 *   2. Run Mini_sat_solver
 *   3. If success, run mark_outputs_mapped() in order to process unmapped latches.
 *      Mark logic nodes that have been mapped.
 *   4. If fail, mark root node that have been tried.
 * Return 1 if success,
 * Return 0 if fail.
 */
int run_sat_solver(t_lgkntwk* lgkntwk,
                   t_ble_arch* ble_arch,
                   t_ble_info* ble_info,
                   int sat_type,
                   int cell_idx,
                   t_lgknd** sat_lgknds,
                   int timing_analysis,
                   t_delay_info* delay_info,
                   float alpha,
                   int verbose)
{
  int* sat_idxes = NULL; 
  int* logic_equivalent = NULL; 
  int* strict = NULL;
  /*Create a local copy*/
  //t_ble_info* ble_info_mirror = (t_ble_info*)my_malloc(sizeof(t_ble_info));

  int im;
  
  sat_idxes = (int*)my_malloc(sizeof(int)*(sat_type+1));

  /*Initial sat_idxes*/
  my_init_int_ary(sat_type+1,sat_idxes,-1); 

  /*Check if BLE has enough resource required...*/
  if (0 == check_sat(sat_type,ble_arch,ble_info,&ble_arch->blends[cell_idx],sat_idxes,0)) {
    /*Don't forget free resources*/
    free(sat_idxes);
    return 0;
  }
  /*Fill the cell idx as the "last" element */
  sat_idxes[sat_type] = cell_idx;
  
  /*Initial local copy*/
  //init_ble_info(ble_arch,ble_info_mirror);
  //copy_ble_info(ble_arch,ble_info,ble_info_mirror);
  /*Initial*/
  logic_equivalent = (int*)my_malloc(sizeof(int)*sat_type+1);
  strict = (int*)my_malloc(sizeof(int)*sat_type+1);

  /*Logic equivalents and strict*/
  my_init_int_ary(sat_type+1,logic_equivalent,1); 
  my_init_int_ary(sat_type+1,strict,1); 
  
  /*Call Mini_SAT_solver*/
  if (0 == mini_sat_solver(sat_type,lgkntwk,sat_lgknds,sat_idxes,ble_arch,ble_info,logic_equivalent,strict,verbose)) {
    sat_lgknds[sat_type]->try_sat = 1;
    /*Don't forget free resources*/
    free(logic_equivalent);
    free(strict);
    free(sat_idxes);
    return 0;
  } 

  /*SAT success, we should mark these lgknds and latch followers*/
  for (im=0; im<sat_type+1; im++) {
    sat_lgknds[im]->mapped = 1;
    sat_lgknds[im]->ble_idx = ble_info->idx;
    sat_lgknds[im]->ble_cell_idx = sat_idxes[im];
    sat_lgknds[im]->ble_info = ble_info;
    if (0 == mark_lgknd_outputs_mapped(lgkntwk,sat_lgknds[im],ble_info->idx,1,ble_info)) {
      /*Depth weighting*/
      depth_weighting_lgkntwk(lgkntwk,verbose);

      /*timing analysis*/
      if (timing_analysis) {
        init_timing_lgkntwk(lgkntwk,delay_info,verbose);
        timing_analysis_lgkntwk(lgkntwk,verbose);
        lgkntwk->critical_delay = critical_delay_lgkntwk(lgkntwk,verbose);
        timing_required_lgkntwk(lgkntwk,lgkntwk->critical_delay,verbose);
        timing_slack_lgkntwk(lgkntwk,verbose);
        critical_paths_affect_lgkntwk(lgkntwk,verbose);
        lgkntwk->max_slack = timing_max_slack_lgkntwk(lgkntwk,verbose);
        timing_weighting_lgkntwk(lgkntwk,lgkntwk->max_slack,verbose);
      }
      // Weighting logic network
      merge_weights_lgkntwk(lgkntwk,alpha,verbose);
    }
  }
  
  /*Don't forget free resources*/
  free(logic_equivalent);
  free(strict);
  free(sat_idxes);

  return 1;
}

/* Count the combinational interconnections between logic nodes and BLE
 * IMPORTANT: 1. COMBINATIONAL ONLY!
 *            2. We DON'T malloc comb_num in this function. Do it before called.
 *               The length should be no less than lgknd_num.
 *            3. TODO: free some BLE inputs whose nets can be absorbed after
 *               success mapping.
 */
int count_lgknds_comb_num_ble(t_ble_info* ble_info,
                              int lgknd_num,
                              t_lgknd** lgknds,
                              int* comb_num,
                              int verbose)
{
  int used_blend_num = count_used_blend(ble_info,verbose);
  int comb_total = lgknd_num + used_blend_num; 
  int* comb_tmp = (int*)my_malloc(sizeof(int)*comb_total);
  t_lgknd** lgknds_tmp = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*comb_total);

  int im; 
  int ip;

  /* Construct the array containg all logic nodes 
   * (lgknds + lgknds_already_in_ble)
   */ 
   my_init_int_ary(comb_total,comb_tmp,0); 
  
  /*Copy lgknds first*/
  for (im=0; im<lgknd_num; im++) {
    lgknds_tmp[im] = lgknds[im];
  } 
  ip = lgknd_num;
  /*Copy lgknds_already_in_ble*/ 
  for (im=0; im<ble_info->blend_num; im++) {
    if (1 == ble_info->blend_used[im]) {
      lgknds_tmp[ip] = ble_info->blend_lgknds[im];
      ip++;
    } 
  }
  
  /*Call count_lgknds_comb_num, update comb_num*/
  count_lgknds_comb_num(comb_total,lgknds_tmp,comb_tmp,verbose);

  /*Copy the results to comb_num*/
  for (im=0; im<lgknd_num; im++) {
    comb_num[im] = comb_tmp[im];
  } 
  
  /*Don't forget free resources*/
  free(comb_tmp);
  free(lgknds_tmp);

  return 1;
}

/* Count the combinational mutual interconnections
 * among given logic nodes.
 * IMPORTANT: 1. COMBINATIONAL ONLY!
 *            2. We DON'T malloc comb_num in this function. Do it before called.
 *               The length should equal to lgknd_num.
 */
int count_lgknds_comb_num(int lgknd_num,
                          t_lgknd** lgknds,
                          int* comb_num,
                          int verbose)
{
  int ind1;
  int im;
  int ind2;

  /*Initial array comb_num*/
  my_init_int_ary(lgknd_num,comb_num,0);

  /* Search each predecessor of lgknds[i] for connections,
   * If exist connections, increase the counter of predecessors.
   * Important: we only search the nodes which sat_idxes is smaller than current node
   */
  for (ind1=0; ind1<lgknd_num; ind1++) {
    for (im=0; im<lgknds[ind1]->input_num; im++) {
      for (ind2=0; ind2<lgknd_num; ind2++) {
        if ((lgknds[ind2] == lgknds[ind1]->inputs[im])&&(ind1 > ind2)) {
          comb_num[ind2]++; 
        }
      }
    } 
  }

  return 1;
}

/* Determine the maximium sat_type of given logic nodes
 * Each unmapped inputs which are not PI/POs or latches can be satisfied.
 * Make sure each sat_lgknd is UNIQUE is very very important!!!
 * IMPORTANT: we DO NOT malloc sat_lgknds, it should be malloced before called!
 *            0 <= sat_type <= root->input_num. 
 *            Set upbound root->input_num+1 for sat_lgknds is preferred(Last one reserverd for root). 
 * (Optional)strong: turn on filling sat_lgknds.
 */
int lgknds_sat_type(t_lgknd* root,
                    t_lgknd** sat_lgknds,
                    int strong,
                    enum e_pattern_member_type pattern_member_type,
                    int verbose)
{       
  int im;
  int sat_type = 0;
  t_lgknd** preds = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*root->input_num);
  float* preds_edge_weight = (float*)my_malloc(sizeof(float)*root->input_num); 
  int* cur_depths = NULL;
  
  /*Initial*/
  for (im=0; im<root->input_num; im++) {
    preds[im] = NULL;
    preds_edge_weight[im] = 0.0;
  }

  /*First, form the array with predecessors*/
  for (im=0; im<root->input_num; im++) {
    if (PATTERN_MEMBER_NORMAL == pattern_member_type) {
      if ((ND_LATCH == root->inputs[im]->type)&&(0 == root->inputs[im]->inputs[0]->mapped)) {
        /*When come across self feedback, we don't increase sat_type, and avoid duplicate sat_lgknds */
        if ((root != root->inputs[im]->inputs[0])&&(-1 == spot_index_lgknd_array(root->input_num,preds,root->inputs[im]->inputs[0]))) {
          preds[sat_type] = root->inputs[im]->inputs[0];
          preds_edge_weight[sat_type] = root->in_edges[im].weight;
          //for (j=0; j<root->inputs[im]->output_num; j++) {
          //  preds_edge_weight[sat_type] += root->inputs[im]->out_edges[j]->weight;
          //}
          //for (j=0; j<root->inputs[im]->input_num; j++) {
          //  preds_edge_weight[sat_type] += root->inputs[im]->in_edges[j].weight;
          //}
          //preds_edge_weight[sat_type] = root->in_edges[im].weight/preds_edge_weight[sat_type];
          sat_type++;
        }
      } else if ((ND_PI != root->inputs[im]->type)&&(ND_PO != root->inputs[im]->type)&&(0 == root->inputs[im]->mapped)) {
        if (-1 == spot_index_lgknd_array(root->input_num,preds,root->inputs[im])) {
          preds[sat_type] = root->inputs[im];
          preds_edge_weight[sat_type] = root->in_edges[im].weight;
          //for (j=0; j<root->inputs[im]->output_num; j++) {
          //  preds_edge_weight[sat_type] += root->inputs[im]->out_edges[j]->weight;
          //}
          //for (j=0; j<root->inputs[im]->input_num; j++) {
          //  preds_edge_weight[sat_type] += root->inputs[im]->in_edges[j].weight;
          //}
          //preds_edge_weight[sat_type] = root->in_edges[im].weight/preds_edge_weight[sat_type];
          sat_type++;
        }
      }
    } else if (PATTERN_MEMBER_COMBONLY == pattern_member_type) {
      /*Check sat_lgknds do not include any PI/PO*/
      if ((ND_PI != root->inputs[im]->type)&&(ND_PO != root->inputs[im]->type)&&(ND_LATCH != root->inputs[im]->type)&&(0 == root->inputs[im]->mapped)) {
        if (-1 == spot_index_lgknd_array(root->input_num,preds,root->inputs[im])) {
          preds[sat_type] = root->inputs[im];
          preds_edge_weight[sat_type] = root->in_edges[im].weight;
          //for (j=0; j<root->inputs[im]->output_num; j++) {
          //  preds_edge_weight[sat_type] += root->inputs[im]->out_edges[j]->weight;
          //}
          //for (j=0; j<root->inputs[im]->input_num; j++) {
          //  preds_edge_weight[sat_type] += root->inputs[im]->in_edges[j].weight;
          //}
          //preds_edge_weight[sat_type] = root->in_edges[im].weight/preds_edge_weight[sat_type];
          sat_type++;
        }
      }
    }
  }
  /*Sort the depths of predecessors*/
  //if (sat_type > 0) {
  //  cur_depths = local_dfs_lgknds(sat_type,preds,verbose);
  //}
  //if (0 == sat_type) {
  //  return sat_type;
  //}
  
  /*Copy preds to sat_lgknds if strong mode is turned on*/
  if (1 == strong) {
    for (im=0; im<sat_type; im++) {
      sat_lgknds[im] = preds[im];
    }
  }

  /*Local dfs, sort the sat_lgknds by depths from bottom to top*/
  /*SAT type should be greater than 0, or it is meaningless to do local DFS*/
  if ((sat_type > 0)&&(1 == strong)) {
    // sort
    cur_depths = local_dfs_lgknds(sat_type,sat_lgknds,preds_edge_weight,0);
  }

  /*Don't forget free resources*/
  free(preds_edge_weight);
  free(cur_depths);
  free(preds);

  return sat_type;
}

/* SAT mapper based on sat_solver. 
 * Accroding to given sat_type, determine root's predecessor number 
 * is able to run the sat_solver. And organize the sat_lgknds for 
 * SAT solver.
 */
int sat_mapper(t_lgkntwk* lgkntwk,
               t_ble_arch* ble_arch,
               t_ble_info* ble_info,
               int sat_type,
               int cell_idx,
               t_lgknd* root,
               int timing_analysis,
               t_delay_info* delay_info,
               t_mpack_conf mpack_opts,
               int verbose)
{
  int lgknd_sat_type = 0;
  t_lgknd** sat_lgknds = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*(root->input_num+1));
 
  /* Determine the largest possible sat_type, constraints come from two side:
   * 1. Logic node's inputs
   * 2. BLE cell idx's inputs
   * After we get the MAX possible sat_type from logic node, we try iteratively for each
   * possible sat_type (0<= i <= sat_type). 
   */
  lgknd_sat_type = lgknds_sat_type(root,sat_lgknds,1,mpack_opts.pattern_member_type,verbose);
  /*Fill the root*/
  sat_lgknds[lgknd_sat_type] = root;
  
  /*We have enough resources, run_sat_solver directly*/
  /*If lgknd don't have enough resources, fail it*/
  if (lgknd_sat_type == sat_type) {
    if (1 == run_sat_solver(lgkntwk,ble_arch,ble_info,sat_type,cell_idx,sat_lgknds,timing_analysis,delay_info,mpack_opts.alpha,verbose)) {
      /*Don't forget free resources*/
      free(sat_lgknds);
      return 1;
    }
  }
  //else if (lgknd_sat_type > sat_type) {
    /*We use depth-prior strategy. SAT_solver only consider predecessors have maximum depths*/
  //  if (1 == run_sat_solver(lgkntwk,ble_arch,ble_info,sat_type,cell_idx,sat_lgknds+(lgknd_sat_type-sat_type),verbose)) {
      /*Don't forget free resources*/
 //     free(sat_lgknds);
 //     return 1;
 //   }
//  }
   
  /*Don't forget free resources*/
  free(sat_lgknds);
  return 0;
}

/* Free BLE inputs that has been used by specified logic node,
 * Free cell input muxes in specific cell.
 * REMEMBER: We DON'T free any outputs.
 */ 
int free_lgknd_inputs_in_ble(t_ble_arch* ble_arch,
                             t_ble_info* ble_info,
                             int cell_idx,
                             t_lgknd* lgknd,
                             int verbose)
{
  int im;
  int cnt = 0;

  /*Free cell input muxes*/  
  for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
    ble_info->blend_port_used[cell_idx][im] = 0;
    ble_info->blend_muxes[cell_idx][im] = -1;
    ble_info->blend_lgknd_input_idxes[cell_idx][im] = -1;
  }
  /*Free BLE inputs*/
  for (im=0; im<ble_info->input_num; im++) {
    if (lgknd == ble_info->input_lgknds[im]) {
      ble_info->input_used[im] = 0;
      ble_info->input_lgknds[im] = NULL;
      ble_info->input_lgknds_input_idx[im] = -1;
      /*Update_counter*/
      cnt++;
    }
  }
 
  if (1 == verbose) {
    printf("Info: Free %d lgknd(idx:%d) inputs in BLE(index: %d)\n",cnt,lgknd->idx,ble_info->idx);
  }

  return 1;
}

/* Determine the MAXIMUM sat type a BLE cell can support
 * Iteratively call function check_sat to determine.
 */
int determine_max_sat_type(t_ble_arch* ble_arch,
                           t_ble_info* ble_info,
                           int cell_idx,
                           int verbose)
{
  int max_sat_type = ble_arch->blends[cell_idx].input_num;
  int* sat_idxes = (int*)my_malloc(sizeof(int)*(max_sat_type+1));

  for (max_sat_type=ble_arch->blends[cell_idx].input_num; max_sat_type > 0; max_sat_type--) {
    if (1 == check_sat(max_sat_type,ble_arch,ble_info,&ble_arch->blends[cell_idx],sat_idxes,verbose)) {
      /*Don't forget free resources*/
      free(sat_idxes);
      return max_sat_type;
    }
  }

  /*Don't forget free resources*/
  free(sat_idxes);

  return max_sat_type;
}

int subgraph_legality_check(int num,
                            t_lgknd** subgraph,
                            t_ble_info* ble_info,
                            int verbose) {
  int isrc,ides,ipin;
  int edge_num = 0;
  t_lgknd** edges; 
  int iedge = 0;
  int jedge = 0;
  int absorb = 0;
  t_lgknd* swap;

  // Check each inputs of sub graph, see it can be absorbed in sub_graph,
  // if not, increase the edge_num
  for (isrc=0; isrc<num; isrc++) {
    for (ipin=0; ipin<subgraph[isrc]->input_num; ipin++) {
      // Check Each input pin
      absorb = 0;
      for (ides=0; ides<num; ides++) {
        // Don't check self connected
        if (isrc == ides) {
          if (subgraph[isrc]->inputs[ipin] == subgraph[ides]) {
            printf("Error:(subgraph_legality_check)Self Loop found in lgknd(%d)!\n",subgraph[isrc]->idx);
            exit(1);
          }
          continue;
        }
        if (subgraph[isrc]->inputs[ipin] == subgraph[ides]) { 
          absorb = 1;
          break;
        }
      }
      if (0 == absorb) { // Not absorbed, alloc a edge
        edge_num++;
      } 
    }
  }
  // Alloc edges
  edges = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*edge_num);
  iedge = 0;
  for (isrc=0; isrc<num; isrc++) {
    for (ipin=0; ipin<subgraph[isrc]->input_num; ipin++) {
      // Check Each input pin
      absorb = 0;
      for (ides=0; ides<num; ides++) {
        // Don't check self connected
        if (isrc == ides) {
          if (subgraph[isrc]->inputs[ipin] == subgraph[ides]) {
            printf("Error:(subgraph_legality_check)Self Loop found in lgknd(%d)!\n",subgraph[isrc]->idx);
            exit(1);
          }
          continue;
        }
        if (subgraph[isrc]->inputs[ipin] == subgraph[ides]) { 
          absorb = 1;
          break;
        }
      }
      if (0 == absorb) { // Not absorbed, alloc a edge
        assert(ND_LATCH != subgraph[isrc]->type); // Latches should all be absorbed
        edges[iedge] = subgraph[isrc]->inputs[ipin];
        iedge++;
      } 
    }
  }
  assert(iedge == edge_num);
  // Remove the deplicated edges
  for (iedge=0; iedge<edge_num; iedge++) {
    for (jedge=iedge+1; jedge<edge_num; jedge++) {
      if (edges[iedge] == edges[jedge]) {
        swap = edges[jedge];
        edges[jedge] = edges[edge_num-1];
        edges[edge_num-1] = swap;
        edge_num--;
      }
    }
  }
  // Free memory
  free(edges);
  // Edge_num should not exceed ble_info->input_num
  if (edge_num > ble_info->input_num) {
    return 0;
  } else {
    return 1;
  }
}

int legality_check(int sat_lgknd_num,
                   t_lgknd** sat_lgknds,
                   t_ble_info* ble_info,
                   int verbose) {
  int i,j;
  int free_input_num = 0;
  int new_input_num = 0;
  int edge_num = 0;
  int isrc = 0;
  t_lgknd** src;
  int* absorb;
  t_lgknd* swap;
  int absorb_latch_num = 0;
  t_lgknd** absorb_latch;

  // Find the free input num of ble_info
  for (i=0; i<ble_info->input_num; i++) {
    if (0 == ble_info->input_used[i]) {
      assert(NULL == ble_info->input_lgknds[i]);
      free_input_num++;
    }
  }

  // Get the number of edges
  // Edges of sat_lgknds
  for (i=0; i<sat_lgknd_num; i++) {
    edge_num += sat_lgknds[i]->input_num;
  } 
  // Alloc edges
  isrc = 0;
  src = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*edge_num);
  absorb_latch = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*absorb_latch_num);
  absorb = (int*)my_malloc(sizeof(int)*edge_num);
  for (i=0; i<sat_lgknd_num; i++) {
    for (j=0; j<sat_lgknds[i]->input_num; j++) {
      src[isrc] = sat_lgknds[i]->inputs[j];
      absorb[isrc] = 0;
      isrc++;
    }
  }
  assert(isrc == edge_num);
  // Remove the deplicated src
  for (isrc=0; isrc<edge_num; isrc++) {
    for (j=isrc+1; j<edge_num; j++) {
      if (src[isrc] == src[j]) {
        swap = src[j];
        src[j] = src[edge_num-1];
        src[edge_num-1] = swap;
        edge_num--;
      }
    }
  }
  
  /** If src is 1. among sat_lgknds 2. among the ble_info->input_lgknds,
   *  3. among the ble_info->blend_lgknds 4. among the ble_info->output_lgknds
   *  5. src is a unmapped latch and the input of latch satisfy 1,2,3,4.
   */
  for (isrc=0; isrc<edge_num; isrc++) {
    // Case 1.
    for (i=0; i<sat_lgknd_num; i++) {
      if (sat_lgknds[i] == src[isrc]) {
        absorb[isrc]= 1;
      }
    }
    // Case 2.
    for (i=0; i<ble_info->input_num; i++) {
      if ((1 == ble_info->input_used[i])&&(src[isrc] == ble_info->input_lgknds[i])) {
        absorb[isrc]= 1;
      }
    }
    // Case 3. 
    for (i=0; i<ble_info->blend_num; i++) {
      if ((1 == ble_info->blend_used[i])&&(src[isrc] == ble_info->blend_lgknds[i])) {
        absorb[isrc]= 1;
      }
    }
    // Case 4. 
    for (i=0; i<ble_info->output_num; i++) {
      if ((1 == ble_info->output_used[i])&&(src[isrc] == ble_info->output_lgknds[i])) {
        absorb[isrc]= 1;
      }
    }
    // Case 5.
    if ((0 == src[isrc]->mapped)&&(ND_LATCH == src[isrc]->type)) {
      assert(1 == src[isrc]->input_num);
      // Case 1.
      for (i=0; i<sat_lgknd_num; i++) {
        if (sat_lgknds[i] == src[isrc]->inputs[0]) {
          absorb[isrc]= 1;
        }
      }
      // Case 2.
      for (i=0; i<ble_info->input_num; i++) {
        if ((1 == ble_info->input_used[i])&&(src[isrc]->inputs[0] == ble_info->input_lgknds[i])) {
          absorb[isrc]= 1;
        }
      }
      // Case 3. 
      for (i=0; i<ble_info->blend_num; i++) {
        if ((1 == ble_info->blend_used[i])&&(src[isrc]->inputs[0] == ble_info->blend_lgknds[i])) {
          absorb[isrc]= 1;
        }
      }
      // Case 4. 
      for (i=0; i<ble_info->output_num; i++) {
        if ((1 == ble_info->output_used[i])&&(src[isrc]->inputs[0] == ble_info->output_lgknds[i])) {
          absorb[isrc]= 1;
        }
      }
    }
  }
  // Check unabsorbed - new_input_num
  for (isrc=0; isrc<edge_num; isrc++) {
    if (absorb[isrc] == 0) {
      new_input_num++;
    }
  }
  // Update - free_input_num
  for (i=0; i<sat_lgknd_num; i++) {
    for (j=0; j<sat_lgknds[i]->output_num; j++) {
      for (isrc=0; isrc<ble_info->blend_num; isrc++) {
        if ((1 == ble_info->blend_used[isrc])&&(sat_lgknds[i]->outputs[j] == ble_info->blend_lgknds[isrc])) {
          free_input_num++;
        }
      }
    }
  }

  free(src);
  free(absorb);
  free(absorb_latch);
  // Legal: free_input_num >= new_input_num
  if (free_input_num < new_input_num) {
    return 0;
  } else {
    return 1;
  }
}

int lgknd_input_sum_weight_subgraph(int num,
                                    t_lgknd** sub_graph,
                                    t_lgknd* lgknd,
                                    int lgknd_idx,
                                    int input_idx,
                                    int* sub_graph_fanout_absorb,
                                    float* sum_timing_weight,
                                    float* sum_depth_weight_input,
                                    float* sum_depth_weight_output) {
  int k,l; 
  int ret = 0;
  int in_subgraph = 0;
 
  for (k=0; k<num; k++) { // Search sub_graph
    in_subgraph = 0;
    if (lgknd->inputs[input_idx] == sub_graph[k]) { // Output Absorbed
      if (lgknd_idx == k) {
        printf("Error: An internal loop is found for lgknd(index=%d)!",lgknd->idx);
        exit(1);
      } else {
        sub_graph_fanout_absorb[k]--;
        *sum_timing_weight += lgknd->in_edges[input_idx].timing_weight;
        *sum_depth_weight_output += lgknd->in_edges[input_idx].depth_weight;
        //*sum_depth_weight_input += lgknd->in_edges[input_idx].depth_weight;
        ret = 1;
        in_subgraph = 1;
      }
    }
    //*sum_timing_weight += lgknd->in_edges[input_idx].timing_weight;
    if (lgknd_idx != k) { // Seach shared inputs
      for (l=0; l<sub_graph[k]->input_num; l++) {
        if (lgknd->inputs[input_idx] == sub_graph[k]->inputs[l]) { // Input shared
          if (0 == in_subgraph) {
            *sum_timing_weight += lgknd->in_edges[input_idx].timing_weight;
            *sum_depth_weight_input += lgknd->in_edges[input_idx].depth_weight;
          }
          ret = 1;
        }
      }
    }
  }
  return ret;
}

int latch_absorb_in_subgraph(int num,
                             t_lgknd** sub_graph,
                             t_lgknd* latch) {
  int i,j;
  int pred_fanout_in_graph = 0;
  int pred_in_subgraph = 0;
  //Only 1 input for latch
  assert(1 == latch->input_num);
  for (i=0; i<num; i++) {
    if (latch->inputs[0] == sub_graph[i]) {
      // Find predecessor in sub_graph 
      pred_in_subgraph = 1;
    }
  } 
  // Predecessor isn't in the graph 
  if (0 == pred_in_subgraph) {
    return 0;
  }
  // Predecessor in the graph
  // Search the fanout of predecessor in the graph
  for (j=0; j<latch->inputs[0]->output_num; j++) {
    for (i=0; i<num; i++) {
      if ((ND_LATCH != latch->inputs[0]->outputs[j]->type)&&(latch->inputs[0]->outputs[j] == sub_graph[i])) { 
        pred_fanout_in_graph++;
      }
    }
  }
  if ((latch->inputs[0]->output_num-1) == pred_fanout_in_graph) {
    return 1;
  } else {
    return 0;
  }
}

float sat_lgknds_ble_average_weight(int sat_lgknd_num,
                                    t_lgknd** sat_lgknds,
                                    t_ble_info* ble_info,
                                    float alpha,
                                    float beta,
                                    enum e_packer_area_attraction packer_area_attraction,
                                    float* paths_affect,
                                    int verbose) {
  float sum_depth_weight_input = 0.0;
  float sum_depth_weight_output = 0.0;
  float sum_timing_weight = 0.0;
  int num = 0;
  t_lgknd** sub_graph = NULL;
  int i,j,cur_lgknd;
  int average_num = 0;
  int max_net = ble_info->input_num + ble_info->output_num;
  int* sub_graph_fanout_absorb;

  // We construct a sub graph including all sat_lgknds and lgknds already packed in ble_info
  num += sat_lgknd_num;
  average_num += sat_lgknd_num;
  // Search mapped
  for (i=0; i<ble_info->blend_num; i++) {
    if (1 == ble_info->blend_used[i]) {
      num++;
    }
  }
  // For output_lgknds, only latch should be counted in.
  for (i=0; i<ble_info->output_num; i++) {
    if ((1 == ble_info->output_used[i])&&(ND_LATCH == ble_info->output_lgknds[i]->type)) {
      num++;
    }
  }
  // Alloc sub graph
  cur_lgknd = 0;
  sub_graph = (t_lgknd**)my_malloc(num*sizeof(t_lgknd*));
  for (i=0; i<sat_lgknd_num; i++) {
    assert(NULL != sat_lgknds[i]);
    sub_graph[cur_lgknd] = sat_lgknds[i];
    cur_lgknd++;
  }
  
  // Search mapped
  for (i=0; i<ble_info->blend_num; i++) {
    if (1 == ble_info->blend_used[i]) {
      sub_graph[cur_lgknd] = ble_info->blend_lgknds[i];
      cur_lgknd++;
    }
  }
  // For output_lgknds, only latch should be counted in.
  for (i=0; i<ble_info->output_num; i++) {
    if ((1 == ble_info->output_used[i])&&(ND_LATCH == ble_info->output_lgknds[i]->type)) {
      sub_graph[cur_lgknd] = ble_info->output_lgknds[i];
      cur_lgknd++;
    }
  }
  assert(cur_lgknd == num);
  // Search the fanout latches and see if they can be absorbed in BLE
  for (i=0; i<sat_lgknd_num; i++) {
    for (j=0; j<sat_lgknds[i]->output_num; j++) {
      if ((ND_LATCH == sat_lgknds[i]->outputs[j]->type)&&(1 == latch_absorb_in_subgraph(num,sub_graph,sat_lgknds[i]->outputs[j]))) {
        num++;
        average_num++;
        sub_graph = (t_lgknd**)my_realloc(sub_graph,num*sizeof(t_lgknd*));
        sub_graph[cur_lgknd] = sat_lgknds[i]->outputs[j];
        cur_lgknd++;
        //lgknd_input_sum_weight_subgraph(num,sub_graph,sat_lgknds[i]->outputs[j],-1,0,&sum_weight_input,&sum_weight_output);
      }
    }
  }
  // Realloc the subgraph
  //if (num != cur_lgknd) {
  //  sub_graph = (t_lgknd**)my_realloc(sub_graph,num*sizeof(t_lgknd*));
    // Search the fanout latches and see if they can be absorbed in BLE
  //  for (i=0; i<sat_lgknd_num; i++) {
  //    for (j=0; j<sat_lgknds[i]->output_num; j++) {
  //      if ((ND_LATCH == sat_lgknds[i]->outputs[j]->type)&&(1 == latch_absorb_in_subgraph(num,sub_graph,sat_lgknds[i]->outputs[j]))) {
   //       sub_graph[cur_lgknd] = sat_lgknds[i]->outputs[j];
   //       cur_lgknd++;
          //lgknd_input_sum_weight_subgraph(num,sub_graph,sat_lgknds[i]->outputs[j],-1,0,&sum_weight_input,&sum_weight_output);
  //      }
  //    }
  //  }
  //}
  assert(cur_lgknd == num);
  // Legality checking 
  if (0 == subgraph_legality_check(num,sub_graph,ble_info,verbose)) {
    free(sub_graph);
    return -1.0; 
  }
  
  sub_graph_fanout_absorb = (int*)my_malloc(num*sizeof(int));
  for (i=0; i< num; i++) {
    sub_graph_fanout_absorb[i] = sub_graph[i]->output_num;
  }
  // Find each inputs is absorbed in the sub_graph
  for (i=0; i<num; i++) {
    for (j=0; j<sub_graph[i]->input_num; j++) {
      // Check those unmapped or predecessor unmapped.
      if (!((1 == sub_graph[i]->mapped)&&(1 == sub_graph[i]->inputs[j]->mapped))) {
      //if (0 == sub_graph[i]->mapped) {
        lgknd_input_sum_weight_subgraph(num,sub_graph,sub_graph[i],i,j,sub_graph_fanout_absorb,&sum_timing_weight,&sum_depth_weight_input,&sum_depth_weight_output);
      }
    }
  }
  // sum_depth_weight_output = sum(1/non_absorb_fanout)
  if (PACKER_AREA_ATTRACTION_AAPACK == packer_area_attraction) {
    sum_depth_weight_input += sum_depth_weight_output;
    sum_depth_weight_output = 0.0;
    for (i=0; i<num; i++) {
      assert((sub_graph_fanout_absorb[i] > 0)||(sub_graph_fanout_absorb[i] == 0));
      sum_depth_weight_output += (float)sub_graph_fanout_absorb[i];
    }
    sum_depth_weight_output = 1/sum_depth_weight_output;
  }

  (*paths_affect) = sat_lgknds_ble_paths_affect(num,sub_graph,ble_info,verbose)/sat_lgknd_num;
  //(*paths_affect) = sat_lgknds_ble_paths_affect(num,sub_graph,ble_info,verbose)/average_num;
  //(*paths_affect) = sat_lgknds_ble_paths_affect(num,sub_graph,ble_info,verbose);

  // Free the subgraph
  free(sub_graph);
  free(sub_graph_fanout_absorb);
 
  // Return average weight
  if (0 == num) {
    return -1.0;
  } else {
    return ((1-alpha)*(beta*sum_depth_weight_output+(1-beta)*sum_depth_weight_input)/max_net + alpha*sum_timing_weight); 
  }
}

/** Get Path Affect of those lgknds on Critical Path
 * Two Paths: 
 * 1. From source to current BLE
 * 2. From sink to current BLE
 * Additional: perfer the lgknds with larger depth
 */
float sat_lgknds_ble_paths_affect(int sat_lgknd_num,
                                  t_lgknd** sat_lgknds,
                                  t_ble_info* ble_info,
                                  int verbose) {
  int i;
  int sum_depth = 0;
  float epsilon = 0.001;
  float input_paths_affect = 0.0;
  float output_paths_affect = 0.0;
  int max_ble_depth = 0;
  int max_sat_lgknd_depth = 0;
  // Sum
  for (i=0; i<sat_lgknd_num; i++) {
    if (sat_lgknds[i]->ble_info == ble_info) {
      assert(sat_lgknds[i]->mapped == 1);
      if ((0 == max_ble_depth)||(max_ble_depth < sat_lgknds[i]->depth_source)) {
        max_ble_depth = sat_lgknds[i]->depth_source;
      }
      continue;
    }
    if ((0 == max_sat_lgknd_depth)||(max_sat_lgknd_depth < sat_lgknds[i]->depth_source)) {
      max_sat_lgknd_depth = sat_lgknds[i]->depth_source;
    }
    sum_depth += sat_lgknds[i]->depth_source;
    input_paths_affect += sat_lgknds[i]->input_paths_affect;
    output_paths_affect += sat_lgknds[i]->output_paths_affect;
    //printf("DEBUG: input_paths_affect=%.3g,output_paths_affect=%.3g.\n",input_paths_affect,output_paths_affect);
  }

  return (input_paths_affect+output_paths_affect) + epsilon*(float)(sum_depth); 
}


/* Find a lgknd from lgkntwk which match sat_type
 * Return the Pointer of unmapped lgknd whose try_sat = 0 with largest attract 
 * to current BLE info.
 */
t_lgknd* pick_sat_lgknd_from_lgkntwk(t_lgkntwk* lgkntwk,
                                     int sat_type,
                                     t_ble_info* ble_info,
                                     float alpha,
                                     float beta,
                                     enum e_pattern_member_type pattern_member_type,
                                     enum e_packer_area_attraction packer_area_attraction,
                                     int verbose)
{
  int im; 
  float ble_average_weight = -1.0;
  float ble_org_average_weight = -1.0;
  float average_weight = -1.0;
  float paths_affect = 0.0;
  float temp_paths_affect = 0.0;
  //int temp_sat_type;
  t_lgknd** sat_lgknds= (t_lgknd**)my_malloc(sizeof(t_lgknd*)*(sat_type+1));
  //int ret_sat_type = 0;
  t_lgknd* ret = NULL;
  
  // We calculate the average weight of current ble first... 
  ble_org_average_weight = sat_lgknds_ble_average_weight(0,NULL,ble_info,alpha,beta,packer_area_attraction,&temp_paths_affect,verbose);
  //printf("DEBUG: ble_average_weight = %.2f\n",ble_average_weight);

  for (im=0; im<lgkntwk->libgate_num; im++) {
    if ((0 == lgkntwk->libgate_ptrs[im]->mapped)&&(0 == lgkntwk->libgate_ptrs[im]->try_sat)) {
      if (sat_type != lgknds_sat_type(lgkntwk->libgate_ptrs[im],NULL,0,pattern_member_type,verbose)) {
        // Pass the sat_type we cannot satisfy
        continue;
      }
      // Copy sat_lgknds
      lgknds_sat_type(lgkntwk->libgate_ptrs[im],sat_lgknds,1,pattern_member_type,verbose); 
      sat_lgknds[sat_type] = lgkntwk->libgate_ptrs[im];
      // Try each possible solution 
      //for (j=0; j<temp_sat_type; j++) { 
      /*Legality checking should be added here*/
      average_weight = sat_lgknds_ble_average_weight(sat_type+1,sat_lgknds,ble_info,alpha,beta,packer_area_attraction,&temp_paths_affect,verbose);
      if (-1.0 == average_weight) {//||(0.0 == average_weight)) {
        continue;
      }
      //if ((-1.0 != ble_org_average_weight)&&(0.0 == average_weight)) {
      //  continue;
      //}
      if (ble_average_weight < average_weight) {
      //if ((ble_average_weight < average_weight)||(ble_average_weight == average_weight)) {
        //printf("DEBUG: Average_weight=%.3g,\tPaths Affect=%.3g\n",average_weight,temp_paths_affect);
        ble_average_weight = average_weight;
        paths_affect = temp_paths_affect; 
        ret = lgkntwk->libgate_ptrs[im];
      } else if (ble_average_weight == average_weight) {
        if ((0.0 != ble_org_average_weight)&&(-1.0 != ble_org_average_weight)&&((temp_paths_affect > paths_affect)||(temp_paths_affect == paths_affect))) {
        ble_average_weight = average_weight;
        paths_affect = temp_paths_affect; 
        ret = lgkntwk->libgate_ptrs[im];
        } 
      }
    }
  }

  /*Don't forget free resources*/
  free(sat_lgknds); 

  return ret; 
}

int determine_unused_blend_num(t_ble_arch* ble_arch,
                               t_ble_info* ble_info,
                               int verbose) {
  int ret = 0;
  int i;
  
  assert(ble_arch->blend_num == ble_info->blend_num);
  for (i=0; i<ble_arch->blend_num; i++) {
    if (0 == ble_info->blend_used[i]) {
      ret++;
    }
  }
  return ret;
}

/* Find a lgknd from lgkntwk which match sat_type
 * Return the Pointer of unmapped lgknd whose try_sat = 0 with largest attract 
 * to current BLE info.
 */
int pick_pattern_fit_ble(t_lgkntwk* lgkntwk,
                         int arch_pattern_type,
                         t_lgknd** pattern_lgknds,// Remember to free it otherwhere 
                         t_ble_info* ble_info,
                         float alpha,
                         float beta,
                         enum e_pattern_member_type pattern_member_type,
                         enum e_packer_area_attraction packer_area_attraction,
                         int forbid_unrelated_clustering,
                         int verbose) {
  int im; 
  float ble_average_weight = -1.0;
  float ble_org_average_weight = -1.0;
  float average_weight = -1.0;
  float paths_affect = 0.0;
  float temp_paths_affect = 0.0;
  int temp_pattern_type,itype;
  t_lgknd** temp_pattern_lgknds = NULL;
  t_lgknd** temp = NULL;
  int pattern_type = 0;
  //int iptr;
  
  //if (pattern_lgknds != NULL) {
  // free(pattern_lgknds);
  //}
  //pattern_lgknds = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*(max_pattern_type));
  temp_pattern_lgknds = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*(arch_pattern_type));
  memset(temp_pattern_lgknds,0,arch_pattern_type*sizeof(t_lgknd*));
  // We calculate the average weight of current ble first... 
  ble_org_average_weight = sat_lgknds_ble_average_weight(0,NULL,ble_info,alpha,beta,packer_area_attraction,&temp_paths_affect,verbose);
  //assert((0.0 == ble_org_average_weight)||(-1.0 == ble_org_average_weight));
  //printf("DEBUG: ble_average_weight = %.2f\n",ble_average_weight);

  for (im=0; im<lgkntwk->libgate_num; im++) {
    // Pass mapped
    if (1 == lgkntwk->libgate_ptrs[im]->mapped) {
      continue;
    }
    temp_pattern_type = 1 + lgknds_sat_type(lgkntwk->libgate_ptrs[im],NULL,0,pattern_member_type,verbose);// return is the number of predecessors
    if (temp_pattern_type > arch_pattern_type) {
      //copy least depth nodes
      temp = (t_lgknd**)my_malloc(sizeof(t_lgknd*)*temp_pattern_type);
      memset(temp,0,temp_pattern_type*sizeof(t_lgknd*));
      lgknds_sat_type(lgkntwk->libgate_ptrs[im],temp,1,pattern_member_type,verbose); 
      temp[temp_pattern_type-1] = lgkntwk->libgate_ptrs[im];
      memset(temp_pattern_lgknds,0,arch_pattern_type*sizeof(t_lgknd*));
      memcpy(temp_pattern_lgknds,temp+temp_pattern_type-arch_pattern_type,sizeof(t_lgknd*)*arch_pattern_type); 
      temp_pattern_type = arch_pattern_type;
      // free
      free(temp);
    } else {
      // Copy pattern_lgknds, [preds...,pattern_root]
      memset(temp_pattern_lgknds,0,arch_pattern_type*sizeof(t_lgknd*));
      lgknds_sat_type(lgkntwk->libgate_ptrs[im],temp_pattern_lgknds,1,pattern_member_type,verbose); 
      temp_pattern_lgknds[temp_pattern_type-1] = lgkntwk->libgate_ptrs[im];
    }
    assert(!(temp_pattern_type > (lgkntwk->libgate_ptrs[im]->input_num + 1)));
    // Try each possible solution 
    for (itype = temp_pattern_type; itype > 0; itype--) {
      assert((0 == lgkntwk->libgate_ptrs[im]->try_pattern_types[itype-1])||(1 == lgkntwk->libgate_ptrs[im]->try_pattern_types[itype-1]));
      if (1 == lgkntwk->libgate_ptrs[im]->try_pattern_types[itype-1]) {
        continue;
      }
      assert(((temp_pattern_type-itype) == 0)||((temp_pattern_type-itype) > 0));
      // Mark try_pattern_types
      //lgkntwk->libgate_ptrs[im]->try_pattern_types[itype-1] = 1;
      /*Legality checking, attraction calculating*/
      average_weight = sat_lgknds_ble_average_weight(itype,
                                                     temp_pattern_lgknds+temp_pattern_type-itype, // Need part of pattern_lgknds
                                                     ble_info,
                                                     alpha,
                                                     beta,
                                                     packer_area_attraction,
                                                     &temp_paths_affect,
                                                     verbose);
      // Average when we are finding a seed
      if (-1.0 == ble_org_average_weight) {
        // Target local minimal average_weight
        average_weight = average_weight/itype;
      } else if ((0.0 == average_weight)&&(1 == forbid_unrelated_clustering)) {
        // Forbid zero-related pattern
        lgkntwk->libgate_ptrs[im]->try_pattern_types[itype-1] = 1;
        continue;
      } 
      //if (-1.0 == average_weight) { //||(0.0 == average_weight)) {
        // Mark try_pattern_types
      //  lgkntwk->libgate_ptrs[im]->try_pattern_types[itype-1] = 1;
      //  continue;
      //}
      if (ble_average_weight < average_weight) {
      //if ((ble_average_weight < average_weight)||(ble_average_weight == average_weight)) {
        ble_average_weight = average_weight;
        paths_affect = temp_paths_affect; 
        pattern_type = itype; 
        memset(pattern_lgknds,0,arch_pattern_type*sizeof(t_lgknd*));
        // Set the length correctly! if you want to copy a pointer array, size should be the number*pointer_size
        memcpy(pattern_lgknds,temp_pattern_lgknds+temp_pattern_type-itype,itype*sizeof(t_lgknd*)); 
      } else if (ble_average_weight == average_weight) {
        if ((0.0 != ble_org_average_weight)&&(-1.0 != ble_org_average_weight)&&((temp_paths_affect > paths_affect)||(temp_paths_affect == paths_affect))) {//When ble is blank, we relax the constraint
          ble_average_weight = average_weight;
          paths_affect = temp_paths_affect; 
          pattern_type = itype; 
          memset(pattern_lgknds,0,arch_pattern_type*sizeof(t_lgknd*));
          memcpy(pattern_lgknds,temp_pattern_lgknds+temp_pattern_type-itype,itype*sizeof(t_lgknd*)); 
        }
      } 
    }
  }

  /*Don't forget free resources*/
  free(temp_pattern_lgknds); 

  return pattern_type; 
}

