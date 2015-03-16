Auto Spice
.include '/home/xitang/tangxifan-eda-tools/branches/fpga_flow/tech/22nm_HP.pm'
.param tech = 22e-9
.param tempr = 25
.param simt = 5n
.param Vol=0.6
.param pnratio=2
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/nmos_pmos.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux2.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux2trans.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux3.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux4.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux5.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/inv.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/dff.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/level_restorer.sp
.param rise = 'simt/1000'

Vdd Vdd 0 Vol
Vd Vd 0 Vol
Vdref Vdref 0 Vol
Vclk Vclk 0 Vol
Vclkref Vclkref 0 Vol
Vclkn Vclkn 0 Vol
Vclknref Vclknref 0 Vol

Vdn Dn 0 PWL(0 Vol 'simt/8' Vol 'simt/8+rise' 0  '3*simt/8' 0 '3*simt/8+rise' Vol '5*simt/8' Vol '5*simt/8+rise' 0 '7*simt/8' 0 '7*simt/8+rise' Vol '11*simt/8' Vol '11*simt/8+rise' 0 '15*simt/8' 0 '15*simt/8+rise' Vol)
Vclki clki 0 PULSE(0 Vol 'simt/4' 0n 0n 'simt/2' 'simt')
Vclkni clkni 0 PULSE(Vol 0 'simt/4' 0n 0n 'simt/2' 'simt')

X0 D Q clk clkn Vdd Gnd dff pnratio='pnratio'

* D driver and reference driver
X1 Dn D Vd 0 inv nsize=1 psize='pnratio'
X1r Dn x1 Vdref 0 inv nsize=1 psize='pnratio' 

X2 clkni clk Vclk 0 inv nsize=1 psize='pnratio'
X2r clkni x2 Vclkref 0 inv nsize=1 psize='pnratio'

X3 clki clkn Vclkn 0 inv nsize=1 psize='pnratio'
X3r clki x3 Vclknref 0 inv nsize=1 psize='pnratio'

.TEMP tempr

.OP
.OPTIONS POST

.IC V(Q)=0

.tran 'simt/1000' 'simt*2'

.measure tran Ebuffers INTEG I(Vdd)
.measure tran Ed INTEG I(Vd)
.measure tran Edref INTEG I(Vdref)
.measure tran Eclk INTEG I(Vclk)
.measure tran Eclkref INTEG I(Vclkref)
.measure tran Eclkn INTEG I(Vclkn)
.measure tran Eclknref INTEG I(Vclknref)
.measure tran E Param=('(-Ebuffers-Ed+Edref-Eclk+Eclkref-Eclkn+Eclknref)*Vol')
.measure tran power Param=('E/simt/2')
.end


