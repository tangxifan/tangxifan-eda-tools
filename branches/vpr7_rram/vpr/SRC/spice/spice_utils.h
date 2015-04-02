void my_free(void* ptr);

char* my_gettime();

char* format_dir_path(char* dir_path);

int try_access_file(char* file_path);

void my_remove_file(char* file_path);

int create_dir_path(char* dir_path);

char* my_strcat(char* str1,
                char* str2);

int split_path_prog_name(char* prog_path,
                         char  split_token,
                         char** ret_path,
                         char** ret_prog_name);

char* chomp_file_name_postfix(char* file_name);

t_spice_model* find_name_matched_spice_model(char* spice_model_name,
                                             int num_spice_model,
                                             t_spice_model* spice_models);

t_spice_model* get_default_spice_model(enum e_spice_model_type default_spice_model_type,
                                       int num_spice_model,
                                       t_spice_model* spice_models);

t_spice_model_port** find_spice_model_ports(t_spice_model* spice_model,
                                            enum e_spice_model_port_type port_type,
                                            int* port_num);

void fprint_spice_head(FILE* fp,
                       char* usage);

t_spice_transistor_type* find_mosfet_tech_lib(t_spice_tech_lib tech_lib,
                                              e_spice_trans_type trans_type);

char* my_itoa(int input);

char* chomp_spice_node_prefix(char* spice_node_prefix);

char* format_spice_node_prefix(char* spice_node_prefix);

t_port* find_pb_type_port_match_spice_model_port(t_pb_type* pb_type,
                                                 t_spice_model_port* spice_model_port);

char* format_spice_node_prefix(char* spice_node_prefix);

t_port** find_pb_type_ports_match_spice_model_port_type(t_pb_type* pb_type,
                                                        enum e_spice_model_port_type port_type,
                                                        int* port_num);


t_block* search_mapped_block(int x, int y, int z);

int determine_mux_level(int mux_size);

int mux_last_level_input_num(int num_level,
                             int mux_size);

int determine_lut_path_id(int lut_size,
                          int* lut_inputs);

int* decode_mux_sram_bits(int fan_in,
                          int mux_level,
                          int path_id);

char** my_strtok(char* str, 
                 char* delims, 
                 int* len);

char* convert_side_index_to_string(int side);


void init_spice_net_info(t_spice_net_info* spice_net_info);

t_spice_model* find_inpad_spice_model(int num_spice_model,
                                      t_spice_model* spice_models);

char* generate_string_spice_model_type(enum e_spice_model_type spice_model_type);

int determine_io_grid_side(int x,
                           int y);

void find_prev_rr_nodes_with_src(t_rr_node* src_rr_node,
                                 int* num_drive_rr_nodes,
                                 t_rr_node*** drive_rr_nodes,
                                 int** switch_indices);

int find_path_id_prev_rr_node(int num_drive_rr_nodes,
                              t_rr_node** drive_rr_nodes,
                              t_rr_node* src_rr_node);

float pb_pin_density(t_rr_node* pb_rr_graph, 
                     t_pb_graph_pin* pin);

float pb_pin_probability(t_rr_node* pb_rr_graph, 
                         t_pb_graph_pin* pin);

int pb_pin_init_value(t_rr_node* pb_rr_graph, 
                      t_pb_graph_pin* pin);

float get_rr_node_net_density(t_rr_node node);

float get_rr_node_net_probability(t_rr_node node);

int get_rr_node_net_init_value(t_rr_node node);

void fprint_voltage_pulse_params(FILE* fp,
                                 int init_val,
                                 float density,
                                 float probability);

int find_parent_pb_type_child_index(t_pb_type* parent_pb_type,
                                    int mode_index,
                                    t_pb_type* child_pb_type);

void gen_spice_name_tag_pb_rec(t_pb* cur_pb,
                               char* prefix);

void gen_spice_name_tags_all_pbs();

void check_pb_graph_edge(t_pb_graph_edge pb_graph_edge);

void check_pb_graph_pin_edges(t_pb_graph_pin pb_graph_pin);

void backup_one_pb_rr_node_pack_prev_node_edge(t_rr_node* pb_rr_node);

void update_one_grid_pack_prev_node_edge(int x, int y);

void update_grid_pbs_post_route_rr_graph();

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

void map_clb_pins_to_pb_graph_pins();

int recommend_num_sim_clock_cycle();

void auto_select_num_sim_clock_cycle(t_spice* spice);
