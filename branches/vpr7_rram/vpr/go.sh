#!/bin/sh
# Example of how to run vpr

# Pack, place, and route a heterogeneous FPGA
# Packing uses the AAPack algorithm
#./vpr sample_arch.xml or1200.latren.blif
#rm -rf spice_test
#valgrind ./vpr ARCH/K6_N10_I33_frac_UMC180nm_1p8V.xml Circuits/s298_K6_N10_ace.blif --activity_file Circuits/s298_K6_N10_ace.act --nodisp --fpga_spice --fpga_spice_dir /home/xitang/tangxifan-eda-tools/branches/vpr7_rram/vpr/spice_test  --print_spice_grid_testbench --print_spice_lut_testbench --print_spice_dff_testbench --print_spice_pb_mux_testbench --print_spice_cb_mux_testbench --print_spice_sb_mux_testbench --print_spice_top_testbench #--fpga_spice_parasitic_net_estimation_off #--print_spice_grid_testbench  #--fpga_spice_leakage_only
#valgrind ./vpr ARCH/K6_N10_I33_frac_UMC180nm_1p8V.xml Circuits/apex4_K6_N10_ace.blif --activity_file Circuits/apex4_K6_N10_ace.act --nodisp --fpga_spice --fpga_spice_dir /home/xitang/tangxifan-eda-tools/branches/vpr7_rram/vpr/spice_test --route_chan_width 120  --print_spice_grid_testbench --print_spice_lut_testbench --print_spice_dff_testbench --print_spice_pb_mux_testbench --print_spice_cb_mux_testbench --print_spice_sb_mux_testbench --print_spice_top_testbench --fpga_spice_parasitic_net_estimation_off #--print_spice_grid_testbench  #--fpga_spice_leakage_only
#valgrind 
#rm -rf spice_test_rram
#./vpr ARCH/K6_N10_I33_frac_UMC180nm_1p2V_rram.xml Circuits/s298_K6_N10_ace.blif --activity_file Circuits/s298_K6_N10_ace.act --nodisp --fpga_spice --fpga_spice_dir /home/xitang/tangxifan-eda-tools/branches/vpr7_rram/vpr/spice_test_rram --print_spice_mux_testbench --fpga_spice_leakage_only

#rm -rf spice_test_rram_sized
#./vpr ARCH/K6_N10_I33_frac_UMC180nm_1p2V_rram_sizing.xml Circuits/s298_K6_N10_ace.blif --activity_file Circuits/s298_K6_N10_ace.act --nodisp --fpga_spice --fpga_spice_dir /home/xitang/tangxifan-eda-tools/branches/vpr7_rram/vpr/spice_test_rram_sized --print_spice_mux_testbench --fpga_spice_leakage_only
#./vpr ./ARCH/k6_N10_sram_tsmc40nm.xml Circuits/simple_gates_prevpr.blif --full_stats --nodisp --activity_file Circuits/simple_gates_prevpr.act --fpga_spice --fpga_spice_dir ./sram_fpga_homo --fpga_spice_print_top_testbench --fpga_spice_rename_illegal_port --route_chan_width 60
#./vpr ./ARCH/k6_N10_sram_tsmc40nm.xml Circuits/s298_prevpr.blif --full_stats --nodisp --activity_file Circuits/s298_prevpr.act --fpga_spice --fpga_spice_dir ./sram_fpga_homo --fpga_spice_print_top_testbench --fpga_spice_print_grid_testbench --fpga_spice_print_cb_testbench --fpga_spice_print_sb_testbench --fpga_spice_print_lut_testbench --fpga_spice_print_hardlogic_testbench --fpga_spice_print_pb_mux_testbench --fpga_spice_print_cb_mux_testbench --fpga_spice_print_sb_mux_testbench --fpga_spice_rename_illegal_port --route_chan_width 60
#./vpr ./ARCH/k6_N10_sram_tsmc40nm.xml Circuits/s298_prevpr.blif --full_stats --nodisp --activity_file Circuits/s298_prevpr.act --fpga_syn_verilog --fpga_syn_verilog_dir ./sram_fpga_homo --fpga_spice_rename_illegal_port --route_chan_width 60
#./vpr ./ARCH/k6_N10_sram_tsmc40nm_mem_TT.xml Circuits/ch_intrinsics_prevpr.blif --full_stats --nodisp --activity_file Circuits/ch_intrinsics_prevpr.act --fpga_spice --fpga_spice_dir ./sram_fpga_hetero --fpga_spice_print_top_testbench --fpga_spice_print_grid_testbench --fpga_spice_print_cb_testbench --fpga_spice_print_sb_testbench --fpga_spice_print_lut_testbench --fpga_spice_print_hardlogic_testbench --fpga_spice_print_pb_mux_testbench --fpga_spice_print_cb_mux_testbench --fpga_spice_print_sb_mux_testbench --fpga_spice_rename_illegal_port --route_chan_width 100
#./vpr ./ARCH/k6_N10_sram_tsmc40nm_chain_TT.xml Circuits/s298_prevpr.blif --full_stats --nodisp --activity_file Circuits/s298_prevpr.act --fpga_verilog --fpga_verilog_dir ./sram_fpga_hetero --fpga_spice_rename_illegal_port --route_chan_width 100 --fpga_verilog_compact_netlist --fpga_bitstream_generator
./vpr ./ARCH/k6_N10_sram_tsmc40nm_chain_TT_SC.xml Circuits/s298_prevpr.blif --full_stats --nodisp --activity_file Circuits/s298_prevpr.act --fpga_verilog --fpga_verilog_dir ./sram_fpga_hetero --fpga_spice_rename_illegal_port --route_chan_width 100 --fpga_verilog_compact_netlist --fpga_bitstream_generator
#./vpr ./ARCH/k6_N10_sram_tsmc40nm_chain_mem_TT.xml Circuits/ch_intrinsics_prevpr.blif --full_stats --nodisp --activity_file Circuits/ch_intrinsics_prevpr.act --fpga_verilog --fpga_verilog_dir ./sram_fpga_hetero --fpga_spice_rename_illegal_port --route_chan_width 100 --fpga_verilog_compact_netlist
#./vpr ./ARCH/k6_N10_sram_tsmc40nm_chain_mem_TT.xml Circuits/ch_intrinsics_prevpr.blif --full_stats --nodisp --activity_file Circuits/ch_intrinsics_prevpr.act --fpga_bitstream_generator --route_chan_width 100 

