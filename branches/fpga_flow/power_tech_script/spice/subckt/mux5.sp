* 4 input mux

*.subckt nfet drain gate source size=1 

.subckt mux5 in0 in1 in2 in3 in4 out sel0 sel1 sel2 sel3 sel4 size=1 

Xinv0 in0 in0_inv vdd gnd inv nsize='size' psize='2*size'
Xinv1 in1 in1_inv vdd gnd inv nsize='size' psize='2*size'
Xinv2 in2 in2_inv vdd gnd inv nsize='size' psize='2*size'
Xinv3 in3 in3_inv vdd gnd inv nsize='size' psize='2*size'
Xinv4 in4 in4_inv vdd gnd inv nsize='size' psize='2*size'
X0 in0_inv sel0 out_inv 0 nfet size='size'
X1 in1_inv sel1 out_inv 0 nfet size='size'
X2 in2_inv sel2 out_inv 0 nfet size='size'
X3 in3_inv sel3 out_inv 0 nfet size='size'
X4 in4_inv sel4 out_inv 0 nfet size='size'
Xinvo out_inv out vdd gnd inv nsize='size' psize='2*size'

*X0 in0 sel0 out 0 nfet size='size'
*X1 in1 sel1 out 0 nfet size='size'
*X2 in2 sel2 out 0 nfet size='size'
*X3 in3 sel3 out 0 nfet size='size'
*X4 in4 sel4 out 0 nfet size='size'
	
.ends
