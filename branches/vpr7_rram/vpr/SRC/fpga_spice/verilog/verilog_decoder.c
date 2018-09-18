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
#include "fpga_spice_bitstream_utils.h"
#include "fpga_spice_globals.h"

/* Include verilog utils */
#include "verilog_global.h"
#include "verilog_utils.h"

/***** Subroutines *****/

void dump_verilog_decoder(t_sram_orgz_info* cur_sram_orgz_info,
                          char* submodule_dir) {
  int num_array_bl, num_array_wl;
  int bl_decoder_size, wl_decoder_size;
  FILE* fp = NULL;
  t_spice_model* mem_model = NULL;
  boolean bl_inverted = FALSE;
  boolean wl_inverted = FALSE;

  char* verilog_name = my_strcat(submodule_dir, decoders_verilog_file_name);
  
  /* Print the muxes netlist*/
  fp = fopen(verilog_name, "w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create decoder SPICE netlist %s", 
               __FILE__, __LINE__, verilog_name); 
    exit(1);
  } 

  /* Check */
  assert(SPICE_SRAM_MEMORY_BANK == cur_sram_orgz_info->type);

  /* Get number of BLs,WLs and decoder sizes */
  determine_blwl_decoder_size(cur_sram_orgz_info, 
                              &num_array_bl, &num_array_wl, 
                              &bl_decoder_size, &wl_decoder_size);
   
  /* Generate file header*/ 
  vpr_printf(TIO_MESSAGE_INFO, "Writing Decoder verilog netlist...\n");
 
  /* Generate the descriptions*/
  dump_verilog_file_header(fp, " Verilog Decoders");

  /* Different design technology requires different BL decoder logic */
  get_sram_orgz_info_mem_model(cur_sram_orgz_info, &mem_model); 
  /* Find if we need an inversion of the BL */
  check_mem_model_blwl_inverted(mem_model, SPICE_MODEL_PORT_BL, &bl_inverted); 
  check_mem_model_blwl_inverted(mem_model, SPICE_MODEL_PORT_WL, &wl_inverted); 

  switch (mem_model->design_tech) {
  case SPICE_MODEL_DESIGN_CMOS: /* CMOS SRAM*/
    /* SRAM technology requires its BL decoder has an additional input called data_in 
     * only the selected BL will be set to the value of data_in, other BLs will be in high-resistance state
     */
     /* Start the BL decoder module definition */
    fprintf(fp, "//----- BL Decoder convert %d bits to binary %d bits -----\n",
            bl_decoder_size, num_array_bl);
    fprintf(fp, "module bl_decoder%dto%d (\n",
                 bl_decoder_size, num_array_bl);
    fprintf(fp, "input wire enable,\n"); 
    fprintf(fp, "input wire [%d:0] addr_in,\n",
               bl_decoder_size - 1);
    fprintf(fp, "input wire data_in,\n"); 
    fprintf(fp, "output reg [0:%d] addr_out\n",
               num_array_bl - 1);
    fprintf(fp, ");\n");
  
    /* Wee need to know the default value of bl port and wl port */

    /* Internal logics */

    fprintf(fp, "always@(addr_out,addr_in,enable, data_in)\n");
    fprintf(fp, "begin\n");
    fprintf(fp, "\taddr_out = %d'bz;\n", num_array_bl);
    fprintf(fp, "\tif (1'b1 == enable) begin\n");
    fprintf(fp, "\t\taddr_out[addr_in] = data_in;\n");
    fprintf(fp, "\tend\n");
    fprintf(fp, "end\n");

    fprintf(fp, "endmodule\n");
    break;
  case SPICE_MODEL_DESIGN_RRAM: /* RRAM */
    /* For RRAM technology, BL decoder should be same as the WL decoder */
    /* Start the BL decoder module definition */
    fprintf(fp, "//----- BL Decoder convert %d bits to binary %d bits -----\n",
            bl_decoder_size, num_array_bl);
    fprintf(fp, "module bl_decoder%dto%d (\n",
                 bl_decoder_size, num_array_bl);
    fprintf(fp, "input wire enable,\n"); 
    fprintf(fp, "input wire [%d:0] addr_in,\n",
               bl_decoder_size-1);
    fprintf(fp, "output reg [0:%d] addr_out\n",
               num_array_bl-1);
    fprintf(fp, ");\n");
  
    /* Internal logics */
    fprintf(fp, "always@(addr_out,addr_in,enable)\n");
    fprintf(fp, "begin\n");
    if (TRUE == bl_inverted) {
      fprintf(fp, "\taddr_out = %d'b1;\n", num_array_bl);
    } else {
      assert (FALSE == bl_inverted);
      fprintf(fp, "\taddr_out = %d'b0;\n", num_array_bl);
    }
    fprintf(fp, "\tif (1'b1 == enable) begin\n");
    if (TRUE == bl_inverted) {
      fprintf(fp, "\t\taddr_out[addr_in] = 1'b0;\n");
    } else {
      assert (FALSE == bl_inverted);
      fprintf(fp, "\t\taddr_out[addr_in] = 1'b1;\n");
    }
    fprintf(fp, "\tend\n");
    fprintf(fp, "end\n");

    fprintf(fp, "endmodule\n");
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid design technology [CMOS|RRAM] for memory technology!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* WL decoder logic is the same whatever SRAM or RRAM technology is considered */
  /* Start the WL module definition */
  fprintf(fp, "//----- WL Decoder convert %d bits to binary %d bits -----\n",
          wl_decoder_size, num_array_wl);
  fprintf(fp, "module wl_decoder%dto%d (\n",
               wl_decoder_size, num_array_wl);
  fprintf(fp, "input wire enable,\n"); 
  fprintf(fp, "input wire [%d:0] addr_in,\n",
               wl_decoder_size-1);
  fprintf(fp, "output reg [0:%d] addr_out\n",
               num_array_bl-1);
  fprintf(fp, ");\n");
  
  /* Internal logics */
  fprintf(fp, "always@(addr_out,addr_in,enable)\n");
  fprintf(fp, "begin\n");
  if (TRUE == wl_inverted) {
    fprintf(fp, "\taddr_out = %d'b1;\n", num_array_wl);
  } else {
    assert (FALSE == wl_inverted);
    fprintf(fp, "\taddr_out = %d'b0;\n", num_array_wl);
  }
  fprintf(fp, "\tif (1'b1 == enable) begin\n");
  if (TRUE == wl_inverted) {
    fprintf(fp, "\t\taddr_out[addr_in] = 1'b0;\n");
  } else {
    assert (FALSE == wl_inverted);
    fprintf(fp, "\t\taddr_out[addr_in] = 1'b1;\n");
  }
  fprintf(fp, "\tend\n");
  fprintf(fp, "end\n");

  fprintf(fp, "endmodule\n");

  /* Close the file*/
  fclose(fp);
  
  return;
}


void dump_verilog_mem_decoders(t_sram_orgz_info* cur_sram_orgz_info,
                               char* submodule_dir_path) {
  switch(cur_sram_orgz_info->type) {
  case SPICE_SRAM_STANDALONE:
  case SPICE_SRAM_SCAN_CHAIN:
    break;
  case SPICE_SRAM_MEMORY_BANK:
    /* Dump verilog decoder */
    dump_verilog_decoder(cur_sram_orgz_info, submodule_dir_path);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type of SRAM organization in Verilog Generator!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  return;
}

