/***********************************/
/*  Synthesizable Verilog Dumping  */
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

/* Include FPGA-SPICE utils */
#include "read_xml_spice_util.h"
#include "linkedlist.h"
#include "spice_utils.h"
#include "spice_backannotate_utils.h"

/* Include SynVerilog headers */
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_pbtypes.h"
#include "verilog_routing.h"
#include "verilog_top_netlist.h"

/* Global Variants available only in this source file */
static char* default_verilog_dir_name = "syn_verilogs/";
static char* default_lb_dir_name = "lb/";
static char* default_rr_dir_name = "routing/";
static char* default_submodule_dir_name = "sub_module/";

/***** Subroutines *****/
 
/* Top-level function*/
void vpr_dump_syn_verilog(t_vpr_setup vpr_setup,
                          t_arch Arch,
                          char* circuit_name) {
  /* Timer */
  clock_t t_start;
  clock_t t_end;
  float run_time_sec;
  
  int num_clocks = 0;
  
  /* Directory paths */
  char* verilog_dir_formatted = NULL;
  char* submodule_dir_path= NULL;
  char* lb_dir_path = NULL;
  char* rr_dir_path = NULL;
  char* top_netlist_file = NULL;
  char* top_netlist_path = NULL;

  char* chomped_parent_dir = NULL;
  char* chomped_circuit_name = NULL;

  /* Check if the routing architecture we support*/
  if (UNI_DIRECTIONAL != vpr_setup.RoutingArch.directionality) {
    vpr_printf(TIO_MESSAGE_ERROR, "FPGA synthesizable Verilog dumping only support uni-directional routing architecture!\n");
    exit(1);
  }
  
  /* We don't support mrFPGA */
#ifdef MRFPGA_H
  if (is_mrFPGA) {
    vpr_printf(TIO_MESSAGE_ERROR, "FPGA synthesizable verilog dumping do not support mrFPGA!\n");
    exit(1);
  }
#endif
  
  assert ( TRUE == vpr_setup.SynVerilogOpts.dump_syn_verilog);
 
  /* Start time count */
  t_start = clock();

  /* Format the directory paths */
  split_path_prog_name(circuit_name, '/', &chomped_parent_dir, &chomped_circuit_name);

  if (NULL != vpr_setup.SynVerilogOpts.syn_verilog_dump_dir) {
    verilog_dir_formatted = format_dir_path(vpr_setup.SynVerilogOpts.syn_verilog_dump_dir);
  } else { 
    verilog_dir_formatted = format_dir_path(my_strcat(format_dir_path(chomped_parent_dir),default_verilog_dir_name));
  }
  /* lb directory */
  (lb_dir_path) = my_strcat(verilog_dir_formatted, default_lb_dir_name);
  /* routing resources directory */
  (rr_dir_path) = my_strcat(verilog_dir_formatted, default_rr_dir_name);
  /* submodule_dir_path */
  (submodule_dir_path) = my_strcat(verilog_dir_formatted, default_submodule_dir_name);
  /* Top netlists dir_path */
  top_netlist_file = my_strcat(chomped_circuit_name, verilog_top_postfix);
  top_netlist_path = my_strcat(verilog_dir_formatted, top_netlist_file);
  
  /* Create directories */
  create_dir_path(verilog_dir_formatted);
  create_dir_path(lb_dir_path);
  create_dir_path(rr_dir_path);
  create_dir_path(submodule_dir_path);

  /* Get the number of clocks */
  num_clocks = count_netlist_clocks();

  /* backannotation to complete routing information */
  backannotate_vpr_post_route_info(); 

  /* Dump internal structures of submodules */
  /* 
  dump_verilog_submodules(submodule_dir_path, Arch, &vpr_setup.RoutingArch);
  */

  /* Dump routing resources: switch blocks, connection blocks and channel tracks */
  dump_verilog_routing_resources(rr_dir_path, Arch, &vpr_setup.RoutingArch, rr_node_indices);

  /* Dump logic blocks */
  dump_verilog_logic_blocks(lb_dir_path, &Arch);

  /* Dump top-level verilog */
  dump_verilog_top_netlist(chomped_circuit_name, top_netlist_path, lb_dir_path, rr_dir_path, 
                           rr_node_indices, num_clocks, *(Arch.spice));

  /* Dump SDC constraints */
  // dump_verilog_sdc_file();

  /* End time count */
  t_end = clock();
 
  run_time_sec = (float)(t_end - t_start) / CLOCKS_PER_SEC;
  vpr_printf(TIO_MESSAGE_INFO, "Synthesizable verilog dumping took %g seconds\n", run_time_sec);  

  /* Free */
  my_free(verilog_dir_formatted);
  my_free(lb_dir_path);
  my_free(rr_dir_path);
  my_free(top_netlist_file);
  my_free(top_netlist_path);
  my_free(submodule_dir_path);

  return;
}
