%% Format
% input size, number of fin in each transistor, delay, leakage power, dynamic power, energy per toggle;

%% RRAM MUX improved design, long wires is placed close to input inverters
rram_mux_naive_1level_parasitic_0p5V = [
% input size = 2
2,3,2.2445e-11,9.821e-10,2.524e-06,1.02825e-16;
% input size 4
4,3,2.3395e-11,1.636e-09,2.6235e-06,1.11015e-16;
% input size 6
6,3,2.3555e-11,2.247e-09,2.752e-06,1.2075e-16;
% input size 8
8,3,2.5605e-11,2.957e-09,2.846e-06,1.3255e-16;
% input size 10
10,3,2.7405e-11,3.641e-09,2.926e-06,1.48e-16;
% input size 12
12,3,3.2925e-11,4.327e-09,2.928e-06,1.635e-16;
% input size 14
14,3,3.3875e-11,4.817e-09,3.0365e-06,1.9245e-16;
% input size 16
16,3,3.818e-11,5.666e-09,3.148e-06,2.279e-16;
];

rram_mux_naive_1level_parasitic_0p6V = [
% input size = 2
2,3,1.741e-11,1.306e-09,4.7315e-06,1.556e-16;
% input size 4
4,3,1.7995e-11,2.181e-09,4.947e-06,1.684e-16;
% input size 6
6,3,1.949e-11,3.088e-09,5.146e-06,1.818e-16;
% input size 8
8,3,2.0905e-11,3.931e-09,5.2105e-06,1.9825e-16;
% input size 10
10,3,2.2365e-11,4.873e-09,5.4105e-06,2.196e-16;
% input size 12
12,3,2.4445e-11,5.764e-09,5.5545e-06,2.462e-16;
% input size 14
14,3,2.6545e-11,6.655e-09,5.6395e-06,2.8225e-16;
% input size 16
16,3,3.0735e-11,7.547e-09,5.772e-06,3.347e-16;
];

rram_mux_naive_1level_parasitic_0p7V = [
% input size = 2
2,3,1.4825e-11,1.684e-09,8.059e-06,2.36e-16;
% input size 4
4,3,1.6045e-11,2.83e-09,8.3955e-06,2.5525e-16;
% input size 6
6,3,1.773e-11,3.978e-09,8.6625e-06,2.8035e-16;
% input size 8
8,3,1.885e-11,5.129e-09,8.8725e-06,3.0785e-16;
% input size 10
10,3,2.1085e-11,6.274e-09,9.113e-06,3.576e-1;
% input size 12
12,3,2.42865e-11,7.427e-09,9.661e-06,4.3465e-16;
% input size 14
14,3,3.2738e-11,8.574e-09,9.905e-06,5.9155e-16;
% input size 16
% input size 18
];
