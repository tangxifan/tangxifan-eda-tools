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
#include "fpga_spice_globals.h"

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
  int i; 

  /* Check */
  assert(0 < bit_len);
  assert(NULL != conf_bits);
  assert((-1 < path_id)&&((path_id < bit_len/2 - 1)||(path_id == bit_len/2 - 1)));

  /* All the others should be zero */
  for (i = 0; i < bit_len; i++) {
    conf_bits[i] = 0;
  }

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

/* Find the matched BL = 0 && WL == 1 according to the given BL=1 && WL = 0
 */
t_conf_bit_info* dump_verilog_find_paired_conf_bit(t_llist* conf_bits_llist_head,
                                                   t_conf_bit_info* src_conf_bit_info) {
  t_llist* temp_llist = conf_bits_llist_head;
  t_conf_bit_info* temp_conf_bit_info = NULL;

  /* return directly when source is NULL */
  if (NULL == src_conf_bit_info) {
    return NULL;
  }

  /* Check */
  assert((1 == src_conf_bit_info->bl_val)&&(0 == src_conf_bit_info->wl_val));

  /* Traverse the linked list, find a paired element 
   * with same parent_spice_model, parent_spice_model_index and BL = 0, WL = 1
   */  
  while (NULL != temp_llist) {
    /* Fetch the information of a configuration bit */
    temp_conf_bit_info = (t_conf_bit_info*)(temp_llist->dptr);
    if ((temp_conf_bit_info->parent_spice_model == src_conf_bit_info->parent_spice_model)
      &&(temp_conf_bit_info->parent_spice_model_index == src_conf_bit_info->parent_spice_model_index)
      &&(0 == temp_conf_bit_info->bl_val) 
      &&(1 == temp_conf_bit_info->wl_val)) {
      return temp_conf_bit_info; 
    }
    /* Go to the next element */
    temp_llist = temp_llist->next;
  }

  /* Reach here means we find nothing! */
  return NULL;
}

/** Generate the pairs for two configuration bits, which should be enabled at the same programming cycle
 * These pair are only created between two configuration bits of RRAM MUX:
 * 1. A conf_bit with BL=1, WL=0 is paired to a conf_bit BL=0, WL=1.
 * 2. A conf_bit with BL=1, WL=1 should not have a pair.
 */
void dump_verilog_gen_pairs_conf_bits(t_llist* conf_bits_llist_head) {
  t_llist* temp_llist = conf_bits_llist_head;
  t_conf_bit_info* temp_conf_bit_info = NULL;

  t_llist* temp_compnt_llist_head = NULL; /* Store the head of linked list of current component */
  t_conf_bit_info* temp_compnt_head_conf_bit_info = NULL;
  
  /* Paired conf_conf_bit_info*/
  t_conf_bit_info* paired_conf_bit_info = NULL;

  /* Traverse the linked list, find any element with BL=1, WL=0 */
  while (NULL != temp_llist) {
    /* Fetch the information of a configuration bit */
    temp_conf_bit_info = (t_conf_bit_info*)(temp_llist->dptr);
    /* Check and update the head of current component */
    if (NULL == temp_compnt_llist_head) {
    /* If it is NULL, just give the value */
      temp_compnt_llist_head = temp_llist;
      temp_compnt_head_conf_bit_info = (t_conf_bit_info*)(temp_compnt_llist_head->dptr);
    } else {
    /* If it is not NULL, we need to check if current component matches the existed one. */
      if ((temp_conf_bit_info->parent_spice_model != temp_compnt_head_conf_bit_info->parent_spice_model)
        ||(temp_conf_bit_info->parent_spice_model_index != temp_compnt_head_conf_bit_info->parent_spice_model_index)) {
        temp_compnt_llist_head = temp_llist;
        temp_compnt_head_conf_bit_info = (t_conf_bit_info*)(temp_compnt_llist_head->dptr);
      }
    }
    /* Check only for conf_bit belonging to a RRAM MUX, with  BL = 1 and WL = 0; */
    if ((SPICE_MODEL_MUX == temp_conf_bit_info->parent_spice_model->type) 
      &&(SPICE_MODEL_DESIGN_RRAM == temp_conf_bit_info->parent_spice_model->design_tech) 
      &&(1 == temp_conf_bit_info->bl_val) 
      &&(0 == temp_conf_bit_info->wl_val)) {
      /* Find its paired element with BL = 0 and WL = 1 */
      /* Start from the found component linked-list head */
      paired_conf_bit_info = dump_verilog_find_paired_conf_bit(temp_compnt_llist_head, temp_conf_bit_info);
      /* Should not be NULL ! */
      assert(NULL != paired_conf_bit_info);
      /* Establish mutual link */
      temp_conf_bit_info->pair_conf_bit = paired_conf_bit_info;
      paired_conf_bit_info->pair_conf_bit = temp_conf_bit_info;
    }
    /* Go to the next element */
    temp_llist = temp_llist->next;
  }

  return;
}

/* Return the port_type in a verilog format */
char* verilog_convert_port_type_to_string(enum e_spice_model_port_type port_type) {
  switch (port_type) {
  case SPICE_MODEL_PORT_INPUT: 
  case SPICE_MODEL_PORT_CLOCK: 
  case SPICE_MODEL_PORT_SRAM:
  case SPICE_MODEL_PORT_BL:
  case SPICE_MODEL_PORT_WL:
    return "input";
  case SPICE_MODEL_PORT_OUTPUT: 
    return "output";
  case SPICE_MODEL_PORT_INOUT: 
    return "inout";
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid port type!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  return NULL;
}

/* Dump all the global ports that are stored in the linked list 
 * Return the number of ports that have been dumped 
 */
int rec_dump_verilog_spice_model_global_ports(FILE* fp, 
                                              t_spice_model* cur_spice_model,
                                              boolean dump_port_type, boolean recursive) {
  int iport, dumped_port_cnt;
  boolean dump_comma = FALSE;

  dumped_port_cnt = 0;

  /* Check */
  assert(NULL != cur_spice_model);
  if (0 < cur_spice_model->num_port) {
    assert(NULL != cur_spice_model->ports);
  }

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
  }

  fprintf(fp, "//----- BEGIN Global ports of SPICE_MODEL(%s) -----\n",
          cur_spice_model->name);

  for (iport = 0; iport < cur_spice_model->num_port; iport++) {
    /* if this spice model requires customized netlist to be included, we do not go recursively */
    if (TRUE == recursive) { 
      /* GO recursively first, and meanwhile count the number of global ports */
      /* For the port that requires another spice_model, i.e., SRAM
       * We need include any global port in that spice model
       */
      if (NULL != cur_spice_model->ports[iport].spice_model) {
        /* Check if we need to dump a comma */
        if (TRUE == dump_comma) {
          fprintf(fp, ",\n");
        }
        dumped_port_cnt += 
           rec_dump_verilog_spice_model_global_ports(fp, cur_spice_model->ports[iport].spice_model, 
                                                     dump_port_type, recursive);
        /* Decide if we need a comma */
        dump_comma = TRUE; 
        continue;
      }
    }
    /* By pass non-global ports*/
    if (FALSE == cur_spice_model->ports[iport].is_global) {
      continue;
    }
    /* Check if we need to dump a comma */
    if (TRUE == dump_comma) {
      fprintf(fp, ",\n");
    }
    if (TRUE == dump_port_type) {
      fprintf(fp, "%s [0:%d] %s", 
              verilog_convert_port_type_to_string(cur_spice_model->ports[iport].type),
              cur_spice_model->ports[iport].size - 1, 
              cur_spice_model->ports[iport].prefix);
    } else {
      fprintf(fp, "%s[0:%d]", 
            cur_spice_model->ports[iport].prefix,
            cur_spice_model->ports[iport].size - 1); 
    }
    /* Decide if we need a comma */
    dump_comma = TRUE; 
    /* Update counter */
    dumped_port_cnt++;
  }

  fprintf(fp, "//----- END Global ports of SPICE_MODEL(%s)-----\n",
          cur_spice_model->name);

  return dumped_port_cnt;
}

/* Dump all the global ports that are stored in the linked list */
int dump_verilog_global_ports(FILE* fp, t_llist* head,
                               boolean dump_port_type) {
  t_llist* temp = head;
  t_spice_model_port* cur_global_port = NULL;
  int dumped_port_cnt = 0;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
  }

  fprintf(fp, "//----- BEGIN Global ports -----\n");
  while(NULL != temp) {
    cur_global_port = (t_spice_model_port*)(temp->dptr); 
    if (TRUE == dump_port_type) {
      fprintf(fp, "%s [0:%d] %s", 
              verilog_convert_port_type_to_string(cur_global_port->type),
              cur_global_port->size - 1, 
              cur_global_port->prefix);
    } else {
      fprintf(fp, "%s[0:%d]", 
              cur_global_port->prefix,
              cur_global_port->size - 1); 
    }
    /* if this is the tail, we do not dump a comma */
    if (NULL != temp->next) {
     fprintf(fp, ",");
    }
    fprintf(fp, "\n");
    /* Update counter */
    dumped_port_cnt++;
    /* Go to the next */
    temp = temp->next;
  }
  fprintf(fp, "//----- END Global ports -----\n");

  return dumped_port_cnt;
}

