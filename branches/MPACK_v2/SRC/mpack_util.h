/*Subrouting global declarations*/
void init_lgkntwk(t_lgkntwk* lgkntwk);

void init_stats(t_stats* stats);

void init_edge(t_edge* edge);

void init_lgknd(t_lgknd* lgknd);

t_lgknd* create_lgknd_list(int len,
                           size_t size_node);

t_lgknd* insert_lgknd_list_node(t_lgknd* cur,
                                size_t size_node);

void remove_lgknd_list_node(t_lgknd* cur);

t_lgknd* cat_lgknd_lists(t_lgknd* head1,
                         t_lgknd* head2);

/**
 * Struct ble_info
 */
int init_ble_info(t_ble_arch* ble_arch,
                  t_ble_info* ble_info);

int blank_ble_info(t_ble_arch* ble_arch,
                   t_ble_info* ble_info);

t_ble_info* create_ble_list(int len,
                            size_t size_node);

t_ble_info* insert_ble_list_node(t_ble_info* cur,
                                 size_t size_node);

void remove_ble_list_node(t_ble_info* cur);

t_ble_info* cat_ble_lists(t_ble_info* head1,
                          t_ble_info* head2);

void free_lgknd(t_lgknd* lgknd);

void free_lgkntwk(t_lgkntwk* lgkntwk);

void free_ble_info(t_ble_info* ble_info);

void free_ble_info_list(t_ble_info* head);

void remove_ble_info_from_llist(t_ble_info* head,
                                t_ble_info* target);


int copy_ble_info(t_ble_arch* ble_arch,
                  t_ble_info* src,
                  t_ble_info* des);

void my_init_int_ary(int ary_len, int* ary, int init_val);

int spot_index_lgknd_array(int lgknd_num,
                           t_lgknd** lgknds,
                           t_lgknd* subject);

int check_ble_absorb_lgknd_output(t_ble_info* ble_info,
                                  t_lgknd* lgknd,
                                  int verbose);

int check_blank_ble_info(t_ble_arch* ble_arch,
                         t_ble_info* ble_info);

int check_bleport_conkt(t_ble_arch* ble_arch,
                        t_ble_info* ble_info,
                        t_bleport* port,
                        int verbose);

void init_delay_info(t_delay_info* delay_info,
                     t_mpack_conf mpack_opts,
                     t_ble_arch ble_arch);
