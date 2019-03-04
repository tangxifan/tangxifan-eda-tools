#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

/* Include vpr structs*/
#include "util.h"
#include "physical_types.h"
#include "vpr_types.h"
#include "globals.h"
#include "rr_graph_util.h"
#include "rr_graph.h"
#include "rr_graph2.h"
#include "vpr_utils.h"


/* Include SPICE support headers*/
#include "quicksort.h"
#include "linkedlist.h"
#include "fpga_x2p_globals.h"
#include "fpga_x2p_utils.h"
#include "shell_types.h"
#include "read_opt_types.h"
#include "read_opt.h"

/* Include APIs */
#include "vpr_api.h"
#include "fpga_x2p_api.h"
#include "cmd_vpr_setup.h"
#include "cmd_vpr_pack.h"
#include "cmd_vpr_place_and_route.h"
#include "cmd_vpr_power.h"
#include "cmd_fpga_x2p_setup.h"
#include "cmd_fpga_spice.h"
#include "cmd_fpga_verilog.h"
#include "cmd_fpga_bitstream.h"
#include "mini_shell.h"

#include <readline/readline.h>
#include <readline/history.h>


t_shell_env shell_env;

char* vpr_shell_name = "VPR7-OpenFPGA";
char* vpr_shell_prefix = "VPR7-OpenFPGA> ";


/* Identify the command to launch */
void process_shell_command(char* line) {
  int itok;
  int num_tokens = 0;
  char** token = NULL;

  /* Tokenize the line */
  token = fpga_spice_strtok(line, " ", &num_tokens);  

  /* Get command name: the first token */
  if (0 == strcmp("vpr_setup", token[0])) {  
    /* Read options of read_blif */
    if (FALSE == read_options(num_tokens, token, setup_vpr_opts)) {
      return;
    }
    /* Execute setup_vpr engine*/
    shell_execute_vpr_setup(&shell_env, setup_vpr_opts);
  } else if (0 == strcmp("vpr_pack", token[0])) {  
    if (FALSE == read_options(num_tokens, token, vpr_pack_opts)) {
      return;
    }
    /* Execute setup_vpr engine*/
    shell_execute_vpr_pack(&shell_env, vpr_pack_opts);
  } else if (0 == strcmp("vpr_place_and_route", token[0])) {  
    if (FALSE == read_options(num_tokens, token, vpr_place_and_route_opts)) {
      return;
    }
    shell_execute_vpr_place_and_route(&shell_env, vpr_place_and_route_opts);
  } else if (0 == strcmp("vpr_versapower", token[0])) {  
    read_options(num_tokens, token, vpr_versapower_opts);
    shell_execute_vpr_versapower(&shell_env, vpr_versapower_opts);
  } else if (0 == strcmp("fpga_x2p_setup", token[0])) {  
    read_options(num_tokens, token, fpga_x2p_setup_opts);
    shell_execute_fpga_x2p_setup(&shell_env, fpga_x2p_setup_opts);
  } else if (0 == strcmp("fpga_spice", token[0])) {  
    read_options(num_tokens, token, fpga_spice_opts);
    shell_execute_fpga_spice(&shell_env, fpga_spice_opts);
  } else if (0 == strcmp("fpga_verilog", token[0])) {  
    read_options(num_tokens, token, fpga_verilog_opts);
    shell_execute_fpga_verilog(&shell_env, fpga_verilog_opts);
  } else if (0 == strcmp("fpga_bitstream", token[0])) {  
    read_options(num_tokens, token, fpga_bitstream_opts);
    shell_execute_fpga_bitstream(&shell_env, fpga_bitstream_opts);
  } else if (0 == strcmp("exit", token[0])) {
    vpr_printf(TIO_MESSAGE_INFO, 
               "Thank you for using %s!\n",
               vpr_shell_name);
    exit(1);
  } else {
    /* Invalid command */
    vpr_printf(TIO_MESSAGE_INFO, 
               "Invalid command!\n");
    vpr_print_usage(); 
  }

  /* Free */
  for (itok = 0; itok < num_tokens; itok++) { 
    my_free(token[itok]);
  }
  my_free(token);

  return;
}

void init_shell_env(t_shell_env* env) {
  memset(&(env->vpr_setup), 0, sizeof(t_vpr_setup));
  memset(&(env->arch), 0, sizeof(t_arch));
  
  return;
}

/* Start the interactive shell */
void vpr_run_interactive_mode() {

  vpr_printf(TIO_MESSAGE_INFO, "Start interactive mode...\n");

  vpr_print_title();
  
  /* Initialize file handler */
  vpr_init_file_handler();

  /* Initialize shell_env */
  init_shell_env(&shell_env);
 
  while (1) {
    char * line = readline(vpr_shell_prefix);
    process_shell_command(line);
    /* Add to history */
    add_history(line);
    free (line);
  }

  return;
}

void vpr_run_script_mode(char* script_file_name) {
  FILE* fp = NULL;
  char buffer[BUFSIZE];
  char str_end = '\0'; 
  int i;

  vpr_print_title();
  
  /* Initialize file handler */
  vpr_init_file_handler();

  /* Initialize shell_env */
  init_shell_env(&shell_env);

  vpr_printf(TIO_MESSAGE_INFO, "Reading script file %s!\n", script_file_name);
 
  /* Read the file */
  fp = fopen(script_file_name, "r");
  /* Error out if the File handle is empty */
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,
               "Fail to open the script file: %s.\n",
				script_file_name);
    exit(1);
  }

  /* Read line by line */
  while (NULL != my_fgets(buffer, BUFSIZE, fp)) {
    if ((0 == strlen(buffer))
      || (0 == strcmp(buffer, ""))) {
      continue;
    }
    /* Chomp the \n of buffer */
    for (i = 0; i < strlen(buffer); i++) { 
      if ('\n' == buffer[i]) {
        buffer[i] = str_end;
      }
    }
    /* Treat each valid line as a command */ 
    process_shell_command(buffer);
  }

  /* Close file */
  fclose(fp);

  return;
}

void run_shell(int argc, char ** argv) {

  /* Parse the options and decide which interface to go */
  read_options(argc, argv, shell_opts);

  /* Interface 1: run through -i, --interactive 
   * Or with 0 arguments, we start the interactive shell 
   */
  if ( (1 == argc) 
    || (TRUE == is_opt_set(shell_opts, "i", FALSE)) ) {
    vpr_run_interactive_mode();
  } else if (TRUE == is_opt_set(shell_opts, "f", FALSE)) {
  /* Interface 2: run through -f, --file */
    char* script_file_name = get_opt_val(shell_opts, "f");
    vpr_run_script_mode(script_file_name);
  } else {
    vpr_printf(TIO_MESSAGE_ERROR, "Invalid command!\n");
    print_opt_info_help_desk(shell_opts);
  }

  return;
}


