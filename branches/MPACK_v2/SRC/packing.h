
int check_all_mapped(t_lgkntwk* lgkntwk,
                     int verbose);

t_ble_info* sat_pack(t_lgkntwk* lgkntwk,
                     t_ble_arch* ble_arch,
                     int timing_analysis,
                     t_delay_info* delay_info,
                     t_mpack_conf mpack_opts,
                     int verbose);

t_ble_info* pattern_pack(t_lgkntwk* lgkntwk,
                         t_ble_arch* ble_arch,
                         int timing_analysis,
                         t_delay_info* delay_info,
                         t_mpack_conf mpack_opts,
                         int verbose);
