MOS capacitance 
* Technology
*.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/MM180_LVT18_V113.lib' TT
*.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/tsmc40nm.sp'
*.include './process/22nm_HP.pm'
*.include './process/45nm_LP.pm'
*.include './process/st_28nm.sp'

* TSMC40nm
*.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/tsmc40nm/toplevel_crn45gs_2d5_v1d1_shrink0d9_embedded_usage.l' TOP_TT

* Leti 130nm 
.include '/research/ece/lnis/USERS/tang/github/tangxifan-eda-tools/branches/subvt_fpga/process/leti130nm/corners'
.include '/research/ece/lnis/USERS/tang/github/tangxifan-eda-tools/branches/subvt_fpga/process/leti130nm/CORE9A85L05.spi'

* General-purpose NMOS/PMOS wrapper
.include '/research/ece/lnis/USERS/tang/github/tangxifan-eda-tools/branches/subvt_fpga/spice/nmos_pmos.sp'

.temp 25
.param beta = 1.5
.param alpha = 1

* TSMC 40nm Tech. Logic trans.
*.param nl = 270e-9
*.param pl = 270e-9
*.param wn = '320e-9'
*.param wp = 'beta*320e-9'

* TSMC 40nm Tech. Logic trans.
*.param nl = 40e-9
*.param pl = 40e-9
*.param wn = '140e-9'
*.param wp = 'beta*140e-9'

* Leti 130nm tech. logic trans.
.param nl = 0.35e-6
.param pl = 0.35e-6
.param wn = 0.5e-6
.param wp = 'beta*0.5e-6'

*.param clk_freq = 1e8

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
.param input_pwl=2e-09
.param input_pwh=2e-08
.param input_slew=1e-10

.option POST
.option captab

.param vsp = 0.9

* Test case 1 : NMOS OPEN CAP, VDD
X1 i1 vdd t1 gnd nmos L=nl W=wn
* Test case 2 : NMOS OPEN CAP, GND
X2 i2 vdd t2 gnd nmos L=nl W=wn
* Test case 3 : PMOS OPEN CAP, VDD
X3 i3 gnd t3 vdd pmos L=pl W=wp
* Test case 4 : PMOS OPEN CAP, GND
X4 i4 gnd t4 vdd pmos L=pl W=wp
* Test case 5 : Inverter, NMOS OPEN
X5_p vdd i5 t5 vdd pmos L=pl W=wp
X5_n gnd i5 t5 gnd nmos L=nl W=wn
* Test case 6 : Inverter, PMOS OPEN
X6_p vdd i6 t6 vdd pmos L=pl W=wp
X6_n gnd i6 t6 gnd nmos L=nl W=wn
* Test case 7 : Inverter, rising edge
X7_p vdd i7 t7 vdd pmos L=pl W=wp
X7_n gnd i7 t7 gnd nmos L=nl W=wn
*C7   t7 gnd 1p
* Test case 8 : Inverter, falling edge
X8_p vdd i8 t8 vdd pmos L=pl W=wp
X8_n gnd i8 t8 gnd nmos L=nl W=wn
*C8   t8 gnd 1p
* Test case 9 : NMOS OFF CAP, VDD,GND
X9 i9 gnd t9 gnd nmos L=nl W=wn
* Test case 10 : NMOS OFF CAP, VDD,VDD
X10 i10 gnd t10 gnd nmos L=nl W=wn
* Test case 11 : NMOS OFF CAP, GND,GND
X11 i11 gnd t11 gnd nmos L=nl W=wn
* Test case 12 : PMOS OFF CAP, VDD,GND
X12 i12 vdd t12 gnd pmos L=pl W=wp
* Test case 13 : PMOS OFF CAP, VDD, VDD
X13 i13 vdd t13 vdd pmos L=pl W=wp
* Test case 14 : PMOS OFF CAP, GND, GND
X14 i14 vdd t14 vdd pmos L=pl W=wp

* Voltage & Current Source
Vsupply vdd gnd vsp
Vi1 i1 gnd vsp
Vi2 i2 gnd 0
Vi3 i3 gnd vsp
Vi4 i4 gnd 0
Vi5 i5 gnd vsp
Vi6 i6 gnd 0  
Vi7 i7 gnd pwl(0 0 input_pwl 0 'input_pwl+input_slew' vsp 'input_pwl+input_slew+input_pwh' vsp)
Vi8 i8 gnd pwl(0 vsp input_pwl vsp 'input_pwl+input_slew' 0 'input_pwl+input_slew+input_pwh' 0)
Vi9 i9 gnd vsp
Vt9 t9 gnd 0
Vi10 i10 gnd vsp
Vt10 t10 gnd vsp
Vi11 i11 gnd 0
Vt11 t11 gnd 0
Vi12 i12 gnd vsp
Vt12 t12 gnd 0
Vi13 i13 gnd vsp
Vt13 t13 gnd vsp
Vi14 i14 gnd 0
Vt14 t14 gnd 0
* Transistion
.tran 1e-15 2.21e-08
.measure tran leak_power find p(Vsupply) at=1e-08
.measure tran dly_t7 trig v(i7) val='input_threshold_pct_rise*vsp' rise=1
+                     targ v(t7) val='output_threshold_pct_fall*vsp' fall=1       td='input_pwl'
.measure tran dly_t8 trig v(i8) val='input_threshold_pct_fall*vsp' fall=1
+                     targ v(t8) val='output_threshold_pct_rise*vsp' rise=1       td='input_pwl'
.measure tran slew_t7 trig v(t7) val='slew_lower_threshold_pct_fall*vsp' fall=1 td='input_pwl'
+                      targ v(t7) val='slew_upper_threshold_pct_fall*vsp' fall=1 td='input_pwl'
.measure tran slew_t8 trig v(t8) val='slew_lower_threshold_pct_rise*vsp' rise=1 td='input_pwl'
+                      targ v(t8) val='slew_upper_threshold_pct_rise*vsp' rise=1 td='input_pwl'
.end MOS capacitance
