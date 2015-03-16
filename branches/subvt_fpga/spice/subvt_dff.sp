* Design Parameters
*.param beta = 2
*.param nl = 0.18e-6
*.param pl = 0.18e-6
*.param wn = '1*nl'
*.param wp = 'beta*wn'
*.include './spice/PTM45nm_tech.sp'
.include './spice/inv_buf.sp'
*******************************
* Sub Circuits
*
* Static D Flip-flop
.subckt static_dff D clk Q set rst
* Input inverter
Xinv_clk clk clk_b vdd gnd inv
Xinv_set set set_b vdd gnd inv
Xinv_rst rst rst_b vdd gnd inv
Xinv0 D s1_n1 vdd gnd inv 
Xcpt0 s1_n1 s1_n2 clk_b clk vdd gnd cpt
Xset0 s1_n2 set_b vdd vdd pmos L=pl W='wp' 
Xrst0 s1_n2 rst gnd gnd nmos L=nl W='wn' 
Xinv1 s1_n2 s1_q vdd gnd inv
Xinv2 s1_q s1_n3 vdd gnd inv 
Xcpt1 s1_3 s1_n2 clk clk_b vdd gnd cpt 
* Stage 2
Xinv3 s1_q s2_n1 vdd gnd inv 
Xcpt2 s2_n1 s2_n2 clk clk_b vdd gnd cpt
Xset1 s2_n2 rst_b vdd vdd pmos L=pl W='wp' 
Xrst1 s2_n2 set gnd gnd nmos L=nl W='wn' 
Xinv4 s2_n2 Q vdd gnd inv
Xinv5 Q s2_n3 vdd gnd inv 
Xcpt3 s2_n3 s2_n2 clk_b clk vdd gnd cpt 
.eom static_dff
