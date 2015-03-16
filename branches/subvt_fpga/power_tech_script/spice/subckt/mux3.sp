* 3 input mux

*.subckt nfet drain gate source size=1 

.subckt mux3 in0 in1 in2 out sel0 sel1 sel2 size=1 

*X0 in0 sel0 out 0 nfet size='size'
*X1 in1 sel1 out 0 nfet size='size'
*X2 in2 sel2 out 0 nfet size='size'
Xinv0 in0 in0_inv vdd gnd inv nsize='size' psize='2*size'
Xinv1 in1 in1_inv vdd gnd inv nsize='size' psize='2*size'
Xinv2 in2 in2_inv vdd gnd inv nsize='size' psize='2*size'
*R0 in0_inv out_inv 5.106k
R0 in0_inv out_inv 3k
R1 in1_inv out_inv 1000k
R2 in2_inv out_inv 1000k
Xpi in0_inv 0 0 0 nfet size='2'
Xp0 in0_inv 0 in1_inv 0 nfet size='2'
Xp1 in1_inv 0 in2_inv 0 nfet size='2'
Xp2 in3_inv 0 0 0 nfet size='2'
Xpo out_inv 0 0 0 nfet size='2'

Xinvo out_inv out vdd gnd inv nsize='size' psize='2*size'
	
.ends
