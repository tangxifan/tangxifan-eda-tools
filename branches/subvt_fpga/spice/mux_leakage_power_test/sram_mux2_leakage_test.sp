Sub-Vt MUX HSPICE Bench
**********************
*   HSPICE Netlist   *
* Author: Xifan TANG *
*         EPFL, LSI  *
* Date: Thu Aug 17 09:52:22 2017    *
**********************
* Test Bench Usage: mux2_delay_power
* Include Technology Library
.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/tsmc40nm/toplevel_crn45gs_2d5_v1d1_shrink0d9_embedded_usage.l' TOP_TT 
*Design Parameters
.param beta = 2
.param nl = 40n
.param pl = 40n
.param wn = 140n
.param wp = 140n
*Include  ELC NMOS and PMOS Package
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/spice/elc_nmos_pmos.sp'
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/spice/mux_leakage_power_test/sram.sp'
* Working Temperature
.temp 25
* Global Nodes
.global vdd vdd_load
* Print Node Capacitance
.option captab
* Print Waveforms
.option POST
.param vsp=0.9
* Parameters for Measuring Slew
.param slew_upper_threshold_pct_rise=0.9
.param slew_lower_threshold_pct_rise=0.1
.param slew_upper_threshold_pct_fall=0.1
.param slew_lower_threshold_pct_fall=0.9
* Parameters for Measuring Delay
.param input_threshold_pct_rise=0.5
.param input_threshold_pct_fall=0.5
.param output_threshold_pct_rise=0.5
.param output_threshold_pct_fall=0.5
.param input_pwl=5e-10
.param input_pwh=5e-09
.param input_slew=2.5e-11
.param cap_in_wire=1.81e-16
.param cap_out_wire=1.81e-16
* Only for SRAM MUX:
.param w_cpt=2
* Input patterns 
.param in0_voltage_level=0
.param in1_voltage_level=0
* Include Circuits Library
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/spice/subvt_lut.sp
.subckt tapbuf_size4 in out svdd sgnd
Xinvlvl0_no0_tapbuf in_lvl0 in_lvl1 svdd sgnd inv size=1
Xinvlvl1_no0_tapbuf in_lvl1 in_lvl2 svdd sgnd inv size=4
Xinvlvl2_no0_tapbuf in_lvl2 in_lvl3 svdd sgnd inv size=4
Xinvlvl2_no1_tapbuf in_lvl2 in_lvl3 svdd sgnd inv size=4
Xinvlvl2_no2_tapbuf in_lvl2 in_lvl3 svdd sgnd inv size=4
Xinvlvl2_no3_tapbuf in_lvl2 in_lvl3 svdd sgnd inv size=4
Rin in in_lvl0 0
Rout in_lvl3 out 0
.eom
.subckt tapbuf_pg_size4 in out svdd sgnd enb en
Xinvlvl0_no0_tapbuf in_lvl0 in_lvl1 svdd_out sgnd_out inv size=1
* Power-gate transistors: no. 0 
Xpg_pmos0 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos0 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
* Power-gate transistors: no. 1 
Xpg_pmos1 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos1 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
* Power-gate transistors: no. 2 
Xpg_pmos2 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos2 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
* Power-gate transistors: no. 3 
Xpg_pmos3 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos3 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
* Power-gate transistors: no. 4 
Xpg_pmos4 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos4 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
* Power-gate transistors: no. 5 
Xpg_pmos5 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos5 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
* Power-gate transistors: no. 6 
Xpg_pmos6 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos6 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
* Power-gate transistors: no. 7 
Xpg_pmos7 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos7 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
* Power-gate transistors: no. 8 
Xpg_pmos8 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos8 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
* Power-gate transistors: no. 9 
Xpg_pmos9 svdd_out enb svdd svdd elc_pmos L='pl' W='wp*beta'
Xpg_nmos9 sgnd_out en sgnd sgnd elc_nmos L='nl' W='wn'
Rin in in_lvl0 0
Rout in_lvl1 out 0
.eom
* Input inverter subckt size=3 
.subckt input_inv in out svdd sgnd
Xinv0 in out svdd sgnd inv size='3'
.eom

* Power-gated input inverter subckt size=3 
.subckt input_inv_pg in out svdd sgnd enb en
Xinv0 in out svdd_out sgnd_out inv size='3'
* Power-gate transistors 
Xpg_pmos0 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos0 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos1 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos1 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos2 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos2 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos3 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos3 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos4 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos4 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos5 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos5 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos6 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos6 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos7 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos7 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos8 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos8 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos9 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos9 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos10 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos10 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos11 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos11 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
.eom

* output inverter subckt size=3 
.subckt output_inv in out svdd sgnd
Xinv0 in out svdd sgnd inv size='3'
.eom
* Power-gated output inverter subckt size=3 
.subckt output_inv_pg in out svdd sgnd enb en
Xinv0 in out svdd_out sgnd_out inv size='3'
* Power-gate transistors 
Xpg_pmos0 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos0 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos1 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos1 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos2 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos2 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos3 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos3 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos4 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos4 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos5 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos5 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos6 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos6 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos7 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos7 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos8 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos8 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos9 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos9 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos10 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos10 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
Xpg_pmos11 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
Xpg_nmos11 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
.eom
.subckt mux2_size2 sel0 sel_inv0 in0 in1 out svdd sgnd 
Xmux1level_0_wcpt0 mux1level_in0 mux1level_out sel0 sel_inv0 svdd sgnd cpt
Xmux1level_0_wcpt1 mux1level_in0 mux1level_out sel0 sel_inv0 svdd sgnd cpt
Xmux1level_0_wcpt2 mux1level_in0 mux1level_out sel0 sel_inv0 svdd sgnd cpt size=1
Xmux1level_1_wcpt0 mux1level_in1 mux1level_out sel_inv0 sel0 svdd sgnd cpt
Xmux1level_1_wcpt1 mux1level_in1 mux1level_out sel_inv0 sel0 svdd sgnd cpt
Xmux1level_1_wcpt2 mux1level_in1 mux1level_out sel_inv0 sel0 svdd sgnd cpt size=1
Xinv0 in0 mux1level_in0 svdd sgnd input_inv
Cwire_in0 mux1level_in0 sgnd cap_in_wire
Cwire_out0 mux1level_out sgnd cap_out_wire
Xinv1 in1 mux1level_in1 svdd sgnd input_inv
Cwire_in1 mux1level_in1 sgnd cap_in_wire
Cwire_out1 mux1level_out sgnd cap_out_wire
Xoutput_inv mux1level_out out svdd sgnd output_inv
*Xtapbuf mux1level_out out svdd sgnd tapbuf_size4
.eom mux2_size2
Xmux2_size2 sel0 sel_inv0 in0 in1 out vdd gnd mux2_size2
* Include SRAM 
Xsram6T_0 sel0 sel0 sel_inv0 vdd gnd sram6T
Xinv_load0 out inv_load0_out vdd_load gnd inv size=1
* Common Part Over.
Vsupply vdd gnd vsp
Vload_supply vdd_load 0 vsp
Vgnd gnd 0 0
Vsram0 sel0 gnd vsp
Vinv_sram0 sel_inv0 gnd 0
*Vin0 in0 gnd pwl(0 vsp input_pwl vsp 'input_pwl+input_slew' 0 'input_pwl+input_slew+input_pwh' 0)
Vin0 in0 gnd 'in0_voltage_level'
Vin1 in1 gnd 'in1_voltage_level'

.tran 1e-13 5.525e-09

.measure tran pleak avg p(vsupply) from=0 to='input_pwl'

****** Alter cases and run again *****
.alter case 1: in0 = 0, in1 = 1
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.end Sub-Vt MUX HSPICE Bench
