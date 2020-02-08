close all;
clear all;

% Delay and power swept from RHRS 0.5MOhm to 5MOhm
sram_mux4_1level_0p9V = [
%w_cpt,delay,leakage,dynamic_power
1,1.3042e-09,9.682e-11,2.206e-06,1.91e-15;
];

sram_mux4_1level_1p8V = [
1,2.741e-10,5.204e-10,3.0235e-05,1.3445e-14;
];

rram_mux4_1level_0p9V = [
%vsp,delay,leakage,dynamic_power,energy_per_toggle
0.1,8.0375e-10,1.955e-07,3.698e-06,5.828e-15;
1,7.6515e-10,3.532e-07,3.0755e-06,4.471e-15;
2,9.5785e-10,2.779e-07,2.192e-06,3.5035e-15;
3,9.0275e-10,2.801e-07,2.116e-06,3.2345e-15;
4,8.7615e-10,2.748e-07,2.076e-06,3.111e-15;
5,8.628e-10,2.798e-07,2.0475e-06,3.034e-15;
];

% Fig. plot
mux_list = [{'SRAM MUX V_{DD}=1.8V'}, {'SRAM MUX V_{DD}=0.9V'}, {'RRAM MUX'}];
roff_list = [0.5,1,2,3,4,5;];
xindex = 1:1:length(rram_mux4_1level_0p9V(:,1));
sram_mux4_power_0p9V = sram_mux4_1level_0p9V(4) * ones(1, length(xindex));
sram_mux4_power_1p8V = sram_mux4_1level_1p8V(4) * ones(1, length(xindex));
fig_handle1 = figure;
plot(sram_mux4_power_1p8V.*1e6,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(sram_mux4_power_0p9V.*1e6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux4_1level_0p9V(:,4).*1e6,'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}({M\Omega})','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Total Power ({\mu}W)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(roff_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',roff_list);
hleg = legend(mux_list);
grid on

% Fig. plot
mux_list = [{'SRAM MUX V_{DD}=1.8V'}, {'SRAM MUX V_{DD}=0.9V'}, {'RRAM MUX'}];
roff_list = [0.5,1,2,3,4,5;];
xindex = 1:1:length(rram_mux4_1level_0p9V(:,1));
sram_mux4_energy_0p9V = sram_mux4_1level_0p9V(2) * sram_mux4_1level_0p9V(4) * ones(1, length(xindex));
sram_mux4_energy_1p8V = sram_mux4_1level_1p8V(2) * sram_mux4_1level_1p8V(4) * ones(1, length(xindex));
fig_handle2 = figure;
plot(sram_mux4_energy_1p8V.*1e15,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(sram_mux4_energy_0p9V.*1e15,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux4_1level_0p9V(:,2).* rram_mux4_1level_0p9V(:,4) .*1e15,'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}({M\Omega})','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Energy (fJ)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(roff_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',roff_list);
hleg = legend(mux_list);
grid on


