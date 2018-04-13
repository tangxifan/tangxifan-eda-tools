#!/bin/csh

# FPGA-SPICE

set vdd_list = (8 7)
foreach j ($vdd_list)
  # Make sure a clear start
  #rm -rf ./results
  cd ./scripts

  # SRAM FPGA
  #perl fpga_flow.pl -conf ../configs/fpga_spice/k6_N10_sram_tsmc40nm_nearVt0p$j\V.conf -benchmark ../benchmarks/mcnc_big20.txt -rpt ../csv_rpts/fpga_spice/k6_N10_sram_tsmc40nm_nearVt0p$j\V_mcncbig20_part2.csv -N 10 -K 6 -power -remove_designs -multi_thread 1 -fix_route_chan_width -vpr_fpga_spice ../vpr_fpga_spice_task_lists/k6_N10_sram_tsmc40nm -vpr_fpga_spice_rename_illegal_port -vpr_fpga_spice_print_component_tb  #-vpr_fpga_spice_parasitic_net_estimation_off #-vpr_fpga_spice_leakage_only

  #perl run_fpga_spice.pl -conf ../vpr_fpga_spice_conf/sample.conf -task ../vpr_fpga_spice_task_lists/k6_N10_sram_tsmc40nm_standard.txt -rpt ../vpr_fpga_spice_csv_rpts/k6_N10_sram_tsmc40_nearVt0p$j\V_spice_mcncbig20_part2.csv -parse_pb_mux_tb -parse_cb_mux_tb -parse_sb_mux_tb -parse_lut_tb -parse_dff_tb -multi_thread 6
  cd ..
end

# Make sure a clear start
#rm -rf ./results
cd ./scripts
# FPGA-SPICE

# SRAM FPGA
#perl fpga_flow.pl -conf ../configs/fpga_spice/k6_N10_sram_tsmc40nm.conf -benchmark ../benchmarks/mcnc_big20.txt -rpt ../csv_rpts/fpga_spice/k6_N10_sram_tsmc40nm_mcncbig20_part2.csv -N 10 -K 6 -power -remove_designs -multi_thread 1 -fix_route_chan_width -vpr_fpga_spice ../vpr_fpga_spice_task_lists/k6_N10_sram_tsmc40nm -vpr_fpga_spice_rename_illegal_port -vpr_fpga_spice_print_component_tb -vpr_fpga_spice_print_grid_tb -vpr_fpga_spice_print_top_tb  #-vpr_fpga_spice_parasitic_net_estimation_off #-vpr_fpga_spice_leakage_only

#perl run_fpga_spice.pl -conf ../vpr_fpga_spice_conf/sample.conf -task ../vpr_fpga_spice_task_lists/k6_N10_sram_tsmc40nm_standard.txt -rpt ../vpr_fpga_spice_csv_rpts/k6_N10_sram_tsmc40_spice_mcncbig20_part2.csv -parse_pb_mux_tb -parse_cb_mux_tb -parse_sb_mux_tb -parse_lut_tb -parse_hardlogic_tb -parse_grid_tb -parse_cb_tb -parse_sb_tb -parse_top_tb -multi_thread 6

# Verilog Generator 
perl fpga_flow.pl -conf ../configs/fpga_spice/k6_N10_sram_tsmc40nm.conf -benchmark ../benchmarks/mcnc_big20.txt -rpt ../csv_rpts/fpga_spice/k6_N10_sram_tsmc40nm_mcncbig20.csv -N 10 -K 6 -power -remove_designs -multi_thread 1 -fix_route_chan_width  ../vpr_fpga_spice_task_lists/k6_N10_sram_tsmc40nm -vpr_fpga_spice_rename_illegal_port -vpr_fpga_spice_verilog_generator  #-vpr_fpga_spice_parasitic_net_estimation_off #-vpr_fpga_spice_leakage_only
cd ..

# RRAM FPGA
#set rhrs_list = (500 100 30)
set rhrs_list = (20)
foreach j ($rhrs_list)
#while ($j <=100)
  #rm -rf ./results
  cd ./scripts
  #perl fpga_flow.pl -conf ../configs/fpga_spice/k6_N10_rram_tsmc40nm_nearVt0p7V_RHRS$j\MOhm.conf -benchmark ../benchmarks/mcnc_big20.txt -rpt ../csv_rpts/fpga_spice/k6_N10_rram_tsmc40nm_nearVt0p7V_mcncbig20.csv -N 10 -K 6 -power -remove_designs -multi_thread 1 -fix_route_chan_width -vpr_fpga_spice ../vpr_fpga_spice_task_lists/k6_N10_rram_tsmc40nm_nearVt -vpr_fpga_spice_rename_illegal_port -vpr_fpga_spice_print_component_tb  #-vpr_fpga_spice_parasitic_net_estimation_off #-vpr_fpga_spice_leakage_only

  #perl run_fpga_spice.pl -conf ../vpr_fpga_spice_conf/sample.conf -task ../vpr_fpga_spice_task_lists/k6_N10_rram_tsmc40nm_nearVt_standard.txt -rpt ../vpr_fpga_spice_csv_rpts/k6_N10_rram_tsmc40_nearVt0p7V_spice_mcncbig20_RHRS$j\MOhm.csv -parse_pb_mux_tb -parse_cb_mux_tb -parse_sb_mux_tb -parse_lut_tb -parse_dff_tb -multi_thread 6
  #@ j += 70
  cd ..
end


#set rhrs_list = (500 100 30)
set rhrs_list = (20)
foreach j ($rhrs_list)
#while ($j <=100)
  #rm -rf ./results
  cd ./scripts
  #perl fpga_flow.pl -conf ../configs/fpga_spice/k6_N10_rram_tsmc40nm_nearVt0p8V_RHRS$j\MOhm.conf -benchmark ../benchmarks/mcnc_big20.txt -rpt ../csv_rpts/fpga_spice/k6_N10_rram_tsmc40nm_nearVt0p8V_mcncbig20.csv -N 10 -K 6 -power -remove_designs -multi_thread 1 -fix_route_chan_width -vpr_fpga_spice ../vpr_fpga_spice_task_lists/k6_N10_rram_tsmc40nm_nearVt -vpr_fpga_spice_rename_illegal_port -vpr_fpga_spice_print_component_tb  #-vpr_fpga_spice_parasitic_net_estimation_off #-vpr_fpga_spice_leakage_only

  #perl run_fpga_spice.pl -conf ../vpr_fpga_spice_conf/sample.conf -task ../vpr_fpga_spice_task_lists/k6_N10_rram_tsmc40nm_nearVt_standard.txt -rpt ../vpr_fpga_spice_csv_rpts/k6_N10_rram_tsmc40_nearVt0p8V_spice_mcncbig20_RHRS$j\MOhm.csv -parse_pb_mux_tb -parse_cb_mux_tb -parse_sb_mux_tb -parse_lut_tb -parse_dff_tb -multi_thread 6
  #@ j += 70
  cd ..
end

# Sweep RHRS from 10MOhm to 100MOhm with a step of 10MOhm
set rhrs_list = (100)
#set rhrs_list = (10 20 30 100 40 50 60 70 80 90)
foreach j ($rhrs_list)
#set j = 100
#while ($j >= 10)
  #rm -rf ./results
  cd ./scripts
  #perl fpga_flow.pl -conf ../configs/fpga_spice/k6_N10_rram_tsmc40nm_RHRS$j\MOhm.conf -benchmark ../benchmarks/mcnc_big20.txt -rpt ../csv_rpts/fpga_spice/k6_N10_rram_tsmc40nm_mcncbig20.csv -N 10 -K 6 -power -remove_designs -multi_thread 1 -fix_route_chan_width -vpr_fpga_spice ../vpr_fpga_spice_task_lists/k6_N10_rram_tsmc40nm -vpr_fpga_spice_rename_illegal_port -vpr_fpga_spice_print_component_tb  #-vpr_fpga_spice_parasitic_net_estimation_off #-vpr_fpga_spice_leakage_only

  #perl run_fpga_spice.pl -conf ../vpr_fpga_spice_conf/sample.conf -task ../vpr_fpga_spice_task_lists/k6_N10_rram_tsmc40nm_standard.txt -rpt ../vpr_fpga_spice_csv_rpts/k6_N10_rram_tsmc40_spice_mcncbig20_RHRS$j\MOhm.csv -parse_pb_mux_tb -parse_cb_mux_tb -parse_sb_mux_tb -parse_lut_tb -parse_hardlogic_tb -multi_thread 6 -monte_carlo detail_rpt
  #@ j -= 10
  cd ..
end

# Sweep RHRS from 10MOhm to 100MOhm with a step of 10MOhm
set rhrs_list = (30)
#set rhrs_list = (10 20 30 100 40 50 60 70 80 90)
foreach j ($rhrs_list)
#set j = 100
#while ($j >= 10)
  #rm -rf ./results
  cd ./scripts
  #perl fpga_flow.pl -conf ../configs/fpga_spice/k6_N10_rram_tsmc40nm_RHRS$j\MOhm.conf -benchmark ../benchmarks/mcnc_big20.txt -rpt ../csv_rpts/fpga_spice/k6_N10_rram_tsmc40nm_mcncbig20.csv -N 10 -K 6 -power -remove_designs -multi_thread 1 -fix_route_chan_width -vpr_fpga_spice ../vpr_fpga_spice_task_lists/k6_N10_rram_tsmc40nm -vpr_fpga_spice_rename_illegal_port -vpr_fpga_spice_print_top_tb  #-vpr_fpga_spice_parasitic_net_estimation_off #-vpr_fpga_spice_leakage_only

  #perl run_fpga_spice.pl -conf ../vpr_fpga_spice_conf/sample.conf -task ../vpr_fpga_spice_task_lists/k6_N10_rram_tsmc40nm_standard.txt -rpt ../vpr_fpga_spice_csv_rpts/k6_N10_rram_tsmc40_spice_mcncbig20_RHRS$j\MOhm.csv -parse_top_tb -multi_thread 6 -monte_carlo detail_rpt
  #@ j -= 10
  cd ..
end

