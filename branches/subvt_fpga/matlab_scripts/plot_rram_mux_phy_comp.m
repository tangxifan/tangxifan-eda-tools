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
rram_mux_naive_1level_parasitic_fig_data
rram_mux_basic_1level_parasitic_fig_data
rram_mux_improv_1level_parasitic_x0_fig_data
rram_mux_improv_1level_parasitic_x1_fig_data

%% Classify the data into different array
% Naive design
% VDD= 0.5V
rram_mux_naive_1level_parasitic_0p5V_3fin = rram_mux_naive_1level_parasitic_0p5V;
% VDD= 0.6V
rram_mux_naive_1level_parasitic_0p6V_3fin = rram_mux_naive_1level_parasitic_0p6V;
% VDD= 0.7V
rram_mux_naive_1level_parasitic_0p7V_3fin = rram_mux_naive_1level_parasitic_0p7V;
% Basic design 
% VDD= 0.5V
rram_mux_basic_1level_parasitic_0p5V_1fin = rram_mux_basic_1level_parasitic_0p5V(1:3:end,:);
rram_mux_basic_1level_parasitic_0p5V_2fin = rram_mux_basic_1level_parasitic_0p5V(2:3:end,:);
rram_mux_basic_1level_parasitic_0p5V_3fin = rram_mux_basic_1level_parasitic_0p5V(3:3:end,:);
% VDD= 0.6V
rram_mux_basic_1level_parasitic_0p6V_1fin = rram_mux_basic_1level_parasitic_0p6V(1:3:end,:);
rram_mux_basic_1level_parasitic_0p6V_2fin = rram_mux_basic_1level_parasitic_0p6V(2:3:end,:);
rram_mux_basic_1level_parasitic_0p6V_3fin = rram_mux_basic_1level_parasitic_0p6V(3:3:end,:);
% VDD= 0.7V
rram_mux_basic_1level_parasitic_0p7V_1fin = rram_mux_basic_1level_parasitic_0p7V(1:3:end,:);
rram_mux_basic_1level_parasitic_0p7V_2fin = rram_mux_basic_1level_parasitic_0p7V(2:3:end,:);
rram_mux_basic_1level_parasitic_0p7V_3fin = rram_mux_basic_1level_parasitic_0p7V(3:3:end,:);
% Improved design 
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
% x = 1, VDD=0.5V
rram_mux_improv_1level_parasitic_x1_0p5V_1fin = rram_mux_improv_1level_parasitic_x1_0p5V(1:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p5V_2fin = rram_mux_improv_1level_parasitic_x1_0p5V(2:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p5V_3fin = rram_mux_improv_1level_parasitic_x1_0p5V(3:3:end,:);
% x = 1, VDD=0.6V
rram_mux_improv_1level_parasitic_x1_0p6V_1fin = rram_mux_improv_1level_parasitic_x1_0p6V(1:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p6V_2fin = rram_mux_improv_1level_parasitic_x1_0p6V(2:3:end,:);
rram_mux_improv_1level_parasitic_x1_0p6V_3fin = rram_mux_improv_1level_parasitic_x1_0p6V(3:3:end,:);
% x = 1, VDD=0.7V
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
hleg = legend([{'Fin no.=1, V_{DD}=0.5V'}
               {'Fin no.=2, V_{DD}=0.5V'}
               {'Fin no.=3, V_{DD}=0.5V'}
               {'Fin no.=1, V_{DD}=0.6V'}
               {'Fin no.=2, V_{DD}=0.6V'}
               {'Fin no.=3, V_{DD}=0.6V'}
               {'Fin no.=1, V_{DD}=0.7V'}
               {'Fin no.=2, V_{DD}=0.7V'}
               {'Fin no.=3, V_{DD}=0.7V'}
              ]);
set(fig_handle0, 'Position', [1 1 650 500]);
grid on

% Fig. 0_1, Power comparsion with different fin number : Improved 4T1R MUX x=1
fig_handle0_1 = figure;
% Improved 4T1R MUX x=1 delay, VDD = 0.5V
plot(rram_mux_improv_1level_parasitic_x1_0p5V_1fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_1fin(:,5)/1e-6,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p5V_2fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_2fin(:,5)/1e-6,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,5)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, VDD = 0.6V
plot(rram_mux_improv_1level_parasitic_x1_0p6V_1fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_1fin(:,5)/1e-6,'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p6V_2fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_2fin(:,5)/1e-6,'b-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,5)/1e-6,'k-s','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, VDD = 0.7V
plot(rram_mux_improv_1level_parasitic_x1_0p7V_1fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_1fin(:,5)/1e-6,'r-^','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p7V_2fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_2fin(:,5)/1e-6,'b-^','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,5)/1e-6,'k-^','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'Fin no.=1, V_{DD}=0.5V'}
               {'Fin no.=2, V_{DD}=0.5V'}
               {'Fin no.=3, V_{DD}=0.5V'}
               {'Fin no.=1, V_{DD}=0.6V'}
               {'Fin no.=2, V_{DD}=0.6V'}
               {'Fin no.=3, V_{DD}=0.6V'}
               {'Fin no.=1, V_{DD}=0.7V'}
               {'Fin no.=2, V_{DD}=0.7V'}
               {'Fin no.=3, V_{DD}=0.7V'}
              ]);
set(fig_handle0_1, 'Position', [1 1 650 500]);
grid on

% Fig. 0_2, Power-Delay Product comparsion with different fin number : Improved 4T1R MUX x=1
fig_handle0_2 = figure;
% Improved 4T1R MUX x=1 delay, VDD = 0.5V
plot(rram_mux_improv_1level_parasitic_x1_0p5V_1fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_1fin(:,3).*rram_mux_improv_1level_parasitic_x1_0p5V_1fin(:,5)/1e-15,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p5V_2fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_2fin(:,3).*rram_mux_improv_1level_parasitic_x1_0p5V_2fin(:,5)/1e-15,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,3).*rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,5)/1e-15,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, VDD = 0.6V
plot(rram_mux_improv_1level_parasitic_x1_0p6V_1fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_1fin(:,3).*rram_mux_improv_1level_parasitic_x1_0p6V_1fin(:,5)/1e-15,'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p6V_2fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_2fin(:,3).*rram_mux_improv_1level_parasitic_x1_0p6V_2fin(:,5)/1e-15,'b-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,3).*rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,5)/1e-15,'k-s','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, VDD = 0.7V
plot(rram_mux_improv_1level_parasitic_x1_0p7V_1fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_1fin(:,3).*rram_mux_improv_1level_parasitic_x1_0p7V_1fin(:,5)/1e-15,'r-^','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p7V_2fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_2fin(:,3).*rram_mux_improv_1level_parasitic_x1_0p7V_2fin(:,5)/1e-15,'b-^','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,3).*rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,5)/1e-15,'k-^','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power-Delay Product (fJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'Fin no.=1, V_{DD}=0.5V'}
               {'Fin no.=2, V_{DD}=0.5V'}
               {'Fin no.=3, V_{DD}=0.5V'}
               {'Fin no.=1, V_{DD}=0.6V'}
               {'Fin no.=2, V_{DD}=0.6V'}
               {'Fin no.=3, V_{DD}=0.6V'}
               {'Fin no.=1, V_{DD}=0.7V'}
               {'Fin no.=2, V_{DD}=0.7V'}
               {'Fin no.=3, V_{DD}=0.7V'}
              ]);
set(fig_handle0_2, 'Position', [1 1 650 500]);
grid on

% Fig. 1, Delay Comparison: SRAM MUX vs. Basic 4T1R MUX, Improved 4T1R MUX x=0, Improved 4T1R MUX x=1
fig_handle1 = figure;
% SRAM MUX delay VDD=0.7V
%plot(sram_mux_parasitic_0p7V(:,1), sram_mux_parasitic_0p7V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
% Naive 4T1R MUX
plot(rram_mux_naive_1level_parasitic_0p7V_3fin(:,1), rram_mux_naive_1level_parasitic_0p7V_3fin(:,3)/1e-12,'m-^','LineWidth', 2, 'MarkerSize',10);
hold on
% Basic 4T1R MUX
%plot(rram_mux_basic_1level_parasitic_0p7V_3fin(:,1), rram_mux_basic_1level_parasitic_0p7V_3fin(:,3)/1e-12,'m-^','LineWidth', 2, 'MarkerSize',10);
%hold on
% Improved 4T1R MUX x=0 delay, RRAM is close to input inverters
plot(rram_mux_improv_1level_parasitic_x0_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x0_0p7V_3fin(:,3)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, RRAM is close to output inverters
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,3)/1e-12,'b-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
set(gca,'ylim',[8 23],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'Naive 4T1R MUX'}, {'Improved 4T1R MUX (x=0)'}, {'Improved 4T1R MUX (x=L)'}]);
set(fig_handle1, 'Position', [1 1 650 500]);
grid on

% Fig. 1-1, Power Comparison: SRAM MUX vs. Basic 4T1R MUX, Improved 4T1R MUX x=0, Improved 4T1R MUX x=1
fig_handle1_1 = figure;
% SRAM MUX delay VDD=0.7V
%plot(sram_mux_parasitic_0p7V(:,1), sram_mux_parasitic_0p7V(:,4)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
% Naive 4T1R MUX
plot(rram_mux_naive_1level_parasitic_0p7V_3fin(:,1), rram_mux_naive_1level_parasitic_0p7V_3fin(:,5)/1e-6,'m-^','LineWidth', 2, 'MarkerSize',10);
hold on
% Basic 4T1R MUX
%plot(rram_mux_basic_1level_parasitic_0p7V_3fin(:,1), rram_mux_basic_1level_parasitic_0p7V_3fin(:,5)/1e-6,'m-^','LineWidth', 2, 'MarkerSize',10);
%hold on
% Improved 4T1R MUX x=0 delay, RRAM is close to input inverters
plot(rram_mux_improv_1level_parasitic_x0_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x0_0p7V_3fin(:,5)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1 delay, RRAM is close to output inverters
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,5)/1e-6,'b-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'Naive 4T1R MUX'}, {'Improved 4T1R MUX (x=0)'}, {'Improved 4T1R MUX (x=L)'}]);
set(fig_handle1, 'Position', [1 1 650 500]);
grid on

% Fig. 2, Delay Comparison: SRAM MUX vs Improved 4T1R MUX under different VDD
fig_handle2 = figure;
% SRAM MUX VDD = 0.5V
plot(sram_mux_parasitic_0p5V(:,1), sram_mux_parasitic_0p5V(:,2)/1e-12,'c-s','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM MUX VDD = 0.7V
plot(sram_mux_parasitic_0p6V(:,1), sram_mux_parasitic_0p6V(:,2)/1e-12,'m-^','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM MUX VDD = 0.7V
plot(sram_mux_parasitic_0p7V(:,1), sram_mux_parasitic_0p7V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.5V
plot(rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,3)/1e-12,'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.6V
plot(rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,3)/1e-12,'g-^','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.7V
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,3)/1e-12,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[15 60],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.5V)'}, {'CMOS MUX (V_{DD}=0.6V)'}, {'CMOS MUX (V_{DD}=0.7V)'}, {'4T1R MUX (V_{DD}=0.5V)'}, {'4T1R MUX (V_{DD}=0.6V)'}, {'4T1R MUX (V_{DD}=0.7V)'}]);
set(fig_handle2, 'Position', [1 1 650 500]);
grid on

% Fig. 2-1, Power Comparison: SRAM MUX vs Improved 4T1R MUX under different VDD
fig_handle2_1 = figure;
% SRAM MUX VDD = 0.5V
plot(sram_mux_parasitic_0p5V(:,1), sram_mux_parasitic_0p5V(:,4)/1e-6,'c-s','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM MUX VDD = 0.6V
plot(sram_mux_parasitic_0p6V(:,1), sram_mux_parasitic_0p6V(:,4)/1e-6,'m-^','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM MUX VDD = 0.7V
plot(sram_mux_parasitic_0p7V(:,1), sram_mux_parasitic_0p7V(:,4)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.5V
plot(rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p5V_3fin(:,5)/1e-6,'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.6V
plot(rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p6V_3fin(:,5)/1e-6,'g-^','LineWidth', 2, 'MarkerSize',10);
hold on
% Improved 4T1R MUX x=1, VDD=0.7V
plot(rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,1), rram_mux_improv_1level_parasitic_x1_0p7V_3fin(:,5)/1e-6,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[15 60],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.5V)'}, {'CMOS MUX (V_{DD}=0.6V)'}, {'CMOS MUX (V_{DD}=0.7V)'}, {'4T1R MUX (V_{DD}=0.5V)'}, {'4T1R MUX (V_{DD}=0.6V)'}, {'4T1R MUX (V_{DD}=0.7V)'}]);
set(fig_handle2_1, 'Position', [1 1 650 500]);
grid on


