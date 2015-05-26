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
module ftl_logical (
   input  wire        clk_50,
   input  wire        reset_n,

   output reg         err_invalid_erase_block,
   input  wire        physical_init_done,
   output reg         init_done,

   input  wire        wb_read,
   input  wire        wb_write,
   input  wire [9:0]  wb_block,
   output reg         wb_ack,
   output reg         wb_done,

   output reg         op_page_do,       // assert to start operation
   output reg  [2:0]  op_page_cmd,      // command to perform
   output reg  [15:0] op_page_num,      // nandc page number
   output reg  [15:0] op_page_bram,     // bram page number
   output reg  [41:0] op_page_spare_wr, // data to write to spare
   input  wire [41:0] op_page_spare_rd, // data read from spare
   input  wire        op_page_status,   // status on program/erase
   input  wire        op_page_ack,      // acknowledge op
   input  wire        op_page_done      // complete
);

`include "ftl_const.vh"
   
reg wb_read_1;
reg wb_write_1;
reg op_page_done_1;
   
//
// storage for the bad block remapping table
//
reg  [9:0]  bram_remap_addr;
reg  [42:0] bram_remap_data;
reg         bram_remap_wren;
wire [42:0] bram_remap_q;
   
wire        round_robin_remap_stat    = bram_remap_q[42];
wire [9:0]  round_robin_remap_block   = bram_remap_q[41:32];
wire [31:0] round_robin_remap_count   = bram_remap_q[31:0];
wire [31:0] round_robin_remap_count_1 = round_robin_remap_count + 1'b1;
   
//   
// free block fifo
//
reg  [9:0]  fifo_data_in;
reg         fifo_data_wr;
wire [9:0]  fifo_data_out;
wire        fifo_data_valid;
reg         fifo_data_rd;
wire        fifo_empty;
   

reg         cached_block_modified;
reg  [9:0]  logical_block_in_cache;
reg  [9:0]  old_remapped_block;
   
reg  [9:0]  scan_block;
reg  [9:0]  erase_block;
reg  [9:0]  read_block;
reg         read_block_empty;
reg  [9:0]  write_block;
reg  [5:0]  block_page_idx;
   
wire [9:0]  remap_phys_block = op_page_spare_rd[41:32];
wire [31:0] remap_phys_count = op_page_spare_rd[31:0];
   
reg  [24:0] flush_timeout;
reg  [15:0] dc;

reg  [6:0]  state;        // main overarching structure
reg  [6:0]  state_inner;  // for routines requiring nesting
reg  [6:0]  state_return;   

parameter [6:0] ST_RESET        = 'd0,
                ST_IDLE         = 'd10,
                ST_IDLE_WAIT    = 'd12,
                ST_IDLE_CACHE   = 'd13,
               
                ST_READ_0       = 'd20,
                ST_READ_1       = 'd21,
                ST_READ_2       = 'd22,
                ST_READ_3       = 'd23,
               
                ST_WRITE_0      = 'd30,
                ST_WRITE_1      = 'd31,
                ST_WRITE_2      = 'd32,
                ST_WRITE_3      = 'd33,
               
                ST_FLUSH_0      = 'd40,
                ST_FLUSH_1      = 'd41,
                ST_FLUSH_2      = 'd42,
                ST_FLUSH_3      = 'd43,
                ST_FLUSH_4      = 'd44,
               
                ST_LOADTABLE_0  = 'd50,
                ST_LOADTABLE_1  = 'd51,
                ST_LOADTABLE_2  = 'd52,
                ST_LOADTABLE_3  = 'd53,
               
                ST_LOADTABLE_4  = 'd54,
                ST_LOADTABLE_5  = 'd55,
               
                ST_SCANBLOCKS_0 = 'd60,
                ST_SCANBLOCKS_1 = 'd61,
                ST_SCANBLOCKS_2 = 'd62,
               
                ST_ERASEBLOCK_0 = 'd70,
                ST_ERASEBLOCK_1 = 'd71,
                ST_ERASEBLOCK_2 = 'd72,
               
                ST_READBLOCK_0  = 'd80,
                ST_READBLOCK_1  = 'd81,
                ST_READBLOCK_2  = 'd82,
               
                ST_WRITEBLOCK_0 = 'd90,
                ST_WRITEBLOCK_1 = 'd91,
                ST_WRITEBLOCK_2 = 'd92,
               
                ST_LAST         = 'd127;

wire wb_read_s;
wire wb_write_s;
synch_3 a (wb_read, wb_read_s, clk_50);
synch_3 b (wb_write, wb_write_s, clk_50);

always @(posedge clk_50) begin   
   op_page_done_1  <= op_page_done;
   wb_read_1       <= wb_read_s;
   wb_write_1      <= wb_write_s;

   
   fifo_data_rd    <= 0;
   fifo_data_wr    <= 0;
   bram_remap_wren <= 0;
   
   dc <= dc + 1'b1;
   
   case(state)
   ST_RESET: begin
      op_page_do <= 0;
      init_done  <= 0;
      wb_ack     <= 0;
      wb_done    <= 0;
      bram_remap_addr <= -1;
      
      err_invalid_erase_block <= 0;
      
      cached_block_modified  <= 0;
      logical_block_in_cache <= -1; // since only ~1020 blocks are allowed this means "no cache"
      
      // don't issue commands until FTL_PHYSICAL can detect a rising edge on op_do
      if(physical_init_done) state <= ST_LOADTABLE_0;
   end
   ST_IDLE: begin
      dc        <= 0;
      init_done <= 1;

      flush_timeout <= flush_timeout + 1'b1;
      
      if(wb_read_s & ~wb_read_1) begin
         wb_ack <= 1;
         state  <= ST_READ_0;
      end else if(wb_write_s & ~wb_write_1) begin
         wb_ack <= 1;
         state  <= ST_WRITE_0;
      end else if(flush_timeout[24]) begin
         flush_timeout <= 0;/*
         if(cached_block_modified) begin
            bram_remap_addr <= logical_block_in_cache;
            state_inner <= ST_IDLE_CACHE;
            state <= ST_FLUSH_0;
         end*/
      end
   end
   ST_IDLE_WAIT: begin
      if(~wb_read_s & ~wb_write_s) begin
         wb_ack  <= 0;
         wb_done <= 0;
         state   <= ST_IDLE;
      end
   end
   ST_IDLE_CACHE: begin
      // clear edge detection flags so new events can be serviced
      wb_read_1  <= 0;
      wb_write_1 <= 0;
      cached_block_modified <= 0;
      state      <= ST_IDLE;
   end
   ST_READ_0: begin
      dc <= 0;
      if(wb_block == logical_block_in_cache) begin
         // already in cache! no need
         wb_done <= 1;
         state   <= ST_IDLE_WAIT;
         $display("FTL_LOGICAL: read: block %d already in cache, exiting", wb_block);
      end else begin
         if(cached_block_modified) begin
            // flush existing bram data out to flash
            $display("FTL_LOGICAL: read: must flush existing block %d data first before reading new block %d", logical_block_in_cache, wb_block);
            // start another lookup
            bram_remap_addr <= logical_block_in_cache;
            state_inner     <= ST_READ_1;
            state           <= ST_FLUSH_0;
         end else begin
            // not dirty, don't bother
            $display("FTL_LOGICAL: read: block %d, remaps to %d, don't need to flush existing data", wb_block, round_robin_remap_block);
            state <= ST_READ_1;
         end
      end
   end
   ST_READ_1: begin
      dc    <= 0;
      // look up remap for desired block
      bram_remap_addr <= wb_block;
      state <= ST_READ_2;
   end
   ST_READ_2: begin
      if(dc == 2) begin
         // if this block is free, just fill the block with 00's
         read_block_empty <= (round_robin_remap_stat == 0);
         read_block   <= round_robin_remap_block;
         state        <= ST_READBLOCK_0;
         state_return <= ST_READ_3;
      end
   end
   ST_READ_3: begin
      cached_block_modified  <= 0;
      logical_block_in_cache <= wb_block;
      wb_done <= 1;
      state   <= ST_IDLE_WAIT;
   end

   ST_WRITE_0: begin
      dc <= 0;
      if(wb_block == logical_block_in_cache) begin
         // already in cache, don't need to load it
         $display("FTL_LOGICAL: write: block %d already in cache, exiting", wb_block);
         cached_block_modified <= 1; // assume that WB master will end up writing to the buffer
         wb_done <= 1;
         state   <= ST_IDLE_WAIT;
      end else begin
         if(cached_block_modified) begin
            $display("FTL_LOGICAL: write: block %d -- cached block %d was modified, flushing first", wb_block, logical_block_in_cache);
            bram_remap_addr <= logical_block_in_cache;
            state       <= ST_FLUSH_0; 
            state_inner <= ST_WRITE_1;
         end else begin
            $display("FTL_LOGICAL: write: block %d -- cached block %d was was unmodified, doesn't need to be flushed", wb_block, logical_block_in_cache);
            state <= ST_WRITE_1;         
         end
      end
   end
   ST_WRITE_1: begin
      dc    <= 0;
      // look up remapping of desired block
      bram_remap_addr <= wb_block;
      state <= ST_WRITE_2;
   end
   ST_WRITE_2: begin
      if(dc == 2) begin
         read_block_empty <= (round_robin_remap_stat == 0);
         if(round_robin_remap_stat == 1) begin
            // if block is valid, read it into the buffer
            $display("FTL_LOGICAL: write: for the written block %d, first reading in block %d", wb_block, round_robin_remap_block);
            read_block   <= round_robin_remap_block;
            state        <= ST_READBLOCK_0; 
            state_return <= ST_WRITE_3;
         end else begin
            $display("FTL_LOGICAL: write: empty, no need to read in the block first");
            state        <= ST_WRITE_3;
            // for debugging's sake we will clear the buffer or the host may choke on garbage
            read_block   <= round_robin_remap_block;
            state        <= ST_READBLOCK_0; 
            state_return <= ST_WRITE_3;
         end
      end
   end
   ST_WRITE_3: begin
      logical_block_in_cache <= wb_block;
      cached_block_modified  <= 1; // assume that WB master will end up writing to the buffer
      wb_done <= 1;
      state   <= ST_IDLE_WAIT;
   end

   //
   // ST_FLUSH: saves current block and puts the old one (if valid) back into the fifo
   //
   ST_FLUSH_0: begin
      if(dc == 2) begin
         if(round_robin_remap_stat == 0) begin
            // FREE, don't bother post-erase
            old_remapped_block <= -1;
         end else begin
            old_remapped_block <= round_robin_remap_block;
         end
         state <= ST_FLUSH_1;
      end
   end
   ST_FLUSH_1: begin
      fifo_data_rd <= 1;
      if(fifo_data_valid) begin
         // update remap table with new block from fifo
         // set entry to VALID
         bram_remap_data <= {1'b1, fifo_data_out, round_robin_remap_count_1};
         bram_remap_wren <= 1;
         // set metadata for current block that'll be written out
         op_page_spare_wr <= {logical_block_in_cache, round_robin_remap_count_1};
         
         write_block  <= fifo_data_out;
         state        <= ST_WRITEBLOCK_0;
         state_return <= ST_FLUSH_2;
      end
   end
   ST_FLUSH_2: begin
      // check if the write had any errors
      if(op_page_status) begin
         // repeat the write, grabbing a new block from fifo (TODO: will this run out?)
         state <= ST_FLUSH_1;
      end else begin
         if(old_remapped_block != 1023) begin // is != -1 ?
            // erase old block
            erase_block  <= old_remapped_block;
            state        <= ST_ERASEBLOCK_0;
            state_return <= ST_FLUSH_3;
            // add to available fifo
            fifo_data_in <= old_remapped_block;
            fifo_data_wr <= 1;
         end else state <= ST_FLUSH_3;
      end
   end
   ST_FLUSH_3: begin
      // return to read/write states
      dc    <= 0;
      state <= state_inner;
   end
   
   //
   // ST_LOADTABLE: read spare data in each block to build remap table and find free blocks
   //
   ST_LOADTABLE_0: begin
      // clear all entries
      bram_remap_addr <= bram_remap_addr + 1'b1;
      bram_remap_data <= 0;
      bram_remap_wren <= 1;
      if(bram_remap_addr == NAND_ALLOWED_LOG_BLOCKS) begin
         bram_remap_addr <= 0;
         scan_block <= 0;
         state      <= ST_LOADTABLE_1;
      end
   end
   ST_LOADTABLE_1: begin
      op_page_do  <= 1;
      op_page_cmd <= OP_PAGE_POKE;
      op_page_num <= scan_block * NAND_PAGE_PER_BLOCK;
      if(op_page_done & ~op_page_done_1) begin
         op_page_do <= 0;
         dc         <= 0;
         bram_remap_addr <= remap_phys_block; // load block stored in spare data (will be F's for erased block)
         state      <= ST_LOADTABLE_2;
      end
   end
   ST_LOADTABLE_2: begin
      if(dc == 2) begin
         // unless otherwise, proceed to next block after this
         state <= ST_LOADTABLE_3;
         // is this block empty?
         if(op_page_spare_rd[41:0] == 42'h3FFFFFFFFFF) begin
            fifo_data_in <= scan_block;
            fifo_data_wr <= 1;
            $display("FTL_LOGICAL: scanning logical block %d: empty, adding to fifo", scan_block);
         end else begin
            // if the block is free, then assign it
            if(round_robin_remap_stat == 0) begin // FREE
               bram_remap_data <= {1'b1, scan_block, remap_phys_count};
               bram_remap_wren <= 1;
               $display("FTL_LOGICAL: scanning logical block %d: assigning as remapped %d", scan_block, bram_remap_addr);
            end else   
            // if we've already recorded this block and it has a lower count, we should free it
            if(round_robin_remap_count < remap_phys_count) begin 
               bram_remap_data <= {round_robin_remap_stat, scan_block, remap_phys_count};
               bram_remap_wren <= 1;
               erase_block     <= round_robin_remap_block;
               state        <= ST_ERASEBLOCK_0;
               state_return <= ST_LOADTABLE_3;
               $display("FTL_LOGICAL: scanning logical block %d: already recorded, freeing", scan_block);
            end else
            // otherwise, the current block is the older copy, so we should free it
            begin
               erase_block  <= scan_block;
               state        <= ST_ERASEBLOCK_0;
               state_return <= ST_LOADTABLE_3;
               
               $display("FTL_LOGICAL: scanning logical block %d: older copy, erasing", scan_block);
            end
         end
      end
   end
   ST_LOADTABLE_3: begin
      // increment scan_block
      if(!op_page_ack) begin
         state      <= ST_LOADTABLE_1;
         scan_block <= scan_block + 1'b1;
         if(scan_block == NAND_ALLOWED_LOG_BLOCKS-1) begin
            // last one! exit
            //init_done <= 1;
            state <= ST_IDLE_CACHE;
            //state <= ST_LOADTABLE_4;
         end
      end
   end
   /*
   ST_LOADTABLE_4: begin
      state <= ST_LOADTABLE_5;
      fifo_data_rd <= 1;
   end
   ST_LOADTABLE_5: begin
      state <= ST_LOADTABLE_4;
      if(fifo_empty) state <= ST_IDLE;
   end
   */
   
   //
   // ST_ERASEBLOCK: erase a block, if it becomes a bad block, keep retrying
   //
   ST_ERASEBLOCK_0: begin
      if(~op_page_ack) begin
         // open to accept new commands
         if(erase_block > NAND_ALLOWED_LOG_BLOCKS) 
            err_invalid_erase_block <= 1; 
         else
            state <= ST_ERASEBLOCK_1; 
            //state <= state_return;
      end
   end
   ST_ERASEBLOCK_1: begin
      op_page_do  <= 1;
      op_page_cmd <= OP_PAGE_BLOCKERASE;
      op_page_num <= erase_block * NAND_PAGE_PER_BLOCK;
      if(op_page_done & ~op_page_done_1) begin
         op_page_do <= 0;
         state      <= ST_ERASEBLOCK_2;
         // but wait, there's error!
         if(op_page_status) begin
            // retry! assume FTL_PHYSICAL is going to update the bad block table
            state <= ST_ERASEBLOCK_0;
         end
      end
   end
   ST_ERASEBLOCK_2: begin
       state <= state_return;
   end
   
   //
   // ST_READBLOCK: read a block (64 pages)
   //
   ST_READBLOCK_0: begin
      if(~op_page_ack) begin
         // open to accept new commands
         block_page_idx <= 0;
         state <= ST_READBLOCK_1;
      end
   end
   ST_READBLOCK_1: begin
      op_page_do  <= 1;
      // if this block was empty, do not read in garbage/FF data from real device
      // instead just tell FTL_PHYSICAL to fill the page with 00's
      op_page_cmd <= read_block_empty ? OP_PAGE_READ_EMPTY : OP_PAGE_READ;
      op_page_num <= read_block * NAND_PAGE_PER_BLOCK + block_page_idx;
      op_page_bram <= block_page_idx;
      if(op_page_done & ~op_page_done_1) begin
         op_page_do <= 0;
         state      <= ST_READBLOCK_2;
      end
   end
   ST_READBLOCK_2: begin   
      if(~op_page_ack) begin
         state          <= ST_READBLOCK_1;
         block_page_idx <= block_page_idx + 1'b1;
         if(block_page_idx == 63) begin
            $display("FTL_LOGICAL: read block %d, empty=%d", read_block, read_block_empty);
            state <= state_return;
         end
      end
   end
   
   //
   // ST_WRITEBLOCK: write a block (64 pages), abort if the block turned bad
   //
   ST_WRITEBLOCK_0: begin
      if(~op_page_ack) begin
         // open to accept new commands
         block_page_idx <= 0;
         state          <= ST_WRITEBLOCK_1;
      end
   end
   ST_WRITEBLOCK_1: begin
      op_page_do   <= 1;
      op_page_cmd  <= OP_PAGE_WRITE;
      op_page_num  <= write_block * NAND_PAGE_PER_BLOCK + block_page_idx;
      op_page_bram <= block_page_idx;
      if(op_page_done & ~op_page_done_1) begin
         op_page_do <= 0;
         state      <= ST_WRITEBLOCK_2;
         // but wait, there's error!
         if(op_page_status) begin
            // can't do anything here, up to the caller to handle it
            $display("FTL_LOGICAL: write block %d failed", write_block);
            state <= state_return;
         end
      end
   end
   ST_WRITEBLOCK_2: begin
      if(~op_page_ack) begin
         state          <= ST_WRITEBLOCK_1;
         block_page_idx <= block_page_idx + 1'b1;
         if(block_page_idx == 63) begin
            $display("FTL_LOGICAL: wrote block %d", write_block);
            state <= state_return;
         end
      end
   end
   
   endcase
   
   if(~reset_n) begin
      state <= ST_RESET;
   end    
end


ftl_free_fifo ilfifo(
   .clk_50          ( clk_50 ),
   .reset_n         ( reset_n ),

   .fifo_data_in    ( fifo_data_in ),
   .fifo_data_wr    ( fifo_data_wr ),
   .fifo_data_out   ( fifo_data_out ),
   .fifo_data_valid ( fifo_data_valid ),
   .fifo_data_rd    ( fifo_data_rd ),
   .fifo_empty      ( fifo_empty )
);

// remap table 32 + 10 + 1
ftl_bram_block_dp  #(43, 10) ilrbram (
   .a_clk           ( clk_50 ),
   .a_wr            ( bram_remap_wren ),
   .a_addr          ( bram_remap_addr ),
   .a_din           ( bram_remap_data ),
   .a_dout          ( bram_remap_q ),
   .b_clk           ( clk_50 ),
   .b_wr            ( 1'b0 ),
   .b_addr          ( 'h0 ),
   .b_din           ( 'h0 ),
   .b_dout          (  )
);

endmodule
