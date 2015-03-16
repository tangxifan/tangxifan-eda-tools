// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon
// Packer module: Maps a logic function graph on a matrix cluster

#include "mpack.h"
#include "matrix_architecture.h"
#include "function.h"


#ifndef MATRIX_PACKER_H_
#define MATRIX_PACKER_H_

// Mapping data structure
typedef struct mapping{
   matrix *arch; // The physical matrix to map on
   function *function; // The function to map
   int * map; // The assignements of the blocks on the cells available in arch
}mapping;

// DATA MANAGEMENT
void free_mapping(mapping *map);
void init_mapping(mapping *map, matrix *arch, function * fct);

// DOT GENERATION
void dot_print_mapping(mapping *map, char filename[]);
void dot_print_mapping_with_name(mapping *map, char filename[]);

// ARCHITECTURE TO FUNCTION FORMATING
void function_formatting(function *fct, int max_depth);

// MAIN ROUTINE
int matrix_mapping(matrix *arch, function * fct, int * matrix_content, boolean echo_dot, char *filename);

// SUB ROUTINES
void function_node_sort (function *fct, int ** list);

#endif
