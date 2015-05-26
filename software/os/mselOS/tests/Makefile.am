
# Default flags
AM_CFLAGS = --std=gnu99 -Wall
AM_LDFLAGS =

# Debugging enabled?
if DEBUG
if SF2BUILD
AM_CFLAGS += -DDEBUG -O0 -gdwarf-2
else
AM_CFLAGS += -DDEBUG -O0 -ggdb
endif
else
AM_CFLAGS += -DRELEASE -Os
endif

# Arch specific flags
if OPENRISC
AM_CFLAGS += -D__GCC_OpenRISC__ -fno-strict-aliasing -fno-builtin -nostartfiles -fno-common
AM_LDFLAGS += -T$(top_srcdir)/src/arch/$(OS_ARCH)/link.ld -Wl,"-e 0x00100100"
endif

if ARM
AM_CFLAGS += -mcpu=cortex-m3 -mthumb -fno-strict-aliasing -nostartfiles -fno-common
if SF2BUILD
AM_LDFLAGS += -T$(top_srcdir)/src/arch/$(OS_ARCH)/link.ld.sf2 
else
AM_LDFLAGS += -T$(top_srcdir)/src/arch/$(OS_ARCH)/link.ld
endif
endif

# Default include paths
AM_CFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/src/arch/$(OS_ARCH)/       \
             -I$(top_srcdir)/src/os -I$(top_srcdir)/src/driver                  \
             -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/arch                   

check_PROGRAMS =
TESTS          =
XFAIL_TESTS    = 

#############################
# Begin individual unit tests

check_PROGRAMS      += boot_test
boot_test_SOURCES     = boot_test.c
boot_test_LDADD       = ../src/libmselos.la

# Uses raw hw, so disable in arm for now
if OPENRISC
check_PROGRAMS      += throughput_test
throughput_test_SOURCES     = throughput_test.c
throughput_test_LDADD       = ../src/libmselos.la
endif

check_PROGRAMS      += aes_test
TESTS               += aes_test
aes_test_SOURCES     = aes_test.c
aes_test_LDADD       = ../src/libmselos.la

check_PROGRAMS      += sha_test
TESTS               += sha_test
sha_test_SOURCES     = sha_test.c
sha_test_LDADD       = ../src/libmselos.la

check_PROGRAMS      += ecc_test
TESTS               += ecc_test
ecc_test_SOURCES     = ecc_test.c
ecc_test_LDADD       = ../src/libmselos.la

# The FFS test requires Qemu and a specific "Android" binary, so only run it
# if enabled
if FFS_TEST
check_PROGRAMS      += ffs_session 
TESTS               += ffs_session
ffs_session_SOURCES  = ffs_session.c 
ffs_session_LDADD    = ../src/libmselos.la
ffs_session_DEPENDENCIES = ffs_android$(EXEEXT)
endif

check_PROGRAMS    += yield_loop
TESTS             += yield_loop
yield_loop_SOURCES = yield_loop.c
yield_loop_LDADD   = ../src/libmselos.la
yield_loop_LDFLAGS = $(AM_LDFLAGS) -Wl,"-Map=yield_loop.map"

check_PROGRAMS   += uart_test
TESTS            += uart_test
uart_test_SOURCES = uart_test.c
uart_test_LDADD   = ../src/libmselos.la

check_PROGRAMS             += task_stack_overflow
TESTS                      += task_stack_overflow
task_stack_overflow_SOURCES = task_stack_overflow.c
task_stack_overflow_LDADD   = ../src/libmselos.la

check_PROGRAMS             += task_malloc
TESTS                      += task_malloc
task_malloc_SOURCES = task_malloc.c
task_malloc_LDADD   = ../src/libmselos.la

# Currently XFAIL because qemu doesn't emulate flash to store MTC
check_PROGRAMS      += mtc_test
TESTS               += mtc_test
XFAIL_TESTS         += mtc_test
mtc_test_SOURCES     = mtc_test.c
mtc_test_LDADD       = ../src/libmselos.la

TEST_EXTENSIONS = .sh
AM_SH_LOG_COMPILER = common/test_harness.sh

ffs_android$(EXEEXT): ffs_android.c
	gcc -o ffs_android ffs_android.c

clean-local:
	-rm -rf ffs_android

# each test will be invoked as ./common/test_harness.sh path/to/test.sh


