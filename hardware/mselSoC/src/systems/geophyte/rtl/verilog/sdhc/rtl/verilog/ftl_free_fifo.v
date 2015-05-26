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
module ftl_free_fifo (
   input  wire       clk_50,
   input  wire       reset_n,

   input  wire [9:0] fifo_data_in,
   input  wire       fifo_data_wr,
   output wire [9:0] fifo_data_out,
   output reg        fifo_data_valid,
   input  wire       fifo_data_rd,
   output wire       fifo_empty
);

`include "ftl_const.vh"

reg        fifo_data_wr_1;
reg        fifo_data_rd_1;
   
//
// storage for the free block fifo
//
reg  [9:0] bram_fifo_wraddr;
reg  [9:0] bram_fifo_rdaddr;
reg  [9:0] bram_fifo_data;
reg        bram_fifo_wren;
wire [9:0] bram_fifo_q;
   
reg  [3:0] state;
parameter [3:0] ST_RESET = 'd0,
                ST_IDLE  = 'd1,   
                ST_UNK   = 'd2;

assign fifo_data_out = bram_fifo_q;
assign fifo_empty    = (bram_fifo_rdaddr == bram_fifo_wraddr);
   
always @(posedge clk_50) begin
   bram_fifo_wren <= 0;

   fifo_data_wr_1 <= fifo_data_wr;
   fifo_data_rd_1 <= fifo_data_rd;

   case(state)
   ST_RESET: begin
      bram_fifo_rdaddr <= 0; // 1020? why
      bram_fifo_wraddr <= 0;
      fifo_data_valid  <= 0;
      state            <= ST_IDLE;
   end
   ST_IDLE: begin
      if(fifo_data_wr & ~fifo_data_wr_1) begin
         // new write
         bram_fifo_data <= fifo_data_in;
         bram_fifo_wren <= 1'b1;
      end else if(fifo_data_rd & ~fifo_data_rd_1) begin
         // new read
         if(!fifo_empty) begin
            fifo_data_valid <= 1;
         end
      end

      if(~fifo_data_wr & fifo_data_wr_1) begin
         bram_fifo_wraddr <= bram_fifo_wraddr + 1'b1;
      end
      if(~fifo_data_rd & fifo_data_rd_1) begin
         fifo_data_valid <= 0;
         if(fifo_data_valid) begin
            bram_fifo_rdaddr <= bram_fifo_rdaddr + 1'b1;
         end
      end

   end
   endcase
   
   if(~reset_n) begin
      state <= ST_RESET;
   end
end
   
ftl_bram_block_dp #(10, 10) iffbram (
   .a_clk  ( clk_50 ),
   .a_wr   ( bram_fifo_wren ),
   .a_addr ( bram_fifo_wraddr ),
   .a_din  ( bram_fifo_data ),
   .a_dout (  ),
   
   .b_clk  ( clk_50 ),
   .b_wr   ( 1'b0 ),
   .b_addr ( bram_fifo_rdaddr ),
   .b_din  ( 'h0 ),
   .b_dout ( bram_fifo_q )
);

endmodule
