
void dump_verilog_pb_primitive_ff(t_sram_orgz_info* cur_sram_orgz_info,
                                  FILE* fp,
                                  char* subckt_prefix,
                                  t_logical_block* mapped_logical_block,
                                  t_pb_graph_node* prim_pb_graph_node,
                                  int index,
                                  t_spice_model* spice_model);


void dump_verilog_pb_primitive_hardlogic(t_sram_orgz_info* cur_sram_orgz_info,
                                         FILE* fp,
                                         char* subckt_prefix,
                                         t_logical_block* mapped_logical_block,
                                         t_pb_graph_node* prim_pb_graph_node,
                                         int index,
                                         t_spice_model* spice_model);

void dump_verilog_pb_primitive_io(t_sram_orgz_info* cur_sram_orgz_info,
                                  FILE* fp,
                                  char* subckt_prefix,
                                  t_logical_block* mapped_logical_block,
                                  t_pb_graph_node* prim_pb_graph_node,
                                  int index,
                                  t_spice_model* spice_model);
