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
#include "spice_utils.h"
#include "spice_mux.h"

/* Include verilog utils */
#include "verilog_utils.h"
#include "verilog_global.h"
#include "verilog_pbtypes.h"

/***** Subroutines *****/
/* Dump a CMOS MUX basis module */
void dump_verilog_cmos_mux_one_basis_module(FILE* fp, 
                                            char* mux_basis_subckt_name, 
                                            int num_input_basis_subckt, 
                                            t_spice_model* cur_spice_model) { 
  int num_mem = num_input_basis_subckt;

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Special: 2-input MUX has only 1 memory bit */
  if (2 == num_input_basis_subckt) {
    num_mem = 1;
  }

  /* Comment lines */
  fprintf(fp, "//---- CMOS MUX basis module: %s -----\n", mux_basis_subckt_name);

  /* Print the port list and definition */
  fprintf(fp, "module %s (\n", mux_basis_subckt_name);
  /* Port list */
  fprintf(fp, "input [0:%d] in,\n", num_input_basis_subckt - 1);
  fprintf(fp, "output out,\n");
  fprintf(fp, "input [0:%d] mem, mem_inv);\n",
          num_mem - 1);
  /* Verilog Behavior description for a MUX */
  fprintf(fp, "//---- Behavior-level description -----\n");
  fprintf(fp, "assign out = in[mem];\n");

  /* Put an end to this module */
  fprintf(fp, "endmodule\n");

  /* Comment lines */
  fprintf(fp, "//---- END CMOS MUX basis module: %s -----\n\n", mux_basis_subckt_name);

  return;
}

/* TODO: UNIFINISHED!!!
 * Dump a RRAM MUX basis module */
void dump_verilog_rram_mux_one_basis_module(FILE* fp, 
                                            char* mux_basis_subckt_name, 
                                            int num_input_basis_subckt, 
                                            t_spice_model* cur_spice_model) { 
  /* RRAM MUX needs 2*(input_size + 1) memory bits for configuration purpose */
  int num_mem = num_input_basis_subckt + 1;
  int i;

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  /* Comment lines */
  fprintf(fp, "//---- RRAM MUX basis module: %s -----\n", mux_basis_subckt_name);

  /* Print the port list and definition */
  fprintf(fp, "module %s (\n", mux_basis_subckt_name);
  /* Port list */
  fprintf(fp, "input [0:%d] in,\n", num_input_basis_subckt - 1);
  fprintf(fp, "output out,\n");
  fprintf(fp, "input [0:%d] bl,wl);\n",
          num_mem - 1);

  /* Print the internal logics: 
   * ONLY 4T1R programming structure is supported up to now
   */
  for (i = 0; i < num_input_basis_subckt; i++) { 
    fprintf(fp, "\tif ((1 == wl[%d])&&(1 == bl[%d])) begin\n", num_mem - 1, i);
    fprintf(fp, "\t\tassign out = in[%d];\n",i);
    fprintf(fp, "\tend else ");
  }
  fprintf(fp, "begin \n");
  fprintf(fp, "\t\tassign out = in[0];\n");
  fprintf(fp, "end\n");
 
  /* Put an end to this module */
  fprintf(fp, "endmodule\n");

  /* Comment lines */
  fprintf(fp, "//---- END RRAM MUX basis module: %s -----\n\n", mux_basis_subckt_name);

  return;
}

/* Print a basis submodule */
void dump_verilog_mux_one_basis_module(FILE* fp, 
                                       char* mux_basis_subckt_name, 
                                       int num_input_basis_subckt, 
                                       t_spice_model* cur_spice_model) { 
  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Depend on the technology */
  switch (cur_spice_model->design_tech) {
  case SPICE_MODEL_DESIGN_CMOS:
    dump_verilog_cmos_mux_one_basis_module(fp, mux_basis_subckt_name,
                                           num_input_basis_subckt,
                                           cur_spice_model);
    break;
  case SPICE_MODEL_DESIGN_RRAM:
    dump_verilog_rram_mux_one_basis_module(fp, mux_basis_subckt_name,
                                           num_input_basis_subckt,
                                           cur_spice_model);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid design_technology of MUX(name: %s)\n",
               __FILE__, __LINE__, cur_spice_model->name); 
    exit(1);
  }

  return;
}

/**
 * Dump a verilog module for the basis circuit of a MUX
 */
void dump_verilog_mux_basis_module(FILE* fp, 
                                   t_spice_mux_model* spice_mux_model) {
  /** Act depends on the structure of MUX
   * 1. tree-like/one-level: we generate a basis module 
   * 2. two/multi-level: we generate a basis and a special module (if required)
   */
  int num_input_basis_subckt = 0;
  int num_input_special_basis_subckt = 0;

  char* mux_basis_subckt_name = NULL;
  char* special_basis_subckt_name = NULL;

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
 
  /* Try to find a mux in cmos technology, 
   * if we have, then build CMOS 2:1 MUX, and given cmos_mux2to1_subckt_name
   */
  /* Exception: LUT require an auto-generation of netlist can run as well*/ 
  assert((SPICE_MODEL_MUX == spice_mux_model->spice_model->type)
       ||(SPICE_MODEL_LUT == spice_mux_model->spice_model->type));

  /* Generate the spice_mux_arch */
  spice_mux_model->spice_mux_arch = (t_spice_mux_arch*)my_malloc(sizeof(t_spice_mux_arch));
  init_spice_mux_arch(spice_mux_model->spice_model, spice_mux_model->spice_mux_arch, spice_mux_model->size);

  /* Corner case: Error out  MUX_SIZE = 2, automatcially give a one-level structure */
  if ((2 == spice_mux_model->size)&&(SPICE_MODEL_STRUCTURE_ONELEVEL != spice_mux_model->spice_model->structure)) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Structure of SPICE model (%s) should be one-level because it is linked to a 2:1 MUX!\n",
               __FILE__, __LINE__, spice_mux_model->spice_model->name);
    exit(1);
  }

  /* Prepare the basis subckt name */
  mux_basis_subckt_name = (char*)my_malloc(sizeof(char)*(strlen(spice_mux_model->spice_model->name) + 5 
                                           + strlen(my_itoa(spice_mux_model->size)) + strlen(verilog_mux_basis_posfix) + 1)); 
  sprintf(mux_basis_subckt_name, "%s_size%d%s",
          spice_mux_model->spice_model->name, spice_mux_model->size, verilog_mux_basis_posfix);

  special_basis_subckt_name = (char*)my_malloc(sizeof(char)*(strlen(spice_mux_model->spice_model->name) + 5 
                                           + strlen(my_itoa(spice_mux_model->size)) + strlen(verilog_mux_special_basis_posfix) + 1)); 
  sprintf(special_basis_subckt_name, "%s_size%d%s",
          spice_mux_model->spice_model->name, spice_mux_model->size, verilog_mux_special_basis_posfix);

  /* deteremine the number of inputs of basis subckt */ 
  num_input_basis_subckt = spice_mux_model->spice_mux_arch->num_input_basis;

  /* Print the basis subckt*/
  dump_verilog_mux_one_basis_module(fp, mux_basis_subckt_name, 
                                num_input_basis_subckt, spice_mux_model->spice_model);
  /* See if we need a special basis */
  switch (spice_mux_model->spice_model->structure) {
  case SPICE_MODEL_STRUCTURE_TREE:
  case SPICE_MODEL_STRUCTURE_ONELEVEL:
    break;
  case SPICE_MODEL_STRUCTURE_MULTILEVEL:
    num_input_special_basis_subckt = find_spice_mux_arch_special_basis_size(*(spice_mux_model->spice_mux_arch));
    if (0 < num_input_special_basis_subckt) {
      dump_verilog_mux_one_basis_module(fp, special_basis_subckt_name, 
                                        num_input_special_basis_subckt, spice_mux_model->spice_model);
    } 
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid structure for spice model (%s)!\n",
               __FILE__, __LINE__, spice_mux_model->spice_model->name);
    exit(1);
  }

  /* Free */
  my_free(mux_basis_subckt_name);
  my_free(special_basis_subckt_name);

  return;
}

void dump_verilog_cmos_mux_tree_structure(FILE* fp, 
                                          char* mux_basis_subckt_name,
                                          t_spice_model spice_model,
                                          t_spice_mux_arch spice_mux_arch,
                                          int num_sram_port, t_spice_model_port** sram_port) {
  int i, j, level, nextlevel;
  int nextj, out_idx;
  int mux_basis_cnt = 0;

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  mux_basis_cnt = 0;
  for (i = 0; i < spice_mux_arch.num_level; i++) {
    level = spice_mux_arch.num_level - i;
    nextlevel = spice_mux_arch.num_level - i - 1; 
    /* Check */
    assert(nextlevel > -1);
    fprintf(fp, "wire [%d:%d] mux2_l%d_in; \n", 
            0, spice_mux_arch.num_input_per_level[nextlevel] -1, /* input0 input1 */
            level);
  }
  fprintf(fp, "wire [%d:%d] mux2_l%d_in; \n",
          0, 0, 0);

  for (i = 0; i < spice_mux_arch.num_level; i++) {
    level = spice_mux_arch.num_level - i;
    nextlevel = spice_mux_arch.num_level - i - 1; 
    /* Check */
    assert(nextlevel > -1);
    /* Print basis mux2to1 for each level*/
    for (j = 0; j < spice_mux_arch.num_input_per_level[nextlevel]; j++) {
      nextj = j + 1;
      out_idx = j/2; 
      /* Each basis mux2to1: <given_name> <input0> <input1> <output> <sram> <sram_inv> svdd sgnd <subckt_name> */
      fprintf(fp, "%s mux_basis_no%d (", mux_basis_subckt_name, mux_basis_cnt); /* given_name */
      fprintf(fp, "mux2_l%d_in[%d:%d], ", level, j, nextj); /* input0 input1 */
      fprintf(fp, "mux2_l%d_in[%d], ", nextlevel, out_idx); /* output */
      fprintf(fp, "%s[%d], %s_inv[%d]);\n", sram_port[0]->prefix, nextlevel, sram_port[0]->prefix, nextlevel); /* sram sram_inv */
      /* Update the counter */
      j = nextj;
      mux_basis_cnt++;
    } 
  }   
  /* Assert */
  assert(0 == nextlevel);
  assert(0 == out_idx);
  assert(mux_basis_cnt == spice_mux_arch.num_input - 1);

  return;
}

void dump_verilog_cmos_mux_multilevel_structure(FILE* fp, 
                                                char* mux_basis_subckt_name,
                                                t_spice_model spice_model,
                                                t_spice_mux_arch spice_mux_arch,
                                                int num_sram_port, t_spice_model_port** sram_port) {
  int i, j, k, level, nextlevel, sram_idx;
  int out_idx;
  int mux_basis_cnt = 0;
  int special_basis_cnt = 0;
  int cur_num_input_basis = 0;

  char* special_basis_subckt_name = NULL;

  special_basis_subckt_name = (char*)my_malloc(sizeof(char)*(strlen(spice_model.name) + 5 
                                           + strlen(my_itoa(spice_mux_arch.num_input)) + strlen(verilog_mux_special_basis_posfix) + 1)); 
  sprintf(special_basis_subckt_name, "%s_size%d%s",
          spice_model.name, spice_mux_arch.num_input, verilog_mux_special_basis_posfix);


  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  mux_basis_cnt = 0;
  assert((2 == spice_mux_arch.num_input_basis)||(2 < spice_mux_arch.num_input_basis));
  for (i = 0; i < spice_mux_arch.num_level; i++) {
    level = spice_mux_arch.num_level - i;
    nextlevel = spice_mux_arch.num_level - i - 1; 
    sram_idx = nextlevel * spice_mux_arch.num_input_basis;
    /* Check */
    assert(nextlevel > -1);
    fprintf(fp, "wire [%d:%d] mux2_l%d_in; \n", 
            0, spice_mux_arch.num_input_per_level[nextlevel] -1, /* input0 input1 */
            level);
  }
  fprintf(fp, "wire [%d:%d] mux2_l%d_in; \n",
          0, 0, 0);
 
  for (i = 0; i < spice_mux_arch.num_level; i++) {
    level = spice_mux_arch.num_level - i;
    nextlevel = spice_mux_arch.num_level - i - 1; 
    sram_idx = nextlevel * spice_mux_arch.num_input_basis;
    /* Check */
    assert(nextlevel > -1);
    /* Print basis muxQto1 for each level*/
    for (j = 0; j < spice_mux_arch.num_input_per_level[nextlevel]; j = j+cur_num_input_basis) {
      /* output index */
      out_idx = j/spice_mux_arch.num_input_basis; 
      /* Determine the number of input of this basis */
      cur_num_input_basis = spice_mux_arch.num_input_basis;
      if ((j + cur_num_input_basis) > spice_mux_arch.num_input_per_level[nextlevel]) {
        cur_num_input_basis = find_spice_mux_arch_special_basis_size(spice_mux_arch);
        if (0 < cur_num_input_basis) {
          /* Print the special basis */
          fprintf(fp, "%s special_basis(", special_basis_subckt_name);
          fprintf(fp, "mux2_l%d_in[%d:%d], ", level, j, j + cur_num_input_basis - 1); /* input0 input1 */
          fprintf(fp, "mux2_l%d_in[%d], ", nextlevel, out_idx); /* output */
          fprintf(fp, "%s[%d:%d], %s_inv[%d:%d] ", 
          sram_port[0]->prefix, sram_idx, sram_idx + cur_num_input_basis -1,
          sram_port[0]->prefix, sram_idx, sram_idx + cur_num_input_basis -1);
          fprintf(fp, ");\n");
          special_basis_cnt++;
        }
        continue;
      }
      /* Each basis muxQto1: <given_name> <input0> <input1> <output> <sram> <sram_inv> svdd sgnd <subckt_name> */
      fprintf(fp, "%s ", mux_basis_subckt_name); /* subckt_name */
      fprintf(fp, "mux_basis_no%d (", mux_basis_cnt); /* given_name */
      fprintf(fp, "mux2_l%d_in[%d:%d], ", level, j, j + cur_num_input_basis - 1); /* input0 input1 */
      fprintf(fp, "mux2_l%d_in[%d], ", nextlevel, out_idx); /* output */
      /* Print number of sram bits for this basis */
      for (k = sram_idx; k < (sram_idx + cur_num_input_basis); k++) {
      }
      /* sram sram_inv */
      fprintf(fp, "%s[%d:%d], %s_inv[%d:%d] ", 
      sram_port[0]->prefix, sram_idx, sram_idx + cur_num_input_basis -1,
      sram_port[0]->prefix, sram_idx, sram_idx + cur_num_input_basis -1);
      fprintf(fp, ");\n");
      /* Update the counter */
      mux_basis_cnt++;
    } 
  }   
  /* Assert */
  assert(0 == nextlevel);
  assert(0 == out_idx);
  assert((1 == special_basis_cnt)||(0 == special_basis_cnt));
  /* assert((mux_basis_cnt + special_basis_cnt) == (int)((spice_mux_arch.num_input - 1)/(spice_mux_arch.num_input_basis - 1)) + 1); */
  my_free(special_basis_subckt_name);
 
  return;
}

void dump_verilog_cmos_mux_onelevel_structure(FILE* fp, 
                                              char* mux_basis_subckt_name,
                                              t_spice_model spice_model,
                                              t_spice_mux_arch spice_mux_arch,
                                              int num_sram_port, t_spice_model_port** sram_port) {
  int i;

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  assert(SPICE_MODEL_DESIGN_CMOS == spice_model.design_tech);

  fprintf(fp, "wire [0:%d] mux2_l%d_in; \n", spice_mux_arch.num_input - 1, 1); /* input0  */
  fprintf(fp, "wire [0:%d] mux2_l%d_in; \n", 0, 0); /* output */

  fprintf(fp, "%s mux_basis (\n", mux_basis_subckt_name); /* given_name */
  fprintf(fp, "//----- MUX inputs -----\n");
  fprintf(fp, "mux2_l%d_in[0:%d], ", 1, spice_mux_arch.num_input - 1); /* input0  */
  fprintf(fp, "mux2_l%d_in[%d], ", 0, 0); /* output */
  fprintf(fp, "\n");
  fprintf(fp, "//----- SRAM ports -----\n");
  /* Special basis for 2-input MUX, there is only one configuration bit */
  if (2 == spice_mux_arch.num_input) {
    fprintf(fp, "%s[0:%d], %s_inv[0:%d] ", 
    sram_port[0]->prefix, 0, 
    sram_port[0]->prefix, 0); /* sram sram_inv */
  } else {
    fprintf(fp, "%s[0:%d], %s_inv[0:%d] ", 
    sram_port[0]->prefix, spice_mux_arch.num_input - 1, 
    sram_port[0]->prefix, spice_mux_arch.num_input - 1); /* sram sram_inv */
  }
  fprintf(fp, "\n");
  fprintf(fp, ");\n");
 
  return;
}

void dump_verilog_cmos_mux_submodule(FILE* fp,
                                     int mux_size,
                                     t_spice_model spice_model,
                                     t_spice_mux_arch spice_mux_arch) {
  int i;
  int num_input_port = 0;
  int num_output_port = 0;
  int num_sram_port = 0;
  t_spice_model_port** input_port = NULL;
  t_spice_model_port** output_port = NULL;
  t_spice_model_port** sram_port = NULL;

  /* Find the basis subckt*/
  char* mux_basis_subckt_name = NULL;

  mux_basis_subckt_name = (char*)my_malloc(sizeof(char)*(strlen(spice_model.name) + 5 
                                           + strlen(my_itoa(mux_size)) + strlen(verilog_mux_basis_posfix) + 1)); 
  sprintf(mux_basis_subckt_name, "%s_size%d%s",
          spice_model.name, mux_size, verilog_mux_basis_posfix);

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  /* Ensure we have a CMOS MUX,
   * ATTENTION: support LUT as well
   */
  assert((SPICE_MODEL_MUX == spice_model.type)||(SPICE_MODEL_LUT == spice_model.type)); 
  assert(SPICE_MODEL_DESIGN_CMOS == spice_model.design_tech);

  /* Find the input port, output port, and sram port*/
  input_port = find_spice_model_ports(&spice_model, SPICE_MODEL_PORT_INPUT, &num_input_port);
  output_port = find_spice_model_ports(&spice_model, SPICE_MODEL_PORT_OUTPUT, &num_output_port);
  sram_port = find_spice_model_ports(&spice_model, SPICE_MODEL_PORT_SRAM, &num_sram_port);

  /* Asserts*/
  assert(1 == num_input_port);
  assert(1 == num_output_port);
  assert(1 == num_sram_port);
  assert(1 == output_port[0]->size);

  if (SPICE_MODEL_LUT == spice_model.type) {
    /* Special for LUT MUX*/
    fprintf(fp, "//------ CMOS MUX info: spice_model_name= %s_MUX, size=%d -----\n", spice_model.name, mux_size);
    fprintf(fp, "module %s_mux_size%d (", spice_model.name, mux_size);
  } else {
    fprintf(fp, "//----- CMOS MUX info: spice_model_name=%s, size=%d, structure: %s -----\n", 
            spice_model.name, mux_size, gen_str_spice_model_structure(spice_model.structure));
    fprintf(fp, "module %s_size%d (", spice_model.name, mux_size);
  }
  /* Print input ports*/
  fprintf(fp, "input [0:%d] %s,\n", mux_size - 1,  input_port[0]->prefix);
  /* Print output ports*/
  fprintf(fp, "output %s,\n", output_port[0]->prefix);
  switch (spice_model.structure) {
  case SPICE_MODEL_STRUCTURE_TREE:
    /* Print sram ports*/
    fprintf(fp, "input [0:%d] %s, %s_inv\n", 
            spice_mux_arch.num_level - 1, sram_port[0]->prefix, sram_port[0]->prefix);
    break;
  case SPICE_MODEL_STRUCTURE_ONELEVEL:
    /* Special case: 2-input MUX, we force to use a one-level structure and 1 configuration bit */
    if (2 == mux_size) { 
      fprintf(fp, "input [0:%d] %s, %s_inv\n", 
              0, sram_port[0]->prefix, sram_port[0]->prefix);
    } else {
      /* Print sram ports*/
      fprintf(fp, "input [0:%d] %s, %s_inv\n", 
              mux_size - 1, sram_port[0]->prefix, sram_port[0]->prefix);
    }
    break;
  case SPICE_MODEL_STRUCTURE_MULTILEVEL:
    /* Print sram ports*/
    fprintf(fp, "input [0:%d] %s, %s_inv\n", 
            spice_mux_arch.num_level*spice_mux_arch.num_input_basis - 1,
            sram_port[0]->prefix, sram_port[0]->prefix); 
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid structure for spice model (%s)!\n",
             __FILE__, __LINE__, spice_model.name);
    exit(1);
  }
  /* Print local vdd and gnd*/
  fprintf(fp, ");");
  fprintf(fp, "\n");
  
  /* Print internal architecture*/ 
  switch (spice_model.structure) {
  case SPICE_MODEL_STRUCTURE_TREE:
    dump_verilog_cmos_mux_tree_structure(fp, mux_basis_subckt_name, spice_model, spice_mux_arch, num_sram_port, sram_port);
    break;
  case SPICE_MODEL_STRUCTURE_ONELEVEL:
    dump_verilog_cmos_mux_onelevel_structure(fp, mux_basis_subckt_name, spice_model, spice_mux_arch, num_sram_port, sram_port);
    break;
  case SPICE_MODEL_STRUCTURE_MULTILEVEL:
    dump_verilog_cmos_mux_multilevel_structure(fp, mux_basis_subckt_name, spice_model, spice_mux_arch, num_sram_port, sram_port);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid structure for spice model (%s)!\n",
               __FILE__, __LINE__, spice_model.name);
    exit(1);
  }

  /* To connect the input ports*/
  for (i = 0; i < mux_size; i++) {
    if (1 == spice_model.input_buffer->exist) {
      switch (spice_model.input_buffer->type) {
      case SPICE_MODEL_BUF_INV:
        /* Each inv: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
        fprintf(fp, "not inv%d (", i); /* Given name*/
        fprintf(fp, "%s[%d], ", input_port[0]->prefix, i); /* input port */ 
        fprintf(fp, "mux2_l%d_in[%d]); ", spice_mux_arch.input_level[i], spice_mux_arch.input_offset[i]); /* output port*/
        fprintf(fp, "\n");
        break;
      case SPICE_MODEL_BUF_BUF:
        /* TODO: what about tapered buffer, can we support? */
        /* Each buf: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
        fprintf(fp, "buf buf%d (", i); /* Given name*/
        fprintf(fp, "%s[%d], ", input_port[0]->prefix, i); /* input port */ 
        fprintf(fp, "mux2_l%d_in[%d]); ", spice_mux_arch.input_level[i], spice_mux_arch.input_offset[i]); /* output port*/
        fprintf(fp, "\n");
        break;
      default:
        vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type for spice_model_buffer.\n",
                   __FILE__, __LINE__);
        exit(1);   
      }
    } else {
      /* There is no buffer, I create a zero resisitance between*/
      /* Resistance R<given_name> <input> <output> 0*/
      fprintf(fp, "assign %s[%d] = mux2_l%d_in[%d];\n", 
              input_port[0]->prefix, i, spice_mux_arch.input_level[i], 
              spice_mux_arch.input_offset[i]);
    }
  }

  /* Output buffer*/
  if (1 == spice_model.output_buffer->exist) {
    switch (spice_model.output_buffer->type) {
    case SPICE_MODEL_BUF_INV:
      if (TRUE == spice_model.output_buffer->tapered_buf) {
        break;
      }
      /* Each inv: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
      fprintf(fp, "not inv_out ("); /* Given name*/
      fprintf(fp, "mux2_l%d_in%d, ", 0, 0); /* input port */ 
      fprintf(fp, "%s );", output_port[0]->prefix); /* Output port*/
      fprintf(fp, "\n");
      break;
    case SPICE_MODEL_BUF_BUF:
      if (TRUE == spice_model.output_buffer->tapered_buf) {
        break;
      }
      /* Each buf: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
      fprintf(fp, "buf buf_out ("); /* Given name*/
      fprintf(fp, "mux2_l%d_in%d, ", 0, 0); /* input port */ 
      fprintf(fp, "%s );", output_port[0]->prefix); /* Output port*/
      fprintf(fp, "\n");
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type for spice_model_buffer.\n",
                 __FILE__, __LINE__);
      exit(1);   
    }
    /* Tapered buffer support */
    if (TRUE == spice_model.output_buffer->tapered_buf) {
      /* Each buf: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
      fprintf(fp, "tapbuf_level%d_f%d buf_out (",
              spice_model.output_buffer->tap_buf_level, spice_model.output_buffer->f_per_stage); /* subckt name */
      fprintf(fp, "mux2_l%d_in%d, ", 0, 0); /* input port */ 
      fprintf(fp, "%s );", output_port[0]->prefix); /* Output port*/
      fprintf(fp, "\n");
    }
  } else {
    /* There is no buffer, I create a zero resisitance between*/
    /* Resistance R<given_name> <input> <output> 0*/
    fprintf(fp, "assign mux2_l0_in[0] = %s;\n",output_port[0]->prefix);
  }
 
  fprintf(fp, "endmodule\n");
  fprintf(fp, "//----- END CMOS MUX info: spice_model_name=%s, size=%d -----\n\n", spice_model.name, mux_size);
  fprintf(fp, "\n");

  /* Free */
  my_free(mux_basis_subckt_name);
  my_free(input_port);
  my_free(output_port);
  my_free(sram_port);

  return;
}

/* Print the RRAM MUX SPICE model.
 * The internal structures of CMOS and RRAM MUXes are similar. 
 * This one can be merged to CMOS function.
 * However I use another function, because in future the internal structure may change.
 * We will suffer less software problems.
 */
void dump_verilog_rram_mux_tree_structure(FILE* fp, 
                                          char* mux_basis_subckt_name,
                                          t_spice_model spice_model,
                                          t_spice_mux_arch spice_mux_arch,
                                          int num_sram_port, t_spice_model_port** sram_port) {
  int i, j, level, nextlevel;
  int nextj, out_idx;
  int mux_basis_cnt = 0;
  int cur_mem_lsb = 0;
  int cur_mem_msb = 0;

  assert(SPICE_MODEL_DESIGN_RRAM == spice_model.design_tech);

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  for (i = 0; i < spice_mux_arch.num_level; i++) {
    level = spice_mux_arch.num_level - i;
    nextlevel = spice_mux_arch.num_level - i - 1; 
    /* Check */
    assert(nextlevel > -1);
    fprintf(fp, "wire [%d:%d] mux2_l%d_in; \n", 
            0, spice_mux_arch.num_input_per_level[nextlevel] -1, /* input0 input1 */
            level);
  }
  fprintf(fp, "wire [%d:%d] mux2_l%d_in; \n",
          0, 0, 0);

  mux_basis_cnt = 0;
  for (i = 0; i < spice_mux_arch.num_level; i++) {
    level = spice_mux_arch.num_level - i;
    nextlevel = spice_mux_arch.num_level - i - 1; 
    /* Check */
    assert(nextlevel > -1);
    /* Print basis mux2to1 for each level*/
    for (j = 0; j < spice_mux_arch.num_input_per_level[nextlevel]; j++) {
      nextj = j + 1;
      out_idx = j/2; 
      cur_mem_lsb = cur_mem_msb;
      cur_mem_msb += 6; 
      /* Each basis mux2to1: <given_name> <input0> <input1> <output> <sram> <sram_inv> svdd sgnd <subckt_name> */
      fprintf(fp, "%s mux_basis_no%d (", mux_basis_subckt_name, mux_basis_cnt); /* given_name */
      fprintf(fp, "mux2_l%d_in[%d], mux2_l%d_in[%d], ", level, j, level, nextj); /* input0 input1 */
      fprintf(fp, "mux2_l%d_in[%d], ", nextlevel, out_idx); /* output */
      fprintf(fp, "%s[%d:%d] %s_inv[%d:%d]);\n", 
              sram_port[0]->prefix, cur_mem_lsb, cur_mem_msb - 1,
              sram_port[0]->prefix, cur_mem_lsb, cur_mem_msb - 1); /* sram sram_inv */
      /* Update the counter */
      j = nextj;
      mux_basis_cnt++;
    } 
  }   
  /* Assert */
  assert(0 == nextlevel);
  assert(0 == out_idx);
  assert(mux_basis_cnt == spice_mux_arch.num_input - 1);
  assert(cur_mem_msb == 6 * spice_mux_arch.num_level); 

  return;
}

void dump_verilog_rram_mux_multilevel_structure(FILE* fp, 
                                                char* mux_basis_subckt_name,
                                                t_spice_model spice_model,
                                                t_spice_mux_arch spice_mux_arch,
                                                int num_sram_port, t_spice_model_port** sram_port) {
  int i, j, k, level, nextlevel, sram_idx;
  int out_idx;
  int mux_basis_cnt = 0;
  int special_basis_cnt = 0;
  int cur_num_input_basis = 0;

  int cur_mem_lsb = 0;
  int cur_mem_msb = 0;

  char* special_basis_subckt_name = NULL;

  assert(SPICE_MODEL_DESIGN_RRAM == spice_model.design_tech);

  special_basis_subckt_name = (char*)my_malloc(sizeof(char)*(strlen(spice_model.name) + 5 
                                           + strlen(my_itoa(spice_mux_arch.num_input)) + strlen(verilog_mux_special_basis_posfix) + 1)); 
  sprintf(special_basis_subckt_name, "%s_size%d%s",
          spice_model.name, spice_mux_arch.num_input, verilog_mux_special_basis_posfix);
  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  for (i = 0; i < spice_mux_arch.num_level; i++) {
    level = spice_mux_arch.num_level - i;
    nextlevel = spice_mux_arch.num_level - i - 1; 
    sram_idx = nextlevel * spice_mux_arch.num_input_basis;
    /* Check */
    assert(nextlevel > -1);
    fprintf(fp, "wire [%d:%d] mux2_l%d_in; \n", 
            0, spice_mux_arch.num_input_per_level[nextlevel] -1, /* input0 input1 */
            level);
  }
  fprintf(fp, "wire [%d:%d] mux2_l%d_in; \n",
          0, 0, 0);

  mux_basis_cnt = 0;
  assert((2 == spice_mux_arch.num_input_basis)||(2 < spice_mux_arch.num_input_basis));
  for (i = 0; i < spice_mux_arch.num_level; i++) {
    level = spice_mux_arch.num_level - i;
    nextlevel = spice_mux_arch.num_level - i - 1; 
    /* Check */
    assert(nextlevel > -1);
    /* Memory port offset update */
    cur_mem_lsb = cur_mem_msb;
    /* Print basis muxQto1 for each level*/
    for (j = 0; j < spice_mux_arch.num_input_per_level[nextlevel]; j = j+cur_num_input_basis) {
      /* output index */
      out_idx = j/spice_mux_arch.num_input_basis; 
      /* Determine the number of input of this basis */
      cur_num_input_basis = spice_mux_arch.num_input_basis;
      if ((j + cur_num_input_basis) > spice_mux_arch.num_input_per_level[nextlevel]) {
        cur_num_input_basis = find_spice_mux_arch_special_basis_size(spice_mux_arch);
        if (0 < cur_num_input_basis) {
          /* Print the special basis */
          fprintf(fp, "%s special_basis(\n", special_basis_subckt_name);
          for (k = 0; k < cur_num_input_basis; k++) {
            fprintf(fp, "mux2_l%d_in[%d], ", level, j + k);
            fprintf(fp, "mux2_l%d_in[%d], ", nextlevel, out_idx); /* output */
          }
          cur_mem_msb += 2 * (cur_num_input_basis + 1);
          for (k = cur_mem_lsb; k < cur_mem_msb; k++) {
            fprintf(fp, "%s[%d], %s_inv[%d], ", 
                    sram_port[0]->prefix, k, 
                    sram_port[0]->prefix, k); /* sram sram_inv */
          }
          fprintf(fp, ");\n");
          special_basis_cnt++;
          continue;
        }
      }
      /* Each basis muxQto1: <given_name> <input0> <input1> <output> <sram> <sram_inv> svdd sgnd <subckt_name> */
      fprintf(fp, "%s ", mux_basis_subckt_name); /* subckt_name */
      fprintf(fp, "mux_basis_no%d (", mux_basis_cnt); /* given_name */
      for (k = 0; k < cur_num_input_basis; k++) {
        fprintf(fp, "mux2_l%d_in[%d], ", level, j + k); /* input0 input1 */
      }
      fprintf(fp, "mux2_l%d_in[%d], ", nextlevel, out_idx); /* output */
      /* Print number of sram bits for this basis */
      cur_mem_msb += 2 * (cur_num_input_basis + 1);
      for (k = cur_mem_lsb; k < cur_mem_msb; k++) {
        fprintf(fp, "%s[%d], %s_inv[%d], ", 
                sram_port[0]->prefix, k, 
                sram_port[0]->prefix, k); /* sram sram_inv */
      }
      fprintf(fp, ");\n");
      /* Update the counter */
      mux_basis_cnt++;
    } 
  }   
  /* Assert */
  assert(0 == nextlevel);
  assert(0 == out_idx);
  assert(1 == special_basis_cnt);
  assert((mux_basis_cnt + special_basis_cnt) == (int)((spice_mux_arch.num_input - 1)/(spice_mux_arch.num_input_basis - 1)) + 1);
 
  /* Free */
  my_free(special_basis_subckt_name);

  return;
}

void dump_verilog_rram_mux_onelevel_structure(FILE* fp, 
                                              char* mux_basis_subckt_name,
                                              t_spice_model spice_model,
                                              t_spice_mux_arch spice_mux_arch,
                                              int num_sram_port, t_spice_model_port** sram_port) {
  int i;

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  assert(SPICE_MODEL_DESIGN_RRAM == spice_model.design_tech);

  fprintf(fp, "wire [0:%d] mux2_l%d_in; \n", spice_mux_arch.num_input - 1, 1); /* input0  */
  fprintf(fp, "wire [0:%d] mux2_l%d_in; \n", 0, 0); /* output */

  fprintf(fp, "%s mux_basis (\n", mux_basis_subckt_name); /* given_name */
  fprintf(fp, "//----- MUX inputs -----\n");
  for (i = 0; i < spice_mux_arch.num_input; i++) {
    fprintf(fp, "mux2_l%d_in[%d], ", 1, i); /* input0  */
    fprintf(fp, "mux2_l%d_in[%d], ", 0, 0); /* output */
  }
  fprintf(fp, "\n");
  fprintf(fp, "//----- SRAM ports -----\n");
  for (i = 0; i < 2*(spice_mux_arch.num_input + 1); i++) {
    fprintf(fp, "%s[%d], %s_inv[%d], ", 
            sram_port[0]->prefix, i, 
            sram_port[0]->prefix, i); /* sram sram_inv */
  }
  fprintf(fp, "\n");
  fprintf(fp, ");\n");
 
  return;
}

void dump_verilog_rram_mux_submodule(FILE* fp,
                                     int mux_size,
                                     t_spice_model spice_model,
                                     t_spice_mux_arch spice_mux_arch) {
  int i;
  int num_input_port = 0;
  int num_output_port = 0;
  int num_sram_port = 0;
  t_spice_model_port** input_port = NULL;
  t_spice_model_port** output_port = NULL;
  t_spice_model_port** sram_port = NULL;

  /* Find the basis subckt*/
  char* mux_basis_subckt_name = NULL;
  mux_basis_subckt_name = (char*)my_malloc(sizeof(char)*(strlen(spice_model.name) + 5 
                                           + strlen(my_itoa(mux_size)) + strlen(verilog_mux_basis_posfix) + 1)); 
  sprintf(mux_basis_subckt_name, "%s_size%d%s",
          spice_model.name, mux_size, verilog_mux_basis_posfix);

  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  /* Ensure we have a RRAM MUX*/
  assert((SPICE_MODEL_MUX == spice_model.type)||(SPICE_MODEL_LUT == spice_model.type)); 
  assert(SPICE_MODEL_DESIGN_RRAM == spice_model.design_tech);

  /* Find the input port, output port, and sram port*/
  input_port = find_spice_model_ports(&spice_model, SPICE_MODEL_PORT_INPUT, &num_input_port);
  output_port = find_spice_model_ports(&spice_model, SPICE_MODEL_PORT_OUTPUT, &num_output_port);
  sram_port = find_spice_model_ports(&spice_model, SPICE_MODEL_PORT_SRAM, &num_sram_port);

  /* Asserts*/
  assert(1 == num_input_port);
  assert(1 == num_output_port);
  assert(1 == num_sram_port);
  assert(1 == output_port[0]->size);

  /* Print the definition of subckt*/
  if (SPICE_MODEL_LUT == spice_model.type) {
    /* RRAM LUT is not supported now... */
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])RRAM LUT is not supported!\n",
               __FILE__, __LINE__);
    exit(1);
    /* Special for LUT MUX*/
    /*
    fprintf(fp, "***** RRAM MUX info: spice_model_name= %s_MUX, size=%d *****\n", spice_model.name, mux_size);
    fprintf(fp, ".subckt %s_mux_size%d ", spice_model.name, mux_size);
    */
  } else {
    fprintf(fp, "//----- RRAM MUX info: spice_model_name=%s, size=%d, structure: %s -----\n", 
            spice_model.name, mux_size, gen_str_spice_model_structure(spice_model.structure));
    fprintf(fp, "module %s_size%d( ", spice_model.name, mux_size);
  }
  /* Print input ports*/
  fprintf(fp, "input [0:%d] %s;\n ", mux_size - 1, input_port[0]->prefix);
  /* Print output ports*/
  fprintf(fp, "output %s;\n ", output_port[0]->prefix);
  switch (spice_model.structure) {
  case SPICE_MODEL_STRUCTURE_TREE:
    /* Print sram ports*/
    fprintf(fp, "input [0:%d] %s, %s_inv;\n", 
            spice_mux_arch.num_level - 1, sram_port[0]->prefix, sram_port[0]->prefix);
    break;
  case SPICE_MODEL_STRUCTURE_ONELEVEL:
    /* Print sram ports*/
    fprintf(fp, "input [0:%d] %s, %s_inv;\n", 
            mux_size - 1, sram_port[0]->prefix, sram_port[0]->prefix);
    break;
  case SPICE_MODEL_STRUCTURE_MULTILEVEL:
    /* Print sram ports*/
    fprintf(fp, "input [0:%d] %s, %s_inv;\n", 
            spice_mux_arch.num_level*spice_mux_arch.num_input_basis - 1,
            sram_port[0]->prefix, sram_port[0]->prefix);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid structure for spice model (%s)!\n",
               __FILE__, __LINE__, spice_model.name);
    exit(1);
  }
  /* Print local vdd and gnd*/
  fprintf(fp, ");\n");
  
  /* Print internal architecture*/ 
  /* RRAM MUX is optimal in terms of area, delay and power for one-level structure.
   */
  switch (spice_model.structure) {
  case SPICE_MODEL_STRUCTURE_TREE:
    dump_verilog_rram_mux_tree_structure(fp, mux_basis_subckt_name, spice_model, spice_mux_arch, num_sram_port, sram_port);
    break;
  case SPICE_MODEL_STRUCTURE_MULTILEVEL:
    dump_verilog_rram_mux_multilevel_structure(fp, mux_basis_subckt_name, spice_model, spice_mux_arch, num_sram_port, sram_port);
    break;
  case SPICE_MODEL_STRUCTURE_ONELEVEL:
    dump_verilog_rram_mux_onelevel_structure(fp, mux_basis_subckt_name, spice_model, spice_mux_arch, num_sram_port, sram_port);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid structure for spice model (%s)!\n",
               __FILE__, __LINE__, spice_model.name);
    exit(1);
  }

  /* To connect the input ports*/
  for (i = 0; i < mux_size; i++) {
    if (1 == spice_model.input_buffer->exist) {
      switch (spice_model.input_buffer->type) {
      case SPICE_MODEL_BUF_INV:
        /* Each inv: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
        fprintf(fp, "not inv%d (", i); /* Given name*/
        fprintf(fp, "%s[%d], ", input_port[0]->prefix, i); /* input port */ 
        fprintf(fp, "mux2_l%d_in[%d]);", spice_mux_arch.input_level[i], spice_mux_arch.input_offset[i]); /* output port*/
        fprintf(fp, "\n");
        break;
      case SPICE_MODEL_BUF_BUF:
        /* TODO: what about tapered buffer, can we support? */
        /* Each buf: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
        fprintf(fp, "buf buf%d (", i); /* Given name*/
        fprintf(fp, "%s[%d], ", input_port[0]->prefix, i); /* input port */ 
        fprintf(fp, "mux2_l%d_in[%d)];", spice_mux_arch.input_level[i], spice_mux_arch.input_offset[i]); /* output port*/
        fprintf(fp, "\n");
        break;
      default:
        vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type for spice_model_buffer.\n",
                   __FILE__, __LINE__);
        exit(1);   
      }
    } else {
      /* There is no buffer, I create a zero resisitance between*/
      /* Resistance R<given_name> <input> <output> 0*/
      fprintf(fp, "assign %s[%d] = mux2_l%d_in[%d];\n", 
              input_port[0]->prefix, i, spice_mux_arch.input_level[i], 
              spice_mux_arch.input_offset[i]);
    }
  }

  /* Output buffer*/
  if (1 == spice_model.output_buffer->exist) {
    switch (spice_model.output_buffer->type) {
    case SPICE_MODEL_BUF_INV:
      if (TRUE == spice_model.output_buffer->tapered_buf) {
        break;
      }
      /* Each inv: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
      fprintf(fp, "not inv_out ("); /* Given name*/
      fprintf(fp, "mux2_l%d_in[%d], ", 0, 0); /* input port */ 
      fprintf(fp, "%s );", output_port[0]->prefix); /* Output port*/
      fprintf(fp, "\n");
      break;
    case SPICE_MODEL_BUF_BUF:
      if (TRUE == spice_model.output_buffer->tapered_buf) {
        break;
      }
      /* Each buf: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
      fprintf(fp, "buf buf_out ("); /* Given name*/
      fprintf(fp, "mux2_l%d_in[%d], ", 0, 0); /* input port */ 
      fprintf(fp, "%s );", output_port[0]->prefix); /* Output port*/
      fprintf(fp, "\n");
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid type for spice_model_buffer.\n",
                 __FILE__, __LINE__);
      exit(1);   
    }
    /* Tapered buffer support */
    if (TRUE == spice_model.output_buffer->tapered_buf) {
      /* Each buf: <given_name> <input0> <output> svdd sgnd <subckt_name> size=param*/
      fprintf(fp, "tapbuf_level%d_f%d buf_out (",
              spice_model.output_buffer->tap_buf_level, spice_model.output_buffer->f_per_stage); /* subckt name */
      fprintf(fp, "mux2_l%d_in[%d], ", 0 , 0); /* input port */ 
      fprintf(fp, "%s );", output_port[0]->prefix); /* Output port*/
      fprintf(fp, "\n");
    }
  } else {
    /* There is no buffer, I create a zero resisitance between*/
    /* Resistance R<given_name> <input> <output> 0*/
    fprintf(fp, "assign mux2_l0_in[0] %s;\n",output_port[0]->prefix);
  }
 
  fprintf(fp, "endmodule\n");
  fprintf(fp, "//------ END RRAM MUX info: spice_model_name=%s, size=%d -----\n\n", spice_model.name, mux_size);
  fprintf(fp, "\n");

  /* Free */
  my_free(mux_basis_subckt_name);
  my_free(input_port);
  my_free(output_port);
  my_free(sram_port);

  return;
}

/** Dump a verilog module for a MUX
 * We always dump a basis submodule for a MUX
 * whatever structure it is: one-level, two-level or multi-level
 */
void dump_verilog_mux_module(FILE* fp, 
                             t_spice_mux_model* spice_mux_model) {
  /* Make sure we have a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid file handler!\n",__FILE__, __LINE__); 
    exit(1);
  }  
  /* Make sure we have a valid spice_model*/
  if (NULL == spice_mux_model) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid spice_mux_model!\n",__FILE__, __LINE__); 
    exit(1);
  }
  /* Make sure we have a valid spice_model*/
  if (NULL == spice_mux_model->spice_model) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid spice_model!\n",__FILE__, __LINE__); 
    exit(1);
  }

  /* Check the mux size*/
  if (spice_mux_model->size < 2) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid MUX size(=%d)! Should be at least 2.\n",
               __FILE__, __LINE__, spice_mux_model->size); 
    exit(1);
  }

  /* Corner case: Error out  MUX_SIZE = 2, automatcially give a one-level structure */
  if ((2 == spice_mux_model->size)&&(SPICE_MODEL_STRUCTURE_ONELEVEL != spice_mux_model->spice_model->structure)) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Structure of SPICE model (%s) should be one-level because it is linked to a 2:1 MUX!\n",
               __FILE__, __LINE__, spice_mux_model->spice_model->name);
    exit(1);
  }

  /* Print the definition of subckt*/
  /* Check the design technology*/
  switch (spice_mux_model->spice_model->design_tech) {
  case SPICE_MODEL_DESIGN_CMOS:
    dump_verilog_cmos_mux_submodule(fp, spice_mux_model->size,
                                    *(spice_mux_model->spice_model), 
                                    *(spice_mux_model->spice_mux_arch));
    break;
  case SPICE_MODEL_DESIGN_RRAM:
    dump_verilog_rram_mux_submodule(fp, spice_mux_model->size,
                                    *(spice_mux_model->spice_model), 
                                    *(spice_mux_model->spice_mux_arch));
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid design_technology of MUX(name: %s)\n",
               __FILE__, __LINE__, spice_mux_model->spice_model->name); 
    exit(1);
  }

  return;
}


/*** Top-level function *****/

/* We should count how many multiplexers with different sizes are needed */

/**/
void dump_verilog_submodule_muxes(char* submodule_dir,
                                  int num_switch,
                                  t_switch_inf* switches,
                                  t_spice* spice,
                                  t_det_routing_arch* routing_arch) {
  
  /* Statisitcs for input sizes and structures of MUXes 
   * used in FPGA architecture 
   */
  /* We have linked list whichs stores spice model information of multiplexer*/
  t_llist* muxes_head = NULL; 
  t_llist* temp = NULL;
  int mux_cnt = 0;
  int max_mux_size = -1;
  int min_mux_size = -1;
  FILE* fp = NULL;
  char* verilog_name = my_strcat(submodule_dir,muxes_verilog_file_name);
  int num_input_ports = 0;
  t_spice_model_port** input_ports = NULL;
  int num_sram_ports = 0;
  t_spice_model_port** sram_ports = NULL;

  int num_input_basis = 0;
  t_spice_mux_model* cur_spice_mux_model = NULL;

  /* Alloc the muxes*/
  muxes_head = stats_spice_muxes(num_switch, switches, spice, routing_arch);

  /* Print the muxes netlist*/
  fp = fopen(verilog_name, "w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create subckt SPICE netlist %s",__FILE__, __LINE__, verilog_name); 
    exit(1);
  } 
  /* Generate the descriptions*/
  dump_verilog_file_header(fp,"MUXes used in FPGA");

  /* Print mux netlist one by one*/
  temp = muxes_head;
  while(temp) {
    assert(NULL != temp->dptr);
    cur_spice_mux_model = (t_spice_mux_model*)(temp->dptr);
    /* Bypass the spice models who has a user-defined subckt */
    if (NULL != cur_spice_mux_model->spice_model->model_netlist) {
      input_ports = find_spice_model_ports(cur_spice_mux_model->spice_model, SPICE_MODEL_PORT_INPUT, &num_input_ports);
      sram_ports = find_spice_model_ports(cur_spice_mux_model->spice_model, SPICE_MODEL_PORT_SRAM, &num_sram_ports);
      assert(0 != num_input_ports);
      assert(0 != num_sram_ports);
      /* Check the Input port size */
      if (cur_spice_mux_model->size != input_ports[0]->size) {
        vpr_printf(TIO_MESSAGE_ERROR, 
                   "(File:%s,[LINE%d])User-defined MUX SPICE MODEL(%s) size(%d) unmatch with the architecture needs(%d)!\n",
                   __FILE__, __LINE__, cur_spice_mux_model->spice_model->name, input_ports[0]->size,cur_spice_mux_model->size);
        exit(1);
      }
      /* Check the SRAM port size */
      num_input_basis = determine_num_input_basis_multilevel_mux(cur_spice_mux_model->size, 
                                                                 cur_spice_mux_model->spice_model->mux_num_level);
      if ((num_input_basis * cur_spice_mux_model->spice_model->mux_num_level) != sram_ports[0]->size) {
        vpr_printf(TIO_MESSAGE_ERROR, 
                   "(File:%s,[LINE%d])User-defined MUX SPICE MODEL(%s) SRAM size(%d) unmatch with the num of level(%d)!\n",
                   __FILE__, __LINE__, cur_spice_mux_model->spice_model->name, sram_ports[0]->size, cur_spice_mux_model->spice_model->mux_num_level*num_input_basis);
        exit(1);
      }
      /* Move on to the next*/
      temp = temp->next;
      continue;
    }
    /* Let's have a N:1 MUX as basis*/
    dump_verilog_mux_basis_module(fp, cur_spice_mux_model);
    /* Print the mux subckt */
    dump_verilog_mux_module(fp, cur_spice_mux_model);
    /* Update the statistics*/
    mux_cnt++;
    if ((-1 == max_mux_size)||(max_mux_size < cur_spice_mux_model->size)) {
      max_mux_size = cur_spice_mux_model->size;
    }
    if ((-1 == min_mux_size)||(min_mux_size > cur_spice_mux_model->size)) {
      min_mux_size = cur_spice_mux_model->size;
    }
    /* Move on to the next*/
    temp = temp->next;
  }

  vpr_printf(TIO_MESSAGE_INFO,"Generated %d Multiplexer submodules.\n",
             mux_cnt);
  vpr_printf(TIO_MESSAGE_INFO,"Max. MUX size = %d.\t",
             max_mux_size);
  vpr_printf(TIO_MESSAGE_INFO,"Min. MUX size = %d.\n",
             min_mux_size);
  

  /* remember to free the linked list*/
  free_muxes_llist(muxes_head);
  /* Free strings */
  free(verilog_name);

  /* Close the file*/
  fclose(fp);

  return;
}

void dump_verilog_wire_module(FILE* fp,
                              char* wire_subckt_name,
                              t_spice_model verilog_model) {
  int num_input_port = 0;
  int num_output_port = 0;
  t_spice_model_port** input_port = NULL;
  t_spice_model_port** output_port = NULL;
 
  /* Ensure a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File handler.\n",
               __FILE__, __LINE__); 
    exit(1);
  } 
  /* Check the wire model*/
  assert(NULL != verilog_model.wire_param);
  assert(0 < verilog_model.wire_param->level);
  /* Find the input port, output port*/
  input_port = find_spice_model_ports(&verilog_model, SPICE_MODEL_PORT_INPUT, &num_input_port);
  output_port = find_spice_model_ports(&verilog_model, SPICE_MODEL_PORT_OUTPUT, &num_output_port);

  /* Asserts*/
  assert(1 == num_input_port);
  assert(1 == num_output_port);
  assert(1 == input_port[0]->size);
  assert(1 == output_port[0]->size);
  /* print the spice model*/
  fprintf(fp, "//-----Wire module, verilog_model_name=%s -----\n", verilog_model.name);  
  switch (verilog_model.type) {
  case SPICE_MODEL_CHAN_WIRE: 
    /* Add an output at middle point for connecting CB inputs */
    fprintf(fp, "module %s (input wire %s, output wire %s, output wire mid_out);\n",
            wire_subckt_name, input_port[0]->prefix, output_port[0]->prefix);
    fprintf(fp, "\tassign %s = %s;\n", output_port[0]->prefix, input_port[0]->prefix);
    fprintf(fp, "\tassign mid_out = %s;\n", input_port[0]->prefix);
    break;
  case SPICE_MODEL_WIRE: 
    /* Add an output at middle point for connecting CB inputs */
    fprintf(fp, "module %s (input wire %s, output wire %s);\n",
            wire_subckt_name, input_port[0]->prefix, output_port[0]->prefix);
    /* Direct shortcut */
    fprintf(fp, "\t\tassign %s = %s;\n", output_port[0]->prefix, input_port[0]->prefix);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid type of spice_model! Expect [chan_wire|wire].\n",
               __FILE__, __LINE__);
    exit(1);
  }
  
  /* Finish*/ 
  fprintf(fp, "endmodule\n");
  fprintf(fp, "//-----END Wire module, verilog_model_name=%s -----\n", verilog_model.name);  
  fprintf(fp, "\n");

  return;
}

/* Dump a submodule which is a constant vdd */
void dump_verilog_hard_wired_vdd(FILE* fp, 
                                 t_spice_model verilog_model) {
  int num_output_port = 0;
  t_spice_model_port** output_port = NULL;

  /* Find the input port, output port*/
  output_port = find_spice_model_ports(&verilog_model, SPICE_MODEL_PORT_OUTPUT, &num_output_port);

  /* Asserts*/
  assert(1 == num_output_port);
  assert(1 == output_port[0]->size);

  /* Ensure a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File handler.\n",
               __FILE__, __LINE__); 
    exit(1);
  }

  /* print the spice model*/
  fprintf(fp, "//-----Hard-wired VDD module, verilog_model_name=%s -----\n", verilog_model.name);  
  fprintf(fp, "module %s(output wire %s);\n", verilog_model.name, output_port[0]->prefix);
  /* Constant logic 1*/
  fprintf(fp, "assign %s = 1\'b1;\n", output_port[0]->prefix);
  /* Finish*/ 
  fprintf(fp, "endmodule\n");
  fprintf(fp, "//-----END VDD module, verilog_model_name=%s -----\n", verilog_model.name);  
  fprintf(fp, "\n");
  return;
}

/* Dump a submodule which is a constant vdd */
void dump_verilog_hard_wired_gnd(FILE* fp, 
                                 t_spice_model verilog_model) {
  int num_output_port = 0;
  t_spice_model_port** output_port = NULL;

  /* Find the input port, output port*/
  output_port = find_spice_model_ports(&verilog_model, SPICE_MODEL_PORT_OUTPUT, &num_output_port);

  /* Asserts*/
  assert(1 == num_output_port);
  assert(1 == output_port[0]->size);

  /* Ensure a valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File handler.\n",
               __FILE__, __LINE__); 
    exit(1);
  }

  /* print the spice model*/
  fprintf(fp, "//-----Hard-wired GND module, verilog_model_name=%s -----\n", verilog_model.name);  
  fprintf(fp, "module %s(output wire %s);\n", verilog_model.name, output_port[0]->prefix);
  /* Constant logic 1*/
  fprintf(fp, "assign %s = 1\'b0;\n", output_port[0]->prefix);
  /* Finish*/ 
  fprintf(fp, "endmodule\n");
  fprintf(fp, "//-----END GND module, verilog_model_name=%s -----\n", verilog_model.name);  
  fprintf(fp, "\n");
  return;
}

void dump_verilog_submodule_wires(char* subckt_dir,
                                  int num_segments,
                                  t_segment_inf* segments,
                                  int num_spice_model,
                                  t_spice_model* spice_models) {
  FILE* fp = NULL;
  char* verilog_name = my_strcat(subckt_dir, wires_verilog_file_name);
  char* seg_wire_subckt_name = NULL;
  char* seg_index_str = NULL;
  int iseg, imodel, len_seg_subckt_name;
 
  fp = fopen(verilog_name, "w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create Verilog netlist %s",__FILE__, __LINE__, wires_verilog_file_name); 
    exit(1);
  } 
  dump_verilog_file_header(fp,"Wires");
  /* Output wire models*/
  for (imodel = 0; imodel < num_spice_model; imodel++) {
    if (SPICE_MODEL_WIRE == spice_models[imodel].type) {
      assert(NULL != spice_models[imodel].wire_param);
      dump_verilog_wire_module(fp, spice_models[imodel].name,
                              spice_models[imodel]);
    }
  }
 
  /* Create wire models for routing segments*/
  fprintf(fp,"//----- Wire models for segments in routing -----\n");
  for (iseg = 0; iseg < num_segments; iseg++) {
    assert(NULL != segments[iseg].spice_model);
    assert(SPICE_MODEL_CHAN_WIRE == segments[iseg].spice_model->type);
    assert(NULL != segments[iseg].spice_model->wire_param);
    /* Give a unique name for subckt of wire_model of segment, 
     * spice_model name is unique, and segment name is unique as well
     */
    seg_index_str = my_itoa(iseg);
    len_seg_subckt_name = strlen(segments[iseg].spice_model->name)
                        + 4 + strlen(seg_index_str) + 1; /* '\0'*/
    seg_wire_subckt_name = (char*)my_malloc(sizeof(char)*len_seg_subckt_name);
    sprintf(seg_wire_subckt_name,"%s_seg%s",
            segments[iseg].spice_model->name, seg_index_str);
    dump_verilog_wire_module(fp, seg_wire_subckt_name,
                            *(segments[iseg].spice_model));
  }

  /* Create module for hard-wired VDD and GND */
  for (imodel = 0; imodel < num_spice_model; imodel++) {
    if (SPICE_MODEL_VDD == spice_models[imodel].type) {
      dump_verilog_hard_wired_vdd(fp, spice_models[imodel]);
    } else if (SPICE_MODEL_GND == spice_models[imodel].type) {
      dump_verilog_hard_wired_gnd(fp, spice_models[imodel]);
    }
  }
  
  /* Close the file handler */
  fclose(fp);

  /*Free*/
  my_free(seg_index_str);
  my_free(seg_wire_subckt_name);

  return;
}


/* Dump verilog files of submodules to be used in FPGA components :
 * 1. MUXes
 */
void dump_verilog_submodules(char* submodule_dir, 
                             t_arch Arch, 
                             t_det_routing_arch* routing_arch) {

  /* 1. MUXes */
  vpr_printf(TIO_MESSAGE_INFO, "Generating modules of multiplexers...\n");
  dump_verilog_submodule_muxes(submodule_dir, routing_arch->num_switch, 
                               switch_inf, Arch.spice, routing_arch);

  /* 2. Hardwires */
  vpr_printf(TIO_MESSAGE_INFO, "Generating modules of hardwires...\n");
  dump_verilog_submodule_wires(submodule_dir, Arch.num_segments, Arch.Segments,
                               Arch.spice->num_spice_model, Arch.spice->spice_models);

  return;
}

