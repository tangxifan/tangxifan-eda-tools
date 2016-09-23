//-----------------------------------------------------
// Design Name : static_dff
// File Name   : ff.v
// Function    : D flip-flop with asyn reset and set
// Coder       : Xifan TANG
//-----------------------------------------------------
module static_dff (
input D, // Data Input
output Q, // Q output 
input clk    , // Clock Input
input reset , // Reset input 
input set     // set input
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
