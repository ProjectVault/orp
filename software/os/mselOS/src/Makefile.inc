
BASE_FLAGS = --std=gnu99 -Wall 

# .. don't include any arch specific paths here, just arch/ for generic arch.h defs
BASE_INCLUDES = -I. -I$(top_srcdir)/include -I$(top_srcdir)/src -I$(top_srcdir)/src/arch

# Arch specific flags
if OPENRISC
  BASE_FLAGS += -D__GCC_OpenRISC__ -fno-strict-aliasing -fno-builtin -nostartfiles
  MSELOS_LD_FILE = $(top_srcdir)/src/arch/$(OS_ARCH)/link.ld
endif
if ARM
  BASE_FLAGS += -mcpu=cortex-m3 -mthumb -fno-builtin -nostartfiles
  MSELOS_LD_FILE = 
endif

# Debugging enabled?
if DEBUG
if SF2BUILD
BASE_FLAGS += -DDEBUG -gdwarf-2
else
BASE_FLAGS += -DDEBUG -ggdb 
endif 
else
BASE_FLAGS += -DRELEASE
endif
