***************************************************
*
*Sample HSPICE Deck
*
***************************************************


.TITLE 'IDS vs VGS for CMOS'

*.lib 'PTM_MG/models' ptm20lstp
*.lib '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/process/MM180_LVT18_V113.lib' TT
.include './process/22nm_HP.pm'
*.include './process/32nmHP.pm'
*.include './process/tsmc40nm.sp'
.include '/home/xitang/tangxifan-eda-tools/branches/subvt_fpga/spice/nmos_pmos.sp'
.temp 25
.param beta = 1.45
.param alpha = 3
.param nl = 22e-9
.param pl = 22e-9
.param wn = '66e-9'
.param wp = 'wn*beta'

.option POST 
.option captab

*Beginning of circuit and device definitions
***************************************************
*Supplies and voltage params:
.param Supply=1.2	
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
*.DC      Vpgg   START=0     STOP='Supply'   STEP='0.01*Supply' 
*+ SWEEP  Vpdd   START=0     STOP='Supply'   STEP='0.1*Supply'
.DC      Vndd   START=0     STOP='Supply'   STEP='0.01*Supply' 
+ SWEEP  Vngg   START=0     STOP='Supply'   STEP='0.1*Supply'
.DC      Vpdd   START=0     STOP='Supply'   STEP='0.01*Supply' 
+ SWEEP  Vpgg   START=0    STOP='Supply'   STEP='0.1*Supply'
***********************************************************************
.print p(Vdd)
.print I(Vdd)
.end 
