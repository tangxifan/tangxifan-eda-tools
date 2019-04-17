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
#include "vpr_utils.h"
#include "route_common.h"

/* Include SPICE support headers*/
#include "linkedlist.h"
#include "fpga_x2p_types.h"
#include "fpga_x2p_utils.h"
#include "fpga_x2p_mux_utils.h"
#include "fpga_x2p_pbtypes_utils.h"
#include "fpga_x2p_bitstream_utils.h"
#include "spice_mux.h"
#include "fpga_x2p_globals.h"

/* Include Synthesizable Verilog headers */
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_primitives.h"
#include "verilog_pbtypes.h"

/***** Subroutines *****/

/* Find spice_model_name definition in pb_types
 * Try to match the name with defined spice_models
 */
void match_pb_types_verilog_model_rec(t_pb_type* cur_pb_type,
                                    int num_verilog_model,
                                    t_spice_model* verilog_models) {
  int imode, ipb, jinterc;
  
  if (NULL == cur_pb_type) {
    vpr_printf(TIO_MESSAGE_WARNING,"(File:%s,LINE[%d])cur_pb_type is null pointor!\n",__FILE__,__LINE__);
    return;
  }

  /* If there is a spice_model_name, this is a leaf node!*/
  if (NULL != cur_pb_type->spice_model_name) {
    /* What annoys me is VPR create a sub pb_type for each lut which suppose to be a leaf node
     * This may bring software convience but ruins SPICE modeling
     */
    /* Let's find a matched verilog model!*/
    printf("INFO: matching cur_pb_type=%s with spice_model_name=%s...\n",cur_pb_type->name, cur_pb_type->spice_model_name);
    assert(NULL == cur_pb_type->spice_model);
    cur_pb_type->spice_model = find_name_matched_spice_model(cur_pb_type->spice_model_name, num_verilog_model, verilog_models);
    if (NULL == cur_pb_type->spice_model) {
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d]) Fail to find a defined SPICE model called %s, in pb_type(%s)!\n",__FILE__, __LINE__, cur_pb_type->spice_model_name, cur_pb_type->name);
      exit(1);
    }
    return;
  }
  /* Traversal the hierarchy*/
  for (imode = 0; imode < cur_pb_type->num_modes; imode++) {
    /* Task 1: Find the interconnections and match the spice_model */
    for (jinterc = 0; jinterc < cur_pb_type->modes[imode].num_interconnect; jinterc++) {
      assert(NULL == cur_pb_type->modes[imode].interconnect[jinterc].spice_model);
      /* If the spice_model_name is not defined, we use the default*/
      if (NULL == cur_pb_type->modes[imode].interconnect[jinterc].spice_model_name) {
        switch (cur_pb_type->modes[imode].interconnect[jinterc].type) {
        case DIRECT_INTERC:
          cur_pb_type->modes[imode].interconnect[jinterc].spice_model = 
            get_default_spice_model(SPICE_MODEL_WIRE,num_verilog_model,verilog_models);
          break;
        case COMPLETE_INTERC:
          /* Special for Completer Interconnection:
           * 1. The input number is 1, this infers a direct interconnection.
           * 2. The input number is larger than 1, this infers multplexers
           * according to interconnect[j].num_mux identify the number of input at this level
           */
          if (0 == cur_pb_type->modes[imode].interconnect[jinterc].num_mux) {
            cur_pb_type->modes[imode].interconnect[jinterc].spice_model = 
              get_default_spice_model(SPICE_MODEL_WIRE,num_verilog_model,verilog_models);
          } else {
            cur_pb_type->modes[imode].interconnect[jinterc].spice_model = 
              get_default_spice_model(SPICE_MODEL_MUX,num_verilog_model,verilog_models);
          } 
          break;
        case MUX_INTERC:
          cur_pb_type->modes[imode].interconnect[jinterc].spice_model = 
            get_default_spice_model(SPICE_MODEL_MUX,num_verilog_model,verilog_models);
          break;
        default:
          break; 
        }        
      } else {
        cur_pb_type->modes[imode].interconnect[jinterc].spice_model = 
          find_name_matched_spice_model(cur_pb_type->modes[imode].interconnect[jinterc].spice_model_name, num_verilog_model, verilog_models);
        if (NULL == cur_pb_type->modes[imode].interconnect[jinterc].spice_model) {
          vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d]) Fail to find a defined SPICE model called %s, in pb_type(%s)!\n",__FILE__, __LINE__, cur_pb_type->modes[imode].interconnect[jinterc].spice_model_name, cur_pb_type->name);
          exit(1);
        } 
        switch (cur_pb_type->modes[imode].interconnect[jinterc].type) {
        case DIRECT_INTERC:
          if (SPICE_MODEL_WIRE != cur_pb_type->modes[imode].interconnect[jinterc].spice_model->type) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d]) Invalid type of matched SPICE model called %s, in pb_type(%s)! Sould be wire!\n",__FILE__, __LINE__, cur_pb_type->modes[imode].interconnect[jinterc].spice_model_name, cur_pb_type->name);
            exit(1);
          }
          break;
        case COMPLETE_INTERC:
          if (0 == cur_pb_type->modes[imode].interconnect[jinterc].num_mux) {
            if (SPICE_MODEL_WIRE != cur_pb_type->modes[imode].interconnect[jinterc].spice_model->type) {
              vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d]) Invalid type of matched SPICE model called %s, in pb_type(%s)! Sould be wire!\n",__FILE__, __LINE__, cur_pb_type->modes[imode].interconnect[jinterc].spice_model_name, cur_pb_type->name);
              exit(1);
            }
          } else {
            if (SPICE_MODEL_MUX != cur_pb_type->modes[imode].interconnect[jinterc].spice_model->type) {
              vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d]) Invalid type of matched SPICE model called %s, in pb_type(%s)! Sould be MUX!\n",__FILE__, __LINE__, cur_pb_type->modes[imode].interconnect[jinterc].spice_model_name, cur_pb_type->name);
              exit(1);
            }
          }
          break;
        case MUX_INTERC:
          if (SPICE_MODEL_MUX != cur_pb_type->modes[imode].interconnect[jinterc].spice_model->type) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d]) Invalid type of matched SPICE model called %s, in pb_type(%s)! Sould be MUX!\n",__FILE__, __LINE__, cur_pb_type->modes[imode].interconnect[jinterc].spice_model_name, cur_pb_type->name);
            exit(1);
          }
          break;
        default:
          break; 
        }        
      }
    }
    /* Task 2: Find the child pb_type, do matching recursively */
    //if (1 == cur_pb_type->modes[imode].define_spice_model) {
    for (ipb = 0; ipb < cur_pb_type->modes[imode].num_pb_type_children; ipb++) {
      match_pb_types_verilog_model_rec(&cur_pb_type->modes[imode].pb_type_children[ipb],
                                     num_verilog_model,
                                     verilog_models);
    }
    //}
  } 
  return;  
}

/* Find the interconnection type of pb_graph_pin edges*/
enum e_interconnect verilog_find_pb_graph_pin_in_edges_interc_type(t_pb_graph_pin pb_graph_pin) {
  enum e_interconnect interc_type;
  int def_interc_type = 0;
  int iedge;

  for (iedge = 0; iedge < pb_graph_pin.num_input_edges; iedge++) {
    /* Make sure all edges are legal: 1 input_pin, 1 output_pin*/
    check_pb_graph_edge(*(pb_graph_pin.input_edges[iedge]));
    /* Make sure all the edges interconnect type is the same*/
    if (0 == def_interc_type) {
      interc_type = pb_graph_pin.input_edges[iedge]->interconnect->type;
    } else if (interc_type != pb_graph_pin.input_edges[iedge]->interconnect->type) {
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d])Interconnection type are not same for port(%s),pin(%d).\n",
                 __FILE__, __LINE__, pb_graph_pin.port->name,pb_graph_pin.pin_number);
      exit(1);
    }
  }

  return interc_type;  
}


/* Find the interconnection type of pb_graph_pin edges*/
t_spice_model* find_pb_graph_pin_in_edges_interc_verilog_model(t_pb_graph_pin pb_graph_pin) {
  t_spice_model* interc_verilog_model;
  int def_interc_model = 0;
  int iedge;

  for (iedge = 0; iedge < pb_graph_pin.num_input_edges; iedge++) {
    /* Make sure all edges are legal: 1 input_pin, 1 output_pin*/
    check_pb_graph_edge(*(pb_graph_pin.input_edges[iedge]));
    /* Make sure all the edges interconnect type is the same*/
    if (0 == def_interc_model) {
      interc_verilog_model= pb_graph_pin.input_edges[iedge]->interconnect->spice_model;
    } else if (interc_verilog_model != pb_graph_pin.input_edges[iedge]->interconnect->spice_model) {
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d])Interconnection spice_model are not same for port(%s),pin(%d).\n",
                 __FILE__, __LINE__, pb_graph_pin.port->name,pb_graph_pin.pin_number);
      exit(1);
    }
  }

  return interc_verilog_model;  
}

/* Recursively do statistics for the
 * multiplexer verilog models inside pb_types
 */
void stats_mux_verilog_model_pb_type_rec(t_llist** muxes_head,
                                       t_pb_type* cur_pb_type) {
  
  int imode, ichild, jinterc;
  t_spice_model* interc_verilog_model = NULL;
 
  if (NULL == cur_pb_type) {
    vpr_printf(TIO_MESSAGE_WARNING,"(File:%s,LINE[%d])cur_pb_type is null pointor!\n",__FILE__,__LINE__);
    return;
  }

  /* If there is spice_model_name, this is a leaf node!*/
  if (NULL != cur_pb_type->spice_model_name) {
    /* What annoys me is VPR create a sub pb_type for each lut which suppose to be a leaf node
     * This may bring software convience but ruins SPICE modeling
     */
    assert(NULL != cur_pb_type->spice_model);
    return;
  }
  /* Traversal the hierarchy*/
  for (imode = 0; imode < cur_pb_type->num_modes; imode++) {
    /* Then we have to statisitic the interconnections*/
    for (jinterc = 0; jinterc < cur_pb_type->modes[imode].num_interconnect; jinterc++) {
      /* Check the num_mux and fan_in*/
      assert((0 == cur_pb_type->modes[imode].interconnect[jinterc].num_mux)
            ||(0 < cur_pb_type->modes[imode].interconnect[jinterc].num_mux));
      if (0 == cur_pb_type->modes[imode].interconnect[jinterc].num_mux) {
        continue;
      }
      interc_verilog_model = cur_pb_type->modes[imode].interconnect[jinterc].spice_model;
      assert(NULL != interc_verilog_model); 
      check_and_add_mux_to_linked_list(muxes_head,
                                       cur_pb_type->modes[imode].interconnect[jinterc].fan_in,
                                       interc_verilog_model);
    }
    for (ichild = 0; ichild < cur_pb_type->modes[imode].num_pb_type_children; ichild++) {
      stats_mux_verilog_model_pb_type_rec(muxes_head,
                                        &cur_pb_type->modes[imode].pb_type_children[ichild]);
    }
  }
  return;
}

/* Statistics the MUX SPICE MODEL with the help of pb_graph
 * Not the most efficient function to finish the job 
 * Abandon it. But remains a good framework that could be re-used in connecting
 * verilog components together
 */
void stats_mux_verilog_model_pb_node_rec(t_llist** muxes_head,
                                       t_pb_graph_node* cur_pb_node) {
  int imode, ipb, ichild, iport, ipin;
  t_pb_type* cur_pb_type = cur_pb_node->pb_type;
  t_spice_model* interc_verilog_model = NULL;
  enum e_interconnect pin_interc_type;
  
  if (NULL == cur_pb_node) {
    vpr_printf(TIO_MESSAGE_WARNING,"(File:%s,LINE[%d])cur_pb_node is null pointor!\n",__FILE__,__LINE__);
    return;
  }

  if (NULL == cur_pb_type) {
    vpr_printf(TIO_MESSAGE_WARNING,"(File:%s,LINE[%d])cur_pb_type is null pointor!\n",__FILE__,__LINE__);
    return;
  }

  /* If there is 0 mode, this is a leaf node!*/
  if (NULL != cur_pb_type->blif_model) {
    assert(0 == cur_pb_type->num_modes);
    assert(NULL == cur_pb_type->modes);
    /* Ensure there is blif_model, and spice_model*/
    assert(NULL != cur_pb_type->model);
    assert(NULL != cur_pb_type->spice_model_name);
    assert(NULL != cur_pb_type->spice_model);
    return;
  }
  /* Traversal the hierarchy*/
  for (imode = 0; imode < cur_pb_type->num_modes; imode++) {
    /* Then we have to statisitic the interconnections*/
    /* See the input ports*/
    for (iport = 0; iport < cur_pb_node->num_input_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_node->num_input_pins[iport]; ipin++) {
        /* Ensure this is an input port */
        assert(IN_PORT == cur_pb_node->input_pins[iport][ipin].port->type);
        /* See the edges, if the interconnetion type infer a MUX, we go next step*/
        pin_interc_type = verilog_find_pb_graph_pin_in_edges_interc_type(cur_pb_node->input_pins[iport][ipin]);
        if ((COMPLETE_INTERC != pin_interc_type)&&(MUX_INTERC != pin_interc_type)) {
          continue;
        }
        /* We shoule check the size of inputs, in some case of complete, the input_edge is one...*/
        if ((COMPLETE_INTERC == pin_interc_type)&&(1 == cur_pb_node->input_pins[iport][ipin].num_input_edges)) {
          continue;
        }
        /* Note: i do care the input_edges only! They may infer multiplexers*/
        interc_verilog_model = find_pb_graph_pin_in_edges_interc_verilog_model(cur_pb_node->input_pins[iport][ipin]);
        check_and_add_mux_to_linked_list(muxes_head,
                                         cur_pb_node->input_pins[iport][ipin].num_input_edges,
                                         interc_verilog_model);
      }
    }
    /* See the output ports*/
    for (iport = 0; iport < cur_pb_node->num_output_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_node->num_output_pins[iport]; ipin++) {
        /* Ensure this is an input port */
        assert(OUT_PORT == cur_pb_node->output_pins[iport][ipin].port->type);
        /* See the edges, if the interconnetion type infer a MUX, we go next step*/
        pin_interc_type = verilog_find_pb_graph_pin_in_edges_interc_type(cur_pb_node->output_pins[iport][ipin]);
        if ((COMPLETE_INTERC != pin_interc_type)&&(MUX_INTERC != pin_interc_type)) {
          continue;
        }
        /* We shoule check the size of inputs, in some case of complete, the input_edge is one...*/
        if ((COMPLETE_INTERC == pin_interc_type)&&(1 == cur_pb_node->output_pins[iport][ipin].num_input_edges)) {
          continue;
        }
        /* Note: i do care the input_edges only! They may infer multiplexers*/
        interc_verilog_model = find_pb_graph_pin_in_edges_interc_verilog_model(cur_pb_node->output_pins[iport][ipin]);
        check_and_add_mux_to_linked_list(muxes_head,
                                         cur_pb_node->output_pins[iport][ipin].num_input_edges,
                                         interc_verilog_model);
      }
    }
    /* See the clock ports*/
    for (iport = 0; iport < cur_pb_node->num_clock_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_node->num_clock_pins[iport]; ipin++) {
        /* Ensure this is an input port */
        assert(IN_PORT == cur_pb_node->clock_pins[iport][ipin].port->type);
        /* See the edges, if the interconnetion type infer a MUX, we go next step*/
        pin_interc_type = verilog_find_pb_graph_pin_in_edges_interc_type(cur_pb_node->clock_pins[iport][ipin]);
        if ((COMPLETE_INTERC != pin_interc_type)&&(MUX_INTERC != pin_interc_type)) {
          continue;
        }
        /* We shoule check the size of inputs, in some case of complete, the input_edge is one...*/
        if ((COMPLETE_INTERC == pin_interc_type)&&(1 == cur_pb_node->clock_pins[iport][ipin].num_input_edges)) {
          continue;
        }
        /* Note: i do care the input_edges only! They may infer multiplexers*/
        interc_verilog_model = find_pb_graph_pin_in_edges_interc_verilog_model(cur_pb_node->clock_pins[iport][ipin]);
        check_and_add_mux_to_linked_list(muxes_head,
                                         cur_pb_node->clock_pins[iport][ipin].num_input_edges,
                                         interc_verilog_model);
      }
    }
    for (ichild = 0; ichild < cur_pb_type->modes[imode].num_pb_type_children; ichild++) {
      /* num_pb is the number of such pb_type in a mode*/
      for (ipb = 0; ipb < cur_pb_type->modes[imode].pb_type_children[ichild].num_pb; ipb++) {
        /* child_pb_grpah_nodes: [0..num_modes-1][0..num_pb_type_in_mode-1][0..num_pb_type-1]*/
        stats_mux_verilog_model_pb_node_rec(muxes_head,
                                          &cur_pb_node->child_pb_graph_nodes[imode][ichild][ipb]);
      }
    }
  } 
  return;  
}

/* Print ports of pb_types,
 * SRAM ports are not printed here!!! 
 * Important feature: manage the comma between ports
 * Make sure there is no redundant comma and there is no comma after the last element if specified
 */
void dump_verilog_pb_type_bus_ports(FILE* fp,
                                    char* port_prefix,
                                    int use_global_clock,
                                    t_pb_type* cur_pb_type,
                                    boolean dump_port_type,
                                    boolean dump_last_comma) {
  int iport;
  int num_pb_type_input_port = 0;
  t_port** pb_type_input_ports = NULL;

  int num_pb_type_output_port = 0;
  t_port** pb_type_output_ports = NULL;

  int num_pb_type_inout_port = 0;
  t_port** pb_type_inout_ports = NULL;

  int num_pb_type_clk_port = 0;
  t_port** pb_type_clk_ports = NULL;

  char* formatted_port_prefix = chomp_verilog_prefix(port_prefix);
  /* A counter to stats the number of dumped ports and pins */
  int num_dumped_port = 0;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
  }

  /* INOUT ports */
  num_dumped_port = 0;
  /* Find pb_type inout ports */
  pb_type_inout_ports = find_pb_type_ports_match_spice_model_port_type(cur_pb_type, SPICE_MODEL_PORT_INOUT, &num_pb_type_inout_port); 

  /* Print all the inout ports  */
  for (iport = 0; iport < num_pb_type_inout_port; iport++) {
    if (0 < num_dumped_port) { 
      if (TRUE == dump_port_type) {
        fprintf(fp, ",\n");
      } else {
        fprintf(fp, ", ");
      }
    }
    if (TRUE == dump_port_type) {
      fprintf(fp, "inout ");
      fprintf(fp, "[0:%d] %s__%s ", 
                  pb_type_inout_ports[iport]->num_pins - 1,
                  formatted_port_prefix, pb_type_inout_ports[iport]->name);
    } else {
      fprintf(fp, "%s__%s[0:%d] ", 
                  formatted_port_prefix, pb_type_inout_ports[iport]->name, 
                  pb_type_inout_ports[iport]->num_pins - 1);
    }
    /* Update the counter */
    num_dumped_port++;
  }

  /* Inputs */
  /* Find pb_type input ports */
  pb_type_input_ports = find_pb_type_ports_match_spice_model_port_type(cur_pb_type, SPICE_MODEL_PORT_INPUT, &num_pb_type_input_port); 
  /* Print all the input ports  */
  for (iport = 0; iport < num_pb_type_input_port; iport++) {
    if (0 < num_dumped_port) { 
      if (TRUE == dump_port_type) {
        fprintf(fp, ",\n");
      } else {
        fprintf(fp, ", ");
      }
    }
    if (TRUE == dump_port_type) {
      fprintf(fp, "input ");
      fprintf(fp, " [0:%d] %s__%s ", 
                    pb_type_input_ports[iport]->num_pins - 1,
                    formatted_port_prefix, pb_type_input_ports[iport]->name);
    } else {
      fprintf(fp, " %s__%s[0:%d] ", 
                    formatted_port_prefix, pb_type_input_ports[iport]->name,
                    pb_type_input_ports[iport]->num_pins - 1);
    } 
    /* Update the counter */
    num_dumped_port++;
  }
  /* Outputs */
  /* Find pb_type output ports */
  pb_type_output_ports = find_pb_type_ports_match_spice_model_port_type(cur_pb_type, SPICE_MODEL_PORT_OUTPUT, &num_pb_type_output_port); 

  /* Print all the output ports  */
  for (iport = 0; iport < num_pb_type_output_port; iport++) {
    if (0 < num_dumped_port) { 
      if (TRUE == dump_port_type) {
        fprintf(fp, ",\n");
      } else {
        fprintf(fp, ", ");
      }
    }
    if (TRUE == dump_port_type) {
      fprintf(fp, "output ");
      fprintf(fp, " %s__%s[0:%d]", 
                  formatted_port_prefix, pb_type_output_ports[iport]->name,
                  pb_type_output_ports[iport]->num_pins - 1);
    } else {
       fprintf(fp, " %s__%s[0:%d]", 
                  formatted_port_prefix, pb_type_output_ports[iport]->name, 
                  pb_type_output_ports[iport]->num_pins - 1);
    }
    /* Update the counter */
    num_dumped_port++;
  }
  
  /* Clocks */
  /* Find pb_type clock ports */
  pb_type_clk_ports = find_pb_type_ports_match_spice_model_port_type(cur_pb_type, SPICE_MODEL_PORT_CLOCK, &num_pb_type_clk_port); 
  /* Print all the clk ports  */
  for (iport = 0; iport < num_pb_type_clk_port; iport++) {
    if (0 < num_dumped_port) { 
      if (TRUE == dump_port_type) {
        fprintf(fp, ",\n");
      } else {
        fprintf(fp, ", ");
      }
    }
    if (TRUE == dump_port_type) {
      fprintf(fp, "input");
      fprintf(fp, " %s__%s[0:%d]",
                  formatted_port_prefix, pb_type_clk_ports[iport]->name,
                  pb_type_output_ports[iport]->num_pins - 1);
    } else {
      fprintf(fp, " %s__%s[0:%d]",
                  formatted_port_prefix, pb_type_clk_ports[iport]->name,
                  pb_type_output_ports[iport]->num_pins - 1);
    }
    /* Update the counter */
    num_dumped_port++;
  }

  /* Dump the last comma, when the option is enabled and there is something dumped */
  if ((0 < num_dumped_port)&&(TRUE == dump_last_comma)) {
    if (TRUE == dump_port_type) {
      fprintf(fp, ",\n");
    } else {
      fprintf(fp, ",");
    }
  }

  /* Free */
  free(formatted_port_prefix);
  my_free(pb_type_input_ports);
  my_free(pb_type_output_ports);
  my_free(pb_type_inout_ports);
  my_free(pb_type_clk_ports);

  return;
}


/* Print ports of pb_types,
 * SRAM ports are not printed here!!! 
 * Important feature: manage the comma between ports
 * Make sure there is no redundant comma and there is no comma after the last element if specified
 * Check each ports, if it is defined as a global signal, we should not dump it
 */
void dump_verilog_pb_type_ports(FILE* fp,
                          char* port_prefix,
                          int use_global_clock,
                          t_pb_type* cur_pb_type,
                          boolean dump_port_type,
                          boolean dump_last_comma,
                          boolean require_explicit_port_map) {
  int iport, ipin;
  int num_pb_type_input_port = 0;
  t_port** pb_type_input_ports = NULL;

  int num_pb_type_output_port = 0;
  t_port** pb_type_output_ports = NULL;

  int num_pb_type_inout_port = 0;
  t_port** pb_type_inout_ports = NULL;

  int num_pb_type_clk_port = 0;
  t_port** pb_type_clk_ports = NULL;

  char* formatted_port_prefix = chomp_verilog_prefix(port_prefix);
  /* A counter to stats the number of dumped ports and pins */
  int num_dumped_port = 0;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
  }

  /* INOUT ports */
  num_dumped_port = 0;

  /* Find pb_type inout ports */
  pb_type_inout_ports = find_pb_type_ports_match_spice_model_port_type(cur_pb_type, SPICE_MODEL_PORT_INOUT, &num_pb_type_inout_port); 

  /* Print all the inout ports  */
  for (iport = 0; iport < num_pb_type_inout_port; iport++) {
    /* only search mapped ports for primitive node */
    if (NULL != cur_pb_type->spice_model) {
      /* We need to bypass global ports */
      assert(NULL != pb_type_inout_ports[iport]->spice_model_port);
      if (TRUE == pb_type_inout_ports[iport]->spice_model_port->is_global) {
        continue;
      }
    }
    /* Dump non-global ports */
    for (ipin = 0; ipin < pb_type_inout_ports[iport]->num_pins; ipin++) {
      if (0 < num_dumped_port) { 
        if (TRUE == dump_port_type) {
          fprintf(fp, ",\n");
        } else {
          fprintf(fp, ", ");
        }
      }
      if (TRUE == dump_port_type) {
        fprintf(fp, "inout wire ");
      } else if ((NULL != cur_pb_type->spice_model) 
              && (TRUE == require_explicit_port_map)
              && (TRUE == cur_pb_type->spice_model->dump_explicit_port_map)) {
        fprintf(fp, ".%s(", 
                pb_type_inout_ports[iport]->spice_model_port->lib_name);
      }
      fprintf(fp, "%s", 
              gen_verilog_one_pb_type_pin_name(formatted_port_prefix, pb_type_inout_ports[iport], ipin));
      if ((FALSE == dump_port_type) 
       && (NULL != cur_pb_type->spice_model) 
       && (TRUE == require_explicit_port_map)
       && (TRUE == cur_pb_type->spice_model->dump_explicit_port_map)) {
        fprintf(fp, ") ");
      }
      /* Update the counter */
      num_dumped_port++;
    }
  }

  /* Inputs */
  /* Find pb_type input ports */
  pb_type_input_ports = find_pb_type_ports_match_spice_model_port_type(cur_pb_type, SPICE_MODEL_PORT_INPUT, &num_pb_type_input_port); 
  /* Print all the input ports  */
  for (iport = 0; iport < num_pb_type_input_port; iport++) {
    /* only search mapped ports for primitive node */
    if (NULL != cur_pb_type->spice_model) {
      /* We need to bypass global ports */
      assert(NULL != pb_type_input_ports[iport]->spice_model_port);
      if (TRUE == pb_type_input_ports[iport]->spice_model_port->is_global) {
        continue;
      }
    }
    /* Dump non-global ports */
    for (ipin = 0; ipin < pb_type_input_ports[iport]->num_pins; ipin++) {
      if (0 < num_dumped_port) { 
        if (TRUE == dump_port_type) {
          fprintf(fp, ",\n");
        } else {
          fprintf(fp, ", ");
        }
      }
      if (TRUE == dump_port_type) {
        fprintf(fp, "input wire ");
      } else if ((NULL != cur_pb_type->spice_model) 
              && (TRUE == require_explicit_port_map)
              && (TRUE == cur_pb_type->spice_model->dump_explicit_port_map)) {
        fprintf(fp, ".%s(", 
                pb_type_input_ports[iport]->spice_model_port->lib_name);
      }
      fprintf(fp, "%s", 
              gen_verilog_one_pb_type_pin_name(formatted_port_prefix, pb_type_input_ports[iport], ipin));
      if ((FALSE == dump_port_type) 
       && (NULL != cur_pb_type->spice_model) 
       && (TRUE == require_explicit_port_map)
       && (TRUE == cur_pb_type->spice_model->dump_explicit_port_map)) {
        fprintf(fp, ") ");
      }
      /* Update the counter */
      num_dumped_port++;
    }
  }
  /* Outputs */
  /* Find pb_type output ports */
  pb_type_output_ports = find_pb_type_ports_match_spice_model_port_type(cur_pb_type, SPICE_MODEL_PORT_OUTPUT, &num_pb_type_output_port); 
  /* Print all the output ports  */
  for (iport = 0; iport < num_pb_type_output_port; iport++) {
    /* only search mapped ports for primitive node */
    if (NULL != cur_pb_type->spice_model) {
      /* We need to bypass global ports */
      assert(NULL != pb_type_output_ports[iport]->spice_model_port);
      if (TRUE == pb_type_output_ports[iport]->spice_model_port->is_global) {
        continue;
      }
    }
    /* Dump non-global ports */
    for (ipin = 0; ipin < pb_type_output_ports[iport]->num_pins; ipin++) {
      if (0 < num_dumped_port) { 
        if (TRUE == dump_port_type) {
          fprintf(fp, ",\n");
        } else {
          fprintf(fp, ", ");
        }
      }
      if (TRUE == dump_port_type) {
        fprintf(fp, "output wire ");
      } else if ((NULL != cur_pb_type->spice_model) 
              && (TRUE == require_explicit_port_map)
              && (TRUE == cur_pb_type->spice_model->dump_explicit_port_map)) {
        fprintf(fp, ".%s(", 
                pb_type_output_ports[iport]->spice_model_port->lib_name);
      }
      fprintf(fp, "%s", 
              gen_verilog_one_pb_type_pin_name(formatted_port_prefix, pb_type_output_ports[iport], ipin));
      if ((FALSE == dump_port_type) 
       && (NULL != cur_pb_type->spice_model) 
       && (TRUE == require_explicit_port_map)
       && (TRUE == cur_pb_type->spice_model->dump_explicit_port_map)) {
        fprintf(fp, ") ");
      }
      /* Update the counter */
      num_dumped_port++;
    }
  }
  
  /* Clocks */
  /* Find pb_type clock ports */
  /* Only dump clock ports when global clock is not specified */
  pb_type_clk_ports = find_pb_type_ports_match_spice_model_port_type(cur_pb_type, SPICE_MODEL_PORT_CLOCK, &num_pb_type_clk_port); 

  /* Print all the clk ports  */
  for (iport = 0; iport < num_pb_type_clk_port; iport++) {
    /* only search mapped ports for primitive node */
    if (NULL != cur_pb_type->spice_model) {
      /* We need to bypass global ports */
      assert(NULL != pb_type_clk_ports[iport]->spice_model_port);
      if (TRUE == pb_type_clk_ports[iport]->spice_model_port->is_global) {
        continue;
      }
    }
    /* Dump non-global ports */
    for (ipin = 0; ipin < pb_type_clk_ports[iport]->num_pins; ipin++) {
      if (0 < num_dumped_port) { 
        if (TRUE == dump_port_type) {
          fprintf(fp, ",\n");
        } else {
          fprintf(fp, ", ");
        }
      }
      if (TRUE == dump_port_type) {
        fprintf(fp, "input wire ");
      } else if ((NULL != cur_pb_type->spice_model) 
              && (TRUE == require_explicit_port_map)
              && (TRUE == cur_pb_type->spice_model->dump_explicit_port_map)) {
        fprintf(fp, ".%s(", 
                pb_type_clk_ports[iport]->spice_model_port->lib_name);
      }
      fprintf(fp, "%s", 
              gen_verilog_one_pb_type_pin_name(formatted_port_prefix, pb_type_clk_ports[iport], ipin));
      if ((FALSE == dump_port_type) 
       && (NULL != cur_pb_type->spice_model) 
       && (TRUE == require_explicit_port_map)
       && (TRUE == cur_pb_type->spice_model->dump_explicit_port_map)) {
        fprintf(fp, ") ");
      }
      /* Update the counter */
      num_dumped_port++;
    }
  }

  /* Dump the last comma, when the option is enabled and there is something dumped */
  if ((0 < num_dumped_port)&&(TRUE == dump_last_comma)) {
    if (TRUE == dump_port_type) {
      fprintf(fp, ",\n");
    } else {
      fprintf(fp, ",");
    }
  }

  /* Free */
  free(formatted_port_prefix);
  my_free(pb_type_input_ports);
  my_free(pb_type_output_ports);
  my_free(pb_type_inout_ports);
  my_free(pb_type_clk_ports);

  return;
}

/* This is a truncated version of generate_verilog_src_des_pb_graph_pin_prefix
 * Only used to generate prefix for those dangling pin in PB Types
 */
void dump_verilog_dangling_des_pb_graph_pin_interc(FILE* fp,
                                                   t_pb_graph_pin* des_pb_graph_pin,
                                                   t_mode* cur_mode,
                                                   enum e_spice_pin2pin_interc_type pin2pin_interc_type,
                                                   char* parent_pin_prefix) {
  t_pb_graph_node* des_pb_graph_node = NULL;
  t_pb_type* des_pb_type = NULL;
  int des_pb_type_index = -1;
  int fan_in = 0;
  t_interconnect* cur_interc = NULL;
  char* des_pin_prefix = NULL;
  
  /* char* formatted_parent_pin_prefix = format_verilog_node_prefix(parent_pin_prefix);*/  /* Complete a "_" at the end if needed*/
  //char* chomped_parent_pin_prefix = chomp_verilog_prefix(parent_pin_prefix); /* Remove a "_" at the end if needed*/

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check the pb_graph_nodes*/ 
  if (NULL == des_pb_graph_pin) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid pointer: des_pb_graph_pin.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  verilog_find_interc_fan_in_des_pb_graph_pin(des_pb_graph_pin, cur_mode, &cur_interc, &fan_in);
  if ((NULL != cur_interc)&&(0 != fan_in)) {
    return;
    /*
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Cur_interc not NULL & fan_in not zero!\n", 
               __FILE__, __LINE__); 
    exit(1);
    */
  }

  /* Initialize */
  des_pb_graph_node = des_pb_graph_pin->parent_node;
  des_pb_type = des_pb_graph_node->pb_type; 
  des_pb_type_index = des_pb_graph_node->placement_index;

  /* generate the pin prefix for src_pb_graph_node and des_pb_graph_node */
  switch (pin2pin_interc_type) {
  case INPUT2INPUT_INTERC:
    /* src_pb_graph_node.input_pins -----------------> des_pb_graph_node.input_pins 
     * des_pb_graph_node is a child of src_pb_graph_node 
     * parent_pin_prefix is the prefix from parent pb_graph_node, in this case, src_pb_graph_node 
     * src_pin_prefix: we need to handle the feedbacks, they comes from the same-level pb_graph_node 
     * src_pin_prefix = <formatted_parent_pin_prefix>
     * OR
     * src_pin_prefix = <formatted_parent_pin_prefix>_<src_pb_type>[<src_pb_type_index>]
     * des_pin_prefix = <formatted_parent_pin_prefix>mode[<mode_name>]_<des_pb_type>[<des_pb_type_index>]_
     */
    /*
    des_pin_prefix = (char*)my_malloc(sizeof(char)*
                        (strlen(formatted_parent_pin_prefix) + 5 + strlen(cur_mode->name)
                         + 2 + strlen(des_pb_type->name) + 1 + strlen(my_itoa(des_pb_type_index)) + 1 + 1));
    sprintf(des_pin_prefix, "%smode[%s]_%s[%d]",
            formatted_parent_pin_prefix, cur_mode->name, des_pb_type->name, des_pb_type_index);
    */
    /*Simplify the prefix, make the SPICE netlist readable*/
    des_pin_prefix = (char*)my_malloc(sizeof(char)*
                        (strlen(des_pb_type->name) + 1 + strlen(my_itoa(des_pb_type_index)) + 1 + 1));
    sprintf(des_pin_prefix, "%s_%d_",
             des_pb_type->name, des_pb_type_index);
    break;
  case OUTPUT2OUTPUT_INTERC:
    /* src_pb_graph_node.output_pins -----------------> des_pb_graph_node.output_pins 
     * src_pb_graph_node is a child of des_pb_graph_node 
     * parent_pin_prefix is the prefix from parent pb_graph_node, in this case, des_pb_graph_node 
     * src_pin_prefix = <formatted_parent_pin_prefix>mode[<mode_name>]_<src_pb_type>[<src_pb_type_index>]_
     * des_pin_prefix = <formatted_parent_pin_prefix>
     */
    /*
    des_pin_prefix = (char*)my_malloc(sizeof(char)*
                         (strlen(formatted_parent_pin_prefix) + 5 + strlen(cur_mode->name)
                         + 2 + strlen(des_pb_type->name) + 1 + strlen(my_itoa(des_pb_type_index)) + 1 + 1));
    sprintf(des_pin_prefix, "%smode[%s]_%s[%d]",
            formatted_parent_pin_prefix, cur_mode->name, des_pb_type->name, des_pb_type_index);
    */
    if (des_pb_type == cur_mode->parent_pb_type) { /* Interconnection from parent pb_type*/
      des_pin_prefix = (char*)my_malloc(sizeof(char)*
                           (strlen(cur_mode->name) + 1 + 1 ));
      sprintf(des_pin_prefix, "%s_", cur_mode->name);
    } else {
      des_pin_prefix = (char*)my_malloc(sizeof(char)*
                          (strlen(des_pb_type->name) + 1 + strlen(my_itoa(des_pb_type_index)) + 1 + 1));
      sprintf(des_pin_prefix, "%s_%d_",
               des_pb_type->name, des_pb_type_index);
    }
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s [LINE%d])Invalid pin to pin interconnection type!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  my_free(des_pin_prefix);

  return;
}

void generate_verilog_src_des_pb_graph_pin_prefix(t_pb_graph_pin* src_pb_graph_pin,
                                                  t_pb_graph_pin* des_pb_graph_pin,
                                                  enum e_spice_pin2pin_interc_type pin2pin_interc_type,
                                                  t_interconnect* pin2pin_interc,
                                                  char* parent_pin_prefix,
                                                  char** src_pin_prefix,
                                                  char** des_pin_prefix) {
  t_pb_type* src_pb_type = NULL;
  int src_pb_type_index = -1;

  t_pb_type* des_pb_type = NULL;
  int des_pb_type_index = -1;
  
  /* Check the pb_graph_nodes*/ 
  if (NULL == src_pb_graph_pin) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid pointer: src_pb_graph_pin.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  if (NULL == des_pb_graph_pin) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid pointer: des_pb_graph_pin.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  if (NULL == pin2pin_interc) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid pointer: pin2pin_interc.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Initialize */
  src_pb_type = src_pb_graph_pin->parent_node->pb_type; 
  src_pb_type_index = src_pb_graph_pin->parent_node->placement_index;
  des_pb_type = des_pb_graph_pin->parent_node->pb_type; 
  des_pb_type_index = des_pb_graph_pin->parent_node->placement_index;
  
  assert(NULL == (*src_pin_prefix));
  assert(NULL == (*des_pin_prefix));
  /* generate the pin prefix for src_pb_graph_node and des_pb_graph_node */
  switch (pin2pin_interc_type) {
  case INPUT2INPUT_INTERC:
    /* src_pb_graph_node.input_pins -----------------> des_pb_graph_node.input_pins 
     * des_pb_graph_node is a child of src_pb_graph_node 
     * parent_pin_prefix is the prefix from parent pb_graph_node, in this case, src_pb_graph_node 
     * src_pin_prefix: we need to handle the feedbacks, they comes from the same-level pb_graph_node 
     * src_pin_prefix = <formatted_parent_pin_prefix>
     * OR
     * src_pin_prefix = <formatted_parent_pin_prefix>_<src_pb_type>[<src_pb_type_index>]
     * des_pin_prefix = <formatted_parent_pin_prefix>mode[<mode_name>]_<des_pb_type>[<des_pb_type_index>]_
     */
    if (src_pb_type == des_pb_type->parent_mode->parent_pb_type) {  /* Interconnection from parent pb_type*/
      /*
      (*src_pin_prefix) = my_strdup(chomped_parent_pin_prefix);
      */
      /*Simplify the prefix, make the SPICE netlist readable*/
      (*src_pin_prefix) = (char*)my_malloc(sizeof(char)*
                           (strlen(des_pb_type->parent_mode->parent_pb_type->name) + 1 + 1));
      sprintf((*src_pin_prefix), "%s_", des_pb_type->parent_mode->parent_pb_type->name);
    } else {
      /*
      (*src_pin_prefix) = (char*)my_malloc(sizeof(char)*
                           (strlen(formatted_parent_pin_prefix) + 5 + strlen(pin2pin_interc_parent_mode->name)
                           + 2 + strlen(src_pb_type->name) + 1 + strlen(my_itoa(src_pb_type_index)) + 1 + 1));
      sprintf((*src_pin_prefix), "%smode[%s]_%s[%d]",
              formatted_parent_pin_prefix, pin2pin_interc_parent_mode->name, src_pb_type->name, src_pb_type_index);
       */
      /*Simplify the prefix, make the SPICE netlist readable*/
      (*src_pin_prefix) = (char*)my_malloc(sizeof(char)*
                           (strlen(src_pb_type->name) + 1 + strlen(my_itoa(src_pb_type_index)) + 1 + 1));
      sprintf((*src_pin_prefix), "%s_%d_",
              src_pb_type->name, src_pb_type_index);
    }
    /*
    (*des_pin_prefix) = (char*)my_malloc(sizeof(char)*
                        (strlen(formatted_parent_pin_prefix) + 5 + strlen(pin2pin_interc_parent_mode->name)
                         + 2 + strlen(des_pb_type->name) + 1 + strlen(my_itoa(des_pb_type_index)) + 1 + 1));
    sprintf((*des_pin_prefix), "%smode[%s]_%s[%d]",
            formatted_parent_pin_prefix, pin2pin_interc_parent_mode->name, des_pb_type->name, des_pb_type_index);
    */
    /*Simplify the prefix, make the SPICE netlist readable*/
    (*des_pin_prefix) = (char*)my_malloc(sizeof(char)*
                        (strlen(des_pb_type->name) + 1 + strlen(my_itoa(des_pb_type_index)) + 1 + 1));
    sprintf((*des_pin_prefix), "%s_%d_",
             des_pb_type->name, des_pb_type_index);
    break;
  case OUTPUT2OUTPUT_INTERC:
    /* src_pb_graph_node.output_pins -----------------> des_pb_graph_node.output_pins 
     * src_pb_graph_node is a child of des_pb_graph_node 
     * parent_pin_prefix is the prefix from parent pb_graph_node, in this case, des_pb_graph_node 
     * src_pin_prefix = <formatted_parent_pin_prefix>mode[<mode_name>]_<src_pb_type>[<src_pb_type_index>]_
     * des_pin_prefix = <formatted_parent_pin_prefix>
     */
    /*
    (*src_pin_prefix) = (char*)my_malloc(sizeof(char)*
                        (strlen(formatted_parent_pin_prefix) + 5 + strlen(pin2pin_interc_parent_mode->name)
                         + 2 + strlen(src_pb_type->name) + 1 + strlen(my_itoa(src_pb_type_index)) + 1 + 1));
    sprintf((*src_pin_prefix), "%smode[%s]_%s[%d]",
            formatted_parent_pin_prefix, pin2pin_interc_parent_mode->name, src_pb_type->name, src_pb_type_index);
    */
    /*Simplify the prefix, make the SPICE netlist readable*/
    if (des_pb_type == src_pb_type) { /* src pin is an input of  parent pb_type*/
      /*
      (*des_pin_prefix) = my_strdup(chomped_parent_pin_prefix);
      */
      /*Simplify the prefix, make the SPICE netlist readable*/
      (*src_pin_prefix) = (char*)my_malloc(sizeof(char)*
                           (strlen(pin2pin_interc->parent_mode->parent_pb_type->name) + 1 + 1));
      sprintf((*src_pin_prefix), "%s_", pin2pin_interc->parent_mode->parent_pb_type->name);
    } else {
      (*src_pin_prefix) = (char*)my_malloc(sizeof(char)*
                          (strlen(src_pb_type->name) + 1 + strlen(my_itoa(src_pb_type_index)) + 1 + 1));
      sprintf((*src_pin_prefix), "%s_%d_",
              src_pb_type->name, src_pb_type_index);
    }
    /*
    (*des_pin_prefix) = my_strdup(chomped_parent_pin_prefix);
    */
    /*Simplify the prefix, make the SPICE netlist readable*/
    (*des_pin_prefix) = (char*)my_malloc(sizeof(char)*
                         (strlen(pin2pin_interc->parent_mode->parent_pb_type->name) + 1 + 1));
    sprintf((*des_pin_prefix), "%s_", pin2pin_interc->parent_mode->parent_pb_type->name);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s [LINE%d])Invalid pin to pin interconnection type!\n",
               __FILE__, __LINE__);
    exit(1);
  }
  return;
}

void verilog_find_interc_fan_in_des_pb_graph_pin(t_pb_graph_pin* des_pb_graph_pin,
                                         t_mode* cur_mode,
                                         t_interconnect** cur_interc,
                                         int* fan_in) { 
  int iedge;
  
  (*cur_interc) = NULL;
  (*fan_in) = 0;  

  /* Search the input edges only, stats on the size of MUX we may need (fan-in) */
  for (iedge = 0; iedge < des_pb_graph_pin->num_input_edges; iedge++) {
    /* 1. First, we should make sure this interconnect is in the selected mode!!!*/
    if (cur_mode == des_pb_graph_pin->input_edges[iedge]->interconnect->parent_mode) {
      /* Check this edge*/
      check_pb_graph_edge(*(des_pb_graph_pin->input_edges[iedge]));
      /* Record the interconnection*/
      if (NULL == (*cur_interc)) {
        (*cur_interc) = des_pb_graph_pin->input_edges[iedge]->interconnect;
      } else { /* Make sure the interconnections for this pin is the same!*/
        assert((*cur_interc) == des_pb_graph_pin->input_edges[iedge]->interconnect);
      }
      /* Search the input_pins of input_edges only*/
      (*fan_in) += des_pb_graph_pin->input_edges[iedge]->num_input_pins;
    }
  }

  return;
}

/* We check output_pins of cur_pb_graph_node and its the input_edges
 * Built the interconnections between outputs of cur_pb_graph_node and outputs of child_pb_graph_node
 *   src_pb_graph_node.[in|out]_pins -----------------> des_pb_graph_node.[in|out]pins
 *                                        /|\
 *                                         |
 *                         input_pins,   edges,       output_pins
 */ 
void dump_verilog_pb_graph_pin_interc(t_sram_orgz_info* cur_sram_orgz_info,
                                      FILE* fp,
                                      char* parent_pin_prefix,
                                      enum e_spice_pin2pin_interc_type pin2pin_interc_type,
                                      t_pb_graph_pin* des_pb_graph_pin,
                                      t_mode* cur_mode) {
  int i, iedge, ipin;
  int fan_in = 0;
  t_interconnect* cur_interc = NULL; 
  enum e_interconnect verilog_interc_type = DIRECT_INTERC;

  t_pb_graph_pin* src_pb_graph_pin = NULL;
  t_pb_graph_node* src_pb_graph_node = NULL;
  t_pb_type* src_pb_type = NULL;

  t_pb_graph_node* des_pb_graph_node = NULL;

  char* formatted_parent_pin_prefix = chomp_verilog_prefix(parent_pin_prefix); /* Complete a "_" at the end if needed*/
  char* src_pin_prefix = NULL;
  char* des_pin_prefix = NULL;

  int num_mux_sram_bits = 0;
  int* mux_sram_bits = NULL;
  int cur_num_sram = 0;
  int mux_level = 0;
  int num_mux_conf_bits = 0;
  int num_mux_reserved_conf_bits = 0;
  int cur_bl, cur_wl;
  t_spice_model* mem_model = NULL;
  char* mem_subckt_name = NULL;
  char* hierarchical_name = NULL;
  char* mux_name = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* 1. identify pin interconnection type, 
   * 2. Identify the number of fan-in (Consider interconnection edges of only selected mode)
   * 3. Select and print the SPICE netlist
   */
  fan_in = 0;
  cur_interc = NULL;
  find_interc_fan_in_des_pb_graph_pin(des_pb_graph_pin, cur_mode, &cur_interc, &fan_in);
  if ((NULL == cur_interc)||(0 == fan_in)) { 
    /* No interconnection matched */
    /* Connect this pin to GND for better convergence */
    /* TODO: find the correct pin name!!!*/
    /*
    dump_verilog_dangling_des_pb_graph_pin_interc(fp, des_pb_graph_pin, cur_mode, pin2pin_interc_type,
                                                  formatted_parent_pin_prefix);
    */
    return;
  }
  /* Initialize the interconnection type that will be implemented in SPICE netlist*/
  verilog_interc_type = determine_actual_pb_interc_type(cur_interc, fan_in);
  /* This time, (2nd round), we print the subckt, according to interc type*/ 
  switch (verilog_interc_type) {
  case DIRECT_INTERC:
    /* Check : 
     * 1. Direct interc has only one fan-in!
     */
    assert(1 == fan_in);
    //assert(1 == des_pb_graph_pin->num_input_edges);
    /* For more than one mode defined, the direct interc has more than one input_edge ,
     * We need to find which edge is connected the pin we want
     */
    for (iedge = 0; iedge < des_pb_graph_pin->num_input_edges; iedge++) {
      if (cur_interc == des_pb_graph_pin->input_edges[iedge]->interconnect) {
        break;
      }
    }
    assert(iedge < des_pb_graph_pin->num_input_edges);
    /* 2. spice_model is a wire */ 
    assert(NULL != cur_interc->spice_model);
    assert(SPICE_MODEL_WIRE == cur_interc->spice_model->type);
    assert(NULL != cur_interc->spice_model->wire_param);
    /* Initialize*/
    /* Source pin, node, pb_type*/
    src_pb_graph_pin = des_pb_graph_pin->input_edges[iedge]->input_pins[0];
    src_pb_graph_node = src_pb_graph_pin->parent_node;
    src_pb_type = src_pb_graph_node->pb_type;
    /* Des pin, node, pb_type */
    des_pb_graph_node  = des_pb_graph_pin->parent_node;
    /* Generate the pin_prefix for src_pb_graph_node and des_pb_graph_node*/
    generate_verilog_src_des_pb_graph_pin_prefix(src_pb_graph_pin, des_pb_graph_pin, pin2pin_interc_type, 
                                                 cur_interc, formatted_parent_pin_prefix, &src_pin_prefix, &des_pin_prefix);
    src_pin_prefix = chomp_verilog_prefix(src_pin_prefix);
    des_pin_prefix = chomp_verilog_prefix(des_pin_prefix);
    /* Call the subckt that has already been defined before */
    fprintf(fp, "%s ", cur_interc->spice_model->name);
    fprintf(fp, "%s_%d_ (", cur_interc->spice_model->prefix, cur_interc->spice_model->cnt); 
    cur_interc->spice_model->cnt++; /* Stats the number of spice_model used*/
    /* Dump global ports */
    if  (0 < rec_dump_verilog_spice_model_global_ports(fp, cur_interc->spice_model, FALSE, FALSE, FALSE)) {
      fprintf(fp, ",\n");
    }
    /* Print the pin names! Input and output
     * Input: port_prefix_<child_pb_graph_node>-><port_name>[pin_index]
     * Output: port_prefix_<port_name>[pin_index]
     */
    /* Input */
    /* Make sure correctness*/
    assert(src_pb_type == des_pb_graph_pin->input_edges[iedge]->input_pins[0]->port->parent_pb_type);
    /* Print */
    fprintf(fp, "%s__%s_%d_, ", 
            src_pin_prefix, src_pb_graph_pin->port->name, src_pb_graph_pin->pin_number);
    /* Output */
    fprintf(fp, "%s__%s_%d_ ", 
            des_pin_prefix, des_pb_graph_pin->port->name, des_pb_graph_pin->pin_number); 
    /* Middle output for wires in logic blocks: TODO: Abolish to save simulation time */
    /* fprintf(fp, "gidle_mid_out "); */
    /* Local vdd and gnd, TODO: we should have an independent VDD for all local interconnections*/
    fprintf(fp, ");\n");
    /* Free */
    my_free(src_pin_prefix);
    my_free(des_pin_prefix);
    src_pin_prefix = NULL;
    des_pin_prefix = NULL;
    break;
  case COMPLETE_INTERC:
  case MUX_INTERC:
    /* Check : 
     * MUX should have at least 2 fan_in
     */
    assert((2 == fan_in)||(2 < fan_in));
    /* 2. spice_model is a wire */ 
    assert(NULL != cur_interc->spice_model);
    assert(SPICE_MODEL_MUX == cur_interc->spice_model->type);
    /* Create a local bus */
    fprintf(fp, "wire [0:%d] in_bus_%s_size%d_%d_ ;\n", fan_in - 1, 
            cur_interc->spice_model->name, fan_in, cur_interc->spice_model->cnt);

      /* Generation of the name for the SDC */

      // It is actually better to generate the hierarchical name with the pin because otherwise we have issues with the path and miss some of them.
      //hierarchical_name = gen_verilog_one_pb_graph_pin_full_name_in_hierarchy_parent_node(des_pb_graph_pin);
      // testing without the hierarchy because this functions includes the current nodes sometimes whereas there should be nothing about certain muxes
      hierarchical_name = "";
    
      mux_name = (char *) my_malloc(sizeof(char)*(strlen(cur_interc->spice_model->name)
                     + 5 + strlen(my_itoa(fan_in)) + 1 + strlen(my_itoa(cur_interc->spice_model->cnt + 2))));
	  sprintf(mux_name, "%s_size%d_%d_", 
              cur_interc->spice_model->name, fan_in, cur_interc->spice_model->cnt);
      
      des_pb_graph_pin->name_mux = my_strcat(hierarchical_name,mux_name);
      free(mux_name);

    ipin = 0;
    for (iedge = 0; iedge < des_pb_graph_pin->num_input_edges; iedge++) {
      if (cur_mode != des_pb_graph_pin->input_edges[iedge]->interconnect->parent_mode) {
        continue;
      }
      check_pb_graph_edge(*(des_pb_graph_pin->input_edges[iedge]));
      /* Initialize*/
      /* Source pin, node, pb_type*/
      src_pb_graph_pin = des_pb_graph_pin->input_edges[iedge]->input_pins[0];
      src_pb_graph_node = src_pb_graph_pin->parent_node;
      src_pb_type = src_pb_graph_node->pb_type;

      /* Des pin, node, pb_type */
      des_pb_graph_node  = des_pb_graph_pin->parent_node;
      /* Generate the pin_prefix for src_pb_graph_node and des_pb_graph_node*/
      generate_verilog_src_des_pb_graph_pin_prefix(src_pb_graph_pin, des_pb_graph_pin, pin2pin_interc_type, 
                                                   cur_interc, formatted_parent_pin_prefix, &src_pin_prefix, &des_pin_prefix);
      src_pin_prefix = chomp_verilog_prefix(src_pin_prefix);
      
      /* We need to find out if the des_pb_graph_pin is in the mode we want !*/
      /* Print */
      fprintf(fp, "assign in_bus_%s_size%d_%d_[%d] = ",
            cur_interc->spice_model->name, fan_in, cur_interc->spice_model->cnt, ipin);
      fprintf(fp, "%s__%s_%d_ ; \n", 
              src_pin_prefix, src_pb_graph_pin->port->name, src_pb_graph_pin->pin_number);
      /* Baudouin Chauviere: SDC Generation */
      des_pb_graph_pin->input_edges[iedge]->nb_mux = cur_interc->spice_model->cnt;
      des_pb_graph_pin->input_edges[iedge]->nb_pin = ipin;
      /* END */
      

      ipin++;
      /* Free */
      my_free(des_pin_prefix);
      src_pin_prefix = NULL;
      des_pin_prefix = NULL;
    }
    assert(ipin == fan_in);
    /* Print SRAMs that configure this MUX */
    /* cur_num_sram = sram_verilog_model->cnt; */
    cur_num_sram = get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info); 
    get_sram_orgz_info_num_blwl(cur_sram_orgz_info, &cur_bl, &cur_wl);
    /* connect to reserved BL/WLs ? */
    num_mux_reserved_conf_bits = count_num_reserved_conf_bits_one_spice_model(cur_interc->spice_model, 
                                                                              cur_sram_orgz_info->type, 
                                                                              fan_in);
    /* Get the number of configuration bits required by this MUX */
    num_mux_conf_bits = count_num_conf_bits_one_spice_model(cur_interc->spice_model, 
                                                            cur_sram_orgz_info->type, 
                                                            fan_in);

    /* Get the number of SRAM bits required by this MUX */
    num_mux_sram_bits = count_num_sram_bits_one_spice_model(cur_interc->spice_model, 
                                                            fan_in);

    /* Dump the configuration port bus */
    dump_verilog_mux_config_bus(fp, cur_interc->spice_model, cur_sram_orgz_info,
                                fan_in, cur_num_sram, num_mux_reserved_conf_bits, num_mux_conf_bits); 
    /* Dump ports only visible during formal verification*/
    if (0 < num_mux_conf_bits) {
      fprintf(fp, "`ifdef %s\n", verilog_formal_verification_preproc_flag);
      dump_verilog_formal_verification_mux_sram_ports_wiring(fp, cur_sram_orgz_info, 
                                                             cur_interc->spice_model, fan_in,
                                                             cur_num_sram,
                                                             cur_num_sram + num_mux_conf_bits - 1);
      fprintf(fp, "\n");
      fprintf(fp, "`endif\n");
    }
    
    /* Call the subckt that has already been defined before */
    fprintf(fp, "%s_size%d ", cur_interc->spice_model->name, fan_in);
    fprintf(fp, "%s_size%d_%d_ (", cur_interc->spice_model->prefix, fan_in, cur_interc->spice_model->cnt);
    /* Dump global ports */
    if  (0 < rec_dump_verilog_spice_model_global_ports(fp, cur_interc->spice_model, FALSE, FALSE, FALSE)) {
      fprintf(fp, ",\n");
    }
    /* Inputs */
    fprintf(fp, "in_bus_%s_size%d_%d_, ",
            cur_interc->spice_model->name, fan_in, cur_interc->spice_model->cnt);
    /* Generate the pin_prefix for src_pb_graph_node and des_pb_graph_node*/
    generate_verilog_src_des_pb_graph_pin_prefix(src_pb_graph_pin, des_pb_graph_pin, pin2pin_interc_type, 
                                               cur_interc, formatted_parent_pin_prefix, &src_pin_prefix, &des_pin_prefix);
    des_pin_prefix = chomp_verilog_prefix(des_pin_prefix);
    /* Outputs */
    fprintf(fp, "%s__%s_%d_, ", 
            des_pin_prefix, des_pb_graph_pin->port->name, des_pb_graph_pin->pin_number);

    /* Different design technology requires different configuration bus! */
    dump_verilog_mux_config_bus_ports(fp, cur_interc->spice_model, cur_sram_orgz_info,
                                      fan_in, cur_num_sram, num_mux_reserved_conf_bits, num_mux_conf_bits);
  
    fprintf(fp, ");\n");

    get_sram_orgz_info_mem_model(cur_sram_orgz_info, &mem_model);
    /* Dump sram modules */
    switch (cur_interc->spice_model->design_tech) {
    case SPICE_MODEL_DESIGN_CMOS:
      /* Call the memory module defined for this SRAM-based MUX! */
      mem_subckt_name = generate_verilog_mux_subckt_name(cur_interc->spice_model, fan_in, verilog_mem_posfix);
      fprintf(fp, "%s %s_%d_ ( ", 
              mem_subckt_name, mem_subckt_name, cur_interc->spice_model->cnt);
      dump_verilog_mem_sram_submodule(fp, cur_sram_orgz_info, cur_interc->spice_model, fan_in,
                                      mem_model, cur_num_sram, cur_num_sram + num_mux_conf_bits - 1); 
      fprintf(fp, ");\n");
      /* update the number of memory bits */
      update_sram_orgz_info_num_mem_bit(cur_sram_orgz_info, cur_num_sram + num_mux_conf_bits);
      break;
    case SPICE_MODEL_DESIGN_RRAM:
      /* RRAM-based MUX does not need any SRAM dumping
       * But we have to get the number of configuration bits required by this MUX 
       * and update the number of memory bits 
       */
      update_sram_orgz_info_num_mem_bit(cur_sram_orgz_info, cur_num_sram + num_mux_conf_bits);
      update_sram_orgz_info_num_blwl(cur_sram_orgz_info, 
                                     cur_bl + num_mux_conf_bits, 
                                     cur_wl + num_mux_conf_bits);
      break;  
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid design technology for verilog model (%s)!\n",
                 __FILE__, __LINE__, cur_interc->spice_model->name);
    }
  
      /* update sram counter */
    cur_interc->spice_model->cnt++;

    /* Free */
    my_free(mux_sram_bits);
    my_free(src_pin_prefix);
    my_free(des_pin_prefix);
    my_free(mem_subckt_name);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid interconnection type for %s (Arch[LINE%d])!\n",
               __FILE__, __LINE__, cur_interc->name, cur_interc->line_num);
    exit(1);
  }

  return;
}

/* Print the SPICE interconnections of a port defined in pb_graph */
void dump_verilog_pb_graph_port_interc(t_sram_orgz_info* cur_sram_orgz_info,
                                       FILE* fp,
                                       char* formatted_pin_prefix,
                                       t_pb_graph_node* cur_pb_graph_node,
                                       enum e_spice_pb_port_type pb_port_type,
                                       t_mode* cur_mode) {
  int iport, ipin;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  switch (pb_port_type) {
  case SPICE_PB_PORT_INPUT:
    for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_input_pins[iport]; ipin++) {
        /* If this is a idle block, we set 0 to the selected edge*/
        /* Get the selected edge of current pin*/
        dump_verilog_pb_graph_pin_interc(cur_sram_orgz_info,
                                         fp, 
                                         formatted_pin_prefix, /* parent_pin_prefix */
                                         INPUT2INPUT_INTERC,
                                         &(cur_pb_graph_node->input_pins[iport][ipin]),
                                         cur_mode);
      }
    }
    break;
  case SPICE_PB_PORT_OUTPUT:
    for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_output_pins[iport]; ipin++) {
        dump_verilog_pb_graph_pin_interc(cur_sram_orgz_info,
                                         fp, 
                                         formatted_pin_prefix, /* parent_pin_prefix */
                                         OUTPUT2OUTPUT_INTERC,
                                         &(cur_pb_graph_node->output_pins[iport][ipin]),
                                         cur_mode);
      }
    }
    break;
  case SPICE_PB_PORT_CLOCK:
    for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
      for (ipin = 0; ipin < cur_pb_graph_node->num_clock_pins[iport]; ipin++) {
        dump_verilog_pb_graph_pin_interc(cur_sram_orgz_info,
                                         fp, 
                                         formatted_pin_prefix, /* parent_pin_prefix */
                                         INPUT2INPUT_INTERC,
                                         &(cur_pb_graph_node->clock_pins[iport][ipin]),
                                         cur_mode);
      }
    }
    break;
  default:
   vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid pb port type!\n",
               __FILE__, __LINE__);
    exit(1);
  }

  return;
}


/* Print the SPICE interconnections according to pb_graph */
void dump_verilog_pb_graph_interc(t_sram_orgz_info* cur_sram_orgz_info,
                                  FILE* fp, 
                                  char* pin_prefix,
                                  t_pb_graph_node* cur_pb_graph_node,
                                  int select_mode_index) {
  int ipb, jpb;
  t_mode* cur_mode = NULL;
  t_pb_type* cur_pb_type = cur_pb_graph_node->pb_type;
  t_pb_graph_node* child_pb_graph_node = NULL;
  t_pb* child_pb = NULL;
  int is_child_pb_idle = 0;
  
  char* formatted_pin_prefix = format_verilog_node_prefix(pin_prefix); /* Complete a "_" at the end if needed*/

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check cur_pb_type*/
  if (NULL == cur_pb_type) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid cur_pb_type.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  
  /* Assign current mode */
  cur_mode = &(cur_pb_graph_node->pb_type->modes[select_mode_index]);

  /* We check output_pins of cur_pb_graph_node and its the input_edges
   * Built the interconnections between outputs of cur_pb_graph_node and outputs of child_pb_graph_node
   *   child_pb_graph_node.output_pins -----------------> cur_pb_graph_node.outpins
   *                                        /|\
   *                                         |
   *                         input_pins,   edges,       output_pins
   */ 
  dump_verilog_pb_graph_port_interc(cur_sram_orgz_info, fp, formatted_pin_prefix,
                                    cur_pb_graph_node, 
                                    SPICE_PB_PORT_OUTPUT,
                                    cur_mode);
  
  /* We check input_pins of child_pb_graph_node and its the input_edges
   * Built the interconnections between inputs of cur_pb_graph_node and inputs of child_pb_graph_node
   *   cur_pb_graph_node.input_pins -----------------> child_pb_graph_node.input_pins
   *                                        /|\
   *                                         |
   *                         input_pins,   edges,       output_pins
   */ 
  for (ipb = 0; ipb < cur_pb_type->modes[select_mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb_type->modes[select_mode_index].pb_type_children[ipb].num_pb; jpb++) {
      child_pb_graph_node = &(cur_pb_graph_node->child_pb_graph_nodes[select_mode_index][ipb][jpb]);
      /* For each child_pb_graph_node input pins*/
      dump_verilog_pb_graph_port_interc(cur_sram_orgz_info, fp, formatted_pin_prefix,
                                         child_pb_graph_node, 
                                         SPICE_PB_PORT_INPUT,
                                         cur_mode);
      /* TODO: for clock pins, we should do the same work */
      dump_verilog_pb_graph_port_interc(cur_sram_orgz_info, fp, formatted_pin_prefix,
                                        child_pb_graph_node, 
                                        SPICE_PB_PORT_CLOCK,
                                        cur_mode);
 
    }
  }

  return; 
}

/* Print the netlist for primitive pb_types*/
void dump_verilog_pb_graph_primitive_node(FILE* fp,
                                          char* subckt_prefix,
                                          t_pb* cur_pb,
                                          t_pb_graph_node* cur_pb_graph_node,
                                          int pb_type_index) {
  t_pb_type* cur_pb_type = NULL;
  char* formatted_subckt_prefix = format_verilog_node_prefix(subckt_prefix); /* Complete a "_" at the end if needed*/
  t_spice_model* verilog_model = NULL;
  char* subckt_name = NULL;
  
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check cur_pb_graph_node*/
  if (NULL == cur_pb_graph_node) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid cur_pb_graph_node.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  verilog_model = cur_pb_type->spice_model;
  /* If we define a SPICE model for the pb_type,
   * We should print the subckt of it. 
   * 1. If the SPICE model defines an included netlist, we quote the netlist subckt 
   * 2. If not defined an included netlist, we built one only if this is a LUT
   */
  /* Example: <prefix><cur_pb_type_name>[<index>]*/
  subckt_name = (char*)my_malloc(sizeof(char)*
                (strlen(formatted_subckt_prefix) 
                + strlen(cur_pb_type->name) + 1
                + strlen(my_itoa(pb_type_index)) + 1 + 1)); /* Plus the '0' at the end of string*/
  sprintf(subckt_name, "%s%s_%d_", formatted_subckt_prefix, cur_pb_type->name, pb_type_index);
  subckt_name = chomp_verilog_prefix(subckt_name);
  /* Check if defines an included netlist*/
  if (NULL == verilog_model->model_netlist) {
    if (LUT_CLASS == cur_pb_type->class_type) {
      /* For LUT, we have a built-in netlist, See "verilog_lut.c", So we don't do anything here */
    } else {
      /* We report an error */
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Require an included netlist in Pb_type(%s) SPICE_model(%s)!\n",
                __FILE__, __LINE__, cur_pb_type->name, verilog_model->name);
      exit(1);
    }
  }
  /* Print the definition line 
   * IMPORTANT: NO SRAMs ports are created here, they are fixed when quoting spice_models
   */
  fprintf(fp, "module %s (", subckt_name);
  /* subckt_port_name = format_verilog_node_prefix(subckt_name); */
  /* Inputs, outputs, inouts, clocks */
  dump_verilog_pb_type_ports(fp, subckt_name, 0, cur_pb_type, TRUE, FALSE, FALSE);
  /* SRAM ports */
  fprintf(fp, ");\n");
  /* Include the spice_model*/
  fprintf(fp, "%s %s[%d] (", verilog_model->name, verilog_model->prefix, verilog_model->cnt); 
  verilog_model->cnt++; /* Stats the number of verilog_model used*/
  /* Make input, output, inout, clocks connected*/
  /* IMPORTANT: (sequence of these ports should be changed!) */
  dump_verilog_pb_type_ports(fp, subckt_name, 0, cur_pb_type, FALSE, FALSE, TRUE);
  fprintf(fp, ");");
  /* Print end of subckt*/
  fprintf(fp, "endmodule\n");
  /* Free */
  my_free(subckt_name);
  
  return;
}

/* Print the subckt of a primitive pb */
void dump_verilog_pb_primitive_verilog_model(t_sram_orgz_info* cur_sram_orgz_info,
                                             FILE* fp,
                                             char* subckt_prefix,
                                             t_pb_graph_node* prim_pb_graph_node,
                                             int pb_index,
                                             t_spice_model* verilog_model) {
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check cur_pb_graph_node*/
  if (NULL == prim_pb_graph_node) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid prim_pb_graph_node.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
 
  /* Initialize */ 
  /* Asserts*/
  assert(pb_index == prim_pb_graph_node->placement_index);
  assert(0 == strcmp(verilog_model->name, prim_pb_graph_node->pb_type->spice_model->name));

  /* According to different type, we print netlist*/
  switch (verilog_model->type) {
  case SPICE_MODEL_LUT:
    /* If this is a idle block we should set sram_bits to zero*/
    dump_verilog_pb_primitive_lut(cur_sram_orgz_info, fp, subckt_prefix, prim_pb_graph_node,
                                  pb_index, verilog_model);
    break;
  case SPICE_MODEL_FF:
    assert(NULL != verilog_model->model_netlist);
    /* TODO : We should learn trigger type and initial value!!! and how to apply them!!! */
    dump_verilog_pb_generic_primitive(cur_sram_orgz_info, fp, subckt_prefix, prim_pb_graph_node, 
                                      pb_index, verilog_model);
    break;
  case SPICE_MODEL_IOPAD:
    assert(NULL != verilog_model->model_netlist);
    dump_verilog_pb_generic_primitive (cur_sram_orgz_info, fp, subckt_prefix, prim_pb_graph_node,
                                       pb_index, verilog_model);
    break;
  case SPICE_MODEL_HARDLOGIC:
    assert(NULL != verilog_model->model_netlist);
    dump_verilog_pb_generic_primitive(cur_sram_orgz_info, fp, subckt_prefix, prim_pb_graph_node,
                                      pb_index, verilog_model);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid type of verilog_model(%s), should be [LUT|FF|HARD_LOGIC|IO]!\n",
               __FILE__, __LINE__, verilog_model->name);
    exit(1);
  }
 
  return; 
}

/* Print physical mode of pb_types and configure it to the physical pb_types recursively
 * search the physical_mode until we reach the leaf node
 * Now we only dump one Verilog for each pb_type, and instance them when num_pb > 1
 */
void dump_verilog_phy_pb_graph_node_rec(t_sram_orgz_info* cur_sram_orgz_info,
                                        FILE* fp,
                                        char* subckt_prefix,
                                        t_pb_graph_node* cur_pb_graph_node,
                                        int pb_type_index) {
  int mode_index, ipb, jpb, child_mode_index;
  t_pb_type* cur_pb_type = NULL;
  char* subckt_name = NULL;
  char* formatted_subckt_prefix = format_verilog_node_prefix(subckt_prefix); /* Complete a "_" at the end if needed*/
  char* pass_on_prefix = NULL;
  char* child_pb_type_prefix = NULL;
  char* subckt_port_prefix = NULL;

  int child_pb_num_reserved_conf_bits = 0;
  int child_pb_num_conf_bits = 0;
  int child_pb_num_iopads = 0;

  t_spice_model* mem_model = NULL;  
  int num_reserved_conf_bits = 0;
  int num_conf_bits = 0;
  int stamped_sram_cnt = get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info); 
  int stamped_sram_lsb = get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info); 

  int stamped_iopad_cnt = iopad_verilog_model->cnt;

  /* A flag to mark if verilog module has been dumped */
  boolean verilog_module_dumped = FALSE;
  
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check cur_pb_graph_node*/
  if (NULL == cur_pb_graph_node) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid cur_pb_graph_node.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  cur_pb_type = cur_pb_graph_node->pb_type;

  /* Recursively finish all the child pb_types*/
  if (FALSE == is_primitive_pb_type(cur_pb_type)) { 
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_physical_mode_index((*cur_pb_type));
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      /* Initialize that the current verilog module has not been dumped */
      verilog_module_dumped = FALSE;
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
        if (TRUE == verilog_module_dumped) {
          /* Necessary step for the SDC generation, since the name is copied inside 
           * pf the verilog dumping and we skip it here */
          rec_copy_name_mux_in_node(&(cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][0]), 
                                &(cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb]));
          continue;
        }
        /* Pass the SPICE mode prefix on, 
         * <subckt_name>mode[<mode_name>]_
         */
        pass_on_prefix = (char*)my_malloc(sizeof(char)*
                           (strlen(formatted_subckt_prefix) + strlen(cur_pb_type->name)  
                            + 6 + strlen(cur_pb_type->modes[mode_index].name) + 1 + 1 + 1));
        sprintf(pass_on_prefix, "%s%s_mode_%s__", 
                formatted_subckt_prefix, cur_pb_type->name, cur_pb_type->modes[mode_index].name);
        /* Recursive*/
        /* Refer to pack/output_clustering.c [LINE 392] */
        /* Find the child pb that is mapped, and the mapping info is not stored in the physical mode ! */
        dump_verilog_phy_pb_graph_node_rec(cur_sram_orgz_info, fp, pass_on_prefix, 
                                           &(cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb]), jpb);
        /* Free */
        my_free(pass_on_prefix);
        /* Make the current module has been dumped */
        verilog_module_dumped = TRUE;
      }
    }
  }

  /* Check if this has defined a spice_model*/
  if (TRUE == is_primitive_pb_type(cur_pb_type)) { 
    switch (cur_pb_type->class_type) {
    case LUT_CLASS: 
      dump_verilog_pb_primitive_verilog_model(cur_sram_orgz_info, fp, formatted_subckt_prefix, 
                                              cur_pb_graph_node, pb_type_index, 
                                              cur_pb_type->spice_model); /* last param means idle */
      break;
    case LATCH_CLASS:
      assert(0 == cur_pb_type->num_modes);
      /* Consider the num_pb, create all the subckts*/
      dump_verilog_pb_primitive_verilog_model(cur_sram_orgz_info, fp, formatted_subckt_prefix, 
                                              cur_pb_graph_node,  pb_type_index, 
                                              cur_pb_type->spice_model); /* last param means idle */
      break;
    case UNKNOWN_CLASS:
    case MEMORY_CLASS:
      /* Consider the num_pb, create all the subckts*/
      dump_verilog_pb_primitive_verilog_model(cur_sram_orgz_info, fp, formatted_subckt_prefix, 
                                              cur_pb_graph_node , pb_type_index, 
                                              cur_pb_type->spice_model); /* last param means idle */
      break;  
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Unknown class type of pb_type(%s)!\n",
                 __FILE__, __LINE__, cur_pb_type->name);
      exit(1);
    }
    /* update the number of SRAM, I/O pads */
    /* update stamped iopad counter */
    stamped_iopad_cnt += cur_pb_type->physical_mode_num_iopads;
    /* update stamped sram counter */
    stamped_sram_cnt += cur_pb_type->physical_mode_num_conf_bits;
    /* Check */
    assert(stamped_sram_cnt == get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info)); 
    assert(stamped_iopad_cnt == iopad_verilog_model->cnt);
    /* Finish for primitive node, return  */
    return;
  }

  /* Find the mode that define_idle_mode*/
  mode_index = find_pb_type_physical_mode_index((*cur_pb_type));
  /* Create a new subckt */
  /* <formatted_subckt_prefix>mode[<mode_name>]
   */
  subckt_name = (char*)my_malloc(sizeof(char)*
                (strlen(formatted_subckt_prefix) + strlen(cur_pb_type->name)  
                + 6 + strlen(cur_pb_type->modes[mode_index].name) + 1 + 1)); 
  /* Definition*/
  sprintf(subckt_name, "%s%s_mode_%s_", 
          formatted_subckt_prefix, cur_pb_type->name, cur_pb_type->modes[mode_index].name);
  /* Comment lines */
  fprintf(fp, "//----- Physical programmable logic block Verilog module %s -----\n", subckt_name);
  fprintf(fp, "module %s (", subckt_name);
  /* Inputs, outputs, inouts, clocks */
  subckt_port_prefix = (char*)my_malloc(sizeof(char)*
                                       (strlen(cur_pb_type->modes[mode_index].name) + 1 + 1));
  /*sprintf(subckt_port_prefix, "%s_", cur_pb_type->modes[mode_index].name);*/
  sprintf(subckt_port_prefix, "%s_", cur_pb_type->name);
  /*
  dump_verilog_pb_type_ports(fp, subckt_name, 0, cur_pb_type);
  */
  fprintf(fp, "\n");
  /* dump global ports */
  if(0 < dump_verilog_global_ports(fp, global_ports_head, TRUE)) {
    fprintf(fp, ",\n");
  }
  /* Simplify the port prefix, make SPICE netlist readable */
  dump_verilog_pb_type_ports(fp, subckt_port_prefix, 0, cur_pb_type, TRUE, FALSE, FALSE);
  /* Print Input Pad and Output Pad */
  dump_verilog_grid_common_port(fp, iopad_verilog_model,
                                gio_inout_prefix, 
                                stamped_iopad_cnt, iopad_verilog_model->cnt - 1,
                                VERILOG_PORT_INOUT);
  /* Print Configuration ports */
  /* sram_verilog_model->cnt should be updated because all the child pbs have been dumped
   * stamped_sram_cnt remains the old sram_verilog_model->cnt before all the child pbs are dumped
   * Note by far, sram_verilog_model->cnt could be smaller than num_conf_bits
   * because the interconnection of current pb_type/pb has not yet dumped, which may contain
   * a few configuration bits. 
   */
  num_reserved_conf_bits = cur_pb_type->physical_mode_num_reserved_conf_bits;
  if (0 < num_reserved_conf_bits) {
    fprintf(fp, ",\n");
  }
  dump_verilog_reserved_sram_ports(fp, cur_sram_orgz_info,
                                   0, num_reserved_conf_bits - 1,
                                   VERILOG_PORT_INPUT);
  num_conf_bits = cur_pb_type->physical_mode_num_conf_bits;
  if (0 < num_conf_bits) {
    fprintf(fp, ",\n");
  }
  dump_verilog_sram_ports(fp, cur_sram_orgz_info,
                          stamped_sram_cnt, stamped_sram_cnt + num_conf_bits - 1,
                          VERILOG_PORT_INPUT);
  /* Dump ports only visible during formal verification*/
  if (0 < num_conf_bits) { 
    fprintf(fp, "\n");
    fprintf(fp, "`ifdef %s\n", verilog_formal_verification_preproc_flag);
    fprintf(fp, ",\n");
    dump_verilog_formal_verification_sram_ports(fp, cur_sram_orgz_info, 
                                                stamped_sram_cnt, 
                                                stamped_sram_cnt + num_conf_bits - 1,
                                                VERILOG_PORT_INPUT);
    fprintf(fp, "\n");
    fprintf(fp, "`endif\n");
  }

  /* Finish with local vdd and gnd */
  fprintf(fp, ");\n");
  /* Definition ends*/

  /* Local wires for memory configurations */
  dump_verilog_sram_config_bus_internal_wires(fp, cur_sram_orgz_info, 
                                              stamped_sram_cnt, 
                                              stamped_sram_cnt + num_conf_bits - 1); 

  /* Quote all child pb_types */
  for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    /* Each child may exist multiple times in the hierarchy*/
    for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* we should make sure this placement index == child_pb_type[jpb]*/
      assert(jpb == cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb].placement_index);
      /* If the pb_type_children is a leaf node, we don't use the mode to name it,
       * else we can use the mode to name it 
       */
      if (NULL == cur_pb_type->modes[mode_index].pb_type_children[ipb].spice_model) { /* Not a leaf node*/
        child_mode_index = find_pb_type_physical_mode_index(cur_pb_type->modes[mode_index].pb_type_children[ipb]);
        fprintf(fp, "%s_%s_mode_%s_ ",
                subckt_name, cur_pb_type->modes[mode_index].pb_type_children[ipb].name, 
                cur_pb_type->modes[mode_index].pb_type_children[ipb].modes[child_mode_index].name);
      } else { /* Have a verilog model definition, this is a leaf node*/
        fprintf(fp, "%s_%s ",
                subckt_name, cur_pb_type->modes[mode_index].pb_type_children[ipb].name); 
      }
      /* Collect information of child pb_type */
      child_pb_num_reserved_conf_bits = cur_pb_type->modes[mode_index].pb_type_children[ipb].physical_mode_num_reserved_conf_bits;
      child_pb_num_conf_bits = cur_pb_type->modes[mode_index].pb_type_children[ipb].physical_mode_num_conf_bits;
      child_pb_num_iopads = cur_pb_type->modes[mode_index].pb_type_children[ipb].physical_mode_num_iopads;

      /* <formatted_subckt_prefix>mode[<mode_name>]_<child_pb_type_name>[<ipb>]
       */
      fprintf(fp, "%s (", 
              gen_verilog_one_pb_graph_node_instance_name(&(cur_pb_graph_node->child_pb_graph_nodes[mode_index][ipb][jpb])));
      fprintf(fp, "\n");
      /* dump global ports */
      /* If the child node is a primitive, we only dump global ports belonging to this primitive */
      if (NULL == cur_pb_type->modes[mode_index].pb_type_children[ipb].spice_model) {
        if (0 < dump_verilog_global_ports(fp, global_ports_head, FALSE)) {
          fprintf(fp, ",\n");
        }
      } else {
        if (0 < rec_dump_verilog_spice_model_global_ports(fp, 
                                                          cur_pb_type->modes[mode_index].pb_type_children[ipb].spice_model,
                                                          FALSE, TRUE, FALSE)) {
          fprintf(fp, ",\n");
        }
      }
      /* Pass the SPICE mode prefix on, 
       * <subckt_name>mode[<mode_name>]_<child_pb_type_name>[<jpb>]
       * <child_pb_type_name>[<jpb>]
       */
      /* Simplify the prefix! */
      child_pb_type_prefix = (char*)my_malloc(sizeof(char)* 
                                (strlen(cur_pb_type->modes[mode_index].pb_type_children[ipb].name) + 1 
                                 + strlen(my_itoa(jpb)) + 1 + 1));
      sprintf(child_pb_type_prefix, "%s_%d_",
              cur_pb_type->modes[mode_index].pb_type_children[ipb].name, jpb);
      /* Print inputs, outputs, inouts, clocks
       * NO SRAMs !!! They have already been fixed in the bottom level
       */
      dump_verilog_pb_type_ports(fp, child_pb_type_prefix, 0, &(cur_pb_type->modes[mode_index].pb_type_children[ipb]), FALSE, FALSE, FALSE);
      /* Print I/O pads */
      dump_verilog_grid_common_port(fp, iopad_verilog_model,
                                    gio_inout_prefix, 
                                    stamped_iopad_cnt,
                                    stamped_iopad_cnt + child_pb_num_iopads - 1,
                                    VERILOG_PORT_CONKT);
      /* update stamped outpad counter */
      stamped_iopad_cnt += child_pb_num_iopads;
      /* Print configuration ports */
      if (0 < child_pb_num_reserved_conf_bits) {
        fprintf(fp, ",\n");
        dump_verilog_reserved_sram_ports(fp, cur_sram_orgz_info,
                                         0, child_pb_num_reserved_conf_bits - 1,
                                         VERILOG_PORT_CONKT);
      }
      if (0 < child_pb_num_conf_bits) {
        fprintf(fp, ",\n");
        dump_verilog_sram_local_ports(fp, cur_sram_orgz_info,
                                      stamped_sram_cnt,
                                      stamped_sram_cnt + child_pb_num_conf_bits - 1,
                                      VERILOG_PORT_CONKT);
      }
      /* Dump ports only visible during formal verification*/
      if (0 < child_pb_num_conf_bits) {
        fprintf(fp, "\n");
        fprintf(fp, "`ifdef %s\n", verilog_formal_verification_preproc_flag);
        fprintf(fp, ",\n");
        dump_verilog_formal_verification_sram_ports(fp, cur_sram_orgz_info, 
                                                    stamped_sram_cnt,
                                                    stamped_sram_cnt + child_pb_num_conf_bits - 1,
                                                    VERILOG_PORT_CONKT);
        fprintf(fp, "\n");
        fprintf(fp, "`endif\n");
      }

      /* update stamped sram counter */
      stamped_sram_cnt += child_pb_num_conf_bits; 
      fprintf(fp, ");\n"); /* Local vdd and gnd*/
      /* Update mem bits */
      update_sram_orgz_info_num_mem_bit(cur_sram_orgz_info, stamped_sram_cnt);
      my_free(child_pb_type_prefix);
    }
  }
  /* Print interconnections, set is_idle as TRUE*/
  dump_verilog_pb_graph_interc(cur_sram_orgz_info, fp, subckt_name, cur_pb_graph_node, mode_index);
  /* Check each pins of pb_graph_node */ 
  /* Check and update stamped_sram_cnt */
  /* Now we only dump one Verilog for each pb_type, and instance them when num_pb > 1
   * the asserts are no longer valid 
  assert(!(stamped_sram_cnt > (stamped_sram_lsb + num_conf_bits)));
  stamped_sram_cnt = stamped_sram_lsb + num_conf_bits;
  */
  /* End the subckt */
  fprintf(fp, "endmodule\n");
  /* Comment lines */
  fprintf(fp, "//----- END Physical  programmable logic block Verilog module %s -----\n\n", subckt_name);
  /* Free subckt name*/
  my_free(subckt_name);

  /*
  assert(stamped_sram_cnt == get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info)); 
  assert(stamped_iopad_cnt == iopad_verilog_model->cnt);
  */

  return;
}

void rec_copy_name_mux_in_node(t_pb_graph_node* master_node, 
                           t_pb_graph_node* target_node) {

  int ipb, jpb, mode_index; 
  int i_pin, i_port;
  t_pb_type* cur_pb_type = NULL;
  
  /* Desend to the leaf nodes */
  cur_pb_type = master_node->pb_type;

  /* Recursively finish all the child pb_types*/
  if (FALSE == is_primitive_pb_type(cur_pb_type)) { 
    /* Find the mode that define_idle_mode*/
    mode_index = find_pb_type_physical_mode_index((*cur_pb_type));
    for (ipb = 0; ipb < cur_pb_type->modes[mode_index].num_pb_type_children; ipb++) {
      for (jpb = 0; jpb < cur_pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
          rec_copy_name_mux_in_node(&(master_node->child_pb_graph_nodes[mode_index][ipb][0]), 
                                &(target_node->child_pb_graph_nodes[mode_index][ipb][jpb]));
      }
    }
  } 
  


  /* Input pins */
  for (i_port = 0; i_port< master_node->num_input_ports; i_port++) {
    for (i_pin = 0; i_pin < master_node->num_input_pins[i_port]; i_pin++) {
      target_node->input_pins[i_port][i_pin].name_mux = master_node->input_pins[i_port][i_pin].name_mux;
    }
  }
  /* Output pins */
  for (i_port = 0; i_port< master_node->num_output_ports; i_port++) {
    for (i_pin = 0; i_pin < master_node->num_output_pins[i_port]; i_pin++) {
      target_node->output_pins[i_port][i_pin].name_mux = master_node->output_pins[i_port][i_pin].name_mux;
      
    }
  }
  /* Clock pins */
  for (i_port = 0; i_port< master_node->num_clock_ports; i_port++) {
    for (i_pin = 0; i_pin < master_node->num_clock_pins[i_port]; i_pin++) {
      target_node->clock_pins[i_port][i_pin].name_mux = master_node->clock_pins[i_port][i_pin].name_mux;
    }
  }

  return;
}
/* Print an physical logic block
 * Find the physical_mode in arch files,
 * And print the verilog netlist into file
 */
void dump_verilog_physical_block(t_sram_orgz_info* cur_sram_orgz_info,
                                 FILE* fp,
                                 char* subckt_name, 
                                 int x,
                                 int y,
                                 int z,
                                 t_type_ptr type_descriptor) {
  t_pb_graph_node* top_pb_graph_node = NULL;
  t_block* mapped_block = NULL;
  t_pb* top_pb = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Ensure we have a valid type_descriptor*/ 
  assert(NULL != type_descriptor);

  /* Go for the pb_types*/
  top_pb_graph_node = type_descriptor->pb_graph_head;
  assert(NULL != top_pb_graph_node);

  /* Check in all the mapped blocks(clustered logic block), there is a match x,y,z*/
  mapped_block = search_mapped_block(x, y, z); 
  if (NULL != mapped_block) {
    top_pb = mapped_block->pb; 
    assert(NULL != top_pb);
  }

  /* Recursively find all idle mode and print netlist*/
  dump_verilog_phy_pb_graph_node_rec(cur_sram_orgz_info, fp, subckt_name, top_pb_graph_node, z);

  return;
}

/* We print all the pins of a type descriptor in the following sequence 
 * TOP, RIGHT, BOTTOM, LEFT
 */
void dump_verilog_grid_pins(FILE* fp,
                      int x,
                      int y,
                      boolean top_level,
                      boolean dump_port_type,
                      boolean dump_last_comma) {
  int iheight, side, ipin, class_id; 
  int side_pin_index;
  t_type_ptr type_descriptor = grid[x][y].type;
  int capacity = grid[x][y].type->capacity;
  int num_dumped_port = 0;
  int first_dump = 1;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 
  assert(NULL != type_descriptor);
  assert(0 < capacity);

  for (side = 0; side < 4; side++) {
    /* Count the number of pins */
    side_pin_index = 0;
    //for (iz = 0; iz < capacity; iz++) {
      for (iheight = 0; iheight < type_descriptor->height; iheight++) {
        for (ipin = 0; ipin < type_descriptor->num_pins; ipin++) {
          if (1 == type_descriptor->pinloc[iheight][side][ipin]) {
            /* Add comma if needed */
            if (1 == first_dump) {
              first_dump = 0;
            } else { 
              if (TRUE == dump_port_type) {
                fprintf(fp, ",\n");
              } else {
                fprintf(fp, ",\n");
             }
            }
            if (TRUE == dump_port_type) {
              /* Determine this pin is an input or output */
              class_id = type_descriptor->pin_class[ipin];
              switch (type_descriptor->class_inf[class_id].type) {
              case RECEIVER:
                fprintf(fp, "input ");
                break;
              case DRIVER:
                fprintf(fp, "output ");
                break;
              default:
                vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid pin_class_type!\n",
                           __FILE__, __LINE__);
                exit(1);
              }
            }
            /* This pin appear at this side! */
            fprintf(fp, " %s", gen_verilog_grid_one_pin_name(x, y, iheight, side, ipin, top_level)); 
            /* Update counter */
            num_dumped_port++;
            side_pin_index++;
          }
        }
      }  
    //}
  }

  if ((0 < num_dumped_port)&&(TRUE == dump_last_comma)) {
    if (TRUE == dump_port_type) {
      fprintf(fp, ",\n");
    } else {
      fprintf(fp, ",\n");
    }
  }

  return;
} 

/* Special for I/O grid, we need only part of the ports
 * i.e., grid[0][0..ny] only need the right side ports.
 */
/* We print all the pins of a type descriptor in the following sequence 
 * TOP, RIGHT, BOTTOM, LEFT
 */
void dump_verilog_io_grid_pins(FILE* fp,
                               int x, int y,
                               boolean top_level,
                               boolean dump_port_type,
                               boolean dump_last_comma) {
  int iheight, side, ipin; 
  int side_pin_index;
  t_type_ptr type_descriptor = grid[x][y].type;
  int capacity = grid[x][y].type->capacity;
  int class_id = -1;
  int num_dumped_port = 0;
  int first_dump = 1;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 
  assert(NULL != type_descriptor);
  assert(0 < capacity);
  /* Make sure this is IO */
  assert(IO_TYPE == type_descriptor);

  /* identify the location of IO grid and 
   * decide which side of ports we need
   */
  side = determine_io_grid_side(x,y);
 
  /* Count the number of pins */
  side_pin_index = 0;
  //for (iz = 0; iz < capacity; iz++) {
    for (iheight = 0; iheight < type_descriptor->height; iheight++) {
      for (ipin = 0; ipin < type_descriptor->num_pins; ipin++) {
        if (1 == type_descriptor->pinloc[iheight][side][ipin]) {
          /* Add comma if needed */
          if (1 == first_dump) {
            first_dump = 0;
          } else { 
            if (TRUE == dump_port_type) {
              fprintf(fp, ",\n");
            } else {
              fprintf(fp, ",\n");
            }
          }
          /* Determine this pin is an input or output */
          if (TRUE == dump_port_type) {
            class_id = type_descriptor->pin_class[ipin];
            switch (type_descriptor->class_inf[class_id].type) {
            case RECEIVER:
              fprintf(fp, "input ");
              break;
            case DRIVER:
              fprintf(fp, "output ");
              break;
            default:
              vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid pin_class_type!\n",
                         __FILE__, __LINE__);
              exit(1);
            }
          }
          /* This pin appear at this side! */
          fprintf(fp, " %s", gen_verilog_grid_one_pin_name(x, y, iheight, side, ipin, top_level)); 
          /* Update counter */
          num_dumped_port++;
          side_pin_index++;
        }
      }  
    }
  //}
  
  if ((0 < num_dumped_port)&&(TRUE == dump_last_comma)) {
    if (TRUE == dump_port_type) {
      fprintf(fp, ",\n");
    } else {
      fprintf(fp, ",\n");
    }
  }

  return;
} 

char* verilog_get_grid_block_subckt_name(int x, int y, int z,
                                         char* subckt_prefix,
                                         t_block* mapped_block) {
  char* ret = NULL;
  int imode; 
  t_type_ptr type_descriptor = NULL;
  char* formatted_subckt_prefix = format_verilog_node_prefix(subckt_prefix);
  int num_idle_mode = 0;

  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 

  type_descriptor = grid[x][y].type;
  assert(NULL != type_descriptor);

  if (NULL == mapped_block) {
    /* This a NULL logic block... Find the idle mode*/
    for (imode = 0; imode < type_descriptor->pb_type->num_modes; imode++) {
      if (1 == type_descriptor->pb_type->modes[imode].define_idle_mode) {
        num_idle_mode++;
      }
    } 
    assert(1 == num_idle_mode);
    for (imode = 0; imode < type_descriptor->pb_type->num_modes; imode++) {
      if (1 == type_descriptor->pb_type->modes[imode].define_idle_mode) {
        ret = (char*)my_malloc(sizeof(char)* 
               (strlen(formatted_subckt_prefix) + strlen(type_descriptor->name) + 1
                + strlen(my_itoa(z)) + 7 + strlen(type_descriptor->pb_type->modes[imode].name) + 1 + 1)); 
        sprintf(ret, "%s%s_%d__mode_%s_", formatted_subckt_prefix,
                type_descriptor->name, z, type_descriptor->pb_type->modes[imode].name);
        break;
      }
    } 
  } else {
    /* This is a logic block with specific configurations*/ 
    assert(NULL != mapped_block->pb);
    imode = mapped_block->pb->mode;
    ret = (char*)my_malloc(sizeof(char)* 
           (strlen(formatted_subckt_prefix) + strlen(type_descriptor->name) + 1
            + strlen(my_itoa(z)) + 7 + strlen(type_descriptor->pb_type->modes[imode].name) + 1 + 1)); 
    sprintf(ret, "%s%s_%d__mode_%s_", formatted_subckt_prefix,
            type_descriptor->name, z, type_descriptor->pb_type->modes[imode].name);
  }

  return ret;
}                        

/* Physical mode subckt name */
char* verilog_get_grid_phy_block_subckt_name(int x, int y, int z,
                                             char* subckt_prefix,
                                             t_block* mapped_block) {
  char* ret = NULL;
  int imode; 
  t_type_ptr type_descriptor = NULL;
  char* formatted_subckt_prefix = format_verilog_node_prefix(subckt_prefix);
  int num_physical_mode = 0;

  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 

  type_descriptor = grid[x][y].type;
  assert(NULL != type_descriptor);

  if (NULL == mapped_block) {
    /* This a NULL logic block... Find the idle mode*/
    for (imode = 0; imode < type_descriptor->pb_type->num_modes; imode++) {
      if (1 == type_descriptor->pb_type->modes[imode].define_physical_mode) {
        num_physical_mode++;
      }
    } 
    assert(1 == num_physical_mode);
    for (imode = 0; imode < type_descriptor->pb_type->num_modes; imode++) {
      if (1 == type_descriptor->pb_type->modes[imode].define_physical_mode) {
        ret = (char*)my_malloc(sizeof(char)* 
               (strlen(formatted_subckt_prefix) + strlen(type_descriptor->name) + 1
                + strlen(my_itoa(z)) + 7 + strlen(type_descriptor->pb_type->modes[imode].name) + 1 + 1)); 
        sprintf(ret, "%s%s_%d__mode_%s_", formatted_subckt_prefix,
                type_descriptor->name, z, type_descriptor->pb_type->modes[imode].name);
        break;
      }
    } 
  } else {
    /* This is a logic block with specific configurations*/ 
    assert(NULL != mapped_block->pb);
    imode = mapped_block->pb->mode;
    ret = (char*)my_malloc(sizeof(char)* 
           (strlen(formatted_subckt_prefix) + strlen(type_descriptor->name) + 1
            + strlen(my_itoa(z)) + 7 + strlen(type_descriptor->pb_type->modes[imode].name) + 1 + 1)); 
    sprintf(ret, "%s%s_%d__mode_%s_", formatted_subckt_prefix,
            type_descriptor->name, z, type_descriptor->pb_type->modes[imode].name);
  }

  return ret;
}                        


/* Print the pins of grid subblocks */
void dump_verilog_grid_block_subckt_pins(FILE* fp,
                                         int z,
                                         t_type_ptr type_descriptor) {
  int iport, ipin, side, dump_pin_cnt;
  int grid_pin_index, pin_height, side_pin_index;
  t_pb_graph_node* top_pb_graph_node = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert(NULL != type_descriptor);
  top_pb_graph_node = type_descriptor->pb_graph_head;
  assert(NULL != top_pb_graph_node); 

  dump_pin_cnt = 0;

  for (iport = 0; iport < top_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_input_pins[iport]; ipin++) {
      grid_pin_index = top_pb_graph_node->input_pins[iport][ipin].pin_count_in_cluster 
                     + z * type_descriptor->num_pins / type_descriptor->capacity;
      /* num_pins/capacity = the number of pins that each type_descriptor has.
       * Capacity defines the number of type_descriptors in each grid
       * so the pin index at grid level = pin_index_in_type_descriptor 
       *                                + type_descriptor_index_in_capacity * num_pins_per_type_descriptor
       */
      pin_height = type_descriptor->pin_height[grid_pin_index];
      for (side = 0; side < 4; side++) {
        if (1 == type_descriptor->pinloc[pin_height][side][grid_pin_index]) {
          /* This pin appear at this side! */
          if (0 < dump_pin_cnt) {
            fprintf(fp, ",\n");
          }
          fprintf(fp, "%s_height_%d__pin_%d_ ", 
                  convert_side_index_to_string(side), pin_height, grid_pin_index);
          dump_pin_cnt++;
          side_pin_index++;
        }
      }
    }
  }

  for (iport = 0; iport < top_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_output_pins[iport]; ipin++) {
      grid_pin_index = top_pb_graph_node->output_pins[iport][ipin].pin_count_in_cluster 
                     + z * type_descriptor->num_pins / type_descriptor->capacity;
      /* num_pins/capacity = the number of pins that each type_descriptor has.
       * Capacity defines the number of type_descriptors in each grid
       * so the pin index at grid level = pin_index_in_type_descriptor 
       *                                + type_descriptor_index_in_capacity * num_pins_per_type_descriptor
       */
      pin_height = type_descriptor->pin_height[grid_pin_index];
      for (side = 0; side < 4; side++) {
        if (1 == type_descriptor->pinloc[pin_height][side][grid_pin_index]) {
          /* This pin appear at this side! */
          if (0 < dump_pin_cnt) {
            fprintf(fp, ",\n");
          }
          fprintf(fp, "%s_height_%d__pin_%d_ ", 
                  convert_side_index_to_string(side), pin_height, grid_pin_index);
          dump_pin_cnt++;
          side_pin_index++;
        }
      }
    }
  }

  for (iport = 0; iport < top_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_clock_pins[iport]; ipin++) {
      grid_pin_index = top_pb_graph_node->clock_pins[iport][ipin].pin_count_in_cluster 
                     + z * type_descriptor->num_pins / type_descriptor->capacity;
      /* num_pins/capacity = the number of pins that each type_descriptor has.
       * Capacity defines the number of type_descriptors in each grid
       * so the pin index at grid level = pin_index_in_type_descriptor 
       *                                + type_descriptor_index_in_capacity * num_pins_per_type_descriptor
       */
      pin_height = type_descriptor->pin_height[grid_pin_index];
      for (side = 0; side < 4; side++) {
        if (1 == type_descriptor->pinloc[pin_height][side][grid_pin_index]) {
          /* This pin appear at this side! */
          if (0 < dump_pin_cnt) {
            fprintf(fp, ",\n");
          }
          fprintf(fp, "%s_height_%d__pin_%d_ ", 
                  convert_side_index_to_string(side), pin_height, grid_pin_index);
          dump_pin_cnt++;
          side_pin_index++;
        }
      }
    }
  }

  return;
}


/* Print the pins of grid subblocks */
void dump_verilog_io_grid_block_subckt_pins(FILE* fp,
                                      int x,
                                      int y,
                                      int z,
                                      t_type_ptr type_descriptor) {
  int iport, ipin, side, dump_pin_cnt;
  int grid_pin_index, pin_height, side_pin_index;
  t_pb_graph_node* top_pb_graph_node = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert(NULL != type_descriptor);
  top_pb_graph_node = type_descriptor->pb_graph_head;
  assert(NULL != top_pb_graph_node); 

  /* Make sure this is IO */
  assert(IO_TYPE == type_descriptor);

  /* identify the location of IO grid and 
   * decide which side of ports we need
   */
  side = determine_io_grid_side(x, y);

  dump_pin_cnt = 0;

  for (iport = 0; iport < top_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_input_pins[iport]; ipin++) {
      grid_pin_index = top_pb_graph_node->input_pins[iport][ipin].pin_count_in_cluster 
                     + z * type_descriptor->num_pins / type_descriptor->capacity;
      /* num_pins/capacity = the number of pins that each type_descriptor has.
       * Capacity defines the number of type_descriptors in each grid
       * so the pin index at grid level = pin_index_in_type_descriptor 
       *                                + type_descriptor_index_in_capacity * num_pins_per_type_descriptor
       */
      pin_height = type_descriptor->pin_height[grid_pin_index];
      if (1 == type_descriptor->pinloc[pin_height][side][grid_pin_index]) {
        /* This pin appear at this side! */
        if (0 < dump_pin_cnt) {
          fprintf(fp, ",\n");
        }
        fprintf(fp, "%s_height_%d__pin_%d_", 
                convert_side_index_to_string(side), pin_height, grid_pin_index);
        side_pin_index++;
        dump_pin_cnt++;
      }
    }
  }

  for (iport = 0; iport < top_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_output_pins[iport]; ipin++) {
      grid_pin_index = top_pb_graph_node->output_pins[iport][ipin].pin_count_in_cluster 
                     + z * type_descriptor->num_pins / type_descriptor->capacity;
      /* num_pins/capacity = the number of pins that each type_descriptor has.
       * Capacity defines the number of type_descriptors in each grid
       * so the pin index at grid level = pin_index_in_type_descriptor 
       *                                + type_descriptor_index_in_capacity * num_pins_per_type_descriptor
       */
      pin_height = type_descriptor->pin_height[grid_pin_index];
      if (1 == type_descriptor->pinloc[pin_height][side][grid_pin_index]) {
        /* This pin appear at this side! */
        if (0 < dump_pin_cnt) {
          fprintf(fp, ",\n");
        }
        fprintf(fp, "%s_height_%d__pin_%d_", 
                convert_side_index_to_string(side), pin_height, grid_pin_index);
        side_pin_index++;
        dump_pin_cnt++;
      }
    }
  }

  for (iport = 0; iport < top_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_clock_pins[iport]; ipin++) {
      grid_pin_index = top_pb_graph_node->clock_pins[iport][ipin].pin_count_in_cluster 
                     + z * type_descriptor->num_pins / type_descriptor->capacity;
      /* num_pins/capacity = the number of pins that each type_descriptor has.
       * Capacity defines the number of type_descriptors in each grid
       * so the pin index at grid level = pin_index_in_type_descriptor 
       *                                + type_descriptor_index_in_capacity * num_pins_per_type_descriptor
       */
      pin_height = type_descriptor->pin_height[grid_pin_index];
      if (1 == type_descriptor->pinloc[pin_height][side][grid_pin_index]) {
        /* This pin appear at this side! */
        if (0 < dump_pin_cnt) {
          fprintf(fp, ",\n");
        }
        fprintf(fp, "%s_height_%d__pin_%d_", 
                convert_side_index_to_string(side), pin_height, grid_pin_index);
        side_pin_index++;
        dump_pin_cnt++;
      }
    }
  }

  return;
}

/* Print the SPICE netlist for a I/O grid blocks */
void dump_verilog_physical_grid_blocks(t_sram_orgz_info* cur_sram_orgz_info,
                                       char* subckt_dir,
                                       int ix, int iy,
                                       t_arch* arch) {
  int subckt_name_str_len = 0;
  char* subckt_name = NULL;
  int iz;
  int capacity; 
  int cur_num_mem_bit;
  int temp_reserved_conf_bits_msb;
  int temp_conf_bits_lsb, temp_conf_bits_msb;
  int temp_iopad_lsb, temp_iopad_msb;
  FILE* fp = NULL;
  char* fname = NULL;
  t_spice_model* cur_mem_model = NULL;

  /* Check */
  assert((!(0 > ix))&&(!(ix > (nx + 1)))); 
  assert((!(0 > iy))&&(!(iy > (ny + 1)))); 

  /* Make a snapshot for the number of memory bits */
  cur_num_mem_bit = get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info); 
  get_sram_orgz_info_mem_model(cur_sram_orgz_info, &cur_mem_model);

  /* Update the grid_index_low for each spice_model */
  update_spice_models_grid_index_low(ix, iy, arch->spice->num_spice_model, arch->spice->spice_models);

  /* generate_grid_subckt, type_descriptor of each grid defines the capacity,
   * for example, each grid may contains more than one top-level pb_types, such as I/O
   */
  if ((NULL == grid[ix][iy].type)
      ||(EMPTY_TYPE == grid[ix][iy].type)
      ||(0 != grid[ix][iy].offset)) {
    /* Update the grid_index_high for each spice_model */
    update_spice_models_grid_index_high(ix, iy, arch->spice->num_spice_model, arch->spice->spice_models);
    return; 
  }

  /* Create file handler */
  fp = verilog_create_one_subckt_file(subckt_dir, "Physical Logic Block ", grid_verilog_file_name_prefix, ix, iy, &fname);

  capacity= grid[ix][iy].type->capacity;
  assert(0 < capacity);

  /* Make the sub-circuit name*/
  /* Name format: grid[<ix>][<iy>]_*/ 
  subckt_name_str_len = 4 + 1 + strlen(my_itoa(ix)) + 2 
                        + strlen(my_itoa(iy)) + 1 + 1 + 1; /* Plus '0' at the end of string*/
  subckt_name = (char*)my_malloc(sizeof(char)*subckt_name_str_len);
  sprintf(subckt_name, "grid_%d__%d__", ix, iy);

  /* check capacity and if this has been mapped */
  for (iz = 0; iz < capacity; iz++) {
    /* Comments: Grid [x][y]*/
    fprintf(fp, "//----- Grid[%d][%d] type_descriptor: %s[%d] -----\n", ix, iy, grid[ix][iy].type->name, iz);
    /* Print a NULL logic block...*/
    dump_verilog_physical_block(cur_sram_orgz_info, fp, subckt_name, ix, iy, iz, grid[ix][iy].type);
    fprintf(fp, "//----- END -----\n\n");
  } 

  /* Update the grid_index_high for each spice_model */
  update_spice_models_grid_index_high(ix, iy, arch->spice->num_spice_model, arch->spice->spice_models);

  /* Print grid[x][y] top-level module */
  fprintf(fp, "//----- Grid[%d][%d], Capactity: %d -----\n", ix, iy, capacity);
  fprintf(fp, "//----- Top Protocol -----\n");
  /* Definition */
  fprintf(fp, "module %s ( \n", gen_verilog_one_grid_module_name(ix, iy));
  fprintf(fp, "\n");
  /* dump global ports */
  if (0 < dump_verilog_global_ports(fp, global_ports_head, TRUE)) {
    fprintf(fp, ",\n");
  }

  /* Pins */
  /* Special Care for I/O grid */
  if (IO_TYPE == grid[ix][iy].type) {
    dump_verilog_io_grid_pins(fp, ix, iy, FALSE, TRUE, FALSE);
  } else {
    dump_verilog_grid_pins(fp, ix, iy, FALSE, TRUE, FALSE);
  }

  /* IO PAD */
  dump_verilog_grid_common_port(fp, iopad_verilog_model, gio_inout_prefix, 
                                iopad_verilog_model->grid_index_low[ix][iy],
                                iopad_verilog_model->grid_index_high[ix][iy] - 1,
                                VERILOG_PORT_INPUT); 

  /* Print configuration ports */
  /* Reserved configuration ports */
  temp_reserved_conf_bits_msb = grid[ix][iy].type->pb_type->physical_mode_num_reserved_conf_bits; 
  if (0 < temp_reserved_conf_bits_msb) { 
    fprintf(fp, ",\n");
    dump_verilog_reserved_sram_ports(fp, cur_sram_orgz_info,
                                     0, 
                                     temp_reserved_conf_bits_msb - 1,
                                     VERILOG_PORT_INPUT); 
  }
  /* Normal configuration ports */
  if (0 < (get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info) - cur_num_mem_bit)) { 
    fprintf(fp, ",\n");
    dump_verilog_sram_ports(fp, cur_sram_orgz_info,
                            cur_num_mem_bit, 
                            get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info) - 1, 
                            VERILOG_PORT_INPUT); 
  }
  /* Dump ports only visible during formal verification*/
  if (0 < (get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info) - 1 - cur_num_mem_bit)) { 
    fprintf(fp, "\n");
    fprintf(fp, "`ifdef %s\n", verilog_formal_verification_preproc_flag);
    fprintf(fp, ",\n");
    dump_verilog_formal_verification_sram_ports(fp, cur_sram_orgz_info, 
                                                cur_num_mem_bit, 
                                                get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info) - 1, 
                                                VERILOG_PORT_INPUT);
    fprintf(fp, "\n");
    fprintf(fp, "`endif\n");
  }

  fprintf(fp, ");\n");

  /* Record LSB and MSB of reserved_conf_bits and regular conf_bits in sram_orgz_info */
  cur_sram_orgz_info->grid_reserved_conf_bits[ix][iy] = temp_reserved_conf_bits_msb;
  cur_sram_orgz_info->grid_conf_bits_lsb[ix][iy] = cur_num_mem_bit;
  cur_sram_orgz_info->grid_conf_bits_msb[ix][iy] = get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info); 

  /* Initialize temporary counter */
  temp_conf_bits_lsb = cur_num_mem_bit;
  temp_iopad_lsb = iopad_verilog_model->grid_index_low[ix][iy];

  /* Local wires for memory configurations */
  dump_verilog_sram_config_bus_internal_wires(fp, cur_sram_orgz_info, 
                                              cur_sram_orgz_info->grid_conf_bits_lsb[ix][iy],
                                              cur_sram_orgz_info->grid_conf_bits_msb[ix][iy]); 

  /* Quote all the sub blocks*/
  for (iz = 0; iz < capacity; iz++) {
    /* Local Vdd and Gnd, subckt name*/
    fprintf(fp, "%s ", verilog_get_grid_phy_block_subckt_name(ix, iy, iz, subckt_name, NULL));
    fprintf(fp, " %s (", gen_verilog_one_block_instance_name(ix, iy, iz));
    fprintf(fp, "\n");
    /* dump global ports */
    if (0 < dump_verilog_global_ports(fp, global_ports_head, FALSE)) {
      fprintf(fp, ",\n");
    }
    /* Print all the pins */
    /* Special Care for I/O grid */
    if (IO_TYPE == grid[ix][iy].type) {
      dump_verilog_io_grid_block_subckt_pins(fp, ix, iy, iz, grid[ix][iy].type);
    } else {
      dump_verilog_grid_block_subckt_pins(fp, iz, grid[ix][iy].type);
    }
    /* Print configuration ports */
    temp_reserved_conf_bits_msb = grid[ix][iy].type->pb_type->physical_mode_num_reserved_conf_bits; 
    temp_conf_bits_msb = temp_conf_bits_lsb + grid[ix][iy].type->pb_type->physical_mode_num_conf_bits;
    temp_iopad_msb = temp_iopad_lsb + grid[ix][iy].type->pb_type->physical_mode_num_iopads;
    /* Print Input Pad and Output Pad */

    fprintf(fp, "\n//---- IOPAD ----\n");
    dump_verilog_grid_common_port(fp, iopad_verilog_model, gio_inout_prefix,
                                  temp_iopad_lsb,
                                  temp_iopad_msb - 1,
                                  VERILOG_PORT_CONKT); 
    assert(!(0 > temp_conf_bits_msb - temp_conf_bits_lsb));
    /* Reserved configuration ports */
    if (0 < temp_reserved_conf_bits_msb) { 
      fprintf(fp, ",\n");
      dump_verilog_reserved_sram_ports(fp, cur_sram_orgz_info,
                                       0, temp_reserved_conf_bits_msb - 1,
                                       VERILOG_PORT_CONKT); 
    }
    /* Normal configuration ports */
    if (0 < (temp_conf_bits_msb - 1 - temp_conf_bits_lsb)) { 
      fprintf(fp, ",\n");
      fprintf(fp, "//---- SRAM ----\n");
      dump_verilog_sram_local_ports(fp, cur_sram_orgz_info,
                                    temp_conf_bits_lsb, temp_conf_bits_msb - 1, 
                                    VERILOG_PORT_CONKT); 
    }
    /* Dump ports only visible during formal verification*/
    if (0 < (temp_conf_bits_msb - 1 - temp_conf_bits_lsb)) { 
      fprintf(fp, "\n");
      fprintf(fp, "`ifdef %s\n", verilog_formal_verification_preproc_flag);
      fprintf(fp, ",\n");
      dump_verilog_formal_verification_sram_ports(fp, cur_sram_orgz_info, 
                                                  temp_conf_bits_lsb, 
                                                  temp_conf_bits_msb - 1, 
                                                  VERILOG_PORT_INPUT);
      fprintf(fp, "\n");
      fprintf(fp, "`endif\n");
    }

    /* Update temp_sram_lsb */
    temp_conf_bits_lsb = temp_conf_bits_msb;
    temp_iopad_lsb = temp_iopad_msb;
    fprintf(fp, ");\n");
  }

  fprintf(fp, "endmodule\n");
  fprintf(fp, "//----- END Top Protocol -----\n");
  fprintf(fp, "//----- END Grid[%d][%d], Capactity: %d -----\n\n", ix, iy, capacity);

  /* Check */
  assert(temp_conf_bits_msb == get_sram_orgz_info_num_mem_bit(cur_sram_orgz_info)); 
  assert(temp_iopad_msb == iopad_verilog_model->grid_index_high[ix][iy]);

  /* Close the file */
  fclose(fp);

  /* Add fname to the linked list */
  grid_verilog_subckt_file_path_head = add_one_subckt_file_name_to_llist(grid_verilog_subckt_file_path_head, fname);  

  /* Free */
  my_free(subckt_name);
  my_free(fname);

  return;
}

/* Print all logic blocks SPICE models 
 * Each logic blocks in the grid that allocated for the FPGA
 * will be printed. May have an additional option that only
 * output the used logic blocks 
 */
void dump_verilog_logic_blocks(t_sram_orgz_info* cur_sram_orgz_info,
                               char* subckt_dir,
                               t_arch* arch) {
  int ix, iy; 
  
  /* Check the grid*/
  if ((0 == nx)||(0 == ny)) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid grid size (nx=%d, ny=%d)!\n", __FILE__, __LINE__, nx, ny);
    return;    
  }

  vpr_printf(TIO_MESSAGE_INFO,"Grid size of FPGA: nx=%d ny=%d\n", nx + 1, ny + 1);
  assert(NULL != grid);
 
  /* Print the core logic block one by one
   * Note ix=0 and ix = nx + 1 are IO pads. They surround the core logic blocks
   */
  vpr_printf(TIO_MESSAGE_INFO,"Generating core grids...\n");
  for (ix = 1; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      /* Ensure this is not a io */
      assert(IO_TYPE != grid[ix][iy].type);
      /* Ensure a valid usage */
      assert((0 == grid[ix][iy].usage)||(0 < grid[ix][iy].usage));
      dump_verilog_physical_grid_blocks(cur_sram_orgz_info, subckt_dir, ix, iy, arch); 
    }
  }

  vpr_printf(TIO_MESSAGE_INFO,"Generating IO grids...\n");
  /* Print the IO pads */
  /* Top side : x = 1 .. nx + 1, y = nx + 1  */
  iy = ny + 1;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    dump_verilog_physical_grid_blocks(cur_sram_orgz_info, subckt_dir, ix, iy, arch); 
  }

  /* Right side : x = nx + 1, y = 1 .. ny*/
  ix = nx + 1;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    dump_verilog_physical_grid_blocks(cur_sram_orgz_info, subckt_dir, ix, iy, arch); 
  }

  /* Bottom  side : x = 1 .. nx + 1, y = 0 */
  iy = 0;
  for (ix = 1; ix < (nx + 1); ix++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    dump_verilog_physical_grid_blocks(cur_sram_orgz_info, subckt_dir, ix, iy, arch); 
  }
  /* Left side: x = 0, y = 1 .. ny*/
  ix = 0;
  for (iy = 1; iy < (ny + 1); iy++) {
    /* Ensure this is a io */
    assert(IO_TYPE == grid[ix][iy].type);
    dump_verilog_physical_grid_blocks(cur_sram_orgz_info, subckt_dir, ix, iy, arch); 
  }

  /* Output a header file for all the logic blocks */
  vpr_printf(TIO_MESSAGE_INFO,"Generating header file for grid submodules...\n");
  dump_verilog_subckt_header_file(grid_verilog_subckt_file_path_head,
                                  subckt_dir,
                                  logic_block_verilog_file_name);

  /* Free */
   
  return; 
}

