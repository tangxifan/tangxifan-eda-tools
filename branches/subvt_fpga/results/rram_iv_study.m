clear all
close all
%% Default Device parameters
%gap_max = 1.7e-9;
%gap_min = 0.2e-9;
%g0 = 0.25e-9;
%V0 = 0.25;
%I0 = 1000e-6;

%% Fit Device parameters
gap_max = 2.48e-9;
gap_min = 0.2e-9;
g0 = 0.25e-9;
V0 = 0.8;
I0 = 570e-6;

%% I-V equation
Vtb = -1.3:0.1:1.3;
gap = gap_min:0.1e-9:gap_max;
iv_legend = [{'Ireset(gap=max)'}, {'Iset(gap=min)'}];
Itb_reset = I0 * exp(-gap_max/g0) * sinh(Vtb/V0);
Itb_set = I0 * exp(-gap_min/g0) * sinh(Vtb/V0);

res_legend = [{'Rreset(gap=max)'}, {'Rset(gap=min)'}];
Rtb_reset = Vtb./Itb_reset;
Rtb_set = Vtb./Itb_set;
disp('Rtb_set(kOhm):');
disp(Rtb_set/1e3);
disp('Rtb_reset(MOhm):');
disp(Rtb_reset/1e6);

%% I-V Data for Sweep V0
V0_sweep = [0.2,0.5,0.8]; % [0,10]
V0_sweep_legend = [{'V0=0.2'}, {'V0=0.5'}, {'V0=0.8'}];
Itb_set_V0_1 = I0 * exp(-gap_min/g0) * sinh(Vtb/V0_sweep(1));
Itb_set_V0_2 = I0 * exp(-gap_min/g0) * sinh(Vtb/V0_sweep(2));
Itb_set_V0_3 = I0 * exp(-gap_min/g0) * sinh(Vtb/V0_sweep(3));

%% I-V Data for Sweep g0
g0_sweep = [0.2e-9,0.5e-9,1e-9]; % (0,2e-9);
g0_sweep_legend = [{'g0=0.2e-9'}, {'g0=0.5e-9'}, {'g0=1e-9'}];
Itb_set_g0_1 = I0 * exp(-gap_min/g0_sweep(1)) * sinh(Vtb/V0);
Itb_set_g0_2 = I0 * exp(-gap_min/g0_sweep(2)) * sinh(Vtb/V0);
Itb_set_g0_3 = I0 * exp(-gap_min/g0_sweep(3)) * sinh(Vtb/V0);

%% Fig.1: plot the I-V characteristic Set&Reset
fig_handle1 = figure;
plot(Vtb, Itb_reset/1e-3, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, Itb_set/1e-3, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('V_{TB} (V)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('I_{TB} (mA)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(iv_legend);
grid on

%% Fig.2: plot the R-V characteristic Set&Reset
fig_handle2 = figure;
plot(Vtb, Rtb_reset/1e3, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, Rtb_set/1e3, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('V_{TB} (V)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('R_{TB} (kOhm)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(res_legend);
grid on

%% Fig.3: Sweep V0 for I-V characeristic 
fig_handle3 = figure;
plot(Vtb, Itb_set_V0_1, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, Itb_set_V0_2, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, Itb_set_V0_3, 'g-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('V_{TB} (V)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('I_{TB} (A)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(V0_sweep_legend);
grid on

%% Fig.4: Sweep g0 for I-V characeristic 
fig_handle4 = figure;
plot(Vtb, Itb_set_g0_1, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, Itb_set_g0_2, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, Itb_set_g0_3, 'g-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('V_{TB} (V)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('I_{TB} (A)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(g0_sweep_legend);
grid on
