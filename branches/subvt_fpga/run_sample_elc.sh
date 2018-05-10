set lut_size_list = (6)
set mux_size_list = (32)

# Run D-type Flip-Flop (DFF)  Electrical Simulations  
perl subvt_fpga_elc.pl -conf conf/sample.conf -rpt results/sample_dff_rpt.csv -dff spice/subvt_dff.sp 

# Run Look-Up Table (LUT) Electrical Simulations
foreach j ($lut_size_list) 
  perl subvt_fpga_elc.pl -conf conf/sample.conf -rpt results/sample_lut$j\_rpt.csv -lut spice/subvt_lut.sp -lut_size $j
end

# Run Multiplexer ELectrical Simulations
foreach j ($mux_size_list) 
  # SRAM MUX Electrical Simulation: 1-level structure 
  perl subvt_fpga_elc.pl -conf conf/sample.conf -rpt results/sample_sram_1level_mux$j\_rpt.csv -mux spice/subvt_lut.sp -mux_size $j -one_level_mux cpt
  # SRAM MUX Electrical Simulation: 2-level structure 
  perl subvt_fpga_elc.pl -conf conf/sample.conf -rpt results/sample_sram_2level_mux$j\_rpt.csv -mux spice/subvt_lut.sp -mux_size $j -two_level_mux cpt
  # SRAM MUX Electrical Simulation: 2-level structure, sweeping the size of transmission gates 
  perl subvt_fpga_elc.pl -conf conf/sample.conf -rpt results/sample_sram_2level_mux$j\_wcpt_sweep_rpt.csv -mux spice/subvt_lut.sp -mux_size $j -two_level_mux cpt -w_cpt_sweep 3
  # RRAM MUX Electrical Simulation: basic design, 1-level structure 
  perl subvt_fpga_elc.pl -conf conf/sample.conf -rpt results/sample_basic_rram_mux$j\_rpt.csv -mux spice/subvt_lut.sp -mux_size $j -one_level_mux prog_pair -rram_enhance -reset_trans_ratio 1.05
  # RRAM MUX Electrical Simulation: advanced design, 1-level structure 
  perl subvt_fpga_elc.pl -conf conf/sample.conf -rpt results/sample_rram_mux$j\_rpt.csv -mux spice/subvt_lut.sp -mux_size $j -one_level_mux prog_pair -rram_enhance -advance_rram_mux -reset_trans_ratio 1.05
  # RRAM MUX Electrical Simulation: advanced design, 1-level structure, sweeping the size of programming transistors 
  perl subvt_fpga_elc.pl -conf conf/sample.conf -rpt results/sample_rram_mux$j\_wprog_sweep_rpt.csv -mux spice/subvt_lut.sp -mux_size $j -one_level_mux prog_pair -rram_enhance -advance_rram_mux -reset_trans_ratio 1.05 -wprog_sweep 3
end
