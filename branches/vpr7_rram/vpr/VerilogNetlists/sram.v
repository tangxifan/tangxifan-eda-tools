//------ Module: sram6T_blwl -----//
//------ Verilog file: sram.v -----//
//------ Author: Xifan TANG -----//
module sram6T_blwl(
input read,
input nequalize,
input din, // Data input
output dout, // Data output
output doutb, // Data output
input bl, // Bit line control signal
input wl // Word line control signal
);
  //----- local variable need to be registered
  reg a;

  //----- when wl is enabled, we can read in data from bl
  always @(posedge wl)
  if (1'b1 == wl) begin
    a <= bl;
  end 
  // dout is short-wired to din 
  assign dout = a;
  //---- doutb is always opposite to dout 
  assign doutb = ~dout;
endmodule
