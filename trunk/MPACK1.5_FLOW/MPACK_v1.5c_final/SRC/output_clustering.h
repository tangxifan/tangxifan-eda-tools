// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon
// Output clustering informations (netlist, mappings, ...)

#ifndef OUTPUT_CLUSTERING_H_
#define OUTPUT_CLUSTERING_H_

#include "matrix_architecture.h"

// Utility and debug
void echo_debug_validated (char *blif_file, char *echo_file); 
void echo_debug_current (char *blif_file, char *echo_file); 

// Mapping outputs
void print_blif_cluster (matrix *arch, int num_clusters, int **matrix_contents, boolean global_clocks, boolean *is_clock, char *out_fname);
void print_blif_gate (matrix *arch, int num_clusters, int **matrix_contents, boolean global_clocks, boolean *is_clock, char *out_fname);
void print_blif_names (matrix *arch, int num_clusters, int **matrix_contents, boolean global_clocks, boolean *is_clock, char *out_fname);
void print_rpt_mapping (matrix *arch, int num_clusters, int **matrix_contents, boolean global_clocks, boolean *is_clock, char *out_fname);

// Statistics
void mapping_stat (matrix *arch, int num_clusters, int **matrix_contents);


#endif
