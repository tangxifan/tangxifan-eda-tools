close all;
clear all;

roff_list = [10,50,100,500,1000;];
mux_list = [{'SRAM MUX'}, {'RRAM MUX'}, {'Sized RRAM MUX'};];

static_power_mux2 = [
{'10Meg'}, 8.78e-9, 67.98e-9, 61.16e-9;
{'50Meg'}, 8.78e-9, 28.02e-9, 21.2e-9;
{'100Meg'}, 8.78e-9, 23.02e-9, 16.2e-9;
{'500Meg'}, 8.78e-9, 19.02e-9, 12.21e-9;
{'1000Meg'}, 8.78e-9, 18.52e-9, 11.7e-9;
];

static_power_mux2_tapbuf = [
{'10Meg'}, 24.8e-9, 67e-9, 63.8e-9;
{'50Meg'}, 24.8e-9, 38.5e-9, 33.8e-9;
{'100Meg'}, 24.8e-9, 34.8e-9, 30.3e-9;
{'500Meg'}, 24.8e-9, 32e-9, 27.3e-9;
{'1000Meg'}, 24.8e-9, 31.5e-9, 27e-9;
];

avg_power_mux2_tapbuf = [
{'1'}, 12.45e-6, 13.67e-6, 13.56e-6;
{'10'}, 12.45e-6, 12.71e-6, 12.67e-6;
{'50'}, 12.45e-6, 12.70e-6, 12.60e-6;
{'100'}, 12.45e-6, 12.68e-6, 12.59e-6;
{'500'}, 12.45e-6, 12.68e-6, 12.58e-6;
{'1000'}, 12.45e-6, 12.68e-6, 12.58e-6;
];

avg_power_fpga = [
{'1'}, 1, 50.21e-2, 2.4867, 2.3318;
{'10'}, 1, 50.21e-2, 73.33e-2, 68.28e-2;
{'20'}, 1, 50.21e-2, 64.45e-2, 58.21e-2;
{'50'}, 1, 50.21e-2, 58.44e-2, 52.62e-2;
{'100'}, 1, 50.21e-2, 56.64e-2, 51.34e-2;
];

%% Fig. 1: static power MUX2 without tapered buffer
% Data format
static_power_mux2_format = cell2mat(static_power_mux2(:,(2:4)));
xindex = 1:1:length(roff_list);
% Fig. plot
fig_handle1 = figure;
plot(static_power_mux2_format(:,1).*1e9,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(static_power_mux2_format(:,2).*1e9,'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(static_power_mux2_format(:,3).*1e9,'r-.s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}({M\Omega})','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Static Power (nW)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(roff_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',roff_list);
hleg = legend(mux_list);
grid on

%% Fig. 2: static power MUX2 with tapered buffer
% Data format
static_power_mux2_tapbuf_format = cell2mat(static_power_mux2_tapbuf(:,(2:4)));
% Fig. plot
fig_handle2 = figure;
plot(static_power_mux2_tapbuf_format(:,1).*1e9,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(static_power_mux2_tapbuf_format(:,2).*1e9,'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(static_power_mux2_tapbuf_format(:,3).*1e9,'r-.s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}(M{\Omega})','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Static Power (nW)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(roff_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',roff_list);
hleg = legend(mux_list);
grid on

%% Fig. 3: average power MUX2 with tapered buffer
% Data format
avg_power_mux2_tapbuf_format = cell2mat(avg_power_mux2_tapbuf(:,(2:4)));
% Fig. plot
fig_handle3 = figure;
xindex = 1:1:length(avg_power_mux2_tapbuf(:,1));
plot(avg_power_mux2_tapbuf_format(:,1).*1e6,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(avg_power_mux2_tapbuf_format(:,2).*1e6,'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(avg_power_mux2_tapbuf_format(:,3).*1e6,'r-.s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}(M{\Omega})','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Average Power ({\mu}W)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(avg_power_mux2_tapbuf(:,1))+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick', xindex);
set(gca,'XTickLabel', avg_power_mux2_tapbuf(:,1));
hleg = legend(mux_list);
grid on

%% Fig. 4, FPGA average power 
% Data format
fpga_arch_list = [{'SRAM FPGA,vdd=1.0V'}, {'SRAM FPGA,vdd=0.85V'}, {'RRAM FPGA,vdd=0.85V'},{'Sized RRAM FPGA,vdd=0.85V'};];
avg_power_fpga_format = cell2mat(avg_power_fpga(:,(2:5)));
 
% Fig. plot
fig_handle4 = figure;
xindex = 1:1:length(avg_power_fpga(:,1));
plot(avg_power_fpga_format(:,1).*1e2,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(avg_power_fpga_format(:,2).*1e2,'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(avg_power_fpga_format(:,3).*1e2,'r-.s','LineWidth', 2, 'MarkerSize',10);
hold on
plot(avg_power_fpga_format(:,4).*1e2,'r-x','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}(M{\Omega})','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Normalized Average Power','FontSize',20, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(avg_power_fpga(:,1))+0.5],'Fontsize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'ylim',[45 105],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
yticks = [get(gca,'ytick')]'; % There is a transpose operation here.
percentsy = repmat('%', length(yticks),1);  %  equal to the size
yticklabel = [num2str(yticks) percentsy]; % concatenates the tick labels
set(gca,'YtickLabel',yticklabel)% Sets tick labels back on the Axis
set(gca,'XTick', xindex);
set(gca,'XTickLabel', avg_power_fpga(:,1));
hleg = legend(fpga_arch_list);
grid on
%
%%
static_power_mux32 = [
% RHRS(Omega) = 10M, 20M, 50M, 100M, 200M 
% Wprog = 1
2.1e-6, 1.14e-6, 562e-9, 368e-9, 271e-9;
262e-9, 262e-9, 262e-9,262e-9, 262e-9;
% Wprog = 2
];
rhrs_list = [{'10'},{'20'},{'50'},{'100'},{'200'}];
mux_list = [{'4T1R MUX'},{'SRAM MUX'}];

% Fig. plot
fig_handle5 = figure;
xindex = 1:1:length(static_power_mux32(1,:));
plot(static_power_mux32(1,:)/1e-9,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(static_power_mux32(2,:)/1e-9,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('R_{HRS}(M{\Omega})','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Leakage Power (nW)','FontSize',20, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(static_power_mux32(1,:))+0.5],'Fontsize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'ylim',[250 2100],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%yticks = [get(gca,'ytick')]'; % There is a transpose operation here.
%percentsy = repmat('%', length(yticks),1);  %  equal to the size
%yticklabel = [num2str(yticks) percentsy]; % concatenates the tick labels
%set(gca,'YtickLabel',yticklabel)% Sets tick labels back on the Axis
set(gca,'XTick', xindex);
set(gca,'XTickLabel', rhrs_list);
hleg = legend(mux_list);
grid on

%% Area vs. wprog
wprog_list = 1:0.1:1.9;
pn_ratio = 1.9;
prog_pn_ratio = 3;
mux_size = 1;
sram_area = 6;
area_io_std = 1.4; % area overhead of a I/O transistor compared to a standard logic transistor
mux_list = [{'4T1R cell'},{'SRAM cell'}];
for i = 1:1:length(wprog_list)
  area_rram_multiplexing = (mux_size+1)*area_io_std*(trans_area(wprog_list(i))+trans_area(prog_pn_ratio*wprog_list(i)));
  area_sram_multiplexing = (mux_size) * (trans_area(1)+ trans_area(1*pn_ratio));
  area_srams = sram_area * mux_size;
  if ( mux_size > 12) 
    area_sram_multiplexing = area_sram_multiplexing + ceil(sqrt(mux_size)) * (trans_area(1)+ trans_area(1*pn_ratio));
    area_srams = sram_area * ceil(sqrt(mux_size));
  end 
  area_buf = (mux_size + 1)*(trans_area(1)+trans_area(1*pn_ratio));
  area_rram_mux(i) = (area_rram_multiplexing + 0*area_buf)/mux_size;
  area_sram_mux(i) = (area_sram_multiplexing + 0*area_buf + area_srams)/mux_size; 
end
% Fig. plot
fig_handle6 = figure;
xindex = 1:1:length(wprog_list);
plot(area_rram_mux,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(area_sram_mux,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('Wprog(1 Wprog = 320nm)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Area (#. of Min.Width Trans. Area)','FontSize',20, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'Fontsize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[min(wprog_list) max(wprog_list)],'Fontsize',18, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'ylim',[250 2100],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%yticks = [get(gca,'ytick')]'; % There is a transpose operation here.
%percentsy = repmat('%', length(yticks),1);  %  equal to the size
%yticklabel = [num2str(yticks) percentsy]; % concatenates the tick labels
%set(gca,'YtickLabel',yticklabel)% Sets tick labels back on the Axis
%set(gca,'XTick', xindex);
set(gca,'XTickLabel', wprog_list);
hleg = legend(mux_list);
grid on

