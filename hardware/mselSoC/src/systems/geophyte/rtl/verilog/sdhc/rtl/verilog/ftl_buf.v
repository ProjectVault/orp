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
module ftl_buf (
   input  wire        clk_50,
   input  wire        reset_n,

   // port for ftl_wbs (external clock) (should be SDHC)
   input  wire        bram_wbs_clk,
   input  wire [15:0] bram_wbs_addr,
   input  wire        bram_wbs_wren,
   input  wire [31:0] bram_wbs_data,
   output wire [31:0] bram_wbs_q,

   // port to ftl_physical
   input  wire [15:0] bram_physical_addr,
   input  wire        bram_physical_wren,
   input  wire [31:0] bram_physical_data,
   output wire [31:0] bram_physical_q
);

`include "ftl_const.vh"

// 131072 byte bram (2^15=32768 x 32bit word)
ftl_bram_block_dp #(32, 15) ifbram (
   .a_clk  ( bram_wbs_clk ),
   .a_wr   ( bram_wbs_wren ),
   .a_addr ( bram_wbs_addr ),
   .a_din  ( bram_wbs_data ),
   .a_dout ( bram_wbs_q ),
   
   .b_clk  ( clk_50 ),
   .b_wr   ( bram_physical_wren ),
   .b_addr ( bram_physical_addr ),
   .b_din  ( bram_physical_data ),
   .b_dout ( bram_physical_q )
);

endmodule
