%% A Clear Start
clear all;
close all;

%% Data
mux_size_list = 2:2:50;
mux_list_area_comp = [{'SRAM MUX'}, {'RRAM MUX (1-level)'}, {'RRAM MUX (2-level)'}, {'RRAM MUX (tree-like)'}];
mux_list_delay_comp = [{'SRAM MUX V_{DD}=0.5V'}, {'SRAM MUX V_{DD}=0.6V'}, {'SRAM MUX V_{DD}=0.7V'}];

%% import data
%% SRAM MUX
sram_mux_fig_data

%% circuit design with isolating transistors
%rram_mux_isolate_one_level_fig_data
%rram_mux_isolate_two_level_fig_data
%rram_mux_isolate_multi_level_fig_data

%% non-isolate design
rram_mux_improv_1level_fig_data
rram_mux_improv_2level_fig_data
rram_mux_improv_tree_fig_data

%% Improved design with 2N1R structure  
rram_mux_improv_2n1r_1level_fig_data
rram_mux_improv_2n1r_1level_0p5V_1fin = rram_mux_improv_2n1r_1level_0p5V(1:3:end,:); 
rram_mux_improv_2n1r_1level_0p5V_2fin = rram_mux_improv_2n1r_1level_0p5V(2:3:end,:); 
rram_mux_improv_2n1r_1level_0p5V_3fin = rram_mux_improv_2n1r_1level_0p5V(3:3:end,:); 
rram_mux_improv_2n1r_1level_0p6V_1fin = rram_mux_improv_2n1r_1level_0p6V(1:3:end,:); 
rram_mux_improv_2n1r_1level_0p6V_2fin = rram_mux_improv_2n1r_1level_0p6V(2:3:end,:); 
rram_mux_improv_2n1r_1level_0p6V_3fin = rram_mux_improv_2n1r_1level_0p6V(3:3:end,:); 
rram_mux_improv_2n1r_1level_0p7V_1fin = rram_mux_improv_2n1r_1level_0p7V(1:3:end,:); 
rram_mux_improv_2n1r_1level_0p7V_2fin = rram_mux_improv_2n1r_1level_0p7V(2:3:end,:); 
rram_mux_improv_2n1r_1level_0p7V_3fin = rram_mux_improv_2n1r_1level_0p7V(3:3:end,:); 
% 2-level structure 
rram_mux_improv_2n1r_2level_fig_data
rram_mux_improv_2n1r_2level_0p5V_1fin = rram_mux_improv_2n1r_2level_0p5V(1:3:end,:); 
rram_mux_improv_2n1r_2level_0p5V_2fin = rram_mux_improv_2n1r_2level_0p5V(2:3:end,:); 
rram_mux_improv_2n1r_2level_0p5V_3fin = rram_mux_improv_2n1r_2level_0p5V(3:3:end,:); 
rram_mux_improv_2n1r_2level_0p6V_1fin = rram_mux_improv_2n1r_2level_0p6V(1:3:end,:); 
rram_mux_improv_2n1r_2level_0p6V_2fin = rram_mux_improv_2n1r_2level_0p6V(2:3:end,:); 
rram_mux_improv_2n1r_2level_0p6V_3fin = rram_mux_improv_2n1r_2level_0p6V(3:3:end,:); 
rram_mux_improv_2n1r_2level_0p7V_1fin = rram_mux_improv_2n1r_2level_0p7V(1:3:end,:); 
rram_mux_improv_2n1r_2level_0p7V_2fin = rram_mux_improv_2n1r_2level_0p7V(2:3:end,:); 
rram_mux_improv_2n1r_2level_0p7V_3fin = rram_mux_improv_2n1r_2level_0p7V(3:3:end,:); 
% tree-like structure
rram_mux_improv_2n1r_tree_fig_data
rram_mux_improv_2n1r_tree_0p5V_1fin = rram_mux_improv_2n1r_tree_0p5V(1:3:end,:); 
rram_mux_improv_2n1r_tree_0p5V_2fin = rram_mux_improv_2n1r_tree_0p5V(2:3:end,:); 
rram_mux_improv_2n1r_tree_0p5V_3fin = rram_mux_improv_2n1r_tree_0p5V(3:3:end,:); 
rram_mux_improv_2n1r_tree_0p6V_1fin = rram_mux_improv_2n1r_tree_0p6V(1:3:end,:); 
rram_mux_improv_2n1r_tree_0p6V_2fin = rram_mux_improv_2n1r_tree_0p6V(2:3:end,:); 
rram_mux_improv_2n1r_tree_0p6V_3fin = rram_mux_improv_2n1r_tree_0p6V(3:3:end,:); 
rram_mux_improv_2n1r_tree_0p7V_1fin = rram_mux_improv_2n1r_tree_0p7V(1:3:end,:); 
rram_mux_improv_2n1r_tree_0p7V_2fin = rram_mux_improv_2n1r_tree_0p7V(2:3:end,:); 
rram_mux_improv_2n1r_tree_0p7V_3fin = rram_mux_improv_2n1r_tree_0p7V(3:3:end,:); 

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
%set(fig_handle1, 'Position', [1 1 650 500]);
%grid on

% Fig. 2.1, Delay Comparison 1 - SRAM MUX vs. RRAM MUX with isolated transistors, diff. structures
%fig_handle2_1 = figure;
%plot(sram_mux_0p7V(:,1), sram_mux_0p7V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p7V(:,1), rram_mux_isolate_one_level_0p9V(:,2)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_two_level_0p7V(:,1), rram_mux_isolate_two_level_0p9V(:,2)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_multi_level_0p7V(:,1), rram_mux_isolate_multi_level_0p9V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Delay (ps) and MUX size N','FontSize',18)
%xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'CMOS MUX'}, {'1-level 4T1R MUX with isolate transistors'}, {'2-level 4T1R MUX with isolate transistors'}, {'tree-like 4T1R MUX with isolate transistors'}]);
%set(fig_handle2_1, 'Position', [1 1 650 500]);
%grid on

% Fig. 2.2, Delay Comparison 2 - SRAM MUX vs. 1-level RRAM MUX with isolated transistors, diff. VDD
%fig_handle2_2 = figure;
%plot(sram_mux_0p5V(:,1), sram_mux_0p5V(:,2)/1e-12,'k-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(sram_mux_0p6V(:,1), sram_mux_0p6V(:,2)/1e-12,'k-s','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(sram_mux_0p7V(:,1), sram_mux_0p7V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p7V(:,1), rram_mux_isolate_one_level_0p7V(:,2)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p8V(:,1), rram_mux_isolate_one_level_0p8V(:,2)/1e-12,'b-s','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p9V(:,1), rram_mux_isolate_one_level_0p9V(:,2)/1e-12,'g-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Delay (ps) and MUX size N','FontSize',18)
%xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'CMOS MUX (V_{DD}=0.7V)'}, {'CMOS MUX (V_{DD}=0.8V)'}, {'CMOS MUX (V_{DD}=0.9V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.7V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.8V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.9V)'}]);
%set(fig_handle2_2, 'Position', [1 1 650 500]);
%grid on

% Fig. 3.1, Energy Comparison 1 - SRAM MUX vs. RRAM MUX with isolated transistors, diff. structures
%fig_handle3_1 = figure;
%plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,5)/1e-15,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p9V(:,1), rram_mux_isolate_one_level_0p9V(:,5)/1e-15,'r-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_two_level_0p9V(:,1), rram_mux_isolate_two_level_0p9V(:,5)/1e-15,'b-+','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_multi_level_0p9V(:,1), rram_mux_isolate_multi_level_0p9V(:,5)/1e-15,'g-s','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Energy (fJ) and MUX size N','FontSize',18)
%xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Energy (fJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'CMOS MUX'}, {'1-level 4T1R MUX with isolate transistors'}, {'2-level 4T1R MUX with isolate transistors'}, {'tree-like 4T1R MUX with isolate transistors'}]);
%set(fig_handle3_1, 'Position', [1 1 650 500]);
%grid on

% Fig. 3.2, Energy Comparison 2 - SRAM MUX vs. 1-level RRAM MUX with isolated transistors, diff. VDD
%fig_handle3_2 = figure;
%plot(sram_mux_0p7V(:,1), sram_mux_0p7V(:,5)/1e-15,'k-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(sram_mux_0p9V(:,1), sram_mux_0p9V(:,5)/1e-15,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p7V(:,1), rram_mux_isolate_one_level_0p7V(:,5)/1e-15,'r-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p8V(:,1), rram_mux_isolate_one_level_0p8V(:,5)/1e-15,'b-+','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p9V(:,1), rram_mux_isolate_one_level_0p9V(:,5)/1e-15,'g-s','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Energy (fJ) and MUX size N','FontSize',18)
%xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Energy (fJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'CMOS MUX (V_{DD}=0.7V)'}, {'CMOS MUX (V_{DD}=0.9V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.7V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.8V)'}, {'1-level 4T1R MUX with isolate transistors (V_{DD}=0.9V)'}]);
%set(fig_handle3_2, 'Position', [1 1 650 500]);
%grid on

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
%set(fig_handle4, 'Position', [1 1 650 500]);
%grid on

% Fig. 5.1, Area-Delay Comparison - SRAM MUX vs. RRAM MUX with isolating transistors
%fig_handle8 = figure;
%plot(sram_mux_0p9V(:,1), (area_sram_mux').*sram_mux_0p9V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p7V(:,1), (area_rram_mux_isolate_one_level').*rram_mux_isolate_one_level_0p7V(:,2)/1e-12,'b-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p8V(:,1), (area_rram_mux_isolate_one_level').*rram_mux_isolate_one_level_0p8V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p9V(:,1), (area_rram_mux_isolate_one_level').*rram_mux_isolate_one_level_0p9V(:,2)/1e-12,'r-+','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Area-Delay Product (M.W.T.A * ps) and MUX size N','FontSize',18)
%xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Area-Delay Product(M.W.T.A * ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'SRAM MUX (V_{DD}=0.9V)'}, {'4T1R MUX with isolating transistors (1-level, V_{DD}=0.7V)'}, {'4T1R MUX with isolating transistors (1-level, V_{DD}=0.8V)'},{'4T1R  MUX with with isolating transistors (1-level, V_{DD}=0.9V)'}]);
%set(fig_handle8, 'Position', [1 1 650 500]);
%grid on

% Fig. 5.2, Power-Delay Comparison - SRAM MUX vs. RRAM MUX with isolating transistors
%fig_handle9 = figure;
%plot(sram_mux_0p9V(:,1), (sram_mux_0p9V(:,2)/1e-12).*(sram_mux_0p9V(:,4)/1e-3),'k-o','LineWidth', 2, 'MarkerSize',10);
%plot(sram_mux_0p9V(:,1), (sram_mux_0p9V(:,5)/1e-15),'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p7V(:,1), (rram_mux_isolate_one_level_0p7V(:,2)/1e-12).*(rram_mux_isolate_one_level_0p7V(:,4)/1e-3),'b-*','LineWidth', 2, 'MarkerSize',10);
%plot(rram_mux_nonisolate_one_level_0p7V(:,1), (rram_mux_isolate_one_level_0p7V(:,5)/1e-15),'b-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p8V(:,1), (rram_mux_isolate_one_level_0p8V(:,2)/1e-12).*(rram_mux_isolate_one_level_0p7V(:,4)/1e-3),'r-+','LineWidth', 2, 'MarkerSize',10);
%plot(rram_mux_nonisolate_one_level_0p8V(:,1), (rram_mux_isolate_one_level_0p7V(:,5)/1e-15),'r-+','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux_isolate_one_level_0p9V(:,1), (rram_mux_isolate_one_level_0p9V(:,2)/1e-12).*(rram_mux_isolate_one_level_0p9V(:,4)/1e-3),'g-s','LineWidth', 2, 'MarkerSize',10);
%plot(rram_mux_nonisolate_one_level_0p9V(:,1), (rram_mux_isolate_one_level_0p9V(:,5)/1e-15),'g-s','LineWidth', 2, 'MarkerSize',10);
%hold on
%title('Power-Delay Product (fJ) and MUX size N','FontSize',18)
%xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Energy (Power-Delay Product) (fJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',mux_size_list);
%%set(gca,'XTickLabel',fc_in);
%%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'SRAM MUX (V_{DD}=0.9V)'}, {'4T1R MUX with isolating transistors (1-level, V_{DD}=0.7V)'}, {'4T1R MUX with isolating transistors (1-level, V_{DD}=0.8V)'},{'4T1R MUX with isolating transistors (1-level, V_{DD}=0.9V)'}]);
%set(fig_handle9, 'Position', [1 1 650 500]);
%grid on

% Fig. 6.1, Delay Comparison 1 - SRAM MUX vs. RRAM MUX , diff. structures
fig_handle6_1 = figure;
plot(sram_mux_0p7V(:,1), sram_mux_0p7V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p7V(:,1), rram_mux_nonisolate_one_level_0p7V(:,2)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_two_level_0p7V(:,1), rram_mux_nonisolate_two_level_0p7V(:,2)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_multi_level_0p7V(:,1), rram_mux_nonisolate_multi_level_0p7V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_2n1r_1level_0p7V_1fin(:,1), rram_mux_improv_2n1r_1level_0p7V_1fin(:,3)/1e-12,'m-^','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_2n1r_2level_0p7V_1fin(:,1), rram_mux_improv_2n1r_2level_0p7V_1fin(:,3)/1e-12,'c-<','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_2n1r_tree_0p7V_1fin(:,1), rram_mux_improv_2n1r_tree_0p7V_1fin(:,3)/1e-12,'y->','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'1-level 4T1R MUX'}, {'2-level 4T1R MUX'}, {'tree-like 4T1R MUX'}, {'1-level 2T1R MUX'}]);
set(fig_handle6_1, 'Position', [1 1 650 500]);
grid on

% Fig. 6.2, Delay Comparison 2 - SRAM MUX vs. 1-level RRAM MUX , diff. VDD
fig_handle6_2 = figure;
plot(sram_mux_0p5V(:,1), sram_mux_0p5V(:,2)/1e-12,'c-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(sram_mux_0p7V(:,1), sram_mux_0p7V(:,2)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p5V(:,1), rram_mux_nonisolate_one_level_0p5V(:,2)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p6V(:,1), rram_mux_nonisolate_one_level_0p6V(:,2)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p7V(:,1), rram_mux_nonisolate_one_level_0p7V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[15 60],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.5V)'}, {'CMOS MUX (V_{DD}=0.7V)'}, {'1-level 4T1R MUX (V_{DD}=0.5V)'}, {'1-level 4T1R MUX (V_{DD}=0.6V)'}, {'1-level 4T1R MUX  (V_{DD}=0.7V)'}]);
set(fig_handle6_2, 'Position', [1 1 650 500]);
grid on


% Fig. 7.1, Energy Comparison 1 - SRAM MUX vs. RRAM MUX , diff. structures
fig_handle7_1 = figure;
plot(sram_mux_0p7V(:,1), sram_mux_0p7V(:,2).*sram_mux_0p7V(:,4)/1e-15,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p7V(:,1), rram_mux_nonisolate_one_level_0p7V(:,2).*rram_mux_nonisolate_one_level_0p7V(:,4)/1e-15,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_two_level_0p7V(:,1), rram_mux_nonisolate_two_level_0p7V(:,2).*rram_mux_nonisolate_two_level_0p7V(:,4)/1e-15,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_multi_level_0p7V(:,1), rram_mux_nonisolate_multi_level_0p7V(:,2).*rram_mux_nonisolate_multi_level_0p7V(:,4)/1e-15,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_improv_2n1r_1level_0p7V_1fin(:,1), rram_mux_improv_2n1r_1level_0p7V_1fin(:,6)/1e-15,'m-^','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Energy (fJ) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Energy (Power-Delay Product) (fJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'1-level 4T1R MUX'}, {'2-level 4T1R MUX'}, {'tree-like 4T1R MUX'},{'1-level 2T1R MUX'}]);
set(fig_handle7_1, 'Position', [1 1 650 500]);
grid on

% Fig. 7.2, Power Comparison 2 - SRAM MUX vs. 1-level RRAM MUX , diff. VDD
fig_handle7_2 = figure;
plot(sram_mux_0p5V(:,1), sram_mux_0p5V(:,4)/1e-6,'c-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(sram_mux_0p7V(:,1), sram_mux_0p7V(:,4)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p5V(:,1), rram_mux_nonisolate_one_level_0p5V(:,4)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p6V(:,1), rram_mux_nonisolate_one_level_0p6V(:,4)/1e-6,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux_nonisolate_one_level_0p7V(:,1), rram_mux_nonisolate_one_level_0p7V(:,4)/1e-6,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power ({\mu}W) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.5V)'},{'CMOS MUX (V_{DD}=0.7V)'}, {'1-level 4T1R MUX (V_{DD}=0.5V)'}, {'1-level 4T1R MUX (V_{DD}=0.6V)'}, {'1-level 4T1R MUX(V_{DD}=0.7V)'}]);
set(fig_handle7_2, 'Position', [1 1 650 500]);
grid on

%% RRAM MUX with best PDP
for i = 2:2:50
  if (i < 24) 
    area_rram_mux_nonisolate(i/2) = area_rram_mux_nonisolate_one_level(i/2);
    delay_rram_mux_nonisolate_0p5V(i/2) = rram_mux_nonisolate_one_level_0p5V(i/2,2);
    delay_rram_mux_nonisolate_0p6V(i/2) = rram_mux_nonisolate_one_level_0p6V(i/2,2);
    delay_rram_mux_nonisolate_0p7V(i/2) = rram_mux_nonisolate_one_level_0p7V(i/2,2);
    power_rram_mux_nonisolate_0p5V(i/2) = rram_mux_nonisolate_one_level_0p5V(i/2,4);
    power_rram_mux_nonisolate_0p6V(i/2) = rram_mux_nonisolate_one_level_0p6V(i/2,4);
    power_rram_mux_nonisolate_0p7V(i/2) = rram_mux_nonisolate_one_level_0p7V(i/2,4);
  else 
    area_rram_mux_nonisolate(i/2) = area_rram_mux_nonisolate_two_level(i/2);
    delay_rram_mux_nonisolate_0p5V(i/2) = rram_mux_nonisolate_two_level_0p5V(i/2,2);
    delay_rram_mux_nonisolate_0p6V(i/2) = rram_mux_nonisolate_two_level_0p6V(i/2,2);
    delay_rram_mux_nonisolate_0p7V(i/2) = rram_mux_nonisolate_two_level_0p7V(i/2,2);
    power_rram_mux_nonisolate_0p5V(i/2) = rram_mux_nonisolate_two_level_0p5V(i/2,4);
    power_rram_mux_nonisolate_0p6V(i/2) = rram_mux_nonisolate_two_level_0p6V(i/2,4);
    power_rram_mux_nonisolate_0p7V(i/2) = rram_mux_nonisolate_two_level_0p7V(i/2,4);
  end 
end

% Fig. 8, Area-Delay Comparison - SRAM MUX vs. RRAM MUX 
adp_sram_mux_0p7V = (area_sram_mux').*sram_mux_0p7V(:,2)/1e-12;
adp_rram_mux_nonisolate_0p5V = area_rram_mux_nonisolate.*delay_rram_mux_nonisolate_0p5V/1e-12;
adp_rram_mux_nonisolate_0p6V = area_rram_mux_nonisolate.*delay_rram_mux_nonisolate_0p6V/1e-12;
adp_rram_mux_nonisolate_0p7V = area_rram_mux_nonisolate.*delay_rram_mux_nonisolate_0p7V/1e-12;
% Plot figure
fig_handle8 = figure;
plot(sram_mux_0p7V(:,1), adp_sram_mux_0p7V,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_nonisolate_one_level_0p5V(:,1), (area_rram_mux_nonisolate_one_level').*rram_mux_nonisolate_one_level_0p5V(:,2)/1e-12,'b-*','LineWidth', 2, 'MarkerSize',10);
plot(rram_mux_nonisolate_one_level_0p5V(:,1), adp_rram_mux_nonisolate_0p5V,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_nonisolate_one_level_0p6V(:,1), (area_rram_mux_nonisolate_one_level').*rram_mux_nonisolate_one_level_0p6V(:,2)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
plot(rram_mux_nonisolate_one_level_0p6V(:,1), adp_rram_mux_nonisolate_0p6V,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_nonisolate_one_level_0p7V(:,1), (area_rram_mux_nonisolate_one_level').*rram_mux_nonisolate_one_level_0p7V(:,2)/1e-12,'r-+','LineWidth', 2, 'MarkerSize',10);
plot(rram_mux_nonisolate_one_level_0p7V(:,1), adp_rram_mux_nonisolate_0p7V,'r-+','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Area-Delay Product (M.W.T.A * ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Area-Delay Product(M.W.T.A * ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'XTickLabel',fc_in);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.7V)'}, {'4T1R MUX (V_{DD}=0.5V)'}, {'4T1R MUX (V_{DD}=0.6V)'}, {'4T1R MUX(V_{DD}=0.7V)'}]);
set(fig_handle8, 'Position', [1 1 650 500]);
grid on

% Fig. 9, Power-Delay Comparison - SRAM MUX vs. RRAM MUX 
pdp_sram_mux_0p7V = (sram_mux_0p7V(:,2)/1e-12).*(sram_mux_0p7V(:,4)/1e-3);
pdp_rram_mux_nonisolate_0p5V = (delay_rram_mux_nonisolate_0p5V/1e-12).*(power_rram_mux_nonisolate_0p5V/1e-3);
pdp_rram_mux_nonisolate_0p6V = (delay_rram_mux_nonisolate_0p6V/1e-12).*(power_rram_mux_nonisolate_0p6V/1e-3);
pdp_rram_mux_nonisolate_0p7V = (delay_rram_mux_nonisolate_0p7V/1e-12).*(power_rram_mux_nonisolate_0p7V/1e-3);
% Plot figure
fig_handle9 = figure;
plot(sram_mux_0p7V(:,1), pdp_sram_mux_0p7V,'k-o','LineWidth', 2, 'MarkerSize',10);
%plot(sram_mux_0p7V(:,1), (sram_mux_0p7V(:,5)/1e-15),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_nonisolate_one_level_0p5V(:,1), (rram_mux_nonisolate_one_level_0p5V(:,2)/1e-12).*(rram_mux_nonisolate_one_level_0p5V(:,4)/1e-3),'b-*','LineWidth', 2, 'MarkerSize',10);
plot(rram_mux_nonisolate_one_level_0p5V(:,1), pdp_rram_mux_nonisolate_0p5V,'b-*','LineWidth', 2, 'MarkerSize',10);
%plot(rram_mux_nonisolate_one_level_0p5V(:,1), (rram_mux_nonisolate_one_level_0p5V(:,5)/1e-15),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_nonisolate_one_level_0p6V(:,1), (rram_mux_nonisolate_one_level_0p6V(:,2)/1e-12).*(rram_mux_nonisolate_one_level_0p6V(:,4)/1e-3),'r-+','LineWidth', 2, 'MarkerSize',10);
plot(rram_mux_nonisolate_one_level_0p6V(:,1), pdp_rram_mux_nonisolate_0p6V, 'r-+','LineWidth', 2, 'MarkerSize',10);
%plot(rram_mux_nonisolate_one_level_0p6V(:,1), (rram_mux_nonisolate_one_level_0p6V(:,5)/1e-15),'r-+','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(rram_mux_nonisolate_one_level_0p7V(:,1), (rram_mux_nonisolate_one_level_0p7V(:,2)/1e-12).*(rram_mux_nonisolate_one_level_0p7V(:,4)/1e-3),'g-s','LineWidth', 2, 'MarkerSize',10);
plot(rram_mux_nonisolate_one_level_0p7V(:,1), pdp_rram_mux_nonisolate_0p7V,'g-s','LineWidth', 2, 'MarkerSize',10);
%plot(rram_mux_nonisolate_one_level_0p7V(:,1), (rram_mux_nonisolate_one_level_0p7V(:,5)/1e-15),'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power-Delay Product (fJ) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Energy (Power-Delay Product) (fJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%%set(gca,'XTickLabel',fc_in);
%%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.7V)'}, {'4T1R MUX (V_{DD}=0.5V)'}, {'4T1R MUX (V_{DD}=0.6V)'}, {'4T1R MUX(V_{DD}=0.7V)'}]);
set(fig_handle9, 'Position', [1 1 650 500]);
grid on

% Fig. 10, Prog. Trans. Size
% Read data
wprog_list = [1 2 3];
rram_mux_nonisolate_sweep_wprog_fig_data
% Plot figure
fig_handle10 = figure;
%plot(rram_mux32_nonisolate_one_level_sweep_wprog_0p5V(:,2).*rram_mux32_nonisolate_one_level_sweep_wprog_0p5V(:,4)/1e-15,'r-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram_mux32_nonisolate_one_level_sweep_wprog_0p6V(:,2).*rram_mux32_nonisolate_one_level_sweep_wprog_0p6V(:,4)/1e-15,'r-s','LineWidth', 2, 'MarkerSize',10);
%hold on
plot(rram_mux32_nonisolate_one_level_sweep_wprog_0p7V(:,2).*rram_mux32_nonisolate_one_level_sweep_wprog_0p7V(:,4)/1e-15,'r-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux32_nonisolate_two_level_sweep_wprog_0p7V(:,2).*rram_mux32_nonisolate_two_level_sweep_wprog_0p7V(:,4)/1e-15,'b-s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram_mux32_nonisolate_multi_level_sweep_wprog_0p7V(:,2).*rram_mux32_nonisolate_multi_level_sweep_wprog_0p7V(:,4)/1e-15,'g-*','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power-Delay Product (fJ) and MUX size N','FontSize',18)
xlabel('Number of Fins','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Energy (Power-Delay Product)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',wprog_list);
%set(gca,'XTickLabel',wprog_list);
%%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'one-level 4T1R MUX'}]);
hleg = legend([{'1-level 4T1R MUX (V_{DD}=0.7V)'}, {'2-level 4T1R MUX (V_{DD}=0.7V)'}, {'tree-like 4T1R MUX (V_{DD}=0.7V)'}]);
set(fig_handle10, 'Position', [1 1 650 500]);
grid on


