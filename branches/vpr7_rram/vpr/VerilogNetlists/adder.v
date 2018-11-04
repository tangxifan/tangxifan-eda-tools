//------ Module: sram6T_blwl -----//
//------ Verilog file: sram.v -----//
//------ Author: Xifan TANG -----//
module adder(
input [0:0] a, // Input a
input [0:0] b, // Input b
input [0:0] cin, // Input cin
output [0:0] sumout, // Output sum
output [0:0] cout // Output carry
);
  assign sumout = a ^ b ^ cin;
  assign cout = a & b + a & cin + b & cin; 
endmodule

