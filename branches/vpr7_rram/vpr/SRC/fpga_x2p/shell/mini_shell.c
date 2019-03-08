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
#include "read_opt_types.h"
#include "read_opt.h"
#include "shell_types.h"

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
#include "cmd_exit.h"
#include "shell_cmds.h"
#include "mini_shell.h"

#include <readline/readline.h>
#include <readline/history.h>


t_shell_env shell_env;

char* vpr_shell_prefix = "VPR7-OpenFPGA> ";

void shell_print_usage(t_shell_cmd* cur_shell_cmd) {

  t_shell_cmd* cur_cmd = shell_cmd;

  vpr_printf(TIO_MESSAGE_INFO, 
             "Available commands:\n");

  while (0 != strcmp(LAST_CMD_NAME, cur_cmd->name)) {
    /* Print command name */
    vpr_printf(TIO_MESSAGE_INFO, 
               "%s\n",
               cur_cmd->name);
    /* Go to next command*/
    cur_cmd++;
  } 

  return;
}

/* Identify the command to launch */
void process_shell_command(char* line) {
  int itok;
  int num_tokens = 0;
  char** token = NULL;
  t_shell_cmd* cur_cmd = shell_cmd;

  /* Tokenize the line */
  token = fpga_spice_strtok(line, " ", &num_tokens);  
  
  /* Search the shell command list and execute */ 
  while (0 != strcmp(LAST_CMD_NAME, cur_cmd->name)) {
    if (0 == strcmp(cur_cmd->name, token[0])) {
      /* Read options of read_blif */
      if (FALSE == read_options(num_tokens, token, cur_cmd->opts)) {
        return;
      }
      /* Execute setup_vpr engine*/
      cur_cmd->execute(&shell_env, cur_cmd->opts);
      /* Return here */
      return;
    }
    /* Go to next command*/
    cur_cmd++;
  } 

  /* Invalid command */
  vpr_printf(TIO_MESSAGE_INFO, 
             "Invalid command!\n");
  shell_print_usage(shell_cmd); 

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


