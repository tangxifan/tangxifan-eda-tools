clear all
close all
%% Device parameters
gap_max = 1.7e-9;
gap_min = 0.2e-9;
g0 = 0.25e-9;
V0 = 0.25;
I0 = 1000e-6;
%% I-V equation
Vtb = 0:0.1:1.5;
gap = gap_min;
Itb = I0 * exp(-gap/g0) * sinh(Vtb/V0);

%% plot the I-V characteristic
fig_handle1 = figure;
plot(Vtb, Itb, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('V_{TB} (V)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('I_{TB} (A)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
grid on
