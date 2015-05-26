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
module sd_wishbone (
   input  wire        clk_50,
   input  wire        reset_n,

   // dma bus to MGR
   input  wire        ext_read_act,
   output reg         ext_read_go,
   input  wire [31:0] ext_read_addr,
   input  wire        ext_read_stop,

   input  wire        ext_write_act,
   output reg         ext_write_done,
   input  wire [31:0] ext_write_addr,
   
   output wire        bram_rd_ext_clk,
   output reg  [6:0]  bram_rd_ext_addr,
   output reg         bram_rd_ext_wren,
   output reg  [31:0] bram_rd_ext_data,
   input  wire [31:0] bram_rd_ext_q,

   output wire        bram_wr_ext_clk,
   output reg  [6:0]  bram_wr_ext_addr,
   output wire        bram_wr_ext_wren,
   output wire [31:0] bram_wr_ext_data,
   input  wire [31:0] bram_wr_ext_q,

   // wishbone bus
   output wire        wbm_clk_o,   // clock  - bus clock
   output reg  [31:0] wbm_adr_o,   // addr   - bus address
   input  wire [31:0] wbm_dat_i,   // data   - write data input
   output wire [31:0] wbm_dat_o,   // data   - write data output
   output reg  [3:0]  wbm_sel_o,   // select - 8-bit enable for data bus
   output reg         wbm_cyc_o,   // cycle  - valid bus cycle is in progress
   output reg         wbm_stb_o,   // strobe - slave is selected
   output reg         wbm_we_o,    // write  - bus cycle is in write mode
   input  wire        wbm_ack_i,   // ack    - end of a normal bus cycle
   output reg  [2:0]  wbm_cti_o,   // cti    - cycle type identifier
   output reg  [1:0]  wbm_bte_o    // bte    - burst type
);


assign wbm_clk_o = clk_50;
assign wbm_dat_o = bram_wr_ext_q;

assign bram_rd_ext_clk = wbm_clk_o;
assign bram_wr_ext_clk = wbm_clk_o;

reg [4:0] state;
parameter [4:0] ST_RESET      = 'd0,
                ST_IDLE       = 'd4,
                ST_WB_READ_0  = 'd8,
                ST_WB_READ_1  = 'd9,
                ST_WB_READ_2  = 'd10,
                ST_WB_READ_3  = 'd11,
                ST_WB_READ_4  = 'd12,
                ST_WB_WRITE_0 = 'd13,
                ST_WB_WRITE_1 = 'd14,
                ST_WB_WRITE_2 = 'd15,
                ST_WB_WRITE_3 = 'd16,
                ST_WB_WRITE_4 = 'd17,
                ST_LAST       = 'd31;

wire [6:0]  bram_rd_ext_addr_next = bram_rd_ext_addr + 1'b1;
reg  [15:0] bytes_done;
   
reg  [31:0] ext_read_addr_latch;
reg  [31:0] ext_write_addr_latch;
   
reg  ext_read_act_last;
reg  ext_write_act_last;
reg  wbm_ack_i_last;
   
   
wire reset_s;
synch_3 a(reset_n, reset_s, clk_50);

always @(posedge wbm_clk_o) begin
   ext_read_act_last <= ext_read_act;
   ext_write_act_last <= ext_write_act;
   wbm_ack_i_last <= wbm_ack_i;
   
   wbm_sel_o <= 4'b1111;
   wbm_cyc_o <= 0;
   wbm_stb_o <= 0;
   wbm_we_o <= 0;
   wbm_cti_o <= 3'b000;
   wbm_bte_o <= 2'b00;
   
   bram_rd_ext_wren <= 0;
   ext_read_go <= 0;
   
   case(state)
   ST_RESET: begin
      state <= ST_IDLE;
   end
   ST_IDLE: begin
      if(~ext_read_act_last & ext_read_act) begin
         ext_read_addr_latch <= ext_read_addr;
         bytes_done <= 0;
         bram_rd_ext_addr <= -1;
         state <= ST_WB_READ_0;
      end
      if(~ext_write_act_last & ext_write_act) begin
         ext_write_addr_latch <= ext_write_addr;
         bytes_done <= 0;
         bram_wr_ext_addr <= 0;
         ext_write_done <= 0;
         state <= ST_WB_WRITE_0;
      end
   end
   ST_WB_READ_0: begin
      wbm_cyc_o <= 1;
      if(~wbm_ack_i) begin
         wbm_adr_o <= ext_read_addr_latch * 512 + bram_rd_ext_addr_next * 4;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         // latch incoming data from wishbone slave
         bram_rd_ext_addr <= bram_rd_ext_addr_next;
         bram_rd_ext_data <= wbm_dat_i;
         bram_rd_ext_wren <= 1;
         wbm_stb_o <= 0;
         
         bytes_done <= bytes_done + 16'h4;
         if(bytes_done + 16'h4 == 512) begin
            // done
            state <= ST_WB_READ_1;
         end
      end 
   end
   ST_WB_READ_1: begin   
      // signal MGR that bram is filled and wait for completion
      ext_read_go <= 1;
      if(ext_read_stop) state <= ST_IDLE;
   end
   
   ST_WB_WRITE_0: begin
      wbm_cyc_o <= 1;
      wbm_we_o <= 1;
      if(~wbm_ack_i) begin
         wbm_adr_o <= ext_write_addr_latch * 512 + bram_wr_ext_addr * 4;
         wbm_stb_o <= 1;
      end
      if(wbm_ack_i & ~wbm_ack_i_last) begin
         // increment bram read pointer
         bram_wr_ext_addr <= bram_wr_ext_addr + 1;
         wbm_stb_o <= 0;
         
         bytes_done <= bytes_done + 16'h4;
         if(bytes_done + 16'h4 == 512) begin
            // done
            state <= ST_WB_WRITE_1;
         end
      end 
   end
   ST_WB_WRITE_1: begin   
      // signal MGR that bram was read
      ext_write_done <= 1;
      state <= ST_IDLE;
   end
   endcase
   
   if(~reset_s) begin
      state <= ST_RESET;
   end
end

endmodule
