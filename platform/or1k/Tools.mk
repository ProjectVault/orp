########################################
#
# or1k tools
#
#


THIRD_PARTY := ../../../../../third-party

TOOL_BUILD := build/tools

FLAG_TOOL := $(TOOL_BUILD)/.mkflag.or1k

FLAG_TOOL_OR1K_SRC := $(FLAG_TOOL).src
FLAG_TOOL_OR1K_SRC_CFG1 := $(FLAG_TOOL_OR1K_SRC)-1cfg
FLAG_TOOL_OR1K_SRC_MK1 := $(FLAG_TOOL_OR1K_SRC)-1mk
FLAG_TOOL_OR1K_SRC_MKI1 := $(FLAG_TOOL_OR1K_SRC)-1mki
FLAG_TOOL_OR1K_SRC_CFG2 := $(FLAG_TOOL_OR1K_SRC)-2cfg
FLAG_TOOL_OR1K_SRC_MK2 := $(FLAG_TOOL_OR1K_SRC)-2mk
FLAG_TOOL_OR1K_SRC_MKI2 := $(FLAG_TOOL_OR1K_SRC)-2mki

FLAG_TOOL_OR1K_GCC := $(FLAG_TOOL).gcc
FLAG_TOOL_OR1K_GCC_CFG1 := $(FLAG_TOOL_OR1K_GCC)-1cfg
FLAG_TOOL_OR1K_GCC_MK1 := $(FLAG_TOOL_OR1K_GCC)-1mk
FLAG_TOOL_OR1K_GCC_MKI1 := $(FLAG_TOOL_OR1K_GCC)-1mki
FLAG_TOOL_OR1K_GCC_CFG2 := $(FLAG_TOOL_OR1K_GCC)-2cfg
FLAG_TOOL_OR1K_GCC_MK2 := $(FLAG_TOOL_OR1K_GCC)-2mk
FLAG_TOOL_OR1K_GCC_MKI2 := $(FLAG_TOOL_OR1K_GCC)-2mki

FLAG_TOOL_OR1K_QEMU := $(FLAG_TOOL).qemu
FLAG_TOOL_OR1K_QEMU_CFG := $(FLAG_TOOL_OR1K_QEMU)-cfg
FLAG_TOOL_OR1K_QEMU_MK := $(FLAG_TOOL_OR1K_QEMU)-mk
FLAG_TOOL_OR1K_QEMU_MKI := $(FLAG_TOOL_OR1K_QEMU)-mki

all-tools: $(FLAG_TOOL)

qemu: $(FLAG_TOOL_OR1K_QEMU)

base: $(FLAG_TOOL_OR1K_SRC) $(FLAG_TOOL_OR1K_GCC)


## All tools ##

$(FLAG_TOOL): $(FLAG_TOOL_OR1K_SRC) $(FLAG_TOOL_OR1K_GCC) $(FLAG_TOOL_OR1K_QEMU)
	touch "$(FLAG_TOOL)"


## OR1K SRC ##
$(FLAG_TOOL_OR1K_SRC_CFG1):
	mkdir -p "$(TOOL_BUILD)/or1k-src-1build"
	PATH=$(PATH):$(OR1KTOOLS)/bin cd "$(TOOL_BUILD)/or1k-src-1build" ; \
	  $(THIRD_PARTY)/or1k-src/configure \
	    --target=or1k-elf \
	    --prefix="$(OR1KTOOLS)" \
	    --enable-shared \
	    --disable-itcl \
	    --disable-tk \
	    --disable-tcl \
	    --disable-winsup \
	    --disable-gdbtk \
	    --disable-libgui \
	    --disable-rda \
	    --disable-sid \
	    --disable-sim \
	    --disable-gdb \
	    --with-sysroot \
	    --disable-newlib \
	    --disable-libgloss \
	    --disable-werror
	touch "$(FLAG_TOOL_OR1K_SRC_CFG1)"

$(FLAG_TOOL_OR1K_SRC_MK1): $(FLAG_TOOL_OR1K_SRC_CFG1)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-src-1build"
	touch "$(FLAG_TOOL_OR1K_SRC_MK1)"

$(FLAG_TOOL_OR1K_SRC_MKI1): $(FLAG_TOOL_OR1K_SRC_MK1)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-src-1build" install
	touch "$(FLAG_TOOL_OR1K_SRC_MKI1)"

$(FLAG_TOOL_OR1K_SRC_CFG2): $(FLAG_TOOL_OR1K_GCC_MKI1)
	mkdir -p "$(TOOL_BUILD)/or1k-src-2build"
	PATH=$(PATH):$(OR1KTOOLS)/bin cd "$(TOOL_BUILD)/or1k-src-2build" ; \
	  $(THIRD_PARTY)/or1k-src/configure \
	    --target=or1k-elf \
	    --prefix="$(OR1KTOOLS)" \
	    --enable-shared \
	    --disable-itcl \
	    --disable-tk \
	    --disable-tcl \
	    --disable-winsup \
	    --disable-gdbtk \
	    --disable-libgui \
	    --disable-rda \
	    --disable-sid \
	    --enable-sim \
      --disable-or1ksim \
	    --enable-gdb \
	    --with-or1ksim="$(OR1KTOOLS)" \
	    --with-sysroot \
	    --enable-newlib \
	    --enable-libgloss \
	    --disable-werror
	touch "$(FLAG_TOOL_OR1K_SRC_CFG2)"

$(FLAG_TOOL_OR1K_SRC_MK2): $(FLAG_TOOL_OR1K_SRC_CFG2)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-src-2build"
	touch "$(FLAG_TOOL_OR1K_SRC_MK2)"

$(FLAG_TOOL_OR1K_SRC_MKI2): $(FLAG_TOOL_OR1K_SRC_MK2)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-src-2build" install
	touch "$(FLAG_TOOL_OR1K_SRC_MKI2)"


$(FLAG_TOOL_OR1K_SRC): $(FLAG_TOOL_OR1K_SRC_MKI1) $(FLAG_TOOL_OR1K_SRC_MKI2)
	touch "$(FLAG_TOOL_OR1K_SRC)"


## OR1K GCC ##
$(FLAG_TOOL_OR1K_GCC_CFG1): $(FLAG_TOOL_OR1K_SRC_MKI1)
	mkdir -p "$(TOOL_BUILD)/or1k-gcc-1build"
	PATH=$(PATH):$(OR1KTOOLS)/bin cd "$(TOOL_BUILD)/or1k-gcc-1build" ; \
	  $(THIRD_PARTY)/or1k-gcc/configure \
	    --target=or1k-elf \
	    --prefix="$(OR1KTOOLS)" \
	    --enable-languages=c \
	    --disable-shared \
	    --disable-libssp \
	    --disable-werror
	touch "$(FLAG_TOOL_OR1K_GCC_CFG1)"

$(FLAG_TOOL_OR1K_GCC_MK1): $(FLAG_TOOL_OR1K_GCC_CFG1)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-gcc-1build"
	touch "$(FLAG_TOOL_OR1K_GCC_MK1)"

$(FLAG_TOOL_OR1K_GCC_MKI1): $(FLAG_TOOL_OR1K_GCC_MK1)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-gcc-1build" install
	ln -s $(OR1KTOOLS)/bin/or1k-elf-gcc $(OR1KTOOLS)/bin/or1k-elf-cc
	touch "$(FLAG_TOOL_OR1K_GCC_MKI1)"

$(FLAG_TOOL_OR1K_GCC_CFG2): $(FLAG_TOOL_OR1K_SRC_MKI2)
	mkdir -p "$(TOOL_BUILD)/or1k-gcc-2build"
	PATH=$(PATH):$(OR1KTOOLS)/bin cd "$(TOOL_BUILD)/or1k-gcc-2build" ; \
	  $(THIRD_PARTY)/or1k-gcc/configure \
	    --target=or1k-elf \
	    --prefix="$(OR1KTOOLS)" \
	    --enable-languages=c,c++ \
	    --disable-shared \
	    --disable-libssp \
	    --with-newlib \
	    --disable-werror
	touch "$(FLAG_TOOL_OR1K_GCC_CFG2)"

$(FLAG_TOOL_OR1K_GCC_MK2): $(FLAG_TOOL_OR1K_GCC_CFG2)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-gcc-2build"
	touch "$(FLAG_TOOL_OR1K_GCC_MK2)"

$(FLAG_TOOL_OR1K_GCC_MKI2): $(FLAG_TOOL_OR1K_GCC_MK2)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-gcc-2build" install
	touch "$(FLAG_TOOL_OR1K_GCC_MKI2)"

$(FLAG_TOOL_OR1K_GCC): $(FLAG_TOOL_OR1K_GCC_MKI1) $(FLAG_TOOL_OR1K_GCC_MKI2)
	touch "$(FLAG_TOOL_OR1K_GCC)"



## qemu ##
$(FLAG_TOOL_OR1K_QEMU_CFG):
	mkdir -p "$(TOOL_BUILD)/or1k-qemu-build"
	cd "$(TOOL_BUILD)/or1k-qemu-build" ; \
	  $(THIRD_PARTY)/qemu-orp/configure \
	    --prefix="$(OR1KTOOLS)" \
	    --enable-debug \
	    --target-list=or32-softmmu \
	    --extra-cflags=-Wno-redundant-decls
	touch "$(FLAG_TOOL_OR1K_QEMU_CFG)"

$(FLAG_TOOL_OR1K_QEMU_MK): $(FLAG_TOOL_OR1K_QEMU_CFG)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-qemu-build"
	touch "$(FLAG_TOOL_OR1K_QEMU_MK)"

$(FLAG_TOOL_OR1K_QEMU_MKI): $(FLAG_TOOL_OR1K_QEMU_MK)
	PATH=$(PATH):$(OR1KTOOLS)/bin make -C "$(TOOL_BUILD)/or1k-qemu-build" install
	touch "$(FLAG_TOOL_OR1K_QEMU_MKI)"

$(FLAG_TOOL_OR1K_QEMU): $(FLAG_TOOL_OR1K_QEMU_MKI)
	touch "$(FLAG_TOOL_OR1K_QEMU)"

