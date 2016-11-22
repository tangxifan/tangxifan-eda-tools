

void init_list_include_verilog_netlists(t_spice* spice);

void init_include_user_defined_verilog_netlists(t_spice spice);
 
void dump_include_user_defined_verilog_netlists(FILE* fp,
                                                t_spice spice);

void dump_verilog_file_header(FILE* fp,
                              char* usage);

void decode_verilog_one_level_4t1r_mux(int path_id, 
                                       int bit_len, int* conf_bits); 

void decode_verilog_rram_mux(t_spice_model* mux_spice_model,
                             int mux_size, int path_id,
                             int* bit_len, int** conf_bits, int* mux_level);

int determine_decoder_size(int num_addr_out);

char* chomp_verilog_node_prefix(char* verilog_node_prefix);

char* format_verilog_node_prefix(char* verilog_node_prefix);

t_conf_bit_info* dump_verilog_find_paired_conf_bit(t_llist* conf_bits_llist_head,
                                                   t_conf_bit_info* src_conf_bit_info);

void dump_verilog_gen_pairs_conf_bits(t_llist* conf_bits_llist_head);

char* verilog_convert_port_type_to_string(enum e_spice_model_port_type port_type);

int rec_dump_verilog_spice_model_global_ports(FILE* fp, 
                                              t_spice_model* cur_spice_model,
                                              boolean dump_port_type, boolean recursive);

int dump_verilog_global_ports(FILE* fp, t_llist* head,
                              boolean dump_port_type);
