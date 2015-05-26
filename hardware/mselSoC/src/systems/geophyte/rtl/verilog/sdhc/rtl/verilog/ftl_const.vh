parameter [2:0]  OP_PAGE_POKE       = 'd0,
                 OP_PAGE_READ       = 'd1,
                 OP_PAGE_READ_EMPTY = 'd2,
                 OP_PAGE_WRITE      = 'd3,
                 OP_PAGE_BLOCKERASE = 'd4;

// can't be increased without revamping NANDC and increasing >10bit addressing
parameter [10:0] NAND_DEVICE_NUM_BLOCKS  = 'd1024;
parameter [10:0] NAND_ALLOWED_PHY_BLOCKS = 'd1020;
parameter [10:0] NAND_ALLOWED_LOG_BLOCKS = 'd1000;

parameter [7:0]  NAND_PAGE_PER_BLOCK     = 'd64;
parameter [15:0] NAND_PAGE_SIZE          = 'd2048;
parameter [15:0] NAND_SPARE_SIZE         = 'd64;
parameter [20:0] NAND_BLOCK_SIZE         = NAND_PAGE_SIZE * NAND_PAGE_PER_BLOCK;

`include "nandc_def.vh"
`include "nandc_const.vh"

parameter [31:0] NANDC_REG_OFFSET   = `WB0_PAGE_OFFSET_BASEADDR,
                 NANDC_REG_SPARE_WR = `WB0_SPARE_SPACE_WR_BASEADDR,
                 NANDC_REG_SPARE_RD = `WB0_SPARE_SPACE_RD_BASEADDR,
                 NANDC_REG_ERASE    = `WB0_ERASE_BASEADDR,
                 NANDC_REG_STATUS   = `WB0_STATUS_BASEADDR;

