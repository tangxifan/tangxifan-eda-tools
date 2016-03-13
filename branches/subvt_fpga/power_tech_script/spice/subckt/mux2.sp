* 2 input mux

*.subckt nfet drain gate source size=1 

.subckt mux2 in0 in1 out sel0 sel1 size=1 

X0 in0 sel0 out 0 nfet size='size'
X1 in1 sel1 out 0 nfet size='size'
*Xinv0 in0 in0_inv vdd gnd inv nsize='size' psize='2*size'
*Xinv1 in1 in1_inv vdd gnd inv nsize='size' psize='2*size'
*R0 in0 out_inv 5.106k
*R0 in0 out_inv 3k
*R1 in1 out_inv 1000k
*Xpi in0_inv 0 0 0 nfet size='2'
*Xp0 in0_inv 0 in1_inv 0 nfet size='2'
*Xp1 in1_inv 0 0 0 nfet size='2'
*Xp2 out_inv 0 0 0 nfet size='2'

*Xinvo out_inv out vdd gnd inv nsize='size' psize='2*size'

*R0 in0 out 1k
*R1 in1 out 1000k
	
.ends
