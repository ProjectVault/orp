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
module wb_trng (
	input wb_clk,
	input wb_rst,

	input [31:0] wb_adr_i,
	input [7:0] wb_dat_i,
	input wb_we_i,
	input wb_cyc_i,
	input wb_stb_i,
	input [2:0] wb_cti_i,
	input [1:0] wb_bte_i,
	output [7:0] wb_dat_o,
	output reg wb_ack_o,
	output wb_err_o,
	output wb_rty_o
);

wire valid = wb_cyc_i & wb_stb_i;

trng trng_inst (
	.clk(wb_clk),
	.en(valid),
	.R(wb_dat_o)
);

always @(posedge wb_clk)
begin
	wb_ack_o <= 0;
	if(valid)
		wb_ack_o <= 1;
end

assign wb_err_o = 0;
assign wb_rty_o = 0;

endmodule
