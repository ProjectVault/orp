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
// Create Date:   22:19:37 02/11/2015
// Design Name:   aes_ks
// Project Name:  crypto_aes
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: aes_ks
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module aes_ks_tb;

	// Inputs
	reg clk;
	reg load_i;
	reg en_i;
	reg [1:0] size_i;
	reg [255:0] key_i;

	// Outputs
	wire [127:0] ks_o;
	
	reg [3:0] i;
	wire [127:0] test_val;
	reg [60 * 32 - 1:0] test_data;
	
	// Instantiate the Unit Under Test (UUT)
	aes_ks uut (
		.clk(clk), 
		.load_i(load_i), 
		.en_i(en_i), 
		.size_i(size_i), 
		.key_i(key_i), 
		.ks_o(ks_o)
	);

	initial begin
		// Initialize Inputs
		clk = 0;
		load_i = 0;
		size_i = 0;
		key_i = 0;

		// Wait 100 ns for global reset to finish
		#100;
        
		// Add stimulus here
		key_i[255:128] = 128'h2b_7e_15_16_28_ae_d2_a6_ab_f7_15_88_09_cf_4f_3c;
		test_data = {
			32'h2b7e1516, 32'h28aed2a6, 32'habf71588, 32'h09cf4f3c, /* 00 */
			32'ha0fafe17, 32'h88542cb1, 32'h23a33939, 32'h2a6c7605, /* 04 */
			32'hf2c295f2, 32'h7a96b943, 32'h5935807a, 32'h7359f67f, /* 08 */
			32'h3d80477d, 32'h4716fe3e, 32'h1e237e44, 32'h6d7a883b, /* 0c */
			32'hef44a541, 32'ha8525b7f, 32'hb671253b, 32'hdb0bad00, /* 10 */
			32'hd4d1c6f8, 32'h7c839d87, 32'hcaf2b8bc, 32'h11f915bc, /* 14 */
			32'h6d88a37a, 32'h110b3efd, 32'hdbf98641, 32'hca0093fd, /* 18 */
			32'h4e54f70e, 32'h5f5fc9f3, 32'h84a64fb2, 32'h4ea6dc4f, /* 1c */
			32'head27321, 32'hb58dbad2, 32'h312bf560, 32'h7f8d292f, /* 20 */
			32'hac7766f3, 32'h19fadc21, 32'h28d12941, 32'h575c006e, /* 24 */
			32'hd014f9a8, 32'hc9ee2589, 32'he13f0cc8, 32'hb6630ca6, /* 28 */
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, /* 2c */
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, /* 30 */
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, /* 34 */
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000  /* 38 */
		};
		size_i = 2'b00;
		load_i = 1;
		en_i = 1;
		
		#20;
		load_i = 0;
			
		for(i = 0; i < 11; i = i + 1)
		begin
			$display("i: %d test_val: %h ks_o: %h", i, test_val, ks_o);
			if(test_val != ks_o)
			begin
				$display("AES 128 mismatch step %d", i);
				$stop;
			end
			test_data = test_data << 128;
			#20;
		end
		
		key_i[255:64] = 192'h8e_73_b0_f7_da_0e_64_52_c8_10_f3_2b_80_90_79_e5_62_f8_ea_d2_52_2c_6b_7b;
		test_data = {
			32'h8e73b0f7, 32'hda0e6452, 32'hc810f32b, 32'h809079e5, /* 00 */
			32'h62f8ead2, 32'h522c6b7b, 32'hfe0c91f7, 32'h2402f5a5, /* 04 */
			32'hec12068e, 32'h6c827f6b, 32'h0e7a95b9, 32'h5c56fec2, /* 08 */
			32'h4db7b4bd, 32'h69b54118, 32'h85a74796, 32'he92538fd, /* 0c */
			32'he75fad44, 32'hbb095386, 32'h485af057, 32'h21efb14f, /* 10 */
			32'ha448f6d9, 32'h4d6dce24, 32'haa326360, 32'h113b30e6, /* 14 */
			32'ha25e7ed5, 32'h83b1cf9a, 32'h27f93943, 32'h6a94f767, /* 18 */
			32'hc0a69407, 32'hd19da4e1, 32'hec1786eb, 32'h6fa64971, /* 1c */
			32'h485f7032, 32'h22cb8755, 32'he26d1352, 32'h33f0b7b3, /* 20 */
			32'h40beeb28, 32'h2f18a259, 32'h6747d26b, 32'h458c553e, /* 24 */
			32'ha7e1466c, 32'h9411f1df, 32'h821f750a, 32'had07d753, /* 28 */
			32'hca400538, 32'h8fcc5006, 32'h282d166a, 32'hbc3ce7b5, /* 2c */
			32'he98ba06f, 32'h448c773c, 32'h8ecc7204, 32'h01002202, /* 30 */
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000, /* 34 */
			32'h00000000, 32'h00000000, 32'h00000000, 32'h00000000  /* 38 */
		};
		size_i = 2'b01;
		load_i = 1;
		#20;
		load_i = 0;
			
		for(i = 0; i < 13; i = i + 1)
		begin
			$display("i: %d test_val: %h ks_o: %h", i, test_val, ks_o);
			if(test_val != ks_o)
			begin
				$display("AES 192 mismatch step %d", i);
				$stop;
			end
			test_data = test_data << 128;
			#20;
		end

		key_i[255:0] = 256'h60_3d_eb_10_15_ca_71_be_2b_73_ae_f0_85_7d_77_81_1f_35_2c_07_3b_61_08_d7_2d_98_10_a3_09_14_df_f4;
		test_data = {
			32'h603deb10, 32'h15ca71be, 32'h2b73aef0, 32'h857d7781, /* 00 */
			32'h1f352c07, 32'h3b6108d7, 32'h2d9810a3, 32'h0914dff4, /* 04 */
			32'h9ba35411, 32'h8e6925af, 32'ha51a8b5f, 32'h2067fcde, /* 08 */
			32'ha8b09c1a, 32'h93d194cd, 32'hbe49846e, 32'hb75d5b9a, /* 0c */
			32'hd59aecb8, 32'h5bf3c917, 32'hfee94248, 32'hde8ebe96, /* 10 */
			32'hb5a9328a, 32'h2678a647, 32'h98312229, 32'h2f6c79b3, /* 14 */
			32'h812c81ad, 32'hdadf48ba, 32'h24360af2, 32'hfab8b464, /* 18 */
			32'h98c5bfc9, 32'hbebd198e, 32'h268c3ba7, 32'h09e04214, /* 1c */
			32'h68007bac, 32'hb2df3316, 32'h96e939e4, 32'h6c518d80, /* 20 */
			32'hc814e204, 32'h76a9fb8a, 32'h5025c02d, 32'h59c58239, /* 24 */
			32'hde136967, 32'h6ccc5a71, 32'hfa256395, 32'h9674ee15, /* 28 */
			32'h5886ca5d, 32'h2e2f31d7, 32'h7e0af1fa, 32'h27cf73c3, /* 2c */
			32'h749c47ab, 32'h18501dda, 32'he2757e4f, 32'h7401905a, /* 30 */
			32'hcafaaae3, 32'he4d59b34, 32'h9adf6ace, 32'hbd10190d, /* 34 */
			32'hfe4890d1, 32'he6188d0b, 32'h046df344, 32'h706c631e  /* 38 */
		};
		size_i = 2'b10;
		load_i = 1;
		#20;
		load_i = 0;
			
		for(i = 0; i < 15; i = i + 1)
		begin
			$display("i: %d test_val: %h ks_o: %h", i, test_val, ks_o);
			if(test_val != ks_o)
			begin
				$display("AES 256 mismatch step %d", i);
				$stop;
			end
			test_data = test_data << 128;
			#20;
		end
		
		$display("AES key schedule passed");
		$finish;
	end

always #10 clk <= ~clk;
assign test_val = test_data[32 * 60 - 1:32 * 60 - 128];
      
endmodule

