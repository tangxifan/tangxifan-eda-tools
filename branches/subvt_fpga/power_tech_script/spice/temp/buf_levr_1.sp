Auto Spice
.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/tech/MM180_LVT18_V113.lib' TT
.param tech = 180e-9
.param tempr = 25
.param simt = 5n
.param Vol=1.8
.param pnratio=2
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/nmos_pmos.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux2.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux2trans.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux3.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux4.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux5.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/inv.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/dff.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/level_restorer.sp
.param rise = 'simt/500'


Vdut Vdut 0 Vol
Vdrive Vdrive 0 Vol
Vref Vref 0 Vol

Vin in 0 PULSE(0 Vol 'simt/4' 'rise' 'rise' 'simt/2' 'simt')

X0 in mid1 Vdrive 0 inv nsize='1' psize='1*pnratio'
X1 in x Vref 0 inv nsize='1' psize='1*pnratio'

X2 mid1 mid2 Vdut 0 levr



.TEMP tempr

.OP
.OPTIONS POST

.tran 'simt/10000' 'simt'

.measure tran Edut INTEG I(Vdut)
.measure tran Edrive INTEG I(Vdrive)
.measure tran Eref INTEG I(Vref)
.measure tran E Param=('(-Edut-Edrive+Eref)*Vol')
.measure tran power Param=('E/simt')

.end
