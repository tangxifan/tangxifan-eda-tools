void check_pb_graph_edge(t_pb_graph_edge pb_graph_edge);

void check_pb_graph_pin_edges(t_pb_graph_pin pb_graph_pin);

void backup_one_pb_rr_node_pack_prev_node_edge(t_rr_node* pb_rr_node);

int find_parent_pb_type_child_index(t_pb_type* parent_pb_type,
                                    int mode_index,
                                    t_pb_type* child_pb_type);

void gen_spice_name_tag_pb_rec(t_pb* cur_pb,
                               char* prefix);

void gen_spice_name_tags_all_pbs();

int find_pb_mapped_logical_block_rec(t_pb* cur_pb,
                                     t_spice_model* pb_spice_model, 
                                     char* pb_spice_name_tag);

int find_grid_mapped_logical_block(int x, int y,
                                   t_spice_model* pb_spice_model,
                                   char* pb_spice_name_tag);

void stats_pb_graph_node_port_pin_numbers(t_pb_graph_node* cur_pb_graph_node,
                                          int* num_inputs,
                                          int* num_outputs,
                                          int* num_clock_pins);

int find_pb_type_idle_mode_index(t_pb_type cur_pb_type);

int find_pb_type_physical_mode_index(t_pb_type cur_pb_type);

void mark_grid_type_pb_graph_node_pins_temp_net_num(int x, int y);

void assign_pb_graph_node_pin_temp_net_num_by_mode_index(t_pb_graph_pin* cur_pb_graph_pin,
                                                         int mode_index);

void mark_pb_graph_node_input_pins_temp_net_num(t_pb_graph_node* cur_pb_graph_node,
                                                int mode_index);

void mark_pb_graph_node_clock_pins_temp_net_num(t_pb_graph_node* cur_pb_graph_node,
                                                int mode_index);

void mark_pb_graph_node_output_pins_temp_net_num(t_pb_graph_node* cur_pb_graph_node,
                                                int mode_index);

void rec_mark_pb_graph_node_temp_net_num(t_pb_graph_node* cur_pb_graph_node);

void load_one_pb_graph_pin_temp_net_num_from_pb(t_pb* cur_pb,
                                                t_pb_graph_pin* cur_pb_graph_pin);

void load_pb_graph_node_temp_net_num_from_pb(t_pb* cur_pb);

void rec_mark_one_pb_unused_pb_graph_node_temp_net_num(t_pb* cur_pb);

void update_pb_vpack_net_num_from_temp_net_num(t_pb* cur_pb, 
                                               t_pb_graph_pin* cur_pb_graph_pin);

void update_pb_graph_node_temp_net_num_to_pb(t_pb_graph_node* cur_pb_graph_node,
                                             t_pb* cur_pb);

void rec_load_unused_pb_graph_node_temp_net_num_to_pb(t_pb* cur_pb);

void mark_one_pb_parasitic_nets(t_pb* cur_pb);

int count_num_conf_bit_one_interc(t_interconnect* cur_interc,
                                  enum e_sram_orgz cur_sram_orgz_type);

int count_num_reserved_conf_bit_one_interc(t_interconnect* cur_interc,
                                           enum e_sram_orgz cur_sram_orgz_type);

int count_num_conf_bits_pb_type_mode_interc(t_mode* cur_pb_type_mode,
                                            enum e_sram_orgz cur_sram_orgz_type);

int rec_count_num_conf_bits_pb_type_default_mode(t_pb_type* cur_pb_type,
                                                 t_sram_orgz_info* cur_sram_orgz_info);

int rec_count_num_conf_bits_pb_type_physical_mode(t_pb_type* cur_pb_type,
                                                  t_sram_orgz_info* cur_sram_orgz_info);

int rec_count_num_conf_bits_pb(t_pb* cur_pb,
                               t_sram_orgz_info* cur_sram_orgz_info);

void init_one_grid_num_conf_bits(int ix, int iy,
                                 t_sram_orgz_info* cur_sram_orgz_info);

void init_grids_num_conf_bits(t_sram_orgz_info* cur_sram_orgz_info);

void map_clb_pins_to_pb_graph_pins();

t_port* find_pb_type_port_match_spice_model_port(t_pb_type* pb_type,
                                                 t_spice_model_port* spice_model_port);

t_port** find_pb_type_ports_match_spice_model_port_type(t_pb_type* pb_type,
                                                        enum e_spice_model_port_type port_type,
                                                        int* port_num);


enum e_interconnect find_pb_graph_pin_in_edges_interc_type(t_pb_graph_pin pb_graph_pin);

t_spice_model* find_pb_graph_pin_in_edges_interc_spice_model(t_pb_graph_pin pb_graph_pin);

int find_path_id_between_pb_rr_nodes(t_rr_node* local_rr_graph,
                                     int src_node,
                                     int des_node);

t_pb* get_child_pb_for_phy_pb_graph_node(t_pb* cur_pb, int ipb, int jpb);


enum e_interconnect find_pb_graph_pin_in_edges_interc_type(t_pb_graph_pin pb_graph_pin) ;

t_spice_model* find_pb_graph_pin_in_edges_interc_model(t_pb_graph_pin pb_graph_pin) ;

void find_interc_fan_in_des_pb_graph_pin(t_pb_graph_pin* des_pb_graph_pin,
                                         t_mode* cur_mode,
                                         t_interconnect** cur_interc,
                                         int* fan_in) ; 

void rec_count_num_iopads_pb_type_physical_mode(t_pb_type* cur_pb_type);

void rec_count_num_iopads_pb_type_default_mode(t_pb_type* cur_pb_type);

void rec_count_num_iopads_pb(t_pb* cur_pb);

void init_one_grid_num_iopads(int ix, int iy);

void init_grids_num_iopads();

void rec_count_num_mode_bits_pb_type_default_mode(t_pb_type* cur_pb_type);

void rec_count_num_mode_bits_pb(t_pb* cur_pb);

void init_one_grid_num_mode_bits(int ix, int iy);

void init_grids_num_mode_bits();

t_pb* get_lut_child_pb(t_pb* cur_lut_pb,
                       int mode_index);

t_pb* get_hardlogic_child_pb(t_pb* cur_hardlogic_pb,
                             int mode_index);

int get_grid_pin_height(int grid_x, int grid_y, int pin_index);

int get_grid_pin_side(int grid_x, int grid_y, int pin_index);

int* decode_mode_bits(char* mode_bits, int* num_sram_bits);

enum e_interconnect determine_actual_pb_interc_type(t_interconnect* def_interc, 
                                                    int fan_in) ;


