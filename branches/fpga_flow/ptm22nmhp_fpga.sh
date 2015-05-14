# Near VT FPGA
rm -rf ./results_0p8V/
cd ./scripts
perl fpga_flow.pl -conf ../configs/ptm22nmhp_fpga_conf/K6_N10_I33_frac_ptm22nmhp_0p8V.conf -benchmark ../benchmarks/mcnc_big20.txt -rpt ../csv_rpts/ptm22nmhp_fpga/K6_N10_I33_frac_ptm22nmhp_0p8V.csv -N 10 -K 6 -power -remove_designs -multi_thread 1 -fix_route_chan_width  -vpr_fpga_spice ../vpr_fpga_spice_task_lists/K6_N10_I33_frac_ptm22nmhp_0p8V -vpr_fpga_spice_print_pbtb -vpr_fpga_spice_print_cbsbtb -vpr_fpga_spice_print_gridtb -vpr_fpga_spice_print_toptb #-vpr_fpga_spice_leakage_only
perl run_fpga_spice.pl -conf ../vpr_fpga_spice_conf/sample.conf -task ../vpr_fpga_spice_task_lists/K6_N10_I33_frac_ptm22nmhp_0p8V_standard.txt -rpt ../vpr_fpga_spice_csv_rpts/K6_N10_I33_frac_ptm22nmhp_0p8V_standard_spice.csv -multi_thread 1 -parse_pb_mux_tb -parse_cb_mux_tb -parse_sb_mux_tb -parse_lut_tb -parse_dff_tb -parse_grid_tb -parse_top_tb #-parse_mux_testbench -sim_leakage_power_only
cd ..
