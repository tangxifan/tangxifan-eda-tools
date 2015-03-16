// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon

// Clustering of the gates into MClusters

// This source file will greedily fill the clusters with gates. The legitimity test will be done by the matrix packer.
// Most of the selection algorithm and speed up tricks are adapted from original VPack

#ifndef CLUSTER_H_
#define CLUSTER_H_

void do_clustering (int matrix_width, int matrix_depth, int matrix_topo, int cell_size, boolean global_clocks, boolean *is_clock, char *out_fname, boolean echo_dot);
int num_input_pins (int iblk); 

#endif

