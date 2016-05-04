%% A Clear Start
clear all;
close all;

%% Data
mux_size_list = 2:2:32;
mux_list_area_comp = [{'SRAM MUX'}, {'RRAM MUX (1-level)'}, {'RRAM MUX (2-level)'}, {'RRAM MUX (tree-like)'}];
mux_list_delay_comp = [{'SRAM MUX V_{DD}=0.7V'}, {'SRAM MUX V_{DD}=0.8V'}, {'SRAM MUX V_{DD}=0.9V'}];

%% import data
%% SRAM MUX
sram_mux_fig_data

%% RRAM MUX data
% Study the impact of prog. trans sizes
wprog_list = 1:0.1:2;
rram_mux2_isolate_one_level_sweep_wprog = [%wprog,delay,leakage,dynamic_power,
1,3.147e-11,1.413e-05,8.31315e-06,3.53061e-16;
1.1,3.204e-11,1.407e-05,8.2705e-06,3.5383e-16;
1.2,3.258e-11,1.596e-05,8.0855e-06,3.486975e-16;
1.3,3.302e-11,1.594e-05,8.09755e-06,3.52055e-16;
1.4,3.371e-11,1.591e-05,8.1067e-06,3.577865e-16;
1.5,3.4345e-11,1.584e-05,8.1183e-06,3.495905e-16;
1.6,3.499e-11,1.575e-05,8.132645e-06,3.51172e-16;
1.7,3.561e-11,1.567e-05,8.151395e-06,3.56661e-16;
1.8,3.5845e-11,1.552e-05,8.165905e-06,3.68276e-16;
1.9,3.6675e-11,1.535e-05,8.17636e-06,3.726315e-16;
2,3.731e-11,1.516e-05,8.19424e-06,3.751045e-16;
];

%% circuit design with isolating transistors
rram_mux_isolate_one_level_fig_data
%rram_mux_isolate_two_level_fig_data
%rram_mux_isolate_multi_level_fig_data

%% non-isolate design
rram_mux_nonisolate_one_level_fig_data
rram_mux_nonisolate_two_level_fig_data
rram_mux_nonisolate_multi_level_fig_data

% Fig. 1, Area Comparison - SRAM MUX vs. RRAM MUX
%fig_handle1 = figure;
%plot(mux_size_list, area_sram_mux,'b-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(mux_size_list, area_rram_mux_isolate_one_level,'r-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(mux_size_list, area_rram_mux_isolate_two_level,'b-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(mux_size_list, area_rram_mux_isolate_multi_level,'r-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Area (#. MWTA) and MUX size N','FontSize',18)
%xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Area (#. of Min. Width Trans. Area)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend(mux_list_area_comp);
%set(fig_handle1, 'Position', [1 1 800 600]);
%grid on

% Fig. 2.1, Delay Comparison 1 - SRAM MUX vs. RRAM MUX with isolated transistors, diff. structures
fig_handle2_1 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_one_level_0p9V(:,1), rram_mux_isolate_one_level_0p9V(:,2)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_two_level_0p9V(:,1), rram_mux_isolate_two_level_0p9V(:,2)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_multi_level_0p9V(:,1), rram_mux_isolate_multi_level_0p9V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'1-level 4T1R MUX with isolate transistors'}, {'2-level 4T1R MUX with isolate transistors'}, {'tree-like 4T1R MUX with isolate transistors'}]);
set(fig_handle2_1, 'Position', [1 1 800 600]);
grid on

% Fig. 2.2, Delay Comparison 2 - SRAM MUX vs. 1-level RRAM MUX with isolated transistors, diff. VDD
fig_handle2_2 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_one_level_0p7V(:,1), rram_mux_isolate_one_level_0p7V(:,2)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_one_level_0p8V(:,1), rram_mux_isolate_one_level_0p8V(:,2)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_one_level_0p9V(:,1), rram_mux_isolate_one_level_0p9V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.9V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.7V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.8V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.9V)'}]);
set(fig_handle2_2, 'Position', [1 1 800 600]);
grid on


% Fig. 3.1, Power Comparison 1 - SRAM MUX vs. RRAM MUX with isolated transistors, diff. structures
fig_handle3_1 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,4)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_one_level_0p9V(:,1), rram_mux_isolate_one_level_0p9V(:,4)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_two_level_0p9V(:,1), rram_mux_isolate_two_level_0p9V(:,4)/1e-6,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_multi_level_0p9V(:,1), rram_mux_isolate_multi_level_0p9V(:,4)/1e-6,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power ({\mu}W) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'1-level 4T1R MUX with isolate transistors'}, {'2-level 4T1R MUX with isolate transistors'}, {'tree-like 4T1R MUX with isolate transistors'}]);
set(fig_handle3_1, 'Position', [1 1 800 600]);
grid on

% Fig. 3.2, Power Comparison 2 - SRAM MUX vs. 1-level RRAM MUX with isolated transistors, diff. VDD
fig_handle3_2 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,4)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_one_level_0p7V(:,1), rram_mux_isolate_one_level_0p7V(:,4)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_one_level_0p8V(:,1), rram_mux_isolate_one_level_0p8V(:,4)/1e-6,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_isolate_one_level_0p9V(:,1), rram_mux_isolate_one_level_0p9V(:,4)/1e-6,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power ({\mu}W) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.9V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.7V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.8V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.9V)'}]);
set(fig_handle3_2, 'Position', [1 1 800 600]);
grid on

% Fig. 4, Delay Comparison - Wprog
%fig_handle4 = figure;
%plot(rram_mux2_isolate_one_level_sweep_wprog(:,2)/1e-12,'b-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Delay (ns) and Wprog','FontSize',18)
%xlabel('Wprog (1 Wprog = 280nm)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%%set(gca,'XTick',rram_mux2_isolate_one_level_sweep_wprog(:,1));
%set(gca,'XTickLabel',rram_mux2_isolate_one_level_sweep_wprog(:,1));
%%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend({'RRAM MUX size=2'});
%set(fig_handle4, 'Position', [1 1 800 600]);
%grid on

% Fig. 5, Area Comparison - SRAM MUX vs. RRAM MUX non-isolate
%fig_handle5 = figure;
%plot(mux_size_list, area_sram_mux,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(mux_size_list, area_rram_mux_isolate_one_level,'r-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(mux_size_list, area_rram_mux_nonisolate_one_level,'r-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(mux_size_list, area_rram_mux_nonisolate_two_level,'b-+','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(mux_size_list, area_rram_mux_nonisolate_multi_level,'r-x','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Area (#. MWTA) and MUX size N','FontSize',18)
%xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Area (#. of Min. Width Trans. Area)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',mux_size_list);
%%set(gca,'XTickLabel',fc_in);
%%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'SRAM MUX'}, {'RRAM MUX (1-level)'}, {'RRAM MUX (2-level)'}, {'RRAM MUX (tree-like)'}]);
%set(fig_handle5, 'Position', [1 1 800 600]);
%grid on

% Fig. 6.1, Delay Comparison 1 - SRAM MUX vs. RRAM MUX with dynamic voltage control, diff. structures
fig_handle6_1 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p9V(:,1), rram_mux_nonisolate_one_level_0p9V(:,2)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_two_level_0p9V(:,1), rram_mux_nonisolate_two_level_0p9V(:,2)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_multi_level_0p9V(:,1), rram_mux_nonisolate_multi_level_0p9V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'1-level 4T1R MUX with dynamic voltage control'}, {'2-level 4T1R MUX with dynamic voltage control'}, {'tree-like 4T1R MUX with dynamic voltage control'}]);
set(fig_handle6_1, 'Position', [1 1 800 600]);
grid on

% Fig. 6.2, Delay Comparison 2 - SRAM MUX vs. 1-level RRAM MUX with dynamic voltage control, diff. VDD
fig_handle6_2 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p7V(:,1), rram_mux_nonisolate_one_level_0p7V(:,2)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p8V(:,1), rram_mux_nonisolate_one_level_0p8V(:,2)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p9V(:,1), rram_mux_nonisolate_one_level_0p9V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.9V)'}, {'1-level 4T1R MUX with dynamic voltage control (V_{DD}=0.7V)'}, {'1-level 4T1R MUX with dynamic voltage control (V_{DD}=0.8V)'}, {'1-level 4T1R MUX with dynamic voltage control (V_{DD}=0.9V)'}]);
set(fig_handle6_2, 'Position', [1 1 800 600]);
grid on


% Fig. 7.1, Power Comparison 1 - SRAM MUX vs. RRAM MUX with dynamic voltage control, diff. structures
fig_handle7_1 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,4)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p9V(:,1), rram_mux_nonisolate_one_level_0p9V(:,4)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_two_level_0p9V(:,1), rram_mux_nonisolate_two_level_0p9V(:,4)/1e-6,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_multi_level_0p9V(:,1), rram_mux_nonisolate_multi_level_0p9V(:,4)/1e-6,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power ({\mu}W) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'1-level 4T1R MUX with dynamic voltage control'}, {'2-level 4T1R MUX with dynamic voltage control'}, {'tree-like 4T1R MUX with dynamic voltage control'}]);
set(fig_handle7_1, 'Position', [1 1 800 600]);
grid on

% Fig. 7.2, Power Comparison 2 - SRAM MUX vs. 1-level RRAM MUX with dynamic voltage control, diff. VDD
fig_handle7_2 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,4)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p7V(:,1), rram_mux_nonisolate_one_level_0p7V(:,4)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p8V(:,1), rram_mux_nonisolate_one_level_0p8V(:,4)/1e-6,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p9V(:,1), rram_mux_nonisolate_one_level_0p9V(:,4)/1e-6,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power ({\mu}W) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.9V)'}, {'1-level 4T1R MUX with dynamic voltage control (V_{DD}=0.7V)'}, {'1-level 4T1R MUX with dynamic voltage control (V_{DD}=0.8V)'}, {'1-level 4T1R MUX with dynamic voltage control (V_{DD}=0.9V)'}]);
set(fig_handle7_2, 'Position', [1 1 800 600]);
grid on

% Fig. 8, Area-Delay Comparison - SRAM MUX vs. RRAM MUX with dynamic voltage control
fig_handle8 = figure;
plot(sram_mux_0p9V(:,1), (area_sram_mux').*sram_mux_0p9V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p7V(:,1), (area_rram_mux_nonisolate_one_level').*rram_mux_nonisolate_one_level_0p7V(:,2)/1e-12,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p8V(:,1), (area_rram_mux_nonisolate_one_level').*rram_mux_nonisolate_one_level_0p8V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p9V(:,1), (area_rram_mux_nonisolate_one_level').*rram_mux_nonisolate_one_level_0p9V(:,2)/1e-12,'r-+','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Area-Delay Product (M.W.T.A * ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Area-Delay Product(M.W.T.A * ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'SRAM MUX (V_{DD}=0.9V)'}, {'RRAM MUX (1-level, V_{DD}=0.7V)'}, {'RRAM MUX (1-level, V_{DD}=0.8V)'},{'RRAM MUX (1-level, V_{DD}=0.9V)'}]);
set(fig_handle8, 'Position', [1 1 800 600]);
grid on

% Fig. 9, Power-Delay Comparison - SRAM MUX vs. RRAM MUX with dynamic voltage control
%fig_handle9 = figure;
%plot(sram_mux_0p9V(:,1), (sram_mux_0p9V(:,2)/1e-12).*(sram_mux_0p9V(:,4)/1e-6),'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_nonisolate_one_level_0p9V(:,1), (rram_mux_nonisolate_one_level_0p9V(:,2)/1e-12).*(rram_mux_nonisolate_one_level_0p9V(:,4)/1e-6),'r-+','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_nonisolate_one_level_0p7V(:,1), (rram_mux_nonisolate_one_level_0p7V(:,2)/1e-12).*(rram_mux_nonisolate_one_level_0p7V(:,4)/1e-6),'b-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Power-Delay Product ({\mu}W * ps) and MUX size N','FontSize',18)
%xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Power-Delay Product ({\mu}W * ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',mux_size_list);
%%set(gca,'XTickLabel',fc_in);
%%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'SRAM MUX (V_{DD}=0.9V)'}, {'RRAM MUX (1-level, V_{DD}=0.9V)'}, {'RRAM MUX (1-level, V_{DD}=0.7V)'}]);
%set(fig_handle9, 'Position', [1 1 800 600]);
%grid on


