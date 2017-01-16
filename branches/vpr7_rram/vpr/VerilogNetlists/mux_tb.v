//-----------------------------------------------------
// Design Name : testbench for 1-level RRAM MUX
// File Name   : mux_tb.v
// Function    : 4T1R-based 1-level MUXes
// Coder       : Xifan TANG
//-----------------------------------------------------
//----- Time scale: simulation time step and accuracy -----
`timescale 1ns / 1ps

module rram_mux1level_tb;
// Parameters
parameter SIZE_OF_MUX = 4; //---- MUX input size 
parameter SIZE_OF_BLWL = 5; //---- MUX input size 
parameter prog_clk_period = 1; // [ns] half clock period
parameter op_clk_period = 1; // [ns] half clock period
parameter config_period =  2 * prog_clk_period; // [ns] One full clock period
parameter operating_period =  SIZE_OF_MUX * 2 * op_clk_period; // [ns] One full clock period

// voltage sources
wire [0:SIZE_OF_MUX-1] in;
wire out;
wire [0:SIZE_OF_BLWL-1] bl;
wire [0:SIZE_OF_BLWL-1] wl; 
wire prog_EN;
wire prog_ENb;
// clocks
wire prog_clock;
wire op_clock;
// registered ports 
reg prog_clock_reg;
reg op_clock_reg;
reg [0:SIZE_OF_MUX-1] in_reg;
reg [0:SIZE_OF_BLWL-1] bl_reg;
reg [0:SIZE_OF_BLWL-1] wl_reg;
// Config done signal;
reg config_done;
// Temp register for rotating shift
reg temp;

// Unit Under Test
mux_1level_size4 U0 (prog_EN, prog_ENb, in, out, bl, wl);

// Task: assign BL and WL values 
task prog_blwl;
  begin
    @(posedge prog_clock);
    // Rotate left shift
    temp = bl_reg[SIZE_OF_BLWL-1];
    //bl_reg = bl_reg >> 1;
    bl_reg[1:SIZE_OF_BLWL-1] = bl_reg[0:SIZE_OF_BLWL-2]; 
    bl_reg[0] = temp;
  end 
endtask

// Task: assign inputs
task op_mux_input;
  begin
    @(posedge op_clock);
    temp = in_reg[SIZE_OF_MUX-1];
    in_reg[1:SIZE_OF_MUX-1] = in_reg[0:SIZE_OF_MUX-2];
    in_reg[0] = temp;
  end 
endtask

// Configuration done signal
initial
begin
  config_done = 1'b0;
end
// Enabled during config_period, Disabled during op_period 
always
begin
  #config_period config_done = ~config_done;
  #operating_period config_done = ~config_done;
end

// Programming clocks
initial 
begin
  prog_clock_reg = 1'b0;
end
always
begin
  #prog_clk_period prog_clock_reg = ~prog_clock_reg; 
end

// Operating clocks
initial 
begin
  op_clock_reg = 1'b0;
end
always
begin
  #op_clk_period op_clock_reg = ~op_clock_reg; 
end

// Programming and Operating clocks 
assign prog_clock = prog_clock_reg & (~config_done);
assign op_clock = op_clock_reg & config_done;

// Programming Enable signals
assign prog_EN = prog_clock & (~config_done);
assign prog_ENb = ~prog_EN;

// Programming phase: BL/WL 
initial
begin
  // Initialize BL registers
  bl_reg = {SIZE_OF_BLWL {1'b0}}; 
  bl_reg[0] = 1'b1;
  wl_reg = {SIZE_OF_BLWL {1'b0}}; 
  wl_reg[SIZE_OF_BLWL-1] = 1'b1;
end
always wait (~config_done) // Only invoked when config_done is 0
begin
  // Propagate input 1 to the output
  // BL[0] = 1, WL[4] = 1
  prog_blwl;
end

// Operating Phase
initial
begin
  in_reg = {SIZE_OF_MUX {1'b0}}; 
  in_reg[0] = 1'b1; // Last bit is 1 initially
end
always wait (config_done) // Only invoked when config_done is 1
begin
  /* Update inputs */
  op_mux_input;
end

// Wire ports
assign in = in_reg;
assign bl = bl_reg;
assign wl = wl_reg;

endmodule

