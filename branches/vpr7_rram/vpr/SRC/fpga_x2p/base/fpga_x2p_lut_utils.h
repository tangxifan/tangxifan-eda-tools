/* Useful functions for LUT decoding */
char* complete_truth_table_line(int lut_size,
                                char* input_truth_table_line);

void configure_lut_sram_bits_per_line_rec(int** sram_bits, 
                                          int lut_size,
                                          char* truth_table_line,
                                          int start_point);

int* generate_lut_sram_bits(int truth_table_len,
                            char** truth_table,
                            int lut_size,
                            int default_sram_bit_value);

char** assign_lut_truth_table(t_logical_block* mapped_logical_block,
                              int* truth_table_length);

char** get_wired_lut_truth_table();

char** assign_post_routing_lut_truth_table(t_logical_block* mapped_logical_block,
                                           int num_lut_pins, int* lut_pin_vpack_net_num,
                                           int* truth_table_length);

t_pb_graph_pin* get_mapped_lut_phy_pb_output_pin(t_phy_pb* lut_phy_pb, 
                                                 t_logical_block* lut_logical_block);

int get_pb_graph_pin_lut_frac_level(t_pb_graph_pin* out_pb_graph_pin);

int get_pb_graph_pin_lut_output_mask(t_pb_graph_pin* out_pb_graph_pin);

void adapt_truth_table_for_frac_lut(t_phy_pb* prim_phy_pb, 
                                    t_logical_block* lut_logical_block, 
                                    int truth_table_length, 
                                    char** truth_table);

int determine_lut_path_id(int lut_size,
                          int* lut_inputs);

