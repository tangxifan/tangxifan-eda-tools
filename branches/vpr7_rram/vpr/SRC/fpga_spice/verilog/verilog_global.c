/***********************************/
/*  Synthesizable Verilog Dumping  */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
#include <stdio.h>
#include "spice_types.h"
#include "linkedlist.h"
#include "fpga_spice_globals.h"
#include "verilog_global.h"

char* verilog_netlist_file_postfix = ".v";

char* default_verilog_dir_name = "syn_verilogs/";
char* default_lb_dir_name = "lb/";
char* default_rr_dir_name = "routing/";
char* default_submodule_dir_name = "sub_module/";

char* verilog_top_postfix = "_top.v";
char* top_testbench_verilog_file_postfix = "_top_tb.v";
char* blif_testbench_verilog_file_postfix = "_blif_tb.v";
char* submodule_verilog_file_name = "sub_module.v";
char* logic_block_verilog_file_name = "logic_blocks.v";
char* luts_verilog_file_name = "luts.v";
char* routing_verilog_file_name = "routing.v";
char* muxes_verilog_file_name = "muxes.v";
char* memories_verilog_file_name = "memories.v";
char* wires_verilog_file_name = "wires.v";
char* essentials_verilog_file_name = "inv_buf_passgate.v";
char* config_peripheral_verilog_file_name = "config_peripherals.v";

char* verilog_mux_basis_posfix = "_basis";
char* verilog_mux_special_basis_posfix = "_special_basis";
char* verilog_mem_posfix = "_mem";
char* verilog_config_peripheral_prefix = "config_peripheral";

/* Prefix for subckt Verilog netlists */
char* grid_verilog_file_name_prefix = "grid_";
char* chanx_verilog_file_name_prefix = "chanx_";
char* chany_verilog_file_name_prefix = "chany_";
char* sb_verilog_file_name_prefix = "sb_";
char* cbx_verilog_file_name_prefix = "cbx_";
char* cby_verilog_file_name_prefix = "cby_";

/* SRAM SPICE MODEL should be set as global*/
t_spice_model* sram_verilog_model = NULL;

/* Input and Output Pad spice model. should be set as global */
t_spice_model* iopad_verilog_model = NULL;

/* Linked-list that stores all the configuration bits */
t_llist* conf_bits_head = NULL;

/* Linked-list that stores submodule Verilog file mames */
t_llist* grid_verilog_subckt_file_path_head  = NULL;
t_llist* routing_verilog_subckt_file_path_head = NULL;
t_llist* submodule_verilog_subckt_file_path_head = NULL;

int verilog_default_signal_init_value = 0;

char* top_netlist_bl_enable_port_name = "en_bl";
char* top_netlist_wl_enable_port_name = "en_wl";
char* top_netlist_bl_data_in_port_name = "data_in";
char* top_netlist_addr_bl_port_name = "addr_bl";
char* top_netlist_addr_wl_port_name = "addr_wl";
char* top_netlist_array_bl_port_name = "bl_bus";
char* top_netlist_array_wl_port_name = "wl_bus";
char* top_netlist_array_blb_port_name = "blb_bus";
char* top_netlist_array_wlb_port_name = "wlb_bus";
char* top_netlist_reserved_bl_port_postfix = "_reserved_bl";
char* top_netlist_reserved_wl_port_postfix = "_reserved_wl";
char* top_netlist_normal_bl_port_postfix = "_bl";
char* top_netlist_normal_wl_port_postfix = "_wl";
char* top_netlist_normal_blb_port_postfix = "_blb";
char* top_netlist_normal_wlb_port_postfix = "_wlb";
char* top_netlist_scan_chain_head_prefix = "sc_in";

