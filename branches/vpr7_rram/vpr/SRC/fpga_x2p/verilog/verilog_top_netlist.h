
void dump_verilog_top_netlist_ports(t_sram_orgz_info* cur_sram_orgz_info,
                                    FILE* fp,
                                    int num_clocks,
                                    char* circuit_name,
                                    t_spice verilog);

void dump_verilog_top_netlist_internal_wires(t_sram_orgz_info* cur_sram_orgz_info,
                                             FILE* fp);

void dump_verilog_defined_channels(FILE* fp,
                                   int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                   t_ivec*** LL_rr_node_indices);

void dump_verilog_defined_grids(t_sram_orgz_info* cur_sram_orgz_info,
                                FILE* fp);

void dump_verilog_defined_connection_boxes(t_sram_orgz_info* cur_sram_orgz_info,
                                           FILE* fp);

void dump_verilog_defined_switch_boxes(t_sram_orgz_info* cur_sram_orgz_info,
                                       FILE* fp);

void dump_verilog_clb2clb_directs(FILE* fp, 
                                  int num_directs, t_clb_to_clb_directs* direct);

void dump_verilog_configuration_circuits(t_sram_orgz_info* cur_sram_orgz_info,
                                         FILE* fp);

void dump_verilog_top_module_ports(t_sram_orgz_info* cur_sram_orgz_info, 
                                   FILE* fp,
                                   enum e_dump_verilog_port_type dump_port_type);

void dump_verilog_top_netlist(t_sram_orgz_info* cur_sram_orgz_info,
                              char* circuit_name,
                              char* top_netlist_name,
                              char* include_dir_path,
                              char* subckt_dir_path,
                              int LL_num_rr_nodes,
                              t_rr_node* LL_rr_node,
                              t_ivec*** LL_rr_node_indices,
                              int num_clock,
                              t_spice spice);


