include $(top_srcdir)/src/Makefile.inc 

noinst_LTLIBRARIES   = libdriver.la
libdriver_la_CFLAGS  = -Os $(BASE_FLAGS) $(BASE_INCLUDES) $(MSELOS_INCLUDES)
libdriver_la_SOURCES = uart.c swcrypto/sw_aes.c swcrypto/ed521.c trng_driver.c aes_driver.c \
                       sha_driver.c ecc_driver.c ffs_driver.c ffs_session.c led.c   \
                       gpio.c mtc.c master_key.c provision.c pol.c

if SW_AES
libdriver_la_SOURCES += swcrypto/sw_aes.c
BASE_FLAGS += -DUSE_SW_AES
endif

if SW_SHA
libdriver_la_SOURCES += swcrypto/sw_sha.c
BASE_FLAGS += -DUSE_SW_SHA
endif

if SW_ECC
libdriver_la_SOURCES += swcrypto/ed521.c
BASE_FLAGS += -DUSE_SW_ECC
endif
