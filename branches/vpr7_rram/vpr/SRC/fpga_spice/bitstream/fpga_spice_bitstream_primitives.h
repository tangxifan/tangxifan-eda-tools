


void fpga_spice_generate_bitstream_pb_primitive_ff(t_logical_block* mapped_logical_block,
                                                  t_pb_graph_node* prim_pb_graph_node,
                                                  int index,
                                                  t_spice_model* verilog_model,
                                                  t_sram_orgz_info* cur_sram_orgz_info) ;

void fpga_spice_generate_bitstream_pb_primitive_hardlogic(t_logical_block* mapped_logical_block,
                                                          t_pb_graph_node* prim_pb_graph_node,
                                                          int index,
                                                          t_spice_model* verilog_model,
                                                          t_sram_orgz_info* cur_sram_orgz_info) ;

void fpga_spice_generate_bitstream_pb_primitive_io(t_logical_block* mapped_logical_block,
                                                   t_pb_graph_node* prim_pb_graph_node,
                                                   int index,
                                                   t_spice_model* verilog_model,
                                                   t_sram_orgz_info* cur_sram_orgz_info) ;

void fpga_spice_generate_bitstream_pb_primitive_lut(t_logical_block* mapped_logical_block,
                                                    t_pb_graph_node* cur_pb_graph_node,
                                                    int index,
                                                    t_spice_model* verilog_model,
                                                    t_sram_orgz_info* cur_sram_orgz_info) ;

