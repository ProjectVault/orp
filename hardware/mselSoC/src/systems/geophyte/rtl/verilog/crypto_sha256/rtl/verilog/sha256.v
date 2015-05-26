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
// Create Date:    13:49:03 02/02/2015 
// Design Name: 
// Module Name:    sha256 
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
module sha256_core (	
	input clk,
	input load_i,
	input [511:0] data_i,
	input [255:0] state_i,
	output [255:0] state_o,
	output reg busy_o
);

localparam SHA256_MAX_STEP = 64;

`define STATE_LOAD(i) state_i[256 - (i * 32) - 1: 256 - (i * 32) - 32]

reg [31:0] A, B, C, D, E, F, G, H;
reg [31:0] A_new, B_new, C_new, D_new, E_new, F_new, G_new, H_new;
reg [31:0] HKW, HKW_new;
reg [6:0] step, step_new;
reg busy_new;
wire [31:0] W, K;

assign state_o = { A, B, C, D, E, F, G, H };
	
sha256_W W_inst(
	.clk(clk),
	.data_i(data_i),
	.load_i(load_i),
	.busy_i(busy_o),
	.W_o(W)
);

sha256_K K_inst(
	.step_i(step[5:0]),
	.K_o(K)
);

always @(posedge clk)
begin
	busy_o <= busy_new;
	step <= step_new;
	A <= A_new;
	B <= B_new;
	C <= C_new;
	D <= D_new;
	E <= E_new;
	F <= F_new;
	G <= G_new;
	H <= H_new;
	HKW <= HKW_new;
end

always @*
begin
	step_new = 0;
	if(~load_i & busy_o)
		step_new = step + 1;
end

always @*
begin : HKW_update
	reg [31:0] H_pre;
	
	H_pre = G;
	if(step == 0)
		H_pre = `STATE_LOAD(7);	
		
	HKW_new = H_pre + K + W;
end

reg [31:0] T1, T2;

always @*
begin : T1_update
	reg [31:0] Ch, S1;
	Ch = (E & F) ^ (~E & G);
	S1 = {E[5:0],E[31:6]} ^ {E[10:0],E[31:11]} ^ {E[24:0],E[31:25]};
	T1 = S1 + Ch + HKW;
end

always @*
begin : T2_update
	reg [31:0] Maj, S0;
	
	Maj = (A & (B ^ C)) ^ (B & C);
	S0 = {A[1:0],A[31:2]} ^ {A[12:0],A[31:13]} ^ {A[21:0],A[31:22]};
	T2 = S0 + Maj;
end

always @*
begin
	busy_new = 0;
	
	A_new = A;
	B_new = B;
	C_new = C;
	D_new = D;
	E_new = E;
	F_new = F;
	G_new = G;
	H_new = H;
	
	if(load_i)
	begin
		busy_new = 1;
		
		A_new = `STATE_LOAD(0);
		B_new = `STATE_LOAD(1);
		C_new = `STATE_LOAD(2);
		D_new = `STATE_LOAD(3);
		E_new = `STATE_LOAD(4);
		F_new = `STATE_LOAD(5);
		G_new = `STATE_LOAD(6);
		H_new = `STATE_LOAD(7);
	end
	else if(busy_o)
	begin
		if(step == SHA256_MAX_STEP + 1)
		begin			
			A_new = A + `STATE_LOAD(0);
			B_new = B + `STATE_LOAD(1);
			C_new = C + `STATE_LOAD(2);
			D_new = D + `STATE_LOAD(3);
			E_new = E + `STATE_LOAD(4);
			F_new = F + `STATE_LOAD(5);
			G_new = G + `STATE_LOAD(6);
			H_new = H + `STATE_LOAD(7);
		end
		else if(step == 0)
		begin
			busy_new = 1;
			
			A_new = A;
			B_new = B;
			C_new = C;
			D_new = D;
			E_new = E;
			F_new = F;
			G_new = G;
			H_new = H;
		end
		else
		begin
			busy_new = 1;
			
			A_new = T1 + T2;
			B_new = A;
			C_new = B;
			D_new = C;
			E_new = D + T1;
			F_new = E;
			G_new = F;
			H_new = G;
		end
	end
end

endmodule

