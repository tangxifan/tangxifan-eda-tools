
void dump_compact_verilog_logic_blocks(char* subckt_dir,
                                       t_arch* arch);

void dump_compact_verilog_top_netlist(char* circuit_name,
                                      char* top_netlist_name,
                                      char* include_dir_path,
                                      char* subckt_dir_path,
                                      int LL_num_rr_nodes,
                                      t_rr_node* LL_rr_node,
                                      t_ivec*** LL_rr_node_indices,
                                      int num_clock,
                                      t_spice verilog);
