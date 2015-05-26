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
module ftl_wbs (
   input  wire        clk_50,
   input  wire        reset_n,

   // slave wishbone interface (from SDHC)
   input  wire        wbs_clk_i,
   input  wire [31:0] wbs_adr_i,
   output wire [31:0] wbs_dat_o,
   input  wire [31:0] wbs_dat_i,
   input  wire [3:0]  wbs_sel_i,
   input  wire        wbs_cyc_i,
   input  wire        wbs_stb_i,
   input  wire        wbs_we_i,
   output wire        wbs_ack_o,

   // port to cached block ram
   output wire        bram_wbs_clk,
   output wire [15:0] bram_wbs_addr,
   output reg         bram_wbs_wren,
   output reg  [31:0] bram_wbs_data,
   input  wire [31:0] bram_wbs_q,

   input  wire        logical_init_done,
   output reg         wb_read,
   output reg         wb_write,
   output reg  [9:0]  wb_block,
   input  wire        wb_ack,
   input  wire        wb_done
);

`include "ftl_const.vh"

assign     bram_wbs_clk = wbs_clk_i;
assign     bram_wbs_addr = (wbs_adr_i % NAND_BLOCK_SIZE) / 4;
assign     wbs_ack_o = wbs_ack & wbs_stb_i;
assign     wbs_dat_o = bram_wbs_q;
reg        wbs_ack;
reg        wbs_stb_i_1;
   
reg        wb_done_s_1;
   
wire       reset_s;
wire       logical_init_done_s;
wire       wb_ack_s;
wire       wb_done_s;

synch_3 a(reset_n, reset_s, wbs_clk_i);
synch_3 b(logical_init_done, logical_init_done_s, wbs_clk_i);
synch_3 c(wb_ack, wb_ack_s, wbs_clk_i);
synch_3 d(wb_done, wb_done_s, wbs_clk_i);

wire [9:0] req_block = wbs_adr_i / NAND_BLOCK_SIZE;
reg  [9:0] req_block_latch;
reg  [9:0] cached_block;
reg        modified;
   
reg  [3:0] state;

wire ftl_valid = wbs_adr_i >= 32'h200000;

parameter [3:0] ST_RESET      = 'd0,
                ST_IDLE       = 'd1,
                ST_READ       = 'd2,
                ST_WRITE      = 'd3,
                ST_IDLE_WAIT  = 'd5,
                ST_READ_DELAY = 'd6;
               
always @(posedge wbs_clk_i) begin
   wbs_ack     <= 0;
   wbs_stb_i_1 <= wbs_stb_i;
   
   wb_done_s_1 <= wb_done_s;
   wb_read     <= 0;
   wb_write    <= 0;
   
   bram_wbs_wren <= 0;
   
   case(state)
   ST_RESET: begin
      cached_block <= -1;
      modified     <= 0;
      if(logical_init_done_s) state <= ST_IDLE;
   end
   ST_IDLE: begin
      // on rising edge of WBS_STB_I
      if(wbs_cyc_i & wbs_stb_i & ~wbs_stb_i_1 & ftl_valid) begin
         if(wbs_we_i) begin
            if((cached_block == req_block) && modified) begin
               // write to bram
               bram_wbs_data <= wbs_dat_i;
               bram_wbs_wren <= 1;
               wbs_ack       <= 1;
            end else begin
               // switch blocks and mark modified
               state <= ST_WRITE;
               req_block_latch <= req_block;
            end
            //wbs_ack <= 1;
         end else begin
            if(cached_block == req_block) begin
               // read from bram
               state <= ST_READ_DELAY;
            end else begin
               // switch blocks and mark unmodified
               state <= ST_READ;
               req_block_latch <= req_block;
            end
         end
      end
   end
   ST_IDLE_WAIT: begin
      wbs_stb_i_1 <= 0; // make idle state fire again
      if(~wb_ack_s) state <= ST_IDLE;
   end
   ST_WRITE: begin
      modified <= 1;
      wb_block <= req_block_latch;
      wb_write <= 1;
      if(wb_done_s & ~wb_done_s_1) begin
         cached_block <= req_block_latch;
         state <= ST_IDLE_WAIT;
      end   
   end
   ST_READ: begin
      modified <= 0;
      wb_block <= req_block_latch;
      wb_read  <= 1;
      if(wb_done_s & ~wb_done_s_1) begin
         cached_block <= req_block_latch;
         state <= ST_IDLE_WAIT;
      end   
   end
   ST_READ_DELAY: begin
      // delay for bram
      wbs_ack <= 1;
      state   <= ST_IDLE;
   end
   endcase
   
   if(~reset_s) begin
      state <= ST_RESET;
   end
end

endmodule
