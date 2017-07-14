/* global parameters for SPICE support*/
extern char* nmos_subckt_name;
extern char* pmos_subckt_name;
extern char* io_nmos_subckt_name;
extern char* io_pmos_subckt_name;
extern char* cpt_subckt_name;
extern char* rram_veriloga_file_name;
extern char* mux_basis_posfix;
extern char* mux_special_basis_posfix;
extern char* nmos_pmos_spice_file_name;
extern char* basics_spice_file_name;
extern char* muxes_spice_file_name;
extern char* wires_spice_file_name;
extern char* logic_block_spice_file_name;
extern char* luts_spice_file_name;
extern char* routing_spice_file_name;
extern char* meas_header_file_name;
extern char* stimu_header_file_name;

/* Testbench names */
extern char* spice_top_testbench_postfix;
extern char* spice_grid_testbench_postfix;
extern char* spice_pb_mux_testbench_postfix;
extern char* spice_cb_mux_testbench_postfix;
extern char* spice_sb_mux_testbench_postfix;
extern char* spice_cb_testbench_postfix;
extern char* spice_sb_testbench_postfix;
extern char* spice_lut_testbench_postfix;
extern char* spice_dff_testbench_postfix;
extern char* bitstream_spice_file_postfix;
/* RUN HSPICE Shell Script Name */
/*
extern char* run_hspice_shell_script_name;
extern char* default_spice_dir_path;
extern char* sim_results_dir_name;
extern char* spice_top_tb_dir_name;
extern char* spice_grid_tb_dir_name;
extern char* spice_pb_mux_tb_dir_name;
extern char* spice_cb_mux_tb_dir_name;
extern char* spice_sb_mux_tb_dir_name;
extern char* spice_lut_tb_dir_name;
extern char* spice_dff_tb_dir_name;
*/

extern t_spice_model* sram_spice_model;
extern enum e_sram_orgz sram_spice_orgz_type;
extern t_sram_orgz_info* sram_spice_orgz_info;

extern int rram_design_tech;
extern int num_used_grid_mux_tb;
extern int num_used_cb_mux_tb;
extern int num_used_sb_mux_tb;
extern int num_used_grid_tb;
extern int num_used_cb_tb;
extern int num_used_sb_tb;
extern int num_used_lut_tb;
extern int num_used_dff_tb;
extern t_llist* tb_head;
/* Heads of scan-chain */
extern t_llist* scan_chain_heads;

/* Name of global ports used in all netlists */
extern char* spice_top_netlist_global_vdd_port;
extern char* spice_top_netlist_global_set_port;
extern char* spice_top_netlist_global_reset_port;
extern char* spice_top_netlist_global_vdd_localrouting_port;
extern char* spice_top_netlist_global_vdd_io_port;
extern char* spice_top_netlist_global_vdd_hardlogic_port;
extern char* spice_top_netlist_global_vdd_sram_port;
extern char* spice_top_netlist_global_clock_port;


