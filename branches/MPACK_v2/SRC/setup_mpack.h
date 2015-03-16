/*Subroutines declarations*/
int check_lut_blif_netlist(t_blif_arch* blif_arch);

int check_blif_netlist(t_blif_arch* blif_arch);

int assign_lut_logic_nodes(t_blif_arch* blif_arch,
                       t_lgkntwk* lgkntwk,
                       t_stats* stats);

int update_lut_stats(t_blif_arch* blif_arch,
                     t_lgkntwk* lgkntwk,
                     t_stats* stats);

int assign_logic_nodes(t_blif_arch* blif_arch,
                       t_lgkntwk* lgkntwk,
                       t_stats* stats);

int assign_conkts_to_luts(t_blif_arch* blif_arch,
                          t_lgkntwk* lgkntwk);

int assign_conkts_to_lgknds(t_blif_arch* blif_arch,
                            t_lgkntwk* lgkntwk);

int assign_lut_latch_conkts(t_blif_arch* blif_arch,
                            t_lgknd** libgate_ptrs,
                            t_lgknd** latch_ptrs,
                            t_lgknd** pi_ptrs,
                            t_lgknd** po_ptrs);

int assign_lut_conkts(t_blif_arch* blif_arch,
                      t_lgknd** libgate_ptrs,
                      t_lgknd** latch_ptrs,
                      t_lgknd** pi_ptrs,
                      t_lgknd** po_ptrs);

int assign_libgate_conkts(t_blif_arch* blif_arch,
                          t_lgknd** libgate_ptrs,
                          t_lgknd** latch_ptrs,
                          t_lgknd** pi_ptrs,
                          t_lgknd** po_ptrs);

int assign_latch_conkts(t_blif_arch* blif_arch,
                        t_lgknd** libgate_ptrs,
                        t_lgknd** latch_ptrs,
                        t_lgknd** pi_ptrs,
                        t_lgknd** po_ptrs);

int assign_pi_conkts(t_blif_arch* blif_arch,
                     t_conkt* head,
                     t_lgknd** pi_ptrs,
                     t_lgknd*** ptr_lst);

int assign_po_conkts(t_blif_arch* blif_arch,
                     t_conkt* head,
                     t_lgknd** po_ptrs,
                     t_lgknd*** ptr_lst);

int assign_lut(t_lgkntwk* lgkntwk,
                   t_blif_arch* blif_arch,
                   t_stats* stats);

int assign_libgate(t_lgkntwk* lgkntwk,
                   t_blif_arch* blif_arch,
                   t_stats* stats);

int assign_pi(t_lgkntwk* lgkntwk,
              t_blif_arch* blif_arch,
              t_stats* stats);

int assign_po(t_lgkntwk* lgkntwk,
              t_blif_arch* blif_arch,
              t_stats* stats);

int assign_latch(t_lgkntwk* lgkntwk,
                 t_blif_arch* blif_arch,
                 t_stats* stats);

int clear_po_outputs(t_lgkntwk* lgkntwk);

int assign_libgate_latch_conkt_po(t_blif_arch* blif_arch,
                                  t_lgkntwk* lgkntwk);

int assign_lut_lgknds_net_name(t_blif_arch* blif_arch,
                           t_lgkntwk* lgkntwk);

int assign_lgknds_net_name(t_blif_arch* blif_arch,
                           t_lgkntwk* lgkntwk);
