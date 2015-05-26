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
module sd_top (
   // clocking/reset
   input  wire        clk_50,
   input  wire        clk_100,
   input  wire        clk_200,
   input  wire        reset_n,

   // physical interface to SD pins
   input  wire        sd_clk,
   input  wire        sd_cmd_i,
   output wire        sd_cmd_o,
   output wire        sd_cmd_t,
   input  wire [3:0]  sd_dat_i,
   output wire [3:0]  sd_dat_o,
   output wire [3:0]  sd_dat_t,

   // wishbone interface
   output wire        wbm_clk_o,
   output wire [31:0] wbm_adr_o,
   input  wire [31:0] wbm_dat_i,
   output wire [31:0] wbm_dat_o,
   output wire [3:0]  wbm_sel_o,
   output wire        wbm_cyc_o,
   output wire        wbm_stb_o,
   output wire        wbm_we_o,
   input  wire        wbm_ack_i,
   output wire [2:0]  wbm_cti_o,
   output wire [1:0]  wbm_bte_o,

   // options
   input  wire        opt_enable_hs

   // debug (optional)
);

wire        bram_rd_ext_clk;
wire [6:0]  bram_rd_ext_addr;
wire        bram_rd_ext_wren;
wire [31:0] bram_rd_ext_data;
wire [31:0] bram_rd_ext_q;

wire        bram_wr_ext_clk;
wire [6:0]  bram_wr_ext_addr;
wire        bram_wr_ext_wren;
wire [31:0] bram_wr_ext_data;
wire [31:0] bram_wr_ext_q;

wire        ext_read_act;
wire        ext_read_go;
wire [31:0] ext_read_addr;
wire        ext_read_stop;

wire        ext_write_act;
wire        ext_write_done;
wire [31:0] ext_write_addr;

   
sd_wishbone isdw (
   .clk_50           ( clk_50 ),
   .reset_n          ( reset_n ),
   
   .ext_read_act     ( ext_read_act ),
   .ext_read_go      ( ext_read_go ),
   .ext_read_addr    ( ext_read_addr ),
   .ext_read_stop    ( ext_read_stop ),
   
   .ext_write_act    ( ext_write_act ),
   .ext_write_done   ( ext_write_done ),
   .ext_write_addr   ( ext_write_addr ),
   
   .bram_rd_ext_clk  ( bram_rd_ext_clk ),
   .bram_rd_ext_addr ( bram_rd_ext_addr ),
   .bram_rd_ext_wren ( bram_rd_ext_wren ),
   .bram_rd_ext_data ( bram_rd_ext_data ),
   .bram_rd_ext_q    ( bram_rd_ext_q ),
      
   .bram_wr_ext_clk  ( bram_wr_ext_clk ),
   .bram_wr_ext_addr ( bram_wr_ext_addr ),
   .bram_wr_ext_wren ( bram_wr_ext_wren ),
   .bram_wr_ext_data ( bram_wr_ext_data ),
   .bram_wr_ext_q    ( bram_wr_ext_q ),

   .wbm_clk_o        ( wbm_clk_o ),
   .wbm_adr_o        ( wbm_adr_o ),
   .wbm_dat_i        ( wbm_dat_i ),
   .wbm_dat_o        ( wbm_dat_o ),
   .wbm_sel_o        ( wbm_sel_o ),
   .wbm_cyc_o        ( wbm_cyc_o ),
   .wbm_stb_o        ( wbm_stb_o ),
   .wbm_we_o         ( wbm_we_o ),
   .wbm_ack_i        ( wbm_ack_i ),
   .wbm_cti_o        ( wbm_cti_o ),
   .wbm_bte_o        ( wbm_bte_o )
);
   
wire         bram_rd_sd_clk;
wire [6:0]   bram_rd_sd_addr;
wire         bram_rd_sd_wren;
wire [31:0]  bram_rd_sd_data;
wire [31:0]  bram_rd_sd_q;
   
wire         bram_wr_sd_clk;
wire [6:0]   bram_wr_sd_addr;
wire         bram_wr_sd_wren;
wire [31:0]  bram_wr_sd_data;
wire [31:0]  bram_wr_sd_q;

wire         link_read_act;
wire         link_read_go;
wire [31:0]  link_read_addr;
wire [31:0]  link_read_num;
wire         link_read_stop;
    
wire         link_write_act;
wire         link_write_done;
wire [31:0]  link_write_addr;
wire [31:0]  link_write_num;

(* mark_debug = "true" *) wire [47:0] phy_cmd_in;
(* mark_debug = "true" *) wire        phy_cmd_in_crc_good;
(* mark_debug = "true" *) wire        phy_cmd_in_act;
(* mark_debug = "true" *) wire [3:0]  phy_resp_type;
(* mark_debug = "true" *) wire        phy_resp_act;
(* mark_debug = "true" *) wire        phy_resp_done;
(* mark_debug = "true" *) wire [3:0]  link_card_state;
(* mark_debug = "true" *) wire [10:0] odc;
(* mark_debug = "true" *) wire [6:0]  ostate;
(* mark_debug = "true" *) wire [5:0]  cmd_in_cmd;
    
wire         phy_data_in_act;
wire         phy_data_in_busy;
wire         phy_data_in_another;
wire         phy_data_in_stop;
wire         phy_data_in_done;
wire         phy_data_in_crc_good;
   
wire [135:0] phy_resp_out;
wire         phy_resp_busy;
wire         phy_mode_4bit;
wire [511:0] phy_data_out_reg;
wire         phy_data_out_src;
wire [9:0]   phy_data_out_len;
wire         phy_data_out_busy;
wire         phy_data_out_act;
wire         phy_data_out_stop;
wire         phy_data_out_done;

sd_mgr isdm (
   .clk_50           ( clk_50 ),
   .reset_n          ( reset_n ),
   
   .bram_rd_sd_clk   ( bram_rd_sd_clk ),
   .bram_rd_sd_addr  ( bram_rd_sd_addr ),
   .bram_rd_sd_wren  ( bram_rd_sd_wren ),
   .bram_rd_sd_data  ( bram_rd_sd_data ),
   .bram_rd_sd_q     ( bram_rd_sd_q ),
   
   .bram_rd_ext_clk  ( bram_rd_ext_clk ),
   .bram_rd_ext_addr ( bram_rd_ext_addr ),
   .bram_rd_ext_wren ( bram_rd_ext_wren ),
   .bram_rd_ext_data ( bram_rd_ext_data ),
   .bram_rd_ext_q    ( bram_rd_ext_q ),
   
   .bram_wr_sd_clk   ( bram_wr_sd_clk ),
   .bram_wr_sd_addr  ( bram_wr_sd_addr ),
   .bram_wr_sd_wren  ( bram_wr_sd_wren ),
   .bram_wr_sd_data  ( bram_wr_sd_data ),
   .bram_wr_sd_q     ( bram_wr_sd_q ),
   
   .bram_wr_ext_clk  ( bram_wr_ext_clk ),
   .bram_wr_ext_addr ( bram_wr_ext_addr ),
   .bram_wr_ext_wren ( bram_wr_ext_wren ),
   .bram_wr_ext_data ( bram_wr_ext_data ),
   .bram_wr_ext_q    ( bram_wr_ext_q ),
   
   .link_read_act    ( link_read_act ),
   .link_read_go     ( link_read_go ),
   .link_read_addr   ( link_read_addr ),
   .link_read_num    ( link_read_num ),
   .link_read_stop   ( link_read_stop ),
   
   .link_write_act   ( link_write_act ),
   .link_write_done  ( link_write_done ),
   .link_write_addr  ( link_write_addr ),
   .link_write_num   ( link_write_num ),
   
   .ext_read_act     ( ext_read_act ),
   .ext_read_go      ( ext_read_go ),
   .ext_read_addr    ( ext_read_addr ),
   .ext_read_num     (  ),
   .ext_read_stop    ( ext_read_stop ),
   
   .ext_write_act    ( ext_write_act ),
   .ext_write_done   ( ext_write_done ),
   .ext_write_addr   ( ext_write_addr ),
   .ext_write_num    (  )
);


sd_link isdl (
   .clk_50               ( clk_50 ),
   .reset_n              ( reset_n ),

   .link_card_state      ( link_card_state ),
   
   .phy_cmd_in           ( phy_cmd_in ),
   .phy_cmd_in_crc_good  ( phy_cmd_in_crc_good ),
   .phy_cmd_in_act       ( phy_cmd_in_act ),
   .phy_data_in_act      ( phy_data_in_act ),
   .phy_data_in_busy     ( phy_data_in_busy ),
   .phy_data_in_another  ( phy_data_in_another ),
   .phy_data_in_stop     ( phy_data_in_stop ),
   .phy_data_in_done     ( phy_data_in_done),
   .phy_data_in_crc_good ( phy_data_in_crc_good ),
   
   .phy_resp_out         ( phy_resp_out ),
   .phy_resp_type        ( phy_resp_type ),
   .phy_resp_busy        ( phy_resp_busy ),
   .phy_resp_act         ( phy_resp_act ),
   .phy_resp_done        ( phy_resp_done ),
   .phy_mode_4bit        ( phy_mode_4bit ),
   .phy_data_out_reg     ( phy_data_out_reg ),
   .phy_data_out_src     ( phy_data_out_src ),
   .phy_data_out_len     ( phy_data_out_len ),
   .phy_data_out_busy    ( phy_data_out_busy ),
   .phy_data_out_act     ( phy_data_out_act ),
   .phy_data_out_stop    ( phy_data_out_stop ),
   .phy_data_out_done    ( phy_data_out_done ),
   
   .block_read_act       ( link_read_act ),
   .block_read_go        ( link_read_go ),
   .block_read_addr      ( link_read_addr ),
   .block_read_num       ( link_read_num ),
   .block_read_stop      ( link_read_stop ),
   
   .block_write_act      ( link_write_act ),
   .block_write_done     ( link_write_done ),
   .block_write_addr     ( link_write_addr ),
   .block_write_num      ( link_write_num ),
   
   .opt_enable_hs        ( opt_enable_hs )

   /* DEBUG SIGNALS */,
   .cmd_in_cmd           ( cmd_in_cmd )
);
   
sd_phy isdph (
   .clk_50           ( clk_50 ),
   .reset_n          ( reset_n ),
   .sd_clk           ( sd_clk ),
   .sd_cmd_i         ( sd_cmd_i ),
   .sd_cmd_o         ( sd_cmd_o ),
   .sd_cmd_t         ( sd_cmd_t ),
   .sd_dat_i         ( sd_dat_i ),
   .sd_dat_o         ( sd_dat_o ),
   .sd_dat_t         ( sd_dat_t ),
   
   .card_state       ( link_card_state ),
   .cmd_in           ( phy_cmd_in ),
   .cmd_in_crc_good  ( phy_cmd_in_crc_good ),
   .cmd_in_act       ( phy_cmd_in_act ),
   .data_in_act      ( phy_data_in_act ),
   .data_in_busy     ( phy_data_in_busy ),
   .data_in_stop     ( phy_data_in_stop ),
   .data_in_another  ( phy_data_in_another ),
   .data_in_done     ( phy_data_in_done),
   .data_in_crc_good ( phy_data_in_crc_good ),

   .resp_out         ( phy_resp_out ),
   .resp_type        ( phy_resp_type ),
   .resp_busy        ( phy_resp_busy ),
   .resp_act         ( phy_resp_act ),
   .resp_done        ( phy_resp_done ),
   .mode_4bit        ( phy_mode_4bit ),
   .data_out_reg     ( phy_data_out_reg ),
   .data_out_src     ( phy_data_out_src ),
   .data_out_len     ( phy_data_out_len ),
   .data_out_busy    ( phy_data_out_busy ),
   .data_out_act     ( phy_data_out_act ),
   .data_out_stop    ( phy_data_out_stop ),
   .data_out_done    ( phy_data_out_done ),

   .bram_rd_sd_clk   ( bram_rd_sd_clk ),
   .bram_rd_sd_addr  ( bram_rd_sd_addr ),
   .bram_rd_sd_wren  ( bram_rd_sd_wren ),
   .bram_rd_sd_data  ( bram_rd_sd_data ),
   .bram_rd_sd_q     ( bram_rd_sd_q ),
   
   .bram_wr_sd_clk   ( bram_wr_sd_clk ),
   .bram_wr_sd_addr  ( bram_wr_sd_addr ),
   .bram_wr_sd_wren  ( bram_wr_sd_wren ),
   .bram_wr_sd_data  ( bram_wr_sd_data ),
   .bram_wr_sd_q     ( bram_wr_sd_q )

    /* DEBUG SIGNALS */,   
   .odc              ( odc ),
   .ostate           ( ostate )
);

/*
 * ILA Debug
 */
/*
(* mark_debug = "true" *) reg        sd_clk_reg, sd_cmd_reg;
(* mark_debug = "true" *) reg [3:0]  sd_dat_reg;
(* mark_debug = "true" *) reg        wb_clk_reg;
(* mark_debug = "true" *) reg [31:0] wb_adr_reg;
(* mark_debug = "true" *) reg [31:0] wb_dat_i_reg;
(* mark_debug = "true" *) reg [31:0] wb_dat_o_reg;
(* mark_debug = "true" *) reg [3:0]  wb_sel_reg;
(* mark_debug = "true" *) reg        wb_cyc_reg;
(* mark_debug = "true" *) reg        wb_stb_reg;
(* mark_debug = "true" *) reg        wb_we_reg;
(* mark_debug = "true" *) reg        wb_ack_reg;

always @(posedge clk_50) begin
   sd_clk_reg <= sd_clk;
   sd_cmd_reg <= sd_cmd_i;
   sd_dat_reg <= sd_dat_i;
end

always @(posedge wbm_clk_o) begin
   wb_adr_reg   <= wbm_adr_o;
   wb_dat_i_reg <= wbm_dat_i;
   wb_dat_o_reg <= wbm_dat_o;
   wb_sel_reg   <= wbm_sel_o;
   wb_cyc_reg   <= wbm_cyc_o;
   wb_stb_reg   <= wbm_stb_o;
   wb_we_reg    <= wbm_we_o;
   wb_ack_reg   <= wbm_ack_i;
end

ila_0 ila_0 (
    .clk    ( wbm_clk_o ),
    .probe0 ( {wb_adr_reg,
               wb_dat_i_reg,
               wb_dat_o_reg,
               wb_sel_reg,
               wb_cyc_reg,
               wb_stb_reg,
               wb_we_reg,
               wb_ack_reg,
               link_card_state,
               phy_resp_type,
               odc,
               ostate,
               phy_resp_done,
               phy_resp_act,
               phy_cmd_in_act,
               cmd_in_cmd,
               phy_cmd_in,
               phy_cmd_in_crc_good,
               sd_dat_reg,
               sd_cmd_reg,
               sd_clk_reg} )
);
*/

endmodule
