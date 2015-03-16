// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon

// This tool handles the packing of MCluster-based FPGA architectures
// The clustering part of the tool is inspired from VPack 2.09

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "mpack.h"
#include "globals.h"
#include "read_blif.h"
#include "cluster.h"
#include "output_clustering.h"
#include "matrix_architecture.h"

// Instanciation of the global variables: see globals.h for their respective use
int num_nets, num_nets_validated, num_nets_current;
int num_blocks, num_blocks_validated, num_blocks_current;
int num_p_inputs, num_p_outputs;
int num_gates, num_latches;
int buffer_counting_validated, buffer_counting_current, buffer_counting_rollback;
struct s_net *net, *net_validated, *net_current;
struct s_block *block, *block_validated, *block_current;

// Main functions prototypes
static void parse_command (int argc, char *argv[], char *blif_file, char *output_file, boolean *global_clocks, int *matrix_width, int *matrix_depth, int *matrix_topo, int *cell_size, boolean *echo_dot); 
static int read_int_option (int argc, char *argv[], int iarg);
static void free_globals();

int main (int argc, char *argv[]) {

 char title[] = "\nMPack Version 1.5c by PE. Gaillardon\n"
                "Netlist translator and matrix logic block packer\n"
                "Inspired from VPACK 2.09 by V. Betz\n"
                "compiled " __DATE__ "\n"
                "This code is licensed for non-commercial use only.\n\n";

 char blif_file[BUFSIZE], output_file[BUFSIZE];
 boolean global_clocks, echo_dot;
 int matrix_width, matrix_depth, matrix_topo;
 int cell_size;

 clock_t begin, end;

 boolean *is_clock;   // [0..num_nets-1] TRUE if a clock.

 printf("%s",title);

 parse_command (argc, argv, blif_file, output_file, &global_clocks, &matrix_width, &matrix_depth, &matrix_topo, &cell_size, &echo_dot);

 read_blif (blif_file, cell_size);

 mkdir("./WORK",0755);
 mkdir("./WORK/FIG",0755);

 is_clock = alloc_and_load_is_clock (global_clocks);

 begin = clock();
 do_clustering (matrix_width, matrix_depth, matrix_topo, cell_size, global_clocks, is_clock, output_file, echo_dot);
 end = clock();

 free (is_clock);
 free_globals();

 printf("\nNetlist conversion complete.\n\n");
 #ifdef CLOCKS_PER_SEC
   printf("Netlist conversion took %g seconds\n", (float)(end - begin) / CLOCKS_PER_SEC);
 #else
   printf("Netlist conversion took %g seconds\n", (float)(end - begin) / CLK_PER_SEC);
 #endif

 return (EXIT_SUCCESS);
}

// Parse the command line to determine user options.
static void parse_command (int argc, char *argv[], char *blif_file, char *output_file, boolean *global_clocks, int *matrix_width, int *matrix_depth, int *matrix_topo, int *cell_size, boolean *echo_dot) {
 int i;

 if (argc < 3) {
    printf("Usage:  mpack input.blif output_basename\n");
    printf("\t[-matrix_width <int>] [-matrix_depth <int>]\n");
    printf("\t[-global_clocks on|off]\n");
    printf("\t[-echo_dot on|off] \n");
    printf("\n");
    exit(EXIT_FAILURE);
 }

// Set defaults.

 *cell_size = 2; /// Dynamic input size not supported yet
 *global_clocks = TRUE;
 *matrix_width = 2;
 *matrix_depth = 2;
 *matrix_topo = OMEGA; /// Single omega topology supported
 *echo_dot =TRUE;

// Start parsing the command line.

// First two arguments are mandatory.
 strncpy (blif_file, argv[1], BUFSIZE);
 strncpy (output_file, argv[2], BUFSIZE);
 i = 3;

// Now, get any optional arguments.
 while (i < argc) {
    
//     if (strcmp (argv[i], "-cell_size") == 0) {
//        *cell_size = read_int_option (argc, argv, i);
//        if (*cell_size < 2 || *cell_size > GATE_MAX_INPUT_NB) {
//           fprintf(stderr, "Error: cell_size must be between 2 and GATE_MAX_INPUT_NB (%d).\n",GATE_MAX_INPUT_NB);
//           exit(EXIT_FAILURE);
//        }
//        i += 2;
//        continue;
//     }

    if (strcmp (argv[i],"-global_clocks") == 0) {
       if (argc <= i+1) {
          fprintf (stderr, "Error: -global_clocks option requires a string parameter.\n"); 
          exit(EXIT_FAILURE);    
       } 
       if (strcmp(argv[i+1], "on") == 0) {
          *global_clocks = TRUE;
       } 
       else if (strcmp(argv[i+1], "off") == 0) {
          *global_clocks = FALSE;
       } 
       else {
          fprintf(stderr, "Error: -global_clocks must be on or off.\n");
          exit(EXIT_FAILURE);
       } 
       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-matrix_width") == 0) {
       *matrix_width = read_int_option (argc, argv, i);
       if (*matrix_width < 1) {
          fprintf(stderr, "Error: matrix_width must be greater than 0.\n");
          exit(EXIT_FAILURE);
       }
       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-matrix_depth") == 0) {
       *matrix_depth = read_int_option (argc, argv, i);
       if (*matrix_depth < 1) {
          fprintf(stderr, "Error: matrix_depth must be greater than 0.\n");
          exit(EXIT_FAILURE);
       }
       i += 2;
       continue;
    }

//     if (strcmp (argv[i],"-matrix_topo") == 0) {
//        if (argc <= i+1) {
//           fprintf (stderr, "Error:  -matrix_topo option requires a string parameter.\n"); 
//           exit(EXIT_FAILURE);    
//        } 
//        if (strcmp(argv[i+1], "omega") == 0) {
//           *matrix_topo = OMEGA;
//        }
// /// Support for other topologies       
// //        else if (strcmp(argv[i+1], "banyan") == 0) {
// //           *matrix_topo = BANYAN;
// //        } 
//        else {
//           fprintf(stderr,"Error: -matrix_topo must be omega.\n");
//           exit(EXIT_FAILURE);
//        } 
//        i += 2;
//        continue;
//     }

    if (strcmp (argv[i],"-echo_dot") == 0) {
       if (argc <= i+1) {
          fprintf (stderr, "Error: -echo_dot option requires a string parameter.\n"); 
          exit(EXIT_FAILURE);    
       } 
       if (strcmp(argv[i+1], "on") == 0) {
          *echo_dot = TRUE;
       } 
       else if (strcmp(argv[i+1], "off") == 0) {
          *echo_dot = FALSE;
       } 
       else {
          fprintf(stderr,"Error: -echo_dot must be on or off.\n");
          exit(EXIT_FAILURE);
       } 
       i += 2;
       continue;
    }

    fprintf(stderr, "Unrecognized option: %s.  Aborting.\n",argv[i]);
    exit(EXIT_FAILURE);

 } // End of options while loop.

 // Echo back selected options.
 printf("Selected options:\n\n");
 printf("\tCell Size: %d inputs\n", *cell_size);
 printf("\tMatrix size: %d width, %d depth \n", *matrix_width, *matrix_depth);
 printf("\tMatrix topo: %s\n", (*matrix_topo) ? "Banyan" : "Omega"); /// Support for more topologies

 printf("\tClocks Routed via Dedicated Resource: %s\n", (*global_clocks) ? "Yes" : "No");
 printf("\n");
 
 if(*echo_dot==TRUE)
 {
    printf("Echo dot ON\n");
    printf("Use dot to generate the functions graphs : dot -n -Tsvg file.dot -o file.svg\n");     
    printf("Use neato to generate the mapping graphs : neato -n -Tsvg file.dot -o file.svg\n");
    printf("\n");
 }
}

// This routine returns the value in argv[iarg+1].
// This value must exist and be an integer, or an error message is printed and the program exits.
static int read_int_option (int argc, char *argv[], int iarg) {
 int value, num_read;
 
 num_read = 0;
 
// Does value exist for this option?
 
 if (argc > iarg+1)
    num_read = sscanf(argv[iarg+1],"%d",&value);
 
// Value exists and was a proper int?
 
 if (num_read != 1) {
    fprintf(stderr, "Error:  %s option requires an integer parameter.\n\n", argv[iarg]);
    exit(EXIT_FAILURE);
 }
 
 return (value);
}

static void free_globals() {
   int i;

   for(i=0; i<num_blocks; i++)
   {
      if(block[i].name!=NULL) free(block[i].name);
   }
   free(block);
   
   for(i=0; i<num_blocks_validated; i++)
   {
       if(block_validated[i].name!=NULL) free(block_validated[i].name);
    }
   free(block_validated);
   
   for(i=0; i<num_blocks_current; i++)
   {
       if(block_current[i].name!=NULL) free(block_current[i].name);
    }
   free(block_current);

   for(i=0; i<num_nets; i++)
   {
      if(net[i].pins!=NULL) free(net[i].pins);
      if(net[i].name!=NULL) free(net[i].name);
   }
   free(net);
   
   for(i=0; i<num_nets_validated; i++)
   {
      if(net_validated[i].pins!=NULL) free(net_validated[i].pins);
      if(net_validated[i].name!=NULL) free(net_validated[i].name);
   }
   free(net_validated);

   for(i=0; i<num_nets_current; i++)
   {
      if(net_current[i].pins!=NULL) free(net_current[i].pins);
      if(net_current[i].name!=NULL) free(net_current[i].name);
   }
   free(net_current);
}
