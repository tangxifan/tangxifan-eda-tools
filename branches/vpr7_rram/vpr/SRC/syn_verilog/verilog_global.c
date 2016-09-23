/***********************************/
/*  Synthesizable Verilog Dumping  */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
#include <stdio.h>
#include "spice_types.h"
#include "linkedlist.h"
#include "verilog_global.h"

char* verilog_top_postfix = "_top.v";
char* logic_block_verilog_file_name = "logic_blocks.v";
char* luts_verilog_file_name = "luts.v";
char* routing_verilog_file_name = "routing.v";
char* muxes_verilog_file_name = "muxes.v";
char* wires_verilog_file_name = "wires.v";
char* decoders_verilog_file_name = "decoders.v";

char* verilog_mux_basis_posfix = "_basis";
char* verilog_mux_special_basis_posfix = "_special_basis";

/* SRAM SPICE MODEL should be set as global*/
t_spice_model* sram_verilog_model = NULL;
enum e_sram_orgz sram_verilog_orgz_type = SPICE_SRAM_STANDALONE;

/* Number of configuration bits of each switch block */
int** num_conf_bits_sb = NULL;
/* Number of configuration bits of each Connection Box CHANX */
int** num_conf_bits_cbx = NULL;
/* Number of configuration bits of each Connection Box CHANY */
int** num_conf_bits_cby = NULL;

/* Input and Output Pad spice model. should be set as global */
t_spice_model* inpad_verilog_model = NULL;
t_spice_model* outpad_verilog_model = NULL;
t_spice_model* iopad_verilog_model = NULL;
/* Linked-list that stores all the configuration bits */
t_llist* conf_bits_head = NULL;

/* Prefix of global input, output and inout of a I/O pad */
char* gio_input_prefix = "gfpga_input_";
char* gio_output_prefix = "gfpga_output_";
char* gio_inout_prefix = "gfpga_pad_";
