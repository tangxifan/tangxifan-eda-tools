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
%plot(rram2n1r_vprog2p5V_inv20_format(:,8),'k-*','LineWidth', 2, 'MarkerSize',10);
%hold on
%plot(rram2n1r_vprog3p0V_inv20_format(:,8),'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
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
hleg = legend([{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
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
%set(gca,'ylim',[0.3 1.2],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
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
%set(gca,'ylim',[0.1 1.1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
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
%set(gca,'ylim',[0.7 1.05],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
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
%set(gca,'ylim',[0.4 1.1],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
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
%set(gca,'ylim',[0.4 0.9],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
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

%% Fig. 14: RRAM4T1R vs. RRAM2T1R IprogWprog (diff. Vprog)
xindex = 1:10:length(wprog_list);
% Fig. plot
fig_handle4 = figure;
plot(abs(rram2n1r_vprog2p5V_inv20_format(:,6)),'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(abs(rram2n1r_vprog3p0V_inv20_format(:,6)),'k-o','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(abs(rram2t1r_vprog2p5V_inv20_format(:,6)),'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
%plot(abs(rram2t1r_vprog3p0V_inv20_format(:,6)),'b-o','LineWidth', 2, 'MarkerSize',10);
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
%hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
set(fig_handle4, 'Position', [1 1 800 600]);
grid on

% Calculate the area, delay and power
Ireset = 1000; % [uA]
Coff_p = 165e-18; % [F] PMOS I/O trans Cd
Coff_n = 263e-18; % [F] NMOS I/O trans Cd
Cload = 231e-18; % [F]
Rinv = 8e3; % 7200; % [Ohm]
Cinv = 476e-18; % [F]
vsp = 0.9; % [V]
beta = 3; % p/n ratio
gamma = 1.2; % pmos area penalty
M = 60; % Size of a one-level multiplexer, no. of prog. structures that share a output programming pair.
std_beta = 2; % beta for standard p/n transistors
sram_area = 4; % area of a SRAM cell
Winv_rram = 2; % size of inverter for SRAM circuit
Winv_sram = 7; % size of inverter for SRAM circuit
Wnmos = 2; % size of a nmos transistor
Rnmos = 8e3; % [Ohm] equivalent resistance of a nmos 
Cnmos = 99e-18; % [F] source/drain capacitance of a nmos
trans_area_sram_inv = trans_area(Winv_sram) + trans_area(std_beta*Winv_sram)*gamma;
trans_area_rram_inv = trans_area(Winv_rram) + trans_area(std_beta*Winv_rram)*gamma;
% RRAM 2N1R vprog=2.5V
ron_rram2n1r_vprog2p5V = rram2n1r_vprog2p5V_inv20_format(:,8);
area_rram2n1r_vprog2p5V = cell2mat(rram2n1r_vprog2p5V_inv20(:,1));
wprog_min = 0;
for i=1:1:length(area_rram2n1r_vprog2p5V)
  if abs(rram2n1r_vprog2p5V_inv20_format(i,6)) > Ireset
    wprog_min = wprog_list(i);
    break;
  end 
end
if (0 == wprog_min) 
  wprog_min = Ireset/min(abs(rram2n1r_vprog2p5V_inv20_format(:,7)));
  wprog_min = abs(wprog_min);
end
display(wprog_min);
for i=1:1:length(area_rram2n1r_vprog2p5V)
  if wprog_list(i) < wprog_min
    area_rram2n1r_vprog2p5V(i) = trans_area_rram_inv*(M+1)/2 + trans_area(wprog_min)*(M+1) + 20*(1+beta*gamma)*(1/32+1/32);
    delay_rram2n1r_vprog2p5V(i) = (Rinv/Winv_rram*(Winv_rram*Cinv+2*wprog_min*(Coff_n)+Cload) + (wprog_min*(Coff_n)+Cload)*rram2n1r_vprog2p5V_inv20_format(i,8)*1e3)/1e-12;
    power_rram2n1r_vprog2p5V(i) = (M/2*Cinv*Winv_rram + (M/2+1)*(Coff_n)*wprog_min+Cload)*vsp*vsp;
    power_rram2n1r_vprog2p5V(i) = power_rram2n1r_vprog2p5V(i)/delay_rram2n1r_vprog2p5V(i);
  else 
    area_rram2n1r_vprog2p5V(i) = trans_area_rram_inv*(M+1)/2 + trans_area(wprog_list(i))*(M+1) + 20*(1+beta*gamma)*(1/32+1/32);
    delay_rram2n1r_vprog2p5V(i) = (Rinv/Winv_rram*(Cinv*Winv_rram+2*wprog_list(i)*(Coff_n)+Cload) + (wprog_list(i)*(Coff_n)+Cload)*rram2n1r_vprog2p5V_inv20_format(i,8)*1e3)/1e-12;
    power_rram2n1r_vprog2p5V(i) = (M/2*Cinv*Winv_rram + (M/2+1)*(Coff_n)*wprog_list(i)+Cload)*vsp*vsp;
    power_rram2n1r_vprog2p5V(i) = power_rram2n1r_vprog2p5V(i)/delay_rram2n1r_vprog2p5V(i);
  end 
end 
% RRAM 2N1R vprog=3.0V
ron_rram2n1r_vprog3p0V = rram2n1r_vprog3p0V_inv20_format(:,8);
area_rram2n1r_vprog3p0V = cell2mat(rram2n1r_vprog3p0V_inv20(:,1));
wprog_min = 0;
for i=1:1:length(area_rram2n1r_vprog3p0V)
  if abs(rram2n1r_vprog3p0V_inv20_format(i,6)) > Ireset
    wprog_min = wprog_list(i);
    break;
  end 
end
if (0 == wprog_min) 
  wprog_min = Ireset/min(abs(rram2n1r_vprog3p0V_inv20_format(:,7)));
  wprog_min = abs(wprog_min);
end
for i=1:1:length(area_rram2n1r_vprog3p0V)
  if wprog_list(i) < wprog_min
    area_rram2n1r_vprog3p0V(i) = trans_area_rram_inv*(M+1)/2 + trans_area(wprog_min)*(M+1) + 20*(1+beta*gamma)*(1/32+1/32);
    delay_rram2n1r_vprog3p0V(i) = (Rinv/Winv_rram*(Cinv*Winv_rram+2*wprog_min*(Coff_p+Coff_n)/2+Cload) + (wprog_min*(Coff_p+Coff_n)/2+Cload)*rram2n1r_vprog3p0V_inv20_format(i,8)*1e3)/1e-12;
    power_rram2n1r_vprog3p0V(i) = (M/2*Cinv*Winv_rram + (M/2+1)*(Coff_p+Coff_n)/2*2*wprog_min+Cload)*vsp*vsp;
    power_rram2n1r_vprog3p0V(i) = power_rram2n1r_vprog3p0V(i)/delay_rram2n1r_vprog3p0V(i);
  else 
    area_rram2n1r_vprog3p0V(i) = trans_area_rram_inv*(M+1)/2 + trans_area(wprog_list(i))*(M+1) + 20*(1+beta*gamma)*(1/32+1/32);
    delay_rram2n1r_vprog3p0V(i) = (Rinv/Winv_rram*(Cinv*Winv_rram+2*wprog_list(i)*(Coff_p+Coff_n)/2+Cload) + (wprog_list(i)*(Coff_p+Coff_n)/2+Cload)*rram2n1r_vprog3p0V_inv20_format(i,8)*1e3)/1e-12;
    power_rram2n1r_vprog3p0V(i) = (M/2*Cinv*Winv_rram + (M/2+1)*(Coff_p+Coff_n)/2*wprog_list(i)+Cload)*vsp*vsp;
    power_rram2n1r_vprog3p0V(i) = power_rram2n1r_vprog3p0V(i)/delay_rram2n1r_vprog3p0V(i);
  end 
end
% RRAM 2T1R vprog=2.5V
ron_rram2t1r_vprog2p5V = rram2t1r_vprog2p5V_inv20_format(:,8);
area_rram2t1r_vprog2p5V = cell2mat(rram2t1r_vprog2p5V_inv20(:,1));
wprog_min = 0;
for i=1:1:length(area_rram2t1r_vprog2p5V)
  if abs(rram2t1r_vprog2p5V_inv20_format(i,6)) > Ireset
    wprog_min = wprog_list(i);
    break;
  end 
end
if (0 == wprog_min) 
  wprog_min = Ireset/min(abs(rram2t1r_vprog2p5V_inv20_format(:,7)));
  wprog_min = abs(wprog_min);
end
for i=1:1:length(area_rram2t1r_vprog2p5V)
  if wprog_list(i) < wprog_min
    area_rram2t1r_vprog2p5V(i) = trans_area_rram_inv*(M+1)/2 + (trans_area(wprog_min)+trans_area(wprog_min*beta)*gamma)*(M+1) + 20*(1+beta*gamma)*(1/32+1/32);
    delay_rram2t1r_vprog2p5V(i) = (Rinv/Winv_rram*(Cinv*Winv_rram+2*wprog_min*(beta+1)*(Coff_n+Coff_p)/2+Cload) + (wprog_min*(beta+1)*(Coff_p+Coff_n)/2+Cload)*rram2t1r_vprog2p5V_inv20_format(i,8)*1e3)/1e-12;
    power_rram2t1r_vprog2p5V(i) = (M/2*Cinv*Winv_rram + (M/2+1)*(Coff_n+Coff_p)/2*wprog_min*(beta+1)+Cload)*vsp*vsp/1e-15;
    power_rram2t1r_vprog2p5V(i) = power_rram2t1r_vprog2p5V(i)/delay_rram2t1r_vprog2p5V(i);
  else 
    area_rram2t1r_vprog2p5V(i) = trans_area_rram_inv*(M+1)/2 + (trans_area(wprog_list(i))+trans_area(wprog_list(i)*beta)*gamma)*(M+1) + 20*(1+beta*gamma)*(1/32+1/32);
    delay_rram2t1r_vprog2p5V(i) = (Rinv/Winv_rram*(Cinv*Winv_rram+2*wprog_list(i)*(beta+1)*(Coff_n+Coff_p)/2+Cload) + (wprog_list(i)*(beta+1)*(Coff_p+Coff_n)/2+Cload)*rram2t1r_vprog2p5V_inv20_format(i,8)*1e3)/1e-12;
    power_rram2t1r_vprog2p5V(i) = (M/2*Cinv*Winv_rram + (M/2+1)*(Coff_n+Coff_p)/2*wprog_list(i)*(beta+1)+Cload)*vsp*vsp/1e-15;
    power_rram2t1r_vprog2p5V(i) = power_rram2t1r_vprog2p5V(i)/delay_rram2t1r_vprog2p5V(i);
  end 
end
% RRAM 2T1R vprog=3.0V
ron_rram2t1r_vprog3p0V = rram2t1r_vprog3p0V_inv20_format(:,8);
area_rram2t1r_vprog3p0V = cell2mat(rram2t1r_vprog3p0V_inv20(:,1));
wprog_min = 0;
for i=1:1:length(area_rram2t1r_vprog3p0V)
  if abs(rram2t1r_vprog3p0V_inv20_format(i,6)) > Ireset
    wprog_min = wprog_list(i);
    break;
  end 
end
if (0 == wprog_min) 
  wprog_min = Ireset/min(abs(rram2t1r_vprog3p0V_inv20_format(:,7)));
  wprog_min = abs(wprog_min);
end
for i=1:1:length(area_rram2t1r_vprog3p0V)
  if wprog_list(i) < wprog_min
    area_rram2t1r_vprog3p0V(i) = trans_area_rram_inv*(M+1)/2 + (trans_area(wprog_min)+trans_area(wprog_min*beta)*gamma)*(M+1) + 20*(1+beta*gamma)*(1/32+1/32);
    delay_rram2t1r_vprog3p0V(i) = (Rinv/Winv_rram*(Cinv*Winv_rram+2*wprog_min*(beta+1)*(Coff_n+Coff_p)/2+Cload) + (wprog_min*(beta+1)*(Coff_p+Coff_n)/2+Cload)*rram2t1r_vprog3p0V_inv20_format(i,8)*1e3)/1e-12;
    power_rram2t1r_vprog3p0V(i) = (M/2*Cinv*Winv_rram + (M/2+1)*(Coff_n+Coff_p)/2*wprog_min*(beta+1)+Cload)*vsp*vsp/1e-15;
    power_rram2t1r_vprog3p0V(i) = power_rram2t1r_vprog3p0V(i)/delay_rram2t1r_vprog3p0V(i);
  else 
    area_rram2t1r_vprog3p0V(i) = trans_area_rram_inv*(M+1)/2 + (trans_area(wprog_list(i))+trans_area(wprog_list(i)*beta*gamma))*(M+1) + 20*(1+beta*gamma)*(1/32+1/32);
    delay_rram2t1r_vprog3p0V(i) = (Rinv/Winv_rram*(Cinv*Winv_rram+2*wprog_list(i)*(beta+1)*(Coff_p+Coff_n)/2+Cload) + (wprog_list(i)*(beta+1)*(Coff_p+Coff_n)/2+Cload)*rram2t1r_vprog3p0V_inv20_format(i,8)*1e3)/1e-12;
    power_rram2t1r_vprog3p0V(i) = (M/2*Cinv*Winv_rram + (M/2+1)*(Coff_p+Coff_n)/2*wprog_list(i)*(beta+1)+Cload)*vsp*vsp/1e-15;
    power_rram2t1r_vprog3p0V(i) = power_rram2t1r_vprog3p0V(i)/delay_rram2t1r_vprog3p0V(i);
  end 
end
% RRAM 4T1R vprog=2.5V
ron_rram4t1r_vprog2p5V = rram4t1r_vprog2p5V_format(:,8);
area_rram4t1r_vprog2p5V = cell2mat(rram4t1r_vprog2p5V(:,1));
wprog_min = 0;
for i=1:1:length(area_rram4t1r_vprog2p5V)
  if abs(rram4t1r_vprog2p5V_format(i,6)) > Ireset
    wprog_min = wprog_list(i);
    break; 
  end 
end
if (0 == wprog_min) 
  wprog_min = Ireset/min(abs(rram4t1r_vprog2p5V_format(:,7)));
  wprog_min = abs(wprog_min);
end
for i=1:1:length(area_rram4t1r_vprog2p5V)
  area_rram4t1r_vprog2p5V(i) = trans_area_rram_inv*(M+1)/2 + trans_area(wprog_list(i)) + M*trans_area(wprog_list(i)*beta)*gamma + M*trans_area(wprog_min) + trans_area(beta*wprog_min)*gamma;
  delay_rram4t1r_vprog2p5V(i) = (Rinv/Winv_rram*(Cinv*Winv_rram+(wprog_list(i)*(beta+1)+wprog_min*(beta+1))*(Coff_p+Coff_n)/2+Cload) + ((wprog_list(i)*(1)*Coff_n+wprog_min*(beta)*Coff_p)+Cload)*rram4t1r_vprog2p5V_format(i,8)*1e3)/1e-12;
  power_rram4t1r_vprog2p5V(i) = (M/2*Cinv*Winv_rram + (Coff_n+Coff_p)/2*(wprog_min*(beta+1*M/2)+wprog_list(i)*(M*beta/2+1))+Cload)*vsp*vsp/1e-15;
  power_rram4t1r_vprog2p5V(i) = power_rram4t1r_vprog2p5V(i)/delay_rram4t1r_vprog2p5V(i);
end
% RRAM 4T1R vprog=3.0V
ron_rram4t1r_vprog3p0V = rram4t1r_vprog3p0V_format(:,8);
area_rram4t1r_vprog3p0V = cell2mat(rram4t1r_vprog3p0V(:,1));
wprog_min = 0;
for i=1:1:length(area_rram4t1r_vprog3p0V)
  if abs(rram4t1r_vprog3p0V_format(i,6)) > Ireset
    wprog_min = wprog_list(i);
    break; 
  end 
end
if (0 == wprog_min) 
  wprog_min = Ireset/min(abs(rram4t1r_vprog3p0V_format(:,7)));
  wprog_min = abs(wprog_min);
end
for i=1:1:length(area_rram4t1r_vprog3p0V)
  area_rram4t1r_vprog3p0V(i) = trans_area_rram_inv*(M+1)/2 + trans_area(wprog_list(i)) + M*trans_area(wprog_list(i)*beta)*gamma + M*trans_area(wprog_min) + trans_area(beta*wprog_min)*gamma;
  delay_rram4t1r_vprog3p0V(i) = (Rinv/Winv_rram*(Cinv*Winv_rram+(wprog_list(i)*(beta+1)+wprog_min*(beta+1))*(Coff_n+Coff_p)/2+Cload) + ((wprog_list(i)*(1)*Coff_n+wprog_min*(beta)*Coff_p)+Cload)*rram4t1r_vprog3p0V_format(i,8)*1e3)/1e-12;
  power_rram4t1r_vprog3p0V(i) = (M/2*Cinv*Winv_rram + (Coff_n+Coff_p)/2*(wprog_min*(beta+1*M/2)+wprog_list(i)*(M*beta/2+1))+Cload)*vsp*vsp/1e-15;
  power_rram4t1r_vprog3p0V(i) = power_rram4t1r_vprog3p0V(i)/delay_rram4t1r_vprog3p0V(i);
  %y_area(i) = 2*wprog_list(i)*(1+3*1.2);
end
% SRAM circuit
area_sram_circuit= cell2mat(rram4t1r_vprog3p0V(:,1));
for i=1:1:length(area_sram_circuit)
  area_sram_circuit(i) = trans_area_sram_inv*(M+1)/2 + ((M+ceil(sqrt(M)))*(trans_area(Wnmos) + trans_area(std_beta*Wnmos)*gamma) + 2*ceil(sqrt(M))*sram_area)/2;
  delay_sram_circuit(i) = (Rinv/Winv_sram*(Cinv*Winv_sram+Cnmos*Wnmos*(std_beta+1)*(2*ceil(sqrt(M))+2)+Cload) + Rnmos/(2)*(3*ceil(sqrt(M))+3)*Cnmos*(std_beta + 1)+2*Rnmos/2/Wnmos*Cload)/1e-12;
  power_sram_circuit(i) = (M*Winv_sram*Cinv/2 + 2*Wnmos*Cnmos*(std_beta + 1)*(M+ceil(sqrt(M)))/2+Cload)*vsp*vsp/1e-15;
  power_sram_circuit(i) = power_sram_circuit(i)/delay_sram_circuit(i);
end

%% Fig. 15: RRAM4T1R vs. RRAM2T1R & 2N1R Area-Wprog
% Fig. plot
fig_handle3 = figure;
% RRAM 2N1R Vprog=2.5V
%plot(ron_rram2n1r_vprog2p5V, area_rram2n1r_vprog2p5V,'k-*','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2N1R Vprog=3.0V
%plot(ron_rram2n1r_vprog3p0V, area_rram2n1r_vprog3p0V,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2T1R Vprog=2.5V
plot(ron_rram2t1r_vprog2p5V, area_rram2t1r_vprog2p5V,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 2T1R Vprog=3.0V
plot(ron_rram2t1r_vprog3p0V, area_rram2t1r_vprog3p0V,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=2.5V
plot(ron_rram4t1r_vprog2p5V, area_rram4t1r_vprog2p5V,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=3.0V
plot(ron_rram4t1r_vprog3p0V, area_rram4t1r_vprog3p0V,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM Circuit
plot(ron_rram4t1r_vprog3p0V, area_sram_circuit,'g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
xlabel('R_{LRS} ({k\Omega})','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Area (No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',xindex);
%set(gca,'XTickLabel',wprog_list(xindex));
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
hleg = legend([{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'},{'SRAM Circuit'}]);
set(fig_handle3, 'Position', [1 1 800 600]);
grid on

%% Fig. 16: RRAM4T1R vs. RRAM2T1R & 2N1R Delay-Ron
fig_handle3 = figure;
% RRAM 2N1R Vprog=2.5V
%plot(ron_rram2n1r_vprog2p5V, delay_rram2n1r_vprog2p5V,'k-*','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2N1R Vprog=3.0V
%plot(ron_rram2n1r_vprog3p0V, delay_rram2n1r_vprog3p0V,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2T1R Vprog=2.5V
plot(ron_rram2t1r_vprog2p5V, delay_rram2t1r_vprog2p5V,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 2T1R Vprog=3.0V
plot(ron_rram2t1r_vprog3p0V, delay_rram2t1r_vprog3p0V,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=2.5V
plot(ron_rram4t1r_vprog2p5V, delay_rram4t1r_vprog2p5V,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=3.0V
plot(ron_rram4t1r_vprog3p0V, delay_rram4t1r_vprog3p0V,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM Circuit
plot(ron_rram4t1r_vprog3p0V, delay_sram_circuit,'g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
ylabel('Delay (ps)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
xlabel('R_{LRS} ({k\Omega})','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'xlim',[0.5 length(wprog_list)+0.5],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',xindex);
%set(gca,'XTickLabel',wprog_list(xindex));
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
hleg = legend([{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'},{'SRAM Circuit'}]);
set(fig_handle3, 'Position', [1 1 800 600]);
grid on

%% Fig. 17: RRAM4T1R vs. RRAM2T1R & 2N1R Power - Ron
% Fig. plot
fig_handle3 = figure;
% RRAM 2N1R Vprog=2.5V
%plot(ron_rram2n1r_vprog2p5V, power_rram2n1r_vprog2p5V,'k-*','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2N1R Vprog=3.0V
%plot(ron_rram2n1r_vprog3p0V, power_rram2n1r_vprog3p0V,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2T1R Vprog=2.5V
plot(ron_rram2t1r_vprog2p5V, power_rram2t1r_vprog2p5V,'b-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 2T1R Vprog=3.0V
plot(ron_rram2t1r_vprog3p0V, power_rram2t1r_vprog3p0V,'b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=2.5V
plot(ron_rram4t1r_vprog2p5V, power_rram4t1r_vprog2p5V,'r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=3.0V
plot(ron_rram4t1r_vprog3p0V, power_rram4t1r_vprog3p0V,'r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM Circuit
plot(ron_rram4t1r_vprog3p0V, power_sram_circuit,'g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
%xlabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
xlabel('R_{LRS} ({k\Omega})','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Delay-Area Product (ps * No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power (mW)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',xindex);
%set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[0 1500],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
hleg = legend([{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'},{'SRAM Circuit'}]);
set(fig_handle3, 'Position', [1 1 800 600]);
grid on

%% Fig. 18: RRAM4T1R vs. RRAM2T1R & 2N1R Area-Delay Product - Ron 
% Fig. plot
fig_handle3 = figure;
% RRAM 2N1R Vprog=2.5V
%plot(ron_rram2n1r_vprog2p5V, power_rram2n1r_vprog2p5V,'k-*','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2N1R Vprog=3.0V
%plot(ron_rram2n1r_vprog3p0V, power_rram2n1r_vprog3p0V,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2T1R Vprog=2.5V
plot(ron_rram2t1r_vprog2p5V, area_rram2t1r_vprog2p5V.*delay_rram2t1r_vprog2p5V','b-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 2T1R Vprog=3.0V
plot(ron_rram2t1r_vprog3p0V, area_rram2t1r_vprog3p0V.*delay_rram2t1r_vprog3p0V','b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=2.5V
plot(ron_rram4t1r_vprog2p5V, area_rram4t1r_vprog2p5V.*delay_rram4t1r_vprog2p5V','r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=3.0V
plot(ron_rram4t1r_vprog3p0V, area_rram4t1r_vprog3p0V.*delay_rram4t1r_vprog3p0V','r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM circuit 
plot(ron_rram4t1r_vprog3p0V, area_sram_circuit.*delay_sram_circuit','g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
%xlabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
xlabel('R_{LRS} ({k\Omega})','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Delay-Area Product (ps * No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Area-Delay Product (# of min width trans. * ps)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',xindex);
%set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[0 1500],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
hleg = legend([{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'},{'SRAM Circuit'}]);
set(fig_handle3, 'Position', [1 1 800 600]);
grid on

%% Fig. 19: RRAM4T1R vs. RRAM2T1R & 2N1R Power-Delay Product - Ron 
% Fig. plot
fig_handle3 = figure;
% RRAM 2N1R Vprog=2.5V
%plot(ron_rram2n1r_vprog2p5V, power_rram2n1r_vprog2p5V,'k-*','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2N1R Vprog=3.0V
%plot(ron_rram2n1r_vprog3p0V, power_rram2n1r_vprog3p0V,'k-o','LineWidth', 2, 'MarkerSize',10);
%hold on
% RRAM 2T1R Vprog=2.5V
plot(ron_rram2t1r_vprog2p5V, delay_rram2t1r_vprog2p5V'.*power_rram2t1r_vprog2p5V','b-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 2T1R Vprog=3.0V
plot(ron_rram2t1r_vprog3p0V, delay_rram2t1r_vprog3p0V'.*power_rram2t1r_vprog3p0V','b-o','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=2.5V
plot(ron_rram4t1r_vprog2p5V, delay_rram4t1r_vprog2p5V'.*power_rram4t1r_vprog2p5V','r-*','LineWidth', 2, 'MarkerSize',10);
hold on
% RRAM 4T1R Vprog=3.0V
plot(ron_rram4t1r_vprog3p0V, delay_rram4t1r_vprog3p0V'.*power_rram4t1r_vprog3p0V','r-o','LineWidth', 2, 'MarkerSize',10);
hold on
% SRAM Circuit
plot(ron_rram4t1r_vprog3p0V, delay_sram_circuit'.*power_sram_circuit','g-o','LineWidth', 2, 'MarkerSize',10);
hold on
%title('R_{on} and W_{prog}, RRAM2T1R Structure','FontSize',18)
%xlabel('Delay (ps)','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%xlabel('W_{prog}(No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
xlabel('R_{LRS} ({k\Omega})','FontSize',18, 'FontWeight','bold', 'FontName', 'Times');
%ylabel('Delay-Area Product (ps * No. of min. trans.)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('Power-Delay Product (fJ)','FontSize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%set(gca,'XTick',xindex);
%set(gca,'XTickLabel',wprog_list(xindex));
%set(gca,'ylim',[0 1500],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca,'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
%hleg = legend([{'2T1R V_{prog}=2.5V'},{'2T1R V_{prog}=3.0V'},{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'}]);
hleg = legend([{'2TG1R V_{prog}=2.5V'},{'2TG1R V_{prog}=3.0V'},{'4T1R V_{prog}=2.5V'},{'4T1R V_{prog}=3.0V'},{'SRAM Circuit'}]);
set(fig_handle3, 'Position', [1 1 800 600]);
grid on
