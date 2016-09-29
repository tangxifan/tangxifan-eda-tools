clear all
close all
%% Figure Data Source
std_1p8V = [ % Circuit Name, # of LUT, Min Chan., Area, Delay, Power 
{'alu4'},503,44,1.74E+06,11.0901,0.8008;
{'apex2'},643,62,2.46E+06,13.0504,1.069;
{'apex4'},577,68,2.39E+06,12.9259,1.086;
{'bigkey'},571,38,2.80E+06,5.64079,3.522;
{'clma'},2787,84,1.09E+07,18.1702,4.139;
{'des'},556,36,2.99E+06,8.40318,4.744;
{'diffeq'},562,42,2.12E+06,9.46399,1.066;
{'dsip'},681,36,2.97E+06,6.40369,3.689;
{'elliptic'},315,30,1.20E+06,9.64305,0.5451;
{'ex5p'},354,64,1.34E+06,11.0057,0.6266;
{'ex1010'},647,70,2.78E+06,10.77,1.05;
{'frisc'},1752,88,7.48E+06,17.5363,2.855;
{'misex3'},438,48,1.63E+06,9.93789,0.8228;
{'pdc'},1350,104,5.67E+06,13.1213,1.995;
{'s38417'},2164,50,7.87E+06,10.0192,3.154;
{'s38584.1'},2093,66,7.67E+06,10.3171,3.269;
{'s298'},18,20,8.87E+04,2.52428,0.07622;
{'seq'},668,74,2.62E+06,11.9027,1.068;
{'spla'},1373,114,5.87E+06,13.865,1.991;
{'tseng'},657,54,1.98E+06,9.58416,0.876;
];

std_1p2V = [ % Circuit Name, # of LUT, Min Chan., Area, Delay, Power 
{'alu4'},503,46,1.74E+06,13.1576,0.2745;
{'apex2'},643,66,2.51E+06,13.5655,0.3676;
{'apex4'},577,66,2.38E+06,14.6732,0.3727;
{'bigkey'},571,38,2.80E+06,7.20877,1.203;
{'clma'},2787,82,1.09E+07,20.4312,1.422;
{'des'},556,40,3.08E+06,9.39861,1.625;
{'diffeq'},562,40,2.12E+06,13.8263,0.3651;
{'dsip'},681,38,3.00E+06,8.03478,1.261;
{'elliptic'},315,30,1.20E+06,12.4401,0.1871;
{'ex5p'},354,62,1.33E+06,15.8401,0.2145;
{'ex1010'},647,70,2.78E+06,13.4124,0.3603;
{'frisc'},1752,88,7.48E+06,22.2794,0.9797;
{'misex3'},438,48,1.60E+06,14.0711,0.2815;
{'pdc'},1350,100,5.63E+06,19.3013,0.6846;
{'s38417'},2164,52,7.85E+06,12.5772,1.085;
{'s38584.1'},2093,66,7.70E+06,13.1104,1.118;
{'s298'},18,20,8.87E+04,6.51511,0.0256;
{'seq'},668,72,2.60E+06,12.814,0.3673;
{'spla'},1373,114,5.87E+06,17.1913,0.684;
{'tseng'},657,54,2.02E+06,13.1106,0.2999;
];

rram_1p2V = [ % Circuit Name, # of LUT, Min Chan., Area, Delay, Power 
{'alu4'},503,40,1.52E+06,8.72591,0.2756;
{'apex2'},643,68,2.10E+06,11.4962,0.3694;
{'apex4'},577,66,2.00E+06,11.868,0.3731;
{'bigkey'},571,42,2.26E+06,4.93097,1.214;
{'clma'},2787,82,9.24E+06,15.6507,1.426;
{'des'},556,42,2.37E+06,6.13233,1.635;
{'diffeq'},562,40,1.86E+06,8.53808,0.3663;
{'dsip'},681,34,2.40E+06,5.39885,1.27;
{'elliptic'},315,28,1.05E+06,10.366,0.1874;
{'ex5p'},354,62,1.12E+06,9.81796,0.2156;
{'ex1010'},647,70,2.41E+06,10.635,0.3611;
{'frisc'},1752,86,6.25E+06,16.2109,0.9815;
{'misex3'},438,44,1.36E+06,10.042,0.2826;
{'pdc'},1350,104,4.73E+06,11.8263,0.6881;
{'s38417'},2164,54,6.87E+06,8.45237,1.097;
{'s38584.1'},2093,68,6.55E+06,8.66856,1.13;
{'s298'},18,18,7.34E+04,2.66432,0.02598;
{'seq'},668,74,2.24E+06,8.70075,0.3707;
{'spla'},1373,114,4.85E+06,11.9705,0.6865;
{'tseng'},657,54,1.76E+06,9.22226,0.3009;
];

rram_sizing_1p2V = [ % Circuit Name, # of LUT, Min Chan., Area, Delay, Power 
{'alu4'},    503,46,1.25E+06,8.79897,0.2762;
{'apex2'},   643,62,1.73E+06,11.6499,0.3689;
{'apex4'},   577,68,1.63E+06,9.7291,0.3736;
{'bigkey'},  571,34,1.80E+06,5.03868,1.21;
{'clma'},    2787,78,7.46E+06,17.3634,1.425;
{'des'},     556,36,1.89E+06,7.16036,1.628;
{'diffeq'},  562,40,1.51E+06,8.04218,0.3666;
{'dsip'},    681,36,1.97E+06,5.99609,1.267;
{'elliptic'},315,30,8.62E+05,8.78802,0.1878;
{'ex5p'},    354,66,9.20E+05,8.74814,0.2161;
{'ex1010'},  647,70,1.97E+06,9.55615,0.3614;
{'frisc'},   1752,88,5.13E+06,15.6679,0.9822;
{'misex3'},  438,50,1.14E+06,7.25313,0.2844;
{'pdc'},     1350,102,3.82E+06,13.573,0.6872;
{'s38417'},  2164,50,5.55E+06,8.51229,1.093;
{'s38584.1'},2093,64,5.30E+06,9.03157,1.125;
{'s298'},    18,20,6.16E+04,2.37737,0.02606;
{'seq'},     668,72,1.81E+06,8.79817,0.3705;
{'spla'},    1373,106,3.92E+06,12.1145,0.6856;
{'tseng'},   657,54,1.44E+06,9.08771,0.3009;
];

%% Pre-process the raw data
circuit_names = std_1p8V(:,1);
std_1p8V_results = cell2mat(std_1p8V(:,(4:6)));
std_1p2V_results = cell2mat(std_1p2V(:,(4:6)));
rram_1p2V_results = cell2mat(rram_1p2V(:,(4:6)));
rram_sizing_1p2V_results = cell2mat(rram_sizing_1p2V(:,(4:6)));
xindex = 1:1:length(circuit_names);
% Table the area, delay, power
area_table = [std_1p8V_results(:,1) std_1p2V_results(:,1) rram_1p2V_results(:,1) rram_sizing_1p2V_results(:,1)];
delay_table = [std_1p8V_results(:,2) std_1p2V_results(:,2) rram_1p2V_results(:,2) rram_sizing_1p2V_results(:,2)];
power_table = [std_1p8V_results(:,3) std_1p2V_results(:,3) rram_1p2V_results(:,3) rram_sizing_1p2V_results(:,3)];
% Average the area, delay and power
average_area_table = mean(area_table);
average_delay_table = mean(delay_table);
average_power_table = mean(power_table);
% normalize area, delay and power 
normalize_area_table = average_area_table./max(average_area_table((1:3)));
normalize_delay_table = average_delay_table./max(average_delay_table((1:3)));
normalize_power_table = average_power_table./max(average_power_table((1:3)));

%% Bar graphs
% Normailized Graph
fig_handle0 = figure;
b = bar([normalize_area_table(:,(1:3))', normalize_delay_table(:,(1:3))', normalize_power_table(:,(1:3))']'); 
ch = get(b,'children');
set(gca,'xlim',[0.5 3+0.5],'Fontsize',16);
set(gca,'ylim',[0 1+0.2],'Fontsize',16);
set(gca,'XTick',xindex);
set(gca,'XTickLabel',[{'Area'};{'Delay'};{'Power'}]);
set(fig_handle0, 'Position', [1 1 1600 400]);
legend('SRAM FPGA, Vdd=1.8V','Near-Vt SRAM FPGA, Vdd=1.2V','Near-Vt RRAM FPGA, Vdd=1.2V');
ylabel('Normalized Area, Delay and Power');
grid on

% normalize area, delay and power 
normalize_area_table = average_area_table./max(average_area_table);
normalize_delay_table = average_delay_table./max(average_delay_table);
normalize_power_table = average_power_table./max(average_power_table);

%% Bar graphs
% Normailized Graph
fig_handle01 = figure;
b = bar([normalize_area_table(:,(1:4))', normalize_delay_table(:,(1:4))', normalize_power_table(:,(1:4))']'); 
ch = get(b,'children');
set(gca,'xlim',[0.5 3+0.5],'Fontsize',16);
set(gca,'ylim',[0 1+0.2],'Fontsize',16);
set(gca,'XTick',xindex);
set(gca,'XTickLabel',[{'Area'};{'Delay'};{'Power'}]);
set(fig_handle01, 'Position', [1 1 1600 400]);
legend('SRAM FPGA, Vdd=1.8V','Near-Vt SRAM FPGA, Vdd=1.2V','Near-Vt RRAM FPGA, Vdd=1.2V', 'Sized Near-Vt RRAM FPGA, Vdd=1.2V');
ylabel('Normalized Area, Delay and Power');
grid on


% Area
fig_handle1 = figure;
%h1 = subplot(3,1,1);
b = bar([std_1p8V_results(:,1),std_1p2V_results(:,1),rram_1p2V_results(:,1),rram_sizing_1p2V_results(:,1)]);
ch = get(b,'children');
set(gca,'xlim',[0.5 length(circuit_names)+0.5],'Fontsize',16);
set(gca,'XTick',xindex);
set(gca,'XTickLabel',circuit_names);
set(fig_handle1, 'Position', [1 1 1600 400]);
legend('Standard CMOS arch. Vdd=1.8V','Standard CMOS arch. Vdd=1.2V','Uniformly sized RRAM arch. Vdd=1.2V','Non-uniformly sized RRAM arch. Vdd=1.2V');
ylabel('Area (# of tran.)');
grid on

% Delay
fig_handle2 = figure;
%h2 = subplot(3,1,2);
b = bar([std_1p8V_results(:,2),std_1p2V_results(:,2),rram_1p2V_results(:,2),rram_sizing_1p2V_results(:,2)]);
ch = get(b,'children');
set(gca,'xlim',[0.5 length(circuit_names)+0.5],'Fontsize',16);
set(gca,'XTick',xindex);
set(gca,'XTickLabel',circuit_names);
set(fig_handle2, 'Position', [1 1 1600 400]);
legend('Standard CMOS arch. Vdd=1.8V','Standard CMOS arch. Vdd=1.2V','Uniformly sized RRAM arch. Vdd=1.2V','Non-uniformly sized RRAM arch. Vdd=1.2V');
ylabel('Critical Path (ns)');
grid on

% Power
fig_handle3 = figure;
%h3 = subplot(3,1,3);
b = bar([std_1p8V_results(:,3),std_1p2V_results(:,3),rram_1p2V_results(:,3),rram_sizing_1p2V_results(:,3)]);
ch = get(b,'children');
set(gca,'xlim',[0.5 length(circuit_names)+0.5],'Fontsize',16);
set(gca,'XTick',xindex);
set(gca,'XTickLabel',circuit_names);
set(fig_handle3, 'Position', [1 1 1600 400]);
legend('Standard CMOS arch. Vdd=1.8V','Standard CMOS arch. Vdd=1.2V','Uniformly sized RRAM arch. Vdd=1.2V','Non-uniformly sized RRAM arch. Vdd=1.2V');
ylabel('Power (W)');
grid on
