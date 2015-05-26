// if you want to change the card size, modify CSD_C_SIZE

parameter [1:0]  STAT_DAT_BUS_WIDTH_1   = 'b00;
parameter [1:0]  STAT_DAT_BUS_WIDTH_4   = 'b10;
parameter [0:0]  STAT_SECURED_MODE      = 'b0;         // Not secured
parameter [15:0] STAT_SD_CARD_TYPE      = 'h0000;      // pages 78-79 of SD Simplified Spec 3.01
parameter [31:0] STAT_SIZE_OF_PROT_AREA = 'h0;         // No protected area
parameter [7:0]  STAT_SPEED_CLASS       = 'h4;         // Class 10
parameter [7:0]  STAT_PERFORMANCE_MOVE  = 'h0;         // Ignored in Class 10
parameter [3:0]  STAT_AU_SIZE           = 'h1;         // 16KB Allocation units   
parameter [15:0] STAT_ERASE_SIZE        = 'h0;         // Erase timeout calculation not supported
parameter [5:0]  STAT_ERASE_TIMEOUT     = 'h0;         // Erase timeout calculation not supported
parameter [1:0]  STAT_ERASE_OFFSET      = 'h0;         // n/a

parameter [23:0] OCR_VOLTAGE_WINDOW     = 24'hFF8000;  // Table 5-1, works at 3.3v

parameter [7:0]  CID_FIELD_MID          = 8'h7F;       // CID Manufacturer ID
parameter [15:0] CID_FIELD_OID          = 16'h5043;    // OEM/Application ID ('PC')
parameter [39:0] CID_FIELD_PNM          = 40'h48454C4C4F; // Product Name ('HELLO')
parameter [7:0]  CID_FIELD_PRV          = 8'h10;       // Product Revision ('1.0')
parameter [31:0] CID_FIELD_PSN          = 32'h00000001;// Product Serial Number
parameter [11:0] CID_FIELD_MDT          = 12'h14F;     // Manufacturing date (2014 December)                                       
                                                
parameter [1:0]  CSD_CSD_STRUCTURE      = 'b01;        // Version 2.00/High Capacity
parameter [7:0]  CSD_TAAC               = 'h0E;        // data read access-time-1
parameter [7:0]  CSD_NSAC               = 'h00;        // data read access-time-2 in CLK cycles (NSAC*100)
parameter [7:0]  CSD_TRAN_SPEED_25      = 'h32;        // max data transfer rate   (25mhz)
parameter [7:0]  CSD_TRAN_SPEED_50      = 'h5A;        // max data transfer rate   (50mhz)
parameter [11:0] CSD_CCC                = 'b01_1_110110101; // card command classes
parameter [3:0]  CSD_READ_BL_LEN        = 'h9;         // max. read data block length   (512 bytes)
parameter [0:0]  CSD_READ_BL_PARTIAL    = 'h0;         // partial blocks for read allowed
parameter [0:0]  CSD_WRITE_BLK_MISALIGN = 'h0;         // write block misalignment
parameter [0:0]  CSD_READ_BLK_MISALIGN  = 'h0;         // read block misalignment
parameter [0:0]  CSD_DSR_IMPL           = 'h0;         // DSR implemented
parameter [21:0] CSD_C_SIZE             = 'd249;       // 1020 blocks of nand   
                                                       // device size (please see p.98 of Simplified Spec 2.00)
                                                       // memory capacity = (C_SIZE+1) * 512K byte
                                                       // 22'h1010 is ~2gb, minimal legal size for SDHC
                                                       // however any size is functional
parameter [31:0] SD_TOTAL_BLOCKS        = (CSD_C_SIZE+1) * 1024;
parameter [0:0]  CSD_ERASE_BLK_EN       = 'h1;         // erase single block enable
parameter [6:0]  CSD_SECTOR_SIZE        = 'h7F;        // erase sector size
parameter [6:0]  CSD_WP_GRP_SIZE        = 'h0;         // write protect group size
parameter [0:0]  CSD_WP_GRP_ENABLE      = 'h0;         // write protect group enable
parameter [2:0]  CSD_R2W_FACTOR         = 'b010;       // write speed factor
parameter [3:0]  CSD_WRITE_BL_LEN       = 'h9;         // max. write data block length
parameter [0:0]  CSD_WRITE_BL_PARTIAL   = 'h0;         // partial blocks for write allowed
parameter [0:0]  CSD_FILE_FORMAT_GRP    = 'h0;         // File format group
parameter [0:0]  CSD_COPY               = 'h0;         // copy flag (OTP)
parameter [0:0]  CSD_PERM_WRITE_PROTECT = 'h0;         // permanent write protection
parameter [0:0]  CSD_TMP_WRITE_PROTECT  = 'h0;         // temporary write protection
parameter [1:0]  CSD_FILE_FORMAT        = 'b00;        // File format

parameter [3:0]  SCR_SCR_STRUCTURE      = 'h0;         // SCR Structure
parameter [3:0]  SCR_SD_SPEC            = 'd2;         // SD Memory Card - Spec. Version 2.00
parameter [0:0]  SCR_DATA_STATE_ERASE   = 'h0;         // data_status_after erase (0 = 0x00, 1 = 0xFF)
parameter [2:0]  SCR_SD_SECURITY        = 'h1;         // SD Security Support (Not used)
parameter [3:0]  SCR_SD_BUS_WIDTHS      = 'b0101;      // DAT Bus widths supported (1, 4bit)
parameter [0:0]  SCR_SD_SPEC3           = 'b0;
