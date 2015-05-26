`timescale 1ns / 1ps
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
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    23:29:02 11/06/2014 
// Design Name: 
// Module Name:    trng_top 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module trng (
    input clk,
    input en,
    output [7:0] R
);


wire [7:0] a, c;
reg [7:0] b, d;

assign a = (en) ? ({ a[6:0], a[7] } ^ a ^ { a[0], a[7:1] } ^ 8'h80) : a;

always @(posedge clk)
begin
	b <= a;
end

assign c = (d & 8'h96) ^ { d[6:0], 1'b0 } ^ { 1'b0, d[7:1] } ^ b;
always @(posedge clk)
begin
	d <= (en) ? c : d;
end

assign R = d;

endmodule
