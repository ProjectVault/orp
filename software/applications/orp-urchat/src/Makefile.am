
# Default flags
noinst_LTLIBRARIES =

BASE_FLAGS = --std=gnu99 -Wall -L$(ORP)/lib
BASE_INCLUDES = -I$(top_srcdir)/include -I$(ORP)/include
AM_LDFLAGS =

# Arch specific flags
if OPENRISC
BASE_FLAGS += -D__GCC_OpenRISC__ -fno-strict-aliasing -fno-builtin -nostartfiles
AM_LDFLAGS += -T$(ORP)/share/mselOS/link.ld -Wl,"-e 0x00100100"
endif
if ARM
BASE_FLAGS += -mcpu=cortex-m3 -mthumb 
endif

# Debugging enabled?
if DEBUG
BASE_FLAGS += -DDEBUG -ggdb
else
BASE_FLAGS += -DRELEASE
endif



lib_LTLIBRARIES = liburchat.la
liburchat_la_CFLAGS = -Os $(BASE_FLAGS) $(BASE_INCLUDES)
liburchat_la_SOURCES = urchat.c chatProto.c
liburchat_la_LIBADD = -lmselos

bin_PROGRAMS = bundle-urchat
bundle_urchat_CFLAGS = -Os $(BASE_FLAGS) $(BASE_INCLUDES)
bundle_urchat_SOURCES = main.c
bundle_urchat_LDADD = -lmselos liburchat.la
