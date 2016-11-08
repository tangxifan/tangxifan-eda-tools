
/* Enumeration for the types of measurements*/
enum e_measure_type {
 SPICE_MEASURE_LEAKAGE_POWER, SPICE_MEASURE_DYNAMIC_POWER
};

/* Subroutines declarations */
void init_include_user_defined_netlists(t_spice spice);

void fprint_include_user_defined_netlists(FILE* fp,
                                          t_spice spice);

void fprint_splited_vdds_spice_model(FILE* fp,
                                     enum e_spice_model_type spice_model_type,
                                     t_spice spice);

void fprint_grid_splited_vdds_spice_model(FILE* fp, int grid_x, int grid_y,
                                          enum e_spice_model_type spice_model_type,
                                          t_spice spice);

void fprint_global_vdds_spice_model(FILE* fp, 
                                    enum e_spice_model_type spice_model_type,
                                    t_spice spice);

void fprint_grid_global_vdds_spice_model(FILE* fp, int x, int y, 
                                         enum e_spice_model_type spice_model_type,
                                         t_spice spice);

void fprint_global_pad_ports_spice_model(FILE* fp, 
                                         t_spice spice);

void fprint_spice_global_vdd_switch_boxes(FILE* fp);

void fprint_spice_global_vdd_connection_boxes(FILE* fp);

void fprint_measure_vdds_spice_model(FILE* fp,
                                     enum e_spice_model_type spice_model_type,
                                     enum e_measure_type meas_type,
                                     int num_cycle,
                                     t_spice spice,
                                     boolean leakage_only);

void fprint_measure_grid_vdds_spice_model(FILE* fp, int grid_x, int grid_y,
                                          enum e_spice_model_type spice_model_type,
                                          enum e_measure_type meas_type,
                                          int num_cycle,
                                          t_spice spice,
                                          boolean leakage_only);

void fprint_call_defined_grids(FILE* fp);

void fprint_call_defined_one_channel(FILE* fp,
                                     t_rr_type chan_type,
                                     int x, int y,
                                     int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                     t_ivec*** LL_rr_node_indices);

void fprint_call_defined_channels(FILE* fp,
                                  int LL_num_rr_nodes, t_rr_node* LL_rr_node,
                                  t_ivec*** LL_rr_node_indices);

void fprint_call_defined_one_connection_box(FILE* fp,
                                            t_cb cur_cb_info);

void fprint_call_defined_connection_boxes(FILE* fp);

void fprint_call_defined_one_switch_box(FILE* fp,
                                        t_sb cur_sb_info);

void fprint_call_defined_switch_boxes(FILE* fp);

void fprint_tech_lib(FILE* fp,
                     t_spice_tech_lib tech_lib);

void fprint_spice_options(FILE* fp,
                          t_spice_params spice_params);

void fprint_spice_include_key_subckts(FILE* fp,
                                      char* subckt_dir_path);

void fprint_spice_include_param_headers(FILE* fp,
                                        char* include_dir_path);

void fprint_spice_netlist_transient_setting(FILE* fp, 
                                            t_spice spice, 
                                            int num_sim_clock_cycles, 
                                            boolean leakage_only);

void fprint_stimulate_dangling_one_grid_pin(FILE* fp,
                                            int x, int y,
                                            int height, int side, int pin_index,
                                            t_ivec*** LL_rr_node_indices);

void fprint_stimulate_dangling_io_grid_pins(FILE* fp,
                                            int x, int y);

void fprint_stimulate_dangling_normal_grid_pins(FILE* fp,
                                                int x, int y);

void fprint_stimulate_dangling_grid_pins(FILE* fp);

void init_logical_block_spice_model_temp_used(t_spice_model* spice_model);

void init_logical_block_spice_model_type_temp_used(int num_spice_models, t_spice_model* spice_model,
                                                   enum e_spice_model_type spice_model_type);

void fprint_global_vdds_logical_block_spice_model(FILE* fp,
                                                  t_spice_model* spice_model);

void fprint_splited_vdds_logical_block_spice_model(FILE* fp,
                                                   t_spice_model* spice_model);

void fprint_measure_vdds_logical_block_spice_model(FILE* fp,
                                                   t_spice_model* spice_model,
                                                   enum e_measure_type meas_type,
                                                   int num_clock_cycle,
                                                   boolean leakage_only);


