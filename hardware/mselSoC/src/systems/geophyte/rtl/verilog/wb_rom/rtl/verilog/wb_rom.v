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
// Create Date:    19:35:02 01/05/2015 
// Design Name: 
// Module Name:    wb_rom 
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
module wb_rom #(
	parameter depth = 65536,
	parameter aw = $clog2(depth),
	parameter memfile = "rom.dat"
) (
        input wb_clk,
        input wb_rst,
        input [31:0] wb_adr_i,
        input [2:0] wb_cti_i,
        input [1:0] wb_bte_i,
        input wb_we_i,
        input wb_cyc_i,
        input wb_stb_i,
        output [31:0] wb_dat_o,
        output reg wb_ack_o,
        output wb_err_o,
        output wb_rty_o
);

`include "wb_common.v"

assign wb_err_o = 0;
assign wb_rty_o = 0;

wire valid = (wb_cyc_i & wb_stb_i);
reg valid_r;
wire new_cycle = valid & ~valid_r;

reg [aw - 1:0] adr_r;
wire [aw - 1:0] next_adr = wb_next_adr(adr_r, wb_cti_i, wb_bte_i, 32);
wire [aw - 1:0] wb_adr = new_cycle ? wb_adr_i : next_adr;

always @(posedge wb_clk)
begin
	adr_r <= wb_adr;
	valid_r <= valid;
	wb_ack_o <= valid & (!((wb_cti_i == 3'b000) | (wb_cti_i == 3'b111)) | !wb_ack_o);
	if(wb_rst)
	begin
		adr_r <= 0;
		valid_r <= 0;
		wb_ack_o <= 0;
	end
end

dp_rom #(
	.memfile(memfile),
	.aw(aw - 2)
) rom_inst (
	.clk1(wb_clk),
	.en1(valid),
	.adr1(wb_adr[aw - 1:2]),
	.dat1(wb_dat_o),
	.clk2(1'b0),
	.en2(1'b0),
	.adr2(0),
	.dat2()
);

endmodule
