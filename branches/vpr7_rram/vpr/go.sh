#! /bin/csh -f
# Example of how to run vpr

# Set variables
# For FPGA-Verilog ONLY 
set verilog_output_dirname = sram_fpga_hetero
set verilog_output_dirpath = $PWD
set modelsim_ini_file = /uusoc/facility/cad_tools/Mentor/modelsim10.7b/modeltech/modelsim.ini
# VPR critical inputs
#set arch_xml_file = ARCH/k6_N10_MD_tsmc40nm_chain_TT.xml 
set arch_xml_file = ARCH/k6_N10_SC_tsmc40nm_chain_TT.xml 
#set arch_xml_file = ARCH/k6_N10_SC_tsmc40nm_chain_TT_yosys.xml 
#set arch_xml_file = ARCH/k6_N10_sram_chain_SC_gf130_2x2.xml
#set blif_file = Circuits/ssp_top_prevpr_ace.blif 
#set act_file = Circuits/ssp_top_prevpr_ace.act 
#set blif_file = Circuits/uart_top_prevpr_ace.blif 
#set act_file = Circuits/uart_top_prevpr_ace.act 
set blif_file = Circuits/s298_prevpr.blif 
set act_file = Circuits/s298_prevpr.act 
#set blif_file = Circuits/add_fs.blif 
#set act_file = Circuits/add_fs.act 
#set blif_file = Circuits/picorv_prevpr_ace.blif 
#set act_file = Circuits/picorv_prevpr_ace.act 
set vpr_route_chan_width = 100

# Step A: Make sure a clean start 
# Recompile if needed
#make clean
#make -j32
# Remove previous designs
rm -rf $verilog_output_dirpath/$verilog_output_dirname

# Run VPR  
#valgrind 
#./vpr $arch_xml_file $blif_file --full_stats --nodisp --activity_file $act_file --fpga_verilog --fpga_verilog_dir $verilog_output_dirpath/$verilog_output_dirname --fpga_x2p_rename_illegal_port --route_chan_width $vpr_route_chan_width --fpga_bitstream_generator --fpga_verilog_print_top_testbench --fpga_verilog_print_input_blif_testbench --fpga_verilog_include_timing --fpga_verilog_print_modelsim_autodeck --fpga_verilog_modelsim_ini_path $modelsim_ini_file
./vpr $arch_xml_file $blif_file --full_stats --nodisp --activity_file $act_file --fpga_verilog --fpga_verilog_dir $verilog_output_dirpath/$verilog_output_dirname --fpga_x2p_rename_illegal_port --fpga_bitstream_generator --fpga_verilog_print_top_testbench --fpga_verilog_print_input_blif_testbench --fpga_verilog_include_timing --fpga_verilog_print_modelsim_autodeck --fpga_verilog_modelsim_ini_path $modelsim_ini_file --fpga_verilog_print_formal_verification_top_netlist



