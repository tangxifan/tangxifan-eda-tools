% one-level 
area_rram_mux_isolate_one_level = 2:2:32;
pn_ratio = 1.9;
prog_pn_ratio = 3;
isol_trans_size = 1;
for i=1:1:length(area_rram_mux_isolate_one_level)
  area_multiplexing = (2*i+1) * (2*trans_area(1)+ 2*trans_area(prog_pn_ratio) + 2*trans_area(isol_trans_size));

  area_buf = (2*i + 1) * (trans_area(2)+ trans_area(2*pn_ratio));

  area_rram_mux_isolate_one_level(i) = area_multiplexing + area_buf;
end

% Delay and power when VDD=0.7V 
rram_mux_isolate_one_level_0p7V = [
2, 5.6205e-11,2.208e-08,5.8935e-05,3.497e-15;
4, 6.242e-11,3.707e-08,5.844e-05,3.4585e-15;
6, 6.427e-11,5.202e-08,5.8235e-05,3.4845e-15;
8, 6.5475e-11,6.698e-08,5.824e-05,3.503e-15;
10, 6.5915e-11,8.194e-08,5.8275e-05,3.499e-15;
12, 6.645e-11,9.736e-08,5.801e-05,3.5445e-15;
14, 6.671e-11,1.119e-07,5.801e-05,3.5475e-15;
16, 6.6885e-11,1.269e-07,5.805e-05,3.548e-15;
18, 6.697e-11,1.419e-07,5.8095e-05,3.5415e-15;
20, 6.7185e-11,1.569e-07,5.8105e-05,3.5855e-15;
22, 6.729e-11,1.719e-07,5.8125e-05,3.5875e-15;
24, 6.7375e-11,1.87e-07,5.814e-05,3.59e-15;
26, 6.7445e-11,2.02e-07,5.8185e-05,3.5935e-15;
28, 6.725e-11,1.66e-07,5.836e-05,3.551e-15;
30, 6.7545e-11,2.303e-07,5.8235e-05,3.597e-15;
32, 6.757e-11,2.457e-07,5.8335e-05,3.5945e-15;
];

% Delay and power when VDD=0.8V 
rram_mux_isolate_one_level_0p8V = [
2, 4.559e-11,2.781e-08,7.819e-05,4.0385e-15;
4, 5.079e-11,4.642e-08,7.657e-05,4.0945e-15;
6, 5.268e-11,6.53e-08,7.6795e-05,4.13e-15;
8, 5.3365e-11,8.591e-08,7.6335e-05,4.136e-15;
10, 5.3705e-11,1.031e-07,7.594e-05,4.17e-15;
12, 5.402e-11,1.219e-07,7.5995e-05,4.1665e-15;
14, 5.4405e-11,1.408e-07,7.6185e-05,4.177e-15;
16, 5.4565e-11,1.597e-07,7.6195e-05,4.1855e-15;
18, 5.4695e-11,1.785e-07,7.6225e-05,4.1935e-15;
20, 5.48e-11,1.974e-07,7.626e-05,4.2e-15;
22, 5.4875e-11,2.163e-07,7.627e-05,4.2065e-15;
24, 5.493e-11,2.352e-07,7.6305e-05,4.2125e-15;
26, 5.5135e-11,2.541e-07,7.705e-05,4.2745e-15;
28, 5.512e-11,2.73e-07,7.699e-05,4.2805e-15;
30, 5.5055e-11,2.919e-07,7.696e-05,4.2885e-15;
32, 5.517e-11,3.108e-07,7.68e-05,4.2245e-15;
];

% Delay and power when VDD=0.9V 
rram_mux_isolate_one_level_0p9V = [
2, 3.9245e-11,3.581e-08,9.9975e-05,4.6455e-15;
4, 4.419e-11,6.026e-08,9.7265e-05,4.854e-15;
6, 4.6175e-11,8.46e-08,9.678e-05,4.7385e-15;
8, 4.6965e-11,1.089e-07,9.654e-05,4.801e-15;
10, 4.7765e-11,1.333e-07,9.775e-05,4.909e-15;
12, 4.7845e-11,1.578e-07,9.699e-05,4.911e-15;
14, 4.8325e-11,1.823e-07,9.767e-05,4.9255e-15;
16, 4.8485e-11,2.066e-07,9.7445e-05,4.9295e-15;
18, 4.855e-11,2.31e-07,9.7255e-05,4.939e-15;
20, 4.835e-11,2.554e-07,9.685e-05,4.888e-15;
22, 4.843e-11,2.797e-07,9.689e-05,4.895e-15;
24, 4.8495e-11,3.041e-07,9.6915e-05,4.901e-15;
26, 4.855e-11,3.285e-07,9.6945e-05,4.907e-15;
28, 4.8585e-11,3.529e-07,9.7005e-05,4.9115e-15;
30, 4.8945e-11,3.803e-07,9.8265e-05,4.992e-15;
32, 4.865e-11,4.016e-07,9.7085e-05,4.9215e-15;
];