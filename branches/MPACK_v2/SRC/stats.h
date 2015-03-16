
float stats_ble_resource_util(t_ble_arch* ble_arch,
                              t_ble_info* ble_info);

int stats_net_num(t_lgkntwk* lgkntwk,
                  t_ble_info* ble_info,
                  int verbose);

int stats_ble_num(t_ble_arch* ble_arch,
                  t_ble_info* head,
                  int verbose);

int stats_lgkntwk(t_lgkntwk* lgkntwk,
                  t_stats* stats);
