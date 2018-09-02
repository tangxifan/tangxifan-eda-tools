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
  if ((0 != strcmp(tokens[1], "1"))&&(0 != strcmp(tokens[1], "0"))) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Last token of truth table line should be [0|1]!\n",
               __FILE__, __LINE__); 
    exit(1);
  }
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
  assert((unsigned)(lut_size + 1 + 1)== strlen(truth_table_line)); /* lut_size + space + '1' */
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

int* generate_lut_sram_bits(int truth_table_len,
                            char** truth_table,
                            int lut_size,
                            int default_sram_bit_value) {
  int num_sram = (int)pow(2.,(double)(lut_size));
  int* ret = (int*)my_malloc(sizeof(int)*num_sram); 
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
  }

  /* Read in truth table lines, decode one by one */
  for (i = 0; i < truth_table_len; i++) {
    /* Complete the truth table line by line*/
    //printf("truth_table[%d] = %s\n", i, truth_table[i]);
    completed_truth_table[i] = complete_truth_table_line(lut_size, truth_table[i]);
    //printf("Completed_truth_table[%d] = %s\n", i, completed_truth_table[i]);
    if (0 == strcmp(" 1", completed_truth_table[i] + lut_size)) {
      on_set = 1;
    } else if (0 == strcmp(" 0", completed_truth_table[i] + lut_size)) {
      off_set = 1;
    }
  }
  //printf("on_set=%d off_set=%d", on_set, off_set);
  if (1 != (on_set + off_set)) {
  assert(1 == (on_set + off_set));
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
    /* We must find one ! */ 
    assert (OPEN != lut_to_lb_net_mapping[inet]);
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
    /* Handle the truth table pin remapping */
    truth_table[cur] = (char*) my_calloc (lut_size + 3, sizeof(char)); /* last 3 bits are space + on/off set + \0 */
    for (inet = 0; inet < lut_size; inet++) {
      /* Open net implies a don't care */
      if (OPEN  == lut_pin_vpack_net_num[inet]) {
        truth_table[cur][inet] = '-';
        continue;
      }
      /* Find the desired truth table bit */
      truth_table[cur][inet] = ((char*)(head->data_vptr))[lut_to_lb_net_mapping[inet]];
    }
    /* Add string ends */
    truth_table[cur][lut_size] = ' ';
    truth_table[cur][lut_size + 1] = ((char*)(head->data_vptr))[strlen((char*)(head->data_vptr)) - 1];
    truth_table[cur][lut_size + 2] = '\0';
    head = head->next;
    cur++;
  }
  assert(cur == (*truth_table_length));

  return truth_table;
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


