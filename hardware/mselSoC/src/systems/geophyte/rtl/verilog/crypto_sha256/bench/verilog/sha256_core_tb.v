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
// Create Date:   21:28:10 02/03/2015
// Design Name:   sha256_core
// Project Name:  crypto_sha256
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: sha256_core
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module sha256_core_tb;

	// Inputs
	reg clk;
	reg rst_i;
	reg load_i;
	reg [511:0] data_i;
	reg [255:0] state_i;
	reg [255:0] test_val;
	
	// Outputs
	wire [255:0] state_o;
	wire busy_o;

	// Instantiate the Unit Under Test (UUT)
	sha256_core uut (
		.clk(clk), 
		//.rst_i(rst_i),
		.load_i(load_i), 
		.data_i(data_i), 
		.state_i(state_i), 
		.state_o(state_o), 
		.busy_o(busy_o)
	);

	initial begin
		// Initialize Inputs
		clk = 0;
		rst_i = 1;
		load_i = 0;
		data_i = 0;
		state_i = 0;

		// Wait 100 ns for global reset to finish
		#100;
        
		// Add stimulus here
		rst_i = 0;
		state_i = {
			32'h6a09e667, 32'hbb67ae85, 32'h3c6ef372, 32'ha54ff53a,
			32'h510e527f, 32'h9b05688c, 32'h1f83d9ab, 32'h5be0cd19
		};
		data_i = {
			32'h61626380, 32'h00000000, 32'h00000000, 32'h00000000,
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000,
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000,
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000018
		};
		test_val = {
			32'hba7816bf, 32'h8f01cfea, 32'h414140de, 32'h5dae2223,
			32'hb00361a3, 32'h96177a9c, 32'hb410ff61, 32'hf20015ad
		};
		
		load_i = 1;
		while(!busy_o) #1;
		load_i = 0;
		while(busy_o) #1;
		if(test_val != state_o)
		begin
			$display("test #1 fail");
			$stop;
		end
		$display("sha256_core passed");
		$finish;
	end
   
always #10 clk <= ~clk;

endmodule

