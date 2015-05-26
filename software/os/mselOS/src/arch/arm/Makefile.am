include $(top_srcdir)/src/Makefile.inc 

noinst_LTLIBRARIES = libarch.la
libarch_la_CFLAGS  = -Os $(BASE_FLAGS) $(BASE_INCLUDES) 
libarch_la_SOURCES = arch.c isr.c trng.c aes.c sha.c ecc.c ffs.c task.c mtc.c \
                     master_key.c uart.c gpio.c led.c startup.S
