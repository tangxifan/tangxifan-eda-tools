close all;
clear all;

%% Global parameters
tag_list = [{'FPGA-SPICE Results'},{'VPR Results'}];
chan_width_list = 50:50:300; 

%% Import figure data
sram_fpga_spice_fig_data_tsmc40nm;

%% Process data
area_vpr_sram_fpga(:,4) = area_vpr_sram_fpga(:,2) + area_vpr_sram_fpga(:,3);
area_vpr_sram_fpga(:,4) = min_width_trans_area * area_vpr_sram_fpga(:,4);
area_vpr_sram_fpga(:,5) = 5 * area_vpr_sram_fpga(:,4);

%% Fig. 1: area comp: VPR vs. FPGA-SPICE 
% Data format
xindex = 1:1:length(area_layout_sram_fpga(:,1));
% Fig. plot
fig_handle1 = figure;
plot(area_layout_sram_fpga(:,4),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(area_vpr_sram_fpga(:,4),'r-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('Channwl Width','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Total Area ({\mu m ^2})','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(area_layout_sram_fpga(:,1))+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',area_layout_sram_fpga(:,1));
hleg = legend(tag_list);
grid on


