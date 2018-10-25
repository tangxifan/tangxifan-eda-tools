/* global parameters for dumping synthesizable verilog */

extern char* verilog_netlist_file_postfix;
extern float verilog_sim_timescale;
extern char* verilog_timing_preproc_flag; // the flag to enable timing definition during compilation

extern char* default_verilog_dir_name;
extern char* default_lb_dir_name;
extern char* default_rr_dir_name;
extern char* default_submodule_dir_name;

extern char* modelsim_project_name_postfix;
extern char* modelsim_proc_script_name_postfix;
extern char* modelsim_top_script_name_postfix;
extern char* modelsim_testbench_module_postfix;
extern char* modelsim_simulation_time_unit;

extern char* verilog_top_postfix;
extern char* top_testbench_verilog_file_postfix;
extern char* blif_testbench_verilog_file_postfix;
extern char* submodule_verilog_file_name;
extern char* logic_block_verilog_file_name;
extern char* luts_verilog_file_name;
extern char* routing_verilog_file_name;
extern char* muxes_verilog_file_name;
extern char* memories_verilog_file_name;
extern char* wires_verilog_file_name;
extern char* essentials_verilog_file_name;
extern char* config_peripheral_verilog_file_name;

extern char* verilog_mux_basis_posfix;
extern char* verilog_mux_special_basis_posfix;
extern char* verilog_mem_posfix;
extern char* verilog_config_peripheral_prefix;

/* Prefix for subckt Verilog netlists */
extern char* grid_verilog_file_name_prefix;
extern char* chanx_verilog_file_name_prefix;
extern char* chany_verilog_file_name_prefix;
extern char* sb_verilog_file_name_prefix;
extern char* cbx_verilog_file_name_prefix;
extern char* cby_verilog_file_name_prefix;

extern t_spice_model* sram_verilog_model;

/* Input and Output Pad spice model. should be set as global */
extern t_spice_model* inpad_verilog_model;
extern t_spice_model* outpad_verilog_model;
extern t_spice_model* iopad_verilog_model;

/* Linked-list that stores all the configuration bits */
extern t_llist* conf_bits_head;

/* Linked-list that stores submodule Verilog file mames */
extern t_llist* grid_verilog_subckt_file_path_head;
extern t_llist* routing_verilog_subckt_file_path_head;
extern t_llist* submodule_verilog_subckt_file_path_head;

extern int verilog_default_signal_init_value;

extern char* top_netlist_bl_enable_port_name;
extern char* top_netlist_wl_enable_port_name;
extern char* top_netlist_bl_data_in_port_name;
extern char* top_netlist_addr_bl_port_name;
extern char* top_netlist_addr_wl_port_name;
extern char* top_netlist_array_bl_port_name;
extern char* top_netlist_array_wl_port_name;
extern char* top_netlist_array_blb_port_name;
extern char* top_netlist_array_wlb_port_name;
extern char* top_netlist_reserved_bl_port_postfix;
extern char* top_netlist_reserved_wl_port_postfix;
extern char* top_netlist_normal_bl_port_postfix;
extern char* top_netlist_normal_wl_port_postfix;
extern char* top_netlist_normal_blb_port_postfix;
extern char* top_netlist_normal_wlb_port_postfix;
extern char* top_netlist_scan_chain_head_prefix;

enum e_dump_verilog_port_type {
VERILOG_PORT_INPUT,
VERILOG_PORT_OUTPUT,
VERILOG_PORT_INOUT,
VERILOG_PORT_WIRE,
VERILOG_PORT_REG,
VERILOG_PORT_CONKT
};
