/*
   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
//
// 2-stage synchronizer
//
module synch_2 #(parameter WIDTH = 1) (
   input  wire [WIDTH-1:0] i,      // input signal
   output reg  [WIDTH-1:0] o,      // synchronized output
   input  wire             clk     // clock to synchronize on
);

reg [WIDTH-1:0] stage_1;
always @(posedge clk)
   {o, stage_1} <= {stage_1, i};

endmodule


//
// 3-stage synchronizer
//
module synch_3 #(parameter WIDTH = 1) (
   input  wire [WIDTH-1:0] i,     // input signal
   output reg  [WIDTH-1:0] o,     // synchronized output
   input  wire             clk,   // clock to synchronize on
   output wire             rise   // one-cycle rising edge pulse
);

reg [WIDTH-1:0] stage_1;
reg [WIDTH-1:0] stage_2;
reg [WIDTH-1:0] stage_3;

assign rise = (WIDTH == 1) ? (o & ~stage_3) : 1'b0;
always @(posedge clk) 
   {stage_3, o, stage_2, stage_1} <= {o, stage_2, stage_1, i};
   
endmodule
