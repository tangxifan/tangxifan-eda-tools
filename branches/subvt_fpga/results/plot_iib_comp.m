%% A Clear Start
clear all;
close all;

%% Data
CW_list = [{'W=320'}];
Arch_list = [{'SRAM-based proposed FPGA (L=4, F_{s}=3)'},{'RRAM-based proposed FPGA (L=4, F_{s}=3)'},{'SRAM-based baseline FPGA (L=4, F_{c,in}=0.15, F_{s}=3)'}, {'RRAM-based baseline FPGA (L=4, F_{c,in}=0.15, F_{s}=3)'}];
fc_in = [0.15, 0.25, 0.33, 0.5, 0.66, 0.75, 1];
tile_area_iib_sram_W320 = [35496,46113.9,54485.03, 72049.87, 88398.89, 97540.53, 122783.79];
tile_area_iib_rram_W320 = [31151.82,41119.02, 49092.78, 66037.01, 81984.53, 90955.01, 115873.00];
tile_area_classical_sram_W320 = [55015.41, 55015.41,55015.41,55015.41,55015.41,55015.41,55015.41];
tile_area_classical_rram_W320 = [45390, 45390, 45390, 45390, 45390, 45390, 45390];
delay_iib_sram_W320 = [57.8,61.9,70.1,78.3,86.5,90.3,98.4];
delay_classical_sram_W320 = [129.8, 129.8, 129.8, 129.8, 129.8, 129.8, 129.8];
delay_iib_rram_W320 = [17.2, 17.2, 17.2, 17.2, 17.2, 17.2, 17.2];
delay_classical_rram_W320 = [48.2, 48.2, 48.2, 48.2, 48.2, 48.2, 48.2];

delay_feedback_classical_sram_W320 = [57.8,61.9,70.1,78.3,86.5,90.3,98.4];
delay_feedback_iib_sram_W320 = [57.8, 57.8, 57.8, 57.8, 57.8, 57.8, 57.8];
delay_feedback_classical_rram_W320 = [17.2, 17.2, 17.2, 17.2, 17.2, 17.2, 17.2];
delay_feedback_iib_rram_W320 = [17.2, 17.2, 17.2, 17.2, 17.2, 17.2, 17.2];

% Fig. 1, Area Comparison 
fig_handle1 = figure;
plot(fc_in, tile_area_iib_sram_W320,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(fc_in, tile_area_iib_rram_W320,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(fc_in, tile_area_classical_sram_W320,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(fc_in, tile_area_classical_rram_W320,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('F_{c,in} (W=320)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Tile Area (#. of M.W.T.A.)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',fc_in);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend(Arch_list);
set(fig_handle1, 'Position', [1 1 800 600]);
grid on

% Fig. 2, Delay Comparison - Routing track to LUT input
fig_handle2 = figure;
plot(fc_in, delay_iib_sram_W320,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(fc_in, delay_iib_rram_W320,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(fc_in, delay_classical_sram_W320,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(fc_in, delay_classical_rram_W320,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('F_{c,in} (W=320)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay from routing tracks to LUT inputs (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',fc_in);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend(Arch_list);
set(fig_handle2, 'Position', [1 1 800 600]);
grid on

% Fig. 3, Delay Comparison - Feedback interconnection 
fig_handle3 = figure;
plot(fc_in, delay_feedback_iib_sram_W320,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(fc_in, delay_feedback_iib_rram_W320,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(fc_in, delay_feedback_classical_sram_W320,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(fc_in, delay_feedback_classical_rram_W320,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('F_{c,in} (W=320)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay of feedback connections (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',fc_in);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend(Arch_list);
set(fig_handle3, 'Position', [1 1 800 600]);
grid on

%% Tile Area vs. channel width W
chanw_list = [100, 150, 200, 250, 300, 350];
% classical SRAM FPGA Arch
tile_area_sram_fpga = [46879.60, 48802.44, 50668.02, 52497.00, 54299.99, 56083.29];
% proposed RRAM FPGA Arch
tile_area_rram_fpga = [29711.13, 35982.81, 42254.49, 48526.18, 54797.86, 61069.54];
% FPGA arch
fpga_arch_list = [{'Classical SRAM FPGA'}, {'Proposed RRAM FPGA'}];

% Fig. 4, Area Comparison 
fig_handle4 = figure;
plot(chanw_list, tile_area_sram_fpga,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(chanw_list, tile_area_rram_fpga,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('Channel Width W','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Tile Area (#. of Min. Width Trans. Area)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[90 360],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',chanw_list);
set(gca,'XTickLabel',chanw_list);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend(fpga_arch_list);
set(fig_handle4, 'Position', [1 1 800 600]);
grid on

%% RRAM MUX 32-input prog. trans. sizing impact on delay
rram_mux32_nonisolate_one_level_sweep_wprog = [ % wprog, delay, leakage power, dynamic power, energy per toggle
1,2.537e-11,9.982e-07,1.801e-05,1.2449e-15;
1.1,2.5635e-11,1.011e-06,1.8485e-05,1.2744e-15;
1.2,2.5415e-11,1.023e-06,1.887e-05,1.29315e-15;
1.3,2.5685e-11,1.05e-06,1.937e-05,1.33085e-15;
1.4,2.491e-11,1.063e-06,1.9435e-05,1.32465e-15;
1.5,2.515e-11,1.076e-06,1.986e-05,1.35235e-15;
1.6,2.5405e-11,1.09e-06,2.031e-05,1.38685e-15;
1.7,2.5415e-11,1.104e-06,2.0605e-05,1.40595e-15;
1.8,2.5435e-11,1.118e-06,2.09e-05,1.4252e-15;
1.9,2.528e-11,1.138e-06,2.087e-05,1.41955e-15;
2,2.5605e-11,1.129e-06,2.1465e-05,1.4635e-15;
];
% Fig. 5, Delay Comparison - Wprog
fig_handle5 = figure;
%plot(rram_mux32_nonisolate_one_level_sweep_wprog(:,2)/1e-12,'b-o','LineWidth', 2, 'MarkerSize',10);
plot(rram_mux32_nonisolate_one_level_sweep_wprog(:,5)/1e-15,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ns) and Wprog','FontSize',18)
xlabel('Wprog (1 Wprog = 320nm)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
ylabel('PDP (FJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',rram_mux32_nonisolate_one_level_sweep_wprog(:,1));
set(gca,'XTickLabel',rram_mux32_nonisolate_one_level_sweep_wprog(:,1));
%%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend({'RRAM MUX size=2'});
set(fig_handle5, 'Position', [1 1 650 500]);
grid on

%% TSMC40nm SRAM MUX (2-level) vs 4T1R MUX (1-level) in delay and power
mux_size_list = 2:2:32;
% TSMC 40nm SRAM MUX and RRAM MUX data
sram_mux_fig_data_tsmc40nm
 
% Fig. 6.2, Delay Comparison - SRAM MUX vs. 1-level RRAM MUX , diff. VDD
fig_handle6_2 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,2)/1e-12,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_0p9V(:,1), rram_mux_0p9V(:,2)/1e-12,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick', sram_mux_0p9V(:,1));
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[15 60],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'SRAM MUX'}, {'4T1R MUX'}]);
set(fig_handle6_2, 'Position', [1 1 650 500]);
grid on

% Fig. 6.2, Power-Delay Product Comparison - SRAM MUX vs. 1-level RRAM MUX , diff. VDD
fig_handle6_3 = figure;
plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,2).*sram_mux_0p9V(:,4)/1e-15,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_0p9V(:,1), rram_mux_0p9V(:,2).*rram_mux_0p9V(:,4)/1e-15,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Energy (Power-Delay Product) (fJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick', sram_mux_0p9V(:,1));
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[15 60],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'SRAM MUX'}, {'4T1R MUX'}]);
set(fig_handle6_3, 'Position', [1 1 650 500]);
grid on
