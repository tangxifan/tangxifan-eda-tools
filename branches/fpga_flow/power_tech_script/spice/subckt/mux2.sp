* 2 input mux

*.subckt nfet drain gate source size=1 

.subckt mux2 in0 in1 out sel0 sel1 size=1 
Xinv0 in0 in0_inv vdd gnd inv nsize='size' psize='2*size'
Xinv1 in1 in1_inv vdd gnd inv nsize='size' psize='2*size'
X0 in0_inv sel0 out_inv 0 nfet size='size'
X1 in1_inv sel1 out_inv 0 nfet size='size'
Xinvo out_inv out vdd gnd inv nsize='size' psize='2*size'
	
*X0 in0 sel0 out 0 nfet size='size'
*X1 in1 sel1 out 0 nfet size='size'

.ends
