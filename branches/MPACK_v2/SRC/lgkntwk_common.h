/*Subroutines declarations*/
int remove_buf_lgkntwk(t_lgkntwk* lgkntwk,
                       t_stats* stats,
                       int verbose);

int remove_isolated_pi_lgkntwk(t_lgkntwk* lgkntwk,
                               t_stats* stats,
                               int verbose);

int remove_isolated_po_lgkntwk(t_lgkntwk* lgkntwk,
                               t_stats* stats,
                               int verbose);

int remove_hanging_latch_lgkntwk(t_lgkntwk* lgkntwk,
                                 t_stats* stats,
                                 int verbose);

int dfs_lgknd_tree_depth(t_lgknd* root,
                         int timing_analysis,
                         int verbose);

int dfs_lgkntwk_depth(t_lgkntwk* lgkntwk,
                      int timing_analysis,
                      int verbose);

int check_lgkntwk(t_lgkntwk* lgkntwk);

int lgknd_fanout_comb_num(t_lgknd* lgknd);

int add_pi_latch_buf(t_lgknd* cur,
                     int input_idx);

int add_buf_latches(t_lgkntwk* lgkntwk,
                    t_ble_arch* ble_arch,
                    t_stats* stats,
                    int strict,
                    int verbose);

int bubble_sort_depth_lgk_dfsnds(int num_dfsnd,
                                 t_lgknd** dfsnds,
                                 int* cur_depths,
                                 float* dfsnds_weight);

int* local_dfs_lgknds(int num_dfsnds,
                      t_lgknd** dfsnds,
                      float* dfsnds_weight,
                      int verbose);

int add_one_buf_lgknd(t_lgknd* cur,
                      int input_idx);

int remove_one_buf_lgknd(t_lgknd* cur,
                         int input_idx);

int add_input_bufs_lgknd_lgkntwk(t_lgkntwk* lgkntwk,
                                 t_lgknd* cur);

int split_lgknd_output(t_lgkntwk* lgkntwk,
                       t_lgknd* lgknd);

int split_lgkntwk_lgknds_outputs(t_lgkntwk* lgkntwk,
                                 int output_num_upbound,
                                 int verbose);

void reset_lgkntwk_try_sat(t_lgkntwk* lgkntwk);

int find_min_fanout_pred(t_lgknd* lgknd,
                         int verbose);

int clear_tmp_redund_output_num(int lgknd_num,
                                t_lgknd** lgknds,
                                int verbose);

int update_redund_output_num(int lgknd_num,
                             t_lgknd** lgknds,
                             int verbose);

int init_lgkntwk_edges(t_lgkntwk* lgkntwk,
                       int verbose);

int init_timing_lgkntwk(t_lgkntwk* lgkntwk,
                       t_delay_info* delay_info,
                       int verbose);

int depth_weighting_lgkntwk(t_lgkntwk* lgkntwk,
                            int verbose);

int reset_dfs_tags_lgkntwk(t_lgkntwk* lgkntwk,
                           int verbose);

int timing_analysis_lgknd(t_lgknd* root,
                          int verbose);

int timing_analysis_lgkntwk(t_lgkntwk* lgkntwk,
                            int verbose);

float critical_delay_lgkntwk(t_lgkntwk* lgkntwk,
                            int verbose);

int timing_required_lgknd(t_lgknd* root,
                          int verbose);

int timing_required_lgkntwk(t_lgkntwk* lgkntwk,
                            float crit_delay,
                            int verbose);

int timing_slack_lgkntwk(t_lgkntwk* lgkntwk,
                         int verbose);

float timing_max_slack_lgkntwk(t_lgkntwk* lgkntwk,
                               int verbose);

int timing_weighting_lgkntwk(t_lgkntwk* lgkntwk,
                             float max_slack,
                             int verbose);

int merge_weights_lgkntwk(t_lgkntwk* lgkntwk,
                          float aplha,
                          int verbose);

int critical_input_paths_affect_lgknd(t_lgknd* lgknd,
                                      int verbose);

int critical_output_paths_affect_lgknd(t_lgknd* lgknd,
                                       int verbose);

int critical_paths_affect_lgkntwk(t_lgkntwk* lgkntwk,
                                  int verbose);

int initial_lgkntwk_try_pattern_types(t_lgkntwk* lgkntwk);

int reset_lgkntwk_try_pattern_types(t_lgkntwk* lgkntwk);
