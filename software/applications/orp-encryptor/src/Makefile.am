
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



lib_LTLIBRARIES = libencryptor.la
libencryptor_la_CFLAGS = -Os $(BASE_FLAGS) $(BASE_INCLUDES)
libencryptor_la_SOURCES = encryptor.c encProto.c
libencryptor_la_LIBADD = -lmselos

bin_PROGRAMS = bundle-encryptor
bundle_encryptor_CFLAGS = -Os $(BASE_FLAGS) $(BASE_INCLUDES)
bundle_encryptor_SOURCES = main.c
bundle_encryptor_LDADD = -lmselos libencryptor.la
