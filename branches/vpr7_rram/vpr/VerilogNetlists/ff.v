//-----------------------------------------------------
// Design Name : static_dff
// File Name   : ff.v
// Function    : D flip-flop with asyn reset and set
// Coder       : Xifan TANG
//-----------------------------------------------------
module static_dff (
/* Global ports go first */
input set,     // set input
input reset, // Reset input 
input clk, // Clock Input
/* Local ports follow */
input D, // Data Input
output Q // Q output 
);
//------------Internal Variables--------
reg q;

//-------------Code Starts Here---------
always @ ( posedge clk or negedge reset or negedge set)
if (~reset) begin
  q <= 1'b0;
end else if (~set) begin
  q <= 1'b1;
end else begin
  q <= D;
end

endmodule //End Of Module static_dff
