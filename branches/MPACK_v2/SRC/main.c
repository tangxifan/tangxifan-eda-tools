#include <stdio.h>
#include <string.h>
#include <assert.h> 
#include <stdlib.h>
#include "util.h"
#include "blif_parser.h"
#include "mpack_conf.h"
#include "mpack_types.h"
#include "Options.h"
#include "blearch_parser.h"
#include "parsers.h"
#include "mpack_util.h"
#include "stats.h"
#include "mpack_api.h"


/*****Subroutines*****/
void free_all_resrc(t_blif_arch* blif_arch,
                    t_lgkntwk* lgkntwk,
                    t_ble_arch* ble_arch,
                    t_ble_info* ble_info);

void back_annotate_net_name(t_blif_arch* blif_arch,
                            t_lgkntwk* lgkntwk,
                            int verbose);

/*****Print Functions*****/
void print_usage()
{
  puts("Usage:"); 
  puts("      mpack2 [-Options <val>]...");
  puts("Mandatory Option:");
  puts("  -blif <blif_file>: input blif.");
  puts("  -ble_arch <arch_file>: Architecture description for BLE");
  puts("Optional:");
  puts("  -mpack_blif <mpack_blif_file>: print mpack output blif.");
  puts("  -net  <net_file>: print VPR netlist.");
  puts("  -vpr_arch <vpr_arch_xml>: Output Architecture description for VPR");
  puts("  -stats <stats_file>: Log file of MPACK (default: <blif_file>.stats)");
  puts("  -timing_analysis <string> : turn (on|off) timing analysis (DEFAULT: ON)");
  puts("  -alpha <FLOAT>: coefficiency to balance the timing-weight and depth-weight(DEFAULT:0.75)");
  puts("  -beta <FLOAT>: coefficiency to balance the input_absorb and output_absorb(DEFAULT:0.9)");
  puts("  -read_stdlib_blif: read the standard library mapped blif file.(DEFAULT: off)");
  puts("  -pattern_member_type: comb_only|normal (DEFAULT: normal)");
  puts("  -inter_cluster_delay <FLOAT> : inter-cluster delay(unit: sec) (DEFAULT: 8.67e-10)");
  puts("  -packer_algorithm <string>: pattern|sat|seed, choose the packer (DEFAULT: pattern)");
  puts("  -packer_area_attraction <string>: AAPACK|VPACK, choose the packer (DEFAULT: VPACK)");
  puts("  -forbid_unrelated_clustering: forbid zero related candidate join cluster(DEFAULT: off)");
  puts("  -verbose: turn on verbose mode");
  puts("  -help: show usage");
  
}

/*****Print Program Title*****/
void print_title()
{
  puts("MPACK V2.0 by Xifan TANG, LSI, EPFL");
  puts("Technology mapping and resynthesis for AmiBipolar-Transistor based FPGAs");
  puts("Compiled on " __DATE__);
  puts("------------------");
}


/*****Main Program*****/
int main(int argc,char **argv)
{
  /*Critical definitions, equivalent globals*/
  t_optinfo* optlst_ptr;
  t_mpack_conf mpack_opts;
  t_blif_arch blif_arch;
  t_lgkntwk lgkntwk;
  t_stats stats;
  t_ble_arch ble_arch;
  t_ble_info* ble_info;
  t_delay_info delay_info;
  
  /*Print Program Title*/
  print_title();
 
  /*Read in Arguments*/ 
  optlst_ptr = read_options(argc,argv);
  
  /*
   * Configure Mpack settings 
   * and Check Options
   */
  if (0 == config_mpack(&mpack_opts,optlst_ptr)) {
    print_usage();
    exit(1);
  }  

  /* Read in Blif file*/
  //blif_parser(mpack_opts.blif,&blif_arch);
  blif_parser_flow(mpack_opts.blif,&blif_arch);

  /*Setup MPack, construct the logic node network*/
  printf("Start setup MPACK...\n");
  setup_mpack(&blif_arch,&lgkntwk,&stats,mpack_opts.read_stdlib_blif);
  // Statistic logic network
  stats_lgkntwk(&lgkntwk,&stats);

  /*Parse the BLE architecture settings*/
  ble_arch_parser_flow(mpack_opts.ble_arch,&ble_arch);
  /*Check BLE Architecture.*/
  check_ble_arch(&ble_arch);

  /*Packing...Here comes the main body*/
  init_delay_info(&delay_info,mpack_opts,ble_arch);

  // Packing
  ble_info = pack_BLEs(&lgkntwk,&stats,&ble_arch,mpack_opts.timing_analysis,&delay_info,mpack_opts,mpack_opts.verbose);
  back_annotate_net_name(&blif_arch,&lgkntwk,mpack_opts.verbose);

  /*Output VPR7-capable Arch*/
  print_vpr7_arch(&ble_arch,mpack_opts.vpr_arch,delay_info,mpack_opts.verbose);

  /*Print out Statistics*/
  print_stats(mpack_opts.stats,&stats,&blif_arch,&lgkntwk,&ble_arch,ble_info,mpack_opts.verbose);  

  /*Print mpack-output blif*/
  print_blif(mpack_opts.mpack_blif,&lgkntwk,&ble_arch,ble_info,blif_arch.model,mpack_opts.verbose);

  /*Print VPR7-capable netlist*/
  print_vpr7_netlist(mpack_opts.net,&ble_arch,ble_info,&lgkntwk,mpack_opts.verbose);

  /*Free all resources*/
  free_all_resrc(&blif_arch,&lgkntwk,&ble_arch,ble_info);

  return 1;
}
 
void back_annotate_net_name(t_blif_arch* blif_arch,
                            t_lgkntwk* lgkntwk,
                            int verbose)
{
  int i,idx;

  for (i=0; i<lgkntwk->pi_num; i++) {
    idx = lgkntwk->pi_ptrs[i]->idx;
    lgkntwk->pi_ptrs[i]->net_name = my_strdup(blif_arch->inputs[idx].name);
  }
}

void free_all_resrc(t_blif_arch* blif_arch,
                    t_lgkntwk* lgkntwk,
                    t_ble_arch* ble_arch,
                    t_ble_info* ble_info)
{
  printf("Free Blif Arch...");
  free_blif_arch(blif_arch);
  printf("Complete\n");

  printf("Free BLE Arch...");
  free_ble_arch(ble_arch);
  printf("Complete\n");

  printf("Free BLE infos...");
  free_ble_info_list(ble_info);
  printf("Complete\n");

  printf("Free Logic Network...");
  free_lgkntwk(lgkntwk);
  printf("Complete\n");

}
