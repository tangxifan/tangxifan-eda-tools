% one-level 
area_rram_mux_improv_2n1r_1level = 2:2:50;
pn_ratio = 1;
prog_pn_ratio = 1;
pullup_trans_size = 0;
for i=1:1:length(area_rram_mux_improv_2n1r_1level)
  area_multiplexing = (2*i+1)*(trans_area(1)+ trans_area(prog_pn_ratio));
  area_pullup_trans = 0*(trans_area(pullup_trans_size)+ trans_area(pullup_trans_size));

  area_buf = (2*i + 1) * (trans_area(1)+ trans_area(1*pn_ratio));

  area_rram_mux_improv_2n1r_1level(i) = area_multiplexing + area_buf + area_pullup_trans;
end

% Delay and power when VDD=0.5V 
rram_mux_improv_2n1r_1level_0p5V = [ % size, #. fins, delay, leakage, dynamic_power, energy_per_toggle
% Input size = 2
2,3,2.585e-11,1.984e-08,1.604e-06,7.3805e-17;
% input size = 4
% input size = 6
6,3,2.731e-11,4.57e-08,1.6125e-06,7.748e-17;
% input size = 8
8,3,2.77e-11,6.172e-08,1.638e-06,8.0705e-17;
% input size = 10
10,3,2.897e-11,7.319e-08,1.6305e-06,8.109e-17;
% input size = 12
% input size = 14
14,3,3.02e-11,1.035e-07,1.651e-06,8.5085e-17;
% input size = 16
16,3,3.056e-11,1.132e-07,1.675e-06,8.693e-17;
% input size = 18
% input size = 20
% input size = 22
% input size = 24
24,3,3.8135e-11,1.683e-07,1.533e-06,9.5755e-17;
% input size = 26
26,3,3.47e-11,1.739e-07,1.702e-06,9.7415e-17;
% input size = 28
28,3,3.5315e-11,1.932e-07,1.7125e-06,9.958e-17;
% input size = 30
% input size = 32
% input size = 34
% input size = 36
% input size = 38
% input size = 40
% input size = 42
% input size = 44
% input size = 46
% input size = 48
% input size = 50
];

% Delay and power when VDD=0.6V 
rram_mux_improv_2n1r_1level_0p6V = [ % size, #. of fins, delay, leakage, dynamic_power, energy_per_toggle
% Input size = 2
2,3,1.957e-11,2.57e-08,3.104e-06,1.11955e-16;
% input size = 4
% input size = 6
6,3,2.0955e-11,5.994e-08,3.104e-06,1.167e-16;
% input size = 8
8,3,2.1645e-11,7.706e-08,3.0995e-06,1.19145e-16;
% input size = 10
10,3,2.223e-11,9.423e-08,3.108e-06,1.21645e-16;
% input size = 12
% input size = 14
14,3,2.3165e-11,1.284e-07,3.1375e-06,1.258e-16;
% input size = 16
16,3,2.3385e-11,1.454e-07,3.148e-06,1.2835e-16;
% input size = 18
% input size = 20
% input size = 22
% input size = 24
24,3,2.459e-11,2.14e-07,3.199e-06,1.3725e-16;
% input size = 26
26,3,2.58e-11,2.311e-07,3.2105e-06,1.404e-16;
% input size = 28
28,3,2.631e-11,2.479e-07,3.2255e-06,1.428e-16;
% input size = 30
% input size = 32
% input size = 34
% input size = 36
% input size = 38
% input size = 40
% input size = 42
% input size = 44
% input size = 46
% input size = 48
% input size = 50
];

% Delay and power when VDD=0.7V 
rram_mux_basic_2n1r_1level_0p7V = [ % size, #. of fins, delay, leakage, dynamic_power, energy_per_toggle
% Input size = 2
2,3,1.609e-11,3.252e-08,5.1445e-06,1.5965e-16;
% input size = 4
2,3,1.638e-11,5.407e-08,5.1545e-06,1.6225e-16;
% input size = 6
6,3,1.672e-11,7.561e-08,5.143e-06,1.653e-16;
% input size = 8
8,3,1.772e-11,9.716e-08,5.157e-06,1.68e-16;
% input size = 10
10,3,1.8255e-11,1.187e-07,5.1105e-06,1.712e-16;
% input size = 12
12,3,1.814e-11,1.403e-07,5.1605e-06,1.7405e-16;
% input size = 14
14,3,1.8855e-11,1.618e-07,5.1735e-06,1.7655e-16;
% input size = 16
16,3,1.9265e-11,1.831e-07,5.1965e-06,1.795e-16;
% input size = 18
18,3,1.996e-11,2.049e-07,5.191e-06,1.832e-16;
% input size = 20
20,3,2.0335e-11,2.261e-07,5.2185e-06,1.8545e-16;
% input size = 22
22,3,2.0165e-11,2.477e-07,5.1635e-06,1.8925e-16;
% input size = 24
24,3,2.07e-11,2.692e-07,5.243e-06,1.917e-16;
% input size = 26
26,3,2.0935e-11,2.907e-07,5.2605e-06,1.943e-16;
% input size = 28
28,3,2.1285e-11,3.126e-07,5.2785e-06,1.974e-16;
% input size = 30
30,3,2.142e-11,3.342e-07,5.272e-06,2.0025e-16;
% input size = 32
32,3,2.188e-11,3.552e-07,5.301e-06,2.0265e-16;
% input size = 34
34,3,2.2395e-11,3.773e-07,5.1855e-06,2.06e-16;
% input size = 36
36,3,2.225e-11,3.988e-07,5.282e-06,2.1e-16;
% input size = 38
38,3,2.2555e-11,4.203e-07,5.28e-06,2.136e-16;
% input size = 40
40,3,2.3145e-11,4.419e-07,5.2905e-06,2.178e-16;
% input size = 42
42,3,2.3495e-11,4.634e-07,5.309e-06,2.2115e-16;
% input size = 44
44,3,2.385e-11,4.85e-07,5.3315e-06,2.242e-16;
% input size = 46
46,3,2.452e-11,5.065e-07,5.4485e-06,2.2385e-16;
% input size = 48
48,3,2.4595e-11,5.281e-07,5.3975e-06,2.2925e-16;
% input size = 50
50,3,2.486e-11,5.496e-07,5.3785e-06,2.3145e-16;
];


% Delay and power when VDD=0.7V 
rram_mux_naive_2n1r_1level_0p7V = [ % size, #. of fins, delay, leakage, dynamic_power, energy_per_toggle
% Input size = 2
2,3,1.709e-11,3.055e-08,3.6385e-06,1.056e-16;
% input size = 4
4,3,1.8115e-11,5.117e-08,4.2095e-06,1.2565e-16;
% input size = 6
6,3,2.5483e-11,7.172e-08,4.929e-06,1.944e-16;
% input size = 8
% input size = 10
% input size = 12
% input size = 14
% input size = 16
% input size = 18
% input size = 20
% input size = 22
% input size = 24
% input size = 26
% input size = 28
% input size = 30
% input size = 32
% input size = 34
% input size = 36
% input size = 38
% input size = 40
% input size = 42
% input size = 44
% input size = 46
% input size = 48
% input size = 50
];
