# For TSMC 40nm Tech FPGA 
# Common Part
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/dff.conf -rpt results/tsmc40nm_fpga/dff.csv -dff spice/subvt_dff.sp 
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/lut4.conf -rpt results/tsmc40nm_fpga/lut4.csv -lut spice/subvt_lut.sp -lut_size 4
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/lut5.conf -rpt results/tsmc40nm_fpga/lut5.csv -lut spice/subvt_lut.sp -lut_size 5
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/lut6.conf -rpt results/tsmc40nm_fpga/lut6.csv -lut spice/subvt_lut.sp -lut_size 6

# SRAM MUXes
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/local_routing_mux60.csv -mux spice/subvt_lut.sp -mux_size 60 -two_level_mux cpt -w_cpt_sweep 3
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/sb_route_L4.conf -rpt results/tsmc40nm_fpga/sbmux4_L4.csv -mux spice/subvt_lut.sp -mux_size 4 -auto_out_tapered_buffer 3 -two_level_mux cpt -w_cpt_sweep 3
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/sb_route_L2.conf -rpt results/tsmc40nm_fpga/sbmux4_L2.csv -mux spice/subvt_lut.sp -mux_size 4 -auto_out_tapered_buffer 3 -two_level_mux cpt -w_cpt_sweep 3
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/sb_route_L1.conf -rpt results/tsmc40nm_fpga/sbmux4_L1.csv -mux spice/subvt_lut.sp -mux_size 4 -auto_out_tapered_buffer 3 -two_level_mux cpt -w_cpt_sweep 3
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/sb_route_ideal.conf -rpt results/tsmc40nm_fpga/sbmux4_zeroload.csv -mux spice/subvt_lut.sp -mux_size 4 -auto_out_tapered_buffer 3 -two_level_mux cpt -w_cpt_sweep 3
# Fc,in = 0.15, 0.25, 0.5, 0.75, 1
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/cb_route.conf -rpt results/tsmc40nm_fpga/cbmux48_fci0p15.csv -mux spice/subvt_lut.sp -mux_size 48 -auto_out_tapered_buffer 3 -two_level_mux cpt -w_cpt_sweep 3
# Fc,out = 0.1, 0.25, 0.5, 0.75, 1
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/ble_mux2_fco0p1.conf -rpt results/tsmc40nm_fpga/blemux2_fco0p1.csv -mux spice/subvt_lut.sp -mux_size 2 -auto_out_tapered_buffer 3 -one_level_mux cpt -w_cpt_sweep 3

# RRAM FPGA
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_rram_fpga/rram_local_routing.conf -rpt results/tsmc40nm_fpga/rram_local_routing_mux50.csv -mux spice/subvt_lut.sp -mux_size 50 -one_level_mux cpt 
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_rram_fpga/rram_local_routing.conf -rpt results/tsmc40nm_fpga/rram_local_routing_mux50.csv -mux spice/subvt_lut.sp -mux_size 50 -two_level_mux cpt 
perl subvt_fpga_elc.pl -conf conf/tsmc40nm_rram_fpga/rram_local_routing.conf -rpt results/tsmc40nm_fpga/rram_local_routing_mux4.csv -mux spice/subvt_lut.sp -mux_size 4 -one_level_mux prog_pair -rram_enhance -reset_trans_ratio 1.05
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_rram_fpga/rram_local_routing.conf -rpt results/tsmc40nm_fpga/rram_local_routing_mux50.csv -mux spice/subvt_lut.sp -mux_size 50 -one_level_mux prog_pair -rram_enhance  -advance_rram_mux -reset_trans_ratio 1.05
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_sb_route_L4.conf -rpt results/tsmc40nm_fpga/rram_sbmux4_L4.csv -mux spice/subvt_lut.sp -mux_size 4 -one_level_mux prog_pair -rram_enhance -auto_out_tapered_buffer 3 -advance_rram_mux -reset_trans_ratio 1.05
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_sb_route_L2.conf -rpt results/tsmc40nm_fpga/rram_sbmux4_L2.csv -mux spice/subvt_lut.sp -mux_size 4 -one_level_mux prog_pair -rram_enhance -auto_out_tapered_buffer 3 -advance_rram_mux -reset_trans_ratio 1.05
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_sb_route_L1.conf -rpt results/tsmc40nm_fpga/rram_sbmux4_L1.csv -mux spice/subvt_lut.sp -mux_size 4 -one_level_mux prog_pair -rram_enhance -auto_out_tapered_buffer 3 -advance_rram_mux -reset_trans_ratio 1.05
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_sb_route_ideal.conf -rpt results/tsmc40nm_fpga/rram_sbmux4_zeroload.csv -mux spice/subvt_lut.sp -mux_size 4 -one_level_mux prog_pair -rram_enhance -auto_out_tapered_buffer 3 -advance_rram_mux -reset_trans_ratio 1.05

# Fc,in = 0.15, 0.25, 0.5, 0.75, 1
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_cb_route.conf -rpt results/tsmc40nm_fpga/rram_cbmux48_fci0p15.csv -mux spice/subvt_lut.sp -mux_size 48 -one_level_mux prog_pair -rram_enhance -auto_out_tapered_buffer 3 -advance_rram_mux -reset_trans_ratio 1.05

# Fc,out = 0.1, 0.25, 0.5, 0.75, 1
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_ble_mux2_fco0p1.conf -rpt results/tsmc40nm_fpga/rram_blemux2_fco0p1.csv -mux spice/subvt_lut.sp -mux_size 2 -one_level_mux prog_pair -rram_enhance -auto_out_tapered_buffer 3 -advance_rram_mux -reset_trans_ratio 1.05
