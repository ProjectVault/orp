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

module nandc_ecc_inline #(
   // base offset on the wishbone bus that everything is located at
   parameter WB_FLASH_BASEADDR = `WB_FLASH_BASEADDR,
   
   // the offset to place wishbone registers at on the bus
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

`include "nandc_const.vh"

parameter WB_FLASH_HIGHADDR          = WB_FLASH_BASEADDR + (WB_FLASH_N << (`FPAGES + `FCOLUMNS));
parameter WB_PAGE_OFFSET_BASEADDR    = WB_REG_BASEADDR   + `WB_PAGE_OFFSET_OFF;
parameter WB_SPARE_SPACE_WR_BASEADDR = WB_REG_BASEADDR   + `WB_SPARE_SPACE_WR_OFF;
parameter WB_SPARE_SPACE_RD_BASEADDR = WB_REG_BASEADDR   + `WB_SPARE_SPACE_RD_OFF;
parameter WB_ERASE_BASEADDR          = WB_REG_BASEADDR   + `WB_ERASE_OFF;
parameter WB_STATUS_BASEADDR         = WB_REG_BASEADDR   + `WB_STATUS_OFF;

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
reg  [31:0]          b_din;
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
                ST_WBREAD    = 'd10,
                ST_WBREAD_0  = 'd11,
                ST_WBREAD_1  = 'd12,
                ST_WBREAD_2  = 'd13,
                ST_WBREAD_3  = 'd14,
                ST_WBREAD_4  = 'd15,
                ST_WBREAD_5  = 'd16,
                ST_WBREAD_6  = 'd17;

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
reg [`FPAGES+`FBLOCKS:0] paged, newpaged;
reg [4:0]  hstate;

always @(posedge wb_clk) begin
   if(wb_rst) begin
      wbs_dat_o  <= 'h0;
      wbs_ack_o  <= 0;
      wbm_cti_o  <= WB_CTI_CLASSIC;
      wbm_bte_o  <= WB_BTE_LINEAR;
      wbm_adr_o  <= 'h0;
      wbm_dat_o  <= 'h0;
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
      paged      <= {`FPAGES+`FBLOCKS+1{1'b1}};
      newpaged   <= {`FPAGES+`FBLOCKS+1{1'b1}};
      hstate     <= ST_IDLE;
   end else begin
      case(hstate)
      ST_IDLE: begin
         wbm_cti_o <= WB_CTI_CLASSIC;
         wbm_bte_o <= WB_BTE_LINEAR;
         wbm_adr_o <= 'h0;
         wbm_dat_o <= 'h0;
         wbm_sel_o <= 'h0;
         //wbm_cyc_o <= 0;
         wbm_stb_o <= 0;
         wbm_we_o  <= 0;
         wbs_ack_o <= 0;
         valid_i   <= 0;
         sof_i     <= 0;
         eof_i     <= 0;
         i         <= 'h0;
         data      <= 'h0;
         hstate    <= ST_IDLE_0;
      end
      ST_IDLE_0: begin
         if(wbs_cyc_i & wbs_stb_i) begin
            if(wbs_we_i) begin
               case(wbs_adr_i)
               // set ECC controller registers
               WB_PAGE_OFFSET_BASEADDR: begin
                  page_offset <= wbs_dat_i[0];
                  wbs_ack_o   <= 1;
                  hstate      <= ST_IDLE;
               end
               WB_SPARE_SPACE_WR_BASEADDR + 'h0: begin
                  spare_space_wr[31:0] <= wbs_dat_i;
                  wbs_ack_o   <= 1;
                  hstate      <= ST_IDLE;
               end
               WB_SPARE_SPACE_WR_BASEADDR + 'h4: begin
                  spare_space_wr[63:32] <= wbs_dat_i;
                  wbs_ack_o   <= 1;
                  hstate      <= ST_IDLE;
               end
               WB_ERASE_BASEADDR: begin
                  // make sure we're erasing a block in our range
                  if(wbs_dat_i < WB_FLASH_N) begin
                     // offset erase by WB_FLASH_S
                     wbm_dat_o   <= wbs_dat_i + WB_FLASH_S;
                     hstate      <= ST_ERASE;
                  end else begin
                     wbs_ack_o <= 1;
                     hstate    <= ST_IDLE;
                  end
               end
               // handle flash write requests within our address boundary
               default: begin
                  if((wbs_adr_i >= WB_FLASH_BASEADDR) && (wbs_adr_i <= WB_FLASH_HIGHADDR) &&
                    // don't allow the user to write when page_offset is set
                    !page_offset) begin
                     // if we're writing to the correct range, then construct new address offset by WB_FLASH_S
                     wbm_adr_o <= {wbs_adr_i[31:`FALL+1], wbs_adr_i[`FALL-1:`FCOLUMNS], 1'b0, wbs_adr_i[`FCOLUMNS-1:0]} + {WB_FLASH_S, {`FCOLUMNS + `FPAGES + 1{1'b0}}};
                     hstate   <= ST_WBWRITE;
                  end else begin
                     wbs_ack_o <= 1;
                     hstate    <= ST_IDLE;
                  end
               end
               endcase
            end else begin
               case(wbs_adr_i)
               // read back ECC controller registers
               WB_PAGE_OFFSET_BASEADDR: begin
                  wbs_dat_o <= {31'h0, page_offset};
                  wbs_ack_o <= 1;
                  hstate    <= ST_IDLE;
               end
               WB_SPARE_SPACE_WR_BASEADDR + 'h0: begin
                  wbs_dat_o <= spare_space_wr[31:0];
                  wbs_ack_o <= 1;
                  hstate    <= ST_IDLE;
               end
               WB_SPARE_SPACE_WR_BASEADDR + 'h4: begin
                  wbs_dat_o <= spare_space_wr[63:32];
                  wbs_ack_o <= 1;
                  hstate    <= ST_IDLE;
               end
               WB_SPARE_SPACE_RD_BASEADDR + 'h0: begin
                  wbs_dat_o <= spare_space_erased ? 32'hffffffff : spare_space_rd[31:0];
                  wbs_ack_o <= 1;
                  hstate    <= ST_IDLE;
               end
               WB_SPARE_SPACE_RD_BASEADDR + 'h4: begin
                  wbs_dat_o <= spare_space_erased ? 32'hffffffff : spare_space_rd[63:32];
                  wbs_ack_o <= 1;
                  hstate    <= ST_IDLE;
               end
               WB_STATUS_BASEADDR: begin
                  hstate    <= ST_STATUS;
               end
               // handle flash read requests within our address boundary
               default: begin
                  if((wbs_adr_i >= WB_FLASH_BASEADDR) && (wbs_adr_i <= WB_FLASH_HIGHADDR)) begin
                     // if we're reading from the correct range, then construct new address offset by WB_FLASH_S
                     wbm_adr_o <= {wbs_adr_i[31:`FALL+1], wbs_adr_i[`FALL-1:`FCOLUMNS], {`FCOLUMNS+1{1'b0}}} + {WB_FLASH_S, {`FCOLUMNS + `FPAGES + 1{1'b0}}};
                     hstate    <= ST_WBREAD;
                  end else begin
                     wbs_ack_o <= 1;
                     hstate    <= ST_IDLE;
                  end
               end
               endcase
            end
         end
      end
      
      
      //
      // ERASE FORWARD
      //
      // forward this transaction until we get an ack
      ST_ERASE: if(wbs_stb_i) begin
         if(!wbm_ack_i) begin
            wbm_cti_o <= wbs_cti_i;
            wbm_bte_o <= wbs_bte_i;
            wbm_adr_o <= wbs_adr_i;
            //wbm_dat_o <= wbs_dat_i;
            wbm_sel_o <= wbs_sel_i;
            wbm_cyc_o <= wbs_cyc_i;
            wbm_stb_o <= wbs_stb_i;
            wbm_we_o  <= wbs_we_i;
         end else begin
            wbm_cti_o <= WB_CTI_CLASSIC;
            wbm_bte_o <= WB_BTE_LINEAR;
            wbm_adr_o <= 'h0;
            wbm_dat_o <= 'h0;
            wbm_sel_o <= 'h0;
            wbm_cyc_o <= 0;
            wbm_stb_o <= 0;
            wbm_we_o  <= 0;
            wbs_dat_o <= wbm_dat_i;
            wbs_ack_o <= 1;

            // if we just erased our currently paged block, then invalidate cache
            if({1'b0, wbm_dat_o[`FBLOCKS-1:0]} == paged[`FBLOCKS+`FPAGES-1:`FPAGES])
               paged <= {`FPAGES+`FBLOCKS+1{1'b1}};
               
            hstate <= ST_IDLE;         
         end
      end
      
      
      //
      // STATUS FORWARD
      //
      // forward this transaction until we get an ack
      ST_STATUS: if(wbs_stb_i) begin
         if(!wbm_ack_i) begin
            wbm_cti_o <= wbs_cti_i;
            wbm_bte_o <= wbs_bte_i;
            wbm_adr_o <= wbs_adr_i;
            wbm_dat_o <= wbs_dat_i;
            wbm_sel_o <= wbs_sel_i;
            wbm_cyc_o <= wbs_cyc_i;
            wbm_stb_o <= wbs_stb_i;
            wbm_we_o  <= wbs_we_i;
         end else begin
            wbm_cti_o <= WB_CTI_CLASSIC;
            wbm_bte_o <= WB_BTE_LINEAR;
            wbm_adr_o <= 'h0;
            wbm_dat_o <= 'h0;
            wbm_sel_o <= 'h0;
            wbm_cyc_o <= 0;
            wbm_stb_o <= 0;
            wbm_we_o  <= 0;
            wbs_dat_o <= wbm_dat_i;
            wbs_ack_o <= 1;

            hstate <= ST_IDLE;         
         end
      end
      
      
      //
      // WRITE WITH ECC
      //
      // forward this transaction to the slave and compute and add ecc while we're doing it
      ST_WBWRITE: begin
         if(!wbm_ack_i || (wbs_cti_i == WB_CTI_INCR_BURST) || (wbs_cti_i == WB_CTI_EOB)) begin
            wbm_cti_o <= wbs_cti_i == WB_CTI_CLASSIC ? WB_CTI_CLASSIC : WB_CTI_INCR_BURST;
            wbm_bte_o <= wbs_bte_i;
            wbm_dat_o <= wbs_dat_i;
            wbm_sel_o <= 'b1111;
            wbm_cyc_o <= 1;
            wbm_we_o  <= 1;
         end
         
         // drop strobe if we've been acked on either side
         wbm_stb_o <= !(wbm_ack_i | wbs_ack_o);
         
         if(wbm_ack_i) begin
            if(wbs_cti_i == WB_CTI_CLASSIC) begin
               wbm_sel_o <= 'h0;
               //wbm_cyc_o <= 0;
               wbm_we_o  <= 0;
            end else
               wbm_adr_o <= wbm_adr_o + 'h4;
            
            data_i   <= wbm_dat_o;
            valid_i  <= 1;
            sof_i    <= wbm_adr_o[8:2] == {9-2{1'b0}};
            eof_i    <= wbm_adr_o[8:2] == {9-2{1'b1}};
            ecc_addr <= wbm_adr_o[10:9];
            
            // if we're in the last word of our 512-byte block then save ECC
            if((wbm_adr_o[8:2] == {9-2{1'b1}}) || (wbs_cti_i == WB_CTI_EOB))
               hstate   <= ST_WBWRITE_0;
            else begin
               wbs_ack_o <= 1;
               if(wbs_cti_i == WB_CTI_CLASSIC)
                  hstate <= ST_IDLE;
            end
         end else begin
            valid_i <= 0;
            sof_i   <= 0;
            eof_i   <= 0;
            
            wbs_ack_o <= 0;
         end
      end
      ST_WBWRITE_0: begin
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
               hstate   <= ST_WBWRITE_1;
            // if we're not in the last 512-byte block, then ack and return to idle state
            end else begin
               wbs_ack_o <= 1;
               if(wbs_cti_i == WB_CTI_CLASSIC)
                  hstate <= ST_IDLE;
               else
                  hstate <= ST_WBWRITE;
            end
         end
      end
      ST_WBWRITE_1: begin
         data_i  <= spare_space_wr[31:0];
         valid_i <= 1;
         sof_i   <= 1;
         eof_i   <= 0;
         hstate  <= ST_WBWRITE_2;
      end
      ST_WBWRITE_2: begin
         data_i  <= spare_space_wr[63:32];
         valid_i <= 1;
         sof_i   <= 0;
         eof_i   <= 1;
         hstate  <= ST_WBWRITE_3;
      end
      ST_WBWRITE_3: begin
         data_i  <= 'h0;
         valid_i <= 0;
         sof_i   <= 0;
         eof_i   <= 0;
         if(eof_o) begin
            wbm_adr_o       <= wbm_adr_o + 'h4;
            spare_space_ecc <= ecc_o;
            i      <= 0;
            hstate <= ST_WBWRITE_4;
         end
      end
      ST_WBWRITE_4: begin
         case(wbm_ack_i ? i + 'h1 : i)
         0: wbm_dat_o <= {ecc1[7:0], ecc0[23:0]};
         1: wbm_dat_o <= {ecc2[15:0], ecc1[23:8]};
         2: wbm_dat_o <= {ecc3[23:0], ecc2[23:16]};
         3: wbm_dat_o <= {spare_space_wr[31:0]};
         4: wbm_dat_o <= {spare_space_wr[63:32]};
         5: wbm_dat_o <= {20'h0, spare_space_ecc[17:12], spare_space_ecc[5:0]};
         endcase
         
         wbm_cti_o <= i == 5 ? WB_CTI_EOB : WB_CTI_INCR_BURST;
         wbm_bte_o <= 'b00;
         wbm_sel_o <= 'b1111;
         wbm_cyc_o <= 1;
         wbm_stb_o <= 1;
         wbm_we_o  <= 1;
         
         if(wbm_ack_i) begin //wbm_ack_i
            wbm_adr_o <= wbm_adr_o + 'h4;
            if(i >= 5) begin
               wbs_ack_o <= 1;
               
               // if we just programmed our currently paged block, then invalidate cache
               if({1'b0, wbm_adr_o[`FALL:`FPAGES+`FCOLUMNS+1]} == paged[`FBLOCKS+`FPAGES-1:`FPAGES])
                  paged <= {`FPAGES+`FBLOCKS+1{1'b1}};
               
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
         newpaged  <= {1'b0, wbm_adr_o[`FALL:`FCOLUMNS+1]};

         if({1'b0, wbm_adr_o[`FALL:`FCOLUMNS+1]} == paged) begin
            hstate   <= ST_WBREAD_5;
         end else begin
            // start read from address 0 of the selected page
            hstate   <= ST_WBREAD_0;
         end
      end
      // perform read cycle
      ST_WBREAD_0, ST_WBREAD_1: begin if(ST_WBREAD_0) begin
         wbm_cti_o <= addr_page_end(wbm_adr_o) ? WB_CTI_EOB : WB_CTI_INCR_BURST;
         wbm_bte_o <= 'b00;
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
            hstate   <= ST_WBREAD_3;
         end
         5'bzzz10: begin
            fix_bit  <= {2'h1, ecc_bit_pos(ecc1, ecc1_cmp)};
            ecc1_cmp <= ecc1;
            hstate   <= ST_WBREAD_3;
         end
         5'bzz100: begin
            fix_bit  <= {2'h2, ecc_bit_pos(ecc2, ecc2_cmp)};
            ecc2_cmp <= ecc2;
            hstate   <= ST_WBREAD_3;
         end
         5'bz1000: begin
            fix_bit  <= {2'h3, ecc_bit_pos(ecc3, ecc2_cmp)};
            ecc3_cmp <= ecc3;
            hstate   <= ST_WBREAD_3;
         end
         5'b10000: begin
            spare_space_rd[ecc_bit_pos(spare_space_ecc, spare_space_ecc_cmp)] <=
              !spare_space_rd[ecc_bit_pos(spare_space_ecc, spare_space_ecc_cmp)];
            hstate <= ST_WBREAD_5;
         end
         default: begin
            hstate <= ST_WBREAD_5;
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
      // now finally return read data to user
      ST_WBREAD_5: begin
         wbm_cti_o <= WB_CTI_CLASSIC;
         wbm_bte_o <= WB_BTE_LINEAR;
         wbm_adr_o <= 'h0;
         wbm_sel_o <= 'h0;
         wbm_cyc_o <= 0;
         wbm_stb_o <= 0;
         wbm_we_o  <= 0;
         
         if(wbs_stb_i) begin
            // make sure we don't read further into BRAM than we can
            if({page_offset, wbs_adr_i[`FCOLUMNS-1:2]} >= (`FCOLUMNS_AND_SPARE/4))
               wbs_dat_o <= 'h0;
            else
               wbs_dat_o <= b_dout;
               
            // save the last address read from, so next address is + 1
            addr      <= {page_offset, wbs_adr_i[`FCOLUMNS-1:2]} + 'h2;
            wbs_ack_o <= 1;
            paged     <= newpaged;
            
            if(wbs_cti_i != WB_CTI_INCR_BURST)
               hstate <= ST_IDLE;
            else
               hstate <= ST_WBREAD_6;
         end else
            wbs_ack_o <= 0;
      end
      // if we're bursting, preemptively increment address
      ST_WBREAD_6: begin
         if(wbs_stb_i) begin
            if(b_addr >= (`FCOLUMNS_AND_SPARE/4))
               wbs_dat_o <= 'h0;
            else
               wbs_dat_o <= b_dout;
               
            wbs_ack_o <= (wbs_cti_i == WB_CTI_EOB) ? 0 : 1;
            addr      <= addr + 'h1;
         end else
            wbs_ack_o <= 0;
         
         if(!wbs_cyc_i || (wbs_cti_i == WB_CTI_EOB))
            hstate <= ST_IDLE;
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
   .b_din  ( b_din ),
   .b_dout ( b_dout )
);

always @(*) begin
   if(wb_rst) begin
      a_wr   <= 0;
      a_addr <= 'h0;
      a_din  <= 'h0;
      b_addr <= 'h0;
   end else begin
      if((hstate == ST_WBREAD_0) & wbm_ack_i & (wbm_adr_o[`FCOLUMNS:2] < (`FCOLUMNS_AND_SPARE/4))) begin
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
   
      if(hstate == ST_WBREAD) begin
         b_addr <= {page_offset, wbs_adr_i[`FCOLUMNS-1:2]};
      end else if(hstate == ST_WBREAD_3) begin
         b_addr <= fix_bit[13:5];
      end else if(hstate == ST_WBREAD_2) begin
         b_addr <= {page_offset, wbs_adr_i[`FCOLUMNS-1:2]};
      end else if((hstate == ST_WBREAD_5) & wbs_stb_i) begin
         b_addr <= {page_offset, wbs_adr_i[`FCOLUMNS-1:2]} + 'h1;
      end else if((hstate == ST_WBREAD_6) & wbs_stb_i) begin
         b_addr <= addr;
      end else
         b_addr <= 'h0;
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
