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
module ftl_top (
   input  wire        clk_50,
   input  wire        reset_n,

   output wire [9:0]  dbg_phy_num_valid_blocks,
   output wire        dbg_phy_rebuilt_badblock,
   output wire        dbg_phy_remapped_runtime,
   output wire        err_phy_out_of_extras,

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

   // master wishbone interface (to NANDC)
   output wire        wbm_clk_o,
   output wire [2:0]  wbm_cti_o,   // type   - cycle type identifier
   output wire [1:0]  wbm_bte_o,   // exten  - burst type extension
   output wire [31:0] wbm_adr_o,
   input  wire [31:0] wbm_dat_i,
   output wire [31:0] wbm_dat_o,
   output wire [3:0]  wbm_sel_o,
   output wire        wbm_cyc_o,
   output wire        wbm_stb_o,
   output wire        wbm_we_o,
   input  wire        wbm_ack_i
);

wire        reset_s;
synch_3 a(reset_n, reset_s, clk_50);
   
wire        physical_init_done;
wire        op_page_do;
wire [2:0]  op_page_cmd;
wire [15:0] op_page_num;
wire [15:0] op_page_bram;
wire [41:0] op_page_spare_wr;
wire [41:0] op_page_spare_rd;
wire        op_page_status;
wire        op_page_ack;
wire        op_page_done;
   
wire        logical_init_done;
   
wire        wb_read;
wire        wb_write;
wire [9:0]  wb_block;
wire        wb_ack;
wire        wb_done;

wire        bram_wbs_clk;
wire [15:0] bram_wbs_addr;
wire        bram_wbs_wren;
wire [31:0] bram_wbs_data;
wire [31:0] bram_wbs_q;

wire        bram_physical_req;
wire        bram_physical_ack;
wire [15:0] bram_physical_addr;
wire        bram_physical_wren;
wire [31:0] bram_physical_data;
wire [31:0] bram_physical_q;

ftl_wbs ifw (
   .clk_50            ( clk_50 ),
   .reset_n           ( reset_s ),

   // slave wishbone interface (from SDHC)
   .wbs_clk_i         ( wbs_clk_i ),
   .wbs_adr_i         ( wbs_adr_i ),
   .wbs_dat_o         ( wbs_dat_o ),
   .wbs_dat_i         ( wbs_dat_i ),
   .wbs_sel_i         ( wbs_sel_i ),
   .wbs_cyc_i         ( wbs_cyc_i ),
   .wbs_stb_i         ( wbs_stb_i ),
   .wbs_we_i          ( wbs_we_i ),
   .wbs_ack_o         ( wbs_ack_o ),

   // port to cached block ram
   .bram_wbs_clk      ( bram_wbs_clk ),
   .bram_wbs_addr     ( bram_wbs_addr ),
   .bram_wbs_wren     ( bram_wbs_wren ),
   .bram_wbs_data     ( bram_wbs_data ),
   .bram_wbs_q        ( bram_wbs_q ),

   .logical_init_done ( logical_init_done ),
   .wb_read           ( wb_read ),
   .wb_write          ( wb_write ),
   .wb_block          ( wb_block ),
   .wb_ack            ( wb_ack ),
   .wb_done           ( wb_done )
);

ftl_logical ilog (
   .clk_50             ( clk_50 ),
   .reset_n            ( reset_s ),
   
   .physical_init_done ( physical_init_done ),
   .init_done          ( logical_init_done ),
   
   .wb_read            ( wb_read ),
   .wb_write           ( wb_write ),
   .wb_block           ( wb_block ),
   .wb_ack             ( wb_ack ),
   .wb_done            ( wb_done ),
   
   .op_page_do         ( op_page_do ),
   .op_page_cmd        ( op_page_cmd ),
   .op_page_num        ( op_page_num ),
   .op_page_bram       ( op_page_bram ),
   .op_page_spare_wr   ( op_page_spare_wr ),
   .op_page_spare_rd   ( op_page_spare_rd ),
   .op_page_status     ( op_page_status ),
   .op_page_ack        ( op_page_ack ),
   .op_page_done       ( op_page_done )
);

ftl_buf ibuf (
   .clk_50             ( clk_50 ),
   .reset_n            ( reset_s ),

   .bram_wbs_clk       ( bram_wbs_clk ),
   .bram_wbs_addr      ( bram_wbs_addr ),
   .bram_wbs_wren      ( bram_wbs_wren ),
   .bram_wbs_data      ( bram_wbs_data ),
   .bram_wbs_q         ( bram_wbs_q ),

   .bram_physical_addr ( bram_physical_addr ),
   .bram_physical_wren ( bram_physical_wren ),
   .bram_physical_data ( bram_physical_data ),
   .bram_physical_q    ( bram_physical_q )
);

ftl_physical iphy(
   .clk_50               ( clk_50 ),
   .reset_n              ( reset_s ),
   
   .init_done            ( physical_init_done ),
   
   .dbg_num_valid_blocks ( dbg_phy_num_valid_blocks ),
   .dbg_rebuilt_badblock ( dbg_phy_rebuilt_badblock ),
   .dbg_remapped_runtime ( dbg_phy_remapped_runtime ),
   .err_out_of_extras    ( err_phy_out_of_extras ),
   
   .bram_page_addr       ( bram_physical_addr ),
   .bram_page_wren       ( bram_physical_wren ),
   .bram_page_data       ( bram_physical_data ),
   .bram_page_q          ( bram_physical_q ),
   
   .op_page_do           ( op_page_do ),
   .op_page_cmd          ( op_page_cmd ),
   .op_page_num          ( op_page_num ),
   .op_page_bram         ( op_page_bram ),
   .op_page_spare_wr     ( op_page_spare_wr ),
   .op_page_spare_rd     ( op_page_spare_rd ),
   .op_page_status       ( op_page_status ),
   .op_page_ack          ( op_page_ack ),
   .op_page_done         ( op_page_done ),

   .wbm_clk_o            ( wbm_clk_o ),
   .wbm_cti_o            ( wbm_cti_o ),
   .wbm_bte_o            ( wbm_bte_o ),
   .wbm_adr_o            ( wbm_adr_o ),
   .wbm_dat_i            ( wbm_dat_i ),
   .wbm_dat_o            ( wbm_dat_o ),
   .wbm_sel_o            ( wbm_sel_o ),
   .wbm_cyc_o            ( wbm_cyc_o ),
   .wbm_stb_o            ( wbm_stb_o ),
   .wbm_we_o             ( wbm_we_o ),
   .wbm_ack_i            ( wbm_ack_i )
);

/*
(* mark_debug = "true" *) reg wb_read_reg;
(* mark_debug = "true" *) reg wb_write_reg;
(* mark_debug = "true" *) reg [9:0] wb_block_reg;
(* mark_debug = "true" *) reg wb_ack_reg;
(* mark_debug = "true" *) reg wb_done_reg;
(* mark_debug = "true" *) reg logical_init_done_reg;
(* mark_debug = "true" *) reg physical_init_done_reg;

always @(posedge clk_50) begin
   wb_read_reg <= wb_read;
   wb_write_reg <= wb_write;
   wb_block_reg <= wb_block;
   wb_ack_reg <= wb_ack;
   wb_done_reg <= wb_done;
   logical_init_done_reg <= logical_init_done;
   physical_init_done_reg <= physical_init_done;
end

ila_0 ila_0 (
   .clk(clk_50),
   .probe0({
      physical_init_done_reg,
      logical_init_done_reg,
      wb_done_reg,
      wb_ack_reg,
      wb_block_reg,
      wb_write_reg,
      wb_read_reg  
   })
);
*/

endmodule
