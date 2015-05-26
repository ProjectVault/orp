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
// Create Date:    13:54:01 02/02/2015 
// Design Name: 
// Module Name:    sha256_K 
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
module sha256_K (
	input [5:0] step_i,
	output reg [31:0] K_o
);

always @*
begin
	case(step_i)
	0: K_o = 32'h428a2f98;
	1: K_o = 32'h71374491;
	2: K_o = 32'hb5c0fbcf;
	3: K_o = 32'he9b5dba5;
	4: K_o = 32'h3956c25b;
	5: K_o = 32'h59f111f1;
	6: K_o = 32'h923f82a4;
	7: K_o = 32'hab1c5ed5;
	8: K_o = 32'hd807aa98;
	9: K_o = 32'h12835b01;
	10: K_o = 32'h243185be;
	11: K_o = 32'h550c7dc3;
	12: K_o = 32'h72be5d74;
	13: K_o = 32'h80deb1fe;
	14: K_o = 32'h9bdc06a7;
	15: K_o = 32'hc19bf174;
	16: K_o = 32'he49b69c1;
	17: K_o = 32'hefbe4786;
	18: K_o = 32'h0fc19dc6;
	19: K_o = 32'h240ca1cc;
	20: K_o = 32'h2de92c6f;
	21: K_o = 32'h4a7484aa;
	22: K_o = 32'h5cb0a9dc;
	23: K_o = 32'h76f988da;
	24: K_o = 32'h983e5152;
	25: K_o = 32'ha831c66d;
	26: K_o = 32'hb00327c8;
	27: K_o = 32'hbf597fc7;
	28: K_o = 32'hc6e00bf3;
	29: K_o = 32'hd5a79147;
	30: K_o = 32'h06ca6351;
	31: K_o = 32'h14292967;
	32: K_o = 32'h27b70a85;
	33: K_o = 32'h2e1b2138;
	34: K_o = 32'h4d2c6dfc;
	35: K_o = 32'h53380d13;
	36: K_o = 32'h650a7354;
	37: K_o = 32'h766a0abb;
	38: K_o = 32'h81c2c92e;
	39: K_o = 32'h92722c85;
	40: K_o = 32'ha2bfe8a1;
	41: K_o = 32'ha81a664b;
	42: K_o = 32'hc24b8b70;
	43: K_o = 32'hc76c51a3;
	44: K_o = 32'hd192e819;
	45: K_o = 32'hd6990624;
	46: K_o = 32'hf40e3585;
	47: K_o = 32'h106aa070;
	48: K_o = 32'h19a4c116;
	49: K_o = 32'h1e376c08;
	50: K_o = 32'h2748774c;
	51: K_o = 32'h34b0bcb5;
	52: K_o = 32'h391c0cb3;
	53: K_o = 32'h4ed8aa4a;
	54: K_o = 32'h5b9cca4f;
	55: K_o = 32'h682e6ff3;
	56: K_o = 32'h748f82ee;
	57: K_o = 32'h78a5636f;
	58: K_o = 32'h84c87814;
	59: K_o = 32'h8cc70208;
	60: K_o = 32'h90befffa;
	61: K_o = 32'ha4506ceb;
	62: K_o = 32'hbef9a3f7;
	63: K_o = 32'hc67178f2;
	endcase
end

endmodule
