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
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    13:14:48 11/12/2014 
// Design Name: 
// Module Name:    orpsoc_top 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
//`define TAORP_DEBUG 1
`include "orpsoc-defines.v"

module orpsoc_top(
	input sys_clk_pad_i,
	input rst_n_pad_i,
	// SDHC
	input wire sd_clk,
	inout wire sd_cmd,
	inout wire [3:0] sd_dat,
	// UART
	input	uart0_srx_pad_i,
	output uart0_stx_pad_o,
	//input uart0_cts_pad_i,
	//output uart0_rts_pad_o,
	// NAND
	inout [7:0] nand_io,
	output nand_cle,
	output nand_ale,
	output nand_ce_n,
	output nand_we_n,
	output nand_re_n,
	output nand_wp_n,
	input nand_rb_n,
	// GPIO
	inout [6:0] gpio0_io
	);

wire sys_rst;

assign sys_rst = ~rst_n_pad_i;

////////////////////////////////////////////////////////////////////////
//
// Clock and reset generation module
//
////////////////////////////////////////////////////////////////////////

wire wb_clk, wb_clk2x, wb_rst;

clkgen clkgen0(
	.sys_clk_i(sys_clk_pad_i),
	.sys_rst_i(sys_rst),
	.wb_clk_o(wb_clk),
	.wb_clk2x_o(wb_clk2x),
	.wb_rst_o(wb_rst)
);

////////////////////////////////////////////////////////////////////////
//
// Modules interconnections
//
////////////////////////////////////////////////////////////////////////

`include "wb_intercon.vh"

////////////////////////////////////////////////////////////////////////
//
// ARTIX7 JTAG TAP
//
////////////////////////////////////////////////////////////////////////
wire jtag_capture_o, jtag_drck_o, jtag_reset_o, jtag_runtest_o;
wire jtag_sel_o, jtag_shift_o, jtag_tck_o, jtag_tdi_o, jtag_tms_o;
wire jtag_update_o, jtag_tdo_i;

wire jtag_tck_unbuf;

wire jtag_pause_o;
assign jtag_pause_o = 1'b0;

BSCANE2 #(
	.JTAG_CHAIN(1)  // Value for USER command. Possible values: 1-4.
)
BSCANE2_inst (
	.CAPTURE(jtag_capture_o),
	// 1-bit output: CAPTURE output from TAP controller.
	.DRCK(jtag_drck_o),
	// 1-bit output: Gated TCK output. When SEL is asserted,
	// DRCK toggles when CAPTURE or SHIFT are asserted.
	.RESET(jtag_reset_o),
	// 1-bit output: Reset output for TAP controller.
	.RUNTEST(jtag_runtest_o),
	// 1-bit output: Output asserted when TAP controller is in Run Test/Idle state.
	.SEL(jtag_sel_o),
	// 1-bit output: USER instruction active output.
	.SHIFT(jtag_shift_o),
	// 1-bit output: SHIFT output from TAP controller.
	.TCK(jtag_tck_unbuf),
	// 1-bit output: Test Clock output. Fabric connection to TAP Clock pin.
	.TDI(jtag_tdi_o),
	// 1-bit output: Test Data Input (TDI) output from TAP controller.
	.TMS(jtag_tms_o),
	// 1-bit output: Test Mode Select output. Fabric connection to TAP.
	.UPDATE(jtag_update_o),
	//.UPDATE(update_bscan),
	// 1-bit output: UPDATE output from TAP controller
	.TDO(jtag_tdo_i)
	// 1-bit input: Test Data Output (TDO) input for USER function.
);

BUFG jtag_tck_bufg (
	.O(jtag_tck_o),
	.I(jtag_tck_unbuf)
);

////////////////////////////////////////////////////////////////////////
//
// OR1K CPU
//
////////////////////////////////////////////////////////////////////////

`ifdef ORP_DEBUG
localparam reset_pc = 32'hf0000100;
`else
localparam reset_pc = 32'h00100100;
`endif

`ifdef MOR1KX_CPU
wire    [31:0]  or1k_irq;
wire    or1k_rst;

wire    [31:0]  or1k_dbg_adr_i;
wire    [31:0]  or1k_dbg_dat_i;
wire	    or1k_dbg_stb_i;
wire	    or1k_dbg_we_i;
wire    [31:0]  or1k_dbg_dat_o;
wire	    or1k_dbg_ack_o;
wire	    or1k_dbg_stall_i;
wire	    or1k_dbg_bp_o;

mor1kx #(
	.FEATURE_DEBUGUNIT("ENABLED"),
	.FEATURE_CMOV("ENABLED"),
	.FEATURE_INSTRUCTIONCACHE("ENABLED"),
	.OPTION_ICACHE_BLOCK_WIDTH(5),
	.OPTION_ICACHE_SET_WIDTH(8),
	.OPTION_ICACHE_WAYS(2),
	.OPTION_ICACHE_LIMIT_WIDTH(32),
	.FEATURE_IMMU("ENABLED"),
	.FEATURE_DATACACHE("ENABLED"),
	.OPTION_DCACHE_BLOCK_WIDTH(5),
	.OPTION_DCACHE_SET_WIDTH(8),
	.OPTION_DCACHE_WAYS(2),
	.OPTION_DCACHE_LIMIT_WIDTH(31),
	.FEATURE_DMMU("ENABLED"),
	.OPTION_PIC_TRIGGER("LATCHED_LEVEL"),

	.IBUS_WB_TYPE("B3_REGISTERED_FEEDBACK"),
	.DBUS_WB_TYPE("B3_REGISTERED_FEEDBACK"),
	.OPTION_CPU0("CAPPUCCINO"),
	.OPTION_RESET_PC(reset_pc)
) mor1kx0 (
	.iwbm_adr_o(wb_m2s_or1k_i_adr),
	.iwbm_stb_o(wb_m2s_or1k_i_stb),
	.iwbm_cyc_o(wb_m2s_or1k_i_cyc),
	.iwbm_sel_o(wb_m2s_or1k_i_sel),
	.iwbm_we_o (wb_m2s_or1k_i_we),
	.iwbm_cti_o(wb_m2s_or1k_i_cti),
	.iwbm_bte_o(wb_m2s_or1k_i_bte),
	.iwbm_dat_o(wb_m2s_or1k_i_dat),
	.dwbm_adr_o(wb_m2s_or1k_d_adr),
	.dwbm_stb_o(wb_m2s_or1k_d_stb),
	.dwbm_cyc_o(wb_m2s_or1k_d_cyc),
	.dwbm_sel_o(wb_m2s_or1k_d_sel),
	.dwbm_we_o (wb_m2s_or1k_d_we ),
	.dwbm_cti_o(wb_m2s_or1k_d_cti),
	.dwbm_bte_o(wb_m2s_or1k_d_bte),
	.dwbm_dat_o(wb_m2s_or1k_d_dat),

	.clk(wb_clk),
	.rst(wb_rst),

	.iwbm_err_i(wb_s2m_or1k_i_err),
	.iwbm_ack_i(wb_s2m_or1k_i_ack),
	.iwbm_dat_i(wb_s2m_or1k_i_dat),
	.iwbm_rty_i(wb_s2m_or1k_i_rty),

	.dwbm_err_i(wb_s2m_or1k_d_err),
	.dwbm_ack_i(wb_s2m_or1k_d_ack),
	.dwbm_dat_i(wb_s2m_or1k_d_dat),
	.dwbm_rty_i(wb_s2m_or1k_d_rty),

	.irq_i(or1k_irq),

	.du_addr_i(or1k_dbg_adr_i[15:0]),
	.du_stb_i(or1k_dbg_stb_i),
	.du_dat_i(or1k_dbg_dat_i),
	.du_we_i(or1k_dbg_we_i),
	.du_dat_o(or1k_dbg_dat_o),
	.du_ack_o(or1k_dbg_ack_o),
	.du_stall_i(or1k_dbg_stall_i),
	.du_stall_o(or1k_dbg_bp_o)
);
`endif // MOR1KX

`ifdef OR1200_CPU
wire [31:0] or1k_irq;

wire [31:0] or1k_dbg_dat_i;
wire [31:0] or1k_dbg_adr_i;
wire or1k_dbg_we_i;
wire or1k_dbg_stb_i;
wire or1k_dbg_ack_o;
wire [31:0] or1k_dbg_dat_o;

wire or1k_dbg_stall_i;
wire or1k_dbg_ewt_i;
wire [3:0] or1k_dbg_lss_o;
wire [1:0] or1k_dbg_is_o;
wire [10:0] or1k_dbg_wp_o;
wire or1k_dbg_bp_o;
wire or1k_dbg_rst;

wire or1k_rst;

assign or1k_rst = wb_rst | or1k_dbg_rst;

or1200_top #(
	.boot_adr(reset_pc)
) or1200_top0(
	// Instruction bus, clocks, reset
	.iwb_clk_i(wb_clk),
	.iwb_rst_i(wb_rst),
	.iwb_ack_i(wb_s2m_or1k_i_ack),
	.iwb_err_i(wb_s2m_or1k_i_err),
	.iwb_rty_i(wb_s2m_or1k_i_rty),
	.iwb_dat_i(wb_s2m_or1k_i_dat),

	.iwb_cyc_o(wb_m2s_or1k_i_cyc),
	.iwb_adr_o(wb_m2s_or1k_i_adr),
	.iwb_stb_o(wb_m2s_or1k_i_stb),
	.iwb_we_o(wb_m2s_or1k_i_we),
	.iwb_sel_o(wb_m2s_or1k_i_sel),
	.iwb_dat_o(wb_m2s_or1k_i_dat),
	.iwb_cti_o(wb_m2s_or1k_i_cti),
	.iwb_bte_o(wb_m2s_or1k_i_bte),
	// Data bus, clocks, reset
	.dwb_clk_i(wb_clk),
	.dwb_rst_i(wb_rst),
	.dwb_ack_i(wb_s2m_or1k_d_ack),
	.dwb_err_i(wb_s2m_or1k_d_err),
	.dwb_rty_i(wb_s2m_or1k_d_rty),
	.dwb_dat_i(wb_s2m_or1k_d_dat),

	.dwb_cyc_o(wb_m2s_or1k_d_cyc),
	.dwb_adr_o(wb_m2s_or1k_d_adr),
	.dwb_stb_o(wb_m2s_or1k_d_stb),
	.dwb_we_o(wb_m2s_or1k_d_we),
	.dwb_sel_o(wb_m2s_or1k_d_sel),
	.dwb_dat_o(wb_m2s_or1k_d_dat),
	.dwb_cti_o(wb_m2s_or1k_d_cti),
	.dwb_bte_o(wb_m2s_or1k_d_bte),

	// Debug interface ports
	.dbg_stall_i(or1k_dbg_stall_i),
	.dbg_ewt_i(1'b0),
	.dbg_lss_o(or1k_dbg_lss_o),
	.dbg_is_o(or1k_dbg_is_o),
	.dbg_wp_o(or1k_dbg_wp_o),
	.dbg_bp_o(or1k_dbg_bp_o),

	.dbg_adr_i(or1k_dbg_adr_i),
	.dbg_we_i(or1k_dbg_we_i),
	.dbg_stb_i(or1k_dbg_stb_i),
	.dbg_dat_i(or1k_dbg_dat_i),
	.dbg_dat_o(or1k_dbg_dat_o),
	.dbg_ack_o(or1k_dbg_ack_o),

	.pm_clksd_o(),
	.pm_dc_gate_o(),
	.pm_ic_gate_o(),
	.pm_dmmu_gate_o(),
	.pm_immu_gate_o(),
	.pm_tt_gate_o(),
	.pm_cpu_gate_o(),
	.pm_wakeup_o(),
	.pm_lvolt_o(),

	.clk_i(wb_clk),
	.rst_i(or1k_rst),

	.clmode_i(2'b00),

	// Interrupts
	.pic_ints_i(or1k_irq),
	.sig_tick(),

	.pm_cpustall_i(1'b0)
);
`endif // OR1200_CPU

////////////////////////////////////////////////////////////////////////
//
// Debug Interface
//
////////////////////////////////////////////////////////////////////////

adbg_top dbg_if0(
	// OR1K interface
	.cpu0_clk_i     (wb_clk),
	.cpu0_rst_o     (or1k_dbg_rst),
	.cpu0_addr_o    (or1k_dbg_adr_i),
	.cpu0_data_o    (or1k_dbg_dat_i),
	.cpu0_stb_o     (or1k_dbg_stb_i),
	.cpu0_we_o      (or1k_dbg_we_i),
	.cpu0_data_i    (or1k_dbg_dat_o),
	.cpu0_ack_i     (or1k_dbg_ack_o),
	.cpu0_stall_o   (or1k_dbg_stall_i),
	.cpu0_bp_i      (or1k_dbg_bp_o),

	// TAP interface
	.tck_i(jtag_tck_o),
	.tdi_i(jtag_tdi_o),
	.tdo_o(jtag_tdo_i),
	//.rst_i(wb_rst),
	.rst_i(jtag_reset_o),
	.capture_dr_i(jtag_capture_o),
	.shift_dr_i(jtag_shift_o),
	.pause_dr_i(jtag_pause_o),
	.update_dr_i(jtag_update_o),
	.debug_select_i(jtag_sel_o),

	// Wishbone debug master
	.wb_rst_i(wb_rst),
	.wb_clk_i(wb_clk),
	.wb_dat_i(wb_s2m_dbg_dat),
	.wb_ack_i(wb_s2m_dbg_ack),
	.wb_err_i(wb_s2m_dbg_err),

	.wb_adr_o(wb_m2s_dbg_adr),
	.wb_dat_o(wb_m2s_dbg_dat),
	.wb_cyc_o(wb_m2s_dbg_cyc),
	.wb_stb_o(wb_m2s_dbg_stb),
	.wb_sel_o(wb_m2s_dbg_sel),
	.wb_we_o(wb_m2s_dbg_we),
	.wb_cti_o(wb_m2s_dbg_cti),
	.wb_bte_o(wb_m2s_dbg_bte)
);

////////////////////////////////////////////////////////////////////////
//
// ROM
//
////////////////////////////////////////////////////////////////////////
localparam ROM0_SIZE = (64 * 1024);

`ifndef ORP_DEBUG

wb_rom #(
	.depth(ROM0_SIZE),
	.memfile("rom.dat")
) rom0 (
	.wb_clk(wb_clk),
	.wb_rst(wb_rst),
	.wb_adr_i(wb_m2s_rom0_adr),
	.wb_cyc_i(wb_m2s_rom0_cyc),
	.wb_stb_i(wb_m2s_rom0_stb),
	.wb_cti_i(wb_m2s_rom0_cti),
	.wb_bte_i(wb_m2s_rom0_bte),
	.wb_dat_o(wb_s2m_rom0_dat),
	.wb_ack_o(wb_s2m_rom0_ack),
	.wb_rty_o(wb_s2m_rom0_rty),
	.wb_err_o(wb_s2m_rom0_err)
);

`else

wb_ram #(
	.depth(ROM0_SIZE)
) rom0 (
	// Wishbone slave interface
	.wb_dat_i(wb_m2s_rom0_dat),
	.wb_adr_i(wb_m2s_rom0_adr[$clog2(ROM0_SIZE) - 1:0]),
	.wb_sel_i(wb_m2s_rom0_sel),
	.wb_cti_i(wb_m2s_rom0_cti),
	.wb_bte_i(wb_m2s_rom0_bte),
	.wb_we_i(wb_m2s_rom0_we),
	.wb_cyc_i(wb_m2s_rom0_cyc),
	.wb_stb_i(wb_m2s_rom0_stb),
	.wb_dat_o(wb_s2m_rom0_dat),
	.wb_ack_o(wb_s2m_rom0_ack),
	.wb_err_o(wb_s2m_rom0_err),
	// Clock, reset
	.wb_clk_i(wb_clk),
	.wb_rst_i(wb_rst)
);

assign wb_s2m_rom0_rty = 1'b0;
`endif

////////////////////////////////////////////////////////////////////////
//
// RAM
//
////////////////////////////////////////////////////////////////////////
localparam RAM0_SIZE = (128 * 1024);
 
wb_ram #(
	.depth(RAM0_SIZE)
) ram0 (
	// Wishbone slave interface
	.wb_dat_i(wb_m2s_ram0_dat),
	.wb_adr_i(wb_m2s_ram0_adr[$clog2(RAM0_SIZE) - 1:0]),
	.wb_sel_i(wb_m2s_ram0_sel),
	.wb_cti_i(wb_m2s_ram0_cti),
	.wb_bte_i(wb_m2s_ram0_bte),
	.wb_we_i(wb_m2s_ram0_we),
	.wb_cyc_i(wb_m2s_ram0_cyc),
	.wb_stb_i(wb_m2s_ram0_stb),
	.wb_dat_o(wb_s2m_ram0_dat),
	.wb_ack_o(wb_s2m_ram0_ack),
	.wb_err_o(wb_s2m_ram0_err),
	// Clock, reset
	.wb_clk_i(wb_clk),
	.wb_rst_i(wb_rst)
);

assign wb_s2m_ram0_rty = 1'b0;

////////////////////////////////////////////////////////////////////////
//
// UART0
//
////////////////////////////////////////////////////////////////////////

parameter uart0_aw = 3;

wire uart0_irq;

assign wb_s2m_uart0_err = 0;
assign wb_s2m_uart0_rty = 0;

uart_top uart16550_0 (
	// Wishbone slave interface
	.wb_clk_i       (wb_clk),
	.wb_rst_i       (wb_rst),
	.wb_adr_i       (wb_m2s_uart0_adr[uart0_aw-1:0]),
	.wb_dat_i       (wb_m2s_uart0_dat),
	.wb_we_i	(wb_m2s_uart0_we),
	.wb_stb_i       (wb_m2s_uart0_stb),
	.wb_cyc_i       (wb_m2s_uart0_cyc),
	.wb_sel_i       (4'b0), // Not used in 8-bit mode
	.wb_dat_o       (wb_s2m_uart0_dat),
	.wb_ack_o       (wb_s2m_uart0_ack),

	// Outputs
	.int_o(uart0_irq),
	.stx_pad_o      (uart0_stx_pad_o),
	.rts_pad_o      (),
	.dtr_pad_o      (),

	// Inputs
	.srx_pad_i      (uart0_srx_pad_i),
	.cts_pad_i      (1'b0),
	.dsr_pad_i      (1'b0),
	.ri_pad_i       (1'b0),
	.dcd_pad_i      (1'b0)
);

////////////////////////////////////////////////////////////////////////
//
// GPIO0
//
////////////////////////////////////////////////////////////////////////

wire [7:0]	gpio0_in;
wire [7:0]	gpio0_out;
wire [7:0]	gpio0_dir;

// Tristate logic for IO
// 0 = input, 1 = output
genvar i;
generate
for (i = 0; i < 7; i = i+1)
begin: gpio0_tris
	assign gpio0_io[i] = gpio0_dir[i] ? gpio0_out[i] : 1'bz;
	assign gpio0_in[i] = gpio0_dir[i] ? gpio0_out[i] : gpio0_io[i];
end
endgenerate

assign gpio0_in[7] = 1'b0;

gpio gpio0(
	// GPIO bus
	.gpio_i(gpio0_in),
	.gpio_o(gpio0_out),
	.gpio_dir_o(gpio0_dir),
	// Wishbone slave interface
	.wb_adr_i(wb_m2s_gpio0_adr[0]),
	.wb_dat_i(wb_m2s_gpio0_dat),
	.wb_we_i(wb_m2s_gpio0_we),
	.wb_cyc_i(wb_m2s_gpio0_cyc),
	.wb_stb_i(wb_m2s_gpio0_stb),
	.wb_cti_i(wb_m2s_gpio0_cti),
	.wb_bte_i(wb_m2s_gpio0_bte),
	.wb_dat_o(wb_s2m_gpio0_dat),
	.wb_ack_o(wb_s2m_gpio0_ack),
	.wb_err_o(wb_s2m_gpio0_err),
	.wb_rty_o(wb_s2m_gpio0_rty),

	.wb_clk(wb_clk),
	.wb_rst(wb_rst)
);

////////////////////////////////////////////////////////////////////////
//
// TRNG
//
////////////////////////////////////////////////////////////////////////

wb_trng trng0 (
	// Wishbone slave interface
	.wb_adr_i(wb_m2s_trng_adr),
	.wb_dat_i(wb_m2s_trng_dat),
	.wb_we_i(wb_m2s_trng_we),
	.wb_cyc_i(wb_m2s_trng_cyc),
	.wb_stb_i(wb_m2s_trng_stb),
	.wb_cti_i(wb_m2s_trng_cti),
	.wb_bte_i(wb_m2s_trng_bte),
	.wb_dat_o(wb_s2m_trng_dat),
	.wb_ack_o(wb_s2m_trng_ack),
	.wb_err_o(wb_s2m_trng_err),
	.wb_rty_o(wb_s2m_trng_rty),

	.wb_clk(wb_clk),
	.wb_rst(wb_rst)
);

////////////////////////////////////////////////////////////////////////
//
// CRYPTO_AES
//
////////////////////////////////////////////////////////////////////////

crypto_aes_top crypto_aes (
	.wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),

        .core_clk(wb_clk2x),

	.wb_adr_i(wb_m2s_crypto_aes_adr),
	.wb_dat_i(wb_m2s_crypto_aes_dat),
	.wb_sel_i(wb_m2s_crypto_aes_sel),
	.wb_cti_i(wb_m2s_crypto_aes_cti),
	.wb_bte_i(wb_m2s_crypto_aes_bte),
	.wb_we_i(wb_m2s_crypto_aes_we),
	.wb_cyc_i(wb_m2s_crypto_aes_cyc),
	.wb_stb_i(wb_m2s_crypto_aes_stb),
	.wb_dat_o(wb_s2m_crypto_aes_dat),
	.wb_ack_o(wb_s2m_crypto_aes_ack),
	.wb_err_o(wb_s2m_crypto_aes_err),
	.wb_rty_o(wb_s2m_crypto_aes_rty)
);

////////////////////////////////////////////////////////////////////////
//
// CRYPTO_SHA256
//
////////////////////////////////////////////////////////////////////////

crypto_sha256_top crypto_sha256 (
	.wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),

        .core_clk(wb_clk2x),

	.wb_adr_i(wb_m2s_crypto_sha256_adr),
	.wb_dat_i(wb_m2s_crypto_sha256_dat),
	.wb_sel_i(wb_m2s_crypto_sha256_sel),
	.wb_cti_i(wb_m2s_crypto_sha256_cti),
	.wb_bte_i(wb_m2s_crypto_sha256_bte),
	.wb_we_i(wb_m2s_crypto_sha256_we),
	.wb_cyc_i(wb_m2s_crypto_sha256_cyc),
	.wb_stb_i(wb_m2s_crypto_sha256_stb),
	.wb_dat_o(wb_s2m_crypto_sha256_dat),
	.wb_ack_o(wb_s2m_crypto_sha256_ack),
	.wb_err_o(wb_s2m_crypto_sha256_err),
	.wb_rty_o(wb_s2m_crypto_sha256_rty)
);

////////////////////////////////////////////////////////////////////////
//
// BOOTROM0
//
////////////////////////////////////////////////////////////////////////
localparam bootrom0_aw = 6;

assign  wb_s2m_bootrom0_err = 1'b0;
assign  wb_s2m_bootrom0_rty = 1'b0;

bootrom #(
	.addr_width(bootrom0_aw)
) bootrom0 (
	.wb_clk(wb_clk),
	.wb_rst(wb_rst),
	.wb_adr_i(wb_m2s_bootrom0_adr[(bootrom0_aw + 2) - 1 : 2]),
	.wb_cyc_i(wb_m2s_bootrom0_cyc),
	.wb_stb_i(wb_m2s_bootrom0_stb),
	.wb_cti_i(wb_m2s_bootrom0_cti),
	.wb_bte_i(wb_m2s_bootrom0_bte),
	.wb_dat_o(wb_s2m_bootrom0_dat),
	.wb_ack_o(wb_s2m_bootrom0_ack)
);

////////////////////////////////////////////////////////////////////////
//
// SDHC 
//
////////////////////////////////////////////////////////////////////////
wire sd_clk_buf, wb_sdhc_clk;

IBUFG sd_clk_IBUFG (
    .I ( sd_clk ),
    .O ( sd_clk_buf )
);
wire sd_cmd_i, sd_cmd_o, sd_cmd_t;
IOBUF sd_cmd_IOBUF (
    .IO ( sd_cmd ),
    .I  ( sd_cmd_o ),
    .O  ( sd_cmd_i ),
    .T  ( sd_cmd_t )
);
wire [3:0] sd_dat_i, sd_dat_o, sd_dat_t;
generate for(i = 0; i < 4; i = i + 1) begin : sd_dat_IOBUF
    IOBUF sd_dat_IOBUF (
        .IO ( sd_dat[i] ),
        .I  ( sd_dat_o[i] ),
        .O  ( sd_dat_i[i] ),
        .T  ( sd_dat_t[i] )
    );
end endgenerate

wire [31:0] wb_m2s_sdhc_adr;
wire [31:0] wb_m2s_sdhc_dat;
wire  [3:0] wb_m2s_sdhc_sel;
wire        wb_m2s_sdhc_we;
wire        wb_m2s_sdhc_cyc;
wire        wb_m2s_sdhc_stb;
wire  [2:0] wb_m2s_sdhc_cti;
wire  [1:0] wb_m2s_sdhc_bte;
wire [31:0] wb_s2m_sdhc_dat;
wire        wb_s2m_sdhc_ack;
wire        wb_s2m_sdhc_err;
wire        wb_s2m_sdhc_rty;

wire [31:0] wb_s2m_sdhc_dat_fauxfs, wb_s2m_sdhc_dat_ftl;
wire        wb_s2m_sdhc_ack_fauxfs, wb_s2m_sdhc_ack_fauxfs;
assign wb_s2m_sdhc_dat = wb_s2m_sdhc_dat_fauxfs | wb_s2m_sdhc_dat_ftl;
assign wb_s2m_sdhc_ack = wb_s2m_sdhc_ack_fauxfs | wb_s2m_sdhc_ack_ftl;

sd_top sd_top (
    .clk_50        ( wb_clk ),
    .reset_n       ( ~wb_rst ),

    .sd_clk        ( sd_clk_buf ),
    .sd_cmd_i      ( sd_cmd_i ),
    .sd_cmd_o      ( sd_cmd_o ),
    .sd_cmd_t      ( sd_cmd_t ),
    .sd_dat_i      ( sd_dat_i ),
    .sd_dat_o      ( sd_dat_o ),
    .sd_dat_t      ( sd_dat_t ),

    .wbm_clk_o     ( wb_sdhc_clk ),
    .wbm_adr_o     ( wb_m2s_sdhc_adr ),
    .wbm_dat_o     ( wb_m2s_sdhc_dat ),
    .wbm_sel_o     ( wb_m2s_sdhc_sel ),
    .wbm_cyc_o     ( wb_m2s_sdhc_cyc ),
    .wbm_stb_o     ( wb_m2s_sdhc_stb ),
    .wbm_we_o      ( wb_m2s_sdhc_we ),
    .wbm_cti_o     ( wb_m2s_sdhc_cti ),
    .wbm_bte_o     ( wb_m2s_sdhc_bte ),
    .wbm_dat_i     ( wb_s2m_sdhc_dat ),
    .wbm_ack_i     ( wb_s2m_sdhc_ack ),

    .opt_enable_hs ( 1 )
);

////////////////////////////////////////////////////////////////////////
//
// FAUXFS
//
////////////////////////////////////////////////////////////////////////
wire rfile_ack_irq, wfile_dat_irq;

fauxfs_top (
	.wb_cpu_clk_i(wb_clk),
	.wb_cpu_rst_i(wb_rst),
	.wb_cpu_adr_i(wb_m2s_fauxfs_cpu_adr),
	.wb_cpu_dat_i(wb_m2s_fauxfs_cpu_dat),
	.wb_cpu_sel_i(wb_m2s_fauxfs_cpu_sel),
	.wb_cpu_cti_i(wb_m2s_fauxfs_cpu_cti),
	.wb_cpu_bte_i(wb_m2s_fauxfs_cpu_bti),
	.wb_cpu_we_i(wb_m2s_fauxfs_cpu_we),
	.wb_cpu_cyc_i(wb_m2s_fauxfs_cpu_cyc),
	.wb_cpu_stb_i(wb_m2s_fauxfs_cpu_stb),
	.wb_cpu_dat_o(wb_s2m_fauxfs_cpu_dat),
	.wb_cpu_ack_o(wb_s2m_fauxfs_cpu_ack),
	.wb_cpu_err_o(wb_s2m_fauxfs_cpu_err),
	.wb_cpu_rty_o(wb_s2m_fauxfs_cpu_rty),

	.wfile_dat_int_o(wfile_dat_irq),
	.rfile_ack_int_o(rfile_ack_irq),
	
	.wb_sdhc_clk_i(wb_sdhc_clk),
	.wb_sdhc_rst_i(wb_rst),
	.wb_sdhc_adr_i(wb_m2s_sdhc_adr),
	.wb_sdhc_dat_i(wb_m2s_sdhc_dat),
	.wb_sdhc_sel_i(wb_m2s_sdhc_sel),
	.wb_sdhc_cti_i(wb_m2s_shdc_cti),
	.wb_sdhc_bte_i(wb_m2s_sdhc_bti),
	.wb_sdhc_we_i(wb_m2s_sdhc_we),
	.wb_sdhc_cyc_i(wb_m2s_sdhc_cyc),
	.wb_sdhc_stb_i(wb_m2s_sdhc_stb),
	.wb_sdhc_dat_o(wb_s2m_sdhc_dat_fauxfs),
	.wb_sdhc_ack_o(wb_s2m_sdhc_ack_fauxfs),
	.wb_sdhc_err_o(wb_s2m_sdhc_err),
	.wb_sdhc_rty_o(wb_s2m_sdhc_rty)
);

////////////////////////////////////////////////////////////////////////
//
// FTL
//
////////////////////////////////////////////////////////////////////////

wire [9:0]  dbg_phy_num_valid_blocks;
wire        dbg_phy_rebuilt_badblock;
wire        dbg_phy_remapped_runtime;
wire        err_phy_out_of_extras;

wire [2:0]  wb_m2s_ftl_cti;
wire [1:0]  wb_m2s_ftl_bte;
wire [31:0] wb_m2s_ftl_adr;
wire [31:0] wb_m2s_ftl_dat;
wire [3:0]  wb_m2s_ftl_sel;
wire        wb_m2s_ftl_cyc;
wire        wb_m2s_ftl_stb;
wire        wb_m2s_ftl_we;
wire [31:0] wb_s2m_ftl_dat;
wire        wb_s2m_ftl_ack;

ftl_top ftl_top (
    .clk_50    ( wb_sdhc_clk ),
    .reset_n   ( ~wb_rst ),
    .dbg_phy_num_valid_blocks ( dbg_phy_num_valid_blocks ),
    .dbg_phy_rebuilt_badblock ( dbg_phy_rebuilt_badblock ),
    .dbg_phy_remapped_runtime ( dbg_phy_remapped_runtime ),
    .err_phy_out_of_extras    ( err_phy_out_of_extras ),

    .wbs_clk_i ( wb_sdhc_clk ),
    .wbs_adr_i ( wb_m2s_sdhc_adr ),
    .wbs_dat_i ( wb_m2s_sdhc_dat ),
    .wbs_sel_i ( wb_m2s_sdhc_sel ),
    .wbs_cyc_i ( wb_m2s_sdhc_cyc ),
    .wbs_stb_i ( wb_m2s_sdhc_stb ),
    .wbs_we_i  ( wb_m2s_sdhc_we ),
    .wbs_dat_o ( wb_s2m_sdhc_dat_ftl ),
    .wbs_ack_o ( wb_s2m_sdhc_ack_ftl ),

    .wbm_cti_o ( wb_m2s_ftl_cti ),
    .wbm_bte_o ( wb_m2s_ftl_bte ),
    .wbm_adr_o ( wb_m2s_ftl_adr ),
    .wbm_dat_o ( wb_m2s_ftl_dat ),
    .wbm_sel_o ( wb_m2s_ftl_sel ),
    .wbm_cyc_o ( wb_m2s_ftl_cyc ),
    .wbm_stb_o ( wb_m2s_ftl_stb ),
    .wbm_we_o  ( wb_m2s_ftl_we ),
    .wbm_dat_i ( wb_s2m_ftl_dat ),
    .wbm_ack_i ( wb_s2m_ftl_ack )
);

////////////////////////////////////////////////////////////////////////
//
// NANDC
//
////////////////////////////////////////////////////////////////////////

wire [7:0] nand_io_i, nand_io_o, nand_io_t;
generate for(i = 0; i < 8; i = i + 1) begin : nand_io_IOBUF
   IOBUF nand_io_IOBUF (
      .IO(nand_io[i]),
      .I(nand_io_o[i]),
      .O(nand_io_i[i]),
      .T(nand_io_t[i])
   );
end endgenerate

nandc_ecc_dual_master #(
    .WB0_FLASH_S ( 0 ),
    .WB0_FLASH_N ( 1020 ),
    .WB1_FLASH_S ( 1020 ),
    .WB1_FLASH_N ( 4 )
) nandc_ecc (
    .wb_clk     ( wb_sdhc_clk ),
    .wb_rst     ( wb_rst ),

    .wbs0_cti_i ( wb_m2s_ftl_cti ),
    .wbs0_bte_i ( wb_m2s_ftl_bte ),
    .wbs0_adr_i ( wb_m2s_ftl_adr ),
    .wbs0_dat_i ( wb_m2s_ftl_dat ),
    .wbs0_sel_i ( wb_m2s_ftl_sel ),
    .wbs0_cyc_i ( wb_m2s_ftl_cyc ),
    .wbs0_stb_i ( wb_m2s_ftl_stb ),
    .wbs0_we_i  ( wb_m2s_ftl_we ),
    .wbs0_dat_o ( wb_s2m_ftl_dat ),
    .wbs0_ack_o ( wb_s2m_ftl_ack ),

    .wbs1_cti_i ( wb_m2s_nand_cpu_cti ),
    .wbs1_bte_i ( wb_m2s_nand_cpu_bte ),
    .wbs1_adr_i ( wb_m2s_nand_cpu_adr ),
    .wbs1_dat_i ( wb_m2s_nand_cpu_dat ),
    .wbs1_sel_i ( wb_m2s_nand_cpu_sel ),
    .wbs1_cyc_i ( wb_m2s_nand_cpu_cyc ),
    .wbs1_stb_i ( wb_m2s_nand_cpu_stb ),
    .wbs1_we_i  ( wb_m2s_nand_cpu_we ),
    .wbs1_dat_o ( wb_s2m_nand_cpu_dat ),
    .wbs1_ack_o ( wb_s2m_nand_cpu_ack ),

    .IO_i       ( nand_io_i ),
    .IO_o       ( nand_io_o ),
    .IO_t       ( nand_io_t ),
    .CLE        ( nand_cle ),
    .ALE        ( nand_ale ),
    .CE_n       ( nand_ce_n ),
    .WE_n       ( nand_we_n ),
    .RE_n       ( nand_re_n ),
    .WP_n       ( nand_wp_n ),
    .RB_n       ( nand_rb_n )
);

assign wb_m2s_nand_cpu_err = 0;
assign wb_m2s_nand_cpu_rty = 0;

////////////////////////////////////////////////////////////////////////
//
// Interrupt assignment
//
////////////////////////////////////////////////////////////////////////

assign or1k_irq[0] = 0; // Non-maskable inside OR1K
assign or1k_irq[1] = 0; // Non-maskable inside OR1K
assign or1k_irq[2] = uart0_irq;
assign or1k_irq[3] = 0;
assign or1k_irq[4] = 0;
assign or1k_irq[5] = 0;
assign or1k_irq[6] = 0;
assign or1k_irq[7] = 0;
assign or1k_irq[8] = 0;
assign or1k_irq[9] = 0;
assign or1k_irq[10] = 0;
assign or1k_irq[11] = 0;
assign or1k_irq[12] = 0;
assign or1k_irq[13] = 0;
assign or1k_irq[14] = 0;
assign or1k_irq[15] = 0;
assign or1k_irq[16] = wfile_dat_irq;
assign or1k_irq[17] = rfile_ack_irq;
assign or1k_irq[18] = 0;
assign or1k_irq[19] = 0;
assign or1k_irq[20] = 0;
assign or1k_irq[21] = 0;
assign or1k_irq[22] = 0;
assign or1k_irq[23] = 0;
assign or1k_irq[24] = 0;
assign or1k_irq[25] = 0;
assign or1k_irq[26] = 0;
assign or1k_irq[27] = 0;
assign or1k_irq[28] = 0;
assign or1k_irq[29] = 0;
assign or1k_irq[30] = 0;
assign or1k_irq[31] = 0;

endmodule
