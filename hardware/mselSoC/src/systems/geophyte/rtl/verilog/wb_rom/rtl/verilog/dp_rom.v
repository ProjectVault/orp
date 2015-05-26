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
// Create Date:    19:46:48 01/05/2015 
// Design Name: 
// Module Name:    db_rom 
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
module dp_rom #(
	parameter aw = 5,
	parameter memfile = ""
) (
	input clk1,
	input en1,
	input [aw - 1:0] adr1,
	output reg [31:0] dat1,
	input clk2,
	input en2,
	input [aw - 1:0] adr2,
	output reg [31:0] dat2
);

reg [31:0] rom_data[2**aw - 1:0];

initial
	$readmemh(memfile, rom_data);
	
always @(posedge clk1)
begin
	if(en1)
		dat1 <= rom_data[adr1];
end

always @(posedge clk2)
begin
	if(en2)
		dat2 <= rom_data[adr2];
end

endmodule
