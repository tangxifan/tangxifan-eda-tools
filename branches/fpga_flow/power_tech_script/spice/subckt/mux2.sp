* 2 input mux

*.subckt nfet drain gate source size=1 

.subckt mux2 in0 in1 out sel0 sel1 size=1 
X0 in0 sel0 out 0 nfet size='size'
X1 in1 sel1 out 0 nfet size='size'

* Xifan TANG: make it transmission-gate
*X0 in0 sel0 out 0 nfet size='size'
*X1 out sel1 in0 Vdd pfet size='size*pnratio'

*X2 in1 sel1 out 0 nfet size='size'
*X3 out sel0 in1 Vdd pfet size='size*pnratio'

.ends
