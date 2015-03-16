
int climb_pack(t_lgkntwk* lgkntwk,
               t_stats* stats,
               t_ble_arch* ble_arch,
               t_ble_info* ble_info_head,
               int verbose);

int join_preds_ble(t_lgkntwk* lgkntwk,
                   t_ble_arch* ble_arch,
                   t_lgknd* cur,
                   t_ble_info* ble_info,
                   int cell_idx);


int fill_ble(t_lgkntwk* lgkntwk,
             t_ble_arch* ble_arch,
             t_ble_info* ble_info);

int count_cell_used(t_ble_info* ble_info,
                    int* cell_idx);

int try_join_ble(t_lgkntwk* lgkntwk,
                 t_ble_arch* ble_arch,
                 t_lgknd* cur,
                 t_ble_info* ble_info);

int new_fill_ble_once(t_lgkntwk* lgkntwk,
                      t_ble_arch* ble_arch,
                      t_ble_info* ble_info,
                      int verbose);

int new_fill_ble(t_lgkntwk* lgkntwk,
                 t_ble_arch* ble_arch,
                 t_ble_info* ble_info,
                 int verbose);
