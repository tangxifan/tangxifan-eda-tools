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

% No. of fins = 1, no parasitics 
rram_mux_naive_1level_0p7V = [
% input size 2
2,3,8.9585e-12,1.545e-08,6.3205e-06,1.2195e-16;
% input size 4
4,3,9.007e-12,2.576e-08,6.678e-06,1.3045e-16;
% input size 6
6,3,9.0275e-12,3.607e-08,7.009e-06,1.3865e-16;
% input size = 8
8,3,9.278e-12,4.638e-08,7.3565e-06,1.4745e-16;
% input size = 10
10,3,9.5375e-12,5.669e-08,7.6525e-06,1.557e-16;
% input size = 12
12,3,9.7515e-12,6.7e-08,7.9315e-06,1.6485e-16;
% input size = 14
14,3,9.8975e-12,7.731e-08,8.1905e-06,1.75e-16;
% input size = 16
16,3,1.01625e-11,8.762e-08,8.5605e-06,1.875e-16;
% input size = 18
18,3,1.0279e-11,9.793e-08,8.692e-06,1.948e-16;
% input size = 20
20,3,1.0807e-11,1.082e-07,8.936e-06,2.0765e-16;
% input size = 22
22,3,1.1225e-11,1.185e-07,9.1615e-06,2.213e-16;
% input size = 24
24,3,1.1511e-11,1.289e-07,9.3965e-06,2.368e-16;
% input size = 26
26,3,1.19825e-11,1.392e-07,9.475e-06,2.532e-16;
% input size = 28
28,3,1.2995e-11,1.495e-07,9.842e-06,2.802e-16;
% input size = 30
30,3,1.3795e-11,1.598e-07,1.00805e-05,3.087e-16;
% input size = 32
32,3,1.44285e-11,1.701e-07,1.03235e-05,3.4705e-16;
% input size = 34
34,3,1.6622e-11,1.804e-07,1.0569e-05,4.0295e-16;
% input size = 36
36,3,1.86e-11,1.907e-07,1.11175e-05,5.062e-16;
% input size = 38
38,3,2.3443e-11,2.01e-07,1.1445e-05,7.298e-16;
% input size = 40
% input size = 42
% input size = 44
% input size = 46
% input size = 48
% input size = 50
];
