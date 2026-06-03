.PHONY: psp vita chovysign updater

CHOVYSIGN = ./Resources/Chovy-Sign

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
#	PS Vita Standalone Installer
	mkdir -p dist
	cp -r Resources/ARK_01234 PSVita/res/save/
	cp -r Resources/Extras/* PSVita/res/save/ARK_01234/
	cp -r Resources/PSVita/* PSVita/res/save/ARK_01234/
	cp -r Resources/LIBS/peops.prx PSVita/res/save/ARK_01234/PS1SPU.PRX
	mkdir -p PSVita/build
	cd PSVita/build && cmake .. && make && cd ../../
	cp PSVita/build/FasterARK.vpk dist/FasterARK_psvita.vpk
	rm -rf PSVita/res/save/ARK_01234

chovysign:
#   Generate Chovy-Sign binaries
	make -C PSVita/psxloader
	make -C PSVita/psxloader/payload
	$(CHOVYSIGN)/ChovySign-CLI --pops PSVita/psxloader/psxloader.cue --pops-info "ARK-X" PSVita/psxloader/ICON0.PNG --pops-eboot PSVita/psxloader/psxloader.prx --no-psvimg --nopspemudrm EP0099-SCPS10084_00-CHOVYSIGN0000000
	cp PSVita/psxloader/payload/ARKX.BIN PSVita/res/psx/
	cp $(CHOVYSIGN)/output/PSP/LICENSE/EP0099-SCPS10084_00-CHOVYSIGN0000000.rif PSVita/res/rif/psx.rif
	cp $(CHOVYSIGN)/output/PSP/GAME/SCPS10084/EBOOT.PBP PSVita/res/psx/

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
	make -C Updater
	mkdir -p dist/tmp/PSP/GAME/UPDATE/
	cp Updater/EBOOT.PBP dist/tmp/PSP/GAME/UPDATE/
	cd dist/tmp/ && zip -m -r ARK_UPDATE.zip * && cd ../../ && mv dist/tmp/ARK_UPDATE.zip dist/
	rm -r dist/tmp/
	rm -rf Updater/Resources/LIBS
	rm -rf Updater/Resources/ARK_01234

clean:
	rm -rf dist
	rm -rf PSVita/build
	rm -rf $(CHOVYSIGN)/output
	make -C PSP clean
	make -C Updater clean
	make -C PSVita/psxloader clean
	make -C PSVita/psxloader/payload clean
