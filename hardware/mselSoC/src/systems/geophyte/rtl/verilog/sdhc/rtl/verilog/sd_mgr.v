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
module sd_mgr (
   input  wire        clk_50,
   input  wire        reset_n,

   input  wire        bram_rd_sd_clk,
   input  wire [6:0]  bram_rd_sd_addr,
   input  wire        bram_rd_sd_wren,
   input  wire [31:0] bram_rd_sd_data,
   output wire [31:0] bram_rd_sd_q,

   input  wire        bram_rd_ext_clk,
   input  wire [6:0]  bram_rd_ext_addr,
   input  wire        bram_rd_ext_wren,
   input  wire [31:0] bram_rd_ext_data,
   output wire [31:0] bram_rd_ext_q,

   input  wire        bram_wr_sd_clk,
   input  wire [6:0]  bram_wr_sd_addr,
   input  wire        bram_wr_sd_wren,
   input  wire [31:0] bram_wr_sd_data,
   output wire [31:0] bram_wr_sd_q,

   input  wire        bram_wr_ext_clk,
   input  wire [6:0]  bram_wr_ext_addr,
   input  wire        bram_wr_ext_wren,
   input  wire [31:0] bram_wr_ext_data,
   output wire [31:0] bram_wr_ext_q,

   input  wire        link_read_act,
   output reg         link_read_go,
   input  wire [31:0] link_read_addr,
   input  wire [31:0] link_read_num,
   input  wire        link_read_stop,

   input  wire        link_write_act,
   output reg         link_write_done,
   input  wire [31:0] link_write_addr,
   input  wire [31:0] link_write_num,

   output reg         ext_read_act,
   input  wire        ext_read_go,
   output reg  [31:0] ext_read_addr,
   output reg  [31:0] ext_read_num,
   output reg         ext_read_stop,

   output reg         ext_write_act,
   input  wire        ext_write_done,
   output reg  [31:0] ext_write_addr,
   output reg  [31:0] ext_write_num
);

reg [31:0] link_read_addr_latch;
reg [15:0] dc;
   
reg [4:0]  state;
parameter [4:0] ST_RESET    = 'd0,
                ST_IDLE     = 'd4,
                ST_BREAD_0  = 'd5,
                ST_BREAD_1  = 'd6,
                ST_BREAD_2  = 'd7,
                ST_BREAD_3  = 'd8,
                ST_BREAD_4  = 'd9,
                ST_BREAD_5  = 'd10,
                ST_BWRITE_0 = 'd13,
                ST_BWRITE_1 = 'd14,
                ST_BWRITE_2 = 'd15,
                ST_BWRITE_3 = 'd16,
                ST_BWRITE_4 = 'd17,
                ST_BWRITE_5 = 'd18,
                ST_LAST     = 'd31;
               
wire reset_s;
wire ext_read_go_s, ext_read_go_r;
wire ext_write_done_s, ext_write_done_r;
synch_3 a(reset_n, reset_s, clk_50);
synch_3 b(ext_read_go, ext_read_go_s, clk_50, ext_read_go_r);
synch_3 c(ext_write_done, ext_write_done_s, clk_50, ext_write_done_r);

always @(posedge clk_50) begin

   // free running counter
   dc <= dc + 1'b1;
   
   link_write_done <= 0;
   
   case(state)
   ST_RESET: begin
      ext_read_act <= 0;
      ext_read_stop <= 0;
      
      ext_write_act <= 0;
      
      sel_rd_sd <= 0;
      sel_rd_ext <= 0;
      
      buf_rd_a_full <= 0;
      buf_rd_b_full <= 0;
      
      sel_wr_sd <= 0;
      sel_wr_ext <= 0;
      
      link_read_go <= 0;
      
      state <= ST_IDLE;
   end
   ST_IDLE: begin
      dc <= 0;
      if(link_read_act) begin
         ext_read_addr <= link_read_addr;
         link_read_addr_latch <= link_read_addr;
         //ext_read_num <= link_read_num;
         ext_read_stop <= 0;
         state <= ST_BREAD_0;
      end else
      if(link_write_act) begin
         ext_write_addr <= link_write_addr;
         //ext_write_num <= link_write_num;
         state <= ST_BWRITE_0;
      end
   end
   ST_BREAD_0: begin
      // 1. check if desired block was already cached in the next read slot
      // 2. if it is, immediately let the SD have it, otherwise stall until it's ready
      // 3. then start another read to preload the next block in the sequence
      if( (sel_rd_sd ? {buf_rd_b_block, buf_rd_b_full} : 
                  {buf_rd_a_block, buf_rd_a_full}) == {ext_read_addr, 1'b1} ) begin
         // selected buffer has the data already
         link_read_go <= 1;
         // swap wishbone pointer to the next slot
         sel_rd_ext <= ~sel_rd_ext;
         state <= ST_BREAD_3;
      end else begin
         // not in next buffer
         state <= ST_BREAD_1;
      end
      dc <= 0;
   end
   ST_BREAD_1: begin
      // load block if one is not cached
      case(sel_rd_ext)
      0: buf_rd_a_block <= link_read_addr_latch;
      1: buf_rd_b_block <= link_read_addr_latch;
      endcase

      // signal to external (wishbone) to start a block read
      ext_read_act <= 1;
      if(ext_read_go_r ) begin
         ext_read_act <= 0;
         ext_read_stop <= 1;
         // tell link to start pushing data to SD
         link_read_go <= 1;
         case(sel_rd_ext)
         0: buf_rd_a_full <= 1;
         1: buf_rd_b_full <= 1;
         endcase
         // swap wishbone pointer to the next slot
         sel_rd_ext <= ~sel_rd_ext;
         state <= ST_BREAD_2;
      end   
   end
   ST_BREAD_2: begin
      if(~ext_read_go) begin
         ext_read_stop <= 0;
         state <= ST_BREAD_3;
      end
      dc <= 0;
   end
   ST_BREAD_3: begin
      // preload next block
      case(sel_rd_ext)
      0: buf_rd_a_block <= link_read_addr_latch + 1;
      1: buf_rd_b_block <= link_read_addr_latch + 1;
      endcase
      
      // signal to external (wishbone) to start a block read
      ext_read_addr <= link_read_addr_latch + 1;
      ext_read_act <= 1;
      if(ext_read_go_r) begin
         // data is valid
         ext_read_act <= 0;
         ext_read_stop <= 1;
         case(sel_rd_ext)
         0: buf_rd_a_full <= 1;
         1: buf_rd_b_full <= 1;
         endcase
         state <= ST_BREAD_4;
      end   
   end
   ST_BREAD_4: begin
      if(~ext_read_go) begin
         ext_read_stop <= 0;
         state <= ST_BREAD_5;
      end
   end
   ST_BREAD_5: begin
      // wait for SD to stop sending
      if(link_read_stop) begin
         // finished, or interrupted
         case(sel_rd_sd)
         0: buf_rd_a_full <= 0;
         1: buf_rd_b_full <= 0;
         endcase
         link_read_go <= 0;
         state <= ST_IDLE;
         // swap buffers
         sel_rd_sd <= ~sel_rd_sd;
      end
   end
   
   ST_BWRITE_0: begin
      // 1. immediately flush loaded data to wishbone ext
      // 2. accept new data from SD after swapping buffers
      // 3. stall done until WB write is finished
      
      // signal to external (wishbone) to start a block write
      ext_write_act <= 1;
      if(~ext_write_done) begin
         // wait for done to fall to be able to detect the rising/risen edge later
         sel_wr_sd <= ~sel_wr_sd;
         state <= ST_BWRITE_1;
      end
   end 
   ST_BWRITE_1: begin
      // tell SD we can accept another block
      link_write_done <= 1;
      // SD may dump another load, we won't service it until the FSM returns to idle and notices
      if(~link_write_act) begin
         state <= ST_BWRITE_2;
      end else
      if(ext_write_done) begin 
         // external has written the buffer, tell link
         sel_wr_ext <= ~sel_wr_ext;
         ext_write_act <= 0;
         state <= ST_BWRITE_3;
      end   
   end
   ST_BWRITE_2: begin
      if(ext_write_done) begin 
         // external has written the buffer, tell link
         sel_wr_ext <= ~sel_wr_ext;
         ext_write_act <= 0;
         state <= ST_BWRITE_3;
      end   
   end
   ST_BWRITE_3: begin
      // invalidate read cache 
      buf_rd_a_full <= 0;
      buf_rd_b_full <= 0;
      state <= ST_IDLE;
   end
   endcase
   
   if(~reset_s) begin
      state <= ST_RESET;
   end    
end

reg  sel_rd_sd;
reg  sel_rd_ext;
   
reg  buf_rd_a_full;
reg  buf_rd_b_full;
   
reg  [31:0] buf_rd_a_block;
reg  [31:0] buf_rd_b_block;
   
// address two sector buffers inside the bram
wire [7:0]  bram_rd_sd_addr_sel = (sel_rd_sd ? bram_rd_sd_addr + 8'd128 : bram_rd_sd_addr) /* synthesis keep */;
wire [7:0]  bram_rd_ext_addr_sel = (sel_rd_ext ? bram_rd_ext_addr + 8'd128 : bram_rd_ext_addr) /* synthesis keep */;
   
// 512 byte bram (2 x 128 x 32bit word)
sd_bram_block_dp #(32, 8) isdb1 (
   .a_clk  ( bram_rd_sd_clk ),
   .a_wr   ( bram_rd_sd_wren ),
   .a_addr ( bram_rd_sd_addr_sel ),
   .a_din  ( bram_rd_sd_data ),
   .a_dout ( bram_rd_sd_q ),
   
   .b_clk  ( bram_rd_ext_clk ),
   .b_wr   ( bram_rd_ext_wren ),
   .b_addr ( bram_rd_ext_addr_sel ),
   .b_din  ( bram_rd_ext_data ),
   .b_dout ( bram_rd_ext_q )
);

reg  sel_wr_sd;
reg  sel_wr_ext;
   
// address two sector buffers inside the bram
wire [7:0] bram_wr_sd_addr_sel = (sel_wr_sd ? bram_wr_sd_addr + 8'd128 : bram_wr_sd_addr) /* synthesis keep */;
wire [7:0] bram_wr_ext_addr_sel = (sel_wr_ext ? bram_wr_ext_addr + 8'd128 : bram_wr_ext_addr) /* synthesis keep */;
   
// 512 byte bram (2 x 128 x 32bit word)
sd_bram_block_dp #(32, 8) isdb2 (
   .a_clk  ( bram_wr_sd_clk ),
   .a_wr   ( bram_wr_sd_wren ),
   .a_addr ( bram_wr_sd_addr_sel ),
   .a_din  ( bram_wr_sd_data ),
   .a_dout ( bram_wr_sd_q ),
   
   .b_clk  ( bram_wr_ext_clk ),
   .b_wr   ( bram_wr_ext_wren ),
   .b_addr ( bram_wr_ext_addr_sel ),
   .b_din  ( bram_wr_ext_data ),
   .b_dout ( bram_wr_ext_q )
);

endmodule
