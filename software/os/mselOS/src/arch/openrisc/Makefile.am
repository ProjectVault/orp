include $(top_srcdir)/src/Makefile.inc 

noinst_LTLIBRARIES = libarch.la
libarch_la_CFLAGS  = -Os $(BASE_FLAGS) $(BASE_INCLUDES) $(MSELOS_INCLUDES)
libarch_la_SOURCES = arch.c gpio.c init.s isr.c spr.c led.c mtc.c \
                     flash.c master_key.c uart.c trng.c aes.c sha.c ecc.c ffs.c stdc.c


