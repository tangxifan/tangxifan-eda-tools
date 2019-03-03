/* Command-line options for VPR setup */
/* Add any option by following the format of t_opt_info */
t_opt_info setup_vpr_opts[] = {
  /* VPR Setup Options should be listed here */
  {"blif_file", 0, OPT_WITHVAL, OPT_CHAR, OPT_REQ, OPT_NONDEF, "Specify the blif for input benchmark"},
  {"arch_file", 0, OPT_WITHVAL, OPT_CHAR, OPT_REQ, OPT_NONDEF, "Specify the XML architecture description file for FPGA"},
  {"activity_file", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify the activity file for input benchmark in purpose of power estimation"},
  {"power_properties", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify the power property XML for power estimation"},
  {"timing_analysis", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify if timing-driven and timing analysis should be enabled"},
  {"out_file_prefix", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify the prefix of files to be "},
  {"net_file", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify the netlist outputted by packers"},
  {"place_file", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify the netlist outputted by placers"},
  {"route_file", 0, OPT_WITHVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify the netlist outputted by routers"},
  {"echo_file", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify if echo files will be outputted"},
  {"generate_postsynthesis_netlist", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify if post synthesis netlists will be outputted"},
  {"constant_net_delay", 0, OPT_WITHVAL, OPT_FLOAT, OPT_OPT, OPT_NONDEF, "Specify if use constant net delay"},
  {"read_xml_fpga_x2p", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify if read XML syntax for FPGA X2P"},
  {"read_xml_versa_power", 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Specify if read XML syntax for VersaPower"},
  {LAST_OPT_NAME, 0, OPT_NONVAL, OPT_CHAR, OPT_OPT, OPT_NONDEF, "Launch Help Desk"}
};

/* Function to execute the command */
void shell_execute_vpr_setup(t_shell_env* env, t_opt_info* opts);
