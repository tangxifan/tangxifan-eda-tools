clear all
close all

%% Import data
rram4t1r_fig_data
% TG-based 2t1r
rram2t1r_inv1_fig_data
rram2t1r_inv5_fig_data
rram2t1r_inv10_fig_data
rram2t1r_inv15_fig_data
rram2t1r_inv20_fig_data
% 2n1r
rram2n1r_inv1_fig_data
rram2n1r_inv5_fig_data
rram2n1r_inv10_fig_data
rram2n1r_inv15_fig_data
rram2n1r_inv20_fig_data

%% Process the data format
%% Tags
vprog_list = [{'V_{prog}=2.5V'},{'V_{prog}=2.6V'},{'V_{prog}=2.7V'},{'V_{prog}=2.8V'},{'V_{prog}=2.9V'},{'V_{prog}=3.0V'};];
tags_list = [{'W_{prog}(No. of Min. Size Trans)'},{'V_{ds1}(V)'},{'V_{ds2}(V)'},{'V_{TE}(V)'},{'V_{BE}(V)'},{'V_{RRAM}(V)'},{'I_{prog}({\mu}A)'},{'I_{prog}/W_{prog}({\mu}A)'},{'R_{on}(k{\Omega})'},{'Breakdown'};];
wprog_list = cell2mat(rram4t1r_vprog2p5V(:,1));
inv_list = [{'W_{inv}=1'},{'W_{inv}=5'},{'W_{inv}=10'},{'W_{inv}=15'},{'W_{inv}=20'}];
% RRAM 4T1R
rram4t1r_vprog2p5V_format = cell2mat(rram4t1r_vprog2p5V(:,(2:9)));
rram4t1r_vprog2p6V_format = cell2mat(rram4t1r_vprog2p6V(:,(2:9)));
rram4t1r_vprog2p7V_format = cell2mat(rram4t1r_vprog2p7V(:,(2:9)));
rram4t1r_vprog2p8V_format = cell2mat(rram4t1r_vprog2p8V(:,(2:9)));
rram4t1r_vprog2p9V_format = cell2mat(rram4t1r_vprog2p9V(:,(2:9)));
rram4t1r_vprog3p0V_format = cell2mat(rram4t1r_vprog3p0V(:,(2:9)));
% RRAM 2N1R inv=1
rram2n1r_vprog2p5V_inv1_format = cell2mat(rram2n1r_vprog2p5V_inv1(:,(2:9)));
rram2n1r_vprog2p6V_inv1_format = cell2mat(rram2n1r_vprog2p6V_inv1(:,(2:9)));
rram2n1r_vprog2p7V_inv1_format = cell2mat(rram2n1r_vprog2p7V_inv1(:,(2:9)));
rram2n1r_vprog2p8V_inv1_format = cell2mat(rram2n1r_vprog2p8V_inv1(:,(2:9)));
rram2n1r_vprog2p9V_inv1_format = cell2mat(rram2n1r_vprog2p9V_inv1(:,(2:9)));
rram2n1r_vprog3p0V_inv1_format = cell2mat(rram2n1r_vprog3p0V_inv1(:,(2:9)));
% RRAM 2N1R inv=5
rram2n1r_vprog2p5V_inv5_format = cell2mat(rram2n1r_vprog2p5V_inv5(:,(2:9)));
rram2n1r_vprog2p6V_inv5_format = cell2mat(rram2n1r_vprog2p6V_inv5(:,(2:9)));
rram2n1r_vprog2p7V_inv5_format = cell2mat(rram2n1r_vprog2p7V_inv5(:,(2:9)));
rram2n1r_vprog2p8V_inv5_format = cell2mat(rram2n1r_vprog2p8V_inv5(:,(2:9)));
rram2n1r_vprog2p9V_inv5_format = cell2mat(rram2n1r_vprog2p9V_inv5(:,(2:9)));
rram2n1r_vprog3p0V_inv5_format = cell2mat(rram2n1r_vprog3p0V_inv5(:,(2:9)));
% RRAM 2N1R inv=10
rram2n1r_vprog2p5V_inv10_format = cell2mat(rram2n1r_vprog2p5V_inv10(:,(2:9)));
rram2n1r_vprog2p6V_inv10_format = cell2mat(rram2n1r_vprog2p6V_inv10(:,(2:9)));
rram2n1r_vprog2p7V_inv10_format = cell2mat(rram2n1r_vprog2p7V_inv10(:,(2:9)));
rram2n1r_vprog2p8V_inv10_format = cell2mat(rram2n1r_vprog2p8V_inv10(:,(2:9)));
rram2n1r_vprog2p9V_inv10_format = cell2mat(rram2n1r_vprog2p9V_inv10(:,(2:9)));
rram2n1r_vprog3p0V_inv10_format = cell2mat(rram2n1r_vprog3p0V_inv10(:,(2:9)));
% RRAM 2N1R inv=15
rram2n1r_vprog2p5V_inv15_format = cell2mat(rram2n1r_vprog2p5V_inv15(:,(2:9)));
rram2n1r_vprog2p6V_inv15_format = cell2mat(rram2n1r_vprog2p6V_inv15(:,(2:9)));
rram2n1r_vprog2p7V_inv15_format = cell2mat(rram2n1r_vprog2p7V_inv15(:,(2:9)));
rram2n1r_vprog2p8V_inv15_format = cell2mat(rram2n1r_vprog2p8V_inv15(:,(2:9)));
rram2n1r_vprog2p9V_inv15_format = cell2mat(rram2n1r_vprog2p9V_inv15(:,(2:9)));
rram2n1r_vprog3p0V_inv15_format = cell2mat(rram2n1r_vprog3p0V_inv15(:,(2:9)));
% RRAM 2N1R inv=20
rram2n1r_vprog2p5V_inv20_format = cell2mat(rram2n1r_vprog2p5V_inv20(:,(2:9)));
rram2n1r_vprog2p6V_inv20_format = cell2mat(rram2n1r_vprog2p6V_inv20(:,(2:9)));
rram2n1r_vprog2p7V_inv20_format = cell2mat(rram2n1r_vprog2p7V_inv20(:,(2:9)));
rram2n1r_vprog2p8V_inv20_format = cell2mat(rram2n1r_vprog2p8V_inv20(:,(2:9)));
rram2n1r_vprog2p9V_inv20_format = cell2mat(rram2n1r_vprog2p9V_inv20(:,(2:9)));
rram2n1r_vprog3p0V_inv20_format = cell2mat(rram2n1r_vprog3p0V_inv20(:,(2:9)));
% RRAM 2T1R inv=1
rram2t1r_vprog2p5V_inv1_format = cell2mat(rram2t1r_vprog2p5V_inv1(:,(2:9)));
rram2t1r_vprog2p6V_inv1_format = cell2mat(rram2t1r_vprog2p6V_inv1(:,(2:9)));
rram2t1r_vprog2p7V_inv1_format = cell2mat(rram2t1r_vprog2p7V_inv1(:,(2:9)));
rram2t1r_vprog2p8V_inv1_format = cell2mat(rram2t1r_vprog2p8V_inv1(:,(2:9)));
rram2t1r_vprog2p9V_inv1_format = cell2mat(rram2t1r_vprog2p9V_inv1(:,(2:9)));
rram2t1r_vprog3p0V_inv1_format = cell2mat(rram2t1r_vprog3p0V_inv1(:,(2:9)));
% RRAM 2T1R inv=5
rram2t1r_vprog2p5V_inv5_format = cell2mat(rram2t1r_vprog2p5V_inv5(:,(2:9)));
rram2t1r_vprog2p6V_inv5_format = cell2mat(rram2t1r_vprog2p6V_inv5(:,(2:9)));
rram2t1r_vprog2p7V_inv5_format = cell2mat(rram2t1r_vprog2p7V_inv5(:,(2:9)));
rram2t1r_vprog2p8V_inv5_format = cell2mat(rram2t1r_vprog2p8V_inv5(:,(2:9)));
rram2t1r_vprog2p9V_inv5_format = cell2mat(rram2t1r_vprog2p9V_inv5(:,(2:9)));
rram2t1r_vprog3p0V_inv5_format = cell2mat(rram2t1r_vprog3p0V_inv5(:,(2:9)));
% RRAM 2T1R inv=10
rram2t1r_vprog2p5V_inv10_format = cell2mat(rram2t1r_vprog2p5V_inv10(:,(2:9)));
rram2t1r_vprog2p6V_inv10_format = cell2mat(rram2t1r_vprog2p6V_inv10(:,(2:9)));
rram2t1r_vprog2p7V_inv10_format = cell2mat(rram2t1r_vprog2p7V_inv10(:,(2:9)));
rram2t1r_vprog2p8V_inv10_format = cell2mat(rram2t1r_vprog2p8V_inv10(:,(2:9)));
rram2t1r_vprog2p9V_inv10_format = cell2mat(rram2t1r_vprog2p9V_inv10(:,(2:9)));
rram2t1r_vprog3p0V_inv10_format = cell2mat(rram2t1r_vprog3p0V_inv10(:,(2:9)));
% RRAM 2T1R inv=15
rram2t1r_vprog2p5V_inv15_format = cell2mat(rram2t1r_vprog2p5V_inv15(:,(2:9)));
rram2t1r_vprog2p6V_inv15_format = cell2mat(rram2t1r_vprog2p6V_inv15(:,(2:9)));
rram2t1r_vprog2p7V_inv15_format = cell2mat(rram2t1r_vprog2p7V_inv15(:,(2:9)));
rram2t1r_vprog2p8V_inv15_format = cell2mat(rram2t1r_vprog2p8V_inv15(:,(2:9)));
rram2t1r_vprog2p9V_inv15_format = cell2mat(rram2t1r_vprog2p9V_inv15(:,(2:9)));
rram2t1r_vprog3p0V_inv15_format = cell2mat(rram2t1r_vprog3p0V_inv15(:,(2:9)));
% RRAM 2T1R inv=20
rram2t1r_vprog2p5V_inv20_format = cell2mat(rram2t1r_vprog2p5V_inv20(:,(2:9)));
rram2t1r_vprog2p6V_inv20_format = cell2mat(rram2t1r_vprog2p6V_inv20(:,(2:9)));
rram2t1r_vprog2p7V_inv20_format = cell2mat(rram2t1r_vprog2p7V_inv20(:,(2:9)));
rram2t1r_vprog2p8V_inv20_format = cell2mat(rram2t1r_vprog2p8V_inv20(:,(2:9)));
rram2t1r_vprog2p9V_inv20_format = cell2mat(rram2t1r_vprog2p9V_inv20(:,(2:9)));
rram2t1r_vprog3p0V_inv20_format = cell2mat(rram2t1r_vprog3p0V_inv20(:,(2:9)));

%% Fig. 1: RRAM4T1R Ids-Wprog (diff. Vprog)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle1 = figure;
plot(abs(rram4t1r_vprog2p5V_format(:,6)),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram4t1r_vprog2p6V_format(:,6)),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram4t1r_vprog2p7V_format(:,6)),'k-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram4t1r_vprog2p8V_format(:,6)),'k-.','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram4t1r_vprog2p9V_format(:,6)),'k-x','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram4t1r_vprog3p0V_format(:,6)),'k-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM4T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('I_{ds} ({\mu}A)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick', xindex);
set(gca,'XTickLabel', wprog_list(xindex));
%set(gca,'ylim',[2 10.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend(vprog_list);
set(fig_handle1, 'Position', [1 1 800 600]);
grid on

%% Fig. 2: RRAM2N1R Ids-Wprog (diff. Vprog) Winv=20
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle2 = figure;
plot(abs(rram2n1r_vprog2p5V_inv20_format(:,6)),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog2p6V_inv20_format(:,6)),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog2p7V_inv20_format(:,6)),'k-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog2p8V_inv20_format(:,6)),'k-.','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog2p9V_inv20_format(:,6)),'k-x','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog3p0V_inv20_format(:,6)),'k-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('I_{ds} ({\mu}A)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend(vprog_list);
set(fig_handle2, 'Position', [1 1 800 600]);
grid on

%% Fig. 3: RRAM2N1R Ids-Wprog (diff. Winv) Vprog=3.0
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle2 = figure;
plot(abs(rram2n1r_vprog3p0V_inv1_format(:,6)),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog3p0V_inv5_format(:,6)),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog3p0V_inv10_format(:,6)),'k-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog3p0V_inv15_format(:,6)),'k-.','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog3p0V_inv20_format(:,6)),'k-x','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('I_{ds} ({\mu}A)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend(inv_list);
set(fig_handle2, 'Position', [1 1 800 600]);
grid on

%% Fig. 4: RRAM2T1R Ids-Wprog (diff. Vprog) Winv=20
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle2 = figure;
plot(abs(rram2t1r_vprog2p5V_inv20_format(:,6)),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog2p6V_inv20_format(:,6)),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog2p7V_inv20_format(:,6)),'k-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog2p8V_inv20_format(:,6)),'k-.','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog2p9V_inv20_format(:,6)),'k-x','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog3p0V_inv20_format(:,6)),'k-s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('I_{ds} ({\mu}A)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend(vprog_list);
set(fig_handle2, 'Position', [1 1 800 600]);
grid on

%% Fig. 5: RRAM2T1R Ids-Wprog (diff. Winv) Vprog=3.0
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle2 = figure;
plot(abs(rram2t1r_vprog3p0V_inv1_format(:,6)),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog3p0V_inv5_format(:,6)),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog3p0V_inv10_format(:,6)),'k-+','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog3p0V_inv15_format(:,6)),'k-.','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog3p0V_inv20_format(:,6)),'k-x','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('I_{ds} ({\mu}A)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[10 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend(inv_list);
set(fig_handle2, 'Position', [1 1 800 600]);
grid on

%% Fig. 6: RRAM4T1R vs. RRAM2T1R & 2N1R Ron-Wprog (diff. Vprog)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle3 = figure;
plot(rram2n1r_vprog2p5V_inv20_format(:,8),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog3p0V_inv20_format(:,8),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog2p5V_inv20_format(:,8),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog3p0V_inv20_format(:,8),'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog2p5V_format(:,8),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog3p0V_format(:,8),'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('R_{LRS} ({k\Omega})','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[2 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
set(fig_handle3, 'Position', [1 1 800 600]);
grid on

%% Fig. 7: RRAM4T1R vs. RRAM2T1R Iprog/Wprog-Wprog (diff. Vprog)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle4 = figure;
plot(abs(rram2n1r_vprog2p5V_inv20_format(:,7)),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog3p0V_inv20_format(:,7)),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog2p5V_inv20_format(:,7)),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog3p0V_inv20_format(:,7)),'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram4t1r_vprog2p5V_format(:,7)),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram4t1r_vprog3p0V_format(:,7)),'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('I_{ds}/W_{prog} ({\mu}A per min. size trans.)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[35 140],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
set(fig_handle4, 'Position', [1 1 800 600]);
grid on

%% Fig. 8: RRAM2N1R V_TE/V_BE vs wprog (diff. Winv)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle5 = figure;
% 2N1R Vprog=3.0V inv=1
plot(rram2n1r_vprog3p0V_inv1_format(:,1),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog3p0V_inv1_format(:,2),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=5
plot(rram2n1r_vprog3p0V_inv5_format(:,1),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog3p0V_inv5_format(:,2),'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=10
plot(rram2n1r_vprog3p0V_inv10_format(:,1),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog3p0V_inv10_format(:,2),'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=15
plot(rram2n1r_vprog3p0V_inv15_format(:,1),'c-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog3p0V_inv15_format(:,2),'c-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=20
plot(rram2n1r_vprog3p0V_inv20_format(:,1),'g-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog3p0V_inv20_format(:,2),'g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('V_{DS} (V)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
set(gca,'ylim',[0.3 1.2],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'V_{DS1} of 2T1R W_{inv}=1'},{'V_{DS2} of 2T1R W_{inv}=1'},{'V_{DS1} of 2T1R W_{inv}=5'},{'V_{DS2} of 2T1R W_{inv}=5'},{'V_{DS1} of 2T1R W_{inv}=10'},{'V_{DS2} of 2T1R W_{inv}=10'},{'V_{DS1} of 2T1R W_{inv}=15'},{'V_{DS2} of 2T1R W_{inv}=15'},{'V_{DS1} of 2T1R W_{inv}=20'},{'V_{DS2} of 2T1R W_{inv}=20'}]);
set(fig_handle4, 'Position', [1 1 1000 800]);
grid on

%% Fig. 9: RRAM2N1R V_TE/V_BE vs wprog (diff. Vprog)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle5 = figure;
% 2N1R Vprog=2.5V inv=20
plot(rram2n1r_vprog2p5V_inv20_format(:,1),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog2p5V_inv20_format(:,2),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.6V inv=20
plot(rram2n1r_vprog2p6V_inv20_format(:,1),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog2p6V_inv20_format(:,2),'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.7V inv=20
plot(rram2n1r_vprog2p7V_inv20_format(:,1),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog2p7V_inv20_format(:,2),'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.8V inv=20
plot(rram2n1r_vprog2p8V_inv20_format(:,1),'c-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog2p8V_inv20_format(:,2),'c-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.9V inv=20
plot(rram2n1r_vprog2p9V_inv20_format(:,1),'y-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog2p9V_inv20_format(:,2),'y-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=20
plot(rram2n1r_vprog3p0V_inv20_format(:,1),'g-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog3p0V_inv20_format(:,2),'g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('V_{DS} (V)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
set(gca,'ylim',[0.1 1.1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'V_{DS1} of 2T1R V_{prog}=2.5V'},{'V_{DS2} of 2T1R V_{prog}=2.5V'},{'V_{DS1} of 2T1R V_{prog}=2.6V'},{'V_{DS2} of 2T1R V_{prog}=2.6V'},{'V_{DS1} of 2T1R V_{prog}=2.7V'},{'V_{DS2} of 2T1R V_{prog}=2.7V'},{'V_{DS1} of 2T1R V_{prog}=2.8V'},{'V_{DS2} of 2T1R V_{prog}=2.8V'},{'V_{DS1} of 2T1R V_{prog}=2.9V'},{'V_{DS2} of 2T1R V_{prog}=2.9V'},{'V_{DS1} of 2T1R V_{prog}=3.0V'},{'V_{DS2} of 2T1R V_{prog}=3.0V'}]);
set(fig_handle4, 'Position', [1 1 1000 800]);
grid on

%% Fig. 10: RRAM2T1R V_TE/V_BE vs wprog (diff. Winv)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle5 = figure;
% 2N1R Vprog=3.0V inv=1
plot(rram2t1r_vprog3p0V_inv1_format(:,1),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog3p0V_inv1_format(:,2),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=5
plot(rram2t1r_vprog3p0V_inv5_format(:,1),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog3p0V_inv5_format(:,2),'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=10
plot(rram2t1r_vprog3p0V_inv10_format(:,1),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog3p0V_inv10_format(:,2),'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=15
plot(rram2t1r_vprog3p0V_inv15_format(:,1),'c-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog3p0V_inv15_format(:,2),'c-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=20
plot(rram2t1r_vprog3p0V_inv20_format(:,1),'g-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog3p0V_inv20_format(:,2),'g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('V_{DS} (V)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
set(gca,'ylim',[0.7 1.05],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'V_{DS1} of 2TG1R W_{inv}=1'},{'V_{DS2} of 2TG1R W_{inv}=1'},{'V_{DS1} of 2TG1R W_{inv}=5'},{'V_{DS2} of 2TG1R W_{inv}=5'},{'V_{DS1} of 2TG1R W_{inv}=10'},{'V_{DS2} of 2TG1R W_{inv}=10'},{'V_{DS1} of 2TG1R W_{inv}=15'},{'V_{DS2} of 2TG1R W_{inv}=15'},{'V_{DS1} of 2TG1R W_{inv}=20'},{'V_{DS2} of 2TG1R W_{inv}=20'}]);
set(fig_handle4, 'Position', [1 1 1000 800]);
grid on

%% Fig. 11: RRAM2T1R V_TE/V_BE vs wprog (diff. Vprog)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle5 = figure;
% 2N1R Vprog=2.5V inv=20
plot(rram2t1r_vprog2p5V_inv20_format(:,1),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog2p5V_inv20_format(:,2),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.6V inv=20
plot(rram2t1r_vprog2p6V_inv20_format(:,1),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog2p6V_inv20_format(:,2),'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.7V inv=20
plot(rram2t1r_vprog2p7V_inv20_format(:,1),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog2p7V_inv20_format(:,2),'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.8V inv=20
plot(rram2t1r_vprog2p8V_inv20_format(:,1),'c-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog2p8V_inv20_format(:,2),'c-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.9V inv=20
plot(rram2t1r_vprog2p9V_inv20_format(:,1),'y-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog2p9V_inv20_format(:,2),'y-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V inv=20
plot(rram2t1r_vprog3p0V_inv20_format(:,1),'g-*','LineWidth', 2, 'MarkerSize',10);
plot(rram2t1r_vprog3p0V_inv20_format(:,2),'g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('V_{DS} (V)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
set(gca,'ylim',[0.4 1.1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'V_{DS1} of 2TG1R V_{prog}=2.5V'},{'V_{DS2} of 2TG1R V_{prog}=2.5V'},{'V_{DS1} of 2TG1R V_{prog}=2.6V'},{'V_{DS2} of 2TG1R V_{prog}=2.6V'},{'V_{DS1} of 2TG1R V_{prog}=2.7V'},{'V_{DS2} of 2TG1R V_{prog}=2.7V'},{'V_{DS1} of 2TG1R V_{prog}=2.8V'},{'V_{DS2} of 2TG1R V_{prog}=2.8V'},{'V_{DS1} of 2TG1R V_{prog}=2.9V'},{'V_{DS2} of 2TG1R V_{prog}=2.9V'},{'V_{DS1} of 2TG1R V_{prog}=3.0V'},{'V_{DS2} of 2TG1R V_{prog}=3.0V'}]);
hold on
set(fig_handle4, 'Position', [1 1 1000 800]);
grid on

%% Fig. 12: RRAM4T1R V_TE/V_BE vs wprog (diff. Vprog)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle5 = figure;
% 2N1R Vprog=2.5V
plot(rram4t1r_vprog2p5V_format(:,1),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog2p5V_format(:,2),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.6V
plot(rram4t1r_vprog2p6V_format(:,1),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog2p6V_format(:,2),'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.7V
plot(rram4t1r_vprog2p7V_format(:,1),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog2p7V_format(:,2),'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.8V
plot(rram4t1r_vprog2p8V_format(:,1),'c-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog2p8V_format(:,2),'c-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=2.9V
plot(rram4t1r_vprog2p9V_format(:,1),'y-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog2p9V_format(:,2),'y-o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V 
plot(rram4t1r_vprog3p0V_format(:,1),'g-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog3p0V_format(:,2),'g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('V_{DS} (V)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
set(gca,'ylim',[0.4 0.9],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'V_{DS1} of 4T1R V_{prog}=2.5V'},{'V_{DS2} of 4T1R V_{prog}=2.5V'},{'V_{DS1} of 4T1R V_{prog}=2.6V'},{'V_{DS2} of 4T1R V_{prog}=2.6V'},{'V_{DS1} of 4T1R V_{prog}=2.7V'},{'V_{DS2} of 4T1R V_{prog}=2.7V'},{'V_{DS1} of 4T1R V_{prog}=2.8V'},{'V_{DS2} of 4T1R V_{prog}=2.8V'},{'V_{DS1} of 4T1R V_{prog}=2.9V'},{'V_{DS2} of 4T1R V_{prog}=2.9V'},{'V_{DS1} of 4T1R V_{prog}=3.0V'},{'V_{DS2} of 4T1R V_{prog}=3.0V'}]);
set(fig_handle4, 'Position', [1 1 1000 800]);
grid on

%% Fig. 13: RRAM4T1R vs. RRAM2T1R & 2N1R Ron-Wprog (diff. Vprog)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle3 = figure;
% 2N1R Vprog=2.5V
plot(rram2n1r_vprog2p5V_inv20_format(:,1),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog2p5V_inv20_format(:,2),'k--o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2N1R Vprog=3.0V
plot(rram2n1r_vprog3p0V_inv20_format(:,1),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2n1r_vprog3p0V_inv20_format(:,2),'k--*','LineWidth', 2, 'MarkerSize',10);
hold on
% 2T1R Vprog=2.5V
plot(rram2t1r_vprog2p5V_inv20_format(:,1),'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog2p5V_inv20_format(:,2),'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
% 2T1R Vprog=3.0V
plot(rram2t1r_vprog3p0V_inv20_format(:,1),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram2t1r_vprog3p0V_inv20_format(:,2),'b--*','LineWidth', 2, 'MarkerSize',10);
hold on
% 4T1R Vprog=2.5V
plot(rram4t1r_vprog2p5V_format(:,1),'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog2p5V_format(:,2),'r--o','LineWidth', 2, 'MarkerSize',10);
hold on
% 4T1R Vprog=3.0V
plot(rram4t1r_vprog3p0V_format(:,1),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(rram4t1r_vprog3p0V_format(:,2),'r--*','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('V_{DS} (V)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[2 22],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'V_{DS1} of 2T1R V_{prog}=2.5V'},{'V_{DS2} of 2T1R V_{prog}=2.5V'},{'V_{DS1} of 2T1R V_{prog}=3.0V'},{'V_{DS2} of 2T1R V_{prog}=3.0V'},{'V_{DS1} of 2TG1R V_{prog}=2.5V'},{'V_{DS2} of 2TG1R V_{prog}=2.5V'},{'V_{DS1} of 2TG1R V_{prog}=3.0V'},{'V_{DS2} of 2TG1R V_{prog}=3.0V'},{'V_{DS1} of 4T1R V_{prog}=2.5V'},{'V_{DS2} of 4T1R V_{prog}=2.5V'},{'V_{DS1} of 4T1R V_{prog}=3.0V'},{'V_{DS2} of 4T1R V_{prog}=3.0V'}]);
set(fig_handle3, 'Position', [1 1 800 600]);
grid on

%% Fig. 14: RRAM4T1R vs. RRAM2T1R & 2N1R Area-Wprog
% Fig. plot
fig_handle3 = figure;
% RRAM 2N1R vprog=2.5V
x_ron = rram2n1r_vprog2p5V_inv20_format(:,8);
y_area = cell2mat(rram2n1r_vprog2p5V_inv20(:,1));
wprog_min = 0;
for i=1:1:length(y_area)
  if abs(rram2n1r_vprog2p5V_inv20_format(i,7))*wprog_list(i) < 200
    if (0 == wprog_min)||(wprog_min < wprog_list(i))
      wprog_min = wprog_list(i);
    end 
  end 
end
for i=1:1:length(y_area)
  if abs(rram2n1r_vprog2p5V_inv20_format(i,7))*wprog_list(i) < 200
    y_area(i) = wprog_min*2 + 20*4*(1/32+1/32);
  else 
    y_area(i) = wprog_list(i)*2 + 20*4*(1/32+1/32);
  end 
end 
plot(x_ron, y_area,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 2N1R vprog=3.0V
x_ron = rram2n1r_vprog3p0V_inv20_format(:,8);
y_area = cell2mat(rram2n1r_vprog3p0V_inv20(:,1));
wprog_min = 0;
for i=1:1:length(y_area)
  if abs(rram2n1r_vprog3p0V_inv20_format(i,7))*wprog_list(i) > 200
    if (0 == wprog_min)||(wprog_min > wprog_list(i))
      wprog_min = wprog_list(i);
    end 
  end 
end
for i=1:1:length(y_area)
  if abs(rram2n1r_vprog3p0V_inv20_format(i,7))*wprog_list(i) < 200
    y_area(i) = wprog_min*2 + 20*4*(1/32+1/32);
  else 
    y_area(i) = wprog_list(i)*2 + 20*4*(1/32+1/32);
  end 
end
plot(x_ron, y_area,'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 2T1R vprog=2.5V
x_ron = rram2t1r_vprog2p5V_inv20_format(:,8);
y_area = cell2mat(rram2t1r_vprog2p5V_inv20(:,1));
wprog_min = 0;
for i=1:1:length(y_area)
  if abs(rram2t1r_vprog2p5V_inv20_format(i,7))*wprog_list(i) > 200
    if (0 == wprog_min)||(wprog_min > wprog_list(i))
      wprog_min = wprog_list(i);
    end 
  end 
end
for i=1:1:length(y_area)
  if abs(rram2t1r_vprog2p5V_inv20_format(i,7))*wprog_list(i) < 200
    y_area(i) = wprog_min*2*4 + 20*4*(1/32+1/32);
  else 
    y_area(i) = wprog_list(i)*2*4 + 20*4*(1/32+1/32);
  end 
end
plot(x_ron, y_area,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 2T1R vprog=3.0V
x_ron = rram2t1r_vprog3p0V_inv20_format(:,8);
y_area = cell2mat(rram2t1r_vprog3p0V_inv20(:,1));
wprog_min = 0;
for i=1:1:length(y_area)
  if abs(rram2t1r_vprog3p0V_inv20_format(i,7))*wprog_list(i) > 200
    if (0 == wprog_min)||(wprog_min > wprog_list(i))
      wprog_min = wprog_list(i);
    end 
  end 
end
for i=1:1:length(y_area)
  if abs(rram2t1r_vprog3p0V_inv20_format(i,7))*wprog_list(i) < 200
    y_area(i) = wprog_min*2*4 + 20*4*(1/32+1/32);
  else 
    y_area(i) = wprog_list(i)*2*4 + 20*4*(1/32+1/32);
  end 
end
plot(x_ron, y_area,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R vprog=2.5V
x_ron = rram4t1r_vprog2p5V_format(:,8);
y_area = cell2mat(rram4t1r_vprog2p5V(:,1));
wprog_min = 0;
for i=1:1:length(y_area)
  if abs(rram4t1r_vprog2p5V_format(i,7))*wprog_list(i) > 200
    if (0 == wprog_min)||(wprog_min > wprog_list(i))
      wprog_min = wprog_list(i);
    end 
  end 
end
for i=1:1:length(y_area)
  y_area(i) = wprog_list(i)*4 + wprog_min*4;
end
plot(x_ron, y_area,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R vprog=3.0V
x_ron = rram4t1r_vprog3p0V_format(:,8);
y_area = cell2mat(rram4t1r_vprog3p0V(:,1));
wprog_min = 0;
for i=1:1:length(y_area)
  if abs(rram4t1r_vprog3p0V_format(i,7))*wprog_list(i) > 200
    if (0 == wprog_min)||(wprog_min > wprog_list(i))
      wprog_min = wprog_list(i);
    end 
  end 
end
for i=1:1:length(y_area)
  y_area(i) = wprog_list(i)*3.7 + wprog_min*3.7;
end
plot(x_ron, y_area,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('R_{LRS} ({k\Omega})','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Area (No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',xindex);
%set(gca,'XTickLabel',wprog_list(xindex));
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
set(fig_handle3, 'Position', [1 1 800 600]);
grid on

%% Fig. 15: RRAM4T1R vs. RRAM2T1R IprogWprog (diff. Vprog)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle4 = figure;
plot(abs(rram2n1r_vprog2p5V_inv20_format(:,6)),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog3p0V_inv20_format(:,6)),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog2p5V_inv20_format(:,6)),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2t1r_vprog3p0V_inv20_format(:,6)),'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram4t1r_vprog2p5V_format(:,6)),'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram4t1r_vprog3p0V_format(:,6)),'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('I_{ds} ({\mu}A)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[35 140],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
set(fig_handle4, 'Position', [1 1 800 600]);
grid on
