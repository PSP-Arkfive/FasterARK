.PHONY: psp vita updater translations resources ark150_latest customipl_latest launchermenu_latest libs_latest dc10_latest

PY = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
BUILDTOOLS = $(PSPDEV)/share/psp-cfw-sdk/build-tools

CHOVYSIGNDIR = ./Resources/Chovy-Sign
CHOVYSIGN = $(CHOVYSIGNDIR)/ChovySign-CLI
LANGFOLDER = Resources/Language/Translations/resources
ARK5_TMP = tmp
FLASH0_ARK = $(ARK5_TMP)/FLASH0.ARK
ARK5_ARCHIVE = $(ARK5_TMP)/ARK-5.zip
FLASH0_ARK_URL = https://github.com/PSP-Arkfive/ARK-5/releases/download/latest/ARK-5.zip
ARK150_DIR = $(ARK5_TMP)/ARK150on660
ARK150_ARCHIVE = $(ARK5_TMP)/ARK-150.zip
ARK150_URL = https://github.com/PSP-Arkfive/ARK-150/releases/download/latest/ARK-150.zip
CUSTOMIPL_DIR = $(ARK5_TMP)/CustomIPL
CUSTOMIPL_ARCHIVE = $(ARK5_TMP)/CustomIPL.zip
CUSTOMIPL_URL = https://github.com/PSP-Arkfive/CustomIPL/releases/download/latest/CustomIPL.zip
LAUNCHERMENU_ARCHIVE = $(ARK5_TMP)/Launcher-Menus.zip
LAUNCHERMENU_URL = https://github.com/PSP-Arkfive/Launcher-Menus/releases/download/latest/Launcher-Menus.zip
LIBS_DIR = $(ARK5_TMP)/LIBS
LIBS_ARCHIVE = $(ARK5_TMP)/LIBS.zip
LIBS_URL = https://github.com/pspdev/psp-dynamic-libraries/releases/download/latest/LIBS.zip
DC10_DIR = $(ARK5_TMP)/DC10
DC10_ARCHIVE = $(ARK5_TMP)/DC10.zip
DC10_URL = https://github.com/PSP-Arkfive/Despertar-Del-Cementerio/releases/download/latest/DC10.zip

all: translations themes psp vita updater
	echo "All Done!"

resources: $(FLASH0_ARK) ark150_latest customipl_latest launchermenu_latest libs_latest dc10_latest

$(FLASH0_ARK):
	mkdir -p $(ARK5_TMP)
	curl --fail --location --silent --show-error "$(FLASH0_ARK_URL)" -o "$(ARK5_ARCHIVE)"
	unzip -p "$(ARK5_ARCHIVE)" FLASH0.ARK > "$@"

ark150_latest:
	mkdir -p $(ARK5_TMP)
	curl --fail --location --silent --show-error "$(ARK150_URL)" -o "$(ARK150_ARCHIVE)"
	unzip -o "$(ARK150_ARCHIVE)" "ARK150on660/*" -d "$(ARK5_TMP)"

customipl_latest:
	mkdir -p $(ARK5_TMP)
	curl --fail --location --silent --show-error "$(CUSTOMIPL_URL)" -o "$(CUSTOMIPL_ARCHIVE)"
	unzip -o "$(CUSTOMIPL_ARCHIVE)" "CustomIPL/*" -d "$(ARK5_TMP)"

launchermenu_latest:
	mkdir -p $(ARK5_TMP)
	curl --fail --location --silent --show-error "$(LAUNCHERMENU_URL)" -o "$(LAUNCHERMENU_ARCHIVE)"
	unzip -o "$(LAUNCHERMENU_ARCHIVE)" "*" -d "$(ARK5_TMP)"

libs_latest:
	mkdir -p $(LIBS_DIR)
	curl --fail --location --silent --show-error "$(LIBS_URL)" -o "$(LIBS_ARCHIVE)"
	unzip -o "$(LIBS_ARCHIVE)" -d "$(LIBS_DIR)"

dc10_latest:
	mkdir -p $(ARK5_TMP)
	curl --fail --location --silent --show-error "$(DC10_URL)" -o "$(DC10_ARCHIVE)"
	unzip -o "$(DC10_ARCHIVE)" "DC10/*" -d "$(ARK5_TMP)"

translations:
	$(PY) $(BUILDTOOLS)/pftools/bdf_to_pf.py $(LANGFOLDER)/satelite_chs_utf8.txt Resources/Language/quan.bdf $(LANGFOLDER)/satelite_chs.txt $(LANGFOLDER)/CHS.pf
	$(PY) $(BUILDTOOLS)/pftools/bdf_to_pf.py $(LANGFOLDER)/satelite_cht_utf8.txt Resources/Language/quan.bdf $(LANGFOLDER)/satelite_cht.txt $(LANGFOLDER)/CHT.pf
	$(PY) $(BUILDTOOLS)/pftools/bdf_to_pf.py $(LANGFOLDER)/satelite_jp_utf8.txt Resources/Language/quan.bdf $(LANGFOLDER)/satelite_jp.txt $(LANGFOLDER)/JP.pf
	$(PY) $(BUILDTOOLS)/pftools/bdf_to_pf.py $(LANGFOLDER)/satelite_kr_utf8.txt Resources/Language/quan.bdf $(LANGFOLDER)/satelite_kr.txt $(LANGFOLDER)/KR.pf
	$(PY) $(BUILDTOOLS)/pack/pkg-res.py Resources/Language LANG.ARK

themes: launchermenu_latest
	mkdir -p dist/tmp/
	mv tmp/themes dist/tmp/
	cd dist/tmp/ && zip -m -r themes.zip * && cd ../../ && mv dist/tmp/themes.zip dist/
	rm -rf dist/tmp

psp: translations resources
	make -C PSP
#	PSP Lite Install
	mkdir -p dist/tmp/PSP/GAME/FasterARK/
	mkdir -p dist/tmp/PSP/SAVEDATA/
	cp -r Resources/ARK_01234 dist/tmp/PSP/SAVEDATA/
	cp $(FLASH0_ARK) dist/tmp/PSP/SAVEDATA/ARK_01234/
	cp -r $(CUSTOMIPL_DIR) dist/tmp/PSP/GAME/
	cp PSP/EBOOT.PBP dist/tmp/PSP/GAME/FasterARK/
	cp $(LIBS_DIR)/ipl_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp $(LIBS_DIR)/kbooti_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp $(LIBS_DIR)/kpspident.prx dist/tmp/PSP/GAME/CustomIPL/
	cp $(LIBS_DIR)/usbdevice.prx dist/tmp/PSP/GAME/FasterARK/
	cp $(LIBS_DIR)/idsregeneration.prx dist/tmp/PSP/GAME/FasterARK/
	cd dist/tmp/ && zip -m -r FasterARK_psp_lite.zip * && cd ../../ && mv dist/tmp/FasterARK_psp_lite.zip dist/
#	PSP Full Install
	mkdir -p dist/tmp/PSP/GAME/FasterARK/
	mkdir -p dist/tmp/PSP/SAVEDATA/
	cp -r Resources/ARK_01234 dist/tmp/PSP/SAVEDATA/
	cp $(FLASH0_ARK) dist/tmp/PSP/SAVEDATA/ARK_01234/
	cp Resources/Extras/* dist/tmp/PSP/SAVEDATA/ARK_01234/
	cp tmp/VBOOT.PBP dist/tmp/PSP/SAVEDATA/ARK_01234/
	cp tmp/VSHMENU.PRX dist/tmp/PSP/SAVEDATA/ARK_01234/
	cp tmp/THEME.ARK dist/tmp/PSP/SAVEDATA/ARK_01234/
	cp Resources/Language/Translations/LANG.ARK dist/tmp/PSP/SAVEDATA/ARK_01234/
	cp -r $(CUSTOMIPL_DIR) dist/tmp/PSP/GAME/
	cp -r $(DC10_DIR) dist/tmp/PSP/GAME/
	cp -r $(ARK150_DIR) dist/tmp/PSP/GAME/
	cp -r Resources/OC_Tester dist/tmp/PSP/GAME/
	cp PSP/EBOOT.PBP dist/tmp/PSP/GAME/FasterARK/
	cp $(LIBS_DIR)/usbdevice.prx dist/tmp/PSP/GAME/FasterARK/
	cp $(LIBS_DIR)/idsregeneration.prx dist/tmp/PSP/GAME/FasterARK/
	cp $(LIBS_DIR)/ipl_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp $(LIBS_DIR)/kbooti_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp $(LIBS_DIR)/kpspident.prx dist/tmp/PSP/GAME/CustomIPL/
	cp $(LIBS_DIR)/iop.prx dist/tmp/PSP/GAME/DC10/
	cp $(LIBS_DIR)/ipl_update.prx dist/tmp/PSP/GAME/DC10/
	cp $(LIBS_DIR)/usbdevice.prx dist/tmp/PSP/GAME/DC10/
	cp $(LIBS_DIR)/idsregeneration.prx dist/tmp/PSP/GAME/DC10/
	cp $(LIBS_DIR)/vlf.prx dist/tmp/PSP/GAME/DC10/
	cp $(LIBS_DIR)/intraFont-vlf.prx dist/tmp/PSP/GAME/DC10/
	cp $(LIBS_DIR)/pspdecrypt.prx dist/tmp/PSP/GAME/DC10/
	cp $(LIBS_DIR)/libpsardumper.prx dist/tmp/PSP/GAME/DC10/
	cp $(LIBS_DIR)/lflash_fdisk.prx dist/tmp/PSP/GAME/DC10/
	cp $(LIBS_DIR)/pspdecrypt.prx dist/tmp/PSP/GAME/ARK150on660/
	cp $(LIBS_DIR)/libpsardumper.prx dist/tmp/PSP/GAME/ARK150on660/
	cd dist/tmp/ && zip -m -r FasterARK_psp_full.zip * && cd ../../ && mv dist/tmp/FasterARK_psp_full.zip dist/
	rm -r dist/tmp/

vita: translations resources
	mkdir -p dist
	mkdir -p PSVita/res/save
	mkdir -p PSVita/loader/psp/eboot/iso_files/psp_game/sysdir
	mkdir -p PSVita/res/psp/
	mkdir -p PSVita/res/rif
	mkdir -p PSVita/res/psx/
	cp -r Resources/ARK_01234 PSVita/res/save/
	cp $(FLASH0_ARK) PSVita/res/save/ARK_01234/
	cp -r Resources/Extras/* PSVita/res/save/ARK_01234/
	cp -r Resources/PSVita/* PSVita/res/save/ARK_01234/
	cp tmp/XBOOT.PBP PSVita/res/save/ARK_01234/
	cp tmp/VBOOT.PBP PSVita/res/save/ARK_01234/
	cp tmp/VSHMENU.PRX PSVita/res/save/ARK_01234/
	cp tmp/THEME.ARK PSVita/res/save/ARK_01234/
	cp Resources/Language/Translations/LANG.ARK PSVita/res/save/ARK_01234/
#   ePSP Bubble
	cp Resources/ARK_01234/ICON0.PNG PSVita/loader/psp/eboot/iso_files/psp_game/
	make -C PSVita/loader/psp/eboot
	cp PSVita/loader/psp/eboot/psploader.prx PSVita/loader/psp/eboot/iso_files/psp_game/sysdir/boot.bin
	mkisofs -o PSVita/loader/psp/eboot/psploader.iso PSVita/loader/psp/eboot/iso_files/
	$(CHOVYSIGN) --psp PSVita/loader/psp/eboot/psploader.iso --no-psvimg --nopspemudrm EP0099-NPUZ01234_00-CHOVYSIGN0000000
	cp $(CHOVYSIGNDIR)/output/PSP/LICENSE/EP0099-NPUZ01234_00-CHOVYSIGN0000000.rif PSVita/res/rif/psp.rif
	cp $(CHOVYSIGNDIR)/output/PSP/GAME/NPUZ01234/EBOOT.PBP PSVita/res/psp/
	cp Resources/ARK_01234/ICON0.PNG PSVita/loader/psp/pboot/
	make -C PSVita/loader/psp/pboot
	cp PSVita/loader/psp/pboot/PBOOT.PBP PSVita/res/psp/
	make -C Resources/Peops
	$(PY) $(BUILDTOOLS)/gz/pspgz.py PSVita/res/save/ARK_01234/PS1SPU.PRX $(BUILDTOOLS)/gz/UserModule.hdr Resources/Peops/peops.prx peops 0x0000
#   ePSX Bubble
	make -C PSVita/loader/psx
	make -C PSVita/loader/psx/payload
	cp PSVita/loader/psx/payload/ARKX.BIN PSVita/res/psx/
	$(CHOVYSIGN) --pops PSVita/loader/psx/psxloader.cue --pops-info "ARK-X" PSVita/loader/psx/ICON0.PNG --pops-eboot PSVita/loader/psx/psxloader.prx --no-psvimg --nopspemudrm EP0099-SCPS10084_00-CHOVYSIGN0000000
	cp $(CHOVYSIGNDIR)/output/PSP/LICENSE/EP0099-SCPS10084_00-CHOVYSIGN0000000.rif PSVita/res/rif/psx.rif
	cp $(CHOVYSIGNDIR)/output/PSP/GAME/SCPS10084/EBOOT.PBP PSVita/res/psx/
#	Installer
	mkdir -p PSVita/build
	cd PSVita/build && cmake .. && make && cd ../../
	cp PSVita/build/FasterARK.vpk dist/FasterARK_psvita.vpk

updater: translations resources
#	Updater
	mkdir -p dist/tmp/
	cp -r $(LIBS_DIR) Updater/Resources/LIBS
	cp -r Resources/ARK_01234 Updater/Resources/
	cp $(FLASH0_ARK) Updater/Resources/ARK_01234/
	cp $(DC10_DIR)/DC10.ARK Updater/Resources/ARK_01234/
	cp $(ARK150_DIR)/FLASH150.ARK Updater/Resources/ARK_01234/
	cp $(ARK150_DIR)/LANG150.ARK Updater/Resources/ARK_01234/
	cp Resources/Language/Translations/LANG.ARK Updater/Resources/ARK_01234/
	cp tmp/VBOOT.PBP Updater/Resources/ARK_01234/
	cp tmp/VSHMENU.PRX Updater/Resources/ARK_01234/
	cp tmp/XBOOT.PBP Updater/Resources/ARK_01234/
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
	rm -rf $(ARK5_TMP)
	rm -rf PSVita/build
	rm -f PSVita/res/psp/*
	rm -f PSVita/res/psx/*
	rm -f PSVita/res/rif/*
	rm -rf PSVita/res/save/ARK_01234
	rm -rf $(CHOVYSIGNDIR)/output
	rm -f Resources/Language/Translations/LANG.ARK
	rm -f PSVita/loader/psp/eboot/psploader.iso
	rm -f PSVita/loader/psp/eboot/iso_files/psp_game/sysdir/*
	rm -f PSVita/loader/psp/eboot/iso_files/psp_game/ICON0.PNG
	rm -f PSVita/loader/psp/pboot/ICON0.PNG
	make -C PSP clean
	make -C Updater clean
	make -C Resources/Peops clean
	make -C PSVita/loader/psp/eboot clean
	make -C PSVita/loader/psp/pboot clean
	make -C PSVita/loader/psx clean
	make -C PSVita/loader/psx/payload clean
