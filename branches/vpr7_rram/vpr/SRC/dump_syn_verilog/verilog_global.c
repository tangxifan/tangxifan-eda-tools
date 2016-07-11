/***********************************/
/*  Synthesizable Verilog Dumping  */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
#include <stdio.h>
#include "spice_types.h"
#include "linkedlist.h"
#include "verilog_global.h"

char* verilog_top_postfix = "_top.sp";
char* logic_block_verilog_file_name = "logic_blocks.v";
char* luts_verilog_file_name = "luts.v";
char* routing_verilog_file_name = "routing.v";

/* SRAM SPICE MODEL should be set as global*/
t_spice_model* sram_verilog_model = NULL;

/* Default init_value */
int default_signal_init_value = 0;

