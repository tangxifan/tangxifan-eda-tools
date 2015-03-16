
void free_blif_arch(t_blif_arch* blif_arch);

void free_ble_arch(t_ble_arch* ble_arch);

t_optinfo* read_options(IN int argc,
                 IN char **argv);

int config_mpack(t_mpack_conf* mpackopts_ptr,
                 t_optinfo* optlst_ptr);

int setup_mpack(t_blif_arch* blif_arch,
                t_lgkntwk* lgkntwk,
                t_stats* stats,
                int read_lut_blif);

int check_ble_arch(t_ble_arch* ble_arch);

t_ble_info* pack_BLEs(t_lgkntwk* lgkntwk,
                      t_stats* stats,
                      t_ble_arch* ble_arch,
                      int timing_analysis,
                      t_delay_info* delay_info,
                      t_mpack_conf mpack_opts,
                      int verbose);

int print_blif(char* output_blif_name,
               t_lgkntwk* lgkntwk,
               t_ble_arch* ble_arch,
               t_ble_info* ble_info,
               char* model_name,
               int verbose);

void print_stats(char* fstats_name,
                 t_stats* stats,
                 t_blif_arch* blif_arch,
                 t_lgkntwk* lgkntwk,
                 t_ble_arch* ble_arch,
                 t_ble_info* ble_info,
                 int verbose);

int print_vpr7_arch(t_ble_arch* ble_arch,
                    char* arch_file,
                    t_delay_info delay_info,
                    int verbose);

int print_vpr7_netlist(char* net_file,
                       t_ble_arch* ble_arch,
                       t_ble_info* ble_info,
                       t_lgkntwk* lgkntwk,
                       int verbose);
