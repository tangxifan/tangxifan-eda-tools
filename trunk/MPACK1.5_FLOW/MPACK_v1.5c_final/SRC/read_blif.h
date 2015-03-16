// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon

// Input netlist read and data structures allocation


#ifndef READ_BLIF_H_
#define READ_BLIF_H_

void read_blif (char *blif_file, int cell_size);
boolean *alloc_and_load_is_clock (boolean global_clocks);
void echo_input (char *blif_file, char *echo_file); 

#endif
