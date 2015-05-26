include $(top_srcdir)/src/Makefile.inc 

noinst_LTLIBRARIES   = libcoreos.la
libcoreos_la_CFLAGS  = -O0 $(BASE_FLAGS) $(BASE_INCLUDES) $(MSELOS_INCLUDES)
libcoreos_la_SOURCES = util.c task.c mutex.c taskmem.c malloc.c system.c \
                       syscall.c stdc.c                                         

