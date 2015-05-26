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

`include "nandc_def.vh"

//`define TEST_1BIT_ERROR
//`define TEST_2BIT_ERROR

//
// WISHBONE NAND CONTROLLER
//
// Note: this controller is designed to have an wishbone ECC controller placed
//       in-line before it. This controller has a few restrictions for it's
//       direct master:
//
// 1. Reads and Writes to flash MUST start at 0 address in page
// 2. They must always write a full page plus spare with ECC (2048+20 bytes)
// 3. The address space of this controller lets you address the page_offset
//    (bit 11), the higher controller should remap to hide bit 11 and provide
//    access to spare space through other registers.
//
module nandc #(
   // base offset on the wishbone bus that everything is located at
   parameter WB_FLASH_BASEADDR = `WB_FLASH_BASEADDR,

   // the offset to place wishbone registers at on the bus
   parameter WB_REG_BASEADDR   = `WB_REG_BASEADDR,

   // the start block that this nand controller is able to address
   parameter WB_FLASH_S        = `WB_FLASH_S,
    
   // the number of blocks this nand controller is able to address (out of 1024)
   parameter WB_FLASH_N        = `WB_FLASH_N,

   // number of wbs_clk cycles that fit in a tRC cycle of the flash
   parameter TRC      = 2, // 10ns * 2 = 20ns > 12.5ns tRC/2
   parameter TADL     = 4, // 20ns * 4 = 80ns > 70ns tADL
   parameter TWB      = 5, // 20ns * 5 = 100ns > 100ns tWB
   parameter TWHR     = 3, // 20ns * 3 = 60ns == 60ns tWHR 
   parameter ADDR_CYC = 4  // 4 address cycles for this part
) (
   input  wire        wb_clk,    // clock - bus clock
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
   output reg         wbs_ack_o, // ack    - end of a normal bus cycle

   input  wire [7:0]  IO_i, // io     - data input from flash
   output reg  [7:0]  IO_o, // io     - data output to flash
   output reg  [7:0]  IO_t, // io     - data tristate control
   output reg         CLE,  // cle    - command latch enable
   output reg         ALE,  // ale    - address latch enable
   output wire        CE_n, // ce     - chip enable
   output wire        WE_n, // we     - write enable
   output wire        RE_n, // re     - read enable
   output wire        WP_n, // wp     - write protect enable
   input  wire        RB_n  // rb     - read/busy signal from flash
);

`include "nandc_const.vh"

parameter WB_ERASE_BASEADDR  = WB_REG_BASEADDR   + `WB_ERASE_OFF;
parameter WB_STATUS_BASEADDR = WB_REG_BASEADDR   + `WB_STATUS_OFF;
parameter WB_FLASH_HIGHADDR  = WB_FLASH_BASEADDR + (WB_FLASH_N << (`FPAGES + `FCOLUMNS + 1));

reg CE, WE, RE, WP;
assign CE_n = !CE;
assign WE_n = !WE;
assign RE_n = !RE;
assign WP_n = !WP;

reg [2:0] RB_n_;
always @(posedge wb_clk) RB_n_ <= {RB_n_[1:0], RB_n};
wire RB  = !RB_n_[2];

reg [5:0] fstate, n_fstate, fstate_l;

parameter [5:0] ST_RESET      = 'd0,
                ST_IDLE       = 'd1,
                ST_IDLE_0     = 'd2,
                ST_WBWRITE    = 'd3,
                ST_WBWRITE_0  = 'd4,
                ST_WBWRITE_1  = 'd5,
                ST_WBWRITE_2  = 'd6,
                ST_WBWRITE_3  = 'd7,
                ST_WBWRITE_4  = 'd8,
                ST_WBWRITE_5  = 'd9,
                ST_WBWRITE_6  = 'd10,
                ST_WBWRITE_7  = 'd11,
                ST_WBREAD     = 'd12,
                ST_WBREAD_0   = 'd13,
                ST_WBREAD_1   = 'd14,
                ST_WBREAD_2   = 'd15,
                ST_WBREAD_3   = 'd16,
                ST_WBREAD_4   = 'd17,
                ST_WBREAD_5   = 'd18,
                ST_WBERASE    = 'd19,
                ST_WBERASE_0  = 'd20,
                ST_WBERASE_1  = 'd21,
                ST_WBERASE_2  = 'd22,
                ST_WBERASE_3  = 'd23,
                ST_WBSTATUS   = 'd24,
                ST_WBSTATUS_0 = 'd25,
                ST_WBSTATUS_1 = 'd26,
                ST_WBSTATUS_2 = 'd27,
                ST_SYNC_CYCLE = 'd28,
                ST_WE_TOGGLE  = 'd29,
                ST_RE_TOGGLE  = 'd30;

reg        trc;
reg [3:0]  trc_state;
reg [31:0] addr, addr_l;
reg [31:0] data;
reg [3:0]  i;

// read buffer for ECC
reg [7:0] bram [2047:0];

// change state
`define state(now) \
   fstate <= now

// change state and the next state
`define next_state(now, next) \
   n_fstate <= next; \
   fstate   <= now
   
parameter IO_TRI   = 8'hff;
parameter IO_DRIVE = 8'h00;

always @(posedge wb_clk) begin
   if(wb_rst) begin
      IO_o <= 0;
      IO_t <= IO_TRI;
      CLE  <= 0;
      ALE  <= 0;
      CE   <= 0;
      WE   <= 0;
      RE   <= 0;
      WP   <= 0;
      wbs_dat_o <= 0;
      wbs_ack_o <= 0;
      fstate    <= ST_RESET;
      n_fstate  <= ST_RESET;
      fstate_l  <= ST_RESET;
      trc       <= 0;
      trc_state <= 'h0;
      addr      <= 'h0;
      addr_l    <= ~'h0;
      data      <= 0;
      i         <= 0;
   end else begin
      case(fstate)
      ST_RESET: begin
         IO_o <= 0;
         IO_t <= IO_TRI;
         CLE  <= 0;
         ALE  <= 0;
         CE   <= 0;
         WE   <= 0;
         RE   <= 0;
         WP   <= 0;
         fstate    <= ST_IDLE;
         n_fstate  <= ST_IDLE;
         fstate_l  <= ST_IDLE;
         addr      <= 'h0;
         addr_l    <= 'h0;
         i         <= 0;
         wbs_ack_o <= 0;
         wbs_dat_o <= 'h0;
      end
      ST_IDLE: begin
         i         <= 0;
         wbs_ack_o <= 0;
         wbs_dat_o <= 'h0;
         `state(ST_IDLE_0);
      end
      ST_IDLE_0: begin
         if(wbs_cyc_i & wbs_stb_i) begin
            if(wbs_we_i) begin
               case(wbs_adr_i)
               WB_ERASE_BASEADDR: `state(ST_WBERASE);
               default: begin
                  if((wbs_adr_i >= WB_FLASH_BASEADDR) &&
                     (wbs_adr_i <  WB_FLASH_HIGHADDR)) begin
                     `state(ST_WBWRITE);
                  end else begin
                     wbs_ack_o <= 1;
                     `state(ST_IDLE);
                  end
               end
               endcase
            end else begin
               case(wbs_adr_i)
               WB_STATUS_BASEADDR: `state(ST_WBSTATUS);
               default: begin
                  if((wbs_adr_i >= WB_FLASH_BASEADDR) &&
                     (wbs_adr_i <  WB_FLASH_HIGHADDR)) begin
                     `state(ST_WBREAD);
                  end else begin
                     wbs_ack_o <= 1;
                     `state(ST_IDLE);
                  end
               end
               endcase
            end
            addr <= wbs_adr_i;
         end
      end 


      //
      // WRITE
      //
      // first check to see if this is a continuation of an old
      // transaction. otherwise reset and start over.
      ST_WBWRITE: if(wbs_stb_i) begin
         // terminate if we don't support the cycle type
         if(!((wbs_cti_i == WB_CTI_CLASSIC) || (wbs_cti_i == WB_CTI_INCR_BURST)) || (wbs_bte_i != WB_BTE_LINEAR)) begin
            wbs_ack_o <= 1;
            `state(ST_RESET);
         // if we're still in a continuous block/burst then carry on
         end else if((fstate == fstate_l) && (addr == addr_l + 4)) begin
            `next_state(ST_SYNC_CYCLE, ST_WBWRITE_3);
         // otherwise begin transaction
         end else begin
            `next_state(ST_SYNC_CYCLE, ST_WBWRITE_0);
         end
         fstate_l <= fstate;
      end
      // send the prog command
      ST_WBWRITE_0: if(trc) begin
         CE   <= 1;
         CLE  <= 1;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 1;
         IO_o <= FCMD_PROG_0;
         IO_t <= IO_DRIVE;
         i    <= 0;
         `next_state(ST_WE_TOGGLE, ST_WBWRITE_1);
      end
      // send our column and row addresses
      ST_WBWRITE_1: if(trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 1;
         RE   <= 0;
         WE   <= 1;
         IO_o <= addr_cycle(i, addr + (WB_FLASH_S << (`FPAGES + `FCOLUMNS + 1)));
         IO_t <= IO_DRIVE;
         if(i < (ADDR_CYC-1)) begin
            i <= i + 1;
            `next_state(ST_WE_TOGGLE, ST_WBWRITE_1);
         end else begin
            `next_state(ST_WE_TOGGLE, ST_WBWRITE_2);
            i <= 0;
         end
      end
      // set ALE and wait tADL
      ST_WBWRITE_2: if(trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 0;
         IO_o <= 'h0;
         IO_t <= IO_DRIVE;
         if(i < (TADL-1)) begin
            i <= i + 1;
            `state(ST_WBWRITE_2);
         end else begin
            i <= 0;
            `state(ST_WBWRITE_3);
         end
      end
      // begin sending data 8 bits at a time
      ST_WBWRITE_3: if(wbs_stb_i && trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 1;
         IO_o <= data_byte(i, wbs_dat_i);
         IO_t <= IO_DRIVE;
         if(i < 3) begin
            i <= i + 1;
            `next_state(ST_WE_TOGGLE, ST_WBWRITE_3);
         end else begin
            // if we're at the end of the page, then flush
            if(addr_page_end(addr) || (wbs_cti_i == WB_CTI_EOB)) begin
               `next_state(ST_WE_TOGGLE, ST_WBWRITE_5);
            // otherwise ack and resume wishbone bus
            end else begin
               `next_state(ST_WE_TOGGLE, ST_WBWRITE_4);
            end
         end
      end
      // send wishbone ack and then return to idle state to receive next command
      ST_WBWRITE_4: if(wbs_stb_i) begin // if(trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 0;
         IO_o <= 'h0;
         IO_t <= IO_DRIVE;
         wbs_ack_o <= 1;
         addr_l    <= addr;
         
         if(wbs_cti_i == WB_CTI_INCR_BURST) begin
            `state(ST_WBWRITE_7);
         end else begin
            `state(ST_IDLE);
         end
      end
      // send program command
      ST_WBWRITE_5: if(trc) begin
         CE   <= 1;
         CLE  <= 1;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 1;
         IO_o <= FCMD_PROG_1;
         IO_t <= IO_DRIVE;
         i    <= 0;
         `next_state(ST_WE_TOGGLE, ST_WBWRITE_6);
      end
      // wait tWB
      ST_WBWRITE_6: if(wbs_stb_i && trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 0;
         IO_o <= 'h0;
         IO_t <= IO_TRI;
         // make sure to wait TWB before polling RB
         if(i < (TWB-1)) begin
            i <= i + 1;
            `state(ST_WBWRITE_6);
         // when RB signals write is done, then reset
         end else if(!RB) begin
            wbs_ack_o <= 1;
            `state(ST_RESET);
         end
      end
      // handle bursts
      ST_WBWRITE_7: begin
         i         <= 0;
         wbs_ack_o <= 0;
         if(wbs_stb_i) begin
            `state(ST_WBWRITE_3);
         end
      end


      //
      // READ
      //
      // save state and synchronize with tRC
      ST_WBREAD: if(wbs_stb_i) begin
         // terminate if we don't support the cycle type
         if(!((wbs_cti_i == WB_CTI_CLASSIC) || (wbs_cti_i == WB_CTI_INCR_BURST)) || (wbs_bte_i != WB_BTE_LINEAR)) begin
            wbs_ack_o <= 1;
            `state(ST_RESET);
         // if we're still in a continuous block/burst then carry on
         end else if((fstate == fstate_l) && (addr == addr_l + 4)) begin
            `next_state(ST_SYNC_CYCLE, ST_WBREAD_4);
         // otherwise begin transaction
         end else begin
            `next_state(ST_SYNC_CYCLE, ST_WBREAD_0);
         end
         fstate_l <= fstate;
      end
      // send read command to flash
      ST_WBREAD_0: if(trc) begin
         CE   <= 1;
         CLE  <= 1;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 1;
         IO_o <= FCMD_READ_0;
         IO_t <= IO_DRIVE;
         i    <= 0;
         `next_state(ST_WE_TOGGLE, ST_WBREAD_1);
      end
      // send address cycle to flash
      ST_WBREAD_1: if(trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 1;
         RE   <= 0;
         WE   <= 1;
         IO_o <= addr_cycle(i, addr + (WB_FLASH_S << (`FPAGES + `FCOLUMNS + 1)));
         IO_t <= IO_DRIVE;
         if(i < (ADDR_CYC-1)) begin
             i <= i + 1;
            `next_state(ST_WE_TOGGLE, ST_WBREAD_1);
         end else begin
            `next_state(ST_WE_TOGGLE, ST_WBREAD_2);
         end
      end
      // begin read
      ST_WBREAD_2: if(trc) begin
         CE   <= 1;
         CLE  <= 1;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 1;
         IO_o <= FCMD_READ_1;
         IO_t <= IO_DRIVE;
         i    <= 0;
         `next_state(ST_WE_TOGGLE, ST_WBREAD_3);
      end
      // wait tWB, then RB to finish
      ST_WBREAD_3: if(trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 0;
         IO_o <= 'h0;
         IO_t <= IO_TRI;
         if(i < (TWB-1)) begin
            i <= i + 1;
            `state(ST_WBREAD_3);
         end else if(!RB) begin
            i <= 0;
            `state(ST_WBREAD_4);
         end
      end
      // then grab 4 bytes of data and output and ack to wishbone
      ST_WBREAD_4: begin if(trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 0;
         RE   <= 1;
         WE   <= 0;
         IO_o <= 'h0;
         IO_t <= IO_TRI;
         if(i < 3) begin
            i  <= i + 1;
            `next_state(ST_RE_TOGGLE, ST_WBREAD_4);
         end else begin
            `next_state(ST_RE_TOGGLE, ST_WBREAD_5);
         end
      end
      wbs_ack_o <= 0;
      end
      // handle ack and state transition
      ST_WBREAD_5: if(wbs_stb_i) begin
         RE <= 0;
`ifdef TEST_1BIT_ERROR
         wbs_dat_o <= wbs_adr_i[`FCOLUMNS-1:2] == 'h137 ? data ^ 'h8000 : data;
`else
`ifdef TEST_2BIT_ERROR
         wbs_dat_o <= wbs_adr_i[`FCOLUMNS-1:2] == 'h137 ? data ^ 'hc000 : data;
`else
         wbs_dat_o <= data;
`endif
`endif
         wbs_ack_o <= 1;
         addr_l    <= addr;
         if(wbs_cti_i == WB_CTI_INCR_BURST) begin
            i <= 0;
            `state(ST_WBREAD_4);
         end else if(addr_page_end(addr) || (wbs_cti_i == WB_CTI_EOB)) begin
            `state(ST_RESET);
         end else begin
            `state(ST_IDLE);
         end
      end


      //
      // ERASE
      //
      // make sure the address we're erasing is in allowed range
      ST_WBERASE: if(wbs_stb_i) begin
         if((wbs_dat_i[9:0] >= WB_FLASH_S) && (wbs_dat_i[9:0] < WB_FLASH_N)) begin
            addr     <= {wbs_dat_i[9:0], {`FPAGES{1'b0}}, {`FCOLUMNS+1{1'b0}}};
            fstate_l <= fstate;
            `next_state(ST_SYNC_CYCLE, ST_WBERASE_0);
         end else begin
            wbs_ack_o <= 1;
            `state(ST_IDLE);
         end
      end
      // issue erase command
      ST_WBERASE_0: if(trc) begin
         CE   <= 1;
         CLE  <= 1;
         ALE  <= 0;
         WE   <= 1;
         RE   <= 0;
         IO_o <= FCMD_ERASE_0;
         IO_t <= IO_DRIVE;
         i    <= 2;
         `next_state(ST_WE_TOGGLE, ST_WBERASE_1);
      end
      // send address cycles for block
      ST_WBERASE_1: if(trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 1;
         RE   <= 0;
         WE   <= 1;
         IO_o <= addr_cycle(i, addr + (WB_FLASH_S << (`FPAGES + `FCOLUMNS + 1)));
         IO_t <= IO_DRIVE;
         if(i < (ADDR_CYC-1)) begin
            i <= i + 1;
            `next_state(ST_WE_TOGGLE, ST_WBERASE_1);
         end else begin
            `next_state(ST_WE_TOGGLE, ST_WBERASE_2);
            i <= 0;
         end
      end
      // send erase command
      ST_WBERASE_2: if(trc) begin
         CE   <= 1;
         CLE  <= 1;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 1;
         IO_o <= FCMD_ERASE_1;
         IO_t <= IO_DRIVE;
         i    <= 0;
         `next_state(ST_WE_TOGGLE, ST_WBERASE_3);
      end
      // wait for command to finish
      ST_WBERASE_3: if(trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 0;
         IO_t <= IO_TRI;
         if(i < (TWB-1))
            i <= i + 1;
         else if(!RB & wbs_stb_i) begin
            wbs_ack_o <= 1;
            `state(ST_RESET);
         end
      end
      
      
      //
      // READ STATUS
      //
      ST_WBSTATUS: if(wbs_stb_i) begin
         `next_state(ST_SYNC_CYCLE, ST_WBSTATUS_0);
      end
      // issue status command
      ST_WBSTATUS_0: if(trc) begin
         CE   <= 1;
         CLE  <= 1;
         ALE  <= 0;
         RE   <= 0;
         WE   <= 1;
         IO_o <= FCMD_READ_STAT_0;
         IO_t <= IO_DRIVE;
         i    <= 0;
         `next_state(ST_WE_TOGGLE, ST_WBSTATUS_1);
      end
      ST_WBSTATUS_1: if(trc) begin
         CE   <= 1;
         CLE  <= 0;
         ALE  <= 0;
         WE   <= 0;
         IO_t <= IO_TRI;
         if(i < (TWHR - 1)) begin
            RE <= 0;
            i  <= i + 1;
         end else begin
            RE <= 1;
            `next_state(ST_RE_TOGGLE, ST_WBSTATUS_2);
         end
      end
      ST_WBSTATUS_2: if(wbs_stb_i) begin
         wbs_dat_o <= {31'h0, data[24]};
         wbs_ack_o <= 1;
         `state(ST_RESET);
      end


      //
      // COMMON STATE FUNCTIONS
      //
      // synchronize with the trc
      ST_SYNC_CYCLE: if(trc) begin
         `state(n_fstate);
      end
      // wait for trc and then toggle WE low
      ST_WE_TOGGLE: if(trc) begin
         WE <= 0;
         `state(n_fstate);
      end
      // wait for trc and then toggle RE low and save data
      ST_RE_TOGGLE: if(trc) begin
         RE   <= 0;
         data <= {IO_i, data[31:8]};
         `state(n_fstate);
      end
      endcase


      //
      // TRC PULSE
      //
      // create a trc pulse that goes high every TRC bus clock cycles
      if(trc_state >= (TRC-1)) begin
         trc_state <= 0;
         trc       <= 1;
      end else begin
         trc_state <= trc_state + 1;
         trc       <= 0;
      end
   end
end

endmodule

