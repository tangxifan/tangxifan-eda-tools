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
#include "spice_utils.h"

/* Include verilog support headers*/
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_pbtypes.h"
#include "verilog_lut.h"


/***** Subroutines *****/
void dump_verilog_pb_primitive_lut(FILE* fp,
                                   char* subckt_prefix,
                                   t_logical_block* mapped_logical_block,
                                   t_pb_graph_node* cur_pb_graph_node,
                                   int index,
                                   t_spice_model* verilog_model) {
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

  char* formatted_subckt_prefix = format_spice_node_prefix(subckt_prefix); /* Complete a "_" at the end if needed*/
  t_pb_type* cur_pb_type = NULL;
  char* port_prefix = NULL;
  int cur_sram = 0;
  int num_sram = 0;

  /* Ensure a valid file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler!\n",
               __FILE__, __LINE__);
    exit(1);
  }

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
    /* Back-annotate to logical block */
    mapped_logical_block->mapped_spice_model = verilog_model;
    mapped_logical_block->mapped_spice_model_index = verilog_model->cnt;
  }
  /* Determine size of LUT*/
  input_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_INPUT, &num_input_port);
  output_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_OUTPUT, &num_output_port);
  sram_ports = find_spice_model_ports(verilog_model, SPICE_MODEL_PORT_SRAM, &num_sram_port);
  assert(1 == num_input_port);
  assert(1 == num_output_port);
  assert(1 == num_sram_port);
  lut_size = input_ports[0]->size;
  num_sram = (int)pow(2.,(double)(lut_size));
  assert(num_sram == sram_ports[0]->size);
  assert(1 == output_ports[0]->size);

  /* Print the subckts*/ 
  cur_pb_type = cur_pb_graph_node->pb_type;
  /* Subckt definition*/
  fprintf(fp, "module %s%s[%d] (", formatted_subckt_prefix, cur_pb_type->name, index);
  /* Print inputs, outputs, inouts, clocks, NO SRAMs*/
  /*
  port_prefix = (char*)my_malloc(sizeof(char)*
                (strlen(formatted_subckt_prefix) + strlen(cur_pb_type->name) + 1 +
                 strlen(my_itoa(index)) + 1 + 1));
  sprintf(port_prefix, "%s%s[%d]", formatted_subckt_prefix, cur_pb_type->name, index);
  */
  /* Simplify the prefix, make the SPICE netlist readable*/
  port_prefix = (char*)my_malloc(sizeof(char)*
                (strlen(cur_pb_type->name) + 1 +
                 strlen(my_itoa(index)) + 1 + 1));
  sprintf(port_prefix, "%s[%d]", cur_pb_type->name, index);
  
  dump_verilog_pb_type_ports(fp, port_prefix, 0, cur_pb_type, TRUE); 
  /* SRAM bits are fixed in this subckt, no need to define them here*/
  /* Local Vdd and gnd*/ 
  fprintf(fp, ");\n");
  /* Definition ends*/

  /* Print the encoding in SPICE netlist for debugging */
  fprintf(fp, "//----- Truth Table for LUT[%d], size=%d. -----\n", 
          verilog_model->cnt, lut_size);
  for (i = 0; i < truth_table_length; i++) {
    fprintf(fp,"//  %s \n", truth_table[i]);
  } 

  fprintf(fp, "//----- SRAM bits for LUT[%d], size=%d, num_sram=%d. -----\n", 
          verilog_model->cnt, lut_size, num_sram);
  fprintf(fp, "//-----");
  for (i = 0; i < num_sram; i++) {
     fprintf(fp, "%d", sram_bits[i]);
  }
  fprintf(fp, "-----\n");

  /* Call SRAM subckts*/
  cur_sram = sram_verilog_model->cnt;
  for (i = 0; i < num_sram; i++) {
    fprintf(fp, "%s %s[%d] (", sram_verilog_model->name, sram_verilog_model->prefix, cur_sram); /* SRAM subckts*/
    /* fprintf(fp, "%s[%d]->in ", sram_verilog_model->prefix, cur_sram);*/ /* Input*/
    fprintf(fp, "%s->in ", sram_verilog_model->prefix); /* Input*/
    fprintf(fp, "%s[%d]->out %s[%d]->outb ", 
            sram_verilog_model->prefix, cur_sram, sram_verilog_model->prefix, cur_sram); /* Outputs */
    fprintf(fp, ");\n");  //
    cur_sram++;
  }

  /* Call LUT subckt*/
  fprintf(fp, "%s %s[%d] ", verilog_model->name, verilog_model->prefix, verilog_model->cnt);
  /* Connect inputs*/ 
  /* Connect outputs*/
  dump_verilog_pb_type_ports(fp, port_prefix, 0, cur_pb_type, FALSE); 
  /* Connect srams*/
  cur_sram = sram_verilog_model->cnt;
  for (i = 0; i < num_sram; i++) {
    fprintf(fp, "%s[%d]->out ", sram_verilog_model->prefix, cur_sram); 
    cur_sram++;
  }
  /* vdd should be connected to special global wire gvdd_lut and gnd,
   * Every LUT has a special VDD for statistics
   */
  fprintf(fp, ");\n");
  /* TODO: Add a nodeset for convergence */

  /* End of subckt*/
  fprintf(fp, "endmodule\n");
  
  verilog_model->cnt++;
  sram_verilog_model->cnt = cur_sram;

  /*Free*/
  my_free(formatted_subckt_prefix);
  my_free(input_ports);
  my_free(output_ports);
  my_free(sram_ports);
  my_free(sram_bits);
  my_free(port_prefix);

  return;
}
