clear all
close all
%% Constants and physical parameters
kb = 1.38e-23;
q = 1.6e-19;
% threshold temperature for signficant random variations
T_crit = 450;
% Variation fitting parameters
T_smth = 500;
% Initial room temperature in devices
T_ini = 273+25;
% minimum field requirement to enhance gap formation, F_min
F_min = 1.4e9;
% Thermal resistance
Rth = 2.1e3;
% oxide thickness
%tox = 12e-9;
tox = 10e-9;
% random seed
rand_seed_ini = 0;
% time step 
time_step = 1e-9;

%% Default Device parameters
%gap_max = 1.7e-9; % Fitting parameter for Ireset
%gap_min = 0.2e-9; % Fitting parameter for Iset
%g0 = 0.25e-9; % Fitting parameter for slope rate of Iset/Ireset
%V0 = 0.25; % Fitting parameter for slope rate of Iset/Ireset
%I0 = 1000e-6; % Fitting paramete for current
% gamma0 = 28; %controls the Vset and Vreset (speed of increase/decrease gap)

%% Fit Device parameters:
% Vset = Vreset = 1.1V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 5MOhm
%gap_max = 2.10e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 13;
%Vel0 = 0.68;
%% Atom spacing
%a0 = 0.2e-10;
%Ea = 0.13;

%% Fit Device parameters:
% Vset = Vreset = 1.1V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 4MOhm
%gap_max = 2.05e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 13;
%Vel0 = 0.68;
%% Atom spacing
%a0 = 0.2e-10;
%Ea = 0.13;

%% Fit Device parameters:
% Vset = Vreset = 1.1V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 3MOhm
%gap_max = 1.98e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 13;
%Vel0 = 0.68;
%% Atom spacing
%a0 = 0.2e-10;
%Ea = 0.13;

%% Fit Device parameters:
% Vset = Vreset = 1.1V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 2MOhm
%gap_max = 1.90e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 13;
%Vel0 = 0.68;
%% Atom spacing
%a0 = 0.2e-10;
%Ea = 0.13;

%% Fit Device parameters:
% Vset = Vreset = 1.1V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 1MOhm
gap_max = 1.70e-9;
gap_min = 0.10e-9;
g0 = 0.25e-9;
V0 = 0.8;
I0 = 570e-6;
beta = 1e-2;
gamma0 = 13;
Vel0 = 0.68;
% Atom spacing
a0 = 0.2e-10;
Ea = 0.13;

%% Fit Device parameters:
% Vset = Vreset = 1.1V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 0.5MOhm
%gap_max = 1.55e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 13;
%Vel0 = 0.68;
%% Atom spacing
%a0 = 0.2e-10;
%Ea = 0.13;

%% Fit Device parameters:
% Vset = Vreset = 1.1V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 0.1MOhm
%gap_max = 1.15e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 13;
%Vel0 = 0.68;
%% Atom spacing
%a0 = 0.2e-10;
%Ea = 0.13;

%% Fit Device parameters:
% Vset = Vreset = 1.2V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 20MOhm
%gap_max = 2.48e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 12;
%Vel0 = 0.4;
% Atom spacing
%a0 = 0.35e-9/10;
%Ea = 0.12;

%% Fit Device parameters:
% Vset = Vreset = 1.1V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 23MOhm
%gap_max = 2.48e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 13;
%Vel0 = 0.68;
% Atom spacing
%a0 = 0.2e-10;
%Ea = 0.13;

% Vset = Vreset = 1.0V
% Iset,max = 500uA
% RLRS = 2.4kOhm, RHRS = 23MOhm
%gap_max = 2.48e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 14;
%Vel0 = 0.7;
% Atom spacing
%a0 = 0.268e-10;
%Ea = 0.14;

%% Fit Device parameters:
% Vset = Vreset = 0.9V
% Iset,max = 500uA
% RLRS = 1.8kOhm, RHRS = 20MOhm
%gap_max = 2.48e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 17;
%Vel0 = 0.7;
% Atom spacing
%a0 = 0.268e-10;
%Ea = 0.165;

%% Fit Device parameters:
% Vset = Vreset = 0.6V
% Iset,max = 500uA
% RLRS = 1.8kOhm, RHRS = 20MOhm
%gap_max = 2.48e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 18;
%Vel0 = 0.7;
% Atom spacing
%a0 = 0.268e-10;
%Ea = 0.165;


%% Fit Device parameters:
% Vset = Vreset = 0.3V
% Iset,max = 500uA
% RLRS = 1.8kOhm, RHRS = 20MOhm
%gap_max = 2.48e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 48;
%Vel0 = 0.7;
% Atom spacing
%a0 = 0.268e-10;
%Ea = 0.165;

%% Fit Device parameters:
% Vset = Vreset = 0.2V
% Iset,max = 500uA
% RLRS = 1.8kOhm, RHRS = 20MOhm
%gap_max = 2.48e-9;
%gap_min = 0.10e-9;
%g0 = 0.25e-9;
%V0 = 0.8;
%I0 = 570e-6;
%beta = 1e-2;
%gamma0 = 75;
%Vel0 = 0.7;
% Atom spacing
%a0 = 0.268e-10;
%Ea = 0.165;

%% I-V equation
Vtb = -1.5:0.1:1.5;
%Vtb = -1.8:0.1:1.8;
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

disp_vtb_max = 1.4;
disp_vtb_min = -1.4;

%% I-V Data for Sweep V0
V0_sweep = [0.6,0.7,0.8]; % [0,10]
V0_sweep_legend = [{'V0=0.6'}, {'V0=0.7'}, {'V0=0.8'}];
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
set(gca,'xlim',[-1.4 1.4],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
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
set(gca,'xlim',[disp_vtb_min disp_vtb_max],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
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
set(gca,'xlim',[disp_vtb_min disp_vtb_max],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
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
set(gca,'xlim',[disp_vtb_min disp_vtb_max],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
ylabel('I_{TB} (A)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(g0_sweep_legend);
grid on

%% Fig. 5: plot Field - Vtb
gamma0_sweep = [gamma0,10,8]; % (0,20);
gamma0_sweep_legend = [{['gamma0=', num2str(gamma0)]}, {'gamma0=10'}, {'gamma0=8'}];

gamma = gamma0 - beta * (gap_max/1e-9)^3;
F = (gamma*abs(Vtb)/tox);

F_gamma0_1 = ((gamma0_sweep(1) - beta*(gap_max/1e-9)^3)*abs(Vtb)/tox);
F_gamma0_2 = ((gamma0_sweep(2) - beta*(gap_max/1e-9)^3)*abs(Vtb)/tox);
F_gamma0_3 = ((gamma0_sweep(3) - beta*(gap_max/1e-9)^3)*abs(Vtb)/tox);

fig_handle5 = figure;
plot(Vtb, F_gamma0_1/1e9, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, F_gamma0_2/1e9, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, F_gamma0_3/1e9, 'g-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, F_min/1e9, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('V_{TB} (V)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('Field strength(1e9), reach F_min will invoke set process', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca,'xlim',[disp_vtb_min disp_vtb_max],'Fontsize',16, 'FontWeight','bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(gamma0_sweep_legend);
grid on

%% Fig. 6: plot gap_ddt - Vtb
Vel0_sweep = [Vel0, 0.8, 0.9]; % (0,20);
Vel0_sweep_legend = [{['Vel0=',num2str(Vel0)]}, {'Vel0=0.8'}, {'Vel0=0.9'}];

T_cur = T_ini;
gap_ddt = zeros(1,length(Vtb));
gap_dt = zeros(1,length(Vtb));
for i = 1:1:length(Vtb)
  gap_ddt(i) = -Vel0*exp(-q*Ea/kb/T_cur)*sinh(gamma*a0/tox*q*Vtb(i)/kb/T_cur);
  gap_dt(i) = gap_ddt(i)*time_step; 
  gap_dt_Vel0_1(i) = gap_ddt(i)/(Vel0)*Vel0_sweep(1)*time_step; 
  gap_dt_Vel0_2(i) = gap_ddt(i)/(Vel0)*Vel0_sweep(2)*time_step; 
  gap_dt_Vel0_3(i) = gap_ddt(i)/(Vel0)*Vel0_sweep(3)*time_step; 
end

fig_handle6 = figure;
plot(Vtb, gap_dt_Vel0_1/gap_max, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, gap_dt_Vel0_2/gap_max, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, gap_dt_Vel0_3/gap_max, 'g-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('V_{TB} (V)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('gap change per time step (#. of gap_max)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(Vel0_sweep_legend);
grid on

%% Fig. 7: plot gamma - beta
beta_sweep = [beta, 0.02, 0.05]; % (0,20);
beta_sweep_legend = [{['beta=',num2str(beta)]}, {'beta=0.02'}, {'beta=0.05'}];

gap = gap_min: 0.1*(gap_max-gap_min): gap_max;
for i=1:1:length(gap)
gamma_beta_1(i) = gamma0 - beta_sweep(1)*((gap(i)/(1e-9))^3);
gamma_beta_2(i) = gamma0 - beta_sweep(2)*((gap(i)/(1e-9))^3);
gamma_beta_3(i) = gamma0 - beta_sweep(3)*((gap(i)/(1e-9))^3);
end
fig_handle7 = figure;
plot(gap, gamma_beta_1, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(gap, gamma_beta_2, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(gap, gamma_beta_3, 'g-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('gap', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('gamma', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(beta_sweep_legend);
grid on

%% Fig. 8: plot gap_ddt - Vtb
Ea_sweep = [Ea, 0.12, 0.15]; % (0,1);
Ea_sweep_legend = [{['Ea=',num2str(Ea)]}, {'Ea=0.12'}, {'Ea=0.15'}];

T_cur = T_ini;
gap_ddt = zeros(1,length(Vtb));
gap_dt = zeros(1,length(Vtb));
for i = 1:1:length(Vtb)
  gap_ddt_Ea_1(i) = -Vel0*exp(-q*Ea_sweep(1)/kb/T_cur)*sinh(gamma*a0/tox*q*Vtb(i)/kb/T_cur);
  gap_dt_Ea_1(i) = gap_ddt_Ea_1(i)*time_step; 
  gap_ddt_Ea_2(i) = -Vel0*exp(-q*Ea_sweep(2)/kb/T_cur)*sinh(gamma*a0/tox*q*Vtb(i)/kb/T_cur);
  gap_dt_Ea_2(i) = gap_ddt_Ea_2(i)*time_step; 
  gap_ddt_Ea_3(i) = -Vel0*exp(-q*Ea_sweep(3)/kb/T_cur)*sinh(gamma*a0/tox*q*Vtb(i)/kb/T_cur);
  gap_dt_Ea_3(i) = gap_ddt_Ea_3(i)*time_step; 
end

fig_handle8 = figure;
plot(Vtb, gap_dt_Ea_1/gap_max, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, gap_dt_Ea_2/gap_max, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, gap_dt_Ea_3/gap_max, 'g-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('V_{TB} (V)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('gap change per time step (#. of gap_max)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(Ea_sweep_legend);
grid on

%% Fig. 9: plot gap_ddt - a0
a0_sweep = [a0, 0.3e-10, 0.35e-10]; % (0,1);
a0_sweep_legend = [{['a0=',num2str(a0)]}, {'a0=0.3e-10'}, {'a0=0.35e-10'}];

T_cur = T_ini;
gap_ddt = zeros(1,length(Vtb));
gap_dt = zeros(1,length(Vtb));
for i = 1:1:length(Vtb)
  gap_ddt_a0_1(i) = -Vel0*exp(-q*Ea/kb/T_cur)*sinh(gamma*a0_sweep(1)/tox*q*Vtb(i)/kb/T_cur);
  gap_dt_a0_1(i) = gap_ddt_a0_1(i)*time_step; 
  gap_ddt_a0_2(i) = -Vel0*exp(-q*Ea/kb/T_cur)*sinh(gamma*a0_sweep(2)/tox*q*Vtb(i)/kb/T_cur);
  gap_dt_a0_2(i) = gap_ddt_a0_2(i)*time_step; 
  gap_ddt_a0_3(i) = -Vel0*exp(-q*Ea/kb/T_cur)*sinh(gamma*a0_sweep(3)/tox*q*Vtb(i)/kb/T_cur);
  gap_dt_a0_3(i) = gap_ddt_a0_3(i)*time_step; 
end

fig_handle8 = figure;
plot(Vtb, gap_dt_a0_1/gap_max, 'b-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, gap_dt_a0_2/gap_max, 'r-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
plot(Vtb, gap_dt_a0_3/gap_max, 'g-*', 'LineWidth', 2, 'MarkerSize', 10);
hold on
xlabel('V_{TB} (V)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
ylabel('gap change per time step (#. of gap_max)', 'FontSize', 18, 'FontWeight', 'bold', 'FontName', 'Times');
set(gca, 'Fontsize', 16, 'FontWeight', 'bold', 'FontName', 'Times');
hleg = legend(a0_sweep_legend);
grid on
