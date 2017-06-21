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
sram_mux_2level = zeros( num_merits, num_swept_wcpt, num_swept_input_size, num_VDD); 

% 16 index: 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32

% input size = 2
% VDD = 0.7V 
sram_mux_2level(:, :, 1, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.7695e-11,1.83151e-06,1.5597e-05,5.8145e-16,,;

2,2.76e-11,6.677e-06,2.695e-05,9.832e-16,,;

3,2.906e-11,9.285e-06,2.982e-05,1.129e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 1, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.0535e-11,4.05276e-06,2.571e-05,8.797e-16,,;

2,2.007e-11,1.462e-05,4.453e-05,1.513e-15,,;

3,2.1e-11,2.01e-05,4.996e-05,1.72e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 1, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,1.628e-11,7.70364e-06,3.8445e-05,1.2896e-15,,;

2,1.565e-11,2.752e-05,6.45e-05,2.16e-15,,;

3,1.632e-11,3.748e-05,7.393e-05,2.475e-15,,;
];

% input size = 4
% VDD = 0.7V 
sram_mux_2level(:, :, 2, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.24e-11,3.592e-08,1.40815e-05,5.9205e-16,,;

2,4.589e-11,4.7e-08,2.617e-05,1.109e-15,,;

3,4.999e-11,5.552e-08,3.016e-05,1.328e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 2, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.0205e-11,5.361e-08,2.2725e-05,8.4525e-16,,;

2,3.153e-11,6.937e-08,4.193e-05,1.544e-15,,;

3,3.421e-11,8.154e-08,4.866e-05,1.86e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 2, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.374e-11,7.8075e-08,3.4905e-05,1.2175e-15,,;

2,2.428e-11,1.001e-07,6.294e-05,2.186e-15,,;

3,2.627e-11,1.172e-07,7.377e-05,2.65e-15,,;
];

% input size = 6
% VDD = 0.7V 
sram_mux_2level(:, :, 3, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.9245e-11,5.269e-08,1.4441e-05,6.521e-16,,;

2,5.412e-11,6.951e-08,2.662e-05,1.219e-15,,;

3,5.962e-11,8.206e-08,3.098e-05,1.474e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 3, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.4635e-11,7.833e-08,2.351e-05,9.109e-16,,;

2,3.668e-11,1.022e-07,4.298e-05,1.708e-15,,;

3,4.021e-11,1.2e-07,5.047e-05,2.047e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 3, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.6975e-11,1.1355e-07,3.5725e-05,1.2941e-15,,;

2,2.791e-11,1.468e-07,6.506e-05,2.36e-15,,;

3,3.045e-11,1.717e-07,7.663e-05,2.861e-15,,;
];

% input size = 8
% VDD = 0.7V 
sram_mux_2level(:, :, 4, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.9245e-11,6.992e-08,1.44585e-05,6.5285e-16,,;

2,5.412e-11,9.331e-08,2.664e-05,1.22e-15,,;

3,5.961e-11,1.105e-07,3.1e-05,1.475e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 4, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.4635e-11,1.0386e-07,2.353e-05,9.119e-16,,;

2,3.668e-11,1.371e-07,4.3e-05,1.709e-15,,;

3,4.02e-11,1.617e-07,5.05e-05,2.048e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 4, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,2.6975e-11,1.504e-07,3.5755e-05,1.2957e-15,,;

2,2.791e-11,1.968e-07,6.51e-05,2.362e-15,,;

3,3.045e-11,2.313e-07,7.667e-05,2.862e-15,,;
];

% input size = 10
% VDD = 0.7V 
sram_mux_2level(:, :, 5, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.5915e-11,8.668e-08,1.48945e-05,7.0855e-16,,;

2,6.211e-11,1.158e-07,2.687e-05,1.343e-15,,;

3,6.901e-11,1.37e-07,3.148e-05,1.656e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 5, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.889e-11,1.2855e-07,2.418e-05,9.9095e-16,,;

2,4.161e-11,1.699e-07,4.413e-05,1.831e-15,,;

3,4.597e-11,2.002e-07,5.201e-05,2.275e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 5, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.004e-11,1.859e-07,3.685e-05,1.3727e-15,,;

2,3.139e-11,2.434e-07,6.702e-05,2.537e-15,,;

3,3.459e-11,2.858e-07,7.922e-05,3.058e-15,,;
];


% input size = 12
% VDD = 0.7V 
sram_mux_2level(:, :, 6, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.5915e-11,1.01535e-07,1.4907e-05,7.0925e-16,,;

2,6.211e-11,1.311e-07,2.688e-05,1.344e-15,,;

3,6.901e-11,1.527e-07,3.149e-05,1.656e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 6, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.889e-11,1.506e-07,2.4205e-05,9.919e-16,,;

2,4.161e-11,1.926e-07,4.415e-05,1.832e-15,,;

3,4.597e-11,2.235e-07,5.204e-05,2.276e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 6, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.004e-11,2.1775e-07,3.688e-05,1.3743e-15,,;

2,3.139e-11,2.763e-07,6.705e-05,2.538e-15,,;

3,3.459e-11,3.196e-07,7.926e-05,3.059e-15,,;
];

% input size = 14
% VDD = 0.7V 
sram_mux_2level(:, :, 7, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.5915e-11,1.1875e-07,1.4925e-05,7.1e-16,,;

2,6.211e-11,1.549e-07,2.69e-05,1.344e-15,,;

3,6.901e-11,1.812e-07,3.151e-05,1.657e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 7, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.889e-11,1.761e-07,2.4225e-05,9.929e-16,,;

2,4.161e-11,2.275e-07,4.418e-05,1.833e-15,,;

3,4.597e-11,2.652e-07,5.207e-05,2.277e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 7, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.004e-11,2.546e-07,3.6915e-05,1.37545e-15,,;

2,3.139e-11,3.263e-07,6.709e-05,2.539e-15,,;

3,3.458e-11,3.792e-07,7.93e-05,3.061e-15,,;
];

% input size = 16
% VDD = 0.7V 
sram_mux_2level(:, :, 8, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.5915e-11,1.3365e-07,1.49375e-05,7.107e-16,,;

2,6.211e-11,1.701e-07,2.692e-05,1.345e-15,,;

3,6.901e-11,1.969e-07,3.153e-05,1.658e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 8, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.889e-11,1.9815e-07,2.425e-05,9.9385e-16,,;

2,4.161e-11,2.502e-07,4.421e-05,1.834e-15,,;

3,4.597e-11,2.886e-07,5.209e-05,2.278e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 8, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.004e-11,2.8645e-07,3.695e-05,1.37655e-15,,;

2,3.139e-11,3.592e-07,6.712e-05,2.541e-15,,;

3,3.458e-11,4.131e-07,7.934e-05,3.062e-15,,;
];

% input size = 18
% VDD = 0.7V 
sram_mux_2level(:, :, 9, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.276e-11,1.504e-07,1.5195e-05,7.6235e-16,,;

2,6.989e-11,1.926e-07,2.721e-05,1.457e-15,,;

3,7.801e-11,2.233e-07,3.199e-05,1.819e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 9, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.323e-11,2.229e-07,2.476e-05,1.068e-15,,;

2,4.643e-11,2.829e-07,4.503e-05,2e-15,,;

3,5.179e-11,3.27e-07,5.352e-05,2.432e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 9, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.3185e-11,3.219e-07,3.7875e-05,1.4646e-15,,;

2,3.488e-11,4.057e-07,6.883e-05,2.702e-15,,;

3,3.855e-11,4.675e-07,8.166e-05,3.341e-15,,;
];

% input size = 20
% VDD = 0.7V 
sram_mux_2level(:, :, 10, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.276e-11,1.6525e-07,1.521e-05,7.632e-16,,;

2,6.989e-11,2.079e-07,2.722e-05,1.458e-15,,;

3,7.801e-11,2.39e-07,3.201e-05,1.819e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 10, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.323e-11,2.449e-07,2.4785e-05,1.06895e-15,,;

2,4.643e-11,3.057e-07,4.505e-05,2.001e-15,,;

3,5.179e-11,3.503e-07,5.354e-05,2.433e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 10, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.3185e-11,3.538e-07,3.791e-05,1.4657e-15,,;

2,3.488e-11,4.386e-07,6.886e-05,2.704e-15,,;

3,3.855e-11,5.013e-07,8.169e-05,3.342e-15,,;
];

% input size = 22
% VDD = 0.7V 
sram_mux_2level(:, :, 11, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.276e-11,1.8245e-07,1.5225e-05,7.641e-16,,;

2,6.988e-11,2.317e-07,2.724e-05,1.458e-15,,;

3,7.8e-11,2.674e-07,3.203e-05,1.821e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 11, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.323e-11,2.704e-07,2.481e-05,1.06995e-15,,;

2,4.643e-11,3.406e-07,4.508e-05,2.002e-15,,;

3,5.179e-11,3.92e-07,5.357e-05,2.435e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 11, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.3185e-11,3.906e-07,3.7945e-05,1.46685e-15,,;

2,3.488e-11,4.886e-07,6.89e-05,2.705e-15,,;

3,3.855e-11,5.609e-07,8.174e-05,3.344e-15,,;
];

% input size = 24
% VDD = 0.7V 
sram_mux_2level(:, :, 12, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.276e-11,1.9735e-07,1.524e-05,7.6495e-16,,;

2,6.988e-11,2.469e-07,2.726e-05,1.459e-15,,;

3,7.8e-11,2.831e-07,3.204e-05,1.821e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 12, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.323e-11,2.9245e-07,2.483e-05,1.0709e-15,,;

2,4.643e-11,3.633e-07,4.51e-05,2.003e-15,,;

3,5.179e-11,4.153e-07,5.359e-05,2.436e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 12, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.3185e-11,4.2245e-07,3.7975e-05,1.4685e-15,,;

2,3.488e-11,5.214e-07,6.893e-05,2.706e-15,,;

3,3.855e-11,5.947e-07,8.177e-05,3.345e-15,,;
];


% input size = 26
% VDD = 0.7V 
sram_mux_2level(:, :, 13, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.8995e-11,2.141e-07,1.5505e-05,8.233e-16,,;

2,7.754e-11,2.694e-07,2.767e-05,1.546e-15,,;

3,8.706e-11,3.096e-07,3.269e-05,1.929e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 13, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.747e-11,3.1715e-07,2.564e-05,1.12805e-15,,;

2,5.134e-11,3.96e-07,4.595e-05,2.137e-15,,;

3,5.725e-11,4.537e-07,5.476e-05,2.651e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 13, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.6195e-11,4.5795e-07,3.891e-05,1.5495e-15,,;

2,3.825e-11,5.68e-07,7.051e-05,2.853e-15,,;

3,4.263e-11,6.49e-07,8.388e-05,3.524e-15,,;
];

% input size = 28
% VDD = 0.7V 
sram_mux_2level(:, :, 14, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.8995e-11,2.2895e-07,1.5525e-05,8.242e-16,,;

2,7.754e-11,2.847e-07,2.769e-05,1.547e-15,,;

3,8.706e-11,3.253e-07,3.271e-05,1.93e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 14, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.747e-11,3.392e-07,2.566e-05,1.129e-15,,;

2,5.134e-11,4.187e-07,4.598e-05,2.138e-15,,;

3,5.725e-11,4.77e-07,5.478e-05,2.652e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 14, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.6195e-11,4.8975e-07,3.894e-05,1.551e-15,,;

2,3.825e-11,6.008e-07,7.055e-05,2.854e-15,,;

3,4.263e-11,6.829e-07,8.391e-05,3.526e-15,,;
];

% input size = 30
% VDD = 0.7V 
sram_mux_2level(:, :, 15, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.8995e-11,2.438e-07,1.5535e-05,8.251e-16,,;

2,7.754e-11,3e-07,2.77e-05,1.548e-15,,;

3,8.706e-11,3.41e-07,3.272e-05,1.93e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 15, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.747e-11,3.6125e-07,2.5685e-05,1.13e-15,,;

2,5.134e-11,4.414e-07,4.6e-05,2.139e-15,,;

3,5.725e-11,5.004e-07,5.48e-05,2.653e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 15, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.6195e-11,5.2165e-07,3.897e-05,1.5525e-15,,;

2,3.825e-11,6.337e-07,7.058e-05,2.856e-15,,;

3,4.263e-11,7.167e-07,8.395e-05,3.527e-15,,;
];

% input size = 32
% VDD = 0.7V 
sram_mux_2level(:, :, 16, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.8995e-11,2.6105e-07,1.5555e-05,8.26e-16,,;

2,7.753e-11,3.237e-07,2.772e-05,1.549e-15,,;

3,8.705e-11,3.693e-07,3.274e-05,1.932e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 16, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.747e-11,3.8675e-07,2.5705e-05,1.131e-15,,;

2,5.134e-11,4.763e-07,4.603e-05,2.14e-15,,;

3,5.724e-11,5.42e-07,5.483e-05,2.654e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 16, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.6195e-11,5.5845e-07,3.901e-05,1.5535e-15,,;

2,3.824e-11,6.836e-07,7.062e-05,2.857e-15,,;

3,4.263e-11,7.762e-07,8.399e-05,3.529e-15,,;
];

% input size = 34
% VDD = 0.7V 
sram_mux_2level(:, :, 17, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.8995e-11,2.759e-07,1.5565e-05,8.2685e-16,,;

2,7.753e-11,3.39e-07,2.774e-05,1.55e-15,,;

3,8.705e-11,3.85e-07,3.276e-05,1.932e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 17, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.747e-11,4.088e-07,2.573e-05,1.132e-15,,;

2,5.134e-11,4.99e-07,4.605e-05,2.141e-15,,;

3,5.724e-11,5.654e-07,5.486e-05,2.656e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 17, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.6195e-11,5.903e-07,3.904e-05,1.555e-15,,;

2,3.824e-11,7.165e-07,7.065e-05,2.859e-15,,;

3,4.263e-11,8.1e-07,8.403e-05,3.531e-15,,;
];

% input size = 36
% VDD = 0.7V 
sram_mux_2level(:, :, 18, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,6.8995e-11,2.9075e-07,1.5585e-05,8.2775e-16,,;

2,7.753e-11,3.543e-07,2.775e-05,1.551e-15,,;

3,8.705e-11,4.007e-07,3.277e-05,1.933e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 18, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.747e-11,4.3085e-07,2.5755e-05,1.133e-15,,;

2,5.134e-11,5.217e-07,4.607e-05,2.143e-15,,;

3,5.724e-11,5.887e-07,5.488e-05,2.657e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 18, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.6195e-11,6.221e-07,3.907e-05,1.5565e-15,,;

2,3.824e-11,7.493e-07,7.069e-05,2.86e-15,,;

3,4.263e-11,8.439e-07,8.406e-05,3.532e-15,,;
];

% input size = 38
% VDD = 0.7V 
sram_mux_2level(:, :, 19, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.5315e-11,3.0755e-07,1.58e-05,8.873e-16,,;

2,8.538e-11,3.767e-07,2.797e-05,1.661e-15,,;

3,9.622e-11,4.271e-07,3.308e-05,2.102e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 19, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.1705e-11,4.5555e-07,2.6295e-05,1.20935e-15,,;

2,5.604e-11,5.544e-07,4.693e-05,2.258e-15,,;

3,6.239e-11,6.27e-07,5.588e-05,2.885e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 19, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.911e-11,6.576e-07,4.012e-05,1.6365e-15,,;

2,4.144e-11,7.958e-07,7.198e-05,3.046e-15,,;

3,4.646e-11,8.981e-07,8.602e-05,3.779e-15,,;
];


% input size = 40
% VDD = 0.7V 
sram_mux_2level(:, :, 20, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.5315e-11,3.2235e-07,1.582e-05,8.882e-16,,;

2,8.538e-11,3.92e-07,2.798e-05,1.662e-15,,;

3,9.622e-11,4.428e-07,3.309e-05,2.103e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 20, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.1705e-11,4.776e-07,2.632e-05,1.21035e-15,,;

2,5.604e-11,5.771e-07,4.695e-05,2.259e-15,,;

3,6.239e-11,6.504e-07,5.591e-05,2.887e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 20, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.911e-11,6.8945e-07,4.015e-05,1.6375e-15,,;

2,4.144e-11,8.287e-07,7.201e-05,3.047e-15,,;

3,4.646e-11,9.32e-07,8.605e-05,3.78e-15,,;
];


% input size = 42
% VDD = 0.7V 
sram_mux_2level(:, :, 21, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.5315e-11,3.3725e-07,1.583e-05,8.891e-16,,;

2,8.538e-11,4.073e-07,2.8e-05,1.663e-15,,;

3,9.622e-11,4.585e-07,3.311e-05,2.104e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 21, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.1705e-11,4.9965e-07,2.634e-05,1.21135e-15,,;

2,5.604e-11,5.998e-07,4.697e-05,2.26e-15,,;

3,6.239e-11,6.738e-07,5.593e-05,2.888e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 21, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.911e-11,7.213e-07,4.0185e-05,1.639e-15,,;

2,4.144e-11,8.615e-07,7.204e-05,3.049e-15,,;

3,4.646e-11,9.658e-07,8.608e-05,3.782e-15,,;
];

% input size = 44
% VDD = 0.7V 
sram_mux_2level(:, :, 22, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.531e-11,3.5445e-07,1.585e-05,8.9e-16,,;

2,8.538e-11,4.31e-07,2.802e-05,1.664e-15,,;

3,9.621e-11,4.869e-07,3.313e-05,2.105e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 22, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.17e-11,5.251e-07,2.636e-05,1.2124e-15,,;

2,5.604e-11,6.346e-07,4.7e-05,2.262e-15,,;

3,6.238e-11,7.153e-07,5.596e-05,2.889e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 22, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.911e-11,7.581e-07,4.022e-05,1.6405e-15,,;

2,4.144e-11,9.115e-07,7.208e-05,3.05e-15,,;

3,4.646e-11,1.025e-06,8.613e-05,3.783e-15,,;
];

% input size = 46
% VDD = 0.7V 
sram_mux_2level(:, :, 23, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.531e-11,3.6935e-07,1.586e-05,8.909e-16,,;

2,8.538e-11,4.463e-07,2.803e-05,1.665e-15,,;

3,9.621e-11,5.026e-07,3.315e-05,2.106e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 23, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.17e-11,5.4715e-07,2.6385e-05,1.2134e-15,,;

2,5.604e-11,6.573e-07,4.702e-05,2.263e-15,,;

3,6.238e-11,7.387e-07,5.598e-05,2.89e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 23, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.911e-11,7.8995e-07,4.025e-05,1.6415e-15,,;

2,4.144e-11,9.443e-07,7.212e-05,3.052e-15,,;

3,4.646e-11,1.059e-06,8.616e-05,3.785e-15,,;
];

% input size = 48
% VDD = 0.7V 
sram_mux_2level(:, :, 24, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,7.531e-11,3.8415e-07,1.588e-05,8.913e-16,,;

2,8.538e-11,4.616e-07,2.805e-05,1.666e-15,,;

3,9.621e-11,5.183e-07,3.316e-05,2.107e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 24, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.17e-11,5.692e-07,2.641e-05,1.2144e-15,,;

2,5.604e-11,6.8e-07,4.705e-05,2.264e-15,,;

3,6.238e-11,7.62e-07,5.601e-05,2.892e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 24, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,3.911e-11,8.218e-07,4.0285e-05,1.643e-15,,;

2,4.144e-11,9.771e-07,7.215e-05,3.053e-15,,;

3,4.646e-11,1.093e-06,8.62e-05,3.786e-15,,;
];

% input size = 50
% VDD = 0.7V 
sram_mux_2level(:, :, 25, 1) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,8.176e-11,4.0095e-07,1.6255e-05,9.2545e-16,,;

2,9.254e-11,4.84e-07,2.825e-05,1.773e-15,,;

3,1.046e-10,5.446e-07,3.342e-05,2.275e-15,,;
];
% VDD = 0.8V
sram_mux_2level(:, :, 25, 2) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,5.565e-11,5.939e-07,2.691e-05,1.29015e-15,,;

2,6.075e-11,7.127e-07,4.772e-05,2.405e-15,,;

3,6.831e-11,8.003e-07,5.737e-05,3.047e-15,,;
];
% VDD = 0.9V
sram_mux_2level(:, :, 25, 3) = [ % size, delay, leakage, dynamic_power, energy_per_toggle
1,4.2115e-11,8.573e-07,4.108e-05,1.735e-15,,;

2,4.48e-11,1.024e-06,7.322e-05,3.255e-15,,;

3,5.008e-11,1.147e-06,8.793e-05,4.057e-15,,;
];

%% Post process the data
% Find the best Wprog and associated performance
sram_mux_2level_best_delay = zeros(num_merits, num_swept_input_size, num_VDD); % Plus 1 flags:  X_opt and delete the 1st flag: Ron
sram_mux_2level_best_power = zeros(num_merits, num_swept_input_size, num_VDD); % Plus 1 flags:  X_opt and delete the 1st flag: Ron 
sram_mux_2level_best_pdp = zeros(num_merits, num_swept_input_size, num_VDD); % Plus 1 flags:  X_opt and delete the 1st flag: Ron
% Search each input size, find the best Wprog and X_opt
for isize = 1:1:num_swept_input_size
  % Traverse VDD 
  for ivdd = 1:1:num_VDD 
    % find the best wcpt
    [best_delay, best_delay_index] = min(sram_mux_2level(:,3,isize,ivdd)); % the third column is delay
    [best_power, best_power_index] = min(sram_mux_2level(:,5,isize,ivdd)); % the fifth column is power
    [best_pdp, best_pdp_index]     = min(sram_mux_2level(:,6,isize,ivdd)); % the sixth column is PDP
    % determine which is the best and fill the data
    sram_mux_2level_best_delay (:,isize, ivdd) = [ sram_mux_2level(best_delay_index,:,isize,ivdd) ]; % 0 indicates this is x=0
    sram_mux_2level_best_power (:,isize, ivdd) = [ sram_mux_2level(best_power_index,:,isize,ivdd) ]; % 0 indicates this is x=0
    sram_mux_2level_best_pdp (:,isize, ivdd)   = [ sram_mux_2level(best_pdp_index,:,isize,ivdd) ]; % 0 indicates this is x=0
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
    wcpt_opt_delay = rram_mux_improve_2level_best_delay(2, isize, ivdd); % the second column is the Wprog
    area_multiplexing = area_multiplexing + ceil(sqrt(2 * isize)) * (trans_area(wcpt_opt_delay)+ trans_area(wcpt_opt_delay * pn_ratio));
    area_sram = one_sram_cell_area*(2*ceil(sqrt(2 * isize)));
    area_buf = (2 * isize + 1) * (trans_area(buf_trans_width)+ trans_area(buf_trans_width * pn_ratio));
    sram_mux_2level_best_delay (1, isize, ivdd) = area_multiplexing + area_buf + area_sram;

    % For the case of best power
    wcpt_opt_power = rram_mux_improve_2level_best_power(2, isize, ivdd); % the second column is the Wprog
    area_multiplexing = area_multiplexing + ceil(sqrt(2 * isize)) * (trans_area(wcpt_opt_power)+ trans_area(wcpt_opt_power * pn_ratio));
    area_sram = one_sram_cell_area*(2*ceil(sqrt(2 * isize)));
    area_buf = (2 * isize + 1) * (trans_area(buf_trans_width)+ trans_area(buf_trans_width * pn_ratio));
    sram_mux_2level_best_power (1, isize, ivdd) = area_multiplexing + area_buf + area_sram;

    % For the case of best pdp
    wcpt_opt_pdp = rram_mux_improve_2level_best_pdp(2, isize, ivdd); % the second column is the Wprog
    area_multiplexing = area_multiplexing + ceil(sqrt(2 * isize)) * (trans_area(wcpt_opt_pdp)+ trans_area(wcpt_opt_pdp * pn_ratio));
    area_sram = one_sram_cell_area*(2*ceil(sqrt(2 * isize)));
    area_buf = (2 * isize + 1) * (trans_area(buf_trans_width)+ trans_area(buf_trans_width * pn_ratio));
    sram_mux_2level_best_pdp (1, isize, ivdd) = area_multiplexing + area_buf + area_sram;
  end 
end



