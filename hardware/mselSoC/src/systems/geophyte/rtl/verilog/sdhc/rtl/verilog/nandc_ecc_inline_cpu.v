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

module nandc_ecc_inline_cpu #(
   // base offset on the CPU wishbone bus that everything is located at
   parameter WBCPU_FLASH_BASEADDR = `WBCPU_FLASH_BASEADDR,
   
   // the offset to place CPU wishbone registers at on the bus
   parameter WBCPU_REG_BASEADDR   = `WBCPU_REG_BASEADDR,
   
   // base offset on the NAND controller wishbone bus that everything is located at
   parameter WB_FLASH_BASEADDR = `WB_FLASH_BASEADDR,
   
   // the offset where the NAND controller wishbone registers are on the bus
   parameter WB_REG_BASEADDR   = `WB_REG_BASEADDR,
   
   // the start block that this nand controller is able to address
   parameter WB_FLASH_S        = `WB_FLASH_S,
   
   // the number of blocks this nand controller is able to address (out of 1024)
   parameter WB_FLASH_N        = `WB_FLASH_N
) (
   input  wire        wb_clk,    // clock  - bus clock
   input  wire        wb_rst,    // reset synchronous with wb_clk
   
   input  wire [2:0]  wbs_cti_i, // type   - cycle type identifier, supports either 000 "Classic cycle" or 010 "Incrementing burst cycle"
   input  wire [1:0]  wbs_bte_i, // exten  - burst type extension, only supports 00 "Linear burst"
   input  wire [31:0] wbs_adr_i, // addr   - bus address
   output reg  [31:0] wbs_dat_o, // data   - write data output
   input  wire [31:0] wbs_dat_i, // data   - write data input
   input  wire [3:0]  wbs_sel_i, // select - 8-bit enable for data bus
   input  wire        wbs_cyc_i, // cycle  - valid bus cycle is in progress
   input  wire        wbs_stb_i, // strobe - slave is selected
   input  wire        wbs_we_i,  // write  - bus cycle is in write mode
   output reg         wbs_ack_o, // ack   - end of a normal bus cycle
   
   output reg  [2:0]  wbm_cti_o, // type   - cycle type identifier, supports either 000 "Classic cycle" or 010 "Incrementing burst cycle"
   output reg  [1:0]  wbm_bte_o, // exten  - burst type extension, only supports 00 "Linear burst"
   output reg  [31:0] wbm_adr_o, // addr   - bus address
   input  wire [31:0] wbm_dat_i, // data   - write data input
   output reg  [31:0] wbm_dat_o, // data   - write data output
   output reg  [3:0]  wbm_sel_o, // select - 8-bit enable for data bus
   output reg         wbm_cyc_o, // cycle  - valid bus cycle is in progress
   output reg         wbm_stb_o, // strobe - slave is selected
   output reg         wbm_we_o,  // write  - bus cycle is in write mode
   input  wire        wbm_ack_i  // ack   - end of a normal bus cycle
);

`include "wb_common.v"

reg  [31:0] adr_r;
wire        valid = wbs_cyc_i & wbs_stb_i;
reg         valid_r;
wire        new_cycle = valid & !valid_r;
wire [31:0] next_adr = wb_next_adr(adr_r, wbs_cti_i, wbs_bte_i, 32);
wire [31:0] adr = new_cycle ? wbs_adr_i : next_adr;

always @(posedge wb_clk) begin
    if(wb_rst) begin
        adr_r     <= 'h0;
        valid_r   <= 'b0;
        wbs_ack_o <= 'b0;
    end else begin
        adr_r     <= adr;
        valid_r   <= valid;
        wbs_ack_o <= valid & (!((wbs_cti_i == 3'b000) | (wbs_cti_i == 3'b111)) | !wbs_ack_o);
    end
end

wire ram_we = wbs_we_i & valid & wbs_ack_o;
reg [31:0] wbs_dat_o_r;
reg [31:0] wbm_dat_o_r;

`include "nandc_const.vh"

// Wishbone addresses on the CPU bus
//
// The main difference with the CPU bus is that the FLASH space is just the size of 1 page
// and acts as a scratch buffer for reads and writes. To initiate a READ or WRITE operation
// you write the address you wish to read/write to to the READ or WRITE address on the bus.
// To determine if the last operation (READ/WRITE/ERASE/STATUS/etc) is finished, you must
// read a 1 for the READY bit at the STATUS address before initiating another command:
//
// bit 0 - READY:  1 = Operation complete, 0 = Bus Busy, any transactions initiated while busy will be ignored
// bit 1 - STATUS: 1 = The last STATUS command reported back an error, 0 = No errors
//
// If you wish to read the STATUS of the last PROG or ERASE operation, you must write to
// the STATUS register and then poll the STATUS register until READY = 1 at which point
// the STATUS bit will reflect the result of the STATUS command.
//
parameter WBCPU_FLASH_HIGHADDR          = WBCPU_FLASH_BASEADDR + (1 << `FCOLUMNS);
parameter WBCPU_PAGE_OFFSET_BASEADDR    = WBCPU_REG_BASEADDR   + `WB_PAGE_OFFSET_OFF;
parameter WBCPU_SPARE_SPACE_WR_BASEADDR = WBCPU_REG_BASEADDR   + `WB_SPARE_SPACE_WR_OFF;
parameter WBCPU_SPARE_SPACE_RD_BASEADDR = WBCPU_REG_BASEADDR   + `WB_SPARE_SPACE_RD_OFF;
parameter WBCPU_ERASE_BASEADDR          = WBCPU_REG_BASEADDR   + `WB_ERASE_OFF;
parameter WBCPU_STATUS_BASEADDR         = WBCPU_REG_BASEADDR   + `WB_STATUS_OFF;
parameter WBCPU_WRITE_BASEADDR          = WBCPU_REG_BASEADDR   + `WB_WRITE_OFF;
parameter WBCPU_READ_BASEADDR           = WBCPU_REG_BASEADDR   + `WB_READ_OFF;

// Wishbone addreses on the NANDC bus
parameter WB_FLASH_HIGHADDR             = WB_FLASH_BASEADDR + (WB_FLASH_N << (`FPAGES + `FCOLUMNS));
parameter WB_FLASH_N_BASEADDR           = 0;
parameter WB_FLASH_N_HIGHADDR           = WB_FLASH_N << (`FPAGES + `FCOLUMNS);
parameter WB_PAGE_OFFSET_BASEADDR       = WB_REG_BASEADDR   + `WB_PAGE_OFFSET_OFF;
parameter WB_SPARE_SPACE_WR_BASEADDR    = WB_REG_BASEADDR   + `WB_SPARE_SPACE_WR_OFF;
parameter WB_SPARE_SPACE_RD_BASEADDR    = WB_REG_BASEADDR   + `WB_SPARE_SPACE_RD_OFF;
parameter WB_ERASE_BASEADDR             = WB_REG_BASEADDR   + `WB_ERASE_OFF;
parameter WB_STATUS_BASEADDR            = WB_REG_BASEADDR   + `WB_STATUS_OFF;

reg  [31:0] data_i;
reg         valid_i, sof_i, eof_i;
wire [31:0] data_o;
wire        valid_o, sof_o, eof_o;
wire [23:0] ecc_o;

hamm_4096x1_512x32 hamm_4096x1_512x32 (
   .clk     ( wb_clk ),
   .rst     ( wb_rst ),
   .data_i  ( data_i ),
   .valid_i ( valid_i ),
   .sof_i   ( sof_i ),
   .eof_i   ( eof_i ),
   .data_o  ( data_o ),
   .valid_o ( valid_o ),
   .sof_o   ( sof_o ),
   .eof_o   ( eof_o ),
   .ecc_o   ( ecc_o )
);

reg                  a_wr;
reg  [`FCOLUMNS-2:0] a_addr;
reg  [31:0]          a_din;
wire [31:0]          a_dout;
reg                  b_wr;
reg  [`FCOLUMNS-2:0] b_addr;
wire [31:0]          b_dout;

parameter [4:0] ST_IDLE      = 'd0,
                ST_IDLE_0    = 'd1,
                ST_ERASE     = 'd2,
                ST_STATUS    = 'd3,
                ST_WBWRITE   = 'd4,
                ST_WBWRITE_0 = 'd5,
                ST_WBWRITE_1 = 'd6,
                ST_WBWRITE_2 = 'd7,
                ST_WBWRITE_3 = 'd8,
                ST_WBWRITE_4 = 'd9,
                ST_WBWRITE_5 = 'd10,
                ST_WBREAD    = 'd11,
                ST_WBREAD_0  = 'd12,
                ST_WBREAD_1  = 'd13,
                ST_WBREAD_2  = 'd14,
                ST_WBREAD_3  = 'd15,
                ST_WBREAD_4  = 'd16;

reg        page_offset;
reg [63:0] spare_space_wr, spare_space_rd;
reg [23:0] spare_space_ecc, spare_space_ecc_cmp;
reg spare_space_erased;
reg [1:0]  ecc_addr;
reg [23:0] ecc0, ecc1, ecc2, ecc3;
reg [23:0] ecc0_cmp, ecc1_cmp, ecc2_cmp, ecc3_cmp;
reg [7:0]  i;
reg [31:0] data, addr;
reg [13:0] fix_bit;
reg ready;
reg status;
reg [4:0]  hstate;

always @(posedge wb_clk) begin
   if(wb_rst) begin
      wbs_dat_o_r <= 'h0;
      wbm_cti_o  <= WB_CTI_CLASSIC;
      wbm_bte_o  <= WB_BTE_LINEAR;
      wbm_adr_o  <= 'h0;
      wbm_dat_o_r <= 'h0;
      wbm_sel_o  <= 'h0;
      wbm_cyc_o  <= 0;
      wbm_stb_o  <= 0;
      wbm_we_o   <= 0;
      page_offset    <= 0;
      spare_space_wr <= 'h0;
      spare_space_rd <= 'h0;
      spare_space_ecc     <= 'h0;
      spare_space_ecc_cmp <= 'h0;
      spare_space_erased  <= 0;
      ecc_addr   <= 'h0;
      ecc0       <= 'h0;
      ecc1       <= 'h0;
      ecc2       <= 'h0;
      ecc3       <= 'h0;
      ecc0_cmp   <= 'h0;
      ecc1_cmp   <= 'h0;
      ecc2_cmp   <= 'h0;
      ecc3_cmp   <= 'h0;
      i          <= 'h0;
      data       <= 'h0;
      fix_bit    <= 'h0;
      ready      <= 1;
      status     <= 0;
      hstate     <= ST_IDLE;
   end else begin
      case(hstate)
      ST_IDLE: begin
         wbm_cti_o <= WB_CTI_CLASSIC;
         wbm_bte_o <= WB_BTE_LINEAR;
         wbm_adr_o <= 'h0;
         wbm_dat_o_r <= 'h0;
         wbm_sel_o <= 'h0;
         wbm_stb_o <= 0;
         wbm_we_o  <= 0;
         valid_i   <= 0;
         sof_i     <= 0;
         eof_i     <= 0;
         i         <= 'h0;
         data      <= 'h0;
         ready     <= 1;
         hstate    <= ST_IDLE_0;
      end
      ST_IDLE_0: begin
         if(valid) begin
            if(wbs_we_i & wbs_ack_o) begin
               case(adr_r)
               // set ECC controller registers
               WBCPU_PAGE_OFFSET_BASEADDR: begin
                  page_offset <= wbs_dat_i[0];
                  hstate      <= ST_IDLE;
               end
               WBCPU_SPARE_SPACE_WR_BASEADDR + 'h0: begin
                  spare_space_wr[31:0] <= wbs_dat_i;
                  hstate      <= ST_IDLE;
               end
               WBCPU_SPARE_SPACE_WR_BASEADDR + 'h4: begin
                  spare_space_wr[63:32] <= wbs_dat_i;
                  hstate      <= ST_IDLE;
               end
               WBCPU_ERASE_BASEADDR: begin
                  // make sure we're erasing a block in our range
                  if(wbs_dat_i < WB_FLASH_N) begin
                     // offset erase by WB_FLASH_S
                     wbm_dat_o_r <= wbs_dat_i + WB_FLASH_S;
                     ready       <= 0;
                     hstate      <= ST_ERASE;
                  end else begin
                     hstate    <= ST_IDLE;
                  end
               end
               WBCPU_STATUS_BASEADDR: begin
                  ready     <= 0;
                  hstate    <= ST_STATUS;
               end
               // handle flash read requests within our address boundary
               WBCPU_READ_BASEADDR: begin
                  if((wbs_dat_i >= WB_FLASH_N_BASEADDR) && (wbs_dat_i <= WB_FLASH_N_HIGHADDR)) begin
                     // if we're reading from the correct range, then construct new address offset by WB_FLASH_S
                     wbm_adr_o <= {wbs_dat_i[31:`FALL+1], wbs_dat_i[`FALL-1:`FCOLUMNS], {`FCOLUMNS+1{1'b0}}} + {WB_FLASH_S, {`FCOLUMNS + `FPAGES + 1{1'b0}}};
                     ready     <= 0;
                     hstate    <= ST_WBREAD;
                  end else begin
                     hstate    <= ST_IDLE;
                  end
               end
               // handle flash write requests within our address boundary
               WBCPU_WRITE_BASEADDR: begin
                  if((wbs_dat_i >= WB_FLASH_N_BASEADDR) && (wbs_dat_i <= WB_FLASH_N_HIGHADDR) &&
                    // don't allow the user to write when page_offset is set
                    !page_offset) begin
                     // if we're writing to the correct range, then construct new address offset by WB_FLASH_S
                     wbm_adr_o <= {wbs_dat_i[31:`FALL+1], wbs_dat_i[`FALL-1:`FCOLUMNS], {`FCOLUMNS+1{1'b0}}} + {WB_FLASH_S, {`FCOLUMNS + `FPAGES + 1{1'b0}}};
                     ready     <= 0;
                     hstate    <= ST_WBWRITE;
                  end else begin
                     hstate    <= ST_IDLE;
                  end
               end
               endcase
            end else if(!wbs_we_i) begin
               case(adr)
               // read back ECC controller registers
               WBCPU_PAGE_OFFSET_BASEADDR: begin
                  wbs_dat_o_r <= {31'h0, page_offset};
                  hstate    <= ST_IDLE;
               end
               WBCPU_SPARE_SPACE_WR_BASEADDR + 'h0: begin
                  wbs_dat_o_r <= spare_space_wr[31:0];
                  hstate    <= ST_IDLE;
               end
               WBCPU_SPARE_SPACE_WR_BASEADDR + 'h4: begin
                  wbs_dat_o_r <= spare_space_wr[63:32];
                  hstate    <= ST_IDLE;
               end
               WBCPU_SPARE_SPACE_RD_BASEADDR + 'h0: begin
                  wbs_dat_o_r <= spare_space_erased ? 32'hffffffff : spare_space_rd[31:0];
                  hstate    <= ST_IDLE;
               end
               WBCPU_SPARE_SPACE_RD_BASEADDR + 'h4: begin
                  wbs_dat_o_r <= spare_space_erased ? 32'hffffffff : spare_space_rd[63:32];
                  hstate    <= ST_IDLE;
               end
               WBCPU_STATUS_BASEADDR: begin
                  wbs_dat_o_r <= {30'h0, status, ready};
                  hstate    <= ST_IDLE;
               end
               endcase
            end
         end
      end
      
      
      //
      // ERASE FORWARD
      //
      // forward this transaction until we get an ack
      ST_ERASE: begin
         if(!wbm_ack_i) begin
            wbm_cti_o <= WB_CTI_CLASSIC;
            wbm_bte_o <= WB_BTE_LINEAR;
            wbm_adr_o <= WB_ERASE_BASEADDR;
            wbm_sel_o <= 1;
            wbm_cyc_o <= 1;
            wbm_stb_o <= 1;
            wbm_we_o  <= 1;
         end else begin
            wbm_cti_o <= WB_CTI_CLASSIC;
            wbm_bte_o <= WB_BTE_LINEAR;
            wbm_adr_o <= 'h0;
            wbm_dat_o_r <= 'h0;
            wbm_sel_o <= 'h0;
            wbm_cyc_o <= 0;
            wbm_stb_o <= 0;
            wbm_we_o  <= 0;
     
            hstate <= ST_IDLE;         
         end
      end
      
      
      //
      // STATUS FORWARD
      //
      // forward this transaction until we get an ack
      ST_STATUS: begin
         if(!wbm_ack_i) begin
            wbm_cti_o <= WB_CTI_CLASSIC;
            wbm_bte_o <= WB_BTE_LINEAR;
            wbm_adr_o <= WB_STATUS_BASEADDR;
            wbm_sel_o <= 1;
            wbm_cyc_o <= 1;
            wbm_stb_o <= 1;
            wbm_we_o  <= 0;
         end else begin
            wbm_cti_o <= WB_CTI_CLASSIC;
            wbm_bte_o <= WB_BTE_LINEAR;
            wbm_adr_o <= 'h0;
            wbm_dat_o_r <= 'h0;
            wbm_sel_o <= 'h0;
            wbm_cyc_o <= 0;
            wbm_stb_o <= 0;
            wbm_we_o  <= 0;
            status    <= wbm_dat_i[0];

            hstate <= ST_IDLE;
         end
      end
      
      
      //
      // WRITE WITH ECC
      //
      // forward this transaction to the slave and compute and add ecc while we're doing it
      ST_WBWRITE: begin
         // insert dummy cycle to read data from BRAM buffer
         hstate <= ST_WBWRITE_0;
      end
      ST_WBWRITE_0: begin
         wbm_cti_o <= WB_CTI_INCR_BURST;
         wbm_bte_o <= WB_BTE_LINEAR;
         wbm_sel_o <= 'b1111;
         wbm_cyc_o <= 1;
         wbm_we_o  <= 1;

         if(wbm_ack_i) begin
            // drop strobe when we have an ack
            wbm_stb_o <= 0;
            wbm_adr_o <= wbm_adr_o + 'h4;
            data_i   <= wbm_dat_o;
            valid_i  <= 1;
            sof_i    <= wbm_adr_o[8:2] == {9-2{1'b0}};
            eof_i    <= wbm_adr_o[8:2] == {9-2{1'b1}};
            ecc_addr <= wbm_adr_o[10:9];
            
            // if we're in the last word of our 512-byte block then save ECC
            if((wbm_adr_o[8:2] == {9-2{1'b1}}))
               hstate   <= ST_WBWRITE_1;
         end else begin
            // otherwise keep strobe high
            wbm_stb_o <= 1;
            valid_i <= 0;
            sof_i   <= 0;
            eof_i   <= 0;
         end
      end
      ST_WBWRITE_1: begin
         valid_i <= 0;
         sof_i   <= 0;
         eof_i   <= 0;
         if(eof_o) begin
            case(ecc_addr)
            0: ecc0 <= ecc_o;
            1: ecc1 <= ecc_o;
            2: ecc2 <= ecc_o;
            3: ecc3 <= ecc_o;
            endcase
            // if we're in the last 512-byte block, then also write ecc, spare_space, and spare_space_ecc
            if(ecc_addr == 3) begin
               hstate   <= ST_WBWRITE_2;
            // if we're not in the last 512-byte block, then ack and return to idle state
            end else begin
               hstate <= ST_WBWRITE_0;
            end
         end
      end
      ST_WBWRITE_2: begin
         data_i  <= spare_space_wr[31:0];
         valid_i <= 1;
         sof_i   <= 1;
         eof_i   <= 0;
         hstate  <= ST_WBWRITE_3;
      end
      ST_WBWRITE_3: begin
         data_i  <= spare_space_wr[63:32];
         valid_i <= 1;
         sof_i   <= 0;
         eof_i   <= 1;
         hstate  <= ST_WBWRITE_4;
      end
      ST_WBWRITE_4: begin
         data_i  <= 'h0;
         valid_i <= 0;
         sof_i   <= 0;
         eof_i   <= 0;
         if(eof_o) begin
            wbm_adr_o       <= wbm_adr_o + 'h4;
            spare_space_ecc <= ecc_o;
            i      <= 0;
            hstate <= ST_WBWRITE_5;
         end
      end
      ST_WBWRITE_5: begin
         case(wbm_ack_i ? i + 'h1 : i)
         0: wbm_dat_o_r <= {ecc1[7:0], ecc0[23:0]};
         1: wbm_dat_o_r <= {ecc2[15:0], ecc1[23:8]};
         2: wbm_dat_o_r <= {ecc3[23:0], ecc2[23:16]};
         3: wbm_dat_o_r <= {spare_space_wr[31:0]};
         4: wbm_dat_o_r <= {spare_space_wr[63:32]};
         5: wbm_dat_o_r <= {20'h0, spare_space_ecc[17:12], spare_space_ecc[5:0]};
         endcase
         
         wbm_cti_o <= i == 5 ? WB_CTI_EOB : WB_CTI_INCR_BURST;
         wbm_bte_o <= WB_BTE_LINEAR;
         wbm_sel_o <= 'b1111;
         wbm_cyc_o <= 1;
         wbm_stb_o <= 1;
         wbm_we_o  <= 1;
         
         if(wbm_ack_i) begin
            wbm_adr_o <= wbm_adr_o + 'h4;
            if(i >= 5) begin
               wbm_cyc_o <= 0;
               hstate    <= ST_IDLE;
            end else
               i     <= i + 1;
         end
      end
      
      
      //
      // READ WITH ECC
      //
      ST_WBREAD: begin
         // start read from address 0 of the selected page
         hstate   <= ST_WBREAD_0;
      end
      // perform read cycle
      ST_WBREAD_0, ST_WBREAD_1: begin if(ST_WBREAD_0) begin
         wbm_cti_o <= addr_page_end(wbm_adr_o) ? WB_CTI_EOB : WB_CTI_INCR_BURST;
         wbm_bte_o <= WB_BTE_LINEAR;
         wbm_sel_o <= 'b1111;
         wbm_cyc_o <= 1;
         wbm_stb_o <= 1;
         wbm_we_o  <= 0;
         
         if(wbm_ack_i) begin
            // calculate ECC on read data
            data_i  <= wbm_dat_i;
            valid_i <= !wbm_adr_o[`FCOLUMNS] || (wbm_adr_o[5:2] == 3) || (wbm_adr_o[5:2] == 4);
            sof_i   <= (!wbm_adr_o[`FCOLUMNS] && (wbm_adr_o[8:2] == {9-2{1'b0}})) || (wbm_adr_o[`FCOLUMNS] && wbm_adr_o[5:2] == 3);
            eof_i   <= (!wbm_adr_o[`FCOLUMNS] && (wbm_adr_o[8:2] == {9-2{1'b1}})) || (wbm_adr_o[`FCOLUMNS] && wbm_adr_o[5:2] == 4);

            if(wbm_adr_o[`FCOLUMNS]) begin
               case(wbm_adr_o[5:2])
               0: begin
                  ecc0_cmp[23:0]  <= wbm_dat_i[23:0];
                  ecc1_cmp[7:0]   <= wbm_dat_i[31:24];
               end
               1: begin
                  ecc1_cmp[23:8]  <= wbm_dat_i[15:0];
                  ecc2_cmp[15:0]  <= wbm_dat_i[31:16];
               end
               2: begin
                  ecc2_cmp[23:16] <= wbm_dat_i[7:0];
                  ecc3_cmp[23:0]  <= wbm_dat_i[31:8];
               end
               3: spare_space_rd[31:0]  <= wbm_dat_i;
               4: spare_space_rd[63:32] <= wbm_dat_i;
               5: begin
                  spare_space_erased <= |wbm_dat_i[31:12];
                  spare_space_ecc_cmp[23:0] <= {6'h0, wbm_dat_i[11:6], 6'h0, wbm_dat_i[5:0]};
               end
               endcase
            end
            
            if(addr_page_end(wbm_adr_o))
               hstate <= ST_WBREAD_1;
            else begin
               wbm_adr_o <= wbm_adr_o + 'h4;
            end
         end else begin
            valid_i <= 0;
            sof_i   <= 0;
            eof_i   <= 0;
         end
      end
         
      // save ecc and keep looping if we haven't gotten the full page yet   
      if(eof_o) begin
         case({wbm_adr_o[2], wbm_adr_o[`FCOLUMNS:9]})
         1: ecc0 <= ecc_o;
         2: ecc1 <= ecc_o;
         3: ecc2 <= ecc_o;
         4: ecc3 <= ecc_o;
         12: spare_space_ecc <= ecc_o;
         endcase
      end
      if(hstate == ST_WBREAD_1) begin
         wbm_cti_o <= WB_CTI_CLASSIC;
         wbm_bte_o <= WB_BTE_LINEAR;
         wbm_adr_o <= 'h0;
         wbm_sel_o <= 'b0000;
         wbm_cyc_o <= 0;
         wbm_stb_o <= 0;
         wbm_we_o  <= 0;
         hstate <= ST_WBREAD_2;
      end
      end
      // done reading full page, now fix bits
      ST_WBREAD_2: begin
         casez({ecc_err(spare_space_ecc, spare_space_ecc_cmp),
            ecc_err(ecc3, ecc3_cmp),
            ecc_err(ecc2, ecc2_cmp),
            ecc_err(ecc1, ecc1_cmp),
            ecc_err(ecc0, ecc0_cmp)})
         5'bzzzz1: begin
            fix_bit  <= {2'h0, ecc_bit_pos(ecc0, ecc0_cmp)};
            ecc0_cmp <= ecc0;
            $display("fixing bit %d", ecc_bit_pos(ecc0, ecc0_cmp));
            hstate   <= ST_WBREAD_3;
         end
         5'bzzz10: begin
            fix_bit  <= {2'h1, ecc_bit_pos(ecc1, ecc1_cmp)};
            ecc1_cmp <= ecc1;
            $display("fixing bit %d", ecc_bit_pos(ecc1, ecc1_cmp) + 2048);
            hstate   <= ST_WBREAD_3;
         end
         5'bzz100: begin
            fix_bit  <= {2'h2, ecc_bit_pos(ecc2, ecc2_cmp)};
            ecc2_cmp <= ecc2;
            $display("fixing bit %d", ecc_bit_pos(ecc2, ecc2_cmp) + 4096);
            hstate   <= ST_WBREAD_3;
         end
         5'bz1000: begin
            fix_bit  <= {2'h3, ecc_bit_pos(ecc3, ecc2_cmp)};
            ecc3_cmp <= ecc3;
            $display("fixing bit %d", ecc_bit_pos(ecc3, ecc3_cmp) + 6144);
            hstate   <= ST_WBREAD_3;
         end
         5'b10000: begin
            spare_space_rd[ecc_bit_pos(spare_space_ecc, spare_space_ecc_cmp)] <=
              !spare_space_rd[ecc_bit_pos(spare_space_ecc, spare_space_ecc_cmp)];
            $display("fixing spare_space bit %d", ecc_bit_pos(spare_space_ecc, spare_space_ecc_cmp));
            hstate <= ST_IDLE;
         end
         default: begin
            hstate <= ST_IDLE;
         end
         endcase
      end
      // to fix the bit error, first read from the bram
      ST_WBREAD_3: begin
         hstate <= ST_WBREAD_4;
      end
      // then fix it!
      ST_WBREAD_4: begin
         hstate <= ST_WBREAD_2;
      end
      endcase
   end
end


//
// Scratch BRAM
//
nand_bram_block_dp #(
   .DATA   ( 32 ),
   .ADDR   ( `FCOLUMNS - 1 ),
   .DEPTH  ( `FCOLUMNS_AND_SPARE / 4 )
) nand_bram_block_dp (
   .a_clk  ( wb_clk ),
   .a_wr   ( a_wr ),
   .a_addr ( a_addr ),
   .a_din  ( a_din ),
   .a_dout ( a_dout ),
   .b_clk  ( wb_clk ),
   .b_wr   ( 1'b0 ),      // read only port
   .b_addr ( b_addr ),
   .b_din  ( 'h0 ),
   .b_dout ( b_dout )
);

always @(*) begin
   if(wb_rst) begin
      a_wr   <= 0;
      a_addr <= 'h0;
      a_din  <= 'h0;
      b_addr <= 'h0;
   end else begin
      // handle write to bram from wishbone bus
      if((hstate == ST_IDLE_0) & ram_we & (adr_r >= WBCPU_FLASH_BASEADDR) & (adr_r <= WBCPU_FLASH_HIGHADDR)) begin
         a_addr <= adr_r[`FCOLUMNS-1:2];
         a_wr   <= 1;
         a_din  <= wbs_dat_i;
      // handle read from bram to wishbone bus
      end else if((hstate == ST_IDLE_0) & valid & (adr >= WBCPU_FLASH_BASEADDR) & (adr <= WBCPU_FLASH_HIGHADDR)) begin
         a_addr <= adr[`FCOLUMNS-1:2];
         a_wr   <= 0;
      end else if((hstate == ST_WBREAD_0) & wbm_ack_i & (wbm_adr_o[`FCOLUMNS:2] < (`FCOLUMNS_AND_SPARE/4))) begin
         a_wr   <= 1;
         a_addr <= wbm_adr_o[`FCOLUMNS:2];
         a_din  <= wbm_dat_i;
      end else if(hstate == ST_WBREAD_4) begin
         a_wr   <= 1;
         a_addr <= fix_bit[13:5];
         a_din  <= b_dout ^ (1'b1 << fix_bit[4:0]);
      end else begin
         a_wr   <= 0;
         a_addr <= 'h0;
         a_din  <= 'h0;
      end
      
      // return data for reads from bram on same clock cycle to wishbone bus
      if((hstate == ST_IDLE_0) & valid & (adr_r >= WBCPU_FLASH_BASEADDR) & (adr_r <= WBCPU_FLASH_HIGHADDR)) begin
         wbs_dat_o <= a_dout;
      end else if(valid & !wbs_we_i & (adr == WBCPU_STATUS_BASEADDR)) begin
         wbs_dat_o <= {30'h0, status, ready};
      end else
         wbs_dat_o <= wbs_dat_o_r;
      
      if((hstate == ST_WBWRITE) || (hstate == ST_WBWRITE_0)) begin
         b_addr <= wbm_adr_o[`FCOLUMNS-1:2];
      end else if(hstate == ST_WBREAD) begin
         b_addr <= {page_offset, wbs_adr_i[`FCOLUMNS-1:2]};
      end else if(hstate == ST_WBREAD_3) begin
         b_addr <= fix_bit[13:5];
      end else if(hstate == ST_WBREAD_2) begin
         b_addr <= {page_offset, wbs_adr_i[`FCOLUMNS-1:2]};
      end else
         b_addr <= 'h0;

      // return data to NANDC wishbone bus when issuing writes directly from BRAM
      if(hstate == ST_WBWRITE_0) begin
         wbm_dat_o <= b_dout;
      end else begin
         wbm_dat_o <= wbm_dat_o_r;
      end
   end
end



//
// FUNCTIONS
//

// return true if there's 1 bit error
function ecc_err;
input [23:0] a;
input [23:0] b;
begin
   ecc_err = (a[11:0] ^ a[23:12] ^ b[11:0] ^ b[23:12]) == {12{1'b1}};
end
endfunction

// return position of bit error
function [11:0] ecc_bit_pos;
input [23:0] a;
input [23:0] b;
begin
   ecc_bit_pos = a[23:12] ^ b[23:12];
end
endfunction


endmodule
