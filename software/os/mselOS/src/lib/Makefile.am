include $(top_srcdir)/src/Makefile.inc 

noinst_LTLIBRARIES     = libmicroSEL.la
libmicroSEL_la_CFLAGS  = -Os $(BASE_FLAGS) $(BASE_INCLUDES)
libmicroSEL_la_SOURCES = crypto/aes.c crypto/aes_gcm.c crypto/aes_xts.c \
                         crypto/kdf.c crypto/prng.c crypto/sha2.c tidl.c
