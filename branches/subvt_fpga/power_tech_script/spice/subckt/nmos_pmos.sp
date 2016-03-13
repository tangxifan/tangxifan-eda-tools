* NMOS/PMOS

* Diffusion sizes:
* Wdiff = size * 2.5 * tech
* Ldiff = 2.25 * tech

.subckt nfet drain gate source body size=1 
* PTM model
*M1 drain gate source body nmos L='tech' W='2.5*size*tech' AS='size*2.5*tech*2.25*tech' AD='size*2.5*tech*2.25*tech' PS='2*2.25*tech+size*2.5*tech'  PD='2*2.25*tech+size*2.5*tech'	
* UMC 180nm model
*M1 drain gate source body N_LV_18_MM L='tech' W='1.3*size*tech' AS='size*1.3*tech*1.3*tech' AD='size*1.3*tech*1.3*tech' PS='2*1.3*tech+size*1.3*tech'  PD='2*1.3*tech+size*1.3*tech'	
* TSMC 40nm 
M1 drain gate source body nch L='tech' W='280e-9' m='size'
* Silicon NanoWire FET
*X1 source vdd gate drain DGNW
.ends

.subckt pfet drain gate source body size=1 
* PTM model
*M1 drain gate source body pmos L='tech' W='2.5*size*tech' AS='size*2.5*tech*2.25*tech' AD='size*2.5*tech*2.25*tech' PS='2*2.25*tech+size*2.5*tech'  PD='2*2.25*tech+size*2.5*tech'		
* UMC 180nm model
*M1 drain gate source body P_LV_18_MM L='tech' W='1.3*size*tech' AS='size*1.3*tech*1.3*tech' AD='size*1.3*tech*1.3*tech' PS='2*1.3*tech+size*1.3*tech'  PD='2*1.3*tech+size*1.3*tech'		
* TSMC 40nm 
M1 drain gate source body pch L='tech' W='280e-9' m='size'
* Silicon NanoWire FET
*X1 source gnd gate drain DGNW
.ends

.subckt pfetz drain gate source body wsize=1 lsize=1
* PTM model
*M1 drain gate source body pmos L='lsize*tech' W='2.5*wsize*tech' AS='wsize*2.5*tech*2.25*lsize*tech' AD='wsize*2.5*tech*2.25*lsize*tech' PS='2*2.25*lsize*tech+wsize*2.5*tech'  PD='2*2.25*lsize*tech+wsize*2.5*tech'	
* UMC 180nm model
*M1 drain gate source body P_LV_18_MM L='lsize*tech' W='1.3*wsize*tech' AS='wsize*1.3*tech*1.3*lsize*tech' AD='wsize*1.3*tech*1.3*lsize*tech' PS='2*1.3*lsize*tech+wsize*1.3*tech'  PD='2*1.3*lsize*tech+wsize*1.3*tech'	
* TSMC 40nm 
M1 drain gate source body pch_25 L='270e-9' W='320e-9' m='lsize'
* Silicon NanoWire FET
*X1 drain gnd gate source DGNW
.ends

.subckt nfetz drain gate source body wsize=1 lsize=1
* PTM model
*M1 drain gate source body nmos L='lsize*tech' W='2.5*wsize*tech' AS='wsize*2.5*tech*2.25*lsize*tech' AD='wsize*2.5*tech*2.25*lsize*tech' PS='2*2.25*lsize*tech+wsize*2.5*tech'  PD='2*2.25*lsize*tech+wsize*2.5*tech'	
* UMC 180nm model
*M1 drain gate source body N_LV_18_MM L='lsize*tech' W='1.3*wsize*tech' AS='wsize*1.3*tech*1.3*lsize*tech' AD='wsize*1.3*tech*1.3*lsize*tech' PS='2*1.3*lsize*tech+wsize*1.3*tech'  PD='2*1.3*lsize*tech+wsize*1.3*tech'	
* TSMC 40nm 
M1 drain gate source body nch_25 L='270e-9' W='320e-9' m='lsize'
* Silicon NanoWire FET
*X1 drain gnd gate source DGNW
.ends

