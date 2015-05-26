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
// Create Date:   16:08:02 02/13/2015
// Design Name:   aes_core
// Project Name:  crypto_aes
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: aes_core
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module aes_core_tb;

	// Inputs
	reg clk;
	reg load_i;
	reg [255:0] key_i;
	reg [127:0] data_i;
	reg [1:0] size_i;
	reg dec_i;

	// Outputs
	wire [127:0] data_o;
	wire busy_o;

	reg [127:0] pt;
	reg [127:0] ct;
	
	// Instantiate the Unit Under Test (UUT)
	aes_core uut (
		.clk(clk), 
		.load_i(load_i), 
		.key_i(key_i), 
		.data_i(data_i), 
		.size_i(size_i), 
		.dec_i(dec_i), 
		.data_o(data_o), 
		.busy_o(busy_o)
	);

	initial begin
		// Initialize Inputs
		clk = 0;
		load_i = 0;
		key_i = 0;
		data_i = 0;
		size_i = 0;
		dec_i = 0;

		// Wait 100 ns for global reset to finish
		#100;
      
		// Add stimulus here
		size_i = 2'b00;	
		key_i[255:128] = 128'h00_01_02_03_04_05_06_07_08_09_0a_0b_0c_0d_0e_0f;
		pt = 128'h00_11_22_33_44_55_66_77_88_99_aa_bb_cc_dd_ee_ff;
		ct = 128'h69_c4_e0_d8_6a_7b_04_30_d8_cd_b7_80_70_b4_c5_5a;
		
		dec_i = 0;
		data_i = pt;
		load_i = 1;
		while(!busy_o) #1;
		load_i = 0;
		while(busy_o) #1;
		
		if(data_o != ct)
		begin
			$display("AES 128 encrypt failed");
			$stop;
		end
		
		dec_i = 1;
		data_i = ct;
		load_i = 1;
		while(!busy_o) #1;
		load_i = 0;
		while(busy_o) #1;
		
		if(data_o != pt)
		begin
			$display("AES 128 decrypt failed");
			$stop;
		end

		size_i = 2'b01;	
		key_i[255:64] = 192'h00_01_02_03_04_05_06_07_08_09_0a_0b_0c_0d_0e_0f_10_11_12_13_14_15_16_17;
		pt = 128'h00_11_22_33_44_55_66_77_88_99_aa_bb_cc_dd_ee_ff;
		ct = 128'hdd_a9_7c_a4_86_4c_df_e0_6e_af_70_a0_ec_0d_71_91;
		
		dec_i = 0;
		data_i = pt;
		load_i = 1;
		while(!busy_o) #1;
		load_i = 0;
		while(busy_o) #1;
		
		if(data_o != ct)
		begin
			$display("AES 192 encrypt failed");
			$stop;
		end

		dec_i = 1;
		data_i = ct;
		load_i = 1;
		while(!busy_o) #1;
		load_i = 0;
		while(busy_o) #1;
		
		if(data_o != pt)
		begin
			$display("AES 192 decrypt failed");
			$stop;
		end

		size_i = 2'b10;	
		key_i[255:0] = 256'h00_01_02_03_04_05_06_07_08_09_0a_0b_0c_0d_0e_0f_10_11_12_13_14_15_16_17_18_19_1a_1b_1c_1d_1e_1f;
		pt = 128'h00_11_22_33_44_55_66_77_88_99_aa_bb_cc_dd_ee_ff;
		ct = 128'h8e_a2_b7_ca_51_67_45_bf_ea_fc_49_90_4b_49_60_89;
		
		dec_i = 0;
		data_i = pt;
		load_i = 1;
		while(!busy_o) #1;
		load_i = 0;
		while(busy_o) #1;
		
		if(data_o != ct)
		begin
			$display("AES 256 encrypt failed");
			$stop;
		end

		dec_i = 1;
		data_i = ct;
		load_i = 1;
		while(!busy_o) #1;
		load_i = 0;
		while(busy_o) #1;
		
		if(data_o != pt)
		begin
			$display("AES 256 decrypt failed");
			$stop;
		end

		$display("AES core passed");
		$finish;
	end

always #10 clk <= ~clk;
 
endmodule

