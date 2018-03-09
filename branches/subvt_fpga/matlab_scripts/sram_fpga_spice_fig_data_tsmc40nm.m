
% Area of Full-chip FPGAs w.r.t. channel width
area_layout_sram_fpga = [ % Channel Width, area of core, total area, utilization ;
50, 301147.106, 386386.384, 475575.568, 0.779;
100, 377815.158, 479569.507, 578405.288, 0.788;
150, 455387.008, 566782.726, 673827.16, 0.803;
200, 536277.899, 659911.694, 775050.509, 0.813;
250, 618798.499, 756221.508, 879149.973, 0.818;
300, 698886.418, 849383.111, 979387.016, 0.823;
];

min_width_trans_area = 0.127;

area_vpr_sram_fpga = [ % Channel Width, logic block area, routing area, total area, fitting total area ;
50, 1.35e6, 374856, 0 , 0;
100,1.35e6, 735060, 0 , 0;
150,1.35e6, 1.06e6, 0 , 0;
200,1.35e6, 1.42e6, 0 , 0;
250,1.35e6, 1.79e6, 0 , 0;
300,1.35e6, 2.15e6, 0 , 0;
];

