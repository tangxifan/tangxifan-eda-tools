int check_cell_input_number(t_ble_arch* ble_arch);

int check_cells_conkted(t_blend* cella,
                        t_blend* cellb,
                        int* port_idx,
                        int* cand_idx,
                        t_ble_info* ble_info);

int check_tree_conkts(t_blend* root,
                      t_blend* leaf0,
                      t_blend* leaf1,
                      t_ble_info* ble_info);

int get_port_used_mux(t_bleport* port,
                      t_ble_info* ble_info,
                      int* used,
                      int* mux);

int check_shared_inputs(t_bleport* porta,
                        t_bleport* portb,
                        t_ble_info* ble_info);

int check_conkt_ble_input(t_bleport* port,
                          t_ble_info* ble_info);

int check_inv_cell(t_ble_arch* ble_arch,
                   t_ble_info* ble_info,
                   int cell_idx);

int mark_invs_ble(t_ble_arch* ble_arch,
                  t_ble_info* ble_info,
                  int verbose);

int** check_fully_sat2_cell(t_ble_arch* ble_arch,
                            t_ble_info* t_ble_info,
                            int cell_idx,
                            int* sat2_num);

int** check_fully_sat3_cell(t_ble_arch* ble_arch,
                            t_ble_info* t_ble_info,
                            int cell_idx,
                            int* sat3_num);

int mark_fully_sat_cells_ble(t_ble_arch* ble_arch,
                             t_ble_info* t_ble_info,
                             int verbose);

int check_sat3_conkts(t_ble_arch* ble_arch,
                      int cell_idx,
                      int input0_cell,
                      int input1_cell,
                      int input2_cell,
                      t_ble_info* ble_info);

int check_sat2_conkts(t_ble_arch* ble_arch,
                      int cell_idx,
                      int input0_cell,
                      int input1_cell,
                      t_ble_info* ble_info);

int dfs_ble_arch(t_ble_arch* ble_arch,
                 t_ble_info* ble_info,
                 int verbose);

int* local_dfs_ble_cell(t_ble_arch* ble_arch,
                        t_ble_info* ble_info,
                        int num_dfsnds,
                        t_blend** dfsnds,
                        int verbose);

int bubble_sort_depth_ble_dfsnds(int num_dfsnd,
                                 t_blend** dfsnds,
                                 int* cur_depths);

int check_independent_cell(t_ble_arch* ble_arch,
                           t_ble_info* ble_info,
                           int cell_idx,
                           int input_num);

int check_sat(int sat_type,
              t_ble_arch* ble_arch,
              t_ble_info* ble_info,
              t_blend* root,
              int* sat_idxes,
              int verbose);

int pick_position_from_ble_arch(t_ble_arch* ble_arch,
                                t_ble_info* ble_info,
                                t_lgknd* lgknd,
                                int* blend_tried);

int pick_cell_idx_from_ble_arch(t_ble_arch* ble_arch,
                                t_ble_info* ble_info,
                                int* blend_tried);

int sum_cell_input_num(t_ble_arch* ble_arch,
                       int verbose);

int sum_ff_num(t_ble_arch* ble_arch,
               int verbose);
