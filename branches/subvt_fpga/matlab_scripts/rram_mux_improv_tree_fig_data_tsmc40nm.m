%% Simulation results of 1-level 4T1R based multiplixers
% Including cases: x = 0 and x = L 
% Input size swept from 2 to 50
% Number of swept Parameter
num_swept_wprog = 11;
num_swept_input_size = 25;
num_VDD = 3;
num_merits = 7;

%% Part I: case x = 0
rram_mux_improve_tree_x0 = zeros( num_merits, num_swept_wprog, num_swept_input_size, num_VDD); 
% Fill data

% Number of input = 2
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,1,1) = [

];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,1,2) = [

];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,1,3) = [

];

%% Number of input = 4
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,2,1) = [
1926.61,1,4.054e-11,1.597e-07,1.0145e-05,8.8115e-16,5.32e+04,;
1605.50,1.2,3.968e-11,1.616e-07,1.07485e-05,9.1855e-16,4.79e+04,;
1376.15,1.4,3.8835e-11,1.638e-07,1.13275e-05,9.527e-16,4.41e+04,;
1204.13,1.6,3.887e-11,1.661e-07,1.18475e-05,9.946e-16,4.12e+04,;
1070.34,1.8,3.9345e-11,1.684e-07,1.2338e-05,1.0486e-15,3.9e+04,;
1000,2,3.859e-11,1.707e-07,1.272e-05,1.0796e-15,3.86e+04,;
1000,2.2,3.91e-11,1.742e-07,1.367e-05,1.14305e-15,4.09e+04,;
1000,2.4,3.999e-11,1.776e-07,1.3755e-05,1.178e-15,4.31e+04,;
1000,2.6,3.978e-11,1.812e-07,1.402e-05,1.18875e-15,4.53e+04,;
1000,2.8,4.016e-11,1.854e-07,1.4515e-05,1.2526e-15,4.75e+04,;
1000,3,4.0845e-11,1.89e-07,1.5185e-05,1.307e-15,4.97e+04,;
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,2,2) = [
1935.21,1,3.393e-11,2.146e-07,1.4825e-05,1.1567e-15,5.34e+04,;
1612.67,1.2,3.3255e-11,2.336e-07,1.6035e-05,1.2383e-15,4.81e+04,;
1382.29,1.4,3.285e-11,2.383e-07,1.6885e-05,1.29285e-15,4.43e+04,;
1209.50,1.6,3.246e-11,2.339e-07,1.7535e-05,1.33855e-15,4.14e+04,;
1075.11,1.8,3.1435e-11,2.388e-07,1.774e-05,1.3349e-15,3.92e+04,;
1000,2,3.196e-11,2.424e-07,1.894e-05,1.4325e-15,3.86e+04,;
1000,2.2,3.2535e-11,2.451e-07,1.994e-05,1.513e-15,4.09e+04,;
1000,2.4,3.194e-11,2.505e-07,2.004e-05,1.5055e-15,4.31e+04,;
1000,2.6,3.277e-11,2.56e-07,2.1415e-05,1.6265e-15,4.53e+04,;
1000,2.8,3.3025e-11,2.613e-07,2.2015e-05,1.677e-15,4.75e+04,;
1000,3,3.3225e-11,2.74e-07,2.262e-05,1.729e-15,4.97e+04,;
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,2,3) = [
1720.18,1,2.942e-11,3.225e-07,2.0795e-05,1.4939e-15,4.75e+04,;
1433.49,1.2,2.925e-11,3.268e-07,2.25e-05,1.6015e-15,4.27e+04,;
1228.70,1.4,2.8895e-11,3.325e-07,2.377e-05,1.6795e-15,3.93e+04,;
1075.11,1.6,2.8765e-11,3.361e-07,2.488e-05,1.753e-15,3.68e+04,;
1000,1.8,2.8525e-11,3.447e-07,2.6005e-05,1.8235e-15,3.64e+04,;
1000,2,2.7815e-11,3.54e-07,2.6545e-05,1.845e-15,3.86e+04,;
1000,2.2,2.82e-11,3.512e-07,2.7765e-05,1.9365e-15,4.09e+04,;
1000,2.4,2.8225e-11,3.583e-07,2.856e-05,1.9935e-15,4.31e+04,;
1000,2.6,2.8385e-11,3.692e-07,3.002e-05,2.1e-15,4.53e+04,;
1000,2.8,2.8755e-11,3.728e-07,3.0955e-05,2.1765e-15,4.75e+04,;
1000,3,2.8185e-11,3.877e-07,3.073e-05,2.1645e-15,4.97e+04,;
];

%% Number of input = 6
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,3,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,3,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,3,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 8
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,4,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,4,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,4,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 10
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,5,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,5,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,5,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 12
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,6,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,6,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,6,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 14
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,7,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,7,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,7,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 16
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,8,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,8,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,8,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 18
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,9,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,9,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,9,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 20
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,10,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,10,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,10,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 22
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,11,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,11,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,11,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 24
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,12,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,12,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,12,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 26
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,13,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,13,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,13,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 28
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,14,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,14,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,14,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 30
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,15,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,15,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,15,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 32
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,16,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,16,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,16,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 34
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,17,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,17,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,17,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 36
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,18,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,18,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,18,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 38
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,19,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,19,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,19,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 40
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,20,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,20,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,20,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 42
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,21,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,21,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,21,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 44
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,22,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,22,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,22,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 46
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,23,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,23,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,23,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 48
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,24,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,24,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,24,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 50
% VDD = 0.7V
rram_mux_improve_tree_x0(:,:,25,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_x0(:,:,25,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_x0(:,:,25,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];


%% Part I: case x = L
rram_mux_improve_tree_xl = zeros( num_merits, num_swept_wprog, num_swept_input_size, num_VDD); 

% Fill data

% Number of input = 2
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,1,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,1,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,1,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 4
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,2,1) = [
1926.61,1,3.2925e-11,1.593e-07,1.0361e-05,8.019e-16,5.32e+04,;
1605.50,1.2,3.3625e-11,1.612e-07,1.086e-05,8.4495e-16,4.79e+04,;
1376.15,1.4,3.344e-11,1.633e-07,1.13085e-05,8.774e-16,4.41e+04,;
1204.13,1.6,3.4245e-11,1.657e-07,1.18645e-05,9.297e-16,4.12e+04,;
1070.34,1.8,3.462e-11,1.678e-07,1.23495e-05,9.7115e-16,3.9e+04,;
1000,2,3.513e-11,1.703e-07,1.2799e-05,1.0141e-15,3.86e+04,;
1000,2.2,3.557e-11,1.731e-07,1.325e-05,1.0561e-15,4.09e+04,;
1000,2.4,3.608e-11,1.77e-07,1.371e-05,1.10375e-15,4.31e+04,;
1000,2.6,3.6565e-11,1.809e-07,1.412e-05,1.14555e-15,4.53e+04,;
1000,2.8,3.7105e-11,1.848e-07,1.4505e-05,1.18365e-15,4.75e+04,;
1000,3,3.747e-11,1.887e-07,1.4775e-05,1.2155e-15,4.97e+04,;
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,2,2) = [
1935.21,1,2.746e-11,2.126e-07,1.506e-05,1.05385e-15,5.34e+04,;
1612.67,1.2,2.784e-11,2.286e-07,1.611e-05,1.1346e-15,4.81e+04,;
1382.29,1.4,2.805e-11,2.303e-07,1.698e-05,1.19415e-15,4.43e+04,;
1209.50,1.6,2.791e-11,2.229e-07,1.7515e-05,1.22895e-15,4.14e+04,;
1075.11,1.8,2.8195e-11,2.399e-07,1.8175e-05,1.28225e-15,3.92e+04,;
1000,2,2.8675e-11,2.301e-07,1.912e-05,1.356e-15,3.86e+04,;
1000,2.2,2.8705e-11,2.342e-07,1.948e-05,1.3985e-15,4.09e+04,;
1000,2.4,2.9e-11,2.397e-07,2.0295e-05,1.4485e-15,4.31e+04,;
1000,2.6,2.9295e-11,2.452e-07,2.09e-05,1.5e-15,4.53e+04,;
1000,2.8,2.941e-11,2.624e-07,2.1245e-05,1.527e-15,4.75e+04,;
1000,3,3.003e-11,2.56e-07,2.233e-05,1.617e-15,4.97e+04,;
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,2,3) = [
1720.18,1,2.405e-11,3.196e-07,2.113e-05,1.386e-15,4.75e+04,;
1433.49,1.2,2.346e-11,3.259e-07,2.147e-05,1.3895e-15,4.27e+04,;
1228.70,1.4,2.4535e-11,3.288e-07,2.3605e-05,1.5465e-15,3.93e+04,;
1075.11,1.6,2.4615e-11,3.347e-07,2.4825e-05,1.632e-15,3.68e+04,;
1000,1.8,2.4405e-11,3.383e-07,2.5835e-05,1.6975e-15,3.64e+04,;
1000,2,2.418e-11,3.408e-07,2.6595e-05,1.75e-15,3.86e+04,;
1000,2.2,2.4185e-11,3.487e-07,2.6425e-05,1.7275e-15,4.09e+04,;
1000,2.4,2.4565e-11,3.559e-07,2.8045e-05,1.851e-15,4.31e+04,;
1000,2.6,2.4845e-11,3.651e-07,2.8975e-05,1.921e-15,4.53e+04,;
1000,2.8,2.5555e-11,3.703e-07,3.0395e-05,2.035e-15,4.75e+04,;
1000,3,2.5775e-11,3.801e-07,3.102e-05,2.081e-15,4.97e+04,;
];

%% Number of input = 6
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,3,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,3,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,3,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 8
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,4,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,4,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,4,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 10
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,5,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,5,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,5,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 12
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,6,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,6,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,6,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 14
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,7,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,7,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,7,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 16
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,8,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,8,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,8,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 18
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,9,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,9,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,9,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 20
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,10,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,10,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,10,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 22
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,11,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,11,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,11,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 24
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,12,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,12,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,12,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 26
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,13,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,13,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,13,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 28
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,14,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,14,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,14,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 30
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,15,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,15,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,15,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 32
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,16,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,16,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,16,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 34
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,17,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,17,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,17,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 36
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,18,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,18,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,18,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 38
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,19,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,19,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,19,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 40
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,20,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,20,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,20,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 42
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,21,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,21,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,21,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 44
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,22,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,22,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,22,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 46
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,23,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,23,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,23,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 48
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,24,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,24,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,24,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Number of input = 50
% VDD = 0.7V
rram_mux_improve_tree_xl(:,:,25,1) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.8V
rram_mux_improve_tree_xl(:,:,25,2) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

% VDD = 0.9V
rram_mux_improve_tree_xl(:,:,25,3) = [
% Test: %random data
%rand(num_merits, num_swept_wprog);
];

%% Post process the data
% Find the best Wprog and associated performance
rram_mux_improve_tree_best_delay = zeros(num_merits + 1, num_swept_input_size, num_VDD); % Plus 1 flags:  X_opt and delete the 1st flag: Ron
rram_mux_improve_tree_best_power = zeros(num_merits + 1, num_swept_input_size, num_VDD); % Plus 1 flags:  X_opt and delete the 1st flag: Ron 
rram_mux_improve_tree_best_pdp = zeros(num_merits + 1, num_swept_input_size, num_VDD); % Plus 1 flags:  X_opt and delete the 1st flag: Ron
% Search each input size, find the best Wprog and X_opt
for isize = 1:1:num_swept_input_size
  % Traverse VDD 
  for ivdd = 1:1:num_VDD 
    % Compare the PDP or delay or power!
    % case: x = 0
    [best_delay_x0, best_delay_x0_index] = min(rram_mux_improve_tree_x0(:,3,isize,ivdd)); % the third column is delay
    [best_power_x0, best_power_x0_index] = min(rram_mux_improve_tree_x0(:,5,isize,ivdd)); % the fifth column is power
    [best_pdp_x0, best_pdp_x0_index]     = min(rram_mux_improve_tree_x0(:,6,isize,ivdd)); % the sixth column is PDP
    % case: x = L
    [best_delay_xl, best_delay_xl_index] = min(rram_mux_improve_tree_xl(:,3,isize,ivdd)); % the third column is delay
    [best_power_xl, best_power_xl_index] = min(rram_mux_improve_tree_xl(:,5,isize,ivdd)); % the fifth column is power
    [best_pdp_xl, best_pdp_xl_index]     = min(rram_mux_improve_tree_xl(:,6,isize,ivdd)); % the sixth column is PDP
    % determine which is the best and fill the data
    % best delay 
    rram_mux_improve_tree_x0_best_delay (:,isize, ivdd) = [ rram_mux_improve_tree_x0(best_delay_x0_index,1:7,isize,ivdd) ]; % 0 indicates this is x=0
    rram_mux_improve_tree_xl_best_delay (:,isize, ivdd) = [ rram_mux_improve_tree_xl(best_delay_xl_index,1:7,isize,ivdd) ]; % 1 indicates this is x=L
    if (best_delay_x0 < best_delay_xl) 
      rram_mux_improve_tree_best_delay (:,isize, ivdd) = [ rram_mux_improve_tree_x0(best_delay_x0_index,1:7,isize,ivdd), 0 ]; % 0 indicates this is x=0
    else 
      rram_mux_improve_tree_best_delay (:,isize, ivdd) = [ rram_mux_improve_tree_xl(best_delay_xl_index,1:7,isize,ivdd), 1 ]; % 1 indicates this is x=L
    end
    % best power 
    rram_mux_improve_tree_x0_best_power (:,isize, ivdd) = [ rram_mux_improve_tree_x0(best_power_x0_index,1:7,isize,ivdd) ]; % 0 indicates this is x=0
    rram_mux_improve_tree_xl_best_power (:,isize, ivdd) = [ rram_mux_improve_tree_xl(best_power_xl_index,1:7,isize,ivdd) ]; % 1 indicates this is x=L
    if (best_power_x0 < best_power_xl) 
      rram_mux_improve_tree_best_power (:,isize, ivdd) = [ rram_mux_improve_tree_x0(best_power_x0_index,1:7,isize,ivdd), 0 ]; % 0 indicates this is x=0
    else 
      rram_mux_improve_tree_best_power (:,isize, ivdd) = [ rram_mux_improve_tree_xl(best_power_xl_index,1:7,isize,ivdd), 1 ]; % 1 indicates this is x=L
    end
    % best pdp 
    rram_mux_improve_tree_x0_best_pdp (:,isize, ivdd) = [ rram_mux_improve_tree_x0(best_pdp_x0_index,1:7,isize,ivdd) ]; % 0 indicates this is x=0
    rram_mux_improve_tree_xl_best_pdp (:,isize, ivdd) = [ rram_mux_improve_tree_xl(best_pdp_xl_index,1:7,isize,ivdd) ]; % 1 indicates this is x=L
    if (best_pdp_x0 < best_pdp_xl) 
      rram_mux_improve_tree_best_pdp (:,isize, ivdd) = [ rram_mux_improve_tree_x0(best_pdp_x0_index,1:7,isize,ivdd), 0 ]; % 0 indicates this is x=0
    else 
      rram_mux_improve_tree_best_pdp (:,isize, ivdd) = [ rram_mux_improve_tree_xl(best_pdp_xl_index,1:7,isize,ivdd), 1 ]; % 1 indicates this is x=L
    end
  end
end

% Area Estimation of tree-like RRAM MUX 
% We need the Wprog_opt in area estimation
% Replace the first column of data (esti. R) with area
buf_trans_width = 3;
pn_ratio = 2;
prog_pn_ratio = 2;
% Search each input size, find the best Wprog and X_opt
for isize = 1:1:num_swept_input_size;
  % Traverse VDD 
  for ivdd = 1:1:num_VDD 
    % For the case of best delay
    wprog_opt_delay = rram_mux_improve_tree_best_delay(2, isize, ivdd); % the second column is the Wprog
    area_multiplexing = (2 * isize - 1)* 3 * (2 * trans_area(wprog_opt_delay)+ 2 * trans_area(wprog_opt_delay * prog_pn_ratio));
    area_buf = (2 * isize + 1) * (trans_area(buf_trans_width)+ trans_area(buf_trans_width * pn_ratio));
    rram_mux_improve_tree_best_delay(1, isize, ivdd) = area_multiplexing + area_buf;

    % For the case of best power
    wprog_opt_power = rram_mux_improve_tree_best_power(2, isize, ivdd); % the second column is the Wprog
    area_multiplexing = (2 * isize - 1)* 3 * (2 * trans_area(wprog_opt_power)+ 2 * trans_area(wprog_opt_power * prog_pn_ratio));
    area_buf = (2 * isize + 1) * (trans_area(buf_trans_width)+ trans_area(buf_trans_width * pn_ratio));
    rram_mux_improve_tree_best_power(1, isize, ivdd) = area_multiplexing + area_buf;

    % For the case of best delay
    wprog_opt_pdp = rram_mux_improve_tree_best_pdp(2, isize, ivdd); % the second column is the Wprog
    area_multiplexing = (2 * isize - 1)* 3 * (2 * trans_area(wprog_opt_pdp)+ 2 * trans_area(wprog_opt_pdp * prog_pn_ratio));
    area_buf = (2 * isize + 1) * (trans_area(buf_trans_width)+ trans_area(buf_trans_width * pn_ratio));
    rram_mux_improve_tree_best_pdp(1, isize, ivdd) = area_multiplexing + area_buf;
  end 
end
