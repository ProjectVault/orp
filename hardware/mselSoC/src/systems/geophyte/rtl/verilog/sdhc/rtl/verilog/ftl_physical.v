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
module ftl_physical (
   input  wire        clk_50,
   input  wire        reset_n,

   output reg         init_done,
   output reg  [9:0]  dbg_num_valid_blocks,
   output reg         dbg_rebuilt_badblock,
   output reg         dbg_remapped_runtime,

   output wire [9:0]  dbg_logical_block,
   output wire [9:0]  dbg_physical_block, 
   output reg         err_out_of_extras,

   input  wire        op_page_do,       // assert to start operation
   input  wire [2:0]  op_page_cmd,      // command to perform
   input  wire [15:0] op_page_num,      // logical page
   input  wire [15:0] op_page_bram,     // bram page number dest
   input  wire [41:0] op_page_spare_wr, // data to write to spare
   output reg  [41:0] op_page_spare_rd, // data read from spare
   output reg         op_page_status,   // status on program/erase
   output reg         op_page_ack,      // acknowledge op
   output reg         op_page_done,     // complete

   // interface to master block bram
   output reg  [15:0] bram_page_addr,
   output reg         bram_page_wren,
   output reg  [31:0] bram_page_data,
   input  wire [31:0] bram_page_q,

   // master wishbone interface (to NANDC)
   output wire        wbm_clk_o,
   output reg  [2:0]  wbm_cti_o,   // type   - cycle type identifier
   output wire [1:0]  wbm_bte_o,   // exten  - burst type extension
   output reg  [31:0] wbm_adr_o,
   input  wire [31:0] wbm_dat_i,
   output wire [31:0] wbm_dat_o,
   output wire [3:0]  wbm_sel_o,
   output reg         wbm_cyc_o,
   output reg         wbm_stb_o,
   output reg         wbm_we_o,
   input  wire        wbm_ack_i

   ,
   output wire        clk_slow
);

`include "ftl_const.vh"

reg [15:0] cnt;
always @(posedge clk_50) cnt <= cnt + 1'b1;
assign clk_slow = cnt[7];

assign wbm_clk_o = clk_50;

assign wbm_bte_o = 2'b00;
assign wbm_sel_o = 4'b1111;
reg [31:0] wbm_dat_out;
reg        wbm_ack_i_last;
   
reg        op_page_do_1;
   
//
// storage for the bad block remapping table
//
reg  [9:0]  bram_badmap_addr;
wire [9:0]  bram_badmap_addr_1 = bram_badmap_addr + 1'b1;
reg  [15:0] bram_badmap_data;
reg         bram_badmap_wren;
wire [15:0] bram_badmap_q;
reg  [31:0] bram_badmap_data_32;

//   
// modelsim refuses to notice these unless they're above the first use
//
reg  [9:0]  scan_block;
reg  [7:0]  scan_page;
reg         scan_blockgood;
   
reg  [9:0]  last_good_block;
reg         remapping_a_block;
   
reg  [15:0] physical_page;
   
reg  [15:0] bytes_done;
   
reg  [15:0] dc;

reg  [6:0]  state;

parameter [6:0] ST_RESET        = 'd0,
                ST_IDLE         = 'd10,
                ST_IDLE_REQ     = 'd11,
                ST_IDLE_WAIT    = 'd12,
                ST_READ_0       = 'd20,
                ST_READ_1       = 'd21,
                ST_READ_2       = 'd22,
                ST_EMPTY_0      = 'd25,
                ST_WRITE_0      = 'd26,
                ST_POKE_0       = 'd27,
                ST_ERASE_0      = 'd28,
                ST_STATUS_RD_0  = 'd29,
                ST_SPARE_RD_0   = 'd30,
                ST_SPARE_WR_0   = 'd31,
               
                ST_CHECKINIT_0  = 'd48,
                ST_CHECKINIT_1  = 'd49,
               
                ST_LOADTABLE_0  = 'd50,
                ST_LOADTABLE_1  = 'd51,
               
                ST_SCANBLOCKS_0 = 'd60,
                ST_SCANBLOCKS_1 = 'd61,
                ST_SCANBLOCKS_2 = 'd62,
                ST_SCANBLOCKS_3 = 'd63,
               
                ST_FLUSHTABLE_0 = 'd64,
                ST_FLUSHTABLE_1 = 'd65,
                ST_FLUSHTABLE_2 = 'd66,
                ST_FLUSHTABLE_3 = 'd67,
               
                ST_REMAPNEW_0   = 'd70,
                ST_REMAPNEW_1   = 'd71,
                ST_REMAPNEW_2   = 'd72,
                ST_REMAPNEW_3   = 'd73,
               
                ST_ERASEBLOCK1  = 'd80,
               
                ST_LAST         = 'd127;

assign wbm_dat_o = ((state == ST_WRITE_0) ? bram_page_q : 
                    (state == ST_FLUSHTABLE_2 || state == ST_FLUSHTABLE_3) ? bram_badmap_data_32 : 
                     wbm_dat_out);
                        
assign dbg_logical_block  = op_page_num / NAND_PAGE_PER_BLOCK;
assign dbg_physical_block = physical_page / NAND_PAGE_PER_BLOCK;
            
always @(posedge clk_50) begin   
   op_page_do_1 <= op_page_do;
   
   wbm_cyc_o <= 0;
   wbm_stb_o <= 0;
   wbm_we_o  <= 0;
   wbm_cti_o <= 3'b000; // classic default
   wbm_ack_i_last <= wbm_ack_i;

   bram_badmap_wren <= 0;
   bram_page_wren   <= 0;
   
   dc <= dc + 1'b1;
   
   // this code especially the the wishbone master I/F could be
   // more compact/efficient, maybe in the future
   case(state)
   ST_RESET: begin
      op_page_status <= 0;
      op_page_ack    <= 0;
      op_page_done   <= 0;
      op_page_do_1   <= 0;
      init_done      <= 0;
      err_out_of_extras <= 0;
      remapping_a_block <= 0;
      
      dbg_num_valid_blocks <= 0;
      dbg_rebuilt_badblock <= 0;
      dbg_remapped_runtime <= 0;
      
      bram_badmap_addr <= -1;
      scan_block <= 1; // set to start scanning factory data at physical block 1 
      scan_page  <= 0;
      state      <= ST_CHECKINIT_0;
      
      // uncomment to wipe all blocks (careful)
      //scan_block <= 0;
      //state <= ST_ERASEBLOCK1;
   end
   ST_ERASEBLOCK1: begin
      //debug, erase block1
      wbm_cyc_o <= 1;
      wbm_we_o  <= 1;
      wbm_dat_out <= scan_block; 
      if(~wbm_ack_i) begin
         wbm_adr_o <= NANDC_REG_ERASE;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         
         scan_block <= scan_block + 1'b1;
         if(scan_block == NAND_ALLOWED_PHY_BLOCKS) begin
            scan_block <= 1;
            state <= ST_CHECKINIT_0;
         end
      end 
   end
   ST_IDLE: begin
      dc <= 0;
      if(op_page_do & ~op_page_do_1) begin   
         // rising edge
         // lookup this block in the remap table
         bram_badmap_addr <= op_page_num / NAND_PAGE_PER_BLOCK;
         state <= ST_IDLE_REQ;
      end 
   end
   ST_IDLE_REQ: begin
      physical_page  <= bram_badmap_q * NAND_PAGE_PER_BLOCK + (op_page_num % NAND_PAGE_PER_BLOCK);
      op_page_ack    <= 1;
      op_page_done   <= 0;
      op_page_status <= 0;
      bytes_done     <= 0;
      
      if(dc == 2) begin // delay to let bram_q settle
         case(op_page_cmd)
         OP_PAGE_READ: begin // read 1 page and its 64bit spare
            bram_page_addr <= op_page_bram * (NAND_PAGE_SIZE / 4) - 1;
            state <= ST_READ_0;
            // ST_READ_0
            // ST_SPARE_RD_0
            // [return]
         end
         OP_PAGE_READ_EMPTY: begin // zero out bram, used for empty physical blocks
            bram_page_addr <= op_page_bram * (NAND_PAGE_SIZE / 4) - 1;
            state <= ST_EMPTY_0;
            // ST_EMPTY_0
            // [return]
         end
         OP_PAGE_WRITE: begin // write 64bit spare and its 1 page
            bram_page_addr <= op_page_bram * (NAND_PAGE_SIZE / 4);
            state <= ST_SPARE_WR_0;
            // ST_SPARE_WR_0
            // ST_WRITE_0
            // ST_STATUS_RD_0
            // [return]
         end
         OP_PAGE_POKE: begin // poke 1 page and return its spare only
            state <= ST_POKE_0;
            // ST_POKE_0
            // ST_SPARE_RD_0
            // [return]
         end
         OP_PAGE_BLOCKERASE: begin // erase an entire block (64 pages)
            state <= ST_ERASE_0;
            // ST_ERASE_0
            // ST_STATUS_RD_0
            // [return]
         end
         endcase
      end
   end
   ST_IDLE_WAIT: begin
      if(~op_page_do) begin
         // clear ACK to logical
         op_page_ack <= 0;
         state <= ST_IDLE;
      end
   end
   ST_READ_0: begin
      /*
      wbm_cyc_o <= 1;
      wbm_cti_o <= 3'b000; // classic mode
      if(~wbm_ack_i) begin
         wbm_adr_o <= physical_page * NAND_PAGE_SIZE + bytes_done;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         // latch incoming data from wishbone slave
         bram_page_addr <= bram_page_addr + 1'b1;
         bram_page_data <= wbm_dat_i;
         bram_page_wren <= 1;
         wbm_stb_o <= 0;
         
         bytes_done <= bytes_done + 16'h4;
         if(bytes_done + 16'h4 == NAND_PAGE_SIZE) begin
            // done, read spare data
            $display("FTL_PHYSICAL: read logical page %d, which is logical block %d mapped to physical block %d", 
               op_page_num, op_page_num / NAND_PAGE_PER_BLOCK, physical_page / NAND_PAGE_PER_BLOCK);
            bytes_done <= 0;
            state <= ST_SPARE_RD_0;
         end
      end 
      */
      wbm_cyc_o <= 1;
      wbm_stb_o <= 1;
      wbm_cti_o <= 3'b010; // incrementing address mode
      wbm_adr_o <= physical_page * NAND_PAGE_SIZE;
      state <= ST_READ_1;
   end
   ST_READ_1: begin
      wbm_cyc_o <= 1;
      wbm_stb_o <= 1;
      wbm_cti_o <= wbm_cti_o; // incrementing address mode
      if(wbm_ack_i) begin
         // latch incoming data from wishbone slave
         bram_page_addr <= bram_page_addr + 1'b1;
         bram_page_data <= wbm_dat_i;
         bram_page_wren <= 1;
         wbm_adr_o  <= wbm_adr_o + 'd4;
         bytes_done <= bytes_done + 16'h4;
         if(bytes_done + 16'h0 == NAND_PAGE_SIZE) wbm_cti_o <= 3'b111; // end of burst
         if(bytes_done + 16'h4 == NAND_PAGE_SIZE) begin
            wbm_cyc_o  <= 0;
            wbm_stb_o  <= 0;
            // done, read spare data
            $display("FTL_PHYSICAL: read logical page %d, which is logical block %d mapped to physical block %d", 
               op_page_num, op_page_num / NAND_PAGE_PER_BLOCK, physical_page / NAND_PAGE_PER_BLOCK);
            bytes_done <= 0;
            // only read in spare space on first page in block
            if(physical_page % NAND_PAGE_PER_BLOCK == 0)
               state <= ST_SPARE_RD_0;
            else
               state <= ST_READ_2;
         end
      end 
   end
   ST_READ_2: begin
      op_page_done <= 1;
      state <= ST_IDLE_WAIT;
   end
   ST_EMPTY_0: begin
      bram_page_addr <= bram_page_addr + 1;
      bram_page_data <= 32'h0; // if you want empty blocks to return FF's, change to 32'hFFFFFFFF
      bram_page_wren <= 1;
      bytes_done     <= bytes_done + 16'h4;
      if(bytes_done + 16'h4 == NAND_PAGE_SIZE) begin
         op_page_done <= 1;
         state <= ST_IDLE_WAIT;
         $display("FTL_PHYSICAL: read empty logical page %d, which is logical block %d mapped to physical block %d", 
               op_page_num, op_page_num / NAND_PAGE_PER_BLOCK, physical_page / NAND_PAGE_PER_BLOCK);
      end 
   end
   ST_POKE_0: begin
      // single word read to make NANDC load the page and its spare data
      wbm_cyc_o <= 1;
      if(~wbm_ack_i) begin
         wbm_adr_o <= physical_page * NAND_PAGE_SIZE;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         $display("FTL_PHYSICAL: poke logical page %d, which is logical block %d mapped to physical block %d", 
               op_page_num, op_page_num / NAND_PAGE_PER_BLOCK, physical_page / NAND_PAGE_PER_BLOCK);
         bytes_done <= 0;
         state <= ST_SPARE_RD_0;
      end
   end
   ST_SPARE_RD_0: begin
      wbm_cyc_o <= 1;
      if(~wbm_ack_i) begin
         wbm_adr_o <= NANDC_REG_SPARE_RD + bytes_done;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         // latch incoming data from wishbone slave
         if(bytes_done == 0) op_page_spare_rd[41:32] <= wbm_dat_i;
         if(bytes_done == 4) op_page_spare_rd[31:0]  <= wbm_dat_i;
         wbm_stb_o <= 0;
         
         bytes_done <= bytes_done + 16'h4;
         if(bytes_done + 16'h4 == 8) begin
            op_page_done <= 1;
            state <= ST_IDLE_WAIT;
         end
      end 
   end
   ST_SPARE_WR_0: begin
      wbm_cyc_o <= 1;
      wbm_we_o  <= 1;
      if(~wbm_ack_i) begin
         wbm_adr_o <= NANDC_REG_SPARE_WR + bytes_done;
         wbm_stb_o <= 1;
         if(bytes_done == 0) wbm_dat_out <= op_page_spare_wr[41:32]; 
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         if(bytes_done == 0) wbm_dat_out <= op_page_spare_wr[31:0];
         
         bytes_done <= bytes_done + 16'h4;
         if(bytes_done + 16'h4 == 8) begin
            // done
            // now write page data
            bytes_done <= 0;
            state <= ST_WRITE_0;
         end
      end 
   end
   ST_WRITE_0: begin
      wbm_cyc_o <= 1;
      wbm_we_o  <= 1;

      if(~wbm_ack_i) begin
         wbm_adr_o <= physical_page * NAND_PAGE_SIZE + bytes_done;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         // increment bram read pointer
         bram_page_addr <= bram_page_addr + 1;
         wbm_stb_o      <= 0;
         
         bytes_done <= bytes_done + 16'h4;
         if(bytes_done + 16'h4 == NAND_PAGE_SIZE) begin
            state <= ST_STATUS_RD_0;
            $display("FTL_PHYSICAL: wrote logical page %d, which is logical block %d mapped to physical block %d : phys page %d", 
               op_page_num, op_page_num / NAND_PAGE_PER_BLOCK, physical_page / NAND_PAGE_PER_BLOCK, physical_page);
         end
      end 
   end
   ST_ERASE_0: begin
      wbm_cyc_o <= 1;
      wbm_we_o  <= 1;
      wbm_dat_out <= physical_page / NAND_PAGE_PER_BLOCK;
      if(~wbm_ack_i) begin
         wbm_adr_o <= NANDC_REG_ERASE;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         state <= ST_STATUS_RD_0;
         $display("FTL_PHYSICAL: erased logical pageblock %d, which is logical block %d mapped to physical block %d", 
               op_page_num, op_page_num / NAND_PAGE_PER_BLOCK, physical_page / NAND_PAGE_PER_BLOCK);
      end 
   end
   ST_STATUS_RD_0: begin
      wbm_cyc_o <= 1;
      if(~wbm_ack_i) begin
         wbm_adr_o <= NANDC_REG_STATUS;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         // latch incoming data from wishbone slave
         op_page_status <= wbm_dat_i[0];
         wbm_stb_o      <= 0;
         if(wbm_dat_i[0]) begin
            // nonzero return flag! error
            $display("FTL_PHYSICAL: nonzero STATUS_RD from program/erase");
            dc    <= 0;
            remapping_a_block <= 1;
            state <= ST_REMAPNEW_0;
            // set table pointer to last entry of bram, prepare to work backwards
            bram_badmap_addr <= -1;
         end else begin
            op_page_done <= 1;
            state <= ST_IDLE_WAIT;
         end
      end 
   end   
   
   //
   // CHECKINIT: load block 0 and see if badblock table is already made
   //
   ST_CHECKINIT_0: begin
      wbm_cyc_o <= 1;
      if(~wbm_ack_i) begin
         wbm_adr_o <= 0;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         if(wbm_dat_i == 32'hDEADBEEF) begin
            $display("FTL_PHYSICAL: valid badblock table magic was found, loading");
            state <= ST_LOADTABLE_0;
         end else begin
            $display("FTL_PHYSICAL: no badblock table found, rebuilding from factory data");
            dbg_rebuilt_badblock <= 1;
            scan_blockgood <= 1;
            state <= ST_SCANBLOCKS_0;
         end
      end 
   end
   ST_CHECKINIT_1: begin   
      // return here from either loading the remap table, or creating it!
      // also here after a flush from a new bad block being added
      init_done <= 1;
      state <= ST_IDLE;
   end
   //
   // ST_LOADTABLE: read bram into local remap table
   //
   ST_LOADTABLE_0: begin
      wbm_cyc_o <= 1;
      if(~wbm_ack_i) begin
         wbm_adr_o <= (bram_badmap_addr_1) * 2 + 4; // skip 4bytes for the magic
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         bram_badmap_data_32 <= wbm_dat_i;
         $display("FTL_PHYSICAL: loading from %h: data %h", wbm_adr_o, wbm_dat_i);
         dc    <= 0;
         state <= ST_LOADTABLE_1;
      end 
   end
   ST_LOADTABLE_1: begin
      wbm_cyc_o <= 1;
      case(dc) // load two entries (each is 16bit) to fill the 32bit wishbone word 
      1: begin
         bram_badmap_data <= bram_badmap_data_32[31:16];
         bram_badmap_addr <= bram_badmap_addr + 1'b1;
         bram_badmap_wren <= 1;
         if(bram_badmap_data_32[31:16]) dbg_num_valid_blocks <= dbg_num_valid_blocks + 1'b1;
      end
      3: begin
         bram_badmap_data <= bram_badmap_data_32[15:0];
         bram_badmap_addr <= bram_badmap_addr + 1'b1;
         bram_badmap_wren <= 1;
         if(bram_badmap_data_32[15:0]) dbg_num_valid_blocks <= dbg_num_valid_blocks + 1'b1;
         
         state <= ST_LOADTABLE_0;
         if(bram_badmap_addr == NAND_ALLOWED_PHY_BLOCKS) begin
            // there is only room for 1022 entries in the first page, NAND_ALLOWED_PHY_BLOCKS 
            // must be smaller or equal to this
            state <= ST_CHECKINIT_1;
         end
      end
      endcase
   end
   
   //
   // ST_SCANBLOCKS: read spare data in each block to determine factory bad block marking
   //
   ST_SCANBLOCKS_0: begin
      wbm_cyc_o <= 1;
      wbm_stb_o <= 1;
      wbm_we_o  <= 1;
      wbm_dat_out <= 1; // enable raw spare data access
      wbm_adr_o <= NANDC_REG_OFFSET;
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         state <= ST_SCANBLOCKS_1;
      end 
   end
   ST_SCANBLOCKS_1: begin
      wbm_cyc_o <= 1;
      wbm_stb_o <= 1;
      wbm_adr_o <= (scan_block * NAND_PAGE_PER_BLOCK + scan_page) * NAND_PAGE_SIZE;
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         if(wbm_dat_i[31:0] != 32'hFFFFFFFF) begin // wbm_dat_i[31:24] == 8'hFF
            scan_blockgood <= 0;
            $display("FTL_PHYSICAL: found bad block factory marked at physical block %d: page %d", scan_block, scan_page);
         end
         state <= ST_SCANBLOCKS_2;
      end 
   end
   ST_SCANBLOCKS_2: begin
      wbm_cyc_o <= 1;
      wbm_stb_o <= 1;
      wbm_we_o  <= 1;
      wbm_dat_out <= 0; // disable raw spare data access (necessary?)
      wbm_adr_o <= NANDC_REG_OFFSET;
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         
         state <= ST_SCANBLOCKS_0;
         case(scan_page)
         0: scan_page <= 1;
         1: scan_page <= 63;
         63: begin
            scan_blockgood <= 1;
            scan_page  <= 0;
            scan_block <= scan_block + 1'b1;
            if(scan_blockgood) begin
               // load this block into the table
               dbg_num_valid_blocks <= dbg_num_valid_blocks + 1'b1;
               bram_badmap_addr <= bram_badmap_addr + 1'b1;
               bram_badmap_data <= scan_block;
               bram_badmap_wren <= 1;
               $display("FTL_PHYSICAL: adding physical block %d as logical %d", scan_block, bram_badmap_addr+1 == 1024 ? 0 : bram_badmap_addr+1);
            end else $display("FTL_PHYSICAL: not adding bad block %d", scan_block);
            if(scan_block == NAND_ALLOWED_PHY_BLOCKS-1) begin
               // last one
               state <= ST_SCANBLOCKS_3;
            end
         end
         endcase
      end 
   end
   ST_SCANBLOCKS_3: begin
      // zero out the remaining entries
      bram_badmap_addr <= bram_badmap_addr + 1'b1;
      bram_badmap_data <= 0;
      bram_badmap_wren <= 1;
      if(bram_badmap_addr == NAND_DEVICE_NUM_BLOCKS-2) state <= ST_FLUSHTABLE_0;
   end
   
   //
   // ST_FLUSHTABLE: erase block0 and write 1022 entries into the first page
   //
   ST_FLUSHTABLE_0: begin
      wbm_cyc_o   <= 1;
      wbm_we_o    <= 1;
      wbm_dat_out <= 0; // erase block 0
      if(~wbm_ack_i) begin
         wbm_adr_o <= NANDC_REG_ERASE;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         $display("FTL_PHYSICAL: flushing badblock table");
         state <= ST_FLUSHTABLE_1;
      end 
   end
   ST_FLUSHTABLE_1: begin
      wbm_cyc_o <= 1;
      wbm_we_o  <= 1;
      wbm_dat_out <= 32'hDEADBEEF; // block0 magic
      if(~wbm_ack_i) begin
         wbm_adr_o <= 0;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         dc        <= 0;
         bram_badmap_addr <= 0;
         state <= ST_FLUSHTABLE_2;
      end 
   end
   ST_FLUSHTABLE_2: begin
      //wbm_cyc_o <= 1;
      case(dc) // load two entries (each is 16bit) to fill the 32bit wishbone word 
      1: begin
         bram_badmap_data_32[31:16] <= bram_badmap_q;
         bram_badmap_addr <= bram_badmap_addr + 1'b1;
      end
      3: begin
         bram_badmap_data_32[15:0] <= bram_badmap_q;
         bram_badmap_addr <= bram_badmap_addr + 1'b1;
         state <= ST_FLUSHTABLE_3;
      end
      endcase
   end
   ST_FLUSHTABLE_3: begin
      wbm_cyc_o <= 1; // write the word into the page of block0
      wbm_we_o  <= 1;
      if(~wbm_ack_i) begin
         wbm_adr_o <= (bram_badmap_addr-2) * 2 + 4; // skip 4bytes for the magic
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         wbm_stb_o <= 0;
         dc        <= 0;
         state <= ST_FLUSHTABLE_2;
         if(bram_badmap_addr == NAND_DEVICE_NUM_BLOCKS-2) begin 
            // end, remember only 1022 entries are allowed to fit this page
            state <= ST_CHECKINIT_1;
            // if this was a flush caused by a new block, return to normal operation
            if(remapping_a_block) begin
               op_page_done <= 1;
               state <= ST_IDLE_WAIT;
            end
         end
      end 
   end
   
   //
   // ST_REMAPNEW: find a substitute for the new bad block and then update&flush the table
   //
   ST_REMAPNEW_0: begin
      dbg_remapped_runtime <= 1;
      if(dc == 2) begin // delay for bram
         state <= ST_REMAPNEW_1;
         if(bram_badmap_q != 0) begin
            // nonzero entry, swap
            last_good_block <= bram_badmap_addr;
            state <= ST_REMAPNEW_2;
         end
      end
   end
   ST_REMAPNEW_1: begin
      dc    <= 0;
      state <= ST_REMAPNEW_0;
      bram_badmap_addr <= bram_badmap_addr - 1'b1;
      // is this entry next to the end of user area?
      if(bram_badmap_addr == NAND_ALLOWED_LOG_BLOCKS) begin
         $display("FTL_PHYSICAL: ran out of remappable blocks and encroaching upon user area");
         err_out_of_extras <= 1;
      end
   end
   ST_REMAPNEW_2: begin
      $display("FTL_PHYSICAL: new bad block; logical block %d was physical %d, now physical %d", 
         op_page_num / NAND_PAGE_PER_BLOCK, 
         physical_page / NAND_PAGE_PER_BLOCK,
         bram_badmap_q);
      bram_badmap_addr <= op_page_num / NAND_PAGE_PER_BLOCK;
      bram_badmap_data <= bram_badmap_q;
      bram_badmap_wren <= 1;
      state <= ST_REMAPNEW_3;
   end
   ST_REMAPNEW_3: begin
      bram_badmap_addr <= last_good_block;
      bram_badmap_data <= 0;
      bram_badmap_wren <= 1;
   
      state <= ST_FLUSHTABLE_0;
   end
   endcase
   
   if(~reset_n) begin
      state <= ST_RESET;
   end    
end

// 2048 byte bram (2^10=1024 x 16bit word)
ftl_bram_block_dp #(16, 10) ipbbram (
   .a_clk  ( clk_50 ),
   .a_wr   ( bram_badmap_wren ),
   .a_addr ( bram_badmap_addr ),
   .a_din  ( bram_badmap_data ),
   .a_dout ( bram_badmap_q )
);

endmodule
