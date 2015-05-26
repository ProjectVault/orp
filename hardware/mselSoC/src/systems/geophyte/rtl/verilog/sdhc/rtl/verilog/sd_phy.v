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
module sd_phy (
   input  wire         clk_50,
   input  wire         reset_n,

   input  wire         sd_clk,
   input  wire         sd_cmd_i,
   output wire         sd_cmd_o,
   output wire         sd_cmd_t,
   input  wire [3:0]   sd_dat_i,
   output wire [3:0]   sd_dat_o,
   output wire [3:0]   sd_dat_t,

   input  wire [3:0]   card_state,

   output reg  [47:0]  cmd_in,
   output reg          cmd_in_crc_good,
   output reg          cmd_in_act,
   input  wire         data_in_act,
   output reg          data_in_busy,
   input  wire         data_in_another,
   input  wire         data_in_stop,
   output reg          data_in_done,
   output reg          data_in_crc_good,

   input  wire [135:0] resp_out,
   input  wire [3:0]   resp_type,
   input  wire         resp_busy,
   input  wire         resp_act,
   output reg          resp_done,
   input  wire         mode_4bit,
   input  wire [511:0] data_out_reg,
   input  wire         data_out_src,
   input  wire [9:0]   data_out_len,
   output reg          data_out_busy,
   input  wire         data_out_act,
   input  wire         data_out_stop,
   output reg          data_out_done,

   output wire         bram_rd_sd_clk,
   output reg  [6:0]   bram_rd_sd_addr,
   output reg          bram_rd_sd_wren,
   output reg  [31:0]  bram_rd_sd_data,
   input  wire [31:0]  bram_rd_sd_q,

   output wire         bram_wr_sd_clk,
   output reg  [6:0]   bram_wr_sd_addr,
   output reg          bram_wr_sd_wren,
   output reg  [31:0]  bram_wr_sd_data,
   input  wire [31:0]  bram_wr_sd_q
   ,
   output reg [10:0]   odc,
   output reg [6:0]    ostate
);

`include "sd_params.vh"
`include "sd_const.vh"
   
assign bram_rd_sd_clk = sd_clk;
assign bram_wr_sd_clk = sd_clk;
   
reg  sd_cmd_out;
reg  sd_cmd_oe;
wire sd_cmd = sd_cmd_i;
//assign         sd_cmd = sd_cmd_oe ? sd_cmd_out : 1'bZ;
assign sd_cmd_t = !sd_cmd_oe;
assign sd_cmd_o = sd_cmd_out;
   
// tristate data lines when card state is DISCONNECT
reg  [3:0] sd_dat_out;
reg  [3:0] sd_dat_oe;
wire [3:0] sd_dat = sd_dat_i;
   
/*
assign         sd_dat[3] = (sd_dat_oe[3] && card_state_s != CARD_DIS) ? sd_dat_out[3] : 1'bZ;
assign         sd_dat[2] = (sd_dat_oe[2] && card_state_s != CARD_DIS) ? sd_dat_out[2] : 1'bZ;
assign         sd_dat[1] = (sd_dat_oe[1] && card_state_s != CARD_DIS) ? sd_dat_out[1] : 1'bZ;
assign         sd_dat[0] = (sd_dat_oe[0] && card_state_s != CARD_DIS) ? sd_dat_out[0] : 1'bZ;
*/
assign sd_dat_o = sd_dat_out;
assign sd_dat_t = ~sd_dat_oe | {4{card_state_s == CARD_DIS}};
   
reg          sd_cmd_last;
reg          sd_dat_last;
reg  [46:0]  cmd_in_latch;
reg  [135:0] resp_out_latch;

reg  [6:0]   crc7_in;
reg  [6:0]   crc7_out;
   
reg  [512:0] data_out_reg_latch;
   
reg  [15:0]  crc16_out3, crc16_out2, crc16_out1, crc16_out0;
reg  [15:0]  crc16_in3, crc16_in2, crc16_in1, crc16_in0;
reg  [15:0]  crc16_check3, crc16_check2, crc16_check1, crc16_check0;

   
reg  [10:0]  idc;
//reg  [10:0]  odc;
   
reg  [6:0]  istate;
//reg  [6:0]  ostate;
      
parameter [6:0] ST_RESET          = 'd0,
                ST_RESET_WAIT     = 'd1,
                ST_IDLE           = 'd4,
               
                ST_CMD_CHECK      = 'd10,
                ST_CMD_READ       = 'd11,
                ST_RESP_PREAMBLE  = 'd14,
                ST_RESP_WRITE     = 'd15,
                ST_RESP_WRITE_END = 'd16,
               
                ST_DATA_READ      = 'd20,
                ST_DATA_READ_1    = 'd21,
                ST_DATA_READ_2    = 'd22,
                ST_DATA_READ_3    = 'd23,
               
                ST_DATA_TOKEN     = 'd24,
                ST_DATA_TOKEN_1   = 'd25,
                ST_DATA_TOKEN_2   = 'd26,
                ST_DATA_TOKEN_3   = 'd27,
               
                ST_DATA_WRITE     = 'd30,
                ST_DATA_WRITE_1   = 'd31,
                ST_DATA_WRITE_2   = 'd32,
                ST_DATA_WRITE_3   = 'd33,
               
                ST_LAST           = 'd127;

reg  [15:0] didc;
reg  [15:0] dodc;

reg  [6:0]  distate;
reg  [6:0]  dostate;
   
reg  [31:0] dout_buf;
wire [3:0]  dout_4bit   = data_out_src ? data_out_reg_latch[511:508] : dout_buf[31:28];
wire [0:0]  dout_1bit   = data_out_src ? data_out_reg_latch[511] : dout_buf[31];
   
reg         do_crc_token;
               
// synchronizers
wire        data_in_act_s, data_in_act_r;
wire        data_in_stop_s;
wire        data_in_another_s;
wire        resp_act_s, resp_act_r;
wire [3:0]  resp_type_s;
wire [9:0]  data_out_len_s;
wire        data_out_act_s, data_out_act_r;
wire        data_out_stop_s;
wire [3:0]  card_state_s;
wire        mode_4bit_s;
synch_3       a(data_in_act, data_in_act_s, sd_clk, data_in_act_r);
synch_3       i(data_in_stop, data_in_stop_s, sd_clk);
synch_3       j(data_in_another, data_in_another_s, sd_clk);
synch_3       b(resp_act, resp_act_s, sd_clk, resp_act_r);
synch_3 #(4)  c(resp_type, resp_type_s, sd_clk);
synch_3 #(10) d(data_out_len, data_out_len_s, sd_clk);
synch_3       e(data_out_act, data_out_act_s, sd_clk, data_out_act_r);
synch_3       f(data_out_stop, data_out_stop_s, sd_clk);
synch_3 #(4)  g(card_state, card_state_s, sd_clk);
synch_3       h(mode_4bit, mode_4bit_s, sd_clk);

always @(posedge sd_clk or negedge reset_n) begin

   if(~reset_n) begin
      istate <= ST_RESET;
      distate <= ST_RESET;
   end else begin
   
   sd_cmd_last <= sd_cmd;
   sd_dat_last <= sd_dat[0];
      
   // free running counter
   idc <= idc + 1'b1;
   
   //
   // command input FSM
   //
   case(istate)
   ST_RESET: begin
      idc <= 0;
      if(sd_cmd) istate <= ST_RESET_WAIT;
   end
   ST_RESET_WAIT: begin
      // spec mandates that hosts send at least 74 clocks before any commands
      // some hosts may send many more, some only a few (which violates spec)
      if (idc == 60) istate <= ST_IDLE;
   end
   ST_IDLE: begin
      // falling edge of CMD, and output state not driving
      if((~sd_cmd & sd_cmd_last) & ~sd_cmd_oe) begin //~resp_act_s) begin
         // start bit 0 of command
         
         cmd_in_latch <= 0;
         cmd_in_act <= 0;
         crc7_in <= 0;
         idc <= 0;
         istate <= ST_CMD_READ;
      end
   end
   ST_CMD_READ: begin
      // shift in command
      cmd_in_latch <= {cmd_in_latch[45:0], sd_cmd};
      // advance CRC over first 40 bits
      if(idc < 39) begin
         crc7_in <=    {   crc7_in[5], crc7_in[4], crc7_in[3], crc7_in[2] ^ crc7_in[6] ^ sd_cmd,
                     crc7_in[1], crc7_in[0], sd_cmd ^ crc7_in[6]   };   
      end
      // after last bit of CRC   
      if(idc == 45) begin
         istate <= ST_CMD_CHECK;
      end
   end
   ST_CMD_CHECK: begin
      // compare CRC7
      cmd_in_crc_good <= ( cmd_in_latch[6:0] == crc7_in );
      cmd_in <= {cmd_in_latch, sd_cmd};
      cmd_in_act <= cmd_in_latch[45];
      
      istate <= ST_IDLE;
   end
   ST_LAST: begin
   end
   default: istate <= ST_RESET;
   endcase
   
   //
   // data input FSM
   //
   didc <= didc + 1'b1;
   bram_wr_sd_wren <= 0;
   case(distate)
   ST_RESET: begin
      do_crc_token <= 0;
      distate <= ST_IDLE;
   end
   ST_IDLE: begin
      if(data_in_act_r | (data_in_another_s & ~data_in_stop_s)) begin
         data_in_busy <= 1;
         data_in_done <= 0;
         distate <= ST_DATA_READ;
      end
   end
   ST_DATA_READ: begin
      // falling edge of DAT0, and output state not driving
      if((~sd_dat[0] & sd_dat_last) & ~sd_dat_oe[0]) begin 
         // start bit 0 of data packet
         crc16_in3 <= 0;
         crc16_in2 <= 0;
         crc16_in1 <= 0;
         crc16_in0 <= 0;
         didc <= 0;
         bram_wr_sd_addr <= -1;
         distate <= ST_DATA_READ_1;
      end
      if(data_in_stop_s) begin
         data_in_done <= 1;
         distate <= ST_DATA_READ_3;
      end
   end
   ST_DATA_READ_1: begin
      // shift in data
      if(mode_4bit_s) begin
         bram_wr_sd_data <= {bram_wr_sd_data[27:0], sd_dat};
         crc16_in3 <= {crc16_in3[14:0], 1'b0} ^ ((sd_dat[3] ^ crc16_in3[15]) ? 16'h1021 : 16'h0);
         crc16_in2 <= {crc16_in2[14:0], 1'b0} ^ ((sd_dat[2] ^ crc16_in2[15]) ? 16'h1021 : 16'h0);
         crc16_in1 <= {crc16_in1[14:0], 1'b0} ^ ((sd_dat[1] ^ crc16_in1[15]) ? 16'h1021 : 16'h0);
         crc16_in0 <= {crc16_in0[14:0], 1'b0} ^ ((sd_dat[0] ^ crc16_in0[15]) ? 16'h1021 : 16'h0);
         
         if(didc[2:0] == 3'd0) bram_wr_sd_addr <= bram_wr_sd_addr + 1'b1;
         if(didc[2:0] == 3'd7) bram_wr_sd_wren <= 1;
      end else begin
         bram_wr_sd_data <= {bram_wr_sd_data[31:0], sd_dat[0]};
         crc16_in0 <= {crc16_in0[14:0], 1'b0} ^ ((sd_dat[0] ^ crc16_in0[15]) ? 16'h1021 : 16'h0);
         
         if(didc[4:0] == 5'd0) bram_wr_sd_addr <= bram_wr_sd_addr + 1'b1;
         if(didc[4:0] == 5'd31) bram_wr_sd_wren <= 1;
      end
      if((mode_4bit_s ? (didc+1) >> 1 : (didc+1) >> 3) == 512) begin
         // end, read crc
         didc <= 0;
         distate <= ST_DATA_READ_2;
      end
      if(data_in_stop_s) begin
         data_in_done <= 1;
         distate <= ST_DATA_READ_3;
      end
   end
   ST_DATA_READ_2: begin
      // shift in CRC16
      crc16_check3[15:0] <= {crc16_check3[14:0], sd_dat[3]};
      crc16_check2[15:0] <= {crc16_check2[14:0], sd_dat[2]};
      crc16_check1[15:0] <= {crc16_check1[14:0], sd_dat[1]};
      crc16_check0[15:0] <= {crc16_check0[14:0], sd_dat[0]};
      
      if(didc == 16 || data_in_stop_s) begin
         // end, including 1 stop bit
         data_in_done <= 1;
         if( {crc16_check3, crc16_check2, crc16_check1, crc16_check0} ==
            {crc16_in3, crc16_in2, crc16_in1, crc16_in0} ) begin
            data_in_crc_good <= 1;
         end else data_in_crc_good <= 0;
         didc <= 0;
         distate <= ST_DATA_READ_3;
         
         // if STOP sent, do not send CRC token, just busy signal   
         do_crc_token <= ~data_in_stop_s;
      end
   end
   ST_DATA_READ_3: begin
      // after telling link we're done, wait for it to respond
      if(~data_in_act_s) begin
         do_crc_token <= 0;
         data_in_busy <= 0;
         distate <= ST_IDLE;
      end
   end
   default: distate <= ST_RESET;
   endcase

   end
end


always @(negedge sd_clk or negedge reset_n) begin

   if(~reset_n) begin
      ostate <= ST_RESET;
      dostate <= ST_RESET;
   end else begin
   
   // free running counter
   odc <= odc + 1'b1;
   
   //
   // command output FSM
   //
   case(ostate)
   ST_RESET: begin
      resp_done <= 0;
      sd_cmd_oe <= 0;
      ostate <= ST_IDLE;
   end
   ST_IDLE: begin
      if(resp_act_r) begin
         resp_done <= 0;
         resp_out_latch <= resp_out;
         crc7_out <= 0;
         
         ostate <= ST_RESP_PREAMBLE;
      end
   end
   ST_RESP_PREAMBLE: begin
      // to meet exact 5 cycle requirement between command and response for CMD2/ACMD41
      odc <= 0;
      ostate <= ST_RESP_WRITE;
   end
   ST_RESP_WRITE: begin
      sd_cmd_oe <= 1;
      sd_cmd_out <= resp_out_latch[135];
      crc7_out <= {   crc7_out[5], crc7_out[4], crc7_out[3], crc7_out[2] ^ crc7_out[6] ^ resp_out_latch[135],
                  crc7_out[1], crc7_out[0], resp_out_latch[135] ^ crc7_out[6]   };   
      resp_out_latch <= {resp_out_latch[134:0], 1'b1};   
      case(resp_type_s)
      RESP_R1, RESP_R1B, RESP_R3, RESP_R6, RESP_R7: begin
         // 48 bit codes   
         if(resp_type_s == RESP_R3) begin
            // fix R3 CRC to 1's
            crc7_out <= 7'b1111111;
         end
         if(odc >= 40) begin
            crc7_out <= {crc7_out[5:0], 1'b1};
            sd_cmd_out <= crc7_out[6];
         end
         if(odc == 47) ostate <= ST_RESP_WRITE_END;
      end
      RESP_R2: begin
         // 136 bit codes
         // only CRC over the last 128 bits
         if(odc < 8) crc7_out <= 0;
         if(odc >= 128) begin
            crc7_out <= {crc7_out[5:0], 1'b1};
            sd_cmd_out <= crc7_out[6];
         end
         if(odc == 135) ostate <= ST_RESP_WRITE_END;
      end
      endcase
   end
   ST_RESP_WRITE_END: begin
      sd_cmd_oe <= 0;
      resp_done <= 1;
      ostate <= ST_IDLE;
   end
   default: ostate <= ST_RESET;
   endcase
   
   
   
   dodc <= dodc + 1'b1;
   bram_rd_sd_wren <= 0;
   
   //
   // data output FSM
   //
   case(dostate)
   ST_RESET: begin
      sd_dat_oe <= 4'b0;
      data_out_busy <= 0;
      data_out_done <= 0;
      dostate <= ST_IDLE;
   end
   ST_IDLE: begin
      dodc <= 0;
      sd_dat_oe <= 4'b0;
      if(data_out_act_r) begin
         // start sending data packet
         data_out_busy <= 1;
         data_out_done <= 0;
         bram_rd_sd_addr <= 0;
         dostate <= ST_DATA_WRITE;
      end else
      if(do_crc_token) begin
         // send data CRC token, busy signal for written blocks
         dostate <= ST_DATA_TOKEN;
      end
   end
   ST_DATA_WRITE: begin
      // delay start of output
      if(dodc == 0) begin // 0: no delay
         dodc <= 0;
         // enable which data lines will be driven
         sd_dat_oe <= mode_4bit_s ? 4'b1111 : 4'b0001;
         sd_dat_out <= 4'b0;
         dostate <= ST_DATA_WRITE_1;
      end
      crc16_out3 <= 16'h0;
      crc16_out2 <= 16'h0;
      crc16_out1 <= 16'h0;
      crc16_out0 <= 16'h0;
      
      // preload bram Q data
      dout_buf <= bram_rd_sd_q;
      data_out_reg_latch <= data_out_reg;
   end
   ST_DATA_WRITE_1: begin
      if(mode_4bit_s) begin
         sd_dat_out <= dout_4bit;
         crc16_out3 <= {crc16_out3[14:0], 1'b0} ^ ((dout_4bit[3] ^ crc16_out3[15]) ? 16'h1021 : 16'h0);
         crc16_out2 <= {crc16_out2[14:0], 1'b0} ^ ((dout_4bit[2] ^ crc16_out2[15]) ? 16'h1021 : 16'h0);
         crc16_out1 <= {crc16_out1[14:0], 1'b0} ^ ((dout_4bit[1] ^ crc16_out1[15]) ? 16'h1021 : 16'h0);
         crc16_out0 <= {crc16_out0[14:0], 1'b0} ^ ((dout_4bit[0] ^ crc16_out0[15]) ? 16'h1021 : 16'h0);
         
         data_out_reg_latch[511:0] <= {data_out_reg_latch[507:0], 4'b0};
         dout_buf[31:0] <= {dout_buf[27:0], 4'b0};
         
         // advance thru 32bit bram output word
         if(dodc[2:0] == 3'd0) bram_rd_sd_addr <= bram_rd_sd_addr + 1'b1;
         if(dodc[2:0] == 3'd7) dout_buf <= bram_rd_sd_q;
      end else begin
         sd_dat_out[0] <= dout_1bit;
         crc16_out0 <= {crc16_out0[14:0], 1'b0} ^ ((dout_1bit[0] ^ crc16_out0[15]) ? 16'h1021 : 16'h0);
         
         data_out_reg_latch[511:0] <= {data_out_reg_latch[510:0], 1'b0};
         dout_buf[31:0] <= {dout_buf[30:0], 1'b0};
         
         // advance thru 32bit bram output word
         if(dodc[4:0] == 5'd0) bram_rd_sd_addr <= bram_rd_sd_addr + 1'b1;
         if(dodc[4:0] == 5'd31) dout_buf <= bram_rd_sd_q;
      end
      
      if((mode_4bit_s ? (dodc+1) >> 1 : (dodc+1) >> 3) == data_out_len_s) begin
         // end, dump crc
         dodc <= 0;
         dostate <= ST_DATA_WRITE_2;
      end

      if(data_out_stop_s) begin
         sd_dat_out <= 4'hF;
         data_out_done <= 1;
         dostate <= ST_DATA_WRITE_3;
      end
   end
   ST_DATA_WRITE_2: begin
      sd_dat_out[3] <= crc16_out3[15];
      sd_dat_out[2] <= crc16_out2[15];
      sd_dat_out[1] <= crc16_out1[15];
      sd_dat_out[0] <= crc16_out0[15];
      
      crc16_out3[15:0] <= {crc16_out3[14:0], 1'b1};
      crc16_out2[15:0] <= {crc16_out2[14:0], 1'b1};
      crc16_out1[15:0] <= {crc16_out1[14:0], 1'b1};
      crc16_out0[15:0] <= {crc16_out0[14:0], 1'b1};
      
      if(dodc == 16 || data_out_stop_s) begin
         // end, including 1 stop bit
         data_out_done <= 1;
         dostate <= ST_DATA_WRITE_3;
      end
   end
   ST_DATA_WRITE_3: begin
      if(~data_out_act_s) begin
         // let link know we are able to detect ACT's next rising edge
         // due to extreme differences in clock rates
         data_out_busy <= 0;
         dostate <= ST_IDLE;
      end
   end
   
   ST_DATA_TOKEN: begin
      // send CRC token
      case(dodc)
      1: begin
         sd_dat_oe[0] <= 1; 
         sd_dat_out[0] <= 0; // start bit
      end
      2: sd_dat_out[0] <= ~data_in_crc_good;
      3: sd_dat_out[0] <= data_in_crc_good;
      4: sd_dat_out[0] <= ~data_in_crc_good;
      5: sd_dat_out[0] <= 1; // stop/end bit
      6: begin
         sd_dat_out[0] <= 0; // start bit of busy
         dostate <= ST_DATA_TOKEN_1;
      end
      endcase
   end
   ST_DATA_TOKEN_1: begin
      // busy signal until data sink deasserts ACT
      sd_dat_oe[0] <= 1;
      sd_dat_out[0] <= 0;
      if(~data_in_act_s) begin
         // drive stop bit high
         sd_dat_oe[0] <= 1;
         sd_dat_out[0] <= 1;
         dostate <= ST_DATA_TOKEN_2;
      end
   end
   ST_DATA_TOKEN_2: begin
      sd_dat_oe[0] <= 0;
      dostate <= ST_IDLE;
   end
   default: dostate <= ST_RESET;
   endcase
   
   // for snooping real cards
   // sd_dat_oe <= 4'b0;
   // sd_cmd_oe <= 1'b0;
   end
end


endmodule
