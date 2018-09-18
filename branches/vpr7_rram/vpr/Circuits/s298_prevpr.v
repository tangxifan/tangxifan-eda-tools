// Benchmark "s298.bench" written by ABC on Mon Sep 17 23:08:06 2018

module s298_prevpr ( G0, G1, G2, clk,
    G117, G132, G66, G118, G133, G67  );
  input  G0, G1, G2, clk;
  output G117, G132, G66, G118, G133, G67;
  reg G10, G11, G12, G13, G14, G15, G16, G17, G18, G19, G20, G21, G22, G23;
  wire n53, n54, n55, n56, n57, n59, n60, n61, n63, n64, n65, n66, n67, n68,
    n69, n70, n72, n73, n74, n75, n76, n77, n79, n80, n81, n82, n83, n84,
    n85, n86, n88, n89, n90, n91, n93, n94, n95, n96, n97, n98, n99, n100,
    n102, n103, n104, n105, n107, n108, n109, n110, n111, n112, n113, n114,
    n115, n116, n117, n118, n119, n120, n121, n123, n124, n125, n127, n128,
    n129, n21_1, n26_1, n31_1, n36_1, n41_1, n46_1, n51_1, n56_1, n61_1,
    n66_2, n71_1, n76_1, n81_1, n86_1;
  assign n21_1 = ~G0 & ~G10;
  assign n53 = G10 & G11;
  assign n54 = ~G12 & G13;
  assign n55 = G10 & ~n54;
  assign n56 = ~G11 & ~n55;
  assign n57 = ~G0 & ~n53;
  assign n26_1 = ~n56 & n57;
  assign n59 = ~G12 & ~n53;
  assign n60 = G12 & n53;
  assign n61 = ~G0 & ~n60;
  assign n31_1 = ~n59 & n61;
  assign n63 = ~G11 & G12;
  assign n64 = G10 & ~n63;
  assign n65 = G13 & ~n64;
  assign n66 = G12 & ~G13;
  assign n67 = G10 & n66;
  assign n68 = ~n54 & ~n67;
  assign n69 = G11 & ~n68;
  assign n70 = ~n65 & ~n69;
  assign n36_1 = ~G0 & ~n70;
  assign n72 = G10 & ~G11;
  assign n73 = n54 & n72;
  assign n74 = ~G23 & ~n73;
  assign n75 = G14 & ~n74;
  assign n76 = ~G14 & n74;
  assign n77 = ~G0 & ~n76;
  assign n41_1 = ~n75 & n77;
  assign n79 = ~G14 & G22;
  assign n80 = n54 & n79;
  assign n81 = ~G15 & ~n80;
  assign n82 = ~G14 & ~G22;
  assign n83 = n54 & n82;
  assign n84 = G15 & ~n83;
  assign n85 = G11 & ~n84;
  assign n86 = ~n81 & ~n85;
  assign n46_1 = ~G0 & n86;
  assign n88 = G14 & ~G16;
  assign n89 = G12 & G14;
  assign n90 = ~G13 & ~n89;
  assign n91 = ~n88 & ~n90;
  assign n51_1 = ~n86 & n91;
  assign n93 = G11 & ~G12;
  assign n94 = ~G11 & ~G14;
  assign n95 = ~n93 & ~n94;
  assign n96 = ~G13 & ~n95;
  assign n97 = ~G12 & ~G13;
  assign n98 = G14 & ~n97;
  assign n99 = G17 & n98;
  assign n100 = ~n96 & ~n99;
  assign n56_1 = ~n86 & ~n100;
  assign n102 = G14 & ~n93;
  assign n103 = ~G13 & ~n102;
  assign n104 = G18 & n98;
  assign n105 = ~n103 & ~n104;
  assign n61_1 = ~n86 & ~n105;
  assign n107 = ~G10 & n86;
  assign n108 = ~G19 & n98;
  assign n109 = ~n103 & ~n108;
  assign n110 = ~n86 & n109;
  assign n111 = ~n107 & ~n110;
  assign n112 = ~G20 & ~n97;
  assign n113 = G11 & n97;
  assign n114 = G14 & ~n113;
  assign n115 = ~n112 & n114;
  assign n116 = ~n86 & n115;
  assign n117 = ~n107 & ~n116;
  assign n118 = G21 & n98;
  assign n119 = G11 & ~G14;
  assign n120 = n66 & n119;
  assign n121 = ~n118 & ~n120;
  assign n76_1 = ~n86 & ~n121;
  assign n123 = ~G2 & G22;
  assign n124 = G2 & ~G22;
  assign n125 = ~n123 & ~n124;
  assign n81_1 = ~G0 & ~n125;
  assign n127 = ~G1 & G23;
  assign n128 = G1 & ~G23;
  assign n129 = ~n127 & ~n128;
  assign n86_1 = ~G0 & ~n129;
  assign G117 = G18;
  assign G132 = G20;
  assign G66 = G16;
  assign G118 = G19;
  assign G133 = G21;
  assign G67 = G17;
  assign n66_2 = ~n111;
  assign n71_1 = ~n117;
  always @ (posedge clk) begin
    G10 <= n21_1;
    G11 <= n26_1;
    G12 <= n31_1;
    G13 <= n36_1;
    G14 <= n41_1;
    G15 <= n46_1;
    G16 <= n51_1;
    G17 <= n56_1;
    G18 <= n61_1;
    G19 <= n66_2;
    G20 <= n71_1;
    G21 <= n76_1;
    G22 <= n81_1;
    G23 <= n86_1;
  end
  initial begin
    G10 <= 1'b0;
    G11 <= 1'b0;
    G12 <= 1'b0;
    G13 <= 1'b0;
    G14 <= 1'b0;
    G15 <= 1'b0;
    G16 <= 1'b0;
    G17 <= 1'b0;
    G18 <= 1'b0;
    G19 <= 1'b0;
    G20 <= 1'b0;
    G21 <= 1'b0;
    G22 <= 1'b0;
    G23 <= 1'b0;
  end
endmodule


