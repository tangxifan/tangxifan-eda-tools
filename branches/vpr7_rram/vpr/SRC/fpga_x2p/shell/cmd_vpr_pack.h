/* Command-line options for Packer */
t_opt_info vpr_pack_opts[] = {
  /* Packer Options should be listed here */
  /* Packer File Options  */
  {"sdc_file", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "File name of SDC constraints"},
  {"net_file", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "File name of post-packing netlist"},
  /* Packer Options  */
  {"global_clocks", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"hill_climb", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"sweep_hanging_nets_and_inputs", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"timing_driven", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"cluster_seed_type", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"alpha", 0, OPT_WITHVAL, OPT_FLOAT, OPT_OPT, OPT_NONDEF, ""},
  {"beta", 0, OPT_WITHVAL, OPT_FLOAT, OPT_OPT, OPT_NONDEF, ""},
  {"recompute_timing_after", 0, OPT_WITHVAL, OPT_INT, OPT_OPT, OPT_NONDEF, ""},
  {"block_delay", 0, OPT_WITHVAL, OPT_INT, OPT_OPT, OPT_NONDEF, ""},
  {"intra_cluster_net_delay", 0, OPT_WITHVAL, OPT_FLOAT, OPT_OPT, OPT_NONDEF, ""},
  {"inter_cluster_net_delay", 0, OPT_WITHVAL, OPT_FLOAT, OPT_OPT, OPT_NONDEF, ""},
  {"auto_compute_inter_cluster_net_delay", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"skip_clustering", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"allow_unrelated_clustering", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"allow_early_exit", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"connection_driven", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"aspect", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {"algorithm", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, ""},
  {LAST_OPT_NAME, 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Launch help desk"}
};

/* Function to execute the command */
void shell_execute_vpr_pack(t_shell_env* env, 
                            t_opt_info* opts);
