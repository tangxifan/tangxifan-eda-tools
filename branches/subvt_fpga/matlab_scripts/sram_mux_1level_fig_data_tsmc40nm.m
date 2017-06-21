%% Simulation results of SRAM MUX APSP 7nm FinFET
% Size <= 12, use area, delay and power of one-level mux
% Size > 12, use area, delay and power of two-level mux

% Number of swept Parameter
% Input size swept from 2 to 50
num_swept_wcpt = 11;
num_swept_input_size = 25;
num_VDD = 3;
num_merits = 5;

%% Data filling
sram_mux_1level = zeros( num_merits, num_swept_wcpt, num_swept_input_size, num_VDD); 

% 16 index: 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32

% input size = 2
% VDD = 0.7V 
sram_mux_1level(:, :, 1, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.7695e-11,1.83151e-06,1.5597e-05,5.8145e-16,,;

2,2.76e-11,6.677e-06,2.695e-05,9.832e-16,,;

3,2.906e-11,9.285e-06,2.982e-05,1.129e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 1, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.0535e-11,4.05276e-06,2.571e-05,8.797e-16,,;

2,2.007e-11,1.462e-05,4.453e-05,1.513e-15,,;

3,2.1e-11,2.01e-05,4.996e-05,1.72e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 1, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.628e-11,7.70364e-06,3.8445e-05,1.2896e-15,,;

2,1.565e-11,2.752e-05,6.45e-05,2.16e-15,,;

3,1.632e-11,3.748e-05,7.393e-05,2.475e-15,,;
];

% input size = 4
% VDD = 0.7V 
sram_mux_1level(:, :, 2, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.6415e-11,3.6745e-08,1.55525e-05,6.449e-16,,;

2,3.754e-11,5.291e-08,2.87e-05,1.178e-15,,;

3,4.034e-11,6.437e-08,3.197e-05,1.353e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 2, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.6275e-11,5.448e-08,2.582e-05,9.588e-16,,;

2,2.664e-11,7.724e-08,4.63e-05,1.707e-15,,;

3,2.836e-11,9.334e-08,5.17e-05,1.932e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 2, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.077e-11,7.888e-08,3.937e-05,1.3759e-15,,;

2,2.077e-11,1.104e-07,6.951e-05,2.383e-15,,;

3,2.235e-11,1.325e-07,7.824e-05,2.732e-15,,;
];

% input size = 6
% VDD = 0.7V 
sram_mux_1level(:, :, 3, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.3945e-11,5.5215e-08,1.61345e-05,7.5085e-16,,;

2,4.516e-11,8.262e-08,3.016e-05,1.367e-15,,;

3,4.887e-11,1.017e-07,3.375e-05,1.617e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 3, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.118e-11,8.1555e-08,2.684e-05,1.0636e-15,,;

2,3.146e-11,1.201e-07,4.927e-05,1.951e-15,,;

3,3.384e-11,1.469e-07,5.529e-05,2.21e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 3, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.454e-11,1.176e-07,4.122e-05,1.5075e-15,,;

2,2.435e-11,1.707e-07,7.477e-05,2.703e-15,,;

3,2.621e-11,2.076e-07,8.412e-05,3.125e-15,,;
];

% input size = 8
% VDD = 0.7V 
sram_mux_1level(:, :, 4, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.106e-11,7.368e-08,1.67025e-05,8.2925e-16,,;

2,5.216e-11,1.123e-07,3.129e-05,1.551e-15,,;

3,5.683e-11,1.39e-07,3.524e-05,1.799e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 4, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.598e-11,1.0861e-07,2.7835e-05,1.171e-15,,;

2,3.58e-11,1.628e-07,5.173e-05,2.165e-15,,;

3,3.908e-11,2.003e-07,5.835e-05,2.561e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 4, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.8135e-11,1.563e-07,4.283e-05,1.6606e-15,,;

2,2.759e-11,2.31e-07,7.893e-05,3.014e-15,,;

3,2.99e-11,2.825e-07,8.875e-05,3.424e-15,,;
];

% input size = 10
% VDD = 0.7V 
sram_mux_1level(:, :, 5, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.79e-11,9.217e-08,1.7115e-05,9.2295e-16,,;

2,5.92e-11,1.419e-07,3.224e-05,1.724e-15,,;

3,6.448e-11,1.762e-07,3.633e-05,2.037e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 5, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.038e-11,1.357e-07,2.8775e-05,1.2834e-15,,;

2,4.03e-11,2.056e-07,5.385e-05,2.364e-15,,;

3,4.415e-11,2.537e-07,6.11e-05,2.843e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 5, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.139e-11,1.9505e-07,4.4385e-05,1.7731e-15,,;

2,3.08e-11,2.912e-07,8.241e-05,3.252e-15,,;

3,3.365e-11,3.573e-07,9.268e-05,3.71e-15,,;
];


% input size = 12
% VDD = 0.7V 
sram_mux_1level(:, :, 6, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.4775e-11,1.106e-07,1.76925e-05,1.0153e-15,,;

2,6.563e-11,1.716e-07,3.308e-05,1.884e-15,,;

3,7.228e-11,2.133e-07,3.733e-05,2.225e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 6, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.4955e-11,1.6275e-07,2.963e-05,1.4214e-15,,;

2,4.444e-11,2.483e-07,5.567e-05,2.587e-15,,;

3,4.896e-11,3.07e-07,6.313e-05,2.996e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 6, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.4705e-11,2.337e-07,4.5685e-05,1.90295e-15,,;

2,3.402e-11,3.514e-07,8.552e-05,3.575e-15,,;

3,3.711e-11,4.321e-07,9.635e-05,4.064e-15,,;
];

% input size = 14
% VDD = 0.7V 
sram_mux_1level(:, :, 7, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.1265e-11,1.2905e-07,1.80555e-05,1.10055e-15,,;

2,7.202e-11,2.011e-07,3.376e-05,2.054e-15,,;

3,7.947e-11,2.504e-07,3.818e-05,2.416e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 7, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.9435e-11,1.8985e-07,3.0465e-05,1.4958e-15,,;

2,4.865e-11,2.91e-07,5.724e-05,2.818e-15,,;

3,5.355e-11,3.602e-07,6.5e-05,3.282e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 7, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.803e-11,2.724e-07,4.6995e-05,2.043e-15,,;

2,3.706e-11,4.115e-07,8.81e-05,3.719e-15,,;

3,4.072e-11,5.067e-07,9.953e-05,4.399e-15,,;
];

% input size = 16
% VDD = 0.7V 
sram_mux_1level(:, :, 8, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.78e-11,1.475e-07,1.83615e-05,1.18205e-15,,;

2,7.849e-11,2.307e-07,3.436e-05,2.22e-15,,;

3,8.65e-11,2.874e-07,3.891e-05,2.616e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 8, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.365e-11,2.1685e-07,3.1185e-05,1.6048e-15,,;

2,5.297e-11,3.336e-07,5.878e-05,2.963e-15,,;

3,5.837e-11,4.133e-07,6.664e-05,3.537e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 8, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.1175e-11,3.1105e-07,4.812e-05,2.139e-15,,;

2,3.998e-11,4.716e-07,9.056e-05,3.992e-15,,;

3,4.423e-11,5.812e-07,0.0001021,4.592e-15,,;
];

% input size = 18
% VDD = 0.7V 
sram_mux_1level(:, :, 9, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,8.407e-11,1.6595e-07,1.8628e-05,1.2634e-15,,;

2,8.485e-11,2.602e-07,3.484e-05,2.411e-15,,;

3,9.404e-11,3.244e-07,3.953e-05,2.825e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 9, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.7725e-11,2.4385e-07,3.1825e-05,1.70765e-15,,;

2,5.701e-11,3.762e-07,6.006e-05,3.186e-15,,;

3,6.317e-11,4.664e-07,6.857e-05,3.799e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 9, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.4395e-11,3.4975e-07,4.9235e-05,2.287e-15,,;

2,4.285e-11,5.316e-07,9.274e-05,4.245e-15,,;

3,4.757e-11,6.556e-07,0.0001049,4.968e-15,,;
];

% input size = 20
% VDD = 0.7V 
sram_mux_1level(:, :, 10, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,9.0175e-11,1.8435e-07,1.882e-05,1.3829e-15,,;

2,9.103e-11,2.897e-07,3.546e-05,2.523e-15,,;

3,1.011e-10,3.613e-07,4.018e-05,2.989e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 10, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.1775e-11,2.709e-07,3.2375e-05,1.82315e-15,,;

2,6.085e-11,4.187e-07,6.122e-05,3.371e-15,,;

3,6.791e-11,5.194e-07,6.995e-05,3.959e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 10, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.73e-11,3.884e-07,5.032e-05,2.441e-15,,;

2,4.606e-11,5.915e-07,9.492e-05,4.554e-15,,;

3,5.079e-11,7.3e-07,0.0001072,5.261e-15,,;
];

% input size = 22
% VDD = 0.7V 
sram_mux_1level(:, :, 11, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,9.6315e-11,2.028e-07,1.9125e-05,1.4567e-15,,;

2,9.705e-11,3.192e-07,3.598e-05,2.653e-15,,;

3,1.078e-10,3.982e-07,4.08e-05,3.132e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 11, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.5965e-11,2.979e-07,3.2985e-05,1.9645e-15,,;

2,6.464e-11,4.612e-07,6.226e-05,3.572e-15,,;

3,7.243e-11,5.723e-07,7.119e-05,4.181e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 11, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.05e-11,4.2705e-07,5.1285e-05,2.5985e-15,,;

2,4.914e-11,6.514e-07,9.668e-05,4.619e-15,,;

3,5.403e-11,8.042e-07,0.0001093,5.584e-15,,;
];

% input size = 24
% VDD = 0.7V 
sram_mux_1level(:, :, 12, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.02455e-10,2.212e-07,1.9395e-05,1.5292e-15,,;

2,1.032e-10,3.486e-07,3.633e-05,2.826e-15,,;

3,1.146e-10,4.35e-07,4.127e-05,3.325e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 12, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.0145e-11,3.2495e-07,3.3365e-05,2.114e-15,,;

2,6.841e-11,5.037e-07,6.317e-05,3.798e-15,,;

3,7.679e-11,6.252e-07,7.231e-05,4.44e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 12, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.369e-11,4.6565e-07,5.225e-05,2.6815e-15,,;

2,5.2e-11,7.112e-07,9.845e-05,4.895e-15,,;

3,5.766e-11,8.783e-07,0.0001115,5.992e-15,,;
];


% input size = 26
% VDD = 0.7V 
sram_mux_1level(:, :, 13, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.09e-10,2.396e-07,1.9735e-05,1.5764e-15,,;

2,1.093e-10,3.78e-07,3.688e-05,2.904e-15,,;

3,1.216e-10,4.718e-07,4.176e-05,3.482e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 13, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.425e-11,3.519e-07,3.3785e-05,2.212e-15,,;

2,7.214e-11,5.461e-07,6.399e-05,4.03e-15,,;

3,8.107e-11,6.78e-07,7.336e-05,4.662e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 13, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.7005e-11,5.043e-07,5.275e-05,2.729e-15,,;

2,5.474e-11,7.71e-07,0.0001001,5.187e-15,,;

3,6.126e-11,9.524e-07,0.0001127,5.862e-15,,;
];

% input size = 28
% VDD = 0.7V 
sram_mux_1level(:, :, 14, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.1515e-10,2.5805e-07,1.994e-05,1.64635e-15,,;

2,1.153e-10,4.074e-07,3.724e-05,3.042e-15,,;

3,1.286e-10,5.085e-07,4.215e-05,3.669e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 14, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.825e-11,3.7895e-07,3.419e-05,2.3285e-15,,;

2,7.58e-11,5.885e-07,6.472e-05,4.487e-15,,;

3,8.531e-11,7.307e-07,7.431e-05,4.896e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 14, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.0075e-11,5.429e-07,5.3455e-05,2.868e-15,,;

2,5.747e-11,8.308e-07,0.0001015,5.422e-15,,;

3,6.457e-11,1.026e-06,0.0001146,6.206e-15,,;
];

% input size = 30
% VDD = 0.7V 
sram_mux_1level(:, :, 15, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.211e-10,2.7645e-07,2.0115e-05,1.71885e-15,,;

2,1.212e-10,4.367e-07,3.749e-05,3.207e-15,,;

3,1.353e-10,5.451e-07,4.243e-05,3.892e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 15, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,8.249e-11,4.059e-07,3.473e-05,2.405e-15,,;

2,8.005e-11,6.308e-07,6.564e-05,4.605e-15,,;

3,8.955e-11,7.833e-07,7.517e-05,5.152e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 15, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.303e-11,5.815e-07,5.414e-05,2.9945e-15,,;

2,6.023e-11,8.904e-07,0.0001029,5.695e-15,,;

3,6.772e-11,1.1e-06,0.0001162,6.551e-15,,;
];

% input size = 32
% VDD = 0.7V 
sram_mux_1level(:, :, 16, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.2715e-10,2.948e-07,2.0305e-05,1.78105e-15,,;

2,1.272e-10,4.66e-07,3.756e-05,3.442e-15,,;

3,1.419e-10,5.817e-07,4.305e-05,3.929e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 16, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,8.679e-11,4.329e-07,3.5375e-05,2.4125e-15,,;

2,8.429e-11,6.731e-07,6.663e-05,4.612e-15,,;

3,9.413e-11,8.359e-07,7.598e-05,5.383e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 16, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.598e-11,6.201e-07,5.476e-05,3.128e-15,,;

2,6.301e-11,9.501e-07,0.0001042,5.978e-15,,;

3,7.082e-11,1.174e-06,0.0001177,6.878e-15,,;
];

% input size = 34
% VDD = 0.7V 
sram_mux_1level(:, :, 17, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.3315e-10,3.1325e-07,2.044e-05,1.858e-15,,;

2,1.33e-10,4.953e-07,3.817e-05,3.443e-15,,;

3,1.483e-10,6.183e-07,4.332e-05,4.127e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 17, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,9.085e-11,4.5985e-07,3.5805e-05,2.48e-15,,;

2,8.837e-11,7.154e-07,6.759e-05,4.553e-15,,;

3,9.889e-11,8.884e-07,7.682e-05,5.505e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 17, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.896e-11,6.5865e-07,5.5325e-05,3.271e-15,,;

2,6.58e-11,1.01e-06,0.0001055,6.582e-15,,;

3,7.394e-11,1.248e-06,0.0001192,7.254e-15,,;
];

% input size = 36
% VDD = 0.7V 
sram_mux_1level(:, :, 18, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.3905e-10,3.3165e-07,2.054e-05,1.9455e-15,,;

2,1.387e-10,5.246e-07,3.851e-05,3.544e-15,,;

3,1.55e-10,6.548e-07,4.364e-05,4.289e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 18, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,9.4795e-11,4.8685e-07,3.611e-05,2.566e-15,,;

2,9.226e-11,7.576e-07,6.819e-05,4.805e-15,,;

3,1.035e-10,9.408e-07,7.753e-05,5.768e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 18, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.197e-11,6.972e-07,5.5835e-05,3.41e-15,,;

2,6.858e-11,1.069e-06,0.0001067,6.908e-15,,;

3,7.709e-11,1.321e-06,0.0001205,7.538e-15,,;
];

% input size = 38
% VDD = 0.7V 
sram_mux_1level(:, :, 19, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.449e-10,3.5e-07,2.0665e-05,2.026e-15,,;

2,1.444e-10,5.538e-07,3.873e-05,3.698e-15,,;

3,1.618e-10,6.912e-07,4.402e-05,4.408e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 19, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,9.873e-11,5.138e-07,3.6405e-05,2.6525e-15,,;

2,9.598e-11,7.998e-07,6.885e-05,4.94e-15,,;

3,1.079e-10,9.932e-07,7.833e-05,5.827e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 19, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.494e-11,7.3575e-07,5.6275e-05,3.675e-15,,;

2,7.135e-11,1.129e-06,0.0001078,7.242e-15,,;

3,8.025e-11,1.395e-06,0.0001217,7.87e-15,,;
];


% input size = 40
% VDD = 0.7V 
sram_mux_1level(:, :, 20, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.508e-10,3.6835e-07,2.085e-05,2.083e-15,,;

2,1.501e-10,5.83e-07,3.891e-05,3.863e-15,,;

3,1.684e-10,7.276e-07,4.428e-05,4.584e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 20, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.0276e-10,5.4075e-07,3.668e-05,2.7335e-15,,;

2,9.963e-11,8.42e-07,6.943e-05,5.098e-15,,;

3,1.121e-10,1.045e-06,7.899e-05,6.045e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 20, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.7915e-11,7.7435e-07,5.677e-05,3.8315e-15,,;

2,7.465e-11,1.188e-06,0.000109,7.242e-15,,;

3,8.365e-11,1.468e-06,0.0001228,8.188e-15,,;
];


% input size = 42
% VDD = 0.7V 
sram_mux_1level(:, :, 21, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.5685e-10,3.867e-07,2.115e-05,2.0965e-15,,;

2,1.56e-10,6.122e-07,3.931e-05,3.91e-15,,;

3,1.751e-10,7.639e-07,4.443e-05,4.813e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 21, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.0691e-10,5.677e-07,3.694e-05,2.814e-15,,;

2,1.033e-10,8.841e-07,6.996e-05,5.274e-15,,;

3,1.163e-10,1.098e-06,7.963e-05,6.22e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 21, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,8.0875e-11,8.1285e-07,5.725e-05,3.9885e-15,,;

2,7.796e-11,1.247e-06,0.0001101,6.92e-15,,;

3,8.726e-11,1.541e-06,0.0001241,8.595e-15,,;
];

% input size = 44
% VDD = 0.7V 
sram_mux_1level(:, :, 22, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.628e-10,4.051e-07,2.1265e-05,2.1675e-15,,;

2,1.619e-10,6.413e-07,3.955e-05,4.034e-15,,;

3,1.816e-10,8.002e-07,4.443e-05,5.124e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 22, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.11e-10,5.946e-07,3.72e-05,2.8975e-15,,;

2,1.069e-10,9.262e-07,7.042e-05,5.46e-15,,;

3,1.204e-10,1.15e-06,8.023e-05,6.429e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 22, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,8.4005e-11,8.514e-07,5.7835e-05,4.0635e-15,,;

2,8.096e-11,1.307e-06,0.0001111,6.871e-15,,;

3,9.088e-11,1.614e-06,0.000125,8.359e-15,,;
];

% input size = 46
% VDD = 0.7V 
sram_mux_1level(:, :, 23, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.6875e-10,4.2345e-07,2.1355e-05,2.2515e-15,,;

2,1.676e-10,6.704e-07,3.97e-05,4.198e-15,,;

3,1.88e-10,8.365e-07,4.5e-05,5.093e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 23, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.15e-10,6.2155e-07,3.745e-05,2.976e-15,,;

2,1.106e-10,9.682e-07,7.086e-05,5.649e-15,,;

3,1.245e-10,1.202e-06,8.077e-05,6.657e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 23, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,8.7485e-11,8.9015e-07,5.8675e-05,4.022e-15,,;

2,8.375e-11,1.366e-06,0.000112,7.182e-15,,;

3,9.428e-11,1.688e-06,0.0001258,8.267e-15,,;
];

% input size = 48
% VDD = 0.7V 
sram_mux_1level(:, :, 24, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.746e-10,4.418e-07,2.1505e-05,2.3045e-15,,;

2,1.733e-10,6.995e-07,3.984e-05,4.36e-15,,;

3,1.942e-10,8.727e-07,4.545e-05,5.119e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 24, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.1895e-10,6.4845e-07,3.7675e-05,3.0765e-15,,;

2,1.143e-10,1.01e-06,7.127e-05,5.838e-15,,;

3,1.287e-10,1.254e-06,8.129e-05,6.884e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 24, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,9.0815e-11,9.286e-07,5.927e-05,3.8845e-15,,;

2,8.644e-11,1.425e-06,0.0001128,7.414e-15,,;

3,9.749e-11,1.761e-06,0.0001268,8.626e-15,,;
];

% input size = 50
% VDD = 0.7V 
sram_mux_1level(:, :, 25, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.8045e-10,4.6015e-07,2.167e-05,2.341e-15,,;

2,1.791e-10,7.286e-07,3.967e-05,4.677e-15,,;

3,2.005e-10,9.088e-07,4.565e-05,5.288e-15,,;
];
% VDD = 0.8V
sram_mux_1level(:, :, 25, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.2285e-10,6.754e-07,3.7895e-05,3.1835e-15,,;

2,1.183e-10,1.052e-06,7.176e-05,5.959e-15,,;

3,1.332e-10,1.306e-06,8.184e-05,7.028e-15,,;
];
% VDD = 0.9V
sram_mux_1level(:, :, 25, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,9.3905e-11,9.67e-07,5.974e-05,4.012e-15,,;

2,8.911e-11,1.484e-06,0.0001136,7.679e-15,,;

3,1.006e-10,1.833e-06,0.0001278,8.917e-15,,;
];

%% Post process the data
% Find the best Wprog and associated performance
sram_mux_1level_best_delay = zeros(num_merits, num_swept_input_size, num_VDD); % Plus 1 flags:  X_opt and delete the 1st flag: Ron
sram_mux_1level_best_power = zeros(num_merits, num_swept_input_size, num_VDD); % Plus 1 flags:  X_opt and delete the 1st flag: Ron 
sram_mux_1level_best_pdp = zeros(num_merits, num_swept_input_size, num_VDD); % Plus 1 flags:  X_opt and delete the 1st flag: Ron
% Search each input size, find the best Wprog and X_opt
for isize = 1:1:num_swept_input_size
  % Traverse VDD 
  for ivdd = 1:1:num_VDD 
    % find the best wcpt
    [best_delay, best_delay_index] = min(sram_mux_1level(:,3,isize,ivdd)); % the third column is delay
    [best_power, best_power_index] = min(sram_mux_1level(:,5,isize,ivdd)); % the fifth column is power
    [best_pdp, best_pdp_index]     = min(sram_mux_1level(:,6,isize,ivdd)); % the sixth column is PDP
    % determine which is the best and fill the data
    sram_mux_1level_best_delay (:,isize, ivdd) = [ sram_mux_1level(best_delay_index,:,isize,ivdd) ]; % 0 indicates this is x=0
    sram_mux_1level_best_power (:,isize, ivdd) = [ sram_mux_1level(best_power_index,:,isize,ivdd) ]; % 0 indicates this is x=0
    sram_mux_1level_best_pdp (:,isize, ivdd)   = [ sram_mux_1level(best_pdp_index,:,isize,ivdd) ]; % 0 indicates this is x=0
  end
end

%% Area Estimation 
% We need the Wprog_opt in area estimation
% Replace the first column of data (esti. R) with area
buf_trans_width = 3;
pn_ratio = 2;
one_sram_cell_area = 4;
for isize = 1 : 1 : num_swept_input_size
  for ivdd = 1:1:num_VDD
    % For the case of best delay
    wcpt_opt_delay = rram_mux_improve_1level_best_delay(2, isize, ivdd); % the second column is the Wprog
    area_multiplexing = (2 * isize) * (trans_area(wcpt_opt_delay)+ trans_area( wcpt_opt_delay * pn_ratio ));
    area_sram = one_sram_cell_area * (2 * isize);
    %area_multiplexing = area_multiplexing + ceil(sqrt(2*i)) * (trans_area(1)+ trans_area(1*pn_ratio));
    %area_sram = one_sram_cell_area*(2*ceil(sqrt(2*i)));
    area_buf = (2 * isize + 1) * (trans_area(buf_trans_width)+ trans_area(buf_trans_width * pn_ratio));
    sram_mux_1level_best_delay (1, isize, ivdd) = area_multiplexing + area_buf + area_sram;

    % For the case of best power
    wcpt_opt_power = rram_mux_improve_1level_best_power(2, isize, ivdd); % the second column is the Wprog
    area_multiplexing = (2 * isize) * (trans_area(wcpt_opt_power)+ trans_area( wcpt_opt_power * pn_ratio ));
    area_sram = one_sram_cell_area * (2 * isize);
    %area_multiplexing = area_multiplexing + ceil(sqrt(2*i)) * (trans_area(1)+ trans_area(1*pn_ratio));
    %area_sram = one_sram_cell_area*(2*ceil(sqrt(2*i)));
    area_buf = (2 * isize + 1) * (trans_area(buf_trans_width)+ trans_area(buf_trans_width * pn_ratio));
    sram_mux_1level_best_power (1, isize, ivdd) = area_multiplexing + area_buf + area_sram;

    % For the case of best pdp
    wcpt_opt_pdp = rram_mux_improve_1level_best_pdp(2, isize, ivdd); % the second column is the Wprog
    area_multiplexing = (2 * isize) * (trans_area(wcpt_opt_pdp)+ trans_area( wcpt_opt_pdp * pn_ratio ));
    area_sram = one_sram_cell_area * (2 * isize);
    %area_multiplexing = area_multiplexing + ceil(sqrt(2*i)) * (trans_area(1)+ trans_area(1*pn_ratio));
    %area_sram = one_sram_cell_area*(2*ceil(sqrt(2*i)));
    area_buf = (2 * isize + 1) * (trans_area(buf_trans_width)+ trans_area(buf_trans_width * pn_ratio));
    sram_mux_1level_best_pdp (1, isize, ivdd) = area_multiplexing + area_buf + area_sram;
  end 
end



