Sub-Vt MUX HSPICE Bench
**********************
*   HSPICE Netlist   *
* Author: Xifan TANG *
*         EPFL, LSI  *
* Date: Tue Jun 14 11:44:06 2016    *
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
* Technology Library for RRAM Programming Transistors is same with standard transistors.
* Additional tech. library is included.

*Design Parameters for RRAM programming transistors
.param prog_beta = 2
.param prog_nl = 40n
.param prog_pl = 40n
.param prog_wn = 140n
.param prog_wp = 140n
*Include  ELC NMOS and PMOS Package
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/spice/elc_nmos_pmos.sp'
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
.param cap_out_wire=1e-20
* Only for SRAM MUX:
.param w_cpt=0
.param N=2
* Only for RRAM MUX:
.param gap_on=3.619e-10
.param gap_off=24.8e-10
.param wprog=1.4
.param tprog=200n
.param vprog=2.4
.param c_rram=13.2e-18
* Include RRAM verilogA model 
*.hdl 'rram_v_1_0_0_fit_finfet_0p8v.va'
*.hdl 'rram_v_1_0_0_fit_finfet_0p6v.va'
*.hdl 'rram_v_1_0_0_fit_finfet_0p4v.va'
*.hdl 'rram_v_1_0_0_fit_finfet_0p3v.va'
*.hdl 'rram_v_1_0_0_fit_finfet_0p2v.va'
*.hdl 'rram_v_1_0_0_fit_finfet_0p1v.va'
.hdl 'rram_v_1_0_0_fit_40nm_1p1V.va'
.param op_clk_period='1.25e-9'
* Input patterns 
.param in0_voltage_level=0
.param in1_voltage_level=0
* RHRS
.param rhrs = 10e6
* Include Circuits Library
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/spice/subvt_lut.sp

.subckt tapbuf_size4 in out svdd sgnd
Xinvlvl0_no0_tapbuf in_lvl0 in_lvl1 svdd sgnd inv size=1
Rin in in_lvl0 0
Rout in_lvl1 out 0
*Xinvlvl0_no0_tapbuf in_lvl0 in_lvl1 svdd sgnd inv size=1
*Xinvlvl1_no0_tapbuf in_lvl1 in_lvl2 svdd sgnd inv size=4
*Xinvlvl2_no0_tapbuf in_lvl2 in_lvl3 svdd sgnd inv size=4
*Xinvlvl2_no1_tapbuf in_lvl2 in_lvl3 svdd sgnd inv size=4
*Xinvlvl2_no2_tapbuf in_lvl2 in_lvl3 svdd sgnd inv size=4
*Xinvlvl2_no3_tapbuf in_lvl2 in_lvl3 svdd sgnd inv size=4
*Rin in in_lvl0 0
*Rout in_lvl3 out 0
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
*Xpg_pmos2 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos2 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos3 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos3 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos4 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos4 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos5 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos5 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos6 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos6 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos7 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos7 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos8 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos8 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos9 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos9 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos10 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos10 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos11 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos11 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
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
*Xpg_pmos2 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos2 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos3 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos3 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos4 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos4 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos5 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos5 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos6 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos6 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos7 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos7 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos8 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos8 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos9 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos9 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos10 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos10 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
*Xpg_pmos11 svdd_out enb svdd svdd elc_pmos L='pl' W='2.5*wp*beta'
*Xpg_nmos11 sgnd_out en sgnd sgnd elc_nmos L='nl' W='2.5*wn'
.eom
.param N_RRAM_TO_SET=1
.param N_RRAM_TO_RST=1
.param vsp_prog=1.2
.global prog_vdd0 prog_gnd0 prog_vdd1 prog_gnd1 
+ op_mode_enb0 op_mode_en0 
+ op_mode_enb1 op_mode_en1 
+ prog_clk op_clk
+ bl[0]_b wl[0] 
+ bl[1]_b wl[1] 
+ bl[2]_b wl[2] 

.subckt mux2_size2 sel0 sel_inv0 sel1 sel_inv1 in0 in1 out svdd sgnd 
Xprog_pmos0 mux1level_in0 bl[0]_b prog_vdd0 prog_vdd0 elc_pmos L='prog_pl' W='wprog*prog_wp*prog_beta'
Xprog_nmos0 mux1level_in0 wl[0] prog_gnd0 prog_gnd0 elc_nmos L='prog_nl' W='wprog*prog_wn'
Xrram0 mux1level_in0 mux1level_out rram_v_1_0_0 gap_ini=gap_on 
*print_gap=1
Cmux1level_0 mux1level_out mux1level_in0 'c_rram'
*.print V(Xrram0.R_out) V(Xrram0.gap_out)
Cwire_in0 mux1level_in0 sgnd 'cap_in_wire'
Cwire_out0 mux1level_out sgnd 'cap_out_wire'
Xprog_pmos1 mux1level_in1 bl[1]_b prog_vdd0 prog_vdd0 elc_pmos L='prog_pl' W='wprog*prog_wp*prog_beta'
Xprog_nmos1 mux1level_in1 wl[1] prog_gnd0 prog_gnd0 elc_nmos L='prog_nl' W='wprog*prog_wn'
*Xrram1 mux1level_in1 mux1level_out rram_v_1_0_0 gap_ini=gap_off
Rrram1 mux1level_in1 mux1level_out 'rhrs'
Cmux1level_1 mux1level_in1 mux1level_out 'c_rram'
*.print V(Xrram1.R_out) V(Xrram1.gap_out)
Cwire_in1 mux1level_in1 sgnd 'cap_in_wire'
Cwire_out1 mux1level_out sgnd 'cap_out_wire'
Xprog_pmos2 mux1level_out bl[2]_b prog_vdd1 prog_vdd1 elc_pmos L='prog_pl' W='wprog*prog_wp*prog_beta'
Xprog_nmos2 mux1level_out wl[2] prog_gnd1 prog_gnd1 elc_nmos L='prog_nl' W='wprog*prog_wn'
Xinv0 in0 mux1level_in0 prog_vdd0 prog_gnd0 op_mode_enb0 op_mode_en0 input_inv_pg
Xinv1 in1 mux1level_in1 prog_vdd0 prog_gnd0 op_mode_enb0 op_mode_en0 input_inv_pg
Xtapbuf mux1level_out out prog_vdd1 prog_gnd1 tapbuf_size4
.eom mux2_size2


Xmux2_size2 sel0 sel_inv0 sel1 sel_inv1 in0 in1 out vdd gnd mux2_size2
Xinv_load0_0 out inv_load0_out0 vdd_load gnd inv size=1
* Common Part Over.
* PUSLE(V1 V2 TDELAY TRISE TFALL PW PERIOD)
* Programming clock
*Vprog_clk prog_clk 0 pulse(0 'vsp' 'tprog/2-input_slew' 'input_slew' 'input_slew' 'tprog/2 - input_slew' 'tprog')
* Operating clock: delay = no. of cycles for SET and RESET + 1 switching cycle
Vop_clk op_clk 0 pulse(0 vsp 0 'input_slew' 'input_slew' 'op_clk_period/2 - input_slew' 'op_clk_period')
* VDD of loads
Vload_vdd vdd_load 0 'vsp'
* Dynamic power rails: prog_vdd0 
Vprog_vdd0 prog_vdd0 0 'vsp'
* Constant power rails: prog_gnd0 
Vprog_gnd0 prog_gnd0 0 0
* Dynamic power rails: prog_vdd1 
Vprog_vdd1 prog_vdd1 0 'vsp'
Vprog_gnd1 prog_gnd1 0 0
* Control signals for Bit lines and Word lines
* WL/BL lines for Level 0
* Control signals for RESET process
Vbl[2]_b bl[2]_b 0 'vsp' 
Vwl[1] wl[1] 0 0
* Control signals for SET process
Vbl[0]_b bl[0]_b 0 'vsp'
Vwl[2] wl[2] 0 0
* Other Bit lines and Word lines are always disabled
Vbl[1]_b bl[1]_b 0 'vsp'
Vwl[0] wl[0] 0 0


* Power-gate control signal for input inverters
Vop_mode_enb0 op_mode_enb0 0 0 
Vop_mode_en0 op_mode_en0 0 'vsp'
* Power-gate control signal for output inverters
Vop_mode_enb1 op_mode_enb1 0 0
Vop_mode_en1 op_mode_en1 0 'vsp'

* MUX inputs signals
Vin0 in0 gnd 'in0_voltage_level'
Vin1 in1 gnd 'in1_voltage_level'

.parameter T_torture = 1
* Transient simulation time period
.tran 1e-13 'T_torture*op_clk_period'
.measure tran pleak_prog_vdd0 avg p(Vprog_vdd0) from=0 to='T_torture*op_clk_period'
.measure tran pleak_prog_vdd1 avg p(Vprog_vdd1) from=0 to='T_torture*op_clk_period'
.measure tran pleak param='abs(pleak_prog_vdd0) + abs(pleak_prog_vdd1)'

****** Default case: in0 = 0, in1 = 0, rhrs=10e6
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

.alter case 0: in0 = 0, in1 = 0, rhrs=20e6
.param rhrs = 20e6
.param in0_voltage_level = '0'
.param in1_voltage_level = '0'

.alter case 1: in0 = 0, in1 = 1, rhrs=20e6
.param rhrs = 20e6
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0, rhrs=20e6
.param rhrs = 20e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1, rhrs=30e6
.param rhrs = 30e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.alter case 0: in0 = 0, in1 = 0, rhrs=30e6
.param rhrs = 30e6
.param in0_voltage_level = '0'
.param in1_voltage_level = '0'

.alter case 1: in0 = 0, in1 = 1, rhrs=30e6
.param rhrs = 30e6
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0, rhrs=30e6
.param rhrs = 30e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1, rhrs=30e6
.param rhrs = 30e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.alter case 0: in0 = 0, in1 = 0, rhrs=40e6
.param rhrs = 40e6
.param in0_voltage_level = '0'
.param in1_voltage_level = '0'

.alter case 1: in0 = 0, in1 = 1, rhrs=40e6
.param rhrs = 40e6
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0, rhrs=40e6
.param rhrs = 40e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1, rhrs=40e6
.param rhrs = 40e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.alter case 0: in0 = 0, in1 = 0, rhrs=50e6
.param rhrs = 50e6
.param in0_voltage_level = '0'
.param in1_voltage_level = '0'

.alter case 1: in0 = 0, in1 = 1, rhrs=50e6
.param rhrs = 50e6
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0, rhrs=50e6
.param rhrs = 50e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1, rhrs=50e6
.param rhrs = 50e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.alter case 0: in0 = 0, in1 = 0, rhrs=60e6
.param rhrs = 60e6
.param in0_voltage_level = '0'
.param in1_voltage_level = '0'

.alter case 1: in0 = 0, in1 = 1, rhrs=60e6
.param rhrs = 60e6
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0, rhrs=60e6
.param rhrs = 60e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1, rhrs=60e6
.param rhrs = 60e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.alter case 0: in0 = 0, in1 = 0, rhrs=70e6
.param rhrs = 70e6
.param in0_voltage_level = '0'
.param in1_voltage_level = '0'

.alter case 1: in0 = 0, in1 = 1, rhrs=70e6
.param rhrs = 70e6
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0, rhrs=70e6
.param rhrs = 70e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1, rhrs=70e6
.param rhrs = 70e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.alter case 0: in0 = 0, in1 = 0, rhrs=80e6
.param rhrs = 80e6
.param in0_voltage_level = '0'
.param in1_voltage_level = '0'

.alter case 1: in0 = 0, in1 = 1, rhrs=80e6
.param rhrs = 80e6
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0, rhrs=80e6
.param rhrs = 80e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1, rhrs=80e6
.param rhrs = 80e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.alter case 0: in0 = 0, in1 = 0, rhrs=90e6
.param rhrs = 90e6
.param in0_voltage_level = '0'
.param in1_voltage_level = '0'

.alter case 1: in0 = 0, in1 = 1, rhrs=90e6
.param rhrs = 90e6
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0, rhrs=90e6
.param rhrs = 90e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1, rhrs=90e6
.param rhrs = 90e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.alter case 0: in0 = 0, in1 = 0, rhrs=100e6
.param rhrs = 100e6
.param in0_voltage_level = '0'
.param in1_voltage_level = '0'

.alter case 1: in0 = 0, in1 = 1, rhrs=100e6
.param rhrs = 100e6
.param in0_voltage_level = '0'
.param in1_voltage_level = 'vsp'

.alter case 2: in0 = 1, in1 = 0, rhrs=100e6
.param rhrs = 100e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = '0'

.alter case 3: in0 = 1, in1 = 1, rhrs=100e6
.param rhrs = 100e6
.param in0_voltage_level = 'vsp'
.param in1_voltage_level = 'vsp'

.end Sub-Vt MUX HSPICE Bench
