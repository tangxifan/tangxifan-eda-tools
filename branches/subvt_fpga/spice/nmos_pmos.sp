* NMOS and PMOS package 
*

* CMOS TSMC 40nm 0.9V
*.subckt nmos drain gate source bulk L=nl W=wn
*M1 drain gate source bulk nch l=L w=W
*.eom nmos

*.subckt pmos drain gate source bulk L=pl W=wp
*M1 drain gate source bulk pch l=L w=W 
*.eom pmos

* CMOS TSMC 40nm I/O transistor 2.5V
*.subckt nmos drain gate source bulk L=nl W=wn
*M1 drain gate source bulk nch_25 l=L w=W
*.eom nmos
*
*.subckt pmos drain gate source bulk L=pl W=wp
*M1 drain gate source bulk pch_25 l=L w=W 
*.eom pmos

* CMOS UMC 0.18um 3.3V
*.subckt nmos drain gate source bulk L=nl W=wn
*M1 drain gate source bulk N_LV_33_MM L=L W=W
*.eom nmos

*.subckt pmos drain gate source bulk L=pl W=wp
*M1 drain gate source bulk P_LV_33_MM L=L W=W
*.eom pmos

* CMOS UMC 0.18um 1.8V
*.subckt nmos drain gate source bulk L=nl W=wn
*M1 drain gate source bulk N_LV_18_MM L=L W=W
*.eom nmos

*.subckt pmos drain gate source bulk L=pl W=wp
*M1 drain gate source bulk P_LV_18_MM L=L W=W
*.eom pmos

* CMOS PTM Models
*.subckt nmos drain gate source bulk L=nl W=wn
*M1 drain gate source bulk nmos L=L W=W
*.eom nmos

*.subckt pmos drain gate source bulk L=L W=wp
*M1 drain gate source bulk pmos L=L W=W
*.eom pmos

* PTM 7nm FinFET models
*.subckt nmos drain gate source bulk L=nl W=wn
*M1 drain gate source bulk nmos_lvt L=L W=W
*.eom nmos

*.subckt pmos drain gate source bulk L=L W=wp
*M1 drain gate source bulk pmos_lvt L=L W=W
*.eom pmos

* ST 28nm Models
*.subckt nmos drain gate source bulk L=nl W=wn
*X1 drain gate source bulk nfet L=L W=W
*.eom nmos

*.subckt pmos drain gate source bulk L=pl W=wp
*X1 drain gate source bulk pfet L=L W=W
*.eom pmos

* Leti 130nm models
.param low_noise_option = 0
.subckt nmos drain gate source bulk L=nl W=wn
XM1 drain gate source bulk EN1V8_BS3JU l=L w=W
*XM1 drain gate source bulk EN5V0_BS3JU l=L w=W nfing=1 ncrsd=1 number=1
*+srcefirst=1 ngcon=1 mismatch=1 po2act=-1 dvt_mdev=0 dmu_mdev=0 soa=1
*+lpe=0
.eom nmos

.subckt pmos drain gate source bulk L=pl W=wp
XM1 drain gate source bulk EP1V8_BS3JU l=L w=W
*XM1 drain gate source bulk EP5V0_BS3JU l=L w=W nfing=1 ncrsd=1
*+number=1 srcefirst=1 ngcon=1 mismatch=1 po2act=-1 dvt_mdev=0 dmu_mdev=0
*+soa=1 lpe=0
.eom pmos
