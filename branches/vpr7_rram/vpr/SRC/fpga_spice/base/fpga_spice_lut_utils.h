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

char** assign_post_routing_lut_truth_table(t_logical_block* mapped_logical_block,
                                           int num_lut_pins, int* lut_pin_vpack_net_num,
                                           int* truth_table_length);

int determine_lut_path_id(int lut_size,
                          int* lut_inputs);

