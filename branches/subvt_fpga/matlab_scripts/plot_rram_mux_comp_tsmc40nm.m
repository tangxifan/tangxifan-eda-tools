%% A Clear Start
clear all;
close all;

%% Data
mux_size_list = 2:2:50;
wprog_list = 1:0.2:3;
mux_list_area_comp = [{'SRAM MUX'}, {'RRAM MUX (1-level)'}, {'RRAM MUX (2-level)'}, {'RRAM MUX (tree-like)'}];
mux_list_delay_comp = [{'SRAM MUX V_{DD}=0.7V'}, {'SRAM MUX V_{DD}=0.8V'}, {'SRAM MUX V_{DD}=0.9V'}];

%% import data
%% SRAM MUX
% one-level
sram_mux_1level_fig_data_tsmc40nm

% two-level
sram_mux_2level_fig_data_tsmc40nm

%% Improved design
rram_mux_improv_1level_fig_data_tsmc40nm
rram_mux_improv_2level_fig_data_tsmc40nm
rram_mux_improv_tree_fig_data_tsmc40nm

% Naive 4T1R MUX
rram_mux_naive_1level_fig_data_tsmc40nm

%% Post-processing data
% find the best SRAM MUX and RRAM MUX in terms of delay, power and PDP respectively
% SRAM MUX 
num_swept_input_size = 25;
num_merits = 5;
num_VDD = 3;
sram_mux_best_delay = zeros(num_merits, num_swept_input_size, num_VDD);
sram_mux_best_power = zeros(num_merits, num_swept_input_size, num_VDD);
sram_mux_best_pdp = zeros(num_merits, num_swept_input_size, num_VDD);
% Determine the best SRAM MUX
for isize = 1:1:num_swept_input_size 
  for iVDD = 1:1:num_VDD
    % In terms of best delay 
    % Form a matrix comparing 1-level and 2-level performance
    temp = [sram_mux_1level_best_delay(:,isize,iVDD)'; 
			sram_mux_2level_best_delay(:,isize,iVDD)';]; 
    [best_delay, best_delay_index] = min(temp(:,2)); % the 3rd column is the delay 
    sram_mux_best_delay(:, isize, iVDD) = temp(best_delay_index,:)';
    % In terms of best power
    % Form a matrix comparing 1-level and 2-level performance
    temp = [sram_mux_1level_best_power(:,isize,iVDD)'; 
			sram_mux_2level_best_power(:,isize,iVDD)';]; 
    [best_power, best_power_index] = min(temp(:,4)); % the 5th column is the power
    sram_mux_best_power(:, isize, iVDD) = temp(best_power_index,:)';
    % In terms of best pdp
    % Form a matrix comparing 1-level and 2-level performance
    temp = [sram_mux_1level_best_pdp(:,isize,iVDD)'; 
			sram_mux_2level_best_pdp(:,isize,iVDD)';]; 
    [best_pdp, best_pdp_index] = min(temp(:,5)); % the 6th column is the PDP 
    sram_mux_best_pdp(:, isize, iVDD) = temp(best_pdp_index,:)';
  end
end

% RRAM MUX 
num_swept_input_size = 25;
num_merits = 8;
num_VDD = 3;
rram_mux_best_delay = zeros(num_merits, num_swept_input_size, num_VDD);
rram_mux_best_power = zeros(num_merits, num_swept_input_size, num_VDD);
rram_mux_best_pdp = zeros(num_merits, num_swept_input_size, num_VDD);
% Determine the best SRAM MUX
for isize = 1:1:num_swept_input_size 
  for iVDD = 1:1:num_VDD
    % In terms of best delay 
    % Form a matrix comparing 1-level and 2-level performance
    temp = [rram_mux_improve_1level_best_delay(:,isize,iVDD)'; 
			rram_mux_improve_2level_best_delay(:,isize,iVDD)';]; 
    [best_delay, best_delay_index] = min(temp(:,3)); % the 3rd column is the delay 
    rram_mux_best_delay(:, isize, iVDD) = temp(best_delay_index,:)';
    % In terms of best power
    % Form a matrix comparing 1-level and 2-level performance
    temp = [rram_mux_improve_1level_best_power(:,isize,iVDD)'; 
			rram_mux_improve_2level_best_power(:,isize,iVDD)';]; 
    [best_power, best_power_index] = min(temp(:,5)); % the 5th column is the power
    rram_mux_best_power(:, isize, iVDD) = temp(best_power_index,:)';
    % In terms of best pdp
    % Form a matrix comparing 1-level and 2-level performance
    temp = [rram_mux_improve_1level_best_pdp(:,isize,iVDD)'; 
			rram_mux_improve_2level_best_pdp(:,isize,iVDD)';]; 
    [best_pdp, best_pdp_index] = min(temp(:,6)); % the 6th column is the PDP
    rram_mux_best_pdp(:, isize, iVDD) = temp(best_pdp_index,:)';
  end
end

%% Fig. 1 is reserved for transient analysis waveforms

%% Fig. 2, Delay of 1-level, 2-level, tree-like RRAM and best CMOS MUX (16-input) w.r.t Wprog, VDD = 0.9V
fig_handle2 = figure;
% best SRAM MUX, VDD = 0.9V, in terms of delay
%plot(wprog_list, sram_mux_best_delay(2,8,3)/1e-12 * ones(length(wprog_list)),'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
% 1-level 4T1R MUX, VDD = 0.9V, input size = 16   
plot(wprog_list, rram_mux_improve_1level_xl(:,3,25,3)/max(rram_mux_improve_1level_xl(:,3,25,3)),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% 2-level 4T1R MUX, VDD = 0.9V, input size = 16   
plot(wprog_list, rram_mux_improve_2level_xl(:,3,25,3)/max(rram_mux_improve_2level_xl(:,3,25,3)),'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
% tree-level 4T1R MUX, VDD = 0.9V, input size = 16   
plot(wprog_list, rram_mux_improve_tree_xl(:,3,25,3)/max(rram_mux_improve_tree_xl(:,3,25,3)),'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and Wprog','FontSize',18)
xlabel('W_{prog} (Minimum Transistor Width)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Normalized Delay','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',wprog_list);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'Improv. 1-level 4T1R MUX'}, {'Improv. 2-level 4T1R MUX'}, {'Improv. tree-like 4T1R MUX'}]);
set(fig_handle2, 'Position', [1 1 650 500]);
grid on

%% Fig. 3, Power of 1-level, 2-level, tree-like RRAM and best CMOS MUX (16-input) w.r.t Wprog, VDD = 0.9V
fig_handle3 = figure;
% best SRAM MUX, VDD = 0.9V, in terms of delay
plot(wprog_list, sram_mux_best_delay(4,8,3)/1e-6 * ones(1,length(wprog_list)), 'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.9V, input size = 16   
plot(wprog_list, rram_mux_improve_1level_xl(:,5,8,3)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% 2-level 4T1R MUX, VDD = 0.9V, input size = 16   
plot(wprog_list, rram_mux_improve_2level_xl(:,5,8,3)/1e-6,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
% tree-level 4T1R MUX, VDD = 0.9V, input size = 16   
plot(wprog_list, rram_mux_improve_tree_xl(:,5,8,3)/1e-6,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power (uW) and Wprog','FontSize',18)
xlabel('W_{prog} (Minimum Transistor Width)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power (\mu W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',wprog_list);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'Improv. 1-level 4T1R MUX'}, {'Improv. 2-level 4T1R MUX'}, {'Improv. tree-like 4T1R MUX'}]);
set(fig_handle3, 'Position', [1 1 650 500]);
grid on

%% Fig. 4, Best Wprog - best SRAM MUX vs. Optimized 4T1R MUX , 1-level/2-level/tree-like structures and Naive 4T1R MUX, 2N1R MUX
%  Wprog,opt, x = L, VDD = 0.9V 
fig_handle4 = figure;
% 1-level 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(2,:,3),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% 2-level 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_2level_best_delay(2,:,3),'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
% tree-like 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_tree_best_delay(2,:,3),'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('W_{prog} (Minimum Transistor Width)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'Improv. 1-level 4T1R MUX'}, {'Improv. 2-level 4T1R MUX'}, {'Improv. tree-like 4T1R MUX'}]);
set(fig_handle4, 'Position', [1 1 650 500]);
grid on

%% Fig. 5, Best Wprog - best SRAM MUX vs. Optimized 4T1R MUX , 1-level/2-level/tree-like structures and Naive 4T1R MUX, 2N1R MUX
%  Wprog,opt, x = L, VDD = 0.9V 
fig_handle5 = figure;
% 1-level 4T1R MUX, VDD = 0.7V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(2,:,1),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.8V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(2,:,2),'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(2,:,3),'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('W_{prog} (Minimum Transistor Width)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'Improv. 1-level 4T1R MUX (V_{DD}=0.7V)'}, {'Improv. 1-level 4T1R MUX (V_{DD}=0.8V)'}, {'Improv. 1-level 4T1R MUX (V_{DD}=0.9V)'}]);
set(fig_handle5, 'Position', [1 1 650 500]);
grid on

%% Fig. 6, Best X - best SRAM MUX vs. Optimized 4T1R MUX , 1-level/2-level structures
fig_handle6 = figure;
% best SRAM MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, sram_mux_best_delay(2,:,3)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.9V, in terms of delay, X = 0
plot(mux_size_list, rram_mux_improve_1level_x0_best_delay(3,:,3)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.9V, in terms of delay, X = L
plot(mux_size_list, rram_mux_improve_1level_xl_best_delay(3,:,3)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
% 2-level 4T1R MUX, VDD = 0.9V, in terms of delay, X = 0
plot(mux_size_list, rram_mux_improve_2level_x0_best_delay(3,:,3)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
% 2-level 4T1R MUX, VDD = 0.9V, in terms of delay, X = L
plot(mux_size_list, rram_mux_improve_2level_xl_best_delay(3,:,3)/1e-12,'m-<','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'Improv. 1-level 4T1R MUX (x=0)'}, {'Improv. 1-level 4T1R MUX (x=L)'}, {'Improv. 2-level 4T1R MUX (x=0)'}, {'Improv. 2-level 4T1R MUX (x=L)'}]);
set(fig_handle6, 'Position', [1 1 650 500]);
grid on

 
%% Fig. 7 is reserved for layout of SRAM and RRAM MUX

%% Fig. 8, Delay Comparison - best SRAM MUX vs. Optimized 4T1R MUX , 1-level/2-level/tree-like structures and Naive 4T1R MUX, 2N1R MUX
%  Wprog,opt, x = L, VDD = 0.9V 
fig_handle8 = figure;
% best SRAM MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, sram_mux_best_delay(2,:,3)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(3,:,3)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% 2-level 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_2level_best_delay(3,:,3)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
% tree-like 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_tree_best_delay(3,:,3)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
% Naive 1-level 4T1R MUX, VDD=0.9V, in terms of delay
plot(mux_size_list, rram_mux_naive_1level_0p9V(:,5)/1e-12,'m-^','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'Improv. 1-level 4T1R MUX'}, {'Improv. 2-level 4T1R MUX'}, {'Improv. tree-like 4T1R MUX'}, {'Naive 1-level 4T1R MUX'}]);
set(fig_handle8, 'Position', [1 1 650 500]);
grid on

% Fig. 9, Delay Comparison 2 - SRAM MUX vs. 1-level RRAM MUX , diff. VDD
fig_handle9 = figure;
% best SRAM MUX, VDD = 0.7V, in terms of delay
plot(mux_size_list, sram_mux_best_delay(2,:,1)/1e-12,'c-s','LineWidth', 2, 'MarkerSize',10);
hold on
% best SRAM MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, sram_mux_best_delay(2,:,3)/1e-12,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.7V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(3,:,1)/1e-12,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.8V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(3,:,2)/1e-12,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(3,:,3)/1e-12,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay (ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'ylim',[15 60],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.7V)'}, {'CMOS MUX (V_{DD}=0.9V)'}, {'1-level 4T1R MUX (V_{DD}=0.7V)'}, {'1-level 4T1R MUX (V_{DD}=0.8V)'}, {'1-level 4T1R MUX (V_{DD}=0.9V)'}]);
set(fig_handle9, 'Position', [1 1 650 500]);
grid on


% Fig. 10, Energy Comparison 1 - SRAM MUX vs. RRAM MUX , diff. structures
fig_handle10 = figure;
% best SRAM MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, sram_mux_best_delay(2,:,3).* sram_mux_best_delay(4,:,3)/1e-15,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(3,:,3).*rram_mux_improve_1level_best_delay(5,:,3)/1e-15,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% 2-level 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_2level_best_delay(3,:,3).*rram_mux_improve_2level_best_delay(5,:,3)/1e-15,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
% tree-like 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_tree_best_delay(3,:,3).*rram_mux_improve_tree_best_delay(5,:,3)/1e-15,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
% Naive 1-level 4T1R MUX, VDD=0.9V, in terms of delay
plot(mux_size_list, rram_mux_naive_1level_0p9V(:,5).*rram_mux_naive_1level_0p9V(:,7)/1e-15,'c-<','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Energy (fJ) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Energy (Power-Delay Product) (fJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'ylim',[0 0.2],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX'}, {'Improv. 1-level 4T1R MUX'}, {'Improv. 2-level 4T1R MUX'}, {'Improv. tree-like 4T1R MUX'},{'Naive 1-level 4T1R MUX'}]);
set(fig_handle10, 'Position', [1 1 650 500]);
grid on

% Fig. 11, Power Comparison 2 - SRAM MUX vs. 1-level RRAM MUX , diff. VDD
fig_handle11 = figure;
% best SRAM MUX, VDD = 0.7V, in terms of delay
plot(mux_size_list, sram_mux_best_delay(4,:,1)/1e-6,'c-s','LineWidth', 2, 'MarkerSize',10);
hold on
% best SRAM MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, sram_mux_best_delay(4,:,3)/1e-6,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.7V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(5,:,1)/1e-6,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.8V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(5,:,2)/1e-6,'b-+','LineWidth', 2, 'MarkerSize',10);
hold on
% 1-level 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, rram_mux_improve_1level_best_delay(5,:,3)/1e-6,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power ({\mu}W) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power ({\mu}W)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.7V)'},{'CMOS MUX (V_{DD}=0.9V)'}, {'1-level 4T1R MUX (V_{DD}=0.7V)'}, {'1-level 4T1R MUX (V_{DD}=0.8V)'}, {'1-level 4T1R MUX(V_{DD}=0.9V)'}]);
set(fig_handle11, 'Position', [1 1 650 500]);
grid on

% Fig. 12, Area-Delay Comparison - SRAM MUX vs. RRAM MUX 
adp_best_sram_mux_0p9V = sram_mux_best_delay(1,:,3).*sram_mux_best_delay(2,:,3)/1e-12;
adp_best_rram_mux_0p7V = rram_mux_best_delay(1,:,1).*rram_mux_best_delay(3,:,1)/1e-12;
adp_best_rram_mux_0p8V = rram_mux_best_delay(1,:,2).*rram_mux_best_delay(3,:,2)/1e-12;
adp_best_rram_mux_0p9V = rram_mux_best_delay(1,:,3).*rram_mux_best_delay(3,:,3)/1e-12;
% Plot figure
fig_handle12 = figure;
% best SRAM MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, adp_best_sram_mux_0p9V,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% best 4T1R MUX, VDD = 0.7V, in terms of delay
plot(mux_size_list, adp_best_rram_mux_0p7V,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
% best 4T1R MUX, VDD = 0.8V, in terms of delay
plot(mux_size_list, adp_best_rram_mux_0p8V,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
% best 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, adp_best_rram_mux_0p9V,'r-+','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Area-Delay Product (M.W.T.A * ps) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Area-Delay Product(M.W.T.A * ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.9V)'}, {'4T1R MUX (V_{DD}=0.7V)'}, {'4T1R MUX (V_{DD}=0.8V)'}, {'4T1R MUX(V_{DD}=0.9V)'}]);
set(fig_handle12, 'Position', [1 1 650 500]);
grid on

% Fig. 13, Power-Delay Comparison - SRAM MUX vs. RRAM MUX 
pdp_best_sram_mux_0p7V = (sram_mux_best_delay(2,:,1)/1e-12).*sram_mux_best_delay(4,:,1)/1e-3;
pdp_best_sram_mux_0p9V = (sram_mux_best_delay(2,:,3)/1e-12).*sram_mux_best_delay(4,:,3)/1e-3;
pdp_best_rram_mux_0p7V = (rram_mux_best_delay(3,:,1)/1e-12).*rram_mux_best_delay(5,:,1)/1e-3;
pdp_best_rram_mux_0p8V = (rram_mux_best_delay(3,:,2)/1e-12).*rram_mux_best_delay(5,:,2)/1e-3;
pdp_best_rram_mux_0p9V = (rram_mux_best_delay(3,:,3)/1e-12).*rram_mux_best_delay(5,:,3)/1e-3;
% Plot figure
fig_handle13 = figure;
% best SRAM MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, pdp_best_sram_mux_0p7V,'m-<','LineWidth', 2, 'MarkerSize',10);
hold on
% best SRAM MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, pdp_best_sram_mux_0p9V,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% best 4T1R MUX, VDD = 0.7V, in terms of delay
plot(mux_size_list, pdp_best_rram_mux_0p7V,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
% best 4T1R MUX, VDD = 0.8V, in terms of delay
plot(mux_size_list, pdp_best_rram_mux_0p8V, 'r-+','LineWidth', 2, 'MarkerSize',10);
hold on
% best 4T1R MUX, VDD = 0.9V, in terms of delay
plot(mux_size_list, pdp_best_rram_mux_0p9V,'g-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Power-Delay Product (fJ) and MUX size N','FontSize',18)
xlabel('MUX size','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Energy (Power-Delay Product) (fJ)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.1 1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',mux_size_list);
%%set(gca,'XTickLabel',fc_in);
%%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'CMOS MUX (V_{DD}=0.7V)'}, {'CMOS MUX (V_{DD}=0.9V)'}, {'4T1R MUX (V_{DD}=0.7V)'}, {'4T1R MUX (V_{DD}=0.8V)'}, {'4T1R MUX(V_{DD}=0.9V)'}]);
set(fig_handle13, 'Position', [1 1 650 500]);
grid on

