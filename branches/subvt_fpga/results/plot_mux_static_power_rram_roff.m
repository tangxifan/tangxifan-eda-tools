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

static_power_mux2_format = cell2mat(static_power_mux2(:,(2:4)));
static_power_mux2_tapbuf_format = cell2mat(static_power_mux2_tapbuf(:,(2:4)));
avg_power_mux2_tapbuf_format = cell2mat(avg_power_mux2_tapbuf(:,(2:4)));
xindex = 1:1:length(roff_list);

fig_handle1 = figure;
plot(static_power_mux2_format(:,1).*1e9,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(static_power_mux2_format(:,2).*1e9,'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(static_power_mux2_format(:,3).*1e9,'r-.s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('RHRS(MegOhm)','FontSize',14, 'FontWeight','bold');
ylabel('Static Power (nW)','FontSize',14, 'FontWeight','bold');
set(gca,'xlim',[0.5 length(roff_list)+0.5],'Fontsize',16, 'FontWeight','bold');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',roff_list);
hleg = legend(mux_list);
grid on

fig_handle2 = figure;
plot(static_power_mux2_tapbuf_format(:,1).*1e9,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(static_power_mux2_tapbuf_format(:,2).*1e9,'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(static_power_mux2_tapbuf_format(:,3).*1e9,'r-.s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('RHRS(MegOhm)','FontSize',14, 'FontWeight','bold');
ylabel('Static Power (nW)','FontSize',14, 'FontWeight','bold');
set(gca,'xlim',[0.5 length(roff_list)+0.5],'Fontsize',16, 'FontWeight','bold');
set(gca,'XTick',xindex);
set(gca,'XTickLabel',roff_list);
hleg = legend(mux_list);
grid on

fig_handle3 = figure;
xindex = 1:1:length(avg_power_mux2_tapbuf(:,1));
plot(avg_power_mux2_tapbuf_format(:,1).*1e6,'k-*','LineWidth', 2, 'MarkerSize',10);
hold on
plot(avg_power_mux2_tapbuf_format(:,2).*1e6,'b--o','LineWidth', 2, 'MarkerSize',10);
hold on
plot(avg_power_mux2_tapbuf_format(:,3).*1e6,'r-.s','LineWidth', 2, 'MarkerSize',10);
hold on
%title('Delay-Wprog of 32-input Multiplexer, UMC 0.18um','FontSize',18)
xlabel('RHRS(MegOhm)','FontSize',14, 'FontWeight','bold');
ylabel('Average Power (uW)','FontSize',14, 'FontWeight','bold');
set(gca,'xlim',[0.5 length(avg_power_mux2_tapbuf(:,1))+0.5],'Fontsize',16, 'FontWeight','bold');
set(gca,'XTick', xindex);
set(gca,'XTickLabel', avg_power_mux2_tapbuf(:,1));
hleg = legend(mux_list);
grid on
