#! /bin/csh -f
# Make sure a clear start
set pwd_path = $PWD
set task_name = k6_N10_MD_frac6_chain_sram_tsmc40nm_TT
set config_file = $PWD/configs/fpga_spice/${task_name}.conf 
#set bench_txt = $PWD/benchmarks/fpga_spice_bench.txt
set bench_txt = $PWD/benchmarks/mcnc_big20.txt
set rpt_file = $PWD/csv_rpts/fpga_spice/${task_name}.csv
set task_file = $PWD/vpr_fpga_spice_task_lists/${task_name}

# FPGA-SPICE
rm -rf ${pwd_path}/results
cd ${pwd_path}/scripts

# SRAM FPGA
# TT case 
perl fpga_flow.pl -conf ${config_file} -benchmark ${bench_txt} -rpt ${rpt_file} -N 10 -K 6 -ace_d 0.5 -power -remove_designs -multi_thread 1 -vpr_fpga_x2p_rename_illegal_port -vpr_fpga_verilog -vpr_fpga_verilog_print_top_tb -vpr_fpga_verilog_print_input_blif_tb -vpr_fpga_bitstream_generator  

cd ${pwd_path}


