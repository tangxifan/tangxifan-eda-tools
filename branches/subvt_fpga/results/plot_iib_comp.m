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
