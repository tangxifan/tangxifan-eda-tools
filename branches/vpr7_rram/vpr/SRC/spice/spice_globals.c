/***********************************/
/*      SPICE Modeling for VPR     */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
/* Include SPICE support headers*/
#include <stdio.h>
#include "spice_types.h"
#include "spice_globals.h"

char* nmos_subckt_name = "vpr_nmos";
char* pmos_subckt_name = "vpr_pmos";
char* mux_basis_posfix = "_basis";
char* nmos_pmos_spice_file_name = "nmos_pmos.sp";
char* basics_spice_file_name = "inv_buf_trans_gate.sp";
char* muxes_spice_file_name = "muxes.sp";
char* rram_veriloga_file_name = "rram_behavior.va";
char* wires_spice_file_name = "wires.sp";
char* logic_block_spice_file_name = "logic_blocks.sp";
char* luts_spice_file_name = "luts.sp";
char* routing_spice_file_name = "routing.sp";
char* meas_header_file_name = "meas_params.sp";
char* stimu_header_file_name = "stimulate_params.sp";

/* Testbench names */
char* top_netlist_postfix = "_top.sp";
char* spice_grid_testbench_postfix = "_grid_testbench.sp";
char* spice_mux_testbench_postfix = "_mux_testbench.sp";
char* spice_routing_mux_testbench_postfix = "_routing_mux_testbench.sp";
/* RUN HSPICE Shell Script Name */
char* run_hspice_shell_script_name = "run_hspice_sim.sh";
char* sim_results_dir_path = "results/";
char* default_spice_dir_path = "spice_netlists/";

/* SRAM SPICE MODEL should be set as global*/
t_spice_model* sram_spice_model = NULL;
int rram_design_tech = 0;
