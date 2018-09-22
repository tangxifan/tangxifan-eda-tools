/***********************************/
/*      SPICE Modeling for VPR     */
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
#include "route_common.h"

/* Include SPICE support headers*/
#include "quicksort.h"
#include "linkedlist.h"
#include "fpga_spice_types.h"
#include "fpga_spice_globals.h"
#include "fpga_spice_utils.h"
#include "fpga_spice_pbtypes_utils.h"


char* complete_truth_table_line(int lut_size,
                                char* input_truth_table_line) {
  char* ret = NULL;
  int num_token = 0;
  char** tokens = NULL;
  int cover_len = 0;
  int j;

  /* Due to the size of truth table may be less than the lut size.
   * i.e. in LUT-6 architecture, there exists LUT1-6 in technology-mapped netlists
   * So, in truth table line, there may be 10- 1
   * In this case, we should complete it by --10- 1
   */ 
  /*Malloc the completed truth table, lut_size + space + truth_val + '\0'*/
  ret = (char*)my_malloc(sizeof(char)*lut_size + 3);
  /* Split one line of truth table line*/
  tokens = fpga_spice_strtok(input_truth_table_line, " ", &num_token); 
  /* Check, only 2 tokens*/
  /* Sometimes, the truth table is ' 0' or ' 1', which corresponds to a constant */
  if (1 == num_token) {
    /* restore the token[0]*/
    tokens = (char**)realloc(tokens, 2*sizeof(char*));
    tokens[1] = tokens[0];
    tokens[0] = my_strdup("-");
    num_token = 2;
  }

  /* In Most cases, there should be 2 tokens. */
  assert(2 == num_token);
  /* We may have two truth table from two LUTs which contain both 0 and 1*/
  /*
  if ((0 != strcmp(tokens[1], "1"))&&(0 != strcmp(tokens[1], "0"))) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Last token of truth table line should be [0|1]!\n",
               __FILE__, __LINE__); 
    exit(1);
  }
  */
  /* Complete the truth table line*/
  cover_len = strlen(tokens[0]); 
  assert((cover_len < lut_size)||(cover_len == lut_size));

  /* Copy the original truth table line */ 
  for (j = 0; j < cover_len; j++) {
    ret[j] = tokens[0][j];
  }
  /* Add the number of '-' we should add in the back !!! */
  for (j = cover_len; j < lut_size; j++) {
    ret[j] = '-';
  }

  /* Copy the original truth table line */ 
  sprintf(ret + lut_size, " %s", tokens[1]);

  /* Free */
  for (j = 0; j < num_token; j++) {
    my_free(tokens[j]);
  }

  return ret;
}

/* For each lut_bit_lines, we should recover the truth table,
 * and then set the sram bits to "1" if the truth table defines so.
 * Start_point: the position we start decode recursively
 */
void configure_lut_sram_bits_per_line_rec(int** sram_bits, 
                                          int lut_size,
                                          char* truth_table_line,
                                          int start_point) {
  int i;
  int num_sram_bit = (int)pow(2., (double)(lut_size));
  char* temp_line = my_strdup(truth_table_line);
  int do_config = 1;
  int sram_id = 0;

  /* Check the length of sram bits and truth table line */
  //assert((sizeof(int)*num_sram_bit) == sizeof(*sram_bits)); /*TODO: fix this assert*/
  if ((unsigned)(lut_size + 1 + 1) != strlen(truth_table_line)){ /* lut_size + space + '1' */
  assert((unsigned)(lut_size + 1 + 1) == strlen(truth_table_line)); /* lut_size + space + '1' */
  }
  /* End of truth_table_line should be "space" and "1" */ 
  assert((0 == strcmp(" 1", truth_table_line + lut_size))||(0 == strcmp(" 0", truth_table_line + lut_size)));
  /* Make sure before start point there is no '-' */
  for (i = 0; i < start_point; i++) {
    assert('-' != truth_table_line[i]);
  }

  /* Configure sram bits recursively */
  for (i = start_point; i < lut_size; i++) {
    if ('-' == truth_table_line[i]) {
      do_config = 0;
      /* if we find a dont_care, we don't do configure now but recursively*/
      /* '0' branch */
      temp_line[i] = '0'; 
      configure_lut_sram_bits_per_line_rec(sram_bits, lut_size, temp_line, start_point + 1);
      /* '1' branch */
      temp_line[i] = '1'; 
      configure_lut_sram_bits_per_line_rec(sram_bits, lut_size, temp_line, start_point + 1);
      break; 
    }
  }

  /* do_config*/
  if (do_config) {
    for (i = 0; i < lut_size; i++) {
      /* Should be either '0' or '1' */
      switch (truth_table_line[i]) {
      case '0':
        /* We assume the 1-lut pass sram1 when input = 0 */
        sram_id += (int)pow(2., (double)(i));
        break;
      case '1':
        /* We assume the 1-lut pass sram0 when input = 1 */
        break;
      case '-':
        assert('-' != truth_table_line[i]); /* Make sure there is no dont_care */
      default :
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid truth_table bit(%c), should be [0|1|'-]!\n",
                   __FILE__, __LINE__, truth_table_line[i]); 
        exit(1);
      }
    }
    /* Set the sram bit to '1'*/
    assert((-1 < sram_id) && (sram_id < num_sram_bit));
    if (0 == strcmp(" 1", truth_table_line + lut_size)) {
      (*sram_bits)[sram_id] = 1; /* on set*/
    } else if (0 == strcmp(" 0", truth_table_line + lut_size)) {
      (*sram_bits)[sram_id] = 0; /* off set */
    } else {
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid truth_table_line ending(=%s)!\n",
                 __FILE__, __LINE__, truth_table_line + lut_size);
      exit(1);
    }
  }
  
  /* Free */
  my_free(temp_line);

  return; 
}

/* Determine if the truth table of a LUT is a on-set or a off-set */
int determine_lut_truth_table_on_set(int truth_table_len,
                                     char** truth_table) {
  int on_set = 0;
  int off_set = 0;
  int i, tt_line_len;

  for (i = 0; i < truth_table_len; i++) {
    tt_line_len = strlen(truth_table[i]);
    switch (truth_table[i][tt_line_len - 1]) {
    case '1':
      on_set = 1;
      break;
    case '0':
      off_set = 1;
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid truth_table_line ending(=%c)!\n",
                 __FILE__, __LINE__, truth_table[i][tt_line_len - 1]);
      exit(1);
    }
  }

  /* Prefer on_set if both are true */
  if (1 == (on_set + off_set)) {
    on_set = 1; off_set = 0;
  }

  return on_set;
}

int* generate_lut_sram_bits(int truth_table_len,
                            char** truth_table,
                            int lut_size,
                            int default_sram_bit_value) {
  int num_sram = (int)pow(2.,(double)(lut_size));
  int* ret = (int*)my_calloc(num_sram, sizeof(int)); 
  char** completed_truth_table = (char**)my_malloc(sizeof(char*)*truth_table_len);
  int on_set = 0;
  int off_set = 0;
  int i;

  /* if No truth_table, do default*/
  if (0 == truth_table_len) {
    switch (default_sram_bit_value) {
    case 0:
      off_set = 0;
      on_set = 1;
      break;
    case 1:
      off_set = 1;
      on_set = 0;
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid default_signal_init_value(=%d)!\n",
                 __FILE__, __LINE__, default_sram_bit_value);
      exit(1);
    }
  } else {
    on_set = determine_lut_truth_table_on_set(truth_table_len, truth_table);
    off_set = 1 - on_set;
  }

  /* Read in truth table lines, decode one by one */
  for (i = 0; i < truth_table_len; i++) {
    /* Complete the truth table line by line*/
    //printf("truth_table[%d] = %s\n", i, truth_table[i]);
    completed_truth_table[i] = complete_truth_table_line(lut_size, truth_table[i]);
    //printf("Completed_truth_table[%d] = %s\n", i, completed_truth_table[i]);
  }

  if (1 == on_set) {
    /* Initial all sram bits to 0*/
    for (i = 0 ; i < num_sram; i++) {
      ret[i] = 0;
    }
  } else if (1 == off_set) {
    /* Initial all sram bits to 1*/
    for (i = 0 ; i < num_sram; i++) {
      ret[i] = 1;
    }
  }

  for (i = 0; i < truth_table_len; i++) {
    /* Update the truth table, sram_bits */
    configure_lut_sram_bits_per_line_rec(&ret, lut_size, completed_truth_table[i], 0);
  }

  /* Free */
  for (i = 0; i < truth_table_len; i++) {
    my_free(completed_truth_table[i]);
  }

  return ret;
}

/* Provide the truth table of a mapped logical block 
 * 1. Reorgainze the truth table to be consistent with the mapped nets of a LUT
 * 2. Allocate the truth table in a clean char array and return
 */
char** assign_lut_truth_table(t_logical_block* mapped_logical_block,
                              int* truth_table_length) {
  char** truth_table = NULL;
  t_linked_vptr* head = NULL;
  int cur = 0;

  if (NULL == mapped_logical_block) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid mapped_logical_block!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Count the lines of truth table*/
  head = mapped_logical_block->truth_table;
  while(head) {
    (*truth_table_length)++;
    head = head->next;
  }
  /* Allocate truth_tables */
  truth_table = (char**)my_malloc(sizeof(char*)*(*truth_table_length));
  /* Fill truth_tables*/
  cur = 0;
  head = mapped_logical_block->truth_table;
  while(head) {
    truth_table[cur] = my_strdup((char*)(head->data_vptr));
    head = head->next;
    cur++;
  }
  assert(cur == (*truth_table_length));

  return truth_table;
}


/* Provide the truth table of a mapped logical block 
 * 1. Reorgainze the truth table to be consistent with the mapped nets of a LUT
 * 2. Allocate the truth table in a clean char array and return
 */
char** assign_post_routing_lut_truth_table(t_logical_block* mapped_logical_block,
                                           int lut_size, int* lut_pin_vpack_net_num,
                                           int* truth_table_length) {
  char** truth_table = NULL;
  t_linked_vptr* head = NULL;
  int cur = 0;
  int inet, jnet;
  int* lut_to_lb_net_mapping = NULL;
  int num_lb_pin = 0;
  int* lb_pin_vpack_net_num = NULL;
  int lb_truth_table_size = 0;

  if (NULL == mapped_logical_block) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid mapped_logical_block!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Allocate */
  lut_to_lb_net_mapping = (int*) my_malloc (sizeof(int) * lut_size);
  /* Find nets mapped to a logical block */
  get_lut_logical_block_input_pin_vpack_net_num(mapped_logical_block,
                                                &num_lb_pin, &lb_pin_vpack_net_num);
  /* Create a pin-to-pin net_num mapping */
  for (inet = 0; inet < lut_size; inet++) {
    lut_to_lb_net_mapping[inet] = OPEN;
    /* Bypass open nets */
    if (OPEN  == lut_pin_vpack_net_num[inet]) {
      continue;
    }
    assert (OPEN  != lut_pin_vpack_net_num[inet]);
    /* Find the position (offset) of each vpack_net_num in lb_pins */
    for (jnet = 0; jnet < num_lb_pin; jnet++) {
      if (lut_pin_vpack_net_num[inet] == lb_pin_vpack_net_num[jnet]) {
        lut_to_lb_net_mapping[inet] = jnet; 
        break;
      }  
    } 
    /* Not neccesary to find a one, some luts just share part of their pins  */ 
  } 

  /* Initialization */
  (*truth_table_length) = 0;
  /* Count the lines of truth table*/
  head = mapped_logical_block->truth_table;
  while(head) {
    (*truth_table_length)++;
    head = head->next;
  }
  /* Allocate truth_tables */
  truth_table = (char**)my_malloc(sizeof(char*)*(*truth_table_length));
  /* Fill truth_tables*/
  cur = 0;
  head = mapped_logical_block->truth_table;
  while(head) {
    /* Handle the truth table pin remapping */
    truth_table[cur] = (char*) my_malloc((lut_size + 3) * sizeof(char));
    /* Initialize */
    lb_truth_table_size = strlen((char*)(head->data_vptr));
    memcpy(truth_table[cur] + lut_size, (char*)(head->data_vptr) + lb_truth_table_size - 2, 3);
    /* Add */
    for (inet = 0; inet < lut_size; inet++) {
      /* Open net implies a don't care, or some nets are not in the list  */
      if ((OPEN  == lut_pin_vpack_net_num[inet]) 
        || (OPEN == lut_to_lb_net_mapping[inet])) {
        truth_table[cur][inet] = '-';
        continue;
      }
      /* Find the desired truth table bit */
      truth_table[cur][inet] = ((char*)(head->data_vptr))[lut_to_lb_net_mapping[inet]];
    }
    
    head = head->next;
    cur++;
  }
  assert(cur == (*truth_table_length));

  return truth_table;
} 

/* Find the output port of LUT that this logical block is mapped to */
t_pb_graph_pin* get_mapped_lut_phy_pb_output_pin(t_phy_pb* lut_phy_pb, 
                                                 t_logical_block* lut_logical_block) {
  int iport, ipin;
  int num_lut_output_ports;
  int* num_lut_output_pins;
  int** lut_output_vpack_net_num;
  int pin_rr_node_index;
  t_pb_graph_pin* ret_pin = NULL; /* The pin to return */
  int found_num_pins = 0;

  /* Find the vpack_net_num of the output of the lut_logical_block */
  get_logical_block_output_vpack_net_num(lut_logical_block, 
                                         &num_lut_output_ports, 
                                         &num_lut_output_pins, 
                                         &lut_output_vpack_net_num);

  /* Check */
  assert ( 1 == num_lut_output_ports);
  assert ( 1 == num_lut_output_pins[0]);
  assert ( OPEN != lut_output_vpack_net_num[0][0]);

  /* Search the output pins of lut_phy_pb in rr_graph in find */
  for (iport = 0; iport < lut_phy_pb->pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < lut_phy_pb->pb_graph_node->num_output_pins[iport]; ipin++) {
      /* Get the rr_node index of the pin */ 
      pin_rr_node_index = lut_phy_pb->pb_graph_node->output_pins[iport][ipin].rr_node_index_physical_pb;
      /* Get the vpack_net_num in the local rr_graph, see if we have a match */
      if (lut_output_vpack_net_num[0][0] != lut_phy_pb->rr_graph->rr_node[pin_rr_node_index].vpack_net_num) {
        continue;
      }
      /* Reach here, it means we have a match! */
      ret_pin = &(lut_phy_pb->pb_graph_node->output_pins[iport][ipin]);
      found_num_pins++;
    }
  }

  /* We should have only one match! */
  assert (1 == found_num_pins);

  /* Free */
  my_free(num_lut_output_pins);
  for (iport = 0; iport < num_lut_output_ports; iport++) {
    my_free(lut_output_vpack_net_num);
  }
  
  return ret_pin; 
}

/* Get LUT fracturable level of a pb_graph_pin */
int get_pb_graph_pin_lut_frac_level(t_pb_graph_pin* out_pb_graph_pin) {
  /* search the corresponding spice_model_port */
  return out_pb_graph_pin->port->spice_model_port->lut_frac_level;
}

/* Get LUT output mask  of a pb_graph_pin */
int get_pb_graph_pin_lut_output_mask(t_pb_graph_pin* out_pb_graph_pin) {
  int pin_number = out_pb_graph_pin->pin_number;
  /* search the corresponding spice_model_port */
  return out_pb_graph_pin->port->spice_model_port->lut_output_mask[pin_number];
}

/* Adapt truth table for a fracturable LUT
 * Determine fixed input bits for this truth table:
 * 1. input bits within frac_level (all '-' if not specified) 
 * 2. input bits outside frac_level, decoded to its output mask (0 -> first part -> all '1') 
 */
void adapt_truth_table_for_frac_lut(t_phy_pb* prim_phy_pb, 
                                    t_logical_block* lut_logical_block, 
                                    int truth_table_length, 
                                    char** truth_table) {
  t_pb_graph_pin* out_pb_graph_pin = NULL;
  int lut_frac_level;
  int lut_output_mask;
  int i, lut_size, num_mask_bits;
  int temp;
  char* mask_bits  = NULL;

  /* Find the output port of LUT that this logical block is mapped to */
  out_pb_graph_pin = get_mapped_lut_phy_pb_output_pin(prim_phy_pb, lut_logical_block); 
  /* find the corresponding SPICE model output port and assoicated lut_output_mask */
  lut_frac_level = get_pb_graph_pin_lut_frac_level(out_pb_graph_pin);
  lut_output_mask = get_pb_graph_pin_lut_output_mask(out_pb_graph_pin);

  /* Apply modification to the truth table */
  for (i = 0; i < truth_table_length; i++) {
    /* Last two chars are fixed */
    lut_size = strlen(truth_table[i]) - 2;
    /* Get the number of bits to be masked (modified) */
    num_mask_bits = lut_size - lut_frac_level;
    /* Check if we need to modify any bits */
    assert (-1 < num_mask_bits);
    if ( 0 == num_mask_bits ) {
      continue;
    }
    /* Modify bits starting from lut_frac_level */
    /* decode the lut_output_mask to LUT input codes */ 
    temp = pow(2., num_mask_bits) - 1 - lut_output_mask;
    mask_bits = my_itobin(temp, num_mask_bits);
    /* copy the bits to the truth table line */
    memcpy(truth_table[i] + lut_frac_level, mask_bits, num_mask_bits);
    /* free */
    my_free(mask_bits);
  }

  return;
}

int determine_lut_path_id(int lut_size,
                          int* lut_inputs) {
  int path_id = 0;
  int i;
  
  for (i = 0; i < lut_size; i++) {
    switch (lut_inputs[i]) {
    case 0:
      path_id += (int)pow(2., (double)(i));
      break;
    case 1:
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid sram_bits[%d]!\n", 
                 __FILE__, __LINE__, i);
      exit(1);
    }
  }

  return path_id;
}


