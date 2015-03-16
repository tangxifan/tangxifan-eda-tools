// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon
// Mapper module: Logic function management - graph representation - function formatting


#ifndef FUNCTION_H_
#define FUNCTION_H_

// Logic function data structure
typedef struct function{
   int nb_nodes; // Number of logic gates (from the initial netlist)
   int nb_input; // Number of inputs of the function (from nets)
   int nb_output; // Number of outputs of the function (from nets)
   int nb_buf; // Number of inserted buffers (after formatting)
   int msize; // Size of the adjacency matrix
   int * levels; // Levels (logic depth) of the different nodes
   int * original_netlist_idx; // Correspondance of the nodes to the block indices of the netlist
   char **names; // Names of the logic nodes
   int ** adjm; // adjacency matrix
   int ** fconm; // full connectivity matrix
} function;

// DATA MANAGEMENT
void init_function (function * fct);
void free_function (function * fct);
void copy_function (function * fctd, function * fcts);

// PRIMITIVES
void autofill_level (function *fct);
void update_node_count(function *fct);
int add_node (function *fct, int level, int register_buf);

// DOT GENERATION
void dot_print_function(function *fct, char name[]);
void dot_print_function_with_name(function *fct, char name[]);

// FUNCTION FORMATING
void input_level_correction(function *fct);
void output_level_correction(function *fct);
void feedback_correction(function *fct);
void check_inter_level_connections(function * fct);
void unique_output_connection_correction(function * fct);
void unique_input_connection_correction(function * fct);
void delete_multiple_connections_to_output(function * fct);
void sort_by_level(function * fct);
void jump_correction(function * fct);
void extended_output_level_correction(function * fct, int wlevel);

void function_formatting(function *fct, int max_depth);

void fill_test_fct1 (function * fct);
#endif
