*.include './UMC180nm_tech.sp'
*.include './PTM45nm_tech.sp'
.include './spice/inv_buf.sp'
* LUT input buffers
.subckt lut_in_buf in in_buf inbar_buf
Xbuf0 in in_buf vdd gnd buf size=2
Xinv0 in in_inv vdd gnd inv size=1
Xbuf1 in_inv inbar_buf vdd gnd buf size=2
.eom
* 1-LUT
.subckt lut1 sram0 sram1 in out
Xin_buf0 in in_buf inbar_buf lut_in_buf
Xmux0 sram0 sram1 in_buf inbar_buf out vdd gnd mux2to1 size=1
.eom
* 4:1 MUX 
.subckt mux4to1 in0 in1 in2 in3 sram0 sram0_inv sram1 sram1_inv out size=1
Xmux0_0 in0 in1 sram0 sram0_inv out0_0 vdd gnd mux2to1 size=size
Xmux0_1 in2 in3 sram0 sram0_inv out0_1 vdd gnd mux2to1 size=size
Xmux1_0 out0_0 out0_1 sram1 sram1_inv out vdd gnd mux2to1 size=size
.eom
* 8:1 MUX 
.subckt mux8to1 in0 in1 in2 in3 in4 in5 in6 in7 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv out size=1
Xmux0_0 in0 in1 in2 in3 sram0 sram0_inv sram1 sram1_inv out0_0 mux4to1 size=size
Xmux0_1 in4 in5 in6 in7 sram0 sram0_inv sram1 sram1_inv out0_1 mux4to1 size=size
Xmux1_0 out0_0 out0_1 sram2 sram2_inv out vdd gnd mux2to1 size=size
.eom
* 16:1 MUX 
.subckt mux16to1 in0 in1 in2 in3 in4 in5 in6 in7 in8 in9 in10 in11 in12 in13 in14 in15 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv sram3 sram3_inv out size=1
Xmux0_0 in0 in1 in2 in3 in4 in5 in6 in7 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv out0_0 mux8to1 size=size
Xmux0_1 in8 in9 in10 in11 in12 in13 in14 in15 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv out0_1 mux8to1 size=size
Xmux1_0 out0_0 out0_1 sram3 sram3_inv out vdd gnd mux2to1 size=size
.eom
* 32:1 MUX
.subckt mux32to1 in0 in1 in2 in3 in4 in5 in6 in7 in8 in9 in10 in11 in12 in13 in14 in15 in16 in17 in18 in19 in20 in21 in22 in23 in24 in25 in26 in27 in28 in29 in30 in31 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv sram3 sram3_inv sram4 sram4_inv out size=1
Xmux0_0 in0 in1 in2 in3 in4 in5 in6 in7 in8 in9 in10 in11 in12 in13 in14 in15 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv sram3 sram3_inv out0_0 mux16to1 size=size
Xmux0_1 in16 in17 in18 in19 in20 in21 in22 in23 in24 in25 in26 in27 in28 in29 in30 in31 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv sram3 sram3_inv out0_1 mux16to1 size=size
Xmux1_0 out0_0 out0_1 sram4 sram4_inv out vdd gnd mux2to1 size=size
.eom
* 64to1 MUX
.subckt mux64to1 in0 in1 in2 in3 in4 in5 in6 in7 in8 in9 in10 in11 in12 in13 in14 in15 in16 in17 in18 in19 in20 in21 in22 in23 in24 in25 in26 in27 in28 in29 in30 in31 in32 in33 in34 in35 in36 in37 in38 in39 in40 in41 in42 in43 in44 in45 in46 in47 in48 in49 in50 in51 in52 in53 in54 in55 in56 in57 in58 in59 in60 in61 in62 in63 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv sram3 sram3_inv sram4 sram4_inv sram5 sram5_inv out size=1
Xmux0_0 in0 in1 in2 in3 in4 in5 in6 in7 in8 in9 in10 in11 in12 in13 in14 in15 in16 in17 in18 in19 in20 in21 in22 in23 in24 in25 in26 in27 in28 in29 in30 in31 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv sram3 sram3_inv sram4 sram4_inv out0_0 mux32to1 size=size
Xmux0_1 in32 in33 in34 in35 in36 in37 in38 in39 in40 in41 in42 in43 in44 in45 in46 in47 in48 in49 in50 in51 in52 in53 in54 in55 in56 in57 in58 in59 in60 in61 in62 in63 sram0 sram0_inv sram1 sram1_inv sram2 sram2_inv sram3 sram3_inv sram4 sram4_inv out0_1 mux32to1 size=size
Xmux1_0 out0_0 out0_1 sram5 sram5_inv out vdd gnd mux2to1 size=size
.eom
* 2-LUT
.subckt lut2 sram0 sram1 sram2 sram3 in0 in1 out size=1
Xin_buf0 in0 in_buf0 inbar_buf0 lut_in_buf
Xin_buf1 in1 in_buf1 inbar_buf1 lut_in_buf
Xmux4to1 sram0 sram1 sram2 sram3 in_buf0 inbar_buf0 in_buf1 inbar_buf1 out mux4to1 size=1
.eom
* Simple 3-LUT
.subckt lut3 sram0 sram1 sram2 sram3 sram4 sram5 sram6 sram7 in0 in1 in2 out size=1
Xin_buf0 in0 in_buf0 inbar_buf0 lut_in_buf
Xin_buf1 in1 in_buf1 inbar_buf1 lut_in_buf
Xin_buf2 in2 in_buf2 inbar_buf2 lut_in_buf
Xmux8to1 sram0 sram1 sram2 sram3 sram4 sram5 sram6 sram7 in_buf0 inbar_buf0 in_buf1 inbar_buf1 in_buf2 inbar_buf2 out mux8to1 size=1
.eom
* Simple 4-LUT
.subckt lut4 sram0 sram1 sram2 sram3 sram4 sram5 sram6 sram7 sram8 sram9 sram10 sram11 sram12 sram13 sram14 sram15 in0 in1 in2 in3 out size=1
Xin_buf0 in0 in_buf0 inbar_buf0 lut_in_buf
Xin_buf1 in1 in_buf1 inbar_buf1 lut_in_buf
Xin_buf2 in2 in_buf2 inbar_buf2 lut_in_buf
Xin_buf3 in3 in_buf3 inbar_buf3 lut_in_buf
Xmux16to1 sram0 sram1 sram2 sram3 sram4 sram5 sram6 sram7 sram8 sram9 sram10 sram11 sram12 sram13 sram14 sram15 in_buf0 inbar_buf0 in_buf1 inbar_buf1 in_buf2 inbar_buf2 in_buf3 inbar_buf3 out mux16to1 size=1
.eom
* LUT 5
.subckt lut5 sram0 sram1 sram2 sram3 sram4 sram5 sram6 sram7 sram8 sram9 sram10 sram11 sram12 sram13 sram14 sram15 sram16 sram17 sram18 sram19 sram20 sram21 sram22 sram23 sram24 sram25 sram26 sram27 sram28 sram29 sram30 sram31 in0 in1 in2 in3 in4 out size=1
Xin_buf0 in0 in_buf0 inbar_buf0 lut_in_buf
Xin_buf1 in1 in_buf1 inbar_buf1 lut_in_buf
Xin_buf2 in2 in_buf2 inbar_buf2 lut_in_buf
Xin_buf3 in3 in_buf3 inbar_buf3 lut_in_buf
Xin_buf4 in4 in_buf4 inbar_buf4 lut_in_buf
Xmux32to1 sram0 sram1 sram2 sram3 sram4 sram5 sram6 sram7 sram8 sram9 sram10 sram11 sram12 sram13 sram14 sram15 sram16 sram17 sram18 sram19 sram20 sram21 sram22 sram23 sram24 sram25 sram26 sram27 sram28 sram29 sram30 sram31  in_buf0 inbar_buf0 in_buf1 inbar_buf1 in_buf2 inbar_buf2 in_buf3 inbar_buf3 in_buf4 inbar_buf4 out_lvl0 mux32to1 size=1
Xinv_out1 out_lvl0 out_lvl1 vdd gnd inv size=1
Xinv_out2 out_lvl1 out_lvl2 vdd gnd inv size=1
Rout out_lvl2 out 0
.eom

* LUT 6
.subckt lut6 sram0 sram1 sram2 sram3 sram4 sram5 sram6 sram7 sram8 sram9 sram10 sram11 sram12 sram13 sram14 sram15 sram16 sram17 sram18 sram19 sram20 sram21 sram22 sram23 sram24 sram25 sram26 sram27 sram28 sram29 sram30 sram31 sram32 sram33 sram34 sram35 sram36 sram37 sram38 sram39 sram40 sram41 sram42 sram43 sram44 sram45 sram46 sram47 sram48 sram49 sram50 sram51 sram52 sram53 sram54 sram55 sram56 sram57 sram58 sram59 sram60 sram61 sram62 sram63 in0 in1 in2 in3 in4 in5 out size=1
Xin_buf0 in0 in_buf0 inbar_buf0 lut_in_buf
Xin_buf1 in1 in_buf1 inbar_buf1 lut_in_buf
Xin_buf2 in2 in_buf2 inbar_buf2 lut_in_buf
Xin_buf3 in3 in_buf3 inbar_buf3 lut_in_buf
Xin_buf4 in4 in_buf4 inbar_buf4 lut_in_buf
Xin_buf5 in5 in_buf5 inbar_buf5 lut_in_buf
Xmux64to1 sram0 sram1 sram2 sram3 sram4 sram5 sram6 sram7 sram8 sram9 sram10 sram11 sram12 sram13 sram14 sram15 sram16 sram17 sram18 sram19 sram20 sram21 sram22 sram23 sram24 sram25 sram26 sram27 sram28 sram29 sram30 sram31 sram32 sram33 sram34 sram35 sram36 sram37 sram38 sram39 sram40 sram41 sram42 sram43 sram44 sram45 sram46 sram47 sram48 sram49 sram50 sram51 sram52 sram53 sram54 sram55 sram56 sram57 sram58 sram59 sram60 sram61 sram62 sram63 in_buf0 inbar_buf0 in_buf1 inbar_buf1 in_buf2 inbar_buf2 in_buf3 inbar_buf3 in_buf4 inbar_buf4 in_buf5 inbar_buf5 out_lvl0 mux64to1 size=1
Xinv_out1 out_lvl0 out_lvl1 vdd gnd inv size=1
Xinv_out2 out_lvl1 out_lvl2 vdd gnd inv size=1
Rout out_lvl2 out 0
.eom
