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
module ftl_bram_block_dp #(
   parameter DATA = 32,
   parameter ADDR = 7
) (
   input  wire            a_clk,
   input  wire            a_wr,
   input  wire [ADDR-1:0] a_addr,
   input  wire [DATA-1:0] a_din,
   output reg  [DATA-1:0] a_dout,
 
   input  wire            b_clk,
   input  wire            b_wr,
   input  wire [ADDR-1:0] b_addr,
   input  wire [DATA-1:0] b_din,
   output reg  [DATA-1:0] b_dout
);

reg [DATA-1:0] mem [(2**ADDR)-1:0];
 
always @(posedge a_clk) begin
   if(a_wr) begin
      a_dout      <= a_din;
      mem[a_addr] <= a_din;
   end else
	a_dout    <= mem[a_addr];
end
 
always @(posedge b_clk) begin
   if(b_wr) begin
      b_dout      <= b_din;
      mem[b_addr] <= b_din;
   end else
	b_dout    <= mem[b_addr];
end

endmodule
