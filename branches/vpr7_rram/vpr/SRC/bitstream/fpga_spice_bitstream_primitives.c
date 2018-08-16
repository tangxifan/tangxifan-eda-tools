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
#include <sys/stat.h>
#include <unistd.h>

/* Include vpr structs*/
#include "util.h"
#include "physical_types.h"
#include "vpr_types.h"
#include "globals.h"
#include "rr_graph.h"
#include "rr_graph_swseg.h"
#include "vpr_utils.h"

/* Include spice support headers*/
#include "linkedlist.h"
#include "fpga_spice_globals.h"
#include "fpga_spice_utils.h"
#include "fpga_spice_lut_utils.h"
#include "fpga_spice_mux_utils.h"
#include "fpga_spice_pbtypes_utils.h"
#include "fpga_spice_bitstream_utils.h"

enum e_ff_trigger_type {
  FF_RE, FF_FE
};

/* Subroutines */
void fpga_spice_generate_bitstream_pb_primitive_ff(t_logical_block* mapped_logical_block,
                                                  t_pb_graph_node* prim_pb_graph_node,
                                                  int index,
                                                  t_spice_model* verilog_model,
                                                  t_sram_orgz_info* cur_sram_orgz_info) {

  /* Back-annotate to logical block */
  if (NULL != mapped_logical_block) {
    mapped_logical_block->mapped_spice_model = verilog_model;
    mapped_logical_block->mapped_spice_model_index = verilog_model->cnt;
  }
 
  assert(SPICE_MODEL_FF == verilog_model->type);

  verilog_model->cnt++;

  return;
}

/* Print hardlogic SPICE subckt*/
void fpga_spice_generate_bitstream_pb_primitive_hardlogic(t_logical_block* mapped_logical_block,
                                                          t_pb_graph_node* prim_pb_graph_node,
                                                          int index,
                                                          t_spice_model* verilog_model,
                                                          t_sram_orgz_info* cur_sram_orgz_info) {

  /* Back-annotate to logical block */
  if (NULL != mapped_logical_block) {
    mapped_logical_block->mapped_spice_model = verilog_model;
    mapped_logical_block->mapped_spice_model_index = verilog_model->cnt;
  }
  
  /* Asserts */
  assert(SPICE_MODEL_HARDLOGIC == verilog_model->type);
  
  verilog_model->cnt++;

  return;
}

/* Dump a I/O pad primitive node */
void fpga_spice_generate_bitstream_pb_primitive_io(t_logical_block* mapped_logical_block,
                                                   t_pb_graph_node* prim_pb_graph_node,
                                                   int index,
                                                   t_spice_model* verilog_model,
                                                   t_sram_orgz_info* cur_sram_orgz_info) {
  int num_pad_port = 0; /* INOUT port */
  t_spice_model_port** pad_ports = NULL;
  int num_input_port = 0;
  t_spice_model_port** input_ports = NULL;
  int num_output_port = 0;
  t_spice_model_port** output_ports = NULL;
  int num_clock_port = 0;
  t_spice_model_port** clock_ports = NULL;
  int num_sram_port = 0;
  t_spice_model_port** sram_ports = NULL;
  
  int i;
  int num_sram = 0;
  int* sram_bits = NULL;

  t_pb_type* prim_pb_type = NULL;

  /* For each SRAM, we could have multiple BLs/WLs */
  int num_bl_ports = 0;
  t_spice_model_port** bl_port = NULL;
  int num_wl_ports = 0;
  t_spice_model_port** wl_port = NULL;
  int num_bl_per_sram = 0;
  int num_wl_per_sram = 0;
  int expected_num_sram;

  int cur_num_sram = 0;
  t_spice_model* mem_model = NULL;

  /* Ensure a valid pb_graph_node */ 
  if (NULL == prim_pb_graph_node) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid prim_pb_graph_node!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Find ports*/
  pad_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_INOUT, &num_pad_port, TRUE);
  input_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_INPUT, &num_input_port, TRUE);
  output_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_OUTPUT, &num_output_port, TRUE);
  clock_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_CLOCK, &num_clock_port, TRUE);
  sram_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_SRAM, &num_sram_port, TRUE);

  /* Check */
  assert((1 == num_sram_port)&&(NULL != sram_ports)&&(1 == sram_ports[0]->size));

  /* Asserts */
  assert(SPICE_MODEL_IOPAD == verilog_model->type); /* Support IO PAD which matches the physical design */
  
  /* Initialize */
  get_sram_orgz_info_mem_model(cur_sram_orgz_info, &mem_model);

  prim_pb_type = prim_pb_graph_node->pb_type;

  switch (cur_sram_orgz_info->type) {
  case SPICE_SRAM_MEMORY_BANK:
    /* Local wires */
    /* Find the number of BLs/WLs of each SRAM */
    /* Detect the SRAM SPICE model linked to this SRAM port */
    assert(NULL != sram_ports[0]->spice_model);
    assert(SPICE_MODEL_SRAM == sram_ports[0]->spice_model->type);
    find_bl_wl_ports_spice_model(sram_ports[0]->spice_model, 
                                 &num_bl_ports, &bl_port, &num_wl_ports, &wl_port); 
    assert(1 == num_bl_ports);
    assert(1 == num_wl_ports);
    num_bl_per_sram = bl_port[0]->size; 
    num_wl_per_sram = wl_port[0]->size; 
    break;
  case SPICE_SRAM_STANDALONE:
  case SPICE_SRAM_SCAN_CHAIN:
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid SRAM organization type!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Check */
  assert((1 == num_sram_port)&&(NULL != sram_ports)&&(1 == sram_ports[0]->size));
  /* what is the SRAM bit of a mode? */
  /* If logical block is not NULL, we need to decode the sram bit */
  if (NULL != mapped_logical_block) {
    assert(NULL != mapped_logical_block->pb->pb_graph_node->pb_type->mode_bits);
    sram_bits = decode_mode_bits(mapped_logical_block->pb->pb_graph_node->pb_type->mode_bits, &expected_num_sram);
    assert(expected_num_sram == num_sram);
  } else {
    /* Initialize */
    sram_bits = (int*)my_calloc(num_sram, sizeof(int));
    for (i = 0; i < num_sram; i++) { 
      sram_bits[i] = sram_ports[0]->default_val;
    }
  }
  /* SRAM_bit will be later reconfigured according to operating mode */
  switch (cur_sram_orgz_info->type) {
  case SPICE_SRAM_MEMORY_BANK:
    for (i = 0; i < num_sram; i++) {
      /* Decode the SRAM bits to BL/WL bits.
       * first half part is BL, the other half part is WL 
       */
      decode_and_add_sram_membank_conf_bit_to_llist(cur_sram_orgz_info, cur_num_sram + i,
                                                    num_bl_per_sram, num_wl_per_sram,
                                                    sram_bits[i]);
    }
    break;
  case SPICE_SRAM_STANDALONE:
  case SPICE_SRAM_SCAN_CHAIN:
    /* Store the configuraion bit to linked-list */
    add_mux_conf_bits_to_llist(0, cur_sram_orgz_info, 
                               num_sram, sram_bits,
                               verilog_model);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid SRAM organization type!\n",
               __FILE__, __LINE__);
    exit(1);
  }
 
  /* Back-annotate to logical block */
  if (NULL != mapped_logical_block) {
    mapped_logical_block->mapped_spice_model = verilog_model;
    mapped_logical_block->mapped_spice_model_index = verilog_model->cnt;
  }

  /* Update the verilog_model counter */
  verilog_model->cnt++;

  /*Free*/ 
  my_free(input_ports);
  my_free(output_ports);
  my_free(pad_ports);
  my_free(clock_ports);
  my_free(sram_ports);
  my_free(sram_bits);

  return;
}

void fpga_spice_generate_bitstream_pb_primitive_lut(t_logical_block* mapped_logical_block,
                                                    t_pb_graph_node* cur_pb_graph_node,
                                                    int index,
                                                    t_spice_model* verilog_model,
                                                    t_sram_orgz_info* cur_sram_orgz_info) {
  int i;
  int* sram_bits = NULL; /* decoded SRAM bits */ 
  int truth_table_length = 0;
  char** truth_table = NULL;
  int lut_size = 0;
  int num_input_port = 0;
  t_spice_model_port** input_ports = NULL;
  int num_output_port = 0;
  t_spice_model_port** output_ports = NULL;
  int num_sram_port = 0;
  t_spice_model_port** sram_ports = NULL;

  t_pb_type* cur_pb_type = NULL;
  int cur_num_sram = 0;
  int num_sram = 0;
  /* For each SRAM, we could have multiple BLs/WLs */
  int num_bl_ports = 0;
  t_spice_model_port** bl_port = NULL;
  int num_wl_ports = 0;
  t_spice_model_port** wl_port = NULL;
  int num_bl_per_sram = 0;
  int num_wl_per_sram = 0;
  t_spice_model* mem_model = NULL;

  /* Ensure a valid pb_graph_node */ 
  if (NULL == cur_pb_graph_node) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid cur_pb_graph_node!\n",
               __FILE__, __LINE__);
    exit(1);
  }
  /* Asserts */
  assert(SPICE_MODEL_LUT == verilog_model->type);

  /* Check if this is an idle logical block mapped*/
  if (NULL != mapped_logical_block) {
    truth_table = assign_lut_truth_table(mapped_logical_block, &truth_table_length); 
    /* Back-annotate to logical block */
    mapped_logical_block->mapped_spice_model = verilog_model;
    mapped_logical_block->mapped_spice_model_index = verilog_model->cnt;
  }
  /* Determine size of LUT*/
  input_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_INPUT, &num_input_port, TRUE);
  output_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_OUTPUT, &num_output_port, TRUE);
  assert(1 == num_input_port);
  assert(1 == num_output_port);
  lut_size = input_ports[0]->size;
  assert(1 == output_ports[0]->size);
  /* Find SRAM ports */
  sram_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_SRAM, &num_sram_port, TRUE);
  assert(1 == num_sram_port);
  /* Count the number of configuration bits */
  num_sram = count_num_sram_bits_one_spice_model(verilog_model, -1);
  /* Get memory model */
  get_sram_orgz_info_mem_model(cur_sram_orgz_info, &mem_model);

  /* Find the number of BLs/WLs of each SRAM */
  switch (cur_sram_orgz_info->type) {
  case SPICE_SRAM_MEMORY_BANK:
    /* Detect the SRAM SPICE model linked to this SRAM port */
    assert(NULL != sram_ports[0]->spice_model);
    assert(SPICE_MODEL_SRAM == sram_ports[0]->spice_model->type);
    find_bl_wl_ports_spice_model(sram_ports[0]->spice_model, 
                                 &num_bl_ports, &bl_port, &num_wl_ports, &wl_port); 
    assert(1 == num_bl_ports);
    assert(1 == num_wl_ports);
    num_bl_per_sram = bl_port[0]->size; 
    num_wl_per_sram = wl_port[0]->size; 
    /* Asserts */
    assert(num_bl_per_sram == num_wl_per_sram);
    break;
  case SPICE_SRAM_STANDALONE:
  case SPICE_SRAM_SCAN_CHAIN:
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid SRAM organization type!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* Generate sram bits*/
  sram_bits = generate_lut_sram_bits(truth_table_length, truth_table, 
                                     lut_size, sram_ports[0]->default_val);

  /* Print the subckts*/ 
  cur_pb_type = cur_pb_graph_node->pb_type;

  /* Decode the SRAM bits to BL/WL bits. */ 
  switch (cur_sram_orgz_info->type) {
  case SPICE_SRAM_MEMORY_BANK:
    cur_num_sram = get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info);
    for (i = 0; i < num_sram; i++) {
      /* TODO: should be more structural in coding !!! */
      /* Decode the SRAM bits to BL/WL bits.
       * first half part is BL, the other half part is WL 
       */
      decode_and_add_sram_membank_conf_bit_to_llist(cur_sram_orgz_info, cur_num_sram + i,
                                                    num_bl_per_sram, num_wl_per_sram,
                                                    sram_bits[i]);
    }
    /* NUM_SRAM is set to be consistent with number of BL/WLs
     * TODO: NUM_SRAM should be the as they are. 
     * Should use another variable i.e., num_bl
     */
    break;
  case SPICE_SRAM_STANDALONE:
  case SPICE_SRAM_SCAN_CHAIN:
    /* Store the configuraion bit to linked-list */
    add_mux_conf_bits_to_llist(0, cur_sram_orgz_info, 
                               num_sram, sram_bits,
                               verilog_model);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid SRAM organization type!\n",
               __FILE__, __LINE__);
    exit(1);
  }
 
  /* Update counter */
  verilog_model->cnt++;

  /*Free*/
  my_free(input_ports);
  my_free(output_ports);
  my_free(sram_ports);
  my_free(sram_bits);

  return;
}
