/* List all the commands of the shell */
t_shell_cmd shell_cmd[] = {
  {"vpr_setup", setup_vpr_opts, &shell_execute_vpr_setup }, 
  {"vpr_pack", vpr_pack_opts, &shell_execute_vpr_pack }, 
  {"vpr_place_and_route", vpr_place_and_route_opts, &shell_execute_vpr_place_and_route }, 
  {"vpr_versapower", vpr_versapower_opts, &shell_execute_vpr_versapower }, 
  {"fpga_x2p_setup", fpga_x2p_setup_opts, &shell_execute_fpga_x2p_setup }, 
  {"fpga_spice", fpga_spice_opts, &shell_execute_fpga_spice }, 
  {"fpga_verilog", fpga_verilog_opts, &shell_execute_fpga_verilog }, 
  {"fpga_bitstream", fpga_bitstream_opts, &shell_execute_fpga_bitstream }, 
  {"exit", NULL, &shell_execute_exit }, 
  {"quit", NULL, &shell_execute_exit }, 
  {LAST_CMD_NAME, NULL, NULL}
};
