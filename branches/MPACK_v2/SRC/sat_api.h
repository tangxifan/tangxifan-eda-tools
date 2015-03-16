
int count_used_ble_outputs(t_ble_info* ble_info,
                           int verbose);

int count_used_blend(t_ble_info* ble_info,
                     int verbose);

int spot_lgknd_ble_cell_idx(t_ble_info* ble_info,
                            t_lgknd* cur);

int spot_lgknd_ble_output_idx(t_ble_info* ble_info,
                              t_lgknd* cur,
                              int latch_only);

int check_lgknd_input_remap(t_ble_arch* ble_arch,
                            t_ble_info* ble_info,
                            t_lgknd* sat_lgknd,
                            int output_idx,
                            int* remap_cells,
                            int verbose);

int sat_ble_out(t_ble_arch* ble_arch,
                t_ble_info* ble_info,
                int is_comb,
                int cell_idx,
                t_lgknd* cur,
                t_lgknd* latch);

int sat_lgknd_outputs(t_ble_arch* ble_arch,
                     t_ble_info* ble_info,
                     int cell_idx,
                     t_lgknd* cur,
                     int comb_num,
                     int verbose);

int sat_src_port_des_port(t_ble_arch* ble_arch,
                          t_ble_info* ble_info,
                          t_bleport* src_port,
                          t_bleport* des_port);

int sat_cell_port_target_lgknd(t_ble_arch* ble_arch,
                               t_ble_info* ble_info,
                               t_bleport* src_port,
                               t_lgknd* targ);

int sat_lgknd_input_cell_port(t_ble_arch* ble_arch, 
                              t_ble_info* ble_info,
                              int cell_idx,
                              t_lgknd* lgknd,
                              int lgknd_input_idx,
                              int logic_equivalent);

int sat_cell_redundant_ports(t_ble_arch* ble_arch, 
                             t_ble_info* ble_info,
                             int cell_idx,
                             t_lgknd* lgknd,
                             int strict,
                             int clb_logic_equal,
                             int verbose);

int sat_lgknd_inputs(t_ble_arch* ble_arch, 
                     t_ble_info* ble_info,
                     int cell_idx,
                     t_lgknd* lgknd,
                     int logic_equivalent,
                     int strict,
                     int verbose);

int mark_lgknd_outputs_mapped(t_lgkntwk* lgkntwk,
                              t_lgknd* cur,
                              int ble_num,
                              int add_buf,
                              t_ble_info* ble_info);

int mini_sat_solver(int sat_type,
                    t_lgkntwk* lgkntwk,
                    t_lgknd** sat_lgknds,
                    int* sat_idxes,
                    t_ble_arch* ble_arch,
                    t_ble_info* ble_info,
                    int* logic_equivalent,
                    int* strict,
                    int verbose);

int run_sat_solver(t_lgkntwk* lgkntwk,
                   t_ble_arch* ble_arch,
                   t_ble_info* ble_info,
                   int sat_type,
                   int cell_idx,
                   t_lgknd** sat_lgknds,
                   int timing_analysis,
                   t_delay_info* delay_info,
                   float alpha,
                   int verbose);

int count_lgknds_comb_num_ble(t_ble_info* ble_info,
                              int lgknd_num,
                              t_lgknd** lgknds,
                              int* comb_num,
                              int verbose);

int count_lgknds_comb_num(int lgknd_num,
                          t_lgknd** lgknds,
                          int* comb_num,
                          int verbose);

float lgknds_attract(t_lgknd* src,
                     t_lgknd* des);

float ble_lgknd_attract(t_ble_info* ble_info,
                        t_lgknd* lgknd);

int lgknds_sat_type(t_lgknd* root,
                    t_lgknd** sat_lgknds,
                    int strong,
                    enum e_pattern_member_type pattern_member_type,
                    int verbose);

int sat_mapper(t_lgkntwk* lgkntwk,
               t_ble_arch* ble_arch,
               t_ble_info* ble_info,
               int sat_type,
               int cell_idx,
               t_lgknd* root,
               int timing_analysis,
               t_delay_info* delay_info,
               t_mpack_conf mpack_opts,
               int verbose);

int free_lgknd_inputs_in_ble(t_ble_arch* ble_arch,
                             t_ble_info* ble_info,
                             int cell_idx,
                             t_lgknd* lgknd,
                             int verbose);

int determine_max_sat_type(t_ble_arch* ble_arch,
                           t_ble_info* ble_info,
                           int cell_idx,
                           int verbose);

int legality_check(int sat_lgknd_num,
                   t_lgknd** sat_lgknds,
                   t_ble_info* ble_info,
                   int verbose);

int lgknd_input_sum_weight_subgraph(int num,
                                    t_lgknd** sub_graph,
                                    t_lgknd* lgknd,
                                    int lgknd_idx,
                                    int input_idx,
                                    int* sub_graph_fanout_absorb,
                                    float* sum_timing_weight,
                                    float* sum_depth_weight_input,
                                    float* sum_depth_weight_output);

int latch_absorb_in_subgraph(int num,
                             t_lgknd** sub_graph,
                             t_lgknd* latch);

float sat_lgknds_ble_average_weight(int sat_lgknd_num,
                                    t_lgknd** sat_lgknds,
                                    t_ble_info* ble_info,
                                    float alpha,
                                    float beta,
                                    enum e_packer_area_attraction packer_area_attraction,
                                    float* paths_affect,
                                    int verbose);

t_lgknd* pick_sat_lgknd_from_lgkntwk(t_lgkntwk* lgkntwk,
                                     int sat_type,
                                     t_ble_info* ble_info,
                                     float alpha,
                                     float beta,
                                     enum e_pattern_member_type pattern_member_type,
                                     enum e_packer_area_attraction packer_area_attraction,
                                     int verbose);

int subgraph_legality_check(int num,
                            t_lgknd** subgraph,
                            t_ble_info* ble_info,
                            int verbose);

int depth_local_router(int sat_lgknd_num,
                       t_lgknd** sat_lgknds,
                       t_lgkntwk* lgkntwk,
                       t_ble_arch* ble_arch,
                       t_ble_info* ble_info,
                       int verbose);

int determine_unused_blend_num(t_ble_arch* ble_arch,
                               t_ble_info* ble_info,
                               int verbose);

float sat_lgknds_ble_paths_affect(int sat_lgknd_num,
                                  t_lgknd** sat_lgknds,
                                  t_ble_info* ble_info,
                                  int verbose);

int pick_pattern_fit_ble(t_lgkntwk* lgkntwk,
                         int arch_pattern_type,
                         t_lgknd** pattern_lgknds,// Remember to free it otherwhere 
                         t_ble_info* ble_info,
                         float alpha,
                         float beta,
                         enum e_pattern_member_type pattern_member_type,
                         enum e_packer_area_attraction packer_area_attraction,
                         int forbid_unrelated_clustering,
                         int verbose);

