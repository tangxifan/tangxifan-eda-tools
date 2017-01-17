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

void config_spice_model_input_output_buffers_pass_gate(int num_spice_models, 
                                                       t_spice_model* spice_model);

t_spice_model_port** find_spice_model_ports(t_spice_model* spice_model,
                                            enum e_spice_model_port_type port_type,
                                            int* port_num, boolean ignore_global_port);

void fprint_spice_head(FILE* fp,
                       char* usage);

void fprint_commented_sram_bits(FILE* fp,
                                int num_sram_bits, int* sram_bits);

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


int determine_tree_mux_level(int mux_size);

int determine_num_input_basis_multilevel_mux(int mux_size,
                                             int mux_level);

int tree_mux_last_level_input_num(int num_level,
                                  int mux_size);

int multilevel_mux_last_level_input_num(int num_level, int num_input_per_unit,
                                        int mux_size);

int determine_lut_path_id(int lut_size,
                          int* lut_inputs);

int* decode_onelevel_mux_sram_bits(int fan_in,
                                   int mux_level,
                                   int path_id);

int* decode_multilevel_mux_sram_bits(int fan_in,
                                     int mux_level,
                                     int path_id);

int* decode_tree_mux_sram_bits(int fan_in,
                               int mux_level,
                               int path_id);

void decode_cmos_mux_sram_bits(t_spice_model* mux_spice_model,
                               int mux_size, int path_id, 
                               int* bit_len, int** conf_bits, int* mux_level);

char** my_strtok(char* str, 
                 char* delims, 
                 int* len);

char* convert_side_index_to_string(int side);

char* convert_chan_type_to_string(t_rr_type chan_type);

char* convert_chan_rr_node_direction_to_string(enum PORTS chan_rr_node_direction);

void init_spice_net_info(t_spice_net_info* spice_net_info);

t_spice_model* find_inpad_spice_model(int num_spice_model,
                                      t_spice_model* spice_models);

t_spice_model* find_outpad_spice_model(int num_spice_model,
                                       t_spice_model* spice_models);

t_spice_model* find_iopad_spice_model(int num_spice_model,
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

int pb_pin_net_num(t_rr_node* pb_rr_graph, 
                   t_pb_graph_pin* pin);

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

void alloc_spice_model_grid_index_low_high(t_spice_model* cur_spice_model);

void free_one_spice_model_grid_index_low_high(t_spice_model* cur_spice_model);

void free_spice_model_grid_index_low_high(int num_spice_models, 
                                          t_spice_model* spice_model);

void update_one_spice_model_grid_index_low(int x, int y, 
                                           t_spice_model* cur_spice_model);

void update_spice_models_grid_index_low(int x, int y, 
                                        int num_spice_models, 
                                        t_spice_model* spice_model);

void update_one_spice_model_grid_index_high(int x, int y, 
                                           t_spice_model* cur_spice_model);

void update_spice_models_grid_index_high(int x, int y, 
                                        int num_spice_models, 
                                        t_spice_model* spice_model);

void zero_one_spice_model_grid_index_low_high(t_spice_model* cur_spice_model);

void zero_spice_model_grid_index_low_high(int num_spice_models, 
                                          t_spice_model* spice_model);

char* gen_str_spice_model_structure(enum e_spice_model_structure spice_model_structure);

boolean check_spice_model_structure_match_switch_inf(t_switch_inf target_switch_inf);

int find_pb_type_idle_mode_index(t_pb_type cur_pb_type);

int find_pb_type_physical_mode_index(t_pb_type cur_pb_type);

void mark_grid_type_pb_graph_node_pins_temp_net_num(int x, int y);

void rec_mark_pb_graph_node_temp_net_num(t_pb_graph_node* cur_pb_graph_node);

int check_consistency_logical_block_net_num(t_logical_block* lgk_blk, 
                                            int num_inputs, int* input_net_num);

int rr_node_drive_switch_box(t_rr_node* src_rr_node,
                             t_rr_node* des_rr_node,
                             int switch_box_x,
                             int switch_box_y,
                             int chan_side);

void find_drive_rr_nodes_switch_box(int switch_box_x,
                                    int switch_box_y,
                                    t_rr_node* src_rr_node,
                                    int chan_side,
                                    int return_num_only,
                                    int* num_drive_rr_nodes,
                                    t_rr_node*** drive_rr_nodes,
                                    int* switch_index);

int is_sb_interc_between_segments(int switch_box_x, 
                                  int switch_box_y, 
                                  t_rr_node* src_rr_node, 
                                  int chan_side);

int count_num_sram_bits_one_spice_model(t_spice_model* cur_spice_model,
                                        int mux_size);

int count_num_conf_bits_one_spice_model(t_spice_model* cur_spice_model,
                                        enum e_sram_orgz cur_sram_orgz_type,
                                        int mux_size);

int count_num_reserved_conf_bits_one_lut_spice_model(t_spice_model* cur_spice_model,
                                                     enum e_sram_orgz cur_sram_orgz_type);

int count_num_reserved_conf_bits_one_mux_spice_model(t_spice_model* cur_spice_model,
                                                     enum e_sram_orgz cur_sram_orgz_type,
                                                     int mux_size);

int count_num_reserved_conf_bits_one_rram_sram_spice_model(t_spice_model* cur_spice_model,
                                                           enum e_sram_orgz cur_sram_orgz_type);

int count_num_reserved_conf_bits_one_spice_model(t_spice_model* cur_spice_model,
                                                 enum e_sram_orgz cur_sram_orgz_type,
                                                 int mux_size);

int count_num_conf_bit_one_interc(t_interconnect* cur_interc,
                                  enum e_sram_orgz cur_sram_orgz_type);

int count_num_reserved_conf_bit_one_interc(t_interconnect* cur_interc,
                                           enum e_sram_orgz cur_sram_orgz_type);

int count_num_conf_bits_pb_type_mode_interc(t_mode* cur_pb_type_mode,
                                            enum e_sram_orgz cur_sram_orgz_type);

int rec_count_num_conf_bits_pb_type_default_mode(t_pb_type* cur_pb_type,
                                                 enum e_sram_orgz cur_sram_orgz_type);

int rec_count_num_conf_bits_pb_type_physical_mode(t_pb_type* cur_pb_type,
                                                  enum e_sram_orgz cur_sram_orgz_type);

int rec_count_num_conf_bits_pb(t_pb* cur_pb,
                               enum e_sram_orgz cur_sram_orgz_type);

void init_one_grid_num_conf_bits(int ix, int iy,
                                 enum e_sram_orgz cur_sram_orgz_type);

void init_grids_num_conf_bits(enum e_sram_orgz cur_sram_orgz_type);

void zero_spice_models_cnt(int num_spice_models, t_spice_model* spice_model);

void zero_one_spice_model_routing_index_low_high(t_spice_model* cur_spice_model);

void zero_spice_models_routing_index_low_high(int num_spice_models, 
                                              t_spice_model* spice_model);

void alloc_spice_model_routing_index_low_high(t_spice_model* cur_spice_model);

void free_one_spice_model_routing_index_low_high(t_spice_model* cur_spice_model);

void free_spice_model_routing_index_low_high(int num_spice_models, 
                                             t_spice_model* spice_model);

void update_one_spice_model_routing_index_high(int x, int y, t_rr_type chan_type,
                                               t_spice_model* cur_spice_model);

void update_spice_models_routing_index_high(int x, int y, t_rr_type chan_type, 
                                            int num_spice_models, 
                                            t_spice_model* spice_model);

void update_one_spice_model_routing_index_low(int x, int y, t_rr_type chan_type,
                                               t_spice_model* cur_spice_model);

void update_spice_models_routing_index_low(int x, int y, t_rr_type chan_type,
                                           int num_spice_models, 
                                           t_spice_model* spice_model);

void rec_count_num_iopads_pb_type_physical_mode(t_pb_type* cur_pb_type);

void rec_count_num_iopads_pb_type_default_mode(t_pb_type* cur_pb_type);

void rec_count_num_iopads_pb(t_pb* cur_pb);

void init_one_grid_num_iopads(int ix, int iy);

void init_grids_num_iopads();

void rec_count_num_mode_bits_pb_type_default_mode(t_pb_type* cur_pb_type);

void rec_count_num_mode_bits_pb(t_pb* cur_pb);

void init_one_grid_num_mode_bits(int ix, int iy);

void init_grids_num_mode_bits();

void check_sram_spice_model_ports(t_spice_model* cur_spice_model,
                                  boolean include_bl_wl);

void check_ff_spice_model_ports(t_spice_model* cur_spice_model,
                                boolean is_scff);

/* Functions to manipulate t_conf_bit and t_conf_bit_info */
void free_conf_bit(t_conf_bit* conf_bit);
void free_conf_bit_info(t_conf_bit_info* conf_bit_info);

t_conf_bit_info*  
alloc_one_conf_bit_info(int index,
                        t_conf_bit* sram_val,
                        t_conf_bit* bl_val, t_conf_bit* wl_val,
                        t_spice_model* parent_spice_model);

t_llist* 
add_conf_bit_info_to_llist(t_llist* head, int index, 
                           t_conf_bit* sram_val, t_conf_bit* bl_val, t_conf_bit* wl_val,
                           t_spice_model* parent_spice_model);

void  
add_mux_scff_conf_bits_to_llist(int mux_size,
                           t_sram_orgz_info* cur_sram_orgz_info, 
                           int num_mux_sram_bits, int* mux_sram_bits,
                           t_spice_model* mux_spice_model);

void 
add_mux_membank_conf_bits_to_llist(int mux_size,
                                   t_sram_orgz_info* cur_sram_orgz_info, 
                                   int num_mux_sram_bits, int* mux_sram_bits,
                                   t_spice_model* mux_spice_model);

void  
add_mux_conf_bits_to_llist(int mux_size,
                           t_sram_orgz_info* cur_sram_orgz_info, 
                           int num_mux_sram_bits, int* mux_sram_bits,
                           t_spice_model* mux_spice_model);

void add_sram_membank_conf_bits_to_llist(t_sram_orgz_info* cur_sram_orgz_info, int mem_index, 
                                         int num_bls, int num_wls, 
                                         int* bl_conf_bits, int* wl_conf_bits);

void  
add_sram_conf_bits_to_llist(t_sram_orgz_info* cur_sram_orgz_info, int mem_index, 
                            int num_sram_bits, int* sram_bits);

void find_bl_wl_ports_spice_model(t_spice_model* cur_spice_model,
                                  int* num_bl_ports, t_spice_model_port*** bl_ports,
                                  int* num_wl_ports, t_spice_model_port*** wl_ports);

int* decode_mode_bits(char* mode_bits, int* num_sram_bits);

/* Functions to manipulate structs of SRAM orgz */
t_sram_orgz_info* alloc_one_sram_orgz_info();
t_mem_bank_info* alloc_one_mem_bank_info();
void free_one_mem_bank_info(t_mem_bank_info* mem_bank_info);
t_scff_info* alloc_one_scff_info();
void free_one_scff_info(t_scff_info* scff_info);
t_standalone_sram_info* alloc_one_standalone_sram_info();
void free_one_standalone_sram_info(t_standalone_sram_info* standalone_sram_info);
void init_mem_bank_info(t_mem_bank_info* cur_mem_bank_info,
                        t_spice_model* cur_mem_model);
void update_mem_bank_info_reserved_blwl(t_mem_bank_info* cur_mem_bank_info,
                                        int updated_reserved_bl, int updated_reserved_wl);
void get_mem_bank_info_reserved_blwl(t_mem_bank_info* cur_mem_bank_info,
                                     int* num_reserved_bl, int* num_reserved_wl);
void update_mem_bank_info_num_blwl(t_mem_bank_info* cur_mem_bank_info,
                                   int updated_bl, int updated_wl);
void get_sram_orgz_info_reserved_blwl(t_sram_orgz_info* cur_sram_orgz_info,
                                      int* num_reserved_bl, int* num_reserved_wl);
void update_mem_bank_info_num_mem_bit(t_mem_bank_info* cur_mem_bank_info,
                                      int num_mem_bit);
void init_scff_info(t_scff_info* cur_scff_info,
                    t_spice_model* cur_mem_model);
void update_scff_info_num_mem_bit(t_scff_info* cur_scff_info,
                                  int num_mem_bit);
void init_standalone_sram_info(t_standalone_sram_info* cur_standalone_sram_info,
                               t_spice_model* cur_mem_model);
void update_standalone_sram_info_num_mem_bit(t_standalone_sram_info* cur_standalone_sram_info,
                                             int num_mem_bit);
void init_sram_orgz_info(t_sram_orgz_info* cur_sram_orgz_info,
                         enum e_sram_orgz cur_sram_orgz_type,
                         t_spice_model* cur_mem_model, 
                         int grid_nx, int grid_ny);
void free_sram_orgz_info(t_sram_orgz_info* cur_sram_orgz_info,
                         enum e_sram_orgz cur_sram_orgz_type,
                         int grid_nx, int grid_ny);
void update_sram_orgz_info_reserved_blwl(t_sram_orgz_info* cur_sram_orgz_info,
                                         int updated_reserved_bl, int updated_reserved_wl);
int get_sram_orgz_info_num_mem_bit(t_sram_orgz_info* cur_sram_orgz_info);
void get_sram_orgz_info_num_blwl(t_sram_orgz_info* cur_sram_orgz_info,
                                int* cur_bl, int* cur_wl);
void update_sram_orgz_info_num_mem_bit(t_sram_orgz_info* cur_sram_orgz_info,
                                       int new_num_mem_bit);
void update_sram_orgz_info_num_blwl(t_sram_orgz_info* cur_sram_orgz_info,
                                    int new_bl, int new_wl);
void get_sram_orgz_info_mem_model(t_sram_orgz_info* cur_sram_orgz_info,
                                  t_spice_model** mem_model_ptr);

void init_reserved_syntax_char(t_reserved_syntax_char* cur_reserved_syntax_char,
                               char cur_syntax_char, boolean cur_verilog_reserved, boolean cur_spice_reserved);
