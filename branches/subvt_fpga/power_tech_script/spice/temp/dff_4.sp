Auto Spice
.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/MM180_LVT18_V113.lib' TT
.param tech = 180e-9
.param tempr = 25
.param simt = 5n
.param Vol=0.6
.param pnratio=3
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/nmos_pmos.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux2.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux2trans.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux3.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux4.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/mux5.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/inv.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/dff.sp
.include /home/xitang/tangxifan-eda-tools/branches/subvt_fpga/power_tech_script/spice/subckt/level_restorer.sp
