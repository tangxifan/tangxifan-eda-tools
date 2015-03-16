#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "mpack_conf.h"
#include "Options.h"

extern t_optinfo opt_list[];


int check_mpackopts(t_mpack_conf* mpackopts_ptr);

/*****Subroutines*****/
int config_mpack(t_mpack_conf* mpackopts_ptr,
                 t_optinfo* optlst_ptr
                )
{
  int istr;
  int fblif_len;
  int fstats_len;
  int fble_arch_len;
  int fvpr_arch_len;
  int fmpack_blif_len;
  int fnet_len;
  char* tmp;

  /*Copy Blif name*/
  //printf("Copy Blif to mpack configuration...\n");
  if (ARG_DEF == optlst_ptr[OP_BLIF].opt_def) {
    mpackopts_ptr->blif = my_strdup((optlst_ptr+OP_BLIF)->val);
    printf("Blif filename: %s.\n",mpackopts_ptr->blif);
    fblif_len = strlen(mpackopts_ptr->blif)+1;
    if (0 != strcmp(mpackopts_ptr->blif + fblif_len - 5,"blif")) {
      printf("Error: Blif file(%s) should be \"*.blif\"!\n",optlst_ptr[OP_BLIF].val);
      return 0;
    }
  } else {
    printf("Error: Invaild Blif Name(%s)!\n",optlst_ptr[OP_BLIF].val);
    return 0;
  }

  /*Copy MPACK blif name*/
  if (ARG_DEF == optlst_ptr[OP_MPACKBLIF].opt_def) {
    mpackopts_ptr->mpack_blif = my_strdup((optlst_ptr+OP_MPACKBLIF)->val);
  } else {
    //printf("Error: Invaild Net Name(%s)!\n",optlst_ptr[OP_MPACKBLIF].val);
    /* Create default mpack_blif
     *   chop the .blif and replace with _vpr.blif
     */
    tmp = my_strdup(mpackopts_ptr->blif);
    fblif_len = strlen(mpackopts_ptr->blif)+1;
    fmpack_blif_len = fblif_len - 1;
    for (istr=0; istr<fblif_len; istr++) {
      if ('.' == mpackopts_ptr->blif[fblif_len-istr]) {
        fmpack_blif_len = fblif_len - istr;
      }
    }
    tmp[fmpack_blif_len] = '\0';
    /* .stats + \0 = 7 in length */
    fmpack_blif_len += 10; 
    mpackopts_ptr->mpack_blif = (char*)my_malloc(sizeof(char)*fmpack_blif_len); 
    strcpy(mpackopts_ptr->mpack_blif,tmp);
    strcat(mpackopts_ptr->mpack_blif,"_vpr.blif");
  }
  printf("Mpack blif filename: %s.\n",mpackopts_ptr->mpack_blif);

  /*Copy Net name*/
  if (ARG_DEF == optlst_ptr[OP_NET].opt_def) {
    mpackopts_ptr->net = my_strdup((optlst_ptr+OP_NET)->val);
  } else {
    /* Create default mpack_blif
     *   chop the .blif and replace with _vpr.blif
     */
    tmp = my_strdup(mpackopts_ptr->blif);
    fblif_len = strlen(mpackopts_ptr->blif)+1;
    fnet_len = fblif_len - 1;
    for (istr=0; istr<fblif_len; istr++) {
      if ('.' == mpackopts_ptr->blif[fblif_len-istr]) {
        fnet_len = fblif_len - istr;
      }
    }
    tmp[fnet_len] = '\0';
    /* .stats + \0 = 7 in length */
    fnet_len += 9; 
    mpackopts_ptr->net = (char*)my_malloc(sizeof(char)*fnet_len); 
    strcpy(mpackopts_ptr->net,tmp);
    strcat(mpackopts_ptr->net,"_vpr.net");
  }
  printf("Net filename: %s.\n",mpackopts_ptr->net);

  /*Copy BLE arch file*/
  if (ARG_DEF == optlst_ptr[OP_BLEARCH].opt_def) {
    mpackopts_ptr->ble_arch = my_strdup(optlst_ptr[OP_BLEARCH].val);
    printf("BLE arch filename: %s.\n",mpackopts_ptr->ble_arch);
  } else {
    mpackopts_ptr->ble_arch = "ble_arch_default.arch";
  }

  /*Copy VPR architecture XML file*/
  if (ARG_DEF == optlst_ptr[OP_VPRARCH].opt_def)
  {
    mpackopts_ptr->vpr_arch = my_strdup((optlst_ptr+OP_VPRARCH)->val);
  }
  else
  {
    /* Create default vpr architecture filename
     *   chop the .arch and replace with _vpr.xml
     */
    tmp = my_strdup(mpackopts_ptr->ble_arch);
    fble_arch_len = strlen(mpackopts_ptr->ble_arch)+1;
    fvpr_arch_len = fble_arch_len - 1;
    for (istr=0; istr<fble_arch_len; istr++)
    {
      if ('.' == mpackopts_ptr->ble_arch[fble_arch_len-istr])
      {fvpr_arch_len = fble_arch_len - istr;}
    }
    tmp[fvpr_arch_len] = '\0';
    /* _vpr.xml + \0 = 9 in length */
    fvpr_arch_len += 9; 
    mpackopts_ptr->vpr_arch = (char*)my_malloc(sizeof(char)*fvpr_arch_len); 
    strcpy(mpackopts_ptr->vpr_arch,tmp);
    strcat(mpackopts_ptr->vpr_arch,"_vpr.xml");
  }
  printf("VPR Arch File: %s.\n",mpackopts_ptr->vpr_arch);
  
  /*Copy Statistics Statements file*/
  if (ARG_DEF == optlst_ptr[OP_STATS].opt_def)
  {
    mpackopts_ptr->stats = my_strdup((optlst_ptr+OP_STATS)->val);
  }
  else
  {
    /* Create default stats filename
     *   chop the .blif and replace with .stats
     */
    tmp = my_strdup(mpackopts_ptr->blif);
    fblif_len = strlen(mpackopts_ptr->blif)+1;
    fstats_len = fblif_len - 1;
    for (istr=0; istr<fblif_len; istr++)
    {
      if ('.' == mpackopts_ptr->blif[fblif_len-istr])
      {fstats_len = fblif_len - istr;}
    }
    tmp[fstats_len] = '\0';
    /* .stats + \0 = 7 in length */
    fstats_len += 7; 
    mpackopts_ptr->stats = (char*)my_malloc(sizeof(char)*fstats_len); 
    strcpy(mpackopts_ptr->stats,tmp);
    strcat(mpackopts_ptr->stats,".stats");
  }
  printf("Stats File: %s.\n",mpackopts_ptr->stats);

  /*Copy timing_analysis*/
  mpackopts_ptr->timing_analysis = 1;
  if (ARG_DEF == optlst_ptr[OP_TIMING_ANALYSIS].opt_def) {
    if (0 == strcmp((optlst_ptr+OP_TIMING_ANALYSIS)->val,"off")) {
      mpackopts_ptr->timing_analysis = 0;
    }
  }
  if (1 == mpackopts_ptr->timing_analysis) {
    printf("TIMING_ANALYSIS: ON\n");
  } else {
    printf("TIMING_ANALYSIS: OFF\n");
  }

  /*Copy alpha*/
  mpackopts_ptr->alpha = 0.75;
  if (ARG_DEF == optlst_ptr[OP_ALPHA].opt_def) {
    mpackopts_ptr->alpha = my_atof(optlst_ptr[OP_ALPHA].val);
  }
  printf("ALPHA: %.3g.\n",mpackopts_ptr->alpha);
  if ((0.0 > mpackopts_ptr->alpha)||(1 < mpackopts_ptr->alpha)) {
    printf("Error: ALPHA range is [0,1]. \n"); 
    exit(1);
  }

  /*Copy beta*/
  mpackopts_ptr->beta = 0.9;
  if (ARG_DEF == optlst_ptr[OP_BETA].opt_def) {
    mpackopts_ptr->beta = my_atof(optlst_ptr[OP_BETA].val);
  }
  printf("BETA: %.3g.\n",mpackopts_ptr->beta);
  if ((0.0 > mpackopts_ptr->beta)||(1 < mpackopts_ptr->beta)) {
    printf("Error: BETA range is [0,1].\n"); 
    exit(1);
  }

  /*Copy read_lut_blif*/
  mpackopts_ptr->read_stdlib_blif = 0;
  if (ARG_DEF == optlst_ptr[OP_READ_STDLIB_BLIF].opt_def) {
    if (0 == strcmp((optlst_ptr+OP_READ_STDLIB_BLIF)->val,"on")) {
      mpackopts_ptr->read_stdlib_blif = 1;
    }
  }
  if (1 == mpackopts_ptr->read_stdlib_blif) {
    printf("READ_STDLIB_BLIF: ON\n");
  } else {
    printf("READ_STDLIB_BLIF: OFF\n");
  }
  
  /*Deal with pattern member*/
  mpackopts_ptr->pattern_member_type = PATTERN_MEMBER_NORMAL;
  if (ARG_DEF == optlst_ptr[OP_PATTERN_MEMBER_TYPE].opt_def) {
    if (0 == strcmp((optlst_ptr+OP_PATTERN_MEMBER_TYPE)->val,"comb_only")) {
      mpackopts_ptr->pattern_member_type = PATTERN_MEMBER_COMBONLY;
    }
  } else {
    optlst_ptr[OP_PATTERN_MEMBER_TYPE].val = my_strdup("normal");
  }
  printf("PATTERN_MEMBER_TYPE : %s\n",(optlst_ptr+OP_PATTERN_MEMBER_TYPE)->val);

  /*Copy packer*/
  mpackopts_ptr->packer_algorithm = PACKER_ALGORITHM_PATTERN;
  if (ARG_DEF == optlst_ptr[OP_PACKER_ALGORITHM].opt_def) {
    if (0 == strcmp((optlst_ptr+OP_PACKER_ALGORITHM)->val,"seed")) {
      mpackopts_ptr->packer_algorithm = PACKER_ALGORITHM_SEED;
    } else if (0 == strcmp((optlst_ptr+OP_PACKER_ALGORITHM)->val,"sat")) {
      mpackopts_ptr->packer_algorithm = PACKER_ALGORITHM_SAT;
    }
  } else {
    optlst_ptr[OP_PACKER_ALGORITHM].val = my_strdup("pattern");
  }
  printf("Packer_Algorithm : %s\n",(optlst_ptr+OP_PACKER_ALGORITHM)->val);

  /*Copy packer area attraction type*/
  mpackopts_ptr->packer_area_attraction= PACKER_AREA_ATTRACTION_VPACK;
  if (ARG_DEF == optlst_ptr[OP_PACKER_AREA_ATTRACTION].opt_def) {
    if (0 == strcmp((optlst_ptr+OP_PACKER_AREA_ATTRACTION)->val,"AAPACK")) {
      mpackopts_ptr->packer_area_attraction = PACKER_AREA_ATTRACTION_AAPACK;
    }
  } else {
    optlst_ptr[OP_PACKER_AREA_ATTRACTION].val = my_strdup("VPACK");
  }
  printf("Packer_Area_Attraction : %s\n",(optlst_ptr+OP_PACKER_AREA_ATTRACTION)->val);
 
  /*Forbid unrelated clustering*/
  mpackopts_ptr->forbid_unrelated_clustering = 0;
  if (ARG_DEF == optlst_ptr[OP_FORBID_UNRELATED_CLUSTERING].opt_def) {
    if (0 == strcmp((optlst_ptr+OP_FORBID_UNRELATED_CLUSTERING)->val,"on")) {
      mpackopts_ptr->forbid_unrelated_clustering = 1;
    }
  }
  if (1 == mpackopts_ptr->forbid_unrelated_clustering) {
    printf("Forbid_unrelated_clustering: on\n");
  } else {
    printf("Forbid_unrelated_clustering: off\n");
  }

  /*Copy verbose*/
  mpackopts_ptr->verbose = 0;
  if (ARG_DEF == optlst_ptr[OP_VERBOSE].opt_def) {
    if (0 == strcmp((optlst_ptr+OP_VERBOSE)->val,"on")) {
      mpackopts_ptr->verbose = 1;
      printf("Verbose: ON!\n");
    }
  }

  /*Copy inner_cluster_delay*/
  //mpackopts_ptr->inner_cluster_delay = 4.5e-11;
  //if (ARG_DEF == optlst_ptr[OP_INNER_CLUSTER_DELAY].opt_def) {
  //  mpackopts_ptr->inner_cluster_delay = my_atof(optlst_ptr[OP_INNER_CLUSTER_DELAY].val);
  //}
  //printf("Inner-cluster delay: %.3g(sec.)\n",mpackopts_ptr->inner_cluster_delay);
  //if (0.0 > mpackopts_ptr->inner_cluster_delay) {
  //  printf("Error: inner-cluster delay shoule >= 0.0! \n"); 
  //  exit(1);
  //}

  /*Copy inter_cluster_delay*/
  mpackopts_ptr->inter_cluster_delay = 8.67e-10;
  if (ARG_DEF == optlst_ptr[OP_INTER_CLUSTER_DELAY].opt_def) {
    mpackopts_ptr->inter_cluster_delay = my_atof(optlst_ptr[OP_INTER_CLUSTER_DELAY].val);
  }
  printf("Inter-cluster delay: %.3g(sec.)\n",mpackopts_ptr->inter_cluster_delay);
  if (0.0 > mpackopts_ptr->inter_cluster_delay) {
    printf("Error: inter-cluster delay shoule >= 0.0! \n"); 
    exit(1);
  }

  /*Copy help*/
  mpackopts_ptr->help = 0;
  if (ARG_DEF == optlst_ptr[OP_HELP].opt_def) {
    if (0 == strcmp((optlst_ptr+OP_HELP)->val,"on")) {
      mpackopts_ptr->help = 1;
      printf("Help Desk is called!\n");
    }
  }
  
  
  return check_mpackopts(mpackopts_ptr);
}

/*Check Critical Options*/
int check_mpackopts(t_mpack_conf* mpackopts_ptr)
{
  if (0 == strcmp(mpackopts_ptr->blif,"")) {
    printf("Error: BLIF filename is blank!\n");
    return 0;
  }
  if (0 == strcmp(mpackopts_ptr->mpack_blif,"")) {
    printf("Error: MPACK blif filename is blank!\n");
    return 0;
  }
  if (0 == strcmp(mpackopts_ptr->net,"")) {
    printf("Error: NET filename is blank!\n");
    return 0;
  }
  if (0 == strcmp(mpackopts_ptr->ble_arch,"")) {
    printf("Error: BLE arch filename is blank!\n");
    return 0;
  }
   
  return 1;
}
