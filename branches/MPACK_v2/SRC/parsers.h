/*Available Subroutines Outside*/
void blif_parser_flow(char* blifname,
                      t_blif_arch* blif_arch);

int blif_parser(char* blifname,
                t_blif_arch* blif_arch);

t_conkt* build_nets(t_blif_arch* blif_arch);

void ble_arch_parser_flow(char* ble_arch_file,
                          t_ble_arch* ble_arch);

int parse_ble_arch(char* ble_arch_file,
                   t_ble_arch* ble_arch);
