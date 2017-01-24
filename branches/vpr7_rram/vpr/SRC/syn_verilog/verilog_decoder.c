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
#include "linkedlist.h"
#include "fpga_spice_utils.h"
#include "spice_mux.h"
#include "fpga_spice_globals.h"

/* Include verilog utils */
#include "verilog_utils.h"
#include "verilog_global.h"

/***** Subroutines *****/
void dump_verilog_decoder(char* submodule_dir) {
  int num_mem_bit, num_bl, num_wl, bl_decoder_size, wl_decoder_size;
  int num_reserved_bl, num_reserved_wl;
  FILE* fp = NULL;

  char* verilog_name = my_strcat(submodule_dir, decoders_verilog_file_name);
  
  /* Print the muxes netlist*/
  fp = fopen(verilog_name, "w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create decoder SPICE netlist %s",__FILE__, __LINE__, verilog_name); 
    exit(1);
  } 

  /* Check */
  assert(SPICE_SRAM_MEMORY_BANK == sram_verilog_orgz_type);

  num_mem_bit = get_sram_orgz_info_num_mem_bit(sram_verilog_orgz_info);
  get_sram_orgz_info_num_blwl(sram_verilog_orgz_info, &num_bl, &num_wl);
  get_sram_orgz_info_reserved_blwl(sram_verilog_orgz_info, &num_reserved_bl, &num_reserved_wl);
  bl_decoder_size = determine_decoder_size(num_bl);
  wl_decoder_size = determine_decoder_size(num_wl);
   
  /* Generate file header*/ 
  vpr_printf(TIO_MESSAGE_INFO, "Writing Decoder verilog netlist...\n");
 
  /* Generate the descriptions*/
  dump_verilog_file_header(fp, " Verilog Decoders");

  /* Start the BL decoder module definition */
  fprintf(fp, "//----- BL Decoder convert %d bits to binary %d bits -----\n",
          bl_decoder_size, num_bl);
  fprintf(fp, "module bl_decoder%dto%d (\n",
               bl_decoder_size, num_bl);
  fprintf(fp, "input wire enable,\n"); 
  fprintf(fp, "input wire [%d:0] addr_in,\n",
               bl_decoder_size-1);
  fprintf(fp, "output reg [0:%d] addr_out\n",
               num_bl-1);
  fprintf(fp, ");\n");
  
  /* Internal logics */
  /*
  fprintf(fp, "assign decoder_out = (enable) ? (1 << binary_in):%d'b0;\n",
          num_bl);
  */
  fprintf(fp, "always@(addr_out,addr_in,enable)\n");
  fprintf(fp, "begin\n");
  fprintf(fp, "\taddr_out = %d'b0;\n", num_bl);
  fprintf(fp, "\tif (1'b1 == enable) begin\n");
  fprintf(fp, "\t\taddr_out[addr_in] = 1'b1;\n");
  fprintf(fp, "\tend\n");
  fprintf(fp, "end\n");

  fprintf(fp, "endmodule\n");

  /* Start the WL module definition */
  fprintf(fp, "//----- WL Decoder convert %d bits to binary %d bits -----\n",
          wl_decoder_size, num_wl);
  fprintf(fp, "module wl_decoder%dto%d (\n",
               wl_decoder_size, num_wl);
  fprintf(fp, "input wire enable,\n"); 
  fprintf(fp, "input wire [%d:0] addr_in,\n",
               wl_decoder_size-1);
  fprintf(fp, "output reg [0:%d] addr_out\n",
               num_bl-1);
  fprintf(fp, ");\n");
  
  /* Internal logics */
  /*
  fprintf(fp, "assign decoder_out = (enable) ? (1 << binary_in):%d'b0;\n",
          num_bl);
  */
  fprintf(fp, "always@(addr_out,addr_in,enable)\n");
  fprintf(fp, "begin\n");
  fprintf(fp, "\taddr_out = %d'b0;\n", num_wl);
  fprintf(fp, "\tif (1'b1 == enable) begin\n");
  fprintf(fp, "\t\taddr_out[addr_in] = 1'b1;\n");
  fprintf(fp, "\tend\n");
  fprintf(fp, "end\n");

  fprintf(fp, "endmodule\n");

  

  /* Close the file*/
  fclose(fp);
  
  return;
}
