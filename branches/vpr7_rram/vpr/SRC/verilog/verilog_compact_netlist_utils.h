
void verilog_compact_generate_fake_xy_for_io_border_side(int border_side,  
                                                         int* ix, int* iy) ;

void dump_compact_verilog_grid_pins(FILE* fp,
                                    t_type_ptr grid_type_descriptor,
                                    boolean dump_port_type,
                                    boolean dump_last_comma) ;

void dump_compact_verilog_io_grid_pins(FILE* fp,
                                       t_type_ptr grid_type_descriptor,
                                       int border_side,
                                       boolean dump_port_type,
                                       boolean dump_last_comma) ;

char* compact_verilog_get_grid_phy_block_subckt_name(t_type_ptr grid_type_descriptor,
                                                     int z,
                                                     char* subckt_prefix);

void dump_compact_verilog_io_grid_block_subckt_pins(FILE* fp,
                                                    t_type_ptr grid_type_descriptor,
                                                    int border_side,
                                                    int z) ;
