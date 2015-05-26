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

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   18:26:44 02/02/2015
// Design Name:   sha256_W
// Project Name:  crypto_sha256
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: sha256_W
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module sha256_W_tb;

	// Inputs
	reg clk;
	reg load_i;
	reg busy_i;
	reg [511:0] data_i;
	
	reg [31:0] i;
	reg [32 * 64 - 1:0] test_data;
	
	// Outputs
	wire [31:0] W_o;

	wire [31:0] test_val;
	
	// Instantiate the Unit Under Test (UUT)
	sha256_W uut (
		.clk(clk), 
		.load_i(load_i), 
		.busy_i(busy_i), 
		.data_i(data_i), 
		.W_o(W_o)
	);

	initial begin
		// Initialize Inputs
		clk = 0;
		load_i = 0;
		busy_i = 0;
		data_i = 0;

		// Wait 100 ns for global reset to finish
		#100;
        
		// Add stimulus here
		data_i = {
			32'h61626380, 32'h00000000, 32'h00000000, 32'h00000000,
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000,
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000,
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000018
		};
		test_data = {
			32'h61626380, 32'h00000000, 32'h00000000, 32'h00000000,
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000,
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000,
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000018,
			32'h61626380, 32'h000f0000, 32'h7da86405, 32'h600003c6,
			32'h3e9d7b78, 32'h0183fc00, 32'h12dcbfdb, 32'he2e2c38e,
			32'hc8215c1a, 32'hb73679a2, 32'he5bc3909, 32'h32663c5b,
			32'h9d209d67, 32'hec8726cb, 32'h702138a4, 32'hd3b7973b,
			32'h93f5997f, 32'h3b68ba73, 32'haff4ffc1, 32'hf10a5c62,
			32'h0a8b3996, 32'h72af830a, 32'h9409e33e, 32'h24641522,
			32'h9f47bf94, 32'hf0a64f5a, 32'h3e246a79, 32'h27333ba3,
			32'h0c4763f2, 32'h840abf27, 32'h7a290d5d, 32'h065c43da,
			32'hfb3e89cb, 32'hcc7617db, 32'hb9e66c34, 32'ha9993667,
			32'h84badedd, 32'hc21462bc, 32'h1487472c, 32'hb20f7a99,
			32'hef57b9cd, 32'hebe6b238, 32'h9fe3095e, 32'h78bc8d4b,
			32'ha43fcf15, 32'h668b2ff8, 32'heeaba2cc, 32'h12b1edeb,
			32'hb80a5a34, 32'h1ce797d5, 32'h310fc1d7, 32'ha31968ce,
			32'h7d8ce042, 32'hd5892c87, 32'h0ef2e48b, 32'h86476aaa
		};
		load_i = 1;
		#20;
		load_i = 0;
		busy_i = 1;
		
		for(i = 0; i < 64; i = i + 1)
		begin
			if(test_val != W_o)
			begin
				$display("MISMATCH: %d %h != %h",
						i, test_val, W_o);
				$stop;
			end
			test_data <= test_data << 32;
			#20;
		end
		$display("sha256_W passed");
		$finish;
	end
   
always #10 clk <= ~clk;
   
assign test_val = test_data[32 * 80 - 1:32 * 80 - 32];
endmodule

