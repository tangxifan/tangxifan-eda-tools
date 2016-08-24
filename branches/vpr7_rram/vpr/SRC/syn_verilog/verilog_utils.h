

void dump_verilog_file_header(FILE* fp,
                              char* usage);

void decode_verilog_rram_mux(t_spice_model* mux_spice_model,
                             int mux_size, int path_id,
                             int* bit_len, int** conf_bits, int* mux_level);

int determine_decoder_size(int num_addr_out);
