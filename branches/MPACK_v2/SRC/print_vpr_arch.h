
int print_vpr7_arch_mux_inputs(t_ble_arch* ble_arch,
                                     t_bleport* port,
                                     FILE* farch,
                                     int verbose);

int print_vpr7_arch_comb_part_inputs(t_ble_arch* ble_arch,
                                     t_bleport* port,
                                     FILE* farch,
                                     t_delay_info delay_info,
                                     int verbose);

int print_vpr7_arch_comb_part_outputs(t_ble_arch* ble_arch,
                                      FILE* farch,
                                      int verbose);

int print_vpr7_arch_follower_interconnect(t_ble_arch* ble_arch,
                                          FILE* farch,
                                          int verbose);

int print_vpr7_arch_followers(t_ble_arch* ble_arch,
                              FILE* farch,
                              t_delay_info delay_info,
                              int verbose);

