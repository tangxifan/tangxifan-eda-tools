Automated spice simuation: Tue Nov 18 17:56:03 2014
.global Vdd gnd
.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/tech/MM180_LVT18_V113.lib' TT
.param tech = 180e-9
.param Vol = 1.8
.param simt = 5n
.param rise = 'simt/500'
.param fall = 'simt/500'
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/nmos_pmos.sp'
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/inv.sp'
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/level_restorer.sp'
Vdd Vdd 0 'Vol'
Vin in 0 1.8
X0 in 0 0 0 nfet size='22.74'
.TEMP 25
.OP
.OPTIONS LIST NODE POST CAPTAB
.tran 'simt/10' simt
.measure tran leakage avg I(Vin)
.end

