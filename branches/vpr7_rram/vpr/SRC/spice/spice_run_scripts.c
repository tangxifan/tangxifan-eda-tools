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
#include "path_delay.h"
#include "stats.h"

/* Include spice support headers*/
#include "read_xml_spice_util.h"
#include "linkedlist.h"
#include "spice_globals.h"
#include "spice_utils.h"
#include "spice_subckt.h"
#include "spice_pbtypes.h"
#include "spice_heads.h"
#include "spice_lut.h"
#include "spice_top_netlist.h"
#include "spice_mux_testbench.h"
#include "spice_grid_testbench.h"
#include "spice_lut_testbench.h"
#include "spice_dff_testbench.h"

static char* run_hspice_shell_script_name = "run_hspice_sim.sh";
static char* sim_results_dir_name = "results/";

void fprint_run_hspice_shell_script(char* spice_dir_path,
                                    char* subckt_dir_path) {
  FILE* fp = NULL;
  /* Format the directory path */
  char* spice_dir_formatted = format_dir_path(spice_dir_path);
  char* shell_script_path = my_strcat(spice_dir_path, run_hspice_shell_script_name);
  char* testbench_file = NULL;
  char* chomped_testbench_file = NULL;
  char* chomped_testbench_path = NULL;
  char* chomped_testbench_name = NULL;
  char* sim_results_dir_path = my_strcat(spice_dir_formatted, sim_results_dir_name);
  t_llist* temp = tb_head;

  create_dir_path(sim_results_dir_path);

  /* Check if the path exists*/
  fp = fopen(shell_script_path,"w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create Shell Script for running HSPICE %s!",__FILE__, __LINE__, shell_script_path); 
    exit(1);
  } 

  /* Go to the subckt dir for HSPICE VerilogA sim*/
  fprintf(fp, "cd %s\n", subckt_dir_path);

  /* For VerilogA initilization */
  fprintf(fp, "source /softs/synopsys/hspice/2013.12/hspice/bin/cshrc.meta\n");
 

  /* Run hspice lut testbench netlist */
  while(temp) {
    testbench_file = (char*)(temp->dptr);
    chomped_testbench_file = chomp_file_name_postfix(testbench_file);
    split_path_prog_name(chomped_testbench_file,'/',&chomped_testbench_path ,&chomped_testbench_name);
    fprintf(fp, "hspice64 -mt 8 -i %s -o %s%s.lis -hdlpath /softs/synopsys/hspice/2013.12/hspice/include\n",
            testbench_file, sim_results_dir_path, chomped_testbench_name);
    temp = temp->next;
  }

  fprintf(fp, "cd %s\n", spice_dir_path);

  /* close fp */
  fclose(fp);

  /* Free */  
  my_free(spice_dir_formatted);
  my_free(shell_script_path);
  my_free(chomped_testbench_file);

  return;
}
