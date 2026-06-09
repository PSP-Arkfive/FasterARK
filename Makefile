.PHONY: psp vita updater

PY = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
BUILDTOOLS = $(PSPDEV)/share/psp-cfw-sdk/build-tools
CHOVYSIGNDIR = ./Resources/Chovy-Sign
CHOVYSIGN = $(CHOVYSIGNDIR)/ChovySign-CLI

all: psp vita updater

psp:
	make -C PSP
#	PSP Lite Install
	mkdir -p dist/tmp/PSP/GAME/FasterARK/
	mkdir -p dist/tmp/PSP/SAVEDATA/
	cp -r Resources/ARK_01234 dist/tmp/PSP/SAVEDATA/
	cp -r Resources/CustomIPL dist/tmp/PSP/GAME/
	cp PSP/EBOOT.PBP dist/tmp/PSP/GAME/FasterARK/
	cp Resources/LIBS/ipl_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp Resources/LIBS/kbooti_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp Resources/LIBS/kpspident.prx dist/tmp/PSP/GAME/CustomIPL/
	cp Resources/LIBS/usbdevice.prx dist/tmp/PSP/GAME/FasterARK/
	cp Resources/LIBS/idsregeneration.prx dist/tmp/PSP/GAME/FasterARK/
	cd dist/tmp/ && zip -m -r FasterARK_psp_lite.zip * && cd ../../ && mv dist/tmp/FasterARK_psp_lite.zip dist/
#	PSP Full Install
	mkdir -p dist/tmp/PSP/GAME/FasterARK/
	mkdir -p dist/tmp/PSP/SAVEDATA/
	cp -r Resources/ARK_01234 dist/tmp/PSP/SAVEDATA/
	cp Resources/Extras/* dist/tmp/PSP/SAVEDATA/ARK_01234/
	cp -r Resources/CustomIPL dist/tmp/PSP/GAME/
	cp -r Resources/DC10 dist/tmp/PSP/GAME/
	cp -r Resources/ARK150on660 dist/tmp/PSP/GAME/
	cp -r Resources/OC_Tester dist/tmp/PSP/GAME/
	cp PSP/EBOOT.PBP dist/tmp/PSP/GAME/FasterARK/
	cp Resources/LIBS/usbdevice.prx dist/tmp/PSP/GAME/FasterARK/
	cp Resources/LIBS/idsregeneration.prx dist/tmp/PSP/GAME/FasterARK/
	cp Resources/LIBS/ipl_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp Resources/LIBS/kbooti_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp Resources/LIBS/kpspident.prx dist/tmp/PSP/GAME/CustomIPL/
	cp Resources/LIBS/iop.prx dist/tmp/PSP/GAME/DC10/
	cp Resources/LIBS/ipl_update.prx dist/tmp/PSP/GAME/DC10/
	cp Resources/LIBS/usbdevice.prx dist/tmp/PSP/GAME/DC10/
	cp Resources/LIBS/idsregeneration.prx dist/tmp/PSP/GAME/DC10/
	cp Resources/LIBS/vlf.prx dist/tmp/PSP/GAME/DC10/
	cp Resources/LIBS/intraFont-vlf.prx dist/tmp/PSP/GAME/DC10/
	cp Resources/LIBS/pspdecrypt.prx dist/tmp/PSP/GAME/DC10/
	cp Resources/LIBS/libpsardumper.prx dist/tmp/PSP/GAME/DC10/
	cp Resources/LIBS/lflash_fdisk.prx dist/tmp/PSP/GAME/DC10/
	cp Resources/LIBS/pspdecrypt.prx dist/tmp/PSP/GAME/ARK150on660/
	cp Resources/LIBS/libpsardumper.prx dist/tmp/PSP/GAME/ARK150on660/
	cd dist/tmp/ && zip -m -r FasterARK_psp_full.zip * && cd ../../ && mv dist/tmp/FasterARK_psp_full.zip dist/
	rm -r dist/tmp/

vita:
	mkdir -p dist
	cp -r Resources/ARK_01234 PSVita/res/save/
	cp -r Resources/Extras/* PSVita/res/save/ARK_01234/
	cp -r Resources/PSVita/* PSVita/res/save/ARK_01234/
#   ePSP Bubble
	make -C PSVita/loader/psp/eboot
	cp Resources/ARK_01234/ICON0.PNG PSVita/loader/psp/eboot/iso_files/psp_game/
	cp PSVita/loader/psp/eboot/psploader.prx PSVita/loader/psp/eboot/iso_files/psp_game/sysdir/boot.bin
	mkisofs -o PSVita/loader/psp/eboot/psploader.iso PSVita/loader/psp/eboot/iso_files/
	$(CHOVYSIGN) --psp PSVita/loader/psp/eboot/psploader.iso --no-psvimg --nopspemudrm EP0099-NPUZ01234_00-CHOVYSIGN0000000
	cp $(CHOVYSIGNDIR)/output/PGAME/efcdab8967452301/NPUZ01234/PSP/LICENSE/EP0099-NPUZ01234_00-CHOVYSIGN0000000.rif PSVita/res/rif/psp.rif
	cp $(CHOVYSIGNDIR)/output/PGAME/efcdab8967452301/NPUZ01234/PSP/GAME/NPUZ01234/EBOOT.PBP PSVita/res/psp/
	make -C PSVita/loader/psp/pboot
	make -C Resources/PrxEncrypter
	./Resources/PrxEncrypter/prxencrypter PSVita/loader/psp/pboot/psploader.prx
	pack-pbp PSVita/res/psp/PBOOT.PBP PSVita/loader/psp/pboot/PARAM_PBOOT.SFO Resources/ARK_01234/ICON0.PNG NULL NULL NULL NULL DATA.PSP NULL
	make -C Resources/Peops
	$(PY) $(BUILDTOOLS)/gz/pspgz.py PSVita/res/save/ARK_01234/PS1SPU.PRX $(BUILDTOOLS)/gz/UserModule.hdr Resources/Peops/peops.prx peops 0x0000
#   ePSX Bubble
	make -C PSVita/loader/psx
	make -C PSVita/loader/psx/payload
	$(CHOVYSIGN) --pops PSVita/loader/psx/psxloader.cue --pops-info "ARK-X" PSVita/loader/psx/ICON0.PNG --pops-eboot PSVita/loader/psx/psxloader.prx --no-psvimg --nopspemudrm EP0099-SCPS10084_00-CHOVYSIGN0000000
	cp PSVita/loader/psx/payload/ARKX.BIN PSVita/res/psx/
	cp $(CHOVYSIGNDIR)/output/PSP/LICENSE/EP0099-SCPS10084_00-CHOVYSIGN0000000.rif PSVita/res/rif/psx.rif
	cp $(CHOVYSIGNDIR)/output/PSP/GAME/SCPS10084/EBOOT.PBP PSVita/res/psx/
#	Installer
	mkdir -p PSVita/build
	cd PSVita/build && cmake .. && make && cd ../../
	cp PSVita/build/FasterARK.vpk dist/FasterARK_psvita.vpk

updater:
#	Updater
	mkdir -p dist/tmp/
	cp -r Resources/LIBS Updater/Resources/
	cp -r Resources/ARK_01234 Updater/Resources/
	cp Resources/DC10/DC10.ARK Updater/Resources/ARK_01234/
	cp Resources/ARK150on660/FLASH150.ARK Updater/Resources/ARK_01234/
	cp Resources/ARK150on660/LANG150.ARK Updater/Resources/ARK_01234/
	cp Resources/Extras/LANG.ARK Updater/Resources/ARK_01234/
	cp Resources/Extras/VBOOT.PBP Updater/Resources/ARK_01234/
	cp Resources/Extras/VSHMENU.PRX Updater/Resources/ARK_01234/
	cp Resources/PSVita/XBOOT.PBP Updater/Resources/ARK_01234/
	make -C Resources/Peops
	$(PY) $(BUILDTOOLS)/gz/pspgz.py Updater/Resources/ARK_01234/PS1SPU.PRX $(BUILDTOOLS)/gz/UserModule.hdr Resources/Peops/peops.prx peops 0x0000
	make -C Updater
	mkdir -p dist/tmp/PSP/GAME/UPDATE/
	cp Updater/EBOOT.PBP dist/tmp/PSP/GAME/UPDATE/
	cd dist/tmp/ && zip -m -r ARK_UPDATE.zip * && cd ../../ && mv dist/tmp/ARK_UPDATE.zip dist/
	rm -r dist/tmp/
	rm -rf Updater/Resources/LIBS
	rm -rf Updater/Resources/ARK_01234

clean:
	rm -rf dist
	rm -f DATA.PSP
	rm -rf PSVita/build
	rm -f PSVita/res/psp/*
	rm -f PSVita/res/psx/*
	rm -f PSVita/res/rif/*
	rm -rf PSVita/res/save/ARK_01234
	rm -rf $(CHOVYSIGNDIR)/output
	rm -f PSVita/loader/psp/eboot/psploader.iso
	rm -f PSVita/loader/psp/eboot/iso_files/psp_game/sysdir/*
	make -C PSP clean
	make -C Updater clean
	make -C Resources/Peops clean
	make -C Resources/PrxEncrypter clean
	make -C PSVita/loader/psp/eboot clean
	make -C PSVita/loader/psp/pboot clean
	make -C PSVita/loader/psx clean
	make -C PSVita/loader/psx/payload clean
