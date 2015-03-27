Automated spice simuation: Fri Mar 27 14:35:13 2015
.global Vdd gnd
.include '/home/xitang/tangxifan-eda-tools/branches/fpga_flow/tech/45nm_HP.pm'
.param tech = 45e-9
.param Vol = 0.8
.param simt = 5n
.param rise = 'simt/500'
.param fall = 'simt/500'
.include '/home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/nmos_pmos.sp'
.include '/home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/inv.sp'
.include '/home/xitang/tangxifan-eda-tools/branches/fpga_flow/power_tech_script/spice/subckt/level_restorer.sp'
Vdd Vdd 0 'Vol'
Vin in 0 0.4
X0a in Vdd outa 0 nfet size='1.00'
X1a in 0 outa 0 nfet size='1.00'
X2a in 0 outa 0 nfet size='1.00'
X3a in 0 outa 0 nfet size='1.00'
X4a in 0 outa 0 nfet size='1.00'
X5a in 0 outa 0 nfet size='1.00'
X6a in 0 outa 0 nfet size='1.00'
X7a in 0 outa 0 nfet size='1.00'
X8a in 0 outa 0 nfet size='1.00'
X9a in 0 outa 0 nfet size='1.00'
X10a in 0 outa 0 nfet size='1.00'
X11a in 0 outa 0 nfet size='1.00'
X12a in 0 outa 0 nfet size='1.00'
X0b in Vdd outb 0 nfet size='1.00'
X1b 0 0 outb 0 nfet size='1.00'
X2b 0 0 outb 0 nfet size='1.00'
X3b 0 0 outb 0 nfet size='1.00'
X4b 0 0 outb 0 nfet size='1.00'
X5b 0 0 outb 0 nfet size='1.00'
X6b 0 0 outb 0 nfet size='1.00'
X7b 0 0 outb 0 nfet size='1.00'
X8b 0 0 outb 0 nfet size='1.00'
X9b 0 0 outb 0 nfet size='1.00'
X10b 0 0 outb 0 nfet size='1.00'
X11b 0 0 outb 0 nfet size='1.00'
X12b 0 0 outb 0 nfet size='1.00'
.TEMP 25
.OP
.OPTIONS LIST NODE POST CAPTAB
.tran 'simt/10' simt
.measure tran vout_min avg V(outb)
.measure tran vout_max avg V(outa)
.end

