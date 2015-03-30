clear all
close all

dff = [ % vsp,leakage,tsu(rise),tsu(fall),delay(rise),delay(fall),dynamic_power(rise),dynamic_power(fall)
0.4,7.64695e-08,4.921875e-10,6.2109375e-10,3.2900E-10,3.4784E-10,1.86701666666667e-06,1.28605e-06;
0.5,1.06779125e-07,3.28125e-10,3.8671875e-10,2.2212E-10,2.2780E-10,4.28234e-06,3.0743625e-06;
0.6,1.4385625e-07,2.51953125e-10,2.8125e-10,1.6812E-10,1.7098E-10,8.4129e-06,5.81466e-06;
0.7,1.92435e-07,2.05078125e-10,2.28515625e-10,1.3636E-10,1.3871E-10,1.389275e-05,1.06234857142857e-05;
0.8,2.3428875e-07,1.7578125e-10,1.875e-10,1.1613E-10,1.1857E-10,2.09313333333333e-05,1.56928e-05;
0.9,2.9584625e-07,1.5234375e-10,1.640625e-10,1.0197E-10,1.0484E-10,3.04685714285714e-05,2.4568e-05;
1,3.6611e-07,1.40625e-10,1.46484375e-10,9.2026E-11,9.4803E-11,3.848e-05,3.7189375e-05;
1.1,4.4684e-07,1.2890625e-10,1.34765625e-10,8.3806E-11,8.7843E-11,5.23905714285714e-05,4.72954285714286e-05;
1.2,5.40365e-07,1.23046875e-10,1.2890625e-10,7.7724E-11,8.1882E-11,6.868575e-05,5.99675714285714e-05;
1.3,6.477125e-07,1.171875e-10,1.171875e-10,7.2780E-11,7.7380E-11,8.10352857142857e-05,7.48678571428572e-05;
1.4,7.7989625e-07,1.11328125e-10,1.11328125e-10,6.8759E-11,7.3098E-11,0.000102720125,9.7588625e-05;
1.5,9.15195e-07,1.0546875e-10,1.11328125e-10,6.5638E-11,6.9982E-11,0.0001224925,0.0001166075;
1.6,1.1102e-06,1.0546875e-10,1.0546875e-10,6.2661E-11,6.7423E-11,0.00014372125,0.0001385825;
1.7,1.30742375e-06,9.9609375e-11,9.9609375e-11,6.0438E-11,6.5361E-11,0.000174601111111111,0.000170984444444444;
1.8,1.5963575e-06,9.9609375e-11,9.9609375e-11,5.8523E-11,6.4125E-11,0.000200724444444444,0.000197355555555556;
];

lut4 = [ % vsp,delay,leakage,dynamic_power,
0.4,1.17285e-09,1.01356e-07,5.056355e-06;
0.5,7.4288e-10,1.4135e-07,1.18304e-05;
0.6,5.4145e-10,1.8828e-07,2.22232e-05;
0.7,4.2812e-10,2.4274e-07,3.68486e-05;
0.8,3.57055e-10,3.05395e-07,5.56918e-05;
0.9,3.08585e-10,3.76935e-07,7.87566e-05;
1,2.7389e-10,4.5809e-07,0.0001062637;
1.1,2.4776e-10,5.4964e-07,0.00013882;
1.2,2.27545e-10,6.52395e-07,0.00017641069;
1.3,2.11585e-10,7.67215e-07,0.00022386205;
1.4,1.99025e-10,8.95005e-07,0.0002760755;
1.5,1.88795e-10,1.03672e-06,0.00033778;
1.6,1.80275e-10,1.1933e-06,0.000404646;
1.7,1.7294e-10,1.36595e-06,0.0004820535;
1.8,1.6668e-10,1.5556e-06,0.000566615;
];

lut6 = [ % vsp,delay,leakage,dynamic_power,
0.4,2.82805e-09,1.5205e-07,9.52005e-06,
0.5,1.7939e-09,2.12055e-07,2.24943e-05,
0.6,1.30515e-09,2.82445e-07,4.2833e-05,
0.7,1.03086e-09,3.64145e-07,7.1484e-05,
0.8,8.5866e-10,4.58145e-07,0.000109226,
0.9,7.41125e-10,5.6547e-07,0.00015595,
1,6.5715e-10,6.8722e-07,0.0002122435,
1.1,5.9415e-10,8.2456e-07,0.0002792905,
1.2,5.4615e-10,9.7871e-07,0.000356125,
1.3,5.07505e-10,1.15095e-06,0.00044231,
1.4,4.77325e-10,1.34265e-06,0.000538855,
1.5,4.5179e-10,1.55525e-06,0.00064524,
1.6,4.3071e-10,1.7902e-06,0.00076304,
1.7,4.13135e-10,2.04915e-06,0.00088848,
1.8,3.9778e-10,2.33365e-06,0.001025755,
];

mux32_cmos = [%vsp,delay,leakage,dynamic_power
0.4,1.92915e-09,1.6731e-07,2.0462e-05;
0.5,1.18035e-09,2.3342e-07,4.71195e-05;
0.6,8.34635e-10,3.10955e-07,8.6461e-05;
0.7,6.4676e-10,4.0094e-07,0.000140975;
0.8,5.3095e-10,5.0445e-07,0.000204975;
0.9,4.53225e-10,6.22635e-07,0.00028428;
1,3.9877e-10,7.5668e-07,0.000381175;
1.1,3.58135e-10,9.07925e-07,0.000495445;
1.2,3.25375e-10,1.077625e-06,0.00063019;
1.3,3.0142e-10,1.267265e-06,0.000777915;
1.4,2.81145e-10,1.478335e-06,0.000949165;
1.5,2.63505e-10,1.71237e-06,0.0011546;
1.6,2.5074e-10,1.97105e-06,0.0013657;
1.7,2.39425e-10,2.2561e-06,0.0015995;
1.8,2.2886e-10,2.5693e-06,0.0018877;
];

mux32_rram = [%vsp,delay,leakage,dynamic_power
680.85,3,0.4,4.8145e-10,2.0238e-07,3.24865e-05;
851.06,3,0.5,3.20385e-10,2.8311e-07,6.90945e-05;
1021.28,3,0.6,2.45515e-10,3.7815e-07,0.000119256;
1191.49,3,0.7,2.05675e-10,4.8884e-07,0.0001848635;
1361.70,3,0.8,1.8185e-10,6.166e-07,0.0002654675;
1531.91,3,0.9,1.6764e-10,7.62955e-07,0.0003622215;
1702.13,3,1,1.58315e-10,9.2952e-07,0.0004774535;
1872.34,3,1.1,1.52365e-10,1.11795e-06,0.00060444;
2042.55,3,1.2,1.4867e-10,1.330085e-06,0.000750085;
2212.77,3,1.3,1.46515e-10,1.56786e-06,0.000908745;
2382.98,3,1.4,1.4549e-10,1.833215e-06,0.00108494;
2553.19,3,1.5,1.4527e-10,2.12833e-06,0.00130925;
2723.40,3,1.6,1.45735e-10,2.4554e-06,0.0015227;
2893.62,3,1.7,1.46715e-10,2.81685e-06,0.001752895;
3063.83,3,1.8,1.48025e-10,3.21505e-06,0.00199314;
];

% dff
figure;
%plot(dff(:,1),dff(:,2).*((dff(:,5)+dff(:,6))/2).*10e15,'b-+');
[AX,H1,H2] = plotyy(dff(:,1),dff(:,2).*10e6,dff(:,1),(dff(:,5)+dff(:,6))/2.*10e9);
hold on
set(H1,'LineStyle','-','Color','b','Marker','+');
set(H2,'LineStyle','--','Color','r','Marker','*');
set(AX,'xlim',[0.4,1.8]);
set(get(AX(1),'Ylabel'),'String','Leakage(uW)','FontSize',14,'Color','k');
set(get(AX(2),'Ylabel'),'String','Delay(ns)','FontSize',14,'Color','k');
xlabel('Vdd(V)','FontSize',14);
legend([H1,H2],{'Leakage';'Delay'});
title('Delay,Leakage-Vdd of D flip-flop, UMC 0.18um','FontSize',18)
grid on

%figure;
%plot(lut4(:,1),lut4(:,2).*lut4(:,3).*10e15,'b-+');
%[AX,H1,H2] = plotyy(lut4(:,1),lut4(:,3).*10e6,lut4(:,1),lut4(:,2).*10e9);
%hold on
%set(H1,'LineStyle','-','Color','b','Marker','+');
%set(H2,'LineStyle','--','Color','r','Marker','*');
%set(get(AX(1),'Ylabel'),'String','Leakage(uW)','FontSize',14,'Color','k');
%set(get(AX(2),'Ylabel'),'String','Delay(ns)','FontSize',14,'Color','k');
%set(AX,'xlim',[0.4,1.8]);
%xlabel('Vdd(V)','FontSize',14);
%legend([H1,H2],{'Leakage';'Delay'});
%title('Delay,Leakage-Vdd of 4-input LUT, UMC 0.18um','FontSize',18)
%grid on

figure;
[AX,H1,H2] = plotyy(lut6(:,1),lut6(:,3).*10e6,lut6(:,1),lut6(:,2).*10e9);
hold on
set(H1,'LineStyle','-','Color','b','Marker','+');
set(H2,'LineStyle','--','Color','r','Marker','*');
set(get(AX(1),'Ylabel'),'String','Leakage(uW)','FontSize',14,'Color','k');
set(get(AX(2),'Ylabel'),'String','Delay(ns)','FontSize',14,'Color','k');
set(AX,'xlim',[0.4,1.8]);
xlabel('Vdd(V)','FontSize',14);
legend([H1,H2],{'Leakage';'Delay'});
title('Delay,Leakage-Vdd of 6-input LUT, UMC 0.18um','FontSize',18)
grid on

figure;
[AX_cmos,H1_delay,H1_leakage] = plotyy(mux32_cmos(:,1),mux32_cmos(:,2).*1e9,mux32_cmos(:,1),mux32_cmos(:,3).*1e6);
hold on
[AX_rram,H2_delay,H2_leakage] = plotyy(mux32_rram(:,3),mux32_rram(:,4).*1e9,mux32_rram(:,3),mux32_rram(:,5).*1e6);
hold on
set(H1_delay,'LineStyle','-','Color','b','Marker','+');
set(H2_delay,'LineStyle','--','Color','r','Marker','*');
set(H1_leakage,'LineStyle','-','Color','b','Marker','o');
set(H2_leakage,'LineStyle','--','Color','r','Marker','x');
set(get(AX_cmos(1),'Ylabel'),'String','Delay(ns)','FontSize',14,'Color','k');
set(get(AX_cmos(2),'Ylabel'),'String','Leakage(uW)','FontSize',14,'Color','k');
set(AX_cmos,'xlim',[0.4,1.8]);
set(AX_rram,'xlim',[0.4,1.8]);
xlabel('Vdd(V)','FontSize',14);
legend([H1_delay,H1_leakage,H2_delay,H2_leakage],{'SRAM 32-input MUX Delay';'SRAM 32-input MUX Leakage';'RRAM 32-input MUX Delay';'RRAM 32-input MUX Leakage'});
%title('Delay,Leakage-Vdd of 32-input MUX, UMC 0.18um','FontSize',18)
grid on

figure;
[AX_cmos,H1_delay,H1_energy] = plotyy(mux32_cmos(:,1),mux32_cmos(:,2).*1e9,mux32_cmos(:,1),mux32_cmos(:,4).*2.21e-8*1e12);
hold on
[AX_rram,H2_delay,H2_energy] = plotyy(mux32_rram(:,3),mux32_rram(:,4).*1e9,mux32_rram(:,3),mux32_rram(:,6).*2.21e-8*1e12);
hold on
set(H1_delay,'LineStyle','-','Color','b','Marker','+','MarkerSize',15,'LineWidth',2);
set(H2_delay,'LineStyle','--','Color','r','Marker','*','MarkerSize',15,'LineWidth',2);
set(H1_energy,'LineStyle','-','Color','b','Marker','o','MarkerSize',15,'LineWidth',2);
set(H2_energy,'LineStyle','--','Color','r','Marker','x','MarkerSize',15,'LineWidth',2);
set(get(AX_cmos(1),'Ylabel'),'String','Delay(ns)','FontSize',16,'Color','k','FontWeight','bold');
set(get(AX_cmos(2),'Ylabel'),'String','Energy(pJ)','FontSize',16,'Color','k','FontWeight','bold');
set(AX_cmos,'xlim',[0.4,1.8]);
set(AX_rram,'xlim',[0.4,1.8]);
xlabel('Vdd(V)','FontSize',16,'FontWeight','bold');
legend([H1_delay,H1_energy,H2_delay,H2_energy],{'SRAM 32-input MUX Delay';'SRAM 32-input MUX Energy';'RRAM 32-input MUX Delay';'RRAM 32-input MUX Energy'},'FontSize',16,'FontWeight','bold');
%title('Delay,Leakage-Vdd of 32-input MUX, UMC 0.18um','FontSize',18)
grid on
