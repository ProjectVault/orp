include $(top_srcdir)/src/Makefile.inc 

SUBDIRS = arch lib driver os

lib_LTLIBRARIES      = libmselos.la
libmselos_la_SOURCES = 
libmselos_la_LIBADD  = $(top_builddir)/src/arch/$(OS_ARCH)/libarch.la              \
                       $(top_builddir)/src/lib/libmicroSEL.la                      \
                       $(top_builddir)/src/driver/libdriver.la                     \
                       $(top_builddir)/src/os/libcoreos.la

libmselosdir         = $(prefix)/share/mselOS
libmselos_DATA       = $(MSELOS_LD_FILE)

