/* global parameters for SPICE support*/
extern char* nmos_subckt_name;
extern char* pmos_subckt_name;
extern char* rram_veriloga_file_name;
extern char* mux_basis_posfix;
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
extern char* top_netlist_postfix;
extern char* spice_grid_testbench_postfix;
extern char* spice_mux_testbench_postfix;
extern char* spice_routing_mux_testbench_postfix;
/* RUN HSPICE Shell Script Name */
extern char* run_hspice_shell_script_name;
extern char* sim_results_dir_path;
extern char* default_spice_dir_path;

extern t_spice_model* sram_spice_model;
extern int rram_design_tech;
 
/* Enumeration */
enum e_pin2pin_interc_type {
 INPUT2INPUT_INTERC, OUTPUT2OUTPUT_INTERC
};

