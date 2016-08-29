/***********************************/
/* Synthesizable Verilog Dumping   */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Include vpr structs*/
#include "util.h"
#include "physical_types.h"
#include "vpr_types.h"
#include "globals.h"
#include "rr_graph_util.h"
#include "rr_graph.h"
#include "rr_graph2.h"
#include "vpr_utils.h"

/* FPGA-SPICE utils */
#include "linkedlist.h"
#include "spice_utils.h"

/* syn_verilog globals */

/****** Subroutines *******/
void dump_verilog_file_header(FILE* fp,
                              char* usage) {
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d]) FileHandle is NULL!\n",__FILE__,__LINE__); 
    exit(1);
  } 
  fprintf(fp,"//-------------------------------------------\n");
  fprintf(fp,"//    FPGA Synthesizable Verilog Netlist     \n");
  fprintf(fp,"//    Description: %s \n",usage);
  fprintf(fp,"//           Author: Xifan TANG              \n");
  fprintf(fp,"//        Organization: EPFL/IC/LSI          \n");
  fprintf(fp,"//    Date: %s \n",my_gettime());
  fprintf(fp,"//-------------------------------------------\n");
  fprintf(fp,"//----- Time scale -----\n");
  fprintf(fp,"`timescale 1ns / 1ps\n");
  fprintf(fp,"\n");

  return;
}

/** Decode 1-level 4T1R MUX
 */
void decode_verilog_one_level_4t1r_mux(int path_id, 
                                       int bit_len, int* conf_bits) { 
  /* Check */
  assert(0 < bit_len);
  assert(NULL != conf_bits);
  assert((-1 < path_id)&&(path_id < bit_len/2 - 1));

  /* Last bit of WL should be 1 */
  conf_bits[bit_len-1] = 1;
  /* determine  which BL should be 1*/
  conf_bits[path_id] = 1;

  return;
}

/** Decode multi-level 4T1R MUX
 */
void decode_verilog_multilevel_4t1r_mux(int num_level, int num_input_basis,
                                        int mux_size, int path_id, 
                                        int bit_len, int* conf_bits) { 
  int i, active_basis_path_id;

  /* Check */
  assert(0 < bit_len);
  assert(NULL != conf_bits);
  assert((-1 < path_id)&&(path_id < bit_len/2 - 1));
  /* Start from first level to the last level */
  active_basis_path_id = path_id;
  for (i = 0; i < num_level; i++) {
    /* Treat each basis as a 1-level 4T1R MUX */
    active_basis_path_id = active_basis_path_id % num_input_basis;
    /* Last bit of WL should be 1 */
    conf_bits[bit_len/2 + (num_input_basis+1)*(i+1) - 1] = 1;
    /* determine  which BL should be 1*/
    conf_bits[(num_input_basis+1)*i + active_basis_path_id] = 1;
  }

  return;
}

/** Decode the configuration bits for a 4T1R-based MUX
 *  Determine the number of configuration bits 
 *  Configuration bits are decoded depending on the MUX structure:
 *  1. 1-level; 2. multi-level (tree-like);
 */
void decode_verilog_rram_mux(t_spice_model* mux_spice_model,
                             int mux_size, int path_id,
                             int* bit_len, int** conf_bits, int* mux_level) {
  int num_level, num_input_basis;

  /* Check */
  assert(NULL != mux_level);
  assert(NULL != bit_len);
  assert(NULL != conf_bits);
  assert((-1 < path_id)&&(path_id < mux_size));
  assert(SPICE_MODEL_MUX == mux_spice_model->type);
  assert(SPICE_MODEL_DESIGN_RRAM == mux_spice_model->design_tech);
  
  /* Initialization */
  (*mux_level) = 0;
  (*bit_len) = 0;
  (*conf_bits) = NULL;
  
  /* Switch cases: MUX structure */
  switch (mux_spice_model->structure) {
  case SPICE_MODEL_STRUCTURE_ONELEVEL:
    /* Number of configuration bits is 2*(input_size+1) */
    num_level = 1;
    (*bit_len) = 2 * (mux_size + 1);
    break;
  case SPICE_MODEL_STRUCTURE_TREE:
    /* Number of configuration bits is num_level* 2*(basis+1) */
    num_level = determine_tree_mux_level(mux_size); 
    num_input_basis = 2;
    (*bit_len) = num_level * 2 * (num_input_basis + 1);
    break;
  case SPICE_MODEL_STRUCTURE_MULTILEVEL:
    /* Number of configuration bits is num_level* 2*(basis+1) */
    num_level = mux_spice_model->mux_num_level; 
    num_input_basis = determine_num_input_basis_multilevel_mux(mux_size, num_level);
    (*bit_len) = num_level * 2 * (num_input_basis + 1);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid MUX structure!\n", 
               __FILE__, __LINE__);
    exit(1);
  }
 
  /* Malloc configuration bits */
  (*conf_bits) = (int*)my_calloc((*bit_len), sizeof(int));

  /* Decode configuration bits : BL & WL*/
  /* Switch cases: MUX structure */
  switch (mux_spice_model->structure) {
  case SPICE_MODEL_STRUCTURE_ONELEVEL:
    decode_verilog_one_level_4t1r_mux(path_id, (*bit_len), (*conf_bits)); 
    break;
  case SPICE_MODEL_STRUCTURE_TREE:
  case SPICE_MODEL_STRUCTURE_MULTILEVEL:
    decode_verilog_multilevel_4t1r_mux(num_level, num_input_basis, mux_size, 
                                       path_id, (*bit_len), (*conf_bits)); 
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid MUX structure!\n", 
               __FILE__, __LINE__);
    exit(1);
  }
  
  (*mux_level) = num_level;
  
  return;
}

/* Determine the size of input address of a decoder */
int determine_decoder_size(int num_addr_out) {
  return ceil(log(num_addr_out)/log(2.));
}
