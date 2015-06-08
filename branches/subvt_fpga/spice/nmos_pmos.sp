* NMOS and PMOS package 
*

* CMOS TSMC 40nm 0.9V
.subckt nmos drain gate source bulk L=nl W=wn
X1 drain gate source bulk nch_25od33_mac l=L w=W
.eom nmos

.subckt pmos drain gate source bulk L=pl W=wp
X1 drain gate source bulk pch_25od33_mac l=L w=W 
.eom pmos

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

* ST 28nm Models
*.subckt nmos drain gate source bulk L=nl W=wn
*X1 drain gate source bulk nfet L=L W=W
*.eom nmos

*.subckt pmos drain gate source bulk L=pl W=wp
*X1 drain gate source bulk pfet L=L W=W
*.eom pmos

