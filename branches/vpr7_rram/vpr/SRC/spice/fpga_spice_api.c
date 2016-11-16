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
#include "linkedlist.h"
#include "spice_utils.h"
#include "spice_backannotate_utils.h"
#include "spice_api.h"
#include "syn_verilog_api.h"

/* Top-level API of FPGA-SPICE */
void vpr_fpga_spice_tool_suites(t_vpr_setup vpr_setup,
                                t_arch Arch) {
  int num_clocks = 0;
  float vpr_crit_path_delay = 0.; 
  float vpr_clock_freq = 0.; 

  /* Common initializations and malloc operations */
  /* If FPGA-SPICE is not called, we should initialize the spice_models */
  if (TRUE == vpr_setup.FPGA_SPICE_Opts.do_fpga_spice) {

    /* Initialize Arch SPICE MODELS*/
    init_check_arch_spice_models(&Arch, &vpr_setup.RoutingArch);

    /* Initialize verilog netlist to be included */
    /* Add keyword checking */
    check_keywords_conflict(Arch);

    /* Check Activity file is valid */
    if (TRUE == vpr_setup.FPGA_SPICE_Opts.read_act_file) {
      if (1 == try_access_file(vpr_setup.FileNameOpts.ActFile)) {
        vpr_printf(TIO_MESSAGE_ERROR,"Activity file (%s) does not exists! Please provide a valid file path!\n",
                   vpr_setup.FileNameOpts.ActFile);
        exit(1);
      } else {
        vpr_printf(TIO_MESSAGE_INFO,"Check Activity file (%s) is a valid file path!\n",
                   vpr_setup.FileNameOpts.ActFile);
      }
    }

    /* Backannotation for post routing information */
    spice_backannotate_vpr_post_route_info(vpr_setup.RoutingArch,
                                           vpr_setup.FPGA_SPICE_Opts.SpiceOpts.fpga_spice_parasitic_net_estimation_off);

    /* Auto check the density and recommend sim_num_clock_cylce */
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
    Arch.spice->spice_params.stimulate_params.num_clocks = num_clocks;
    Arch.spice->spice_params.stimulate_params.vpr_crit_path_delay = vpr_crit_path_delay;
    auto_select_num_sim_clock_cycle(Arch.spice);
  }
  

  /* Xifan TANG: SPICE Modeling, SPICE Netlist Output  */ 
  if (vpr_setup.FPGA_SPICE_Opts.SpiceOpts.do_spice) {
    vpr_print_spice_netlists(vpr_setup, Arch, vpr_setup.FileNameOpts.CircuitName);
  }

  /* Xifan TANG: Synthesizable verilog dumping */
  if (vpr_setup.FPGA_SPICE_Opts.SynVerilogOpts.dump_syn_verilog) {
    vpr_dump_syn_verilog(vpr_setup, Arch, vpr_setup.FileNameOpts.CircuitName);
  }	

  /* Free */
  if (TRUE == vpr_setup.FPGA_SPICE_Opts.do_fpga_spice) {
    /* Free all the backannotation containing post routing information */
    free_backannotate_vpr_post_route_info();
  }

  return;
}

