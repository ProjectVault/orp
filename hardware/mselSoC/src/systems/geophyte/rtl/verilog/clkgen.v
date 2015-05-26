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
// Create Date:    18:59:52 11/12/2014 
// Design Name: 
// Module Name:    clkgen 
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
module clkgen(
	// Clock in
	input sys_clk_i,
	// Input reset - active on high
	input sys_rst_i,
	// Wishbone clock, 2x clock, and reset out
	output wb_clk_o,
	output wb_clk2x_o,
	output wb_rst_o
);

// Input buffering
IBUFG sys_clk_in_ibufg(
	.I(sys_clk_i),
	.O(sys_clk_ibufg)
);

// Clocking primitive
//------------------------------------
// Instantiation of the MMCM primitive
//    * Unused inputs are tied off
//    * Unused outputs are labeled unused
wire [15:0] do_unused;
wire drdy_unused;
wire psdone_unused;
wire clkfbout;
wire clkfbout_buf;
wire clkfboutb_unused;
wire clkout0b_unused;
wire clkout1b_unused;
wire clkout2_unused;
wire clkout2b_unused;
wire clkout3_unused;
wire clkout3b_unused;
wire clkout4_unused;
wire clkout5_unused;
wire clkout6_unused;
wire clkfbstopped_unused;
wire clkinstopped_unused;

MMCME2_ADV #(
	.BANDWIDTH("OPTIMIZED"),
	.CLKOUT4_CASCADE("FALSE"),
	.COMPENSATION("ZHOLD"),
	.STARTUP_WAIT("FALSE"),
	.DIVCLK_DIVIDE(1),
	.CLKFBOUT_MULT_F(10.000),
	.CLKFBOUT_PHASE(0.000),
	.CLKFBOUT_USE_FINE_PS ("FALSE"),
	.CLKOUT0_DIVIDE_F(20.000),
	.CLKOUT0_PHASE(0.000),
	.CLKOUT0_DUTY_CYCLE(0.500),
	.CLKOUT0_USE_FINE_PS("FALSE"),
	.CLKOUT1_DIVIDE(10),
	.CLKOUT1_PHASE(0.000),
	.CLKOUT1_DUTY_CYCLE(0.500),
	.CLKOUT1_USE_FINE_PS("FALSE"),
	.CLKIN1_PERIOD(10.000),
	.REF_JITTER1(0.010)
) mmcm_adv_inst(
	.CLKFBOUT(clkfbout),
	.CLKFBOUTB(clkfboutb_unused),
	.CLKOUT0(clkout0),
	.CLKOUT0B(clkout0b_unused),
	.CLKOUT1(clkout1),
	.CLKOUT1B(clkout1b_unused),
	.CLKOUT2(clkout2_unused),
	.CLKOUT2B(clkout2b_unused),
	.CLKOUT3(clkout3_unused),
	.CLKOUT3B(clkout3b_unused),
	.CLKOUT4(clkout4_unused),
	.CLKOUT5(clkout5_unused),
	.CLKOUT6(clkout6_unused),
	// Input clock control
	.CLKFBIN(clkfbout_buf),
	.CLKIN1(sys_clk_ibufg),
	.CLKIN2(1'b0),
	// Tied to always select the primary input clock
	.CLKINSEL(1'b1),
	// Ports for dynamic reconfiguration
	.DADDR(7'h0),
	.DCLK(1'b0),
	.DEN(1'b0),
	.DI(16'h0),
	.DO(do_unused),
	.DRDY(drdy_unused),
	.DWE(1'b0),
	// Ports for dynamic phase shift
	.PSCLK(1'b0),
	.PSEN(1'b0),
	.PSINCDEC(1'b0),
	.PSDONE(psdone_unused),
	// Other control and status signals
	.LOCKED(LOCKED),
	.CLKINSTOPPED(clkinstopped_unused),
	.CLKFBSTOPPED(clkfbstopped_unused),
	.PWRDWN(1'b0),
	.RST(sys_rst_i)
);

// Output buffering
//-----------------------------------
BUFG clkf_buf(
	.O(clkfbout_buf),
	.I(clkfbout)
);

BUFG wb_clk_buf(
	.O(wb_clk_o),
	.I(clkout0)
);


BUFG wb_clk2x_buf(
	.O(wb_clk2x_o),
	.I(clkout1)
);


reg [15:0] wb_rst_shr;
always @(posedge wb_clk_o or posedge sys_rst_i)
begin
	if(sys_rst_i)
		wb_rst_shr <= 16'hffff;
	else
		wb_rst_shr <= {wb_rst_shr[14:0], ~(LOCKED)};
end

assign wb_rst_o = wb_rst_shr[15];

endmodule
