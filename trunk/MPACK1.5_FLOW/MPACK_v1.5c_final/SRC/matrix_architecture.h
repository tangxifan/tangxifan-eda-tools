// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon

// Matrix architecture management library: part of the mapper

#ifndef MATRIX_ARCHITECTURE_H_
#define MATRIX_ARCHITECTURE_H_

// DATA STRUCTURES 

// Name of the different interconnect topologies 
/// Only OMEGA is supported
enum interconnect_topology {OMEGA, BANYAN, FLIP, BASELINE, NO};

// Physical MCluster data structure
typedef struct matrix{
   enum interconnect_topology topo;
   int width; // Assuming square matrices: dimensions of the matrix
   int depth;
   int ** adjacency_matrix; // Adjacency matrix of the cluster graph structure
   // (adjcency_natrix[i][j]=1 means that cell the output of cell i is wired to the input of j --> directed connections only)
   int adjm_size; // Adjacency matrix size.
   int ** fconm; // Extended adjacency matrix representing both upstream and downstream interconnections (all connections)
   int * levels; // Level of a given cell in the MCluster pipeline
   int ** terminal_interconnect_matrix; // Adjacency matrix representing the terminals instead of the nodes
} matrix;

// DATA MANAGEMENT
void init_architecture(matrix *arch);
void free_architecture(matrix *arch);

// DOT GENERATION
void dot_print_architecture(matrix *arch);

// UTILITY FUNCTIONS
int number_of_outputs(matrix *arch);
int number_of_inputs(matrix *arch);
int number_of_nodes(matrix *arch);
int number_of_nets(matrix * arch);

// MAIN ROUTINE
void create_architecture(matrix *arch, enum interconnect_topology topo, int width, int depth);

#endif
