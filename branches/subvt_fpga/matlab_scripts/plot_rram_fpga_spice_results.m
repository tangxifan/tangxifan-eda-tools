close all;
clear all;

%% Global parameters
mux_list = [{'SRAM MUX'}, {'4T1R MUX'}];
fpga_list = [{'SRAM FPGA'}, {'RRAM FPGA'}];
num_input_patterns = 4;
input_pattern_list = [{'in[0]=0, in[1]=0'}, {'in[0]=0, in[1]=V_{DD}'}, {'in[0]=V_{DD}, in[1]=0'}, {'in[0]=V_{DD}, in[1]=V_{DD}'}];
%rhrs_list = [{'10'}, {'20'}, {'30'}, {'40'}, {'50'}, {'60'}, {'70'}, {'80'}, {'90'}, {'100'}];
rhrs_list = 10:10:100;
chan_width_list = 50:50:300; 

%% Import figure data
mux_leakage_power_fig_data_tsmc40nm;

%% Process data
% MUXes
% Average leakage power 
average_sram_mux2_leakage = mean(leakage_sram_mux2) * ones(1,length(rhrs_list));
average_rram_mux2_leakage = mean(leakage_rram_mux2');
average_sram_mux2_tapbuf_leakage = mean(leakage_sram_mux2_tapbuf) * ones(1,length(rhrs_list));
average_rram_mux2_tapbuf_leakage = mean(leakage_rram_mux2_tapbuf');

% FPGA architecture
average_sram_fpga_leakage = leakage_sram_fpga(1,1,8)* ones(1, length(rhrs_list));
average_sram_fpga_dynamic = dynamic_sram_fpga(1,1,8)* ones(1, length(rhrs_list));
average_rram_fpga_leakage = leakage_rram_fpga(:,1,8);
average_rram_fpga_dynamic = dynamic_rram_fpga(:,1,8);
% normalize numbers 
for i=1:1:length(leakage_rram_fpga(:,1,8))
  normalized_rram_fpga_leakage(i,:,:) = leakage_rram_fpga(i,:,:)./leakage_sram_fpga(1,:,:);
  average_normalized_rram_fpga_leakage(i) = mean(normalized_rram_fpga_leakage(i,:,8));
end 
for i=1:1:length(dynamic_rram_fpga(:,1,8))
  normalized_rram_fpga_dynamic(i,:,:) = dynamic_rram_fpga(i,:,:)./dynamic_sram_fpga(1,:,:);
  average_normalized_rram_fpga_dynamic(i) = mean(normalized_rram_fpga_dynamic(i,:,8));
end 

%% Fig. 1: static power of MUX2 without tapered buffer
% Data format
xindex = 1:1:length(rhrs_list);
% Fig. plot
fig_handle1 = figure;
plot(average_sram_mux2_leakage.*1e9,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(average_rram_mux2_leakage.*1e9,'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}({M\Omega})','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Leakage Power (nW)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(rhrs_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',rhrs_list);
hleg = legend(mux_list);
grid on

%% Fig. 2: static power of MUX2 with tapered buffer
% Data format
xindex = 1:1:length(rhrs_list);
% Fig. plot
fig_handle2 = figure;
plot(average_sram_mux2_tapbuf_leakage.*1e9,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(average_rram_mux2_tapbuf_leakage.*1e9,'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}({M\Omega})','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Leakage Power (nW)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(rhrs_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',rhrs_list);
hleg = legend(mux_list);
grid on

%% Fig. 3: total power of FPGA 
% Data format
xindex = 1:1:length(rhrs_list);
% Fig. plot
fig_handle3 = figure;
%plot(average_sram_fpga_dynamic.*1e3,'b-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(average_rram_fpga_dynamic.*1e3,'r-s','LineWidth', 2, 'MarkerSize',10);
%hold on
plot(ones(1, length(average_normalized_rram_fpga_dynamic)),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(average_normalized_rram_fpga_dynamic,'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}({M\Omega})','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Dynamic Power (mW)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Normalized Energy Per Clock Cycle','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(rhrs_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',rhrs_list);
hleg = legend(fpga_list);
grid on

%% Fig. 4: full-chip area of FPGA 
% Data format
xindex = 1:1:length(area_layout_sram_fpga(:,1));
% Fig. plot
fig_handle4 = figure;
plot(area_layout_sram_fpga(:,4),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(area_layout_rram_fpga(:,4),'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('Channel Width','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Full Chip Area (\mu m^2)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(area_layout_sram_fpga(:,1))+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',area_layout_sram_fpga(:,1));
hleg = legend(fpga_list);
grid on

%% Fig. 5: standard cell area of FPGA 
% Data format
xindex = 1:1:length(area_layout_sram_fpga(:,1));
% Fig. plot
fig_handle5 = figure;
plot(area_layout_sram_fpga(:,2),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(area_layout_rram_fpga(:,2),'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('Channel Width','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Standard Cell Area (\mu m^2)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(area_layout_sram_fpga(:,1))+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',area_layout_sram_fpga(:,1));
hleg = legend(fpga_list);
grid on





