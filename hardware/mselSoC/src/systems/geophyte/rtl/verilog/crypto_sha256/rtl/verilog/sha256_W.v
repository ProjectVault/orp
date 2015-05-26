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
// Create Date:    18:20:43 02/02/2015 
// Design Name: 
// Module Name:    sha256_W 
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
module sha256_W (
    input clk,
    input load_i,
	 input busy_i,
	 input [511:0] data_i,
    output [31:0] W_o
);

`define DATA_IDX(x) data_i[512 - (x * 32) - 1:512 - (x * 32) - 32]

reg [31:0] W[15:0];
reg [31:0] W_new00, W_new01, W_new02, W_new03, W_new04, W_new05, W_new06, W_new07;
reg [31:0] W_new08, W_new09, W_new10, W_new11, W_new12, W_new13, W_new14, W_new15;
reg [31:0] W_new;
reg [31:0] h0, h1, h0_new, h1_new;

always @(posedge clk)
begin	
	W[ 0] <= W_new00;
	W[ 1] <= W_new01;
	W[ 2] <= W_new02;
	W[ 3] <= W_new03;
	W[ 4] <= W_new04;
	W[ 5] <= W_new05;
	W[ 6] <= W_new06;
	W[ 7] <= W_new07;
	W[ 8] <= W_new08;
	W[ 9] <= W_new09;
	W[10] <= W_new10;
	W[11] <= W_new11;
	W[12] <= W_new12;
	W[13] <= W_new13;
	W[14] <= W_new14;
	W[15] <= W_new15;
		
	h0 <= h0_new;
	h1 <= h1_new;
end

assign W_o = W[0];

always @*
begin : W_update
	reg [31:0] w_0, w_1, w_9, w_14, d0, d1;
	
	W_new00 = 0;
	W_new01 = 0;
	W_new02 = 0;
	W_new03 = 0;
	W_new04 = 0;
	W_new05 = 0;
	W_new06 = 0;
	W_new07 = 0;
	W_new08 = 0;
	W_new09 = 0;
	W_new10 = 0;
	W_new11 = 0;
	W_new12 = 0;
	W_new13 = 0;
	W_new14 = 0;
	W_new15 = 0;

	w_0 = W[1];
	w_1 = W[2];
	w_9 = W[10];
	w_14 = W[15];
	
	W_new = h0 + h1;

	if(load_i)
	begin
		W_new00 = `DATA_IDX( 0);
		W_new01 = `DATA_IDX( 1);
		W_new02 = `DATA_IDX( 2);
		W_new03 = `DATA_IDX( 3);
		W_new04 = `DATA_IDX( 4);
		W_new05 = `DATA_IDX( 5);
		W_new06 = `DATA_IDX( 6);
		W_new07 = `DATA_IDX( 7);
		W_new08 = `DATA_IDX( 8);
		W_new09 = `DATA_IDX( 9);
		W_new10 = `DATA_IDX(10);
		W_new11 = `DATA_IDX(11);
		W_new12 = `DATA_IDX(12);
		W_new13 = `DATA_IDX(13);
		W_new14 = `DATA_IDX(14);
		W_new15 = `DATA_IDX(15);
		
		w_0 = `DATA_IDX(0);
		w_1 = `DATA_IDX(1);
		w_9 = `DATA_IDX(9);
		w_14 = `DATA_IDX(14);
	end
	else if(busy_i)
	begin
		W_new00 = W[ 1];
		W_new01 = W[ 2];
		W_new02 = W[ 3];
		W_new03 = W[ 4];
		W_new04 = W[ 5];
		W_new05 = W[ 6];
		W_new06 = W[ 7];
		W_new07 = W[ 8];
		W_new08 = W[ 9];
		W_new09 = W[10];
		W_new10 = W[11];
		W_new11 = W[12];
		W_new12 = W[13];
		W_new13 = W[14];
		W_new14 = W[15];
		W_new15 = W_new;
	end
	
	d0 = {w_1[ 6: 0], w_1[31: 7]} ^ {w_1[17: 0], w_1[31:18]} ^ {3'h0, w_1[31: 3]};
	d1 = {w_14[16: 0], w_14[31:17]} ^ {w_14[18: 0], w_14[31:19]} ^ {10'h0, w_14[31:10]};
	
	h0_new = d0 + w_0;
	h1_new = d1 + w_9;
end

endmodule

