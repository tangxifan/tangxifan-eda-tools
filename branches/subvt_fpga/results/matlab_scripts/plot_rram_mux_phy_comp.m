%% A Clear Start
clear all;
close all;

%% Data
mux_size_list = 2:2:32;
mux_list_delay_comp = [{'SRAM MUX V_{DD}=0.5V'}, 
                       {'SRAM MUX V_{DD}=0.6V'}, 
                       {'SRAM MUX V_{DD}=0.7V'},
                       {'Improved 4T1R MUX V_{DD}=0.5V'}, 
                       {'Improved 4T1R MUX V_{DD}=0.6V'}, 
                       {'Improved 4T1R MUX V_{DD}=0.7V'},
                      ];

%% import data
%% SRAM MUX
sram_mux_parasitic_fig_data

%% 4T1R MUX
% Basic design

%% improved design 
rram_mux_improv_1level_parasitic_x0_fig_data
rram_mux_improv_1level_parasitic_x1_fig_data

%% Classify the data into different array
% x = 0, VDD=0.5V
rram_mux_improv_1level_parasitic_x0_0p5V_1fin = rram_mux_improv_1level_parasitic_x0_0p5V(1:3:end,:);
rram_mux_improv_1level_parasitic_x0_0p5V_2fin = rram_mux_improv_1level_parasitic_x0_0p5V(2:3:end,:);
rram_mux_improv_1level_parasitic_x0_0p5V_3fin = rram_mux_improv_1level_parasitic_x0_0p5V(3:3:end,:);
% x = 0, VDD=0.6V
rram_mux_improv_1level_parasitic_x0_0p6V_1fin = rram_mux_improv_1level_parasitic_x0_0p6V(1:3:end,:);
rram_mux_improv_1level_parasitic_x0_0p6V_2fin = rram_mux_improv_1level_parasitic_x0_0p6V(2:3:end,:);
rram_mux_improv_1level_parasitic_x0_0p6V_3fin = rram_mux_improv_1level_parasitic_x0_0p6V(3:3:end,:);
% x = 0, VDD=0.7V
rram_mux_improv_1level_parasitic_x0_0p7V_1fin = rram_mux_improv_1level_parasitic_x0_0p7V(1:3:end,:);
rram_mux_improv_1level_parasitic_x0_0p7V_2fin = rram_mux_improv_1level_parasitic_x0_0p7V(2:3:end,:);
rram_mux_improv_1level_parasitic_x0_0p7V_3fin = rram_mux_improv_1level_parasitic_x0_0p7V(3:3:end,:);
% x = 0, VDD=0.5V
rram_mux_improv_1level_parasitic_x1_0p5V_1fin = rram_mux_improv_1level_parasitic_x1_0p5V(1:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p5V_2fin = rram_mux_improv_1level_parasitic_x1_0p5V(2:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p5V_3fin = rram_mux_improv_1level_parasitic_x1_0p5V(3:3:end,:);
% x = 0, VDD=0.6V
rram_mux_improv_1level_parasitic_x1_0p6V_1fin = rram_mux_improv_1level_parasitic_x1_0p6V(1:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p6V_2fin = rram_mux_improv_1level_parasitic_x1_0p6V(2:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p6V_3fin = rram_mux_improv_1level_parasitic_x1_0p6V(3:3:end,:);
% x = 0, VDD=0.7V
rram_mux_improv_1level_parasitic_x1_0p7V_1fin = rram_mux_improv_1level_parasitic_x1_0p7V(1:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p7V_2fin = rram_mux_improv_1level_parasitic_x1_0p7V(2:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p7V_3fin = rram_mux_improv_1level_parasitic_x1_0p7V(3:3:end,:);

% Fig. 0, Delay comparsion with different fin number : Improved 4T1R MUX x=1
fig_handle0 = figure;
% Improved 4T1R MUX x=1 delay, VDD = 0.5V
plot(rram_mux_improv_1level_parasitic_x1_0p5V_1fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_1fin(:,3)/1e-12,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p5V_2fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_2fin(:,3)/1e-12,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,3)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, VDD = 0.6V
plot(rram_mux_improv_1level_parasitic_x1_0p6V_1fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_1fin(:,3)/1e-12,'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p6V_2fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_2fin(:,3)/1e-12,'b-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,3)/1e-12,'k-s','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, VDD = 0.7V
plot(rram_mux_improv_1level_parasitic_x1_0p7V_1fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_1fin(:,3)/1e-12,'r-^','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p7V_2fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_2fin(:,3)/1e-12,'b-^','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,3)/1e-12,'k-^','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'Improved 4T1R MUX (x=1, Fin no.=1, V_{DD}=0.5V)'}
               {'Improved 4T1R MUX (x=1, Fin no.=2, V_{DD}=0.5V)'}
               {'Improved 4T1R MUX (x=1, Fin no.=3, V_{DD}=0.5V)'}
               {'Improved 4T1R MUX (x=1, Fin no.=1, V_{DD}=0.6V)'}
               {'Improved 4T1R MUX (x=1, Fin no.=2, V_{DD}=0.6V)'}
               {'Improved 4T1R MUX (x=1, Fin no.=3, V_{DD}=0.6V)'}
               {'Improved 4T1R MUX (x=1, Fin no.=1, V_{DD}=0.7V)'}
               {'Improved 4T1R MUX (x=1, Fin no.=2, V_{DD}=0.7V)'}
               {'Improved 4T1R MUX (x=1, Fin no.=3, V_{DD}=0.6V)'}
              ]);
set(fig_handle0, 'Position', [1 1 650 500]);
grid on


% Fig. 1, Delay Comparison: SRAM MUX vs. Basic 4T1R MUX, Improved 4T1R MUX x=0, Improved 4T1R MUX x=1
fig_handle1 = figure;
% SRAM MUX delay VDD=0.7V
plot(sram_mux_parasitic_0p7V(:,1), sram_mux_parasitic_0p7V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% Basic 4T1R MUX
hold on
% Improved 4T1R MUX x=0 delay, RRAM is close to input inverters
plot(rram_mux_improv_1level_parasitic_x0_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x0_0p7V_3fin(:,3)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, RRAM is close to output inverters
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,3)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'Improved 4T1R MUX (x=0)'}, {'Improved 4T1R (x=1)'}]);
set(fig_handle1, 'Position', [1 1 650 500]);
grid on

% Fig. 1-1, Power Comparison: SRAM MUX vs. Basic 4T1R MUX, Improved 4T1R MUX x=0, Improved 4T1R MUX x=1
fig_handle1_1 = figure;
% SRAM MUX delay VDD=0.7V
plot(sram_mux_parasitic_0p7V(:,1), sram_mux_parasitic_0p7V(:,4)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% Basic 4T1R MUX
hold on
% Improved 4T1R MUX x=0 delay, RRAM is close to input inverters
plot(rram_mux_improv_1level_parasitic_x0_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x0_0p7V_3fin(:,5)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, RRAM is close to output inverters
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,5)/1e-6,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'Improved 4T1R MUX (x=0)'}, {'Improved 4T1R (x=1)'}]);
set(fig_handle1, 'Position', [1 1 650 500]);
grid on

% Fig. 2, Delay Comparison: SRAM MUX vs Improved 4T1R MUX under different VDD
fig_handle2 = figure;
% SRAM MUX VDD = 0.5V
plot(sram_mux_parasitic_0p5V(:,1), sram_mux_parasitic_0p5V(:,2)/1e-12,'c-s','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM MUX VDD = 0.7V
plot(sram_mux_parasitic_0p7V(:,1), sram_mux_parasitic_0p7V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.5V
plot(rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,3)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.6V
plot(rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,3)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.7V
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,3)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[15 60],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.5V)'}, {'CMOS MUX (V_{DD}=0.7V)'}, {'Improved 4T1R MUX (V_{DD}=0.5V)'}, {'Improved 4T1R MUX (V_{DD}=0.6V)'}, {'Improved 4T1R MUX  (V_{DD}=0.7V)'}]);
set(fig_handle2, 'Position', [1 1 650 500]);
grid on

% Fig. 2-1, Power Comparison: SRAM MUX vs Improved 4T1R MUX under different VDD
fig_handle2_1 = figure;
% SRAM MUX VDD = 0.5V
plot(sram_mux_parasitic_0p5V(:,1), sram_mux_parasitic_0p5V(:,4)/1e-6,'c-s','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM MUX VDD = 0.7V
plot(sram_mux_parasitic_0p7V(:,1), sram_mux_parasitic_0p7V(:,4)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.5V
plot(rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,5)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.6V
plot(rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,5)/1e-6,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.7V
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,5)/1e-6,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[15 60],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.5V)'}, {'CMOS MUX (V_{DD}=0.7V)'}, {'Improved 4T1R MUX (V_{DD}=0.5V)'}, {'Improved 4T1R MUX (V_{DD}=0.6V)'}, {'Improved 4T1R MUX  (V_{DD}=0.7V)'}]);
set(fig_handle2_1, 'Position', [1 1 650 500]);
grid on


