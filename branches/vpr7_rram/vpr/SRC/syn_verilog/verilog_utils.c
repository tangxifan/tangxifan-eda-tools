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
#include "read_xml_spice_util.h"
#include "linkedlist.h"
#include "spice_utils.h"

/* syn_verilog globals */

/****** Subroutines *******/
void init_list_include_verilog_netlists(t_spice* spice) { 
  int i, j, cur;
  int to_include = 0;
  int num_to_include = 0;

  /* Initialize */
  for (i = 0; i < spice->num_include_netlist; i++) { 
    FreeSpiceModelNetlist(&(spice->include_netlists[i]));
  }
  my_free(spice->include_netlists);
  spice->include_netlists = NULL;
  spice->num_include_netlist = 0;

  /* Generate include netlist list */
  vpr_printf(TIO_MESSAGE_INFO, "Listing Verilog Netlist Names to be included...\n");
  for (i = 0; i < spice->num_spice_model; i++) {
    if (NULL != spice->spice_models[i].verilog_netlist) {
      /* Check if this netlist name has already existed in the list */
      to_include = 1;
      for (j = 0; j < i; j++) {
        if (NULL == spice->spice_models[j].verilog_netlist) {
          continue;
        }
        if (0 == strcmp(spice->spice_models[j].verilog_netlist, spice->spice_models[i].verilog_netlist)) {
          to_include = 0;
          break;
        }
      }
      /* Increamental */
      if (1 == to_include) {
        num_to_include++;
      }
    }
  }

  /* realloc */
  spice->include_netlists = (t_spice_model_netlist*)my_realloc(spice->include_netlists, 
                              sizeof(t_spice_model_netlist)*(num_to_include + spice->num_include_netlist));

  /* Fill the new included netlists */
  cur = spice->num_include_netlist;
  for (i = 0; i < spice->num_spice_model; i++) {
    if (NULL != spice->spice_models[i].verilog_netlist) {
      /* Check if this netlist name has already existed in the list */
      to_include = 1;
      for (j = 0; j < i; j++) {
        if (NULL == spice->spice_models[j].verilog_netlist) {
          continue;
        }
        if (0 == strcmp(spice->spice_models[j].verilog_netlist, spice->spice_models[i].verilog_netlist)) {
          to_include = 0;
          break;
        }
      }
      /* Increamental */
      if (1 == to_include) {
        spice->include_netlists[cur].path = my_strdup(spice->spice_models[i].verilog_netlist); 
        spice->include_netlists[cur].included = 0;
        vpr_printf(TIO_MESSAGE_INFO, "[%d] %s\n", cur+1, spice->include_netlists[cur].path);
        cur++;
      }
    }
  }
  /* Check */
  assert(cur == (num_to_include + spice->num_include_netlist));
  /* Update */
  spice->num_include_netlist += num_to_include;
  
  return;
}


void init_include_user_defined_verilog_netlists(t_spice spice) {
  int i;

  /* Include user-defined sub-circuit netlist */
  for (i = 0; i < spice.num_include_netlist; i++) {
    spice.include_netlists[i].included = 0;
  }

  return;
}

void dump_include_user_defined_verilog_netlists(FILE* fp,
                                                t_spice spice) {
  int i;

  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid File Handler!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Include user-defined sub-circuit netlist */
  for (i = 0; i < spice.num_include_netlist; i++) {
    if (0 == spice.include_netlists[i].included) {
      assert(NULL != spice.include_netlists[i].path);
      fprintf(fp, "// `include \"%s\"\n", spice.include_netlists[i].path);
      spice.include_netlists[i].included = 1;
    } else {
      assert(1 == spice.include_netlists[i].included);
    }
  } 

  return;
}

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
  /* assert((-1 < path_id)&&(path_id < bit_len/2 - 1)); */
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

  (*bit_len) = 2* count_num_conf_bits_one_spice_model(mux_spice_model, mux_size);
  
  /* Switch cases: MUX structure */
  switch (mux_spice_model->design_tech_info.structure) {
  case SPICE_MODEL_STRUCTURE_ONELEVEL:
    /* Number of configuration bits is 2*(input_size+1) */
    num_level = 1;
    break;
  case SPICE_MODEL_STRUCTURE_TREE:
    /* Number of configuration bits is num_level* 2*(basis+1) */
    num_level = determine_tree_mux_level(mux_size); 
    num_input_basis = 2;
    break;
  case SPICE_MODEL_STRUCTURE_MULTILEVEL:
    /* Number of configuration bits is num_level* 2*(basis+1) */
    num_level = mux_spice_model->design_tech_info.mux_num_level; 
    num_input_basis = determine_num_input_basis_multilevel_mux(mux_size, num_level);
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
  switch (mux_spice_model->design_tech_info.structure) {
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

char* chomp_verilog_node_prefix(char* verilog_node_prefix) {
  int len = 0;
  char* ret = NULL;

  if (NULL == verilog_node_prefix) {
    return NULL;
  }

  len = strlen(verilog_node_prefix); /* String length without the last "\0"*/
  ret = (char*)my_malloc(sizeof(char)*(len+1));
  
  /* Don't do anything when input is NULL*/
  if (NULL == verilog_node_prefix) {
    my_free(ret);
    return NULL;
  }

  strcpy(ret,verilog_node_prefix);
  /* If the path end up with "_" we should remove it*/
  /*
  if ('_' == ret[len-1]) {
    ret[len-1] = ret[len];
  }
  */

  return ret;
}

char* format_verilog_node_prefix(char* verilog_node_prefix) {
  int len = strlen(verilog_node_prefix); /* String length without the last "\0"*/
  char* ret = (char*)my_malloc(sizeof(char)*(len+1));
 
  /* Don't do anything when input is NULL*/ 
  if (NULL == verilog_node_prefix) {
    my_free(ret);
    return NULL;
  }

  strcpy(ret,verilog_node_prefix);
  /* If the path does not end up with "_" we should complete it*/
  /*
  if (ret[len-1] != '_') {
    strcat(ret, "_");
  }
  */
  return ret;
}

