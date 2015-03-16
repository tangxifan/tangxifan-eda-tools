#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "blif_parser.h"
#include "blearch_parser.h"
#include "mpack_types.h"
#include "mpack_conf.h"
#include "mpack_util.h"
#include "setup_mpack.h"
#include "init_mpack_types.h"
#include "global.h"


/*****Subroutines*****/
void echo_libgate_stats(t_stats* stats)
{
  int i = 0;

  /*Echo Gate Statistics*/ 
  printf("**Standard Cell Statistics**\n");
  //while(NULL != typepair_list[i].type_name)
  for(i=0; i<12; i++)
  {
    printf("No. of %s : %d.\n",typepair_list[i].type_name,stats->stdgate_stats[typepair_list[i].type_id]);
   // i++;
  }
  printf("No. of LATCH : %d.\n", stats->latch_num);
  printf("**END of Std Cell Statistics**\n");
}

int setup_mpack(t_blif_arch* blif_arch,
                t_lgkntwk* lgkntwk,
                t_stats* stats,
                int read_stdlib_blif)
{
  /*Initial the stats struct*/
  printf("Initial statistics...");
  init_stats(stats);
  init_lgkntwk(lgkntwk);
  printf("Completed!\n");
  /*Check if the input blif netlist fits mpack needs*/
  if (1 == read_stdlib_blif) {
    printf("Read Standard Library synthesised blif.\n");
    printf("Check whether blif fits mpack...\n");
    check_blif_netlist(blif_arch);
    /*Map libgates,latches, PI and PO to logic node structs, build the network*/
    printf("Transform blif network into mpack types...\n");
    assign_logic_nodes(blif_arch,lgkntwk,stats);
 
    echo_libgate_stats(stats);
  } else {
    printf("Read FPGA synthesised blif.\n");
    printf("Check whether blif fits mpack...\n");
    check_lut_blif_netlist(blif_arch);
    printf("Transform blif network into mpack types...\n");
    assign_lut_logic_nodes(blif_arch,lgkntwk,stats);
  }

  return 1;
}

int check_lut_blif_netlist(t_blif_arch* blif_arch) {
  if (blif_arch->libgate_num !=0) {
    printf("Error: standard library gates(.gate) are not supported!\n");
    exit(1);
  }
  /*Ensure there is at least one lut*/ 
  if (0 == blif_arch->gate_num) {
    printf("Warning: no LUTs have been found!Packing results may be incorrect!\n");
  }
  /*Check if it is combinational circuit*/
  if (0 == blif_arch->latch_num) {
    printf("Info: 0 latch found => Combinational Circuit!\n");
  }
  if ((0 == blif_arch->gate_num)&&(0 == blif_arch->latch_num)) {
    printf("Error: 0 latch and LUT found! Invalid blif input!\n");
    exit(1);
  }
  return 1;
}

/* check blif netlist
 * Check Details:
 * 1. lib-gates only! Ensure there is no gates(.names)!
 */

int check_blif_netlist(t_blif_arch* blif_arch)
{

  /*Ensure there are no gates(.names)*/
  if (blif_arch->gate_num != 0)
  {
    printf("Error: LUTs(.names) are not support!\n");
    exit(1);
  }
  
  /*Ensure there is at least one lib-gate*/ 
  if (0 == blif_arch->libgate_num)
  {
    printf("Warning: no libgates have been found!Packing results may be incorrect!\n");
  }
  /*Check if it is combinational circuit*/
  if (0 == blif_arch->latch_num)
  {
    printf("Info: 0 latch found => Combinational Circuit!\n");
  }
  
  if ((0 == blif_arch->libgate_num)&&(0 == blif_arch->latch_num))
  {
    printf("Error: 0 latch and libgate found! Invalid blif input!\n");
    exit(1);
  }

  return 1;
}

int assign_lut_logic_nodes(t_blif_arch* blif_arch,
                           t_lgkntwk* lgkntwk,
                           t_stats* stats)
{
  /* 1st step: allocate memory of logic nodes for
   *           libgates, latches, PI and PO.
   *           shape array for their pointors
   * Important: I make the index of logic nodes in    
   *            each list equals to the index in 
   *            blif_arch(libgates,latches, PIs, and POs)
   */
  assign_lut(lgkntwk,blif_arch,stats);
  assign_pi(lgkntwk,blif_arch,stats);
  assign_po(lgkntwk,blif_arch,stats);
  assign_latch(lgkntwk,blif_arch,stats);

  /* 2nd step: assign inputs and outputs for
   *           libgates, latches, PI and PO. 
   */
  assign_conkts_to_luts(blif_arch,lgkntwk);
  /*Fix problems about the libgates and latches connected to PO*/
  assign_libgate_latch_conkt_po(blif_arch,lgkntwk); 

  clear_po_outputs(lgkntwk);

  /*Assign Net name for each lgknds*/
  assign_lut_lgknds_net_name(blif_arch,lgkntwk);
  update_lut_stats(blif_arch,lgkntwk,stats);

  return 1;
}

int update_lut_stats(t_blif_arch* blif_arch,
                     t_lgkntwk* lgkntwk,
                     t_stats* stats) {
  int i;
  // Initial
  assert(stats->max_lut_size == 0);
  assert(stats->stdgate_stats[ND_BUF] == 0);
  assert(stats->stdgate_stats[ND_LUT] == 0);
  // Update stats
  for (i = 0; i < blif_arch->gate_num; i++) {
    assert(ND_LUT == lgkntwk->libgate_ptrs[i]->type);
    stats->stdgate_stats[ND_LUT]++;
    /*identify BUF LUT*/
    if (1 == blif_arch->gates[i].is_buf) {
      lgkntwk->libgate_ptrs[i]->type = ND_BUF;
      stats->stdgate_stats[ND_LUT]--;
      stats->stdgate_stats[ND_BUF]++;
    }
    // Update max_lut_size
    if ((0 == stats->max_lut_size)||(stats->max_lut_size < lgkntwk->libgate_ptrs[i]->input_num)) {
      stats->max_lut_size = lgkntwk->libgate_ptrs[i]->input_num;
    }
  }
  // Stats for LUT size
  assert(stats->lut_stats == NULL);
  stats->lut_stats = (int*)my_malloc((stats->max_lut_size+1)*sizeof(int));
  my_init_int_ary(stats->max_lut_size+1,stats->lut_stats,0);
  printf("MAX LUT size = %d\n",stats->max_lut_size);
  for (i = 0; i < blif_arch->gate_num; i++) {
    assert((ND_BUF == lgkntwk->libgate_ptrs[i]->type)||(ND_LUT == lgkntwk->libgate_ptrs[i]->type));
    stats->lut_stats[lgkntwk->libgate_ptrs[i]->input_num]++;
  } 
  printf("LUT stats:\n");
  printf("LUT No. = %d\n",stats->libgate_num);
  printf("LUT (Not BUF) No. = %d\n",stats->stdgate_stats[ND_LUT]);
  printf("LUT BUF No. = %d\n",stats->stdgate_stats[ND_BUF]);
  for (i=0; i<(stats->max_lut_size+1); i++) {
    printf("No. %d-LUT = %d\n",i,stats->lut_stats[i]);
  }
  printf("Latch No. = %d\n",stats->latch_num);
  return 1;
}

/**
 * Assign the libgates, latches, Primary Input,
 * and Primary Outputs to logic nodes.
 * Meanwhile check the libgates type and input numbers
 */
int assign_logic_nodes(t_blif_arch* blif_arch,
                       t_lgkntwk* lgkntwk,
                       t_stats* stats)
{
  /* 1st step: allocate memory of logic nodes for
   *           libgates, latches, PI and PO.
   *           shape array for their pointors
   * Important: I make the index of logic nodes in    
   *            each list equals to the index in 
   *            blif_arch(libgates,latches, PIs, and POs)
   */
  assign_libgate(lgkntwk,blif_arch,stats);
  assign_pi(lgkntwk,blif_arch,stats);
  assign_po(lgkntwk,blif_arch,stats);
  assign_latch(lgkntwk,blif_arch,stats);

  /* 2nd step: assign inputs and outputs for
   *           libgates, latches, PI and PO. 
   */
  assign_conkts_to_lgknds(blif_arch,lgkntwk);
  /*Fix problems about the libgates and latches connected to PO*/
  assign_libgate_latch_conkt_po(blif_arch,lgkntwk); 

  clear_po_outputs(lgkntwk);

  /*Assign Net name for each lgknds*/
  assign_lgknds_net_name(blif_arch,lgkntwk);

  return 1;
}

/**
 * Save the ass when we assign outputs of 
 * latches and libgates, whose outputs is a PO.
 * In logic network, PO should have zero output.
 */
int clear_po_outputs(t_lgkntwk* lgkntwk)
{
  int im;

  for (im=0; im<lgkntwk->po_num; im++)
  {
    lgkntwk->po_ptrs[im]->output_num = 0;
    free(lgkntwk->po_ptrs[im]->outputs);
  }

  return 1;
}

int assign_conkts_to_luts(t_blif_arch* blif_arch,
                          t_lgkntwk* lgkntwk) {
  t_conkt* head = blif_arch->nets;
  t_lgknd** libgate_ptrs = lgkntwk->libgate_ptrs;
  t_lgknd** latch_ptrs = lgkntwk->latch_ptrs; 
  t_lgknd** pi_ptrs = lgkntwk->pi_ptrs;
  t_lgknd** po_ptrs = lgkntwk->po_ptrs;

  t_lgknd** ptr_lst[4];

  ptr_lst[0] = NULL;
  ptr_lst[1] = NULL;
  ptr_lst[2] = lgkntwk->latch_ptrs; 
  ptr_lst[3] = lgkntwk->libgate_ptrs;
  
  /** Two steps!
   * 1st step: configure the PI and PO first
   * 2nd step: configure the libgates, and latches
   */

  /* Task list of each round
   * 1. if net name match a pi, map to the pi lgknd
   * 2. if net name match a po, map to the po lgknd
   * 3. neither pi nor po, net is a internal connection. leave it to be processed in the second step 
   */
  while(head->next) {
    head = head->next;
    /*Connect the PI with logic nodes*/
    if (assign_pi_conkts(blif_arch,head,pi_ptrs,ptr_lst)) {
      continue;
    }
    /*Connect the PO with logic nodes*/
    if (assign_po_conkts(blif_arch,head,po_ptrs,ptr_lst)) {
      continue;
    }
  }

  /* Connect the libgates and latches!*/
  assign_lut_conkts(blif_arch,libgate_ptrs,latch_ptrs,pi_ptrs,po_ptrs); 
  assign_lut_latch_conkts(blif_arch,libgate_ptrs,latch_ptrs,pi_ptrs,po_ptrs); 

  return 1;
}


/*Assign the connections to logic nodes*/
/*Make logic nodes connected*/
int assign_conkts_to_lgknds(t_blif_arch* blif_arch,
                            t_lgkntwk* lgkntwk)
{
  t_conkt* head = blif_arch->nets;
  //int i;
  //int ind;
  t_lgknd** libgate_ptrs = lgkntwk->libgate_ptrs;
  t_lgknd** latch_ptrs = lgkntwk->latch_ptrs; 
  t_lgknd** pi_ptrs = lgkntwk->pi_ptrs;
  t_lgknd** po_ptrs = lgkntwk->po_ptrs;

  t_lgknd** ptr_lst[3];

  ptr_lst[0] = NULL;
  ptr_lst[1] = lgkntwk->libgate_ptrs;
  ptr_lst[2] = lgkntwk->latch_ptrs; 
  
  /** Two steps!
   * 1st step: configure the PI and PO first
   * 2nd step: configure the libgates, and latches
   */

  /* Task list of each round
   * 1. if net name match a pi, map to the pi lgknd
   * 2. if net name match a po, map to the po lgknd
   * 3. neither pi nor po, net is a internal connection. leave it to be processed in the second step 
   */
  while(head->next)
  {
    head = head->next;
    /*Connect the PI with logic nodes*/
    if (assign_pi_conkts(blif_arch,head,pi_ptrs,ptr_lst)) 
    {continue;}
    /*Connect the PO with logic nodes*/
    if (assign_po_conkts(blif_arch,head,po_ptrs,ptr_lst)) 
    {continue;}
  }

  /* Connect the libgates and latches!*/
  assign_libgate_conkts(blif_arch,libgate_ptrs,latch_ptrs,pi_ptrs,po_ptrs); 
  assign_latch_conkts(blif_arch,libgate_ptrs,latch_ptrs,pi_ptrs,po_ptrs); 

  return 1;
}

int assign_lut_conkts(t_blif_arch* blif_arch,
                      t_lgknd** libgate_ptrs,
                      t_lgknd** latch_ptrs,
                      t_lgknd** pi_ptrs,
                      t_lgknd** po_ptrs) {
  int igate;
  int im;
  int tmp_idx;

  t_lgknd** ptr_lst[4] = {NULL, NULL, latch_ptrs,libgate_ptrs};
  t_lgknd** base_addr;
  
  /* Assign each libgate*/
  for (igate=0; igate<blif_arch->gate_num; igate++) {
    /*Assign input number and output number*/
    libgate_ptrs[igate]->input_num = blif_arch->gates[igate].input_num; 
    libgate_ptrs[igate]->output_num = blif_arch->gates[igate].output.conkt->output_num;

    /**
     * Special Case: if latch is connected to a PO,
     * PO may has zero outputs number.
     */
    if (blif_arch->gates[igate].output.conkt->is_po) {
      libgate_ptrs[igate]->output_num++;
    }

    /*Allocate memory*/
    libgate_ptrs[igate]->inputs = (t_lgknd**)my_malloc(libgate_ptrs[igate]->input_num*sizeof(t_lgknd*));
    libgate_ptrs[igate]->outputs = (t_lgknd**)my_malloc(libgate_ptrs[igate]->output_num*sizeof(t_lgknd*));
    /* Assign each inputs*/
    for (im=0; im<libgate_ptrs[igate]->input_num; im++) {
      /*Check if this is a PI*/
      if (blif_arch->gates[igate].inputs[im].conkt->is_pi) {
        tmp_idx = blif_arch->gates[igate].inputs[im].conkt->pi_idx;
        /*This is a PI! Map connections to PI*/
        libgate_ptrs[igate]->inputs[im] = pi_ptrs[tmp_idx];
      } else if (blif_arch->gates[igate].inputs[im].conkt->is_po) {
        /*Check if this is a PO*/
        tmp_idx = blif_arch->gates[igate].inputs[im].conkt->po_idx;
        /*This is a PI! Map connections to PI*/
        libgate_ptrs[igate]->inputs[im] = po_ptrs[tmp_idx];
      } else {
        /* The input is connected to a gate or latch*/
        /*Find the conkt and spot its input!*/
        base_addr = ptr_lst[blif_arch->gates[igate].inputs[im].conkt->inputs_types];
        libgate_ptrs[igate]->inputs[im] = *(base_addr + blif_arch->gates[igate].inputs[im].conkt->inputs);
      }
    }
    /*Assign outputs*/
    /*If output is a PO, PO may has zero output*/    
    if (blif_arch->gates[igate].output.conkt->is_po) {
      tmp_idx = blif_arch->gates[igate].output.conkt->po_idx;
      libgate_ptrs[igate]->outputs[libgate_ptrs[igate]->output_num-1] = po_ptrs[tmp_idx];
    }
    /*Assign Rest*/ 
    for(im=0; im<blif_arch->gates[igate].output.conkt->output_num; im++)
    {
      base_addr = ptr_lst[blif_arch->gates[igate].output.conkt->outputs_types[im]];
      libgate_ptrs[igate]->outputs[im] = *(base_addr + blif_arch->gates[igate].output.conkt->outputs[im]);
    }
  }

  return 1; 
}


int assign_libgate_latch_conkt_po(t_blif_arch* blif_arch,
                                  t_lgkntwk* lgkntwk)
{
  int im;
  int ip;
  t_lgknd* pre;
  t_lgknd* po;

  /*Traversal all libgates*/
  for (im=0; im<lgkntwk->libgate_num; im++) {
    /*Check if its inputs are connected to PO*/
    for (ip=0; ip<lgkntwk->libgate_ptrs[im]->input_num; ip++) {
      if (ND_PO == lgkntwk->libgate_ptrs[im]->inputs[ip]->type) {
        /*We find one of the problem, fix it!*/
        po = lgkntwk->libgate_ptrs[im]->inputs[ip];
        /*Find the gate we should realloc its output*/
        pre = po->inputs[0];
        //pre->outputs = (t_lgknd**)my_realloc(sizeof(t_lgknd*)*(1 + pre->output_num));
        //pre->outputs[pre->output_num] = lgkntwk->libgate_ptrs[im];
        //pre->output_num++;
        /*Modify current libgate*/
        lgkntwk->libgate_ptrs[im]->inputs[ip] = pre;
      }
    }
  }

  /*Traversal all latches*/
  for (im=0; im<lgkntwk->latch_num; im++) {
    /*Check if its inputs are connected to PO*/
    for (ip=0; ip<lgkntwk->latch_ptrs[im]->input_num; ip++) {
      if (ND_PO == lgkntwk->latch_ptrs[im]->inputs[ip]->type) {
        /*We find one of the problem, fix it!*/
        po = lgkntwk->latch_ptrs[im]->inputs[ip];
        /*Find the gate we should realloc its output*/
        pre = po->inputs[0];
        //pre->outputs = (t_lgknd**)my_realloc(sizeof(t_lgknd*)*(1 + pre->output_num));
        //pre->outputs[pre->output_num] = lgkntwk->libgate_ptrs[im];
        //pre->output_num++;
        /*Modify current libgate*/
        lgkntwk->latch_ptrs[im]->inputs[ip] = pre;
      }
    }
  }

  return 1;
}

int assign_libgate_conkts(t_blif_arch* blif_arch,
                          t_lgknd** libgate_ptrs,
                          t_lgknd** latch_ptrs,
                          t_lgknd** pi_ptrs,
                          t_lgknd** po_ptrs)
{
  int igate;
  int im;
  int tmp_idx;

  t_lgknd** ptr_lst[3] = {NULL, libgate_ptrs, latch_ptrs};
  t_lgknd** base_addr;
  
  /* Assign each libgate*/
  for (igate=0; igate<blif_arch->libgate_num; igate++)
  {
    /*Assign input number and output number*/
    libgate_ptrs[igate]->input_num = blif_arch->libgates[igate].input_num; 
    libgate_ptrs[igate]->output_num = blif_arch->libgates[igate].output.conkt->output_num;

    /**
     * Special Case: if latch is connected to a PO,
     * PO may has zero outputs number.
     */
    if (blif_arch->libgates[igate].output.conkt->is_po)
    {libgate_ptrs[igate]->output_num++;}

    /*Allocate memory*/
    libgate_ptrs[igate]->inputs = (t_lgknd**)my_malloc(libgate_ptrs[igate]->input_num*sizeof(t_lgknd*));
    libgate_ptrs[igate]->outputs = (t_lgknd**)my_malloc(libgate_ptrs[igate]->output_num*sizeof(t_lgknd*));
    /* Assign each inputs*/
    for (im=0; im<libgate_ptrs[igate]->input_num; im++)
    {
      /*Check if this is a PI*/
      if (blif_arch->libgates[igate].inputs[im].conkt->is_pi)
      {
        tmp_idx = blif_arch->libgates[igate].inputs[im].conkt->pi_idx;
        /*This is a PI! Map connections to PI*/
        libgate_ptrs[igate]->inputs[im] = pi_ptrs[tmp_idx];
      }
      /*Check if this is a PO*/
      else if (blif_arch->libgates[igate].inputs[im].conkt->is_po)
      {
        tmp_idx = blif_arch->libgates[igate].inputs[im].conkt->po_idx;
        /*This is a PI! Map connections to PI*/
        libgate_ptrs[igate]->inputs[im] = po_ptrs[tmp_idx];
      }
      /* The input is connected to a gate or latch*/
      else
      {
        /*Find the conkt and spot its input!*/
        base_addr = ptr_lst[blif_arch->libgates[igate].inputs[im].conkt->inputs_types];
        libgate_ptrs[igate]->inputs[im] = *(base_addr + blif_arch->libgates[igate].inputs[im].conkt->inputs);
      }
    }
    /*Assign outputs*/
    /*If output is a PO, PO may has zero output*/    
    if (blif_arch->libgates[igate].output.conkt->is_po)
    {
      tmp_idx = blif_arch->libgates[igate].output.conkt->po_idx;
      libgate_ptrs[igate]->outputs[libgate_ptrs[igate]->output_num-1] = po_ptrs[tmp_idx];
    }
    /*Assign Rest*/ 
    for(im=0; im<blif_arch->libgates[igate].output.conkt->output_num; im++)
    {
      base_addr = ptr_lst[blif_arch->libgates[igate].output.conkt->outputs_types[im]];
      libgate_ptrs[igate]->outputs[im] = *(base_addr + blif_arch->libgates[igate].output.conkt->outputs[im]);
    }
  }

  return 1; 
}

int assign_lut_latch_conkts(t_blif_arch* blif_arch,
                            t_lgknd** libgate_ptrs,
                            t_lgknd** latch_ptrs,
                            t_lgknd** pi_ptrs,
                            t_lgknd** po_ptrs)
{
  int ilatch;
  int im;
  int tmp_idx;

  t_lgknd** ptr_lst[4] = {NULL, NULL, latch_ptrs, libgate_ptrs};
  t_lgknd** base_addr;
  
  /* Assign each latch*/
  for (ilatch=0; ilatch<blif_arch->latch_num; ilatch++)
  {
    /*Assign input number and output number*/
    latch_ptrs[ilatch]->input_num = 1; 
    latch_ptrs[ilatch]->output_num = blif_arch->latches[ilatch].output.conkt->output_num;

    /**
     * Special Case: if latch is connected to a PO,
     * PO may has zero outputs number.
     */
    if (blif_arch->latches[ilatch].output.conkt->is_po)
    {latch_ptrs[ilatch]->output_num++;}

    /*Allocate memory*/
    latch_ptrs[ilatch]->inputs = (t_lgknd**)my_malloc(latch_ptrs[ilatch]->input_num*sizeof(t_lgknd*));
    latch_ptrs[ilatch]->outputs = (t_lgknd**)my_malloc(latch_ptrs[ilatch]->output_num*sizeof(t_lgknd*));
    /* Assign input*/
    /*Check if this is a PI*/
    if (blif_arch->latches[ilatch].input.conkt->is_pi)
    {
      tmp_idx = blif_arch->latches[ilatch].input.conkt->pi_idx;
      /*This is a PI! Map connections to PI*/
      latch_ptrs[ilatch]->inputs[0] = pi_ptrs[tmp_idx];
    }
    /*Check if this is a PO*/
    else if (blif_arch->latches[ilatch].input.conkt->is_po)
    {
      tmp_idx = blif_arch->latches[ilatch].input.conkt->po_idx;
      /*This is a PO! Map connections to PO*/
      latch_ptrs[ilatch]->inputs[0] = po_ptrs[tmp_idx];
    }
    /* The input is connected to a gate or latch*/
    else
    {
      /*Find the conkt and spot its input!*/
      base_addr = ptr_lst[blif_arch->latches[ilatch].input.conkt->inputs_types];
      latch_ptrs[ilatch]->inputs[0] = *(base_addr + blif_arch->latches[ilatch].input.conkt->inputs);
    }
    /*Assign outputs*/
    /*If output is a PO, PO may has zero output*/    
    if (blif_arch->latches[ilatch].output.conkt->is_po)
    {
      tmp_idx = blif_arch->latches[ilatch].output.conkt->po_idx;
      latch_ptrs[ilatch]->outputs[latch_ptrs[ilatch]->output_num-1] = po_ptrs[tmp_idx];
    }
    /*Assign Rest*/ 
    for(im=0; im<blif_arch->latches[ilatch].output.conkt->output_num; im++)
      {
        base_addr = ptr_lst[blif_arch->latches[ilatch].output.conkt->outputs_types[im]];
        latch_ptrs[ilatch]->outputs[im] = *(base_addr + blif_arch->latches[ilatch].output.conkt->outputs[im]);
      
      }
  }

  return 1; 
}


int assign_latch_conkts(t_blif_arch* blif_arch,
                        t_lgknd** libgate_ptrs,
                        t_lgknd** latch_ptrs,
                        t_lgknd** pi_ptrs,
                        t_lgknd** po_ptrs)
{
  int ilatch;
  int im;
  int tmp_idx;

  t_lgknd** ptr_lst[3] = {NULL, libgate_ptrs, latch_ptrs};
  t_lgknd** base_addr;
  
  /* Assign each latch*/
  for (ilatch=0; ilatch<blif_arch->latch_num; ilatch++)
  {
    /*Assign input number and output number*/
    latch_ptrs[ilatch]->input_num = 1; 
    latch_ptrs[ilatch]->output_num = blif_arch->latches[ilatch].output.conkt->output_num;

    /**
     * Special Case: if latch is connected to a PO,
     * PO may has zero outputs number.
     */
    if (blif_arch->latches[ilatch].output.conkt->is_po)
    {latch_ptrs[ilatch]->output_num++;}

    /*Allocate memory*/
    latch_ptrs[ilatch]->inputs = (t_lgknd**)my_malloc(latch_ptrs[ilatch]->input_num*sizeof(t_lgknd*));
    latch_ptrs[ilatch]->outputs = (t_lgknd**)my_malloc(latch_ptrs[ilatch]->output_num*sizeof(t_lgknd*));
    /* Assign input*/
    /*Check if this is a PI*/
    if (blif_arch->latches[ilatch].input.conkt->is_pi)
    {
      tmp_idx = blif_arch->latches[ilatch].input.conkt->pi_idx;
      /*This is a PI! Map connections to PI*/
      latch_ptrs[ilatch]->inputs[0] = pi_ptrs[tmp_idx];
    }
    /*Check if this is a PO*/
    else if (blif_arch->latches[ilatch].input.conkt->is_po)
    {
      tmp_idx = blif_arch->latches[ilatch].input.conkt->po_idx;
      /*This is a PO! Map connections to PO*/
      latch_ptrs[ilatch]->inputs[0] = po_ptrs[tmp_idx];
    }
    /* The input is connected to a gate or latch*/
    else
    {
      /*Find the conkt and spot its input!*/
      base_addr = ptr_lst[blif_arch->latches[ilatch].input.conkt->inputs_types];
      latch_ptrs[ilatch]->inputs[0] = *(base_addr + blif_arch->latches[ilatch].input.conkt->inputs);
    }
    /*Assign outputs*/
    /*If output is a PO, PO may has zero output*/    
    if (blif_arch->latches[ilatch].output.conkt->is_po)
    {
      tmp_idx = blif_arch->latches[ilatch].output.conkt->po_idx;
      latch_ptrs[ilatch]->outputs[latch_ptrs[ilatch]->output_num-1] = po_ptrs[tmp_idx];
    }
    /*Assign Rest*/ 
    for(im=0; im<blif_arch->latches[ilatch].output.conkt->output_num; im++)
      {
        base_addr = ptr_lst[blif_arch->latches[ilatch].output.conkt->outputs_types[im]];
        latch_ptrs[ilatch]->outputs[im] = *(base_addr + blif_arch->latches[ilatch].output.conkt->outputs[im]);
      
      }
  }

  return 1; 
}


int assign_pi_conkts(t_blif_arch* blif_arch,
                     t_conkt* head,
                     t_lgknd** pi_ptrs,
                     t_lgknd*** ptr_lst)
{
  int i;
  int ind;

  char* lgk_name[3] = {"NULL","libgate","latch"};

  /*Check if the conkt is a pi*/ 
  /*Find the PI information in conkt, 
   *Make sure that the index of PI in blif_arch equals
   *to the one of PI in pt_ptrs
   */
  for (i=0; i<blif_arch->input_num; i++)
  {
    if (0 == strcmp(blif_arch->inputs[i].name,head->name))
    {
      /*The net is a PI, configure its connections*/
      pi_ptrs[i]->input_num = head->input_num;
      pi_ptrs[i]->output_num = head->output_num;
      if (pi_ptrs[i]->input_num != 0)
      {
        printf("Error: PI(%s) has %d inputs! It should be zero!\n", head->name, pi_ptrs[i]->input_num);
        exit(1);
      }
      /*Allocate memory for inputs and outputs*/
      //pi_ptrs[i]->inputs = (t_lgknd**)my_malloc(pi_ptrs[i]->input_num*sizeof(t_lgknd*));
      pi_ptrs[i]->outputs = (t_lgknd**)my_malloc(pi_ptrs[i]->output_num*sizeof(t_lgknd*));
      /*Assign pointers of connection*/
      /*Inputs*/
      /*None Inputs...*/
      /*Outputs*/
      for(ind=0; ind<pi_ptrs[i]->output_num; ind++)
      { 
        pi_ptrs[i]->outputs[ind] = *(ptr_lst[head->outputs_types[ind]] + head->outputs[ind]);
        //DEBUG
        if (DEBUG)
        {printf("DEBUG: assign PI(name: %s,index: %d) outputs[%d] to type(%s) offset(%d)\n", head->name, i, ind, lgk_name[head->outputs_types[ind]], head->outputs[ind]);}
         
      }
      /*Return 1: match a PI and complete configurations*/
      return 1; 
    }
  }
  /*Return 0: fail to match a PI*/ 
  return 0;
}
    
int assign_po_conkts(t_blif_arch* blif_arch,
                     t_conkt* head,
                     t_lgknd** po_ptrs,
                     t_lgknd*** ptr_lst)
{
  int i;
  int ind;

  char* lgk_name[3] = {"NULL","libgate","latch"};

  /*Check if the conkt is a po*/ 
  /*Find the PO information in conkt, 
   *Make sure that the index of PO in blif_arch equals
   *to the one of PO in pt_ptrs
   */
  for (i=0; i<blif_arch->output_num; i++)
  {
    if (0 == strcmp(blif_arch->outputs[i].name,head->name))
    {
      /*The net is a PO, configure its connections*/
      po_ptrs[i]->input_num = head->input_num;
      po_ptrs[i]->output_num = head->output_num;
      if (po_ptrs[i]->input_num != 1)
      {
        printf("Error: PO(%s) has %d inputs! It should be one!\n", head->name, po_ptrs[i]->input_num);
        exit(1);
      }
      /*Allocate memory for inputs and outputs*/
      po_ptrs[i]->inputs = (t_lgknd**)my_malloc(po_ptrs[i]->input_num*sizeof(t_lgknd*));
      po_ptrs[i]->outputs = (t_lgknd**)my_malloc(po_ptrs[i]->output_num*sizeof(t_lgknd*));
      /*Assign pointers of connection*/
      /*Inputs*/
      po_ptrs[i]->inputs[0] = *(ptr_lst[head->inputs_types] + head->inputs);
      //DEBUG
      if (DEBUG)
      {printf("DEBUG: assign PO(name: %s,index: %d) inputs[%d] to type(%s) offset(%d)\n", head->name, i, 0, lgk_name[head->inputs_types], head->inputs);}
      /*Outputs*/
      for(ind=0; ind<po_ptrs[i]->output_num; ind++)
      { 
        po_ptrs[i]->outputs[ind] = *(ptr_lst[head->outputs_types[ind]] + head->outputs[ind]);
        //DEBUG
        if (DEBUG)
        {printf("DEBUG: assign PO(name: %s,index: %d) outputs[%d] to type(%s) offset(%d)\n", head->name, i, ind, lgk_name[head->outputs_types[ind]], head->outputs[ind]);}
         
      }
      /*Return 1: match a PO and complete configurations*/
      return 1; 
    }
  }
  /*Return 0: fail to match a PO*/ 
  return 0;
}

/*Assign LUTs*/ 
int assign_lut(t_lgkntwk* lgkntwk,
               t_blif_arch* blif_arch,
               t_stats* stats)
{
  /*Temp head*/
  t_lgknd* tmp;
  int lut_cnt;

  t_lgknd* libgate_head = lgkntwk->libgate_head;
  t_lgknd** libgate_ptrs = lgkntwk->libgate_ptrs;

  /*Allocate memory for linked list*/
  libgate_head = (t_lgknd*)create_lgknd_list(blif_arch->gate_num,sizeof(t_lgknd));
  /*Allocate memory for pointer array*/
  libgate_ptrs = (t_lgknd**)my_malloc(blif_arch->gate_num*sizeof(t_lgknd*));

  /*Initial and give value*/ 
  tmp = libgate_head;
  init_lgknd(tmp);
  tmp = tmp->next;
  lut_cnt = 0;
  stats->libgate_num = 0;

  while(NULL != tmp)
  {
    /*Initial*/
    init_lgknd(tmp);
    /*Fill array pointers*/
    libgate_ptrs[lut_cnt] = tmp;
    /*Assign details*/
    tmp->idx = lut_cnt;
    /* Assign logic node info*/
    /* Only assign type*/
    tmp->type = ND_LUT;
    /*incremental counter*/
    lut_cnt++;
    tmp = tmp->next;
  }

  stats->libgate_num = lut_cnt;

  lgkntwk->libgate_num = lut_cnt;
  lgkntwk->libgate_head = libgate_head;
  lgkntwk->libgate_ptrs = libgate_ptrs;

  return 1;
}

  
/*Assign library gates*/ 
int assign_libgate(t_lgkntwk* lgkntwk,
                   t_blif_arch* blif_arch,
                   t_stats* stats)
{
  /*Temp head*/
  t_lgknd* tmp;
  int itype;
  int libgate_cnt;

  t_lgknd* libgate_head = lgkntwk->libgate_head;
  t_lgknd** libgate_ptrs = lgkntwk->libgate_ptrs;

  /*Allocate memory for linked list*/
  libgate_head = (t_lgknd*)create_lgknd_list(blif_arch->libgate_num,sizeof(t_lgknd));
  /*Allocate memory for pointer array*/
  libgate_ptrs = (t_lgknd**)my_malloc(blif_arch->libgate_num*sizeof(t_lgknd*));

  /*Initial and give value*/ 
  tmp = libgate_head;
  init_lgknd(tmp);
  tmp = tmp->next;
  libgate_cnt = 0;
  stats->libgate_num = 0;

  while(NULL != tmp)
  {
    /*Initial*/
    init_lgknd(tmp);
    /*Fill array pointers*/
    libgate_ptrs[libgate_cnt] = tmp;
    /*Assign details*/
    tmp->idx = libgate_cnt;
    itype = 0;
    /* Match types, INV/AND/XNOR...*/
    while(typepair_list[itype].type_name)
    {
      /* Find matched type of logic node*/
      if (0 == strcmp(typepair_list[itype].type_name, blif_arch->libgates[libgate_cnt].name)) {
        /* Assign logic node info*/
        /* Only assign type*/
        tmp->type = typepair_list[itype].type_id;
       // if (0 == strcmp("ZERO",blif_arch->libgates[libgate_cnt].name)) {
        if (typepair_list[itype].type_id == ND_ZERO) {
          printf("DEBUG :Find Zero! Name: %s. \n",blif_arch->libgates[libgate_cnt].name);
        }
        //if ((DEBUG)&&(0 == ))
        //{printf("DEBUG: Assign libgate(name: %s index:%d");} 

        /*Stats*/
        stats->stdgate_stats[typepair_list[itype].type_id]++;

        /*Quit inner loop*/
        break;
      }
      itype++;
    } 

    /*Mismatch check*/
    if (NULL == typepair_list[itype].type_name)
    {
      printf("Error: Undefined type for logic cell!\n");
      printf("Error: it should be the following type: <AND|XNOR|ADD|CARRY|INV|BUF|LATCH>\n");
      exit(1);
    }

    /*incremental counter*/
    libgate_cnt++;
    tmp = tmp->next;
  }

  stats->libgate_num = libgate_cnt;

  lgkntwk->libgate_num = libgate_cnt;
  lgkntwk->libgate_head = libgate_head;
  lgkntwk->libgate_ptrs = libgate_ptrs;

  return 1;
}


/*Assign primary inputs*/
int assign_pi(t_lgkntwk* lgkntwk,
              t_blif_arch* blif_arch,
              t_stats* stats)
{
  /*Temp head*/
  t_lgknd* tmp;
  int pi_cnt;

  t_lgknd* pi_head = lgkntwk->pi_head;
  t_lgknd** pi_ptrs = lgkntwk->pi_ptrs;

  /*Allocate memory*/
  pi_head = (t_lgknd*)create_lgknd_list(blif_arch->input_num,sizeof(t_lgknd));
  /*Allocate memory for pointer array*/
  pi_ptrs = (t_lgknd**)my_malloc(blif_arch->input_num*sizeof(t_lgknd*));
  
  /*Assign PIs*/
  tmp = pi_head;
  init_lgknd(tmp);
  pi_cnt = 0;  
  stats->pi_num = 0;

  while(NULL != tmp->next)
  {
    tmp = tmp->next;
    /*Initial*/
    init_lgknd(tmp);
    /*Fill array pointers*/
    pi_ptrs[pi_cnt] = tmp;
    tmp->idx = pi_cnt; 
    tmp->type = ND_PI;
    /*Increamental counter*/
    pi_cnt++;
    /*Stats*/
    stats->pi_num++;
  }
  
  lgkntwk->pi_num = pi_cnt;
  lgkntwk->pi_head = pi_head;
  lgkntwk->pi_ptrs = pi_ptrs;

  return 1;
}


/*Assign primary outputs*/
int assign_po(t_lgkntwk* lgkntwk,
              t_blif_arch* blif_arch,
              t_stats* stats)
{
  /*Temp head*/
  t_lgknd* tmp;
  int po_cnt;

  t_lgknd* po_head = lgkntwk->po_head;
  t_lgknd** po_ptrs = lgkntwk->po_ptrs;

  /*Allocate memory*/
  po_head = (t_lgknd*)create_lgknd_list(blif_arch->output_num,sizeof(t_lgknd));
  /*Allocate memory for pointer array*/
  po_ptrs = (t_lgknd**)my_malloc(blif_arch->output_num*sizeof(t_lgknd*));
  
  /*Assign POs*/
  tmp = po_head;
  init_lgknd(tmp);
  po_cnt = 0;
  stats->po_num = 0;

  while(NULL != tmp->next)
  {
    tmp = tmp->next;
    /*Initial*/
    init_lgknd(tmp);
    /*Fill array pointers*/
    po_ptrs[po_cnt] = tmp;
    tmp->idx = po_cnt;
    tmp->type = ND_PO;
    /*Increamental counter*/
    po_cnt++;
    /*Stats*/
    stats->po_num++;
  }

  lgkntwk->po_num = po_cnt;
  lgkntwk->po_head = po_head;
  lgkntwk->po_ptrs = po_ptrs;

  return 1;
}


/*Assign latches*/
int assign_latch(t_lgkntwk* lgkntwk,
                 t_blif_arch* blif_arch,
                 t_stats* stats)
{
  /*Temp head*/
  t_lgknd* tmp;
  int latch_cnt;

  t_lgknd* latch_head = lgkntwk->latch_head;
  t_lgknd** latch_ptrs = lgkntwk->latch_ptrs;

  /*Allocate memory*/
  latch_head = (t_lgknd*)create_lgknd_list(blif_arch->latch_num,sizeof(t_lgknd));
  /*Allocate memory for pointer array*/
  latch_ptrs = (t_lgknd**)my_malloc(blif_arch->latch_num*sizeof(t_lgknd*));
  
  /*Assign latches*/
  tmp = latch_head;
  init_lgknd(tmp);
  latch_cnt = 0;
  stats->latch_num = 0;

  while(NULL != tmp->next)
  {
    tmp = tmp->next;
    /*Initial*/
    init_lgknd(tmp);
    latch_ptrs[latch_cnt] = tmp;
    tmp->idx = latch_cnt;
    tmp->type = ND_LATCH;
    tmp->init_val = blif_arch->latches[latch_cnt].init_val;
    /*Increamental counter*/
    latch_cnt++;
    /*Stats*/
    stats->latch_num++;
  }

  lgkntwk->latch_num = latch_cnt;
  lgkntwk->latch_head = latch_head;
  lgkntwk->latch_ptrs = latch_ptrs;

  return 1;
}

int assign_lut_lgknds_net_name(t_blif_arch* blif_arch,
                               t_lgkntwk* lgkntwk) {
  int i;

  /*Primary Inputs*/
  for (i=0; i < blif_arch->input_num; i++) {
    lgkntwk->pi_ptrs[i]->net_name = my_strdup(blif_arch->inputs[i].name);
  }
  /*Primary Outputs*/
  for (i=0; i < blif_arch->output_num; i++) {
    lgkntwk->po_ptrs[i]->net_name = my_strdup(blif_arch->outputs[i].name);
  }
  /*Copy net name of libgates*/
  for (i = 0; i < blif_arch->gate_num; i++) {
    lgkntwk->libgate_ptrs[i]->net_name = my_strdup(blif_arch->gates[i].output.conkt->name);
  }

  /*Copy net name of latches*/
  for (i = 0; i < blif_arch->latch_num; i++) {
    lgkntwk->latch_ptrs[i]->net_name = my_strdup(blif_arch->latches[i].output.conkt->name);
  }
   
  return 1;
}


/* Assign net_name for each logic node(libgates and latches)
 * This makes printing netlist easier as well as checking
 * if netlist is correct.
 */
int assign_lgknds_net_name(t_blif_arch* blif_arch,
                           t_lgkntwk* lgkntwk) {
  int i;

  /*Primary Inputs*/
  for (i=0; i < blif_arch->input_num; i++) {
    lgkntwk->pi_ptrs[i]->net_name = my_strdup(blif_arch->inputs[i].name);
  }
  /*Primary Outputs*/
  for (i=0; i < blif_arch->output_num; i++) {
    lgkntwk->po_ptrs[i]->net_name = my_strdup(blif_arch->outputs[i].name);
  }
  /*Copy net name of libgates*/
  for (i = 0; i < blif_arch->libgate_num; i++) {
    lgkntwk->libgate_ptrs[i]->net_name = my_strdup(blif_arch->libgates[i].output.conkt->name);
  }

  /*Copy net name of latches*/
  for (i = 0; i < blif_arch->latch_num; i++) {
    lgkntwk->latch_ptrs[i]->net_name = my_strdup(blif_arch->latches[i].output.conkt->name);
  }
   
  return 1;
}
