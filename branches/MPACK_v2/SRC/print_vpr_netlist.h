
int print_block_inputs(FILE* fnet,
                       t_ble_info* ble_info,
                       int verbose);

int print_block_mux_input(FILE* fnet,
                          t_ble_arch* ble_arch,
                          t_ble_info* ble_info,
                          t_bleport* port,
                          int verbose);

int print_block_outputs(FILE* fnet,
                        t_ble_arch* ble_arch,
                        t_ble_info* ble_info,
                        int verbose);

int print_block_cells(FILE* fnet,
                      t_ble_arch* ble_arch,
                      t_ble_info* ble_info,
                      int verbose);

int print_block_latches(FILE* fnet,
                        t_ble_arch* ble_arch,
                        t_ble_info* ble_info,
                        int verbose);
