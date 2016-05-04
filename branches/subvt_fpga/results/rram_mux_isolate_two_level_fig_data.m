% two-level 
area_rram_mux_isolate_two_level = 2:2:32;
pn_ratio = 1.9;
prog_pn_ratio = 3;
isol_trans_size = 1;
for i=1:1:length(area_rram_mux_isolate_two_level)
  area_multiplexing = (2*i+1) * (2*trans_area(1)+ 2*trans_area(prog_pn_ratio) + 2*trans_area(isol_trans_size)) + ceil(sqrt(2*i)) * (trans_area(1)+ trans_area(prog_pn_ratio));

  area_buf = (2*i + 1) * (trans_area(2)+ trans_area(2*pn_ratio));

  area_rram_mux_isolate_two_level(i) = area_multiplexing + area_buf;
end
% Delay and power when VDD=0.7V 
rram_mux_isolate_two_level_0p7V = [
2, 
4,
6,
8,
10,
12,
14,
16,
18,
20,
22,
24,
26,
28,
30,
32,
];

% Delay and power when VDD=0.8V 
rram_mux_isolate_two_level_0p8V = [
2, 
4,
6,
8,
10,
12,
14,
16,
18,
20,
22,
24,
26,
28,
30,
32,
];

% Delay and power when VDD=0.9V 
rram_mux_isolate_two_level_0p9V = [
2, 
4,
6,
8,
10,
12,
14,
16,
18,
20,
22,
24,
26,
28,
30,
32,
];

