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
// Create Date:    01:09:00 02/19/2015 
// Design Name: 
// Module Name:    fauxfs_top 
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
module fauxfs_top(
	// SDHC wishbone slave
	input wb_sdhc_clk_i,
	input wb_sdhc_rst_i,

	input [31:0] wb_sdhc_adr_i,
	input [31:0] wb_sdhc_dat_i,
	input [3:0] wb_sdhc_sel_i,
	input wb_sdhc_we_i,
	input [1:0] wb_sdhc_bte_i,
	input [2:0] wb_sdhc_cti_i,
	input wb_sdhc_cyc_i,
	input wb_sdhc_stb_i,

	output reg wb_sdhc_ack_o,
	output wb_sdhc_rty_o,
	output wb_sdhc_err_o,
	output reg [31:0] wb_sdhc_dat_o,
	
	// CPU wishbone slave
	input wb_cpu_clk_i,
	input wb_cpu_rst_i,

	input [31:0] wb_cpu_adr_i,
	input [31:0] wb_cpu_dat_i,
	input [3:0] wb_cpu_sel_i,
	input wb_cpu_we_i,
	input [1:0] wb_cpu_bte_i,
	input [2:0] wb_cpu_cti_i,
	input wb_cpu_cyc_i,
	input wb_cpu_stb_i,

	output reg wb_cpu_ack_o,
	output wb_cpu_rty_o,
	output wb_cpu_err_o,
	output reg [31:0] wb_cpu_dat_o,
	
	output wfile_dat_int_o,
	output rfile_ack_int_o
);

`include "wb_common.v"
// common memories
reg [31:0] rfile_dat_mem[511:0];
reg [31:0] rfile_ack_mem[3:0];
reg [31:0] wfile_dat_mem[511:0];
reg [31:0] wfile_ack_mem[3:0];

// CPU side
reg wfile_dat_int_cpu, rfile_ack_int_cpu;
reg wfile_dat_clr_cpu, rfile_ack_clr_cpu;

assign wfile_dat_int_o = wfile_dat_int_cpu;
assign rfile_ack_int_o = rfile_ack_int_cpu;

assign wb_cpu_rty_o = 0;
assign wb_cpu_err_o = 0;

reg [12:0] cpu_adr_r;
wire [12:0] cpu_next_adr;
wire cpu_valid = wb_cpu_cyc_i & wb_cpu_stb_i;
reg cpu_valid_r;

wire cpu_new_cycle = cpu_valid & !cpu_valid_r;

assign cpu_next_adr = wb_next_adr(cpu_adr_r, wb_cpu_cti_i, wb_cpu_bte_i, 32);

wire [12:0] cpu_adr = cpu_new_cycle ? wb_cpu_adr_i[12:0] : cpu_next_adr;

always@(posedge wb_cpu_clk_i)
begin
	cpu_adr_r <= cpu_adr;
	cpu_valid_r <= cpu_valid;

	wb_cpu_ack_o <= cpu_valid & (!((wb_cpu_cti_i == 3'b000) | (wb_cpu_cti_i == 3'b111)) | !wb_cpu_ack_o);
	if(wb_cpu_rst_i)
	begin
		cpu_adr_r <= 0;
		cpu_valid_r <= 0;
		wb_cpu_ack_o <= 0;
	end
end

always @(posedge wb_cpu_clk_i)
begin
	wfile_dat_clr_cpu <= wfile_dat_clr_cpu & wfile_dat_int_cpu;
	rfile_ack_clr_cpu <= rfile_ack_clr_cpu & rfile_ack_int_cpu;
	if(wb_cpu_rst_i)
	begin
		wfile_dat_clr_cpu <= 0;
		rfile_ack_clr_cpu <= 0;
	end
	else if(cpu_valid)
	begin
		case(cpu_adr[12:11])
		2'b00: wb_cpu_dat_o <= wfile_dat_mem[cpu_adr[10:2]];
		2'b01:
		begin
			wb_cpu_dat_o <= rfile_dat_mem[cpu_adr[10:2]];
			if(wb_cpu_we_i & wb_cpu_sel_i[0])
				rfile_dat_mem[cpu_adr[10:2]][ 7: 0] <= wb_cpu_dat_i[ 7: 0];
			if(wb_cpu_we_i & wb_cpu_sel_i[1])
				rfile_dat_mem[cpu_adr[10:2]][15: 8] <= wb_cpu_dat_i[15: 8];
			if(wb_cpu_we_i & wb_cpu_sel_i[2])
				rfile_dat_mem[cpu_adr[10:2]][23:16] <= wb_cpu_dat_i[23:16];
			if(wb_cpu_we_i & wb_cpu_sel_i[3])
				rfile_dat_mem[cpu_adr[10:2]][31:24] <= wb_cpu_dat_i[31:24];
		end
		2'b10:
		begin
			case(cpu_adr[10:4])
			7'h00:
			begin
				wb_cpu_dat_o <= wfile_ack_mem[cpu_adr[3:2]];
				if(wb_cpu_we_i & wb_cpu_sel_i[0])
					wfile_ack_mem[cpu_adr[3:2]][ 7: 0] <= wb_cpu_dat_i[ 7: 0];
				if(wb_cpu_we_i & wb_cpu_sel_i[1])
					wfile_ack_mem[cpu_adr[3:2]][15: 8] <= wb_cpu_dat_i[15: 8];
				if(wb_cpu_we_i & wb_cpu_sel_i[2])
					wfile_ack_mem[cpu_adr[3:2]][23:16] <= wb_cpu_dat_i[23:16];
				if(wb_cpu_we_i & wb_cpu_sel_i[3])
					wfile_ack_mem[cpu_adr[3:2]][31:24] <= wb_cpu_dat_i[31:24];
			end
			7'h01:
				wb_cpu_dat_o <= rfile_ack_mem[cpu_adr[3:2]];
			7'h02:
			begin
				wb_cpu_dat_o <= { 30'h0, rfile_ack_int_cpu, wfile_dat_int_cpu };
				if(wb_cpu_we_i & wb_cpu_sel_i[0])
				begin
					wfile_dat_clr_cpu <= wb_cpu_dat_i[0];
					rfile_ack_clr_cpu <= wb_cpu_dat_i[1];
				end
			end
			default: wb_cpu_dat_o <= 32'h0;
			endcase
		end
		2'b11: wb_cpu_dat_o <= 32'h0;
		endcase
	end
end

// SDHC side
assign wb_sdhc_rty_o = 0;
assign wb_sdhc_err_o = 0;

reg wfile_dat_int_sdhc, rfile_ack_int_sdhc;
reg wfile_dat_clr_sdhc, rfile_ack_clr_sdhc;

reg [31:0] sdhc_adr_r;
wire [31:0] sdhc_next_adr;
// only ack if we're being addressed (< 2MB)
// FTL will also be tied to this wishbone bus but will only ack when
// receiving requests >2MB
wire sdhc_valid = wb_sdhc_cyc_i & wb_sdhc_stb_i & ((wb_sdhc_adr_i < 32'h200000));
reg sdhc_valid_r;

wire sdhc_new_cycle = sdhc_valid & !sdhc_valid_r;

assign sdhc_next_adr = wb_next_adr(sdhc_adr_r, wb_sdhc_cti_i, wb_sdhc_bte_i, 32);

wire [31:0] sdhc_adr = sdhc_new_cycle ? wb_sdhc_adr_i[31:0] : sdhc_next_adr;

always@(posedge wb_sdhc_clk_i)
begin
	sdhc_adr_r <= sdhc_adr;
	sdhc_valid_r <= sdhc_valid;

	wb_sdhc_ack_o <= sdhc_valid & (!((wb_sdhc_cti_i == 3'b000) | (wb_sdhc_cti_i == 3'b111)) | !wb_sdhc_ack_o);
	if(wb_sdhc_rst_i)
	begin
		sdhc_adr_r <= 0;
		sdhc_valid_r <= 0;
		wb_sdhc_ack_o <= 0;
	end
end

wire mbr_rom_sel = ((sdhc_adr & 32'hfffffe00) == 32'h00000000);
wire fat_rom_sel = ((sdhc_adr & 32'hfffff000) == 32'h00100000);
wire rfile_sel = ((sdhc_adr & 32'hfffff800) == 32'h00105000);
wire wfile_sel = ((sdhc_adr & 32'hfffff800) == 32'h00105800);

always @(posedge wb_sdhc_clk_i)
begin
	rfile_ack_int_sdhc <= rfile_ack_int_sdhc & ~rfile_ack_clr_sdhc;
	wfile_dat_int_sdhc <= wfile_dat_int_sdhc & ~wfile_dat_clr_sdhc;
	
	if(wb_sdhc_rst_i)
	begin
		rfile_ack_int_sdhc <= 0;
		wfile_dat_int_sdhc <= 0;
	end
	else if(sdhc_valid)
	begin
		wb_sdhc_dat_o <= 32'h0;
		if(mbr_rom_sel)
		begin
			case(sdhc_adr[8:2])
			`include "mbr_rom.vh"
			default: wb_sdhc_dat_o <= 32'h0;
			endcase
		end
		if(fat_rom_sel)
		begin
			case(sdhc_adr[11:2])
			`include "fat_rom.vh"
			default: wb_sdhc_dat_o <= 32'h0;
			endcase
		end
		if(rfile_sel)
		begin
			wb_sdhc_dat_o <= rfile_dat_mem[sdhc_adr[10:2]];
			if(wb_sdhc_we_i && ((sdhc_adr & 32'hfffffff0) == 32'h00105000))
			begin
				if((sdhc_adr & 32'hfffffffc) == 32'h0010500c)
					rfile_ack_int_sdhc <= 1;
				rfile_ack_mem[sdhc_adr[3:2]] <= wb_sdhc_dat_i;
			end
		end
		if(wfile_sel)
		begin
			wb_sdhc_dat_o <= wfile_ack_mem[sdhc_adr[3:2]];
			if(wb_sdhc_we_i)
			begin
				if((sdhc_adr & 32'hfffffffc) == 32'h00105ffc)
					wfile_dat_int_sdhc <= 1;
				wfile_dat_mem[sdhc_adr[10:2]] <= wb_sdhc_dat_i;
			end
		end
	end
end

// CDC stuff

reg wfile_dat_int_cpu_buf, rfile_ack_int_cpu_buf;
reg wfile_dat_clr_sdhc_buf, rfile_ack_clr_sdhc_buf;

always @(posedge wb_cpu_clk_i)
begin
	wfile_dat_int_cpu <= wfile_dat_int_cpu_buf;
	wfile_dat_int_cpu_buf <= wfile_dat_int_sdhc;
	if(wb_cpu_rst_i)
	begin
		wfile_dat_int_cpu <= 0;
		wfile_dat_int_cpu_buf <= 0;
	end
end

always @(posedge wb_cpu_clk_i)
begin
	rfile_ack_int_cpu <= rfile_ack_int_cpu_buf;
	rfile_ack_int_cpu_buf <= rfile_ack_int_sdhc;
	if(wb_cpu_rst_i)
	begin
		rfile_ack_int_cpu <= 0;
		rfile_ack_int_cpu_buf <= 0;
	end
end

always @(posedge wb_sdhc_clk_i)
begin
	wfile_dat_clr_sdhc <= wfile_dat_clr_sdhc_buf;
	wfile_dat_clr_sdhc_buf <= wfile_dat_clr_cpu;
	if(wb_cpu_rst_i)
	begin
		wfile_dat_clr_sdhc <= 0;
		wfile_dat_clr_sdhc_buf <= 0;
	end
end

always @(posedge wb_sdhc_clk_i)
begin
	rfile_ack_clr_sdhc <= rfile_ack_clr_sdhc_buf;
	rfile_ack_clr_sdhc_buf <= rfile_ack_clr_cpu;
	if(wb_cpu_rst_i)
	begin
		rfile_ack_clr_sdhc <= 0;
		rfile_ack_clr_sdhc_buf <= 0;
	end
end

endmodule
