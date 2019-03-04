# Regular VPR commands
# Setup VPR
vpr_setup -blif_file ./Circuits/s298_prevpr.blif -arch_file ./ARCH/k8_N10_sram_chain_FC_tsmc40_stratix4_auto.xml -activity_file ./Circuits/s298_prevpr.act -read_xml_fpga_x2p
# Run packer
vpr_pack
# Run place and route
vpr_place_and_route 
# Run FPGA SPICE
fpga_x2p_setup
fpga_spice
# FPGA Verilog
#fpga_verilog
# Bitstream generation
#fpga_bitstream
# END
