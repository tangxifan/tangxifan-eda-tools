
void dump_verilog_top_testbench_global_ports(FILE* fp, t_llist* head,
                                             enum e_dump_verilog_port_type dump_port_type);

void dump_verilog_top_testbench(t_sram_orgz_info* cur_sram_orgz_info,
                                char* circuit_name,
                                char* top_netlist_name,
                                int num_clock,
                                t_syn_verilog_opts syn_verilog_opts,
                                t_spice verilog);

void dump_verilog_input_blif_testbench(char* circuit_name,
                                       char* top_netlist_name,
                                       int num_clock,
                                       t_syn_verilog_opts syn_verilog_opts,
                                       t_spice verilog);
