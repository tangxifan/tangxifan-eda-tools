***************************************************
*
*Sample HSPICE Deck
*
***************************************************


.TITLE 'IDS vs VGS for CMOS'

*.lib 'PTM_MG/models' ptm20lstp
.include '../process/ptm_finfet7nm/7nm_TT_011216.pm'
*.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/MM180_LVT18_V113.lib' TT
*.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/st_28nm.sp'
*.include './process/32nmHP.pm'
*.include './process/tsmc40nm.sp'
*.include './process/22nm_HP.pm'
*.include './process/45nm_LP.pm'
*.include './process/45nm_HP.pm'
* TSMC 40nm 2.5V nch
*.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/tsmc40nm/toplevel_crn45gs_2d5_v1d1_shrink0d9_embedded_usage.l' TOP_TT
* TSMC 40nm 0.9V nch
*.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/tsmc40nm/toplevel_crn45gs_1d2_1d8_ud15_lk_v1d1_shrink0d9_embedded_usage.l' TOP_TT
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/spice/nmos_pmos.sp'
.temp 25
.param beta = 1
.param alpha = 1

* PTM 7nm FinFET
.param nl = 7e-9
.param pl = 7e-9
.param wn = '21e-9'
.param wp = 'beta*21e-9'

* TSMC 40nm Tech. Logic trans.
*.param nl = 40e-9
*.param pl = 40e-9
*.param wn = '140e-9'
*.param wp = 'beta*140e-9'

* TSMC 40nm Tech. I/O trans.
*.param nl = 270e-9
*.param pl = 270e-9
*.param wn = '320e-9'
*.param wp = 'beta*320e-9'

.option POST 
.option captab

*Beginning of circuit and device definitions
***************************************************
*Supplies and voltage params:
.param Supply=0.7
.param Vg='Supply'
.param Vd='Supply'
.param Wprog='wn'

***********************************************************************
* Define power supply
***********************************************************************
Vndd    nDrain     Gnd     Vd
Vnss    nSource    Gnd     0
Vngg 	nGate	  Gnd	  0

Vpdd    pDrain     Gnd     0
Vpss    pSource    Gnd     Vd
Vpgg 	pGate	  Gnd	  0

Vnsub    nSub       Gnd     0
Vpsub    pSub       Gnd     Vd
***********************************************************************
* Main Circuits
***********************************************************************
Xnmos nDrain nGate nSource nSub nmos L=nl W=wn
*Xpmos pSource pGate pDrain pSub pmos L=pl W=wp
Xpmos pDrain pGate pSource pSub pmos L=pl W=wp
* test nFETs, Ids vs. Vgs
*.DC       Wprog   START='wn'    STOP='3*wn'   STEP='0.1*wn'
.DC      Vndd   START=0     STOP='Supply'   STEP='0.01*Supply' 
+ SWEEP  Vngg   START=0     STOP='Supply'   STEP='0.1*Supply'
.DC      Vpdd   START=0     STOP='Supply'   STEP='0.01*Supply' 
+ SWEEP  Vpgg   START=0    STOP='Supply'   STEP='0.1*Supply'
***********************************************************************
.print p(Vdd)
.print I(Vdd)
.end 
