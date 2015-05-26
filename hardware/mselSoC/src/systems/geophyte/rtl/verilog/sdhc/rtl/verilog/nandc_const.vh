                // Page Read
parameter [7:0] FCMD_READ_0 = 8'h00,
                FCMD_READ_1 = 8'h30,

                // Page Program
                FCMD_PROG_0 = 8'h80,
                FCMD_PROG_1 = 8'h10,

                // Random Data Input
                FCMD_RAND_DI_0 = 8'h85,

                // Random Data Output
                FCMD_RAND_DO_0 = 8'h05,
                FCMD_RAND_DO_1 = 8'he0,

                // Multiplane Program
                FCMD_MP_PROG_0 = 8'h80,
                FCMD_MP_PROG_1 = 8'h11,
                FCMD_MP_PROG_2 = 8'h81,
                FCMD_MP_PROG_3 = 8'h10,

                // ONFI Multiplane Program
                FCMD_O_MP_PROG_0 = 8'h80,
                FCMD_O_MP_PROG_1 = 8'h11,
                FCMD_O_MP_PROG_2 = 8'h80,
                FCMD_O_MP_PROG_3 = 8'h10,

                // Page Reprogram
                FCMD_PAGE_RPROG_0 = 8'h8b,
                FCMD_PAGE_RPROG_1 = 8'h10,

                // Multiplane Page Reprogram
                FCMD_MP_PAGE_RPROG_0 = 8'h8b,
                FCMD_MP_PAGE_RPROG_1 = 8'h11,
                FCMD_MP_PAGE_RPROG_2 = 8'h8b,
                FCMD_MP_PAGE_RPROG_3 = 8'h10,

                // Block Erase
                FCMD_ERASE_0 = 8'h60,
                FCMD_ERASE_1 = 8'hd0,

                // Multiplane Block Erase
                FCMD_MP_ERASE_0 = 8'h60,
                FCMD_MP_ERASE_1 = 8'h60,
                FCMD_MP_ERASE_2 = 8'hd0,

                // ONFI Multiplane Block Erase
                FCMD_O_MP_ERASE_0 = 8'h60,
                FCMD_O_MP_ERASE_1 = 8'hd1,
                FCMD_O_MP_ERASE_2 = 8'h60,
                FCMD_O_MP_ERASE_3 = 8'hd0,

                // Copy Back Read
                FCMD_CB_READ_0 = 8'h00,
                FCMD_CB_READ_1 = 8'h35,

                // Copy Back Program
                FCMD_CB_PROG_0 = 8'h85,
                FCMD_CB_PROG_1 = 8'h10,

                // Multiplane Copy Back Program
                FCMD_MP_CB_PROG_0 = 8'h85,
                FCMD_MP_CB_PROG_1 = 8'h11,
                FCMD_MP_CB_PROG_2 = 8'h81,
                FCMD_MP_CB_PROG_3 = 8'h10,

                // ONFI Multiplane Copy Back Program
                FCMD_O_MP_CB_PROG_0 = 8'h85,
                FCMD_O_MP_CB_PROG_1 = 8'h11,
                FCMD_O_MP_CB_PROG_2 = 8'h85,
                FCMD_O_MP_CB_PROG_3 = 8'h10,

                // Special Read For Copy Back
                FCMD_SREAD_CB_0 = 8'h00,
                FCMD_SREAD_CB_1 = 8'h36,

                // Read EDC Status Register
                FCMD_READ_EDC_0 = 8'h7b,

                // Read Status Register
                FCMD_READ_STAT_0 = 8'h70,

                // Read Status Enhanced
                FCMD_READ_ESTAT_0 = 8'h78,

                // Reset
                FCMD_RESET_0 = 8'hff,

                // Read Cache
                FCMD_READ_CACHE_0 = 8'h31,

                // Read Cache Enhanced
                FCMD_READ_ECACHE_0 = 8'h00,
                FCMD_READ_ECACHE_1 = 8'h31,

                // Read Cache End
                FCMD_READ_CACHEE_0 = 8'h3f,

                // Cache Program (End)
                FCMD_PROG_CACHEE_0 = 8'h80,
                FCMD_PROG_CACHEE_1 = 8'h10,

                // Cache Program (Start) / (Continue)
                FCMD_PROG_CACHES_0 = 8'h80,
                FCMD_PROG_CACHES_1 = 8'h15,

                // Multiplane Cache Program (Start/Continue)
                FCMD_MP_PROG_CACHES_0 = 8'h80,
                FCMD_MP_PROG_CACHES_1 = 8'h11,
                FCMD_MP_PROG_CACHES_2 = 8'h81,
                FCMD_MP_PROG_CACHES_3 = 8'h15,

                // ONFI Multiplane Cache Program (Start/Continue)
                FCMD_O_MP_PROG_CACHES_0 = 8'h80,
                FCMD_O_MP_PROG_CACHES_1 = 8'h11,
                FCMD_O_MP_PROG_CACHES_2 = 8'h80,
                FCMD_O_MP_PROG_CACHES_3 = 8'h15,

                // Multiplane Cache Program (End)
                FCMD_MP_PROG_CACHEE_0 = 8'h80,
                FCMD_MP_PROG_CACHEE_1 = 8'h11,
                FCMD_MP_PROG_CACHEE_2 = 8'h81,
                FCMD_MP_PROG_CACHEE_3 = 8'h10,

                // ONFI Multiplane Cache Program (End)
                FCMD_O_MP_PROG_CACHEE_0 = 8'h80,
                FCMD_O_MP_PROG_CACHEE_1 = 8'h11,
                FCMD_O_MP_PROG_CACHEE_2 = 8'h80,
                FCMD_O_MP_PROG_CACHEE_3 = 8'h10,

                // Read ID
                FCMD_READ_ID_0 = 8'h90,

                // Read ID2
                FCMD_READ_ID2_0 = 8'h30, //8'h65, 8'h00
                FCMD_READ_ID2_1 = 8'h30,

                // Read ONFI Signature
                FCMD_READ_O_SIG_0 = 8'h90,

                // Read Parameter Page
                FCMD_READ_PARAM_0 = 8'hec,

                // One-time Programmable (OTP) Area Entry
                FCMD_OTP_ENTRY_0 = 8'h29; //8'h17, 8'h04, 8'h19       

parameter [2:0] WB_CTI_CLASSIC     = 'b000,
                WB_CTI_CONST_BURST = 'b001,
                WB_CTI_INCR_BURST  = 'b010,
                WB_CTI_EOB         = 'b111;

parameter [1:0] WB_BTE_LINEAR      = 'b00,
                WB_BTE_4BEAT       = 'b01,
                WB_BTE_8BEAT       = 'b10,
                WB_BTE_16BEAT      = 'b11;

`define FCOLUMNS 11
`define FPAGES   6
`define FBLOCKS  10
`define FALL     (`FCOLUMNS+`FPAGES+`FBLOCKS)
`define FCOLUMNS_AND_SPARE (2048+20)

function [7:0] addr_cycle;
input [1:0]  cycle;
input [31:0] addr;
begin
   addr_cycle = (cycle == 0) ? {addr[7:0]} :
                (cycle == 1) ? {4'h0, addr[11:8]} :
                (cycle == 2) ? {addr[19:12]} :
                (cycle == 3) ? {addr[27:20]} : 8'h0;
end
endfunction

function [`FBLOCKS-1:0] addr_block;
input [31:0] addr;
begin
   addr_block = addr[`FCOLUMNS+`FPAGES+`FBLOCKS-1:`FCOLUMNS+`FPAGES];
end
endfunction

function [`FPAGES-1:0] addr_page;
input [31:0] addr;
begin
   addr_page = addr[`FCOLUMNS+`FPAGES-1:`FCOLUMNS];
end
endfunction

function [`FCOLUMNS-1:0] addr_column;
input [31:0] addr;
begin
   addr_column = addr[`FCOLUMNS-1:0];
end
endfunction

function addr_page_end;
input [31:0] addr;
begin
   addr_page_end = addr[`FCOLUMNS:0] == {(`FCOLUMNS_AND_SPARE/4), 2'b00};
end
endfunction

function [7:0] data_byte;
input [1:0] i;
input [31:0] data;
begin
   data_byte = (i == 0) ? data[7:0] :
               (i == 1) ? data[15:8] :
               (i == 2) ? data[23:16] :
               (i == 3) ? data[31:24] : 0;
end
endfunction
