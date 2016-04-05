# For TSMC 40nm Tech FPGA 
# Common Part
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/dff.conf -rpt results/tsmc40nm_fpga/dff.csv -dff spice/subvt_dff.sp 
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/lut4.conf -rpt results/tsmc40nm_fpga/lut4.csv -lut spice/subvt_lut.sp -lut_size 4
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/lut5.conf -rpt results/tsmc40nm_fpga/lut5.csv -lut spice/subvt_lut.sp -lut_size 5
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/lut6.conf -rpt results/tsmc40nm_fpga/lut6.csv -lut spice/subvt_lut.sp -lut_size 6

# SRAM MUXes
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/mux9.csv -mux spice/subvt_lut.sp -mux_size 9  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux68.csv -mux spice/subvt_lut.sp -mux_size 68  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux80.csv -mux spice/subvt_lut.sp -mux_size 80  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux100.csv -mux spice/subvt_lut.sp -mux_size 100  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux126.csv -mux spice/subvt_lut.sp -mux_size 126  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux180.csv -mux spice/subvt_lut.sp -mux_size 180  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux230.csv -mux spice/subvt_lut.sp -mux_size 230  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux260.csv -mux spice/subvt_lut.sp -mux_size 260  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux340.csv -mux spice/subvt_lut.sp -mux_size 340  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/local_routing_mux53.csv -mux spice/subvt_lut.sp -mux_size 53  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/local_routing_mux60.csv -mux spice/subvt_lut.sp -mux_size 60 -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux58.csv -mux spice/subvt_lut.sp -mux_size 58  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux90.csv -mux spice/subvt_lut.sp -mux_size 90  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux170.csv -mux spice/subvt_lut.sp -mux_size 170  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux250.csv -mux spice/subvt_lut.sp -mux_size 250  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/local_routing.conf -rpt results/tsmc40nm_fpga/interconn_cbmux330.csv -mux spice/subvt_lut.sp -mux_size 330  -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/sb_route.conf -rpt results/tsmc40nm_fpga/sbmux4.csv -mux spice/subvt_lut.sp -mux_size 4 -auto_out_tapered_buffer 4 -one_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/sb_route.conf -rpt results/tsmc40nm_fpga/sbmux8.csv -mux spice/subvt_lut.sp -mux_size 8 -auto_out_tapered_buffer 4 -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/sb_route.conf -rpt results/tsmc40nm_fpga/sbmux10.csv -mux spice/subvt_lut.sp -mux_size 10 -auto_out_tapered_buffer 4 -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/sb_route_ideal.conf -rpt results/tsmc40nm_fpga/sbmux4_zeroload.csv -mux spice/subvt_lut.sp -mux_size 4 -auto_out_tapered_buffer 4 -one_level_mux cpt
# Fc,in = 0.15, 0.25, 0.5, 0.75, 1
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/cb_route.conf -rpt results/tsmc40nm_fpga/cbmux18_fci0p15.csv -mux spice/subvt_lut.sp -mux_size 18 -auto_out_tapered_buffer 3 -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/cb_route.conf -rpt results/tsmc40nm_fpga/cbmux48_fci0p15.csv -mux spice/subvt_lut.sp -mux_size 48 -auto_out_tapered_buffer 1 -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/cb_route.conf -rpt results/tsmc40nm_fpga/cbmux30_fci0p25.csv -mux spice/subvt_lut.sp -mux_size 30 -auto_out_tapered_buffer 3 -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/cb_route.conf -rpt results/tsmc40nm_fpga/cbmux60_fci0p5.csv -mux spice/subvt_lut.sp -mux_size 60 -auto_out_tapered_buffer 3 -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/cb_route.conf -rpt results/tsmc40nm_fpga/cbmux90_fci0p75.csv -mux spice/subvt_lut.sp -mux_size 90 -auto_out_tapered_buffer 3 -two_level_mux cpt
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/cb_route.conf -rpt results/tsmc40nm_fpga/cbmux120_fci1.csv -mux spice/subvt_lut.sp -mux_size 120 -auto_out_tapered_buffer 3 -two_level_mux cpt 
# Fc,out = 0.1, 0.25, 0.5, 0.75, 1
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/ble_mux2_fco0p1.conf -rpt results/tsmc40nm_fpga/blemux2_fco0p1.csv -mux spice/subvt_lut.sp -mux_size 2 -auto_out_tapered_buffer 3
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/ble_mux2_fco0p25.conf -rpt results/tsmc40nm_fpga/blemux2_fco0p25.csv -mux spice/subvt_lut.sp -mux_size 2 -auto_out_tapered_buffer 4
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/ble_mux2_fco0p5.conf -rpt results/tsmc40nm_fpga/blemux2_fco0p5.csv -mux spice/subvt_lut.sp -mux_size 2 -auto_out_tapered_buffer 4
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/ble_mux2_fco0p75.conf -rpt results/tsmc40nm_fpga/blemux2_fco0p75.csv -mux spice/subvt_lut.sp -mux_size 2 -auto_out_tapered_buffer 5
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/ble_mux2_fco1.conf -rpt results/tsmc40nm_fpga/blemux2_fco1.csv -mux spice/subvt_lut.sp -mux_size 2 -auto_out_tapered_buffer 5

# RRAM FPGA
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_local_routing.conf -rpt results/tsmc40nm_fpga/rram_local_routing_mux53.csv -mux spice/subvt_lut.sp -mux_size 2 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_sb_route.conf -rpt results/tsmc40nm_fpga/rram_sbmux4.csv -mux spice/subvt_lut.sp -mux_size 4 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer 4
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_sb_route_ideal.conf -rpt results/tsmc40nm_fpga/rram_sbmux4_zeroload.csv -mux spice/subvt_lut.sp -mux_size 4 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer 4

# Fc,in = 0.15, 0.25, 0.5, 0.75, 1
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_cb_route.conf -rpt results/tsmc40nm_fpga/rram_cbmux18_fci0p15.csv -mux spice/subvt_lut.sp -mux_size 18 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer 3
# RRAM MUX has a constant delay whatever the input size is, we do not need to run these simulations
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_cb_route.conf -rpt results/tsmc40nm_fpga/rram_cbmux30_fci0p25.csv -mux spice/subvt_lut.sp -mux_size 30 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_cb_route.conf -rpt results/tsmc40nm_fpga/rram_cbmux60_fci0p5.csv -mux spice/subvt_lut.sp -mux_size 60 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_cb_route.conf -rpt results/tsmc40nm_fpga/rram_cbmux90_fci0p75.csv -mux spice/subvt_lut.sp -mux_size 90 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_cb_route.conf -rpt results/tsmc40nm_fpga/rram_cbmux120_fci1.csv -mux spice/subvt_lut.sp -mux_size 120 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer

# Fc,out = 0.1, 0.25, 0.5, 0.75, 1
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_ble_mux2_fco0p1.conf -rpt results/tsmc40nm_fpga/rram_blemux2_fco0p1.csv -mux spice/subvt_lut.sp -mux_size 2 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer 3
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_ble_mux2_fco0p25.conf -rpt results/tsmc40nm_fpga/rram_blemux2_fco0p25.csv -mux spice/subvt_lut.sp -mux_size 2 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer 4
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_ble_mux2_fco0p5.conf -rpt results/tsmc40nm_fpga/rram_blemux2_fco0p5.csv -mux spice/subvt_lut.sp -mux_size 2 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer 4 
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_ble_mux2_fco0p75.conf -rpt results/tsmc40nm_fpga/rram_blemux2_fco0p75.csv -mux spice/subvt_lut.sp -mux_size 2 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer 5
#perl subvt_fpga_elc.pl -conf conf/tsmc40nm_fpga/rram_ble_mux2_fco1.conf -rpt results/tsmc40nm_fpga/rram_blemux2_fco1.csv -mux spice/subvt_lut.sp -mux_size 2 -one_level_mux prog_pair -rram_enhance -wprog_sweep 3 -auto_out_tapered_buffer 5
