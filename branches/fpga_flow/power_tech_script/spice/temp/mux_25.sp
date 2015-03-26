Auto Spice
.include '/home/xitang/tangxifan-eda-tools/branches/fpga_flow/tech/45nm_HP.pm'
.param tech = 45e-9
.param tempr = 25
.param simt = 5n
.param Vol=1.0
.param pnratio=2
.param nmos_size=22.74
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/nmos_pmos.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux2.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux2trans.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux3.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux4.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/mux5.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/inv.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/dff.sp
.include /home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/level_restorer.sp
.param rise = 'simt/500'

Vin in 0 PULSE(Vol 0 'simt/4' 'rise' 'rise' 'simt/2-rise' 'simt')
Vdrive Vdrive 0 Vol
Vref Vref 0 Vol
Vsram sram1 0 Vol
Vbuf Vbuf 0 Vol

X0 in a Vdrive 0 inv nsize=10 psize='10*pnratio'
Xref in x Vref 0 inv nsize=10 psize='10*pnratio'
X1 a a a a a mida sram1 0 0 0 0 mux5 size='nmos_size'
X2 a a a a a midb sram1 0 0 0 0 mux5 size='nmos_size'
X3 a a a a a midc sram1 0 0 0 0 mux5 size='nmos_size'
X4 a a a a a midd sram1 0 0 0 0 mux5 size='nmos_size'
X5 a a a a a mide sram1 0 0 0 0 mux5 size='nmos_size'
X6 mida midb midc midd mide out sram1 0 0 0 0 mux5 size='nmos_size'


.TEMP tempr

.OP
.OPTIONS POST



.tran 'simt/10000' 'simt'

.measure tran Edrive integ I(Vdrive)
.measure tran Eref integ I(Vref)
.measure tran Esram integ I(Vsram)
.measure tran Ebuf integ I(Vbuf)
.measure tran E Param=('(-Edrive+Eref)*Vol')
.measure tran power Param=('E/simt')


.end

