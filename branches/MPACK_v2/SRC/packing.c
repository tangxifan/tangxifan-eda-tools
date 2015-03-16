#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "util.h"
#include "mpack_types.h"
#include "mpack_conf.h"
#include "blearch_parser.h"
#include "mpack_util.h"
#include "stats.h"
#include "climb.h"
#include "check_ble_arch.h"
#include "lgkntwk_common.h"
#include "sat_api.h"
#include "packing.h"
//#include "lgkntwk_timing.h"

/*****Subroutines*****/
t_ble_info* pack_BLEs(t_lgkntwk* lgkntwk,
                      t_stats* stats,
                      t_ble_arch* ble_arch,
                      int timing_analysis,
                      t_delay_info* delay_info,
                      t_mpack_conf mpack_opts, // read_only
                      int verbose)
{
  t_ble_info* ret = NULL;

  /**
   * Pre-process Logic network
   * 1. Remove buffers
   * 2. DFS (Mark depth)
   */ 
  /*Remove buffers*/
  remove_buf_lgkntwk(lgkntwk,stats,verbose);
  /*Sweep away 0-output PIs*/
  remove_isolated_pi_lgkntwk(lgkntwk,stats,verbose);
  /*Sweep away 0-input POs or redundant ones*/
  remove_isolated_po_lgkntwk(lgkntwk,stats,verbose);
  /*Sweep away 0-output latch*/
  remove_hanging_latch_lgkntwk(lgkntwk,stats,verbose);
  /*Check logic network after removing*/
  check_lgkntwk(lgkntwk);
  // Statistic logic network
  stats_lgkntwk(lgkntwk,stats);
  /*After processing the network, we can initial all the edges*/
  init_lgkntwk_edges(lgkntwk,verbose);
  /*Add buffers between Primary input and latches*/
  if ((PACKER_ALGORITHM_SAT == mpack_opts.packer_algorithm)||(PACKER_ALGORITHM_PATTERN == mpack_opts.packer_algorithm)) {
    add_buf_latches(lgkntwk,ble_arch,stats,0,verbose);
  } else if (PACKER_ALGORITHM_SEED == mpack_opts.packer_algorithm){
    add_buf_latches(lgkntwk,ble_arch,stats,1,verbose);
  }
  /*Calculate depths of each logic node*/
  dfs_lgkntwk_depth(lgkntwk,timing_analysis,verbose); 
  /*Depth weighting*/
  depth_weighting_lgkntwk(lgkntwk,verbose);

  // Update intercluster delay
  //delay_info->inter_cluster_delay = delay_info->inter_cluster_delay_per_unit; //* sqrt(lgkntwk->libgate_num/ble_arch->blend_num);

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
    merge_weights_lgkntwk(lgkntwk,mpack_opts.alpha,verbose);
    // Echo Critical Delay
    printf("Info: Before packing, estimated critical delay = %.3g\n",lgkntwk->critical_delay);
    printf("Info: Before packing, max_slack = %.3g\n",lgkntwk->max_slack);
  }
  // Weighting logic network
  //merge_weights_lgkntwk(lgkntwk,alpha,verbose);

  /*New Packer*/
  if (PACKER_ALGORITHM_SAT == mpack_opts.packer_algorithm) {
    ret = sat_pack(lgkntwk,ble_arch,timing_analysis,delay_info,mpack_opts,verbose);
  } else if (PACKER_ALGORITHM_SEED == mpack_opts.packer_algorithm) {
  } else if (PACKER_ALGORITHM_PATTERN == mpack_opts.packer_algorithm) {
    ret = pattern_pack(lgkntwk,ble_arch,timing_analysis,delay_info,mpack_opts,verbose);
  }

  if (timing_analysis) {
    // Echo Critical Delay
    lgkntwk->critical_delay = critical_delay_lgkntwk(lgkntwk,verbose);
    timing_required_lgkntwk(lgkntwk,lgkntwk->critical_delay,verbose);
    lgkntwk->max_slack = timing_max_slack_lgkntwk(lgkntwk,verbose);
    printf("Info: After packing, estimated critical delay = %.3g\n",lgkntwk->critical_delay);
    printf("Info: After packing, max_slack = %.3g\n",lgkntwk->max_slack);
  } 
  
  return ret;
}

/**
 * Check all NONE PI/PO nodes has been mapped
 * Return 1 if exists unmapped nodes
 * Return 0 if all mapped
 */
int check_all_mapped(t_lgkntwk* lgkntwk,
                     int verbose)
{
  t_lgknd* head;
  t_lgknd* tmp;
  //int im;

  /*
  for (im=0; im<lgkntwk->libgate_num; im++) {
    if (1 != lgkntwk->libgate_ptrs[im]->mapped) {
      //printf("Libgate(idx:%d) unmapped\n",im);
      return 1;
    }
  }

  for (im=0; im<lgkntwk->latch_num; im++) {
    if (1 != lgkntwk->latch_ptrs[im]->mapped) {
      //printf("Latch(idx:%d) unmapped\n",im);
      return 1;
    }
  }
  */
  /*Check Libgates*/
  head = lgkntwk->libgate_head->next;
  while(head) {
    if (1 != head->mapped) {
      if (1 == verbose) {
        printf("\nLibgate(idx:%d) unmapped\n",head->idx);
      } 
      return 1;
    }
    tmp = head;
    head = head->next;
  }
  
  /*Check Latches*/
  head = lgkntwk->latch_head->next;
  while(head) {
    if (1 != head->mapped) {
      if (1 == verbose) {
        printf("\nLatch(idx:%d) unmapped\n",head->idx);
      } 
      return 1;
    }
    head = head->next;
  } 

  return 0;
}

t_lgknd* pick_seed(t_lgkntwk* lgkntwk)
{
  t_lgknd* ret = NULL;
  int max_depth = -1;
  int i;
   
  for (i=0; i<lgkntwk->libgate_num; i++) {
    if (0 == lgkntwk->libgate_ptrs[i]->mapped) {
      if ((-1 == max_depth)||(lgkntwk->libgate_ptrs[i]->depth > max_depth)) {
        max_depth = lgkntwk->libgate_ptrs[i]->depth;
        ret = lgkntwk->libgate_ptrs[i];
      }
    }
  }

  return ret;
}

/* Pack BLEs using iteratively SAT solver.
 * Select one position in BLE and start SAT solver iterativly.
 * After each successful mapping, check if all logic nodes have 
 * been mapped or there is no empty position in BLE.
 */
t_ble_info* sat_pack(t_lgkntwk* lgkntwk,
                     t_ble_arch* ble_arch,
                     int timing_analysis,
                     t_delay_info* delay_info,
                     t_mpack_conf mpack_opts,
                     int verbose)
{
  int sat_type = 0;
  int cell_idx = -1;
  int* blends_tried = (int*)my_malloc(ble_arch->blend_num*sizeof(int));
  t_lgknd* sat_root = NULL;
  t_ble_info* ret = create_ble_list(0,sizeof(t_ble_info));
  t_ble_info* cur_ble = ret;

  int im;
  int cur_sat_type = 0;
  int ble_num = 0;
  int sat_done = 0;
  
  init_ble_info(ble_arch,ret);

  /*Packing...*/
  while(check_all_mapped(lgkntwk,0)) {
    /*Create a New BLE info, to store packing result*/
    insert_ble_list_node(cur_ble,sizeof(t_ble_info));
    init_ble_info(ble_arch,cur_ble->next);
    cur_ble->next->idx = ble_num;
    /*Initial BLE node tried list*/
    for (im=0; im<ble_arch->blend_num; im++) {
      blends_tried[im] = 0;
    }
    /*Pack One BLE by trying SAT[n] iteratively*/
    while(1) {
      /*Pick a position in ble_arch*/
      cell_idx = pick_cell_idx_from_ble_arch(ble_arch,cur_ble->next,blends_tried);
      if (-1 == cell_idx) {
        break;
      }
      /*Determine the Maximium SAT type the BLE cell can support*/
      sat_type = determine_max_sat_type(ble_arch,cur_ble->next,cell_idx,verbose); 
      if (1 == verbose) {
        printf("Info: Max SAT type that cell(%d) can support is %d\n",cell_idx,sat_type);
      }
      /*Initial sat_done flag*/
      sat_done = 0;
      for (cur_sat_type=sat_type; cur_sat_type > (-1); cur_sat_type--) {
        /*Pick a root lgknd which match current SAT type,
         *Attract should be considered
         */
        /*Check each logic node has been tried SAT*/
        while(1) {
          sat_root = pick_sat_lgknd_from_lgkntwk(lgkntwk,
                                                 cur_sat_type,
                                                 cur_ble->next,
                                                 mpack_opts.alpha,
                                                 mpack_opts.beta,
                                                 mpack_opts.pattern_member_type,
                                                 mpack_opts.packer_area_attraction,
                                                 verbose);
          if (NULL == sat_root) {
            break;
          }
          /*Try to pack, pack a logic node and all its predecessors*/
          if (1 == sat_mapper(lgkntwk,
                              ble_arch,
                              cur_ble->next,
                              cur_sat_type,
                              cell_idx,
                              sat_root,
                              timing_analysis,
                              delay_info,
                              mpack_opts,
                              verbose)) {
            /*Pack Success*/
            if (1 == verbose) {
              printf("Info:(sat_pack)lgknd(Index:%d) SAT%d mapper success on BLE(Index:%d) cell(Index:%d).\n",sat_root->idx,cur_sat_type,cur_ble->next->idx,cell_idx);
            }
            sat_done = 1;
            break;
          }
          else {
            if (1 == verbose) {
              printf("Info:(sat_pack)lgknd(Index:%d) SAT%d mapper fail on BLE(Index:%d) cell(Index:%d).\n",sat_root->idx,cur_sat_type,cur_ble->next->idx,cell_idx);
            } 
          }
        }
        if (1 == sat_done) {
          break;
        }
        /*Clear SAT tried*/
        reset_lgkntwk_try_sat(lgkntwk);
      }  
      /*If the cell idx cannot be mapped*/
      blends_tried[cell_idx] = 1;
      /*Clear SAT tried*/
      reset_lgkntwk_try_sat(lgkntwk);
    }
    /*Clear SAT tried*/
    reset_lgkntwk_try_sat(lgkntwk);
    if ((1 == check_blank_ble_info(ble_arch,cur_ble->next)&&(1 == check_all_mapped(lgkntwk,verbose)))) {
      printf("Info:(sat_pack) No suitable lgknds for BLE architecture!\n");
      exit(1);
    }
    /*Incremental BLE counter*/
    printf("Info: BLE(Index: %d) packed.",cur_ble->next->idx);
    if (1 == verbose) {
      printf("\n");
    }
    else {
      printf("\r");
    } 
    cur_ble = cur_ble->next;
    ble_num++;
  }
 
  printf("Info: Circuit(%d gates,%d latches) is packed into %d BLEs\n",lgkntwk->libgate_num,lgkntwk->latch_num,ble_num);

  /*Don't forget free resources*/
  free(blends_tried);

  return ret;
}

/* Pack BLEs using iteratively SAT solver.
 * Select one position in BLE and start SAT solver iterativly.
 * After each successful mapping, check if all logic nodes have 
 * been mapped or there is no empty position in BLE.
 */
t_ble_info* pattern_pack(t_lgkntwk* lgkntwk,
                         t_ble_arch* ble_arch,
                         int timing_analysis,
                         t_delay_info* delay_info,
                         t_mpack_conf mpack_opts,
                         int verbose)
{
  int cell_idx = -1;
  int* blends_tried = (int*)my_malloc(ble_arch->blend_num*sizeof(int));
  t_ble_info* ret = create_ble_list(0,sizeof(t_ble_info));
  t_ble_info* cur_ble = ret;

  int im;
  int ble_num = 0;
  int arch_pattern_type = 0;
  int pack_pattern_type = 0;
  t_lgknd** pattern_lgknds = NULL;
  
  init_ble_info(ble_arch,ret);

  initial_lgkntwk_try_pattern_types(lgkntwk);
  /*Packing...*/
  while(check_all_mapped(lgkntwk,0)) {
    /*Create a New BLE info, to store packing result*/
    insert_ble_list_node(cur_ble,sizeof(t_ble_info));
    init_ble_info(ble_arch,cur_ble->next);
    cur_ble->next->idx = ble_num;
    /*Initial BLE node tried list*/
    for (im=0; im<ble_arch->blend_num; im++) {
      blends_tried[im] = 0;
    }
    if (1 == verbose) {
      printf("Info: Logic block(index=%d) is created.\n",cur_ble->next->idx);
    }
    /*Pack One BLE by trying pattern iteratively*/
    while(1) {
      /*Pick a position in ble_arch*/
      cell_idx = pick_cell_idx_from_ble_arch(ble_arch,cur_ble->next,blends_tried);
      if (-1 == cell_idx) {
        break;
      }
      /*Determine the Maximium SAT type the BLE cell can support*/
      arch_pattern_type = 1 + determine_max_sat_type(ble_arch,cur_ble->next,cell_idx,verbose); 
      pattern_lgknds = (t_lgknd**)my_malloc(arch_pattern_type*sizeof(t_lgknd*));
      memset(pattern_lgknds,0,arch_pattern_type*sizeof(t_lgknd*));
      if (1 == verbose) {
        printf("Info: Max Pattern type that cell(%d) can support is %d\n",cell_idx,arch_pattern_type);
      }
      while(1) {
        /*Pick a most attractive pattern*/
        pack_pattern_type = pick_pattern_fit_ble(lgkntwk,
                                                 arch_pattern_type,
                                                 pattern_lgknds,
                                                 cur_ble->next,
                                                 mpack_opts.alpha,
                                                 mpack_opts.beta,
                                                 mpack_opts.pattern_member_type,
                                                 mpack_opts.packer_area_attraction,
                                                 mpack_opts.forbid_unrelated_clustering,
                                                 verbose);
        // Check if exists available pattern
        if (0 == pack_pattern_type) {
          if (1 == verbose) {
            printf("Info: Pattern-0 reached.\n");
          }
          break;
        }
        assert(pack_pattern_type > 0);
        pattern_lgknds[pack_pattern_type-1]->try_pattern_types[pack_pattern_type-1] = 1;
        /*Try to pack, pack a logic node and all its predecessors*/
        if (1 == run_sat_solver(lgkntwk,ble_arch,cur_ble->next,pack_pattern_type-1,cell_idx,pattern_lgknds,timing_analysis,delay_info,mpack_opts.alpha,verbose)) {
          /*Pack Success*/
          if (1 == verbose) {
            printf("Info:(pattern_pack)Pattern-%d packed into logic block(Index:%d) cell_idx(Index:%d).\n",pack_pattern_type,cur_ble->next->idx,cell_idx);
          }
          break;
        } else {
          if (1 == verbose) {
            printf("Info:(pattern_pack)Pattern-%d FAIL packed into logic block(Index:%d) cell_idx(Index:%d).\n",pack_pattern_type,cur_ble->next->idx,cell_idx);
          } 
          assert(!(pack_pattern_type > (pattern_lgknds[pack_pattern_type-1]->input_num+1)));
        }
      }
      /*Clear SAT tried*/
      reset_lgkntwk_try_pattern_types(lgkntwk);
      /*Free it*/
      free(pattern_lgknds);
      pattern_lgknds = NULL;
      /*If the cell idx cannot be mapped*/
      blends_tried[cell_idx] = 1;
    }
    /*Clear flags*/
    reset_lgkntwk_try_pattern_types(lgkntwk);
    if ((1 == check_blank_ble_info(ble_arch,cur_ble->next)&&(1 == check_all_mapped(lgkntwk,1)))) {
      printf("\nInfo:(pattern_pack) No suitable lgknds for logic block architecture!\n");
      exit(1);
    }
    /*Incremental BLE counter*/
    printf("Info: Logic Block(Index: %d) packed.",cur_ble->next->idx);
    if (1 == verbose) {
      printf("\n");
    }
    else {
      printf("\r");
    } 
    cur_ble = cur_ble->next;
    ble_num++;
  }
 
  printf("Info: Circuit(%d gates,%d latches) is packed into %d Logic Blocks\n",lgkntwk->libgate_num,lgkntwk->latch_num,ble_num);

  /*Don't forget free resources*/
  if (NULL != pattern_lgknds) {
    free(pattern_lgknds);
  }
  free(blends_tried);

  return ret;
}



