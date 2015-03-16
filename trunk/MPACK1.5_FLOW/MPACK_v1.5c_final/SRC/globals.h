// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon
// Global Variables 

#ifndef GLOBALS_H_
#define GLOBALS_H_

// Original VPACK variables (net, block, num_nets, ...) contains the initial netlist
// Validated versions of the variables (block_validated) contains the current netlist informations ( updated once each cluster is fully filled)
// Current versions of the variables (block_current) are the working copies of the variables (modified on the fly during the mapping process)
// Rollback versions of the variables are used to revert the previous state during the iterative mapping phase

// Netlist description data structures (initial numbers, numbers after mapping --> will increase with the packing process)
extern int num_nets, num_nets_validated, num_nets_current;
extern int num_blocks, num_blocks_validated, num_blocks_current;     
extern int num_p_inputs, num_p_outputs;

// Number in original netlist, before FF packing.
extern int num_gates, num_latches; 

// Number of additional buffers in the MCluster structure (+temporary structures to support reverting the process back)
extern int buffer_counting_validated, buffer_counting_current, buffer_counting_rollback;

// Physical structures (initial numbers, numbers after mapping --> increases while adding buffers in the matrices)
extern struct s_net *net, *net_validated,*net_current;

extern struct s_block *block, *block_validated, *block_current;

#endif
