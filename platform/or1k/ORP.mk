########################################
#
# orp-or1k tools
#
#


BUILD := build/orp
SLASH := $(OR1KORP)
SWPATH := ../../../../../software
SWPATH2 := ../../software

FLAG_MSELOS := $(BUILD)/.mkflag.mselOS
FLAG_MSELOS_AUTOCONF := $(FLAG_MSELOS)-autoconf
FLAG_MSELOS_CONFIGURE := $(FLAG_MSELOS)-configure
FLAG_MSELOS_BUILD := $(FLAG_MSELOS)-build
FLAG_MSELOS_CHECK := $(FLAG_MSELOS)-check
FLAG_MSELOS_INSTALL := $(FLAG_MSELOS)-install

FLAG_ORP_CHAT := $(BUILD)/.mkflag.orp-urchat
FLAG_ORP_CHAT_AUTOCONF := $(FLAG_ORP_CHAT)-autoconf
FLAG_ORP_CHAT_CONFIGURE := $(FLAG_ORP_CHAT)-configure
FLAG_ORP_CHAT_BUILD := $(FLAG_ORP_CHAT)-build
FLAG_ORP_CHAT_INSTALL := $(FLAG_ORP_CHAT)-install

FLAG_ORP_ENCRYPTOR := $(BUILD)/.mkflag.orp-encryptor
FLAG_ORP_ENCRYPTOR_AUTOCONF := $(FLAG_ORP_ENCRYPTOR)-autoconf
FLAG_ORP_ENCRYPTOR_CONFIGURE := $(FLAG_ORP_ENCRYPTOR)-configure
FLAG_ORP_ENCRYPTOR_BUILD := $(FLAG_ORP_ENCRYPTOR)-build
FLAG_ORP_ENCRYPTOR_INSTALL := $(FLAG_ORP_ENCRYPTOR)-install

FLAG_ORP_XTS_ENCRYPTOR := $(BUILD)/.mkflag.orp-xts-encryptor
FLAG_ORP_XTS_ENCRYPTOR_AUTOCONF := $(FLAG_ORP_XTS_ENCRYPTOR)-autoconf
FLAG_ORP_XTS_ENCRYPTOR_CONFIGURE := $(FLAG_ORP_XTS_ENCRYPTOR)-configure
FLAG_ORP_XTS_ENCRYPTOR_BUILD := $(FLAG_ORP_XTS_ENCRYPTOR)-build
FLAG_ORP_XTS_ENCRYPTOR_INSTALL := $(FLAG_ORP_XTS_ENCRYPTOR)-install

FLAG_ORP_BULK_ENCRYPTION := $(BUILD)/.mkflag.orp-bulk-encryption
FLAG_ORP_BULK_ENCRYPTION_AUTOCONF := $(FLAG_ORP_BULK_ENCRYPTION)-autoconf
FLAG_ORP_BULK_ENCRYPTION_CONFIGURE := $(FLAG_ORP_BULK_ENCRYPTION)-configure
FLAG_ORP_BULK_ENCRYPTION_BUILD := $(FLAG_ORP_BULK_ENCRYPTION)-build
FLAG_ORP_BULK_ENCRYPTION_INSTALL := $(FLAG_ORP_BULK_ENCRYPTION)-install

FLAG_ORP_BUNDLE_DEMO := $(BUILD)/.mkflag.bundle-demo
FLAG_ORP_BUNDLE_DEMO_AUTOCONF := $(FLAG_ORP_BUNDLE_DEMO)-autoconf
FLAG_ORP_BUNDLE_DEMO_CONFIGURE := $(FLAG_ORP_BUNDLE_DEMO)-configure
FLAG_ORP_BUNDLE_DEMO_BUILD := $(FLAG_ORP_BUNDLE_DEMO)-build
FLAG_ORP_BUNDLE_DEMO_INSTALL := $(FLAG_ORP_BUNDLE_DEMO)-install


all-orp: $(FLAG_MSELOS) $(FLAG_ORP_CHAT) $(FLAG_ORP_ENCRYPTOR) $(FLAG_ORP_XTS_ENCRYPTOR) $(FLAG_ORP_BULK_ENCRYPTION) $(FLAG_ORP_BUNDLE_DEMO)


mselOS-configure: $(FLAG_MSELOS_CONFIGURE)

mselOS-build: $(FLAG_MSELOS_BUILD)

mselOS-check: $(FLAG_MSELOS_CHECK)

mselOS-install: $(FLAG_MSELOS_INSTALL)

mselOS: $(FLAG_MSELOS)


orp-urchat-configure: $(FLAG_ORP_CHAT_CONFIGURE)

orp-urchat-build: $(FLAG_ORP_CHAT_BUILD)

orp-urchat-install: $(FLAG_ORP_CHAT_INSTALL)

orp-urchat: $(FLAG_ORP_CHAT)


orp-encryptor-configure: $(FLAG_ORP_ENCRYPTOR_CONFIGURE)

orp-encryptor-build: $(FLAG_ORP_ENCRYPTOR_BUILD)

orp-encryptor-install: $(FLAG_ORP_ENCRYPTOR_INSTALL)

orp-encryptor: $(FLAG_ORP_ENCRYPTOR)


orp-xts-encryptor-configure: $(FLAG_ORP_XTS_ENCRYPTOR_CONFIGURE)

orp-xts-encryptor-build: $(FLAG_ORP_XTS_ENCRYPTOR_BUILD)

orp-xts-encryptor-install: $(FLAG_ORP_XTS_ENCRYPTOR_INSTALL)

orp-xts-encryptor: $(FLAG_ORP_XTS_ENCRYPTOR)


orp-bulkencryption-configure: $(FLAG_ORP_BULK_ENCRYPTION_CONFIGURE)

orp-bulkencryption-build: $(FLAG_ORP_BULK_ENCRYPTION_BUILD)

orp-bulkencryption-install: $(FLAG_ORP_BULK_ENCRYPTION_INSTALL)

orp-bulkencryption: $(FLAG_ORP_BULK_ENCRYPTION)


bundle-demo-configure: $(FLAG_ORP_BUNDLE_DEMO_CONFIGURE)

bundle-demo-build: $(FLAG_ORP_BUNDLE_DEMO_BUILD)

bundle-demo-install: $(FLAG_ORP_BUNDLE_DEMO_INSTALL)

bundle-demo: $(FLAG_ORP_BUNDLE_DEMO)


## mselOS ##
$(FLAG_MSELOS_AUTOCONF):
	mkdir -p "$(BUILD)"
	cd "$(SWPATH2)/os/mselOS" ; autoreconf -i
	touch $(FLAG_MSELOS_AUTOCONF)

$(FLAG_MSELOS_CONFIGURE): $(FLAG_MSELOS_AUTOCONF)
	mkdir -p "$(BUILD)/mselOS"
	cd "$(BUILD)/mselOS" ; $(SWPATH)/os/mselOS/configure --prefix=$(OR1KORP) CFLAGS="-mboard=or1ksim-uart" --host=or1k-elf
	touch $(FLAG_MSELOS_CONFIGURE)

$(FLAG_MSELOS_BUILD): $(FLAG_MSELOS_CONFIGURE)
	make -C "$(BUILD)/mselOS"
	touch $(FLAG_MSELOS_BUILD)

$(FLAG_MSELOS_CHECK): $(FLAG_MSELOS_BUILD)
	make -C "$(BUILD)/mselOS" check

$(FLAG_MSELOS_INSTALL): $(FLAG_MSELOS_BUILD)
	make -C "$(BUILD)/mselOS" install
	touch $(FLAG_MSELOS_INSTALL)

$(FLAG_MSELOS): $(FLAG_MSELOS_INSTALL)
	touch $(FLAG_MSELOS)



## orp-chat ##
$(FLAG_ORP_CHAT_AUTOCONF):
	mkdir -p "$(BUILD)"
	cd "$(SWPATH2)/applications/orp-urchat" ; autoreconf -i
	touch $(FLAG_ORP_CHAT_AUTOCONF)

$(FLAG_ORP_CHAT_CONFIGURE): $(FLAG_MSELOS) $(FLAG_ORP_CHAT_AUTOCONF)
	mkdir -p "$(BUILD)/orp-urchat"
	cd "$(BUILD)/orp-urchat" ; $(SWPATH)/applications/orp-urchat/configure --prefix=$(OR1KORP) CFLAGS="-mboard=or1ksim-uart" --host=or1k-elf
	touch $(FLAG_ORP_CHAT_CONFIGURE)

$(FLAG_ORP_CHAT_BUILD): $(FLAG_ORP_CHAT_CONFIGURE)
	make -C "$(BUILD)/orp-urchat"
	touch $(FLAG_ORP_CHAT_BUILD)

$(FLAG_ORP_CHAT_INSTALL): $(FLAG_ORP_CHAT_BUILD)
	make -C "$(BUILD)/orp-urchat" install
	touch $(FLAG_ORP_CHAT_INSTALL)

$(FLAG_ORP_CHAT): $(FLAG_ORP_CHAT_INSTALL)
	touch $(FLAG_ORP_CHAT)


## orp-encryptor ##
$(FLAG_ORP_ENCRYPTOR_AUTOCONF):
	mkdir -p "$(BUILD)"
	cd "$(SWPATH2)/applications/orp-encryptor" ; autoreconf -i
	touch $(FLAG_ORP_ENCRYPTOR_AUTOCONF)

$(FLAG_ORP_ENCRYPTOR_CONFIGURE): $(FLAG_MSELOS) $(FLAG_ORP_ENCRYPTOR_AUTOCONF)
	mkdir -p "$(BUILD)/orp-encryptor"
	cd "$(BUILD)/orp-encryptor" ; $(SWPATH)/applications/orp-encryptor/configure --prefix=$(OR1KORP) CFLAGS="-mboard=or1ksim-uart" --host=or1k-elf
	touch $(FLAG_ORP_ENCRYPTOR_CONFIGURE)

$(FLAG_ORP_ENCRYPTOR_BUILD): $(FLAG_ORP_ENCRYPTOR_CONFIGURE)
	make -C "$(BUILD)/orp-encryptor"
	touch $(FLAG_ORP_ENCRYPTOR_BUILD)

$(FLAG_ORP_ENCRYPTOR_INSTALL): $(FLAG_ORP_ENCRYPTOR_BUILD)
	make -C "$(BUILD)/orp-encryptor" install
	touch $(FLAG_ORP_ENCRYPTOR_INSTALL)

$(FLAG_ORP_ENCRYPTOR): $(FLAG_ORP_ENCRYPTOR_INSTALL)
	touch $(FLAG_ORP_ENCRYPTOR)


## orp-xts-encryptor ##
$(FLAG_ORP_XTS_ENCRYPTOR_AUTOCONF):
	mkdir -p "$(BUILD)"
	cd "$(SWPATH2)/applications/orp-xts-encryptor" ; autoreconf -i
	touch $(FLAG_ORP_XTS_ENCRYPTOR_AUTOCONF)

$(FLAG_ORP_XTS_ENCRYPTOR_CONFIGURE): $(FLAG_MSELOS) $(FLAG_ORP_XTS_ENCRYPTOR_AUTOCONF)
	mkdir -p "$(BUILD)/orp-xts-encryptor"
	cd "$(BUILD)/orp-xts-encryptor" ; $(SWPATH)/applications/orp-xts-encryptor/configure --prefix=$(OR1KORP) CFLAGS="-mboard=or1ksim-uart" --host=or1k-elf
	touch $(FLAG_ORP_XTS_ENCRYPTOR_CONFIGURE)

$(FLAG_ORP_XTS_ENCRYPTOR_BUILD): $(FLAG_ORP_XTS_ENCRYPTOR_CONFIGURE)
	make -C "$(BUILD)/orp-xts-encryptor"
	touch $(FLAG_ORP_XTS_ENCRYPTOR_BUILD)

$(FLAG_ORP_XTS_ENCRYPTOR_INSTALL): $(FLAG_ORP_XTS_ENCRYPTOR_BUILD)
	make -C "$(BUILD)/orp-xts-encryptor" install
	touch $(FLAG_ORP_XTS_ENCRYPTOR_INSTALL)

$(FLAG_ORP_XTS_ENCRYPTOR): $(FLAG_ORP_XTS_ENCRYPTOR_INSTALL)
	touch $(FLAG_ORP_XTS_ENCRYPTOR)


## orp-bulkencryption ##
$(FLAG_ORP_BULK_ENCRYPTION_AUTOCONF):
	mkdir -p "$(BUILD)"
	cd "$(SWPATH2)/applications/orp-bulkencryption" ; autoreconf -i
	touch $(FLAG_ORP_BULK_ENCRYPTION_AUTOCONF)

$(FLAG_ORP_BULK_ENCRYPTION_CONFIGURE): $(FLAG_MSELOS) $(FLAG_ORP_BULK_ENCRYPTION_AUTOCONF)
	mkdir -p "$(BUILD)/orp-bulkencryption"
	cd "$(BUILD)/orp-bulkencryption" ; $(SWPATH)/applications/orp-bulkencryption/configure --prefix=$(OR1KORP) CFLAGS="-mboard=or1ksim-uart" --host=or1k-elf
	touch $(FLAG_ORP_BULK_ENCRYPTION_CONFIGURE)

$(FLAG_ORP_BULK_ENCRYPTION_BUILD): $(FLAG_ORP_BULK_ENCRYPTION_CONFIGURE)
	make -C "$(BUILD)/orp-bulkencryption"
	touch $(FLAG_ORP_BULK_ENCRYPTION_BUILD)

$(FLAG_ORP_BULK_ENCRYPTION_INSTALL): $(FLAG_ORP_BULK_ENCRYPTION_BUILD)
	make -C "$(BUILD)/orp-bulkencryption" install
	touch $(FLAG_ORP_BULK_ENCRYPTION_INSTALL)

$(FLAG_ORP_BULK_ENCRYPTION): $(FLAG_ORP_BULK_ENCRYPTION_INSTALL)
	touch $(FLAG_ORP_BULK_ENCRYPTION)


## bundle-demo ##
$(FLAG_ORP_BUNDLE_DEMO_AUTOCONF):
	mkdir -p "$(BUILD)"
	cd "$(SWPATH2)/applications/bundle-demo" ; autoreconf -i
	touch $(FLAG_ORP_BUNDLE_DEMO_AUTOCONF)

$(FLAG_ORP_BUNDLE_DEMO_CONFIGURE): $(FLAG_MSELOS) $(FLAG_ORP_CHAT) $(FLAG_ORP_ENCRYPTOR) $(FLAG_ORP_XTS_ENCRYPTOR) $(FLAG_ORP_BULK_ENCRYPTION) $(FLAG_ORP_BUNDLE_DEMO_AUTOCONF)
	mkdir -p "$(BUILD)/bundle-demo"
	cd "$(BUILD)/bundle-demo" ; $(SWPATH)/applications/bundle-demo/configure --prefix=$(OR1KORP) CFLAGS="-mboard=or1ksim-uart" --host=or1k-elf
	touch $(FLAG_ORP_BUNDLE_DEMO_CONFIGURE)

$(FLAG_ORP_BUNDLE_DEMO_BUILD): $(FLAG_ORP_BUNDLE_DEMO_CONFIGURE)
	make -C "$(BUILD)/bundle-demo"
	touch $(FLAG_ORP_BUNDLE_DEMO_BUILD)

$(FLAG_ORP_BUNDLE_DEMO_INSTALL): $(FLAG_ORP_BUNDLE_DEMO_BUILD)
	make -C "$(BUILD)/bundle-demo" install
	touch $(FLAG_ORP_BUNDLE_DEMO_INSTALL)

$(FLAG_ORP_BUNDLE_DEMO): $(FLAG_ORP_BUNDLE_DEMO_INSTALL)
	touch $(FLAG_ORP_BUNDLE_DEMO)
