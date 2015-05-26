`include "nandc_def.vh"
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

module nandc_ecc_dual_master #(
   //
   // PORT 0 (FTL)
   //

   // base offset on the wishbone bus that everything is located at
   parameter WB0_FLASH_BASEADDR = `WB0_FLASH_BASEADDR,
   
   // the offset to place wishbone registers at on the bus
   parameter WB0_REG_BASEADDR   = `WB0_REG_BASEADDR,
   
   // the start block that this nand controller is able to address
   parameter WB0_FLASH_S        = `WB0_FLASH_S,
   
   // the number of blocks this nand controller is able to address (out of 1024)
   parameter WB0_FLASH_N        = `WB0_FLASH_N,


   //
   // PORT 1 (CPU)
   //

   // base offset on the wishbone bus that everything is located at (on CPU side)
   parameter WBCPU_FLASH_BASEADDR = `WBCPU_FLASH_BASEADDR,
   
   // the offset to place wishbone registers at on the bus (on CPU side)
   parameter WBCPU_REG_BASEADDR   = `WBCPU_REG_BASEADDR,

   // base offset on the wishbone bus that everything is located at (on NANDC side)
   parameter WB1_FLASH_BASEADDR   = `WB1_FLASH_BASEADDR,
   
   // the offset to place wishbone registers at on the bus (on NANDC side)
   parameter WB1_REG_BASEADDR     = `WB1_REG_BASEADDR,
   
   // the start block that this nand controller is able to address
   parameter WB1_FLASH_S          = `WB1_FLASH_S,
   
   // the number of blocks this nand controller is able to address (out of 1024)
   parameter WB1_FLASH_N          = `WB1_FLASH_N
) (
   input  wire        wb_clk,    // wishbone clock
   input  wire        wb_rst,    // reset synchronous with wb_clk
   
   input  wire [2:0]  wbs0_cti_i, // type   - cycle type identifier, supports either 000 "Classic cycle" or 010 "Incrementing burst cycle"
   input  wire [1:0]  wbs0_bte_i, // exten  - burst type extension, only supports 00 "Linear burst"
   input  wire [31:0] wbs0_adr_i, // addr   - bus address
   output wire [31:0] wbs0_dat_o, // data   - write data output
   input  wire [31:0] wbs0_dat_i, // data   - write data input
   input  wire [3:0]  wbs0_sel_i, // select - 8-bit enable for data bus
   input  wire        wbs0_cyc_i, // cycle  - valid bus cycle is in progress
   input  wire        wbs0_stb_i, // strobe - slave is selected
   input  wire        wbs0_we_i,  // write  - bus cycle is in write mode
   output wire        wbs0_ack_o, // ack    - end of a normal bus cycle

   input  wire [2:0]  wbs1_cti_i, // type   - cycle type identifier, supports either 000 "Classic cycle" or 010 "Incrementing burst cycle"
   input  wire [1:0]  wbs1_bte_i, // exten  - burst type extension, only supports 00 "Linear burst"
   input  wire [31:0] wbs1_adr_i, // addr   - bus address
   output wire [31:0] wbs1_dat_o, // data   - write data output
   input  wire [31:0] wbs1_dat_i, // data   - write data input
   input  wire [3:0]  wbs1_sel_i, // select - 8-bit enable for data bus
   input  wire        wbs1_cyc_i, // cycle  - valid bus cycle is in progress
   input  wire        wbs1_stb_i, // strobe - slave is selected
   input  wire        wbs1_we_i,  // write  - bus cycle is in write mode
   output wire        wbs1_ack_o, // ack    - end of a normal bus cycle
   
   input  wire [7:0]  IO_i, // io    - data input from flash
   output wire [7:0]  IO_o, // io    - data output to flash
   output wire [7:0]  IO_t, // io    - data tristate control
   output wire        CLE,  // cle   - command latch enable
   output wire        ALE,  // ale   - address latch enable
   output wire        CE_n, // ce    - chip enable
   output wire        WE_n, // we    - write enable
   output wire        RE_n, // re    - read enable
   output wire        WP_n, // wp    - write protect enable
   input  wire        RB_n  // rb    - read/busy signal from flash
);

`include "nandc_const.vh"

wire [2:0]  wbm0_cti_o;
wire [1:0]  wbm0_bte_o;
wire [31:0] wbm0_adr_o, wbm0_dat_i, wbm0_dat_o;
wire [3:0]  wbm0_sel_o;
wire        wbm0_cyc_o;
wire        wbm0_stb_o;
wire        wbm0_we_o;
wire        wbm0_ack_i;

nandc_ecc_inline #(
   .WB_FLASH_BASEADDR ( WB0_FLASH_BASEADDR ),
   .WB_REG_BASEADDR   ( WB0_REG_BASEADDR ),
   .WB_FLASH_S        ( WB0_FLASH_S ),
   .WB_FLASH_N        ( WB0_FLASH_N )
) nandc_ecc_inline (
   .wb_clk    ( wb_clk ),
   .wb_rst    ( wb_rst ),
   .wbs_cti_i ( wbs0_cti_i ),
   .wbs_bte_i ( wbs0_bte_i ),
   .wbs_adr_i ( wbs0_adr_i ),
   .wbs_dat_o ( wbs0_dat_o ),
   .wbs_dat_i ( wbs0_dat_i ),
   .wbs_sel_i ( wbs0_sel_i ),
   .wbs_cyc_i ( wbs0_cyc_i ),
   .wbs_stb_i ( wbs0_stb_i ),
   .wbs_we_i  ( wbs0_we_i ),
   .wbs_ack_o ( wbs0_ack_o ),
   .wbm_cti_o ( wbm0_cti_o ),
   .wbm_bte_o ( wbm0_bte_o ),
   .wbm_adr_o ( wbm0_adr_o ),
   .wbm_dat_i ( wbm0_dat_i ),
   .wbm_dat_o ( wbm0_dat_o ),
   .wbm_sel_o ( wbm0_sel_o ),
   .wbm_cyc_o ( wbm0_cyc_o ),
   .wbm_stb_o ( wbm0_stb_o ),
   .wbm_we_o  ( wbm0_we_o ),
   .wbm_ack_i ( wbm0_ack_i )
);

wire [2:0]  wbm1_cti_o;
wire [1:0]  wbm1_bte_o;
wire [31:0] wbm1_adr_o, wbm1_dat_i, wbm1_dat_o;
wire [3:0]  wbm1_sel_o;
wire        wbm1_cyc_o;
wire        wbm1_stb_o;
wire        wbm1_we_o;
wire        wbm1_ack_i;

nandc_ecc_inline_cpu #(
   .WBCPU_FLASH_BASEADDR ( WBCPU_FLASH_BASEADDR ),
   .WBCPU_REG_BASEADDR   ( WBCPU_REG_BASEADDR ),
   .WB_FLASH_BASEADDR    ( WB1_FLASH_BASEADDR ),
   .WB_REG_BASEADDR      ( WB1_REG_BASEADDR ),
   .WB_FLASH_S           ( WB1_FLASH_S ),
   .WB_FLASH_N           ( WB1_FLASH_N )
) nandc_ecc_inline_cpu (
   .wb_clk    ( wb_clk ),
   .wb_rst    ( wb_rst ),
   .wbs_cti_i ( wbs1_cti_i ),
   .wbs_bte_i ( wbs1_bte_i ),
   .wbs_adr_i ( wbs1_adr_i ),
   .wbs_dat_o ( wbs1_dat_o ),
   .wbs_dat_i ( wbs1_dat_i ),
   .wbs_sel_i ( wbs1_sel_i ),
   .wbs_cyc_i ( wbs1_cyc_i ),
   .wbs_stb_i ( wbs1_stb_i ),
   .wbs_we_i  ( wbs1_we_i ),
   .wbs_ack_o ( wbs1_ack_o ),
   .wbm_cti_o ( wbm1_cti_o ),
   .wbm_bte_o ( wbm1_bte_o ),
   .wbm_adr_o ( wbm1_adr_o ),
   .wbm_dat_i ( wbm1_dat_i ),
   .wbm_dat_o ( wbm1_dat_o ),
   .wbm_sel_o ( wbm1_sel_o ),
   .wbm_cyc_o ( wbm1_cyc_o ),
   .wbm_stb_o ( wbm1_stb_o ),
   .wbm_we_o  ( wbm1_we_o ),
   .wbm_ack_i ( wbm1_ack_i )
);

reg [1:0] master_sel;

always @(posedge wb_clk) begin
   if(wb_rst) begin
      master_sel <= 'h0;
   end else begin
      case(master_sel)
      'b00: begin
         if(wbm1_cyc_o)
            master_sel <= 2'b10;
         else if(wbm0_cyc_o)
            master_sel <= 2'b01;
      end
      'b01: if(!wbm0_cyc_o) master_sel <= 2'b00;
      'b10: if(!wbm1_cyc_o) master_sel <= 2'b00;
      endcase
   end
end

wire [2:0]  wbm_cti_o = master_sel[1] ? wbm1_cti_o : master_sel[0] ? wbm0_cti_o : 'h0;
wire [1:0]  wbm_bte_o = master_sel[1] ? wbm1_bte_o : master_sel[0] ? wbm0_bte_o : 'h0;
wire [31:0] wbm_adr_o = master_sel[1] ? wbm1_adr_o : master_sel[0] ? wbm0_adr_o : 'h0;
wire [31:0] wbm_dat_o = master_sel[1] ? wbm1_dat_o : master_sel[0] ? wbm0_dat_o : 'h0;
wire [3:0]  wbm_sel_o = master_sel[1] ? wbm1_sel_o : master_sel[0] ? wbm0_sel_o : 'h0;
wire        wbm_cyc_o = master_sel[1] ? wbm1_cyc_o : master_sel[0] ? wbm0_cyc_o : 0;
wire        wbm_stb_o = master_sel[1] ? wbm1_stb_o : master_sel[0] ? wbm0_stb_o : 0;
wire        wbm_we_o  = master_sel[1] ? wbm1_we_o  : master_sel[0] ? wbm0_we_o  : 0;

wire [31:0] wbm_dat_i;
wire        wbm_ack_i;

assign wbm0_dat_i = master_sel[0] ? wbm_dat_i : 'h0;
assign wbm0_ack_i = master_sel[0] & wbm_ack_i;
assign wbm1_dat_i = master_sel[1] ? wbm_dat_i : 'h0;
assign wbm1_ack_i = master_sel[1] & wbm_ack_i;

nandc #(
   .WB_FLASH_BASEADDR ( `WB_FLASH_BASEADDR ),
   .WB_REG_BASEADDR   ( `WB_REG_BASEADDR ),
   .WB_FLASH_S        ( `WB_FLASH_S ),
   .WB_FLASH_N        ( `WB_FLASH_N )  
) nandc (
   .wb_clk    ( wb_clk ),
   .wb_rst    ( wb_rst ),
   .wbs_cti_i ( wbm_cti_o ),
   .wbs_bte_i ( wbm_bte_o ),
   .wbs_adr_i ( wbm_adr_o ),
   .wbs_dat_o ( wbm_dat_i ),
   .wbs_dat_i ( wbm_dat_o ),
   .wbs_sel_i ( wbm_sel_o ),
   .wbs_cyc_i ( wbm_cyc_o ),
   .wbs_stb_i ( wbm_stb_o ),
   .wbs_we_i  ( wbm_we_o ),
   .wbs_ack_o ( wbm_ack_i ),
   .IO_i      ( IO_i ),
   .IO_o      ( IO_o ),
   .IO_t      ( IO_t ),
   .CLE       ( CLE ),
   .ALE       ( ALE ),
   .CE_n      ( CE_n ),
   .WE_n      ( WE_n ),
   .RE_n      ( RE_n ),
   .WP_n      ( WP_n ),
   .RB_n      ( RB_n )
);

endmodule

