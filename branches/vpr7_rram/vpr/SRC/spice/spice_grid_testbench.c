/***********************************/
/*      SPICE Modeling for VPR     */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
#include <stdio.h>
#include <stdlib.h>
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

/* Include spice support headers*/
#include "linkedlist.h"
#include "spice_globals.h"
#include "spice_utils.h"
#include "spice_mux.h"
#include "spice_pbtypes.h"
#include "spice_subckt.h"
#include "spice_netlist_utils.h"
#include "spice_grid_testbench.h"

/* Global variable inside this C-source file*/
static int num_inv_load = 0;
static int num_noninv_load = 0;
static int num_grid_load = 0;

/* Local subroutines only accessible in this C-source file */
static 
void fprint_grid_testbench_global_ports(FILE* fp, 
                                        int num_clock, 
                                        t_spice spice);

/* Subroutines in this source file*/
static 
void fprint_grid_testbench_global_ports(FILE* fp, 
                                        int num_clock, 
                                        t_spice spice) {
  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprintf(fp, ".global gvdd ggnd gset greset\n");
  fprintf(fp, ".global gvdd_sram gvdd_local_interc gvdd_hardlogic\n");
  fprintf(fp, ".global gvdd_load\n");
  /* Define a global clock port if we need one*/
  if (1 == num_clock) {
    fprintf(fp, "***** Global Clock Signals *****\n");
    fprintf(fp, ".global gclock\n");
  } else {
    assert(0 == num_clock);
  }
  /*Global Vdds for LUTs*/
  fprint_global_vdds_spice_model(fp, SPICE_MODEL_LUT, spice);
  /*Global Vdds for FFs*/
  fprint_global_vdds_spice_model(fp, SPICE_MODEL_FF, spice);

  return;
}

static void fprint_grid_rr_node_loads_rec(FILE* fp,
                                          char* input_spice_port_name,
                                          t_rr_node* src_rr_node,
                                          t_spice spice) {
  int iedge = 0;
  int to_node = 0;
  int to_node_switch = 0;
  t_spice_model* to_node_spice_model = NULL;
  int inv_size = 0;
  int num_drive_rr_nodes = 0;
  t_rr_node** drive_rr_nodes = NULL;
  int* switch_indices = NULL;
  int path_id;

  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 

  for (iedge = 0; iedge < src_rr_node->num_edges; iedge++) {
    /* For each edge, 
     * 1. if the corresponding spice model of rr_node is buffered, 
     *    we add an inverter
     * 2. if not buffered, we should trace util a rr_node with buffered spice_model.
     */
    to_node = src_rr_node->edges[iedge];
    to_node_switch = rr_node[to_node].switches[iedge];
    to_node_spice_model = switch_inf[to_node_switch].spice_model;
    assert(NULL != to_node_spice_model);
    if (TRUE == switch_inf[to_node_switch].buffered) {
      /* Check the spice model, add an inverter load */
      if (1 == to_node_spice_model->input_buffer->exist) {
        /* This case is simple, we need an inverter*/
        switch (to_node_spice_model->input_buffer->type) {
        case SPICE_MODEL_BUF_INV:
          inv_size = to_node_spice_model->input_buffer->size;
          break;
        case SPICE_MODEL_BUF_BUF:
          inv_size = 1;
          break;
        default:
          vpr_printf(TIO_MESSAGE_ERROR, "(Files:%s, [LINE%d])Invalid type of input buffer of spice_model(%s)!\n", 
                     __FILE__, __LINE__, to_node_spice_model->name);
          exit(1);
        }
        /* Print the inverter */
        fprintf(fp, "Xgrid_load[%d]_inv %s grid_load[%d]_out inv gvdd_load ggnd size=%d\n",
                num_inv_load, input_spice_port_name, num_inv_load, inv_size);
        /* Update the load inverter number */
        num_inv_load++;
        num_grid_load++;
      } else {
        assert(1 == to_node_spice_model->output_buffer->exist);
        assert(SPICE_MODEL_MUX == to_node_spice_model->type);
        /* In this case, we need to call the spice subckt */
        fprintf(fp, "Xgrid_load[%d]_%s ", num_noninv_load, to_node_spice_model->name);
        /* Find all the inputs drive_rr_nodes*/
        find_prev_rr_nodes_with_src(&(rr_node[to_node]), &num_drive_rr_nodes, &drive_rr_nodes, &switch_indices);
        /* Find the path id*/
        path_id = find_path_id_prev_rr_node(num_drive_rr_nodes, drive_rr_nodes, &(rr_node[to_node]));
        /* Free */
        my_free(drive_rr_nodes);
        my_free(switch_indices);
      }
    } else {
      /* Add the spice_model, and then go recursively */
    } 
  }      

  return;
}

static 
void fprint_grid_testbench_loads(FILE* fp,
                                t_spice spice) {
  int inode;

  /* A valid file handler*/
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Invalid File Handler!\n",__FILE__, __LINE__); 
    exit(1);
  } 
  
  /* For each grid output port, we generate equivalent capacitive load(inverters) */
  /* Search all rr_nodes, we only care SOURCE/OPIN type */
  for (inode = 0; inode < num_rr_nodes; inode++) { 
    switch (rr_node[inode].type) {
    case SOURCE:
    case OPIN:
      /*  Make sure 0 fan-in, 1 fan-in is connected to SOURCE */
      assert((0 == rr_node[inode].fan_in)||(1 == rr_node[inode].fan_in));
      if (1 == rr_node[inode].fan_in) {
        assert(SOURCE == rr_node[rr_node[inode].prev_node].type);
      }
      /* Check out the number of fan-out (edge) */
      if (0 == rr_node[inode].num_edges) {
        break;
      }
      /* Check all the fanouts*/
      fprint_grid_rr_node_loads_rec(fp, "grid_", &(rr_node[inode]), spice);
      break; 
    case SINK:
    case IPIN:
    case CHANX:
    case CHANY:
    case INTRA_CLUSTER_EDGE:
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid rr_node type!\n",
                 __FILE__, __LINE__);
      exit(1);
    }
  }

  return;
}

static 
void fprint_grid_testbench_stimulations(FILE* fp, 
                                      t_spice spice) {
  /* For each grid input port, we generate the voltage pulses  */


  return;
}

static 
void fprint_grid_testbench_measurements(FILE* fp, 
                                      t_spice spice) {
  return;
}

/* Top-level function in this source file */
void fprint_spice_grid_testbench(char* circuit_name,
                                 char* grid_test_bench_name,
                                 char* include_dir_path,
                                 char* subckt_dir_path,
                                 t_ivec*** LL_rr_node_indices,
                                 int num_clock,
                                 t_spice spice) {
  FILE* fp = NULL;
  char* formatted_subckt_dir_path = format_dir_path(subckt_dir_path);
  char* temp_include_file_path = NULL;
  char* title = my_strcat("FPGA SPICE Netlist for Design: ", circuit_name);
  int i;

  /* Check if the path exists*/
  fp = fopen(grid_test_bench_name,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create top SPICE netlist %s!",__FILE__, __LINE__, grid_test_bench_name); 
    exit(1);
  } 
  
  vpr_printf(TIO_MESSAGE_INFO, "Writing Top-level FPGA Netlist for %s...\n", circuit_name);
 
  /* Print the title */
  fprint_spice_head(fp, title);
  my_free(title);

  /* print technology library and design parameters*/
  fprint_tech_lib(fp, spice.tech_lib);

  /* Include parameter header files */
  fprint_spice_include_param_headers(fp, include_dir_path);

  /* Include Key subckts */
  fprint_spice_include_key_subckts(fp, subckt_dir_path);

  
  /* Special subckts for Top-level SPICE netlist */
  fprintf(fp, "****** Include subckt netlists: Look-Up Tables (LUTs) *****\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, luts_spice_file_name);
  fprintf(fp, ".include %s\n", temp_include_file_path);
  my_free(temp_include_file_path);

  fprintf(fp, "****** Include subckt netlists: Logic Blocks *****\n");
  temp_include_file_path = my_strcat(formatted_subckt_dir_path, logic_block_spice_file_name);
  fprintf(fp, ".include %s\n", temp_include_file_path);
  my_free(temp_include_file_path);

  /* Print simulation temperature and other options for SPICE */
  fprint_spice_options(fp, spice.spice_params);

  /* Global nodes: Vdd for SRAMs, Logic Blocks(Include IO), Switch Boxes, Connection Boxes */
  fprint_grid_testbench_global_ports(fp, num_clock, spice);
 
  /* Quote defined Logic blocks subckts (Grids) */
  fprint_call_defined_grids(fp);

  fprint_grid_testbench_loads(fp, spice);

  /* Back-anotate activity information to each routing resource node 
   * (We should have activity of each Grid port) 
   */

  /* Add stimulations */
  fprint_grid_testbench_stimulations(fp, spice);

  /* Add measurements */  
  fprint_grid_testbench_measurements(fp, spice);

  /* SPICE ends*/
  fprintf(fp, ".end\n");

  /* Close the file*/
  fclose(fp);

  return;
}

