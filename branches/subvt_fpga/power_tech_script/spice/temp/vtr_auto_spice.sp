Automated spice simuation: Wed Mar  9 15:52:21 2016
.global Vdd gnd
.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/tsmc40nm/toplevel_crn45gs_2d5_v1d1_shrink0d9_embedded_usage.l' TOP_TT
.param tech = 40e-9
.param Vol = 0.9
.param simt = 5n
.param rise = 'simt/500'
.param fall = 'simt/500'
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/nmos_pmos.sp'
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/inv.sp'
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/level_restorer.sp'
Vdd Vdd 0 'Vol'
Vin in 0 0.9
X0 in 0 0 0 nfet size='22.74'
.TEMP 25
.OP
.OPTIONS LIST NODE POST CAPTAB
.tran 'simt/10' simt
.measure tran leakage avg I(Vin)
.end

