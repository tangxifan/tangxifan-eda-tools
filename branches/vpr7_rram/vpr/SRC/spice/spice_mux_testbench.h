
enum e_spice_mux_tb_type {
  SPICE_CB_MUX_TB, SPICE_SB_MUX_TB, SPICE_PB_MUX_TB
};

void fprint_spice_mux_testbench(char* formatted_spice_dir,
                                char* circuit_name,
                                char* include_dir_path,
                                char* subckt_dir_path,
                                t_ivec*** LL_rr_node_indices,
                                int num_clock,
                                t_arch arch,
                                enum e_spice_mux_tb_type mux_tb_type,
                                boolean leakage_only);
