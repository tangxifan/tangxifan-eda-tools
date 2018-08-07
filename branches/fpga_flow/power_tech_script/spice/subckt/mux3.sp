* 3 input mux

*.subckt nfet drain gate source size=1 

.subckt mux3 in0 in1 in2 out sel0 sel1 sel2 sel0_inv sel1_inv sel2_inv size=1 
X0_n in0 sel0 out 0 nfet size='size'
X1_n in1 sel1 out 0 nfet size='size'
X2_n in2 sel2 out 0 nfet size='size'

X0_p in0 sel0_inv out Vdd pfet size='size*pnratio'
X1_p in1 sel1_inv out Vdd pfet size='size*pnratio'
X2_p in2 sel2_inv out Vdd pfet size='size*pnratio'

.ends
