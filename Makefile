all:
	make -C PSP
#	PSP Lite Install
	mkdir -p dist/tmp/PSP/GAME/FasterARK/
	mkdir -p dist/tmp/PSP/SAVEDATA/
	cp -r PSP/res/ARK_01234 dist/tmp/PSP/SAVEDATA/
	cp -r PSP/res/CustomIPL dist/tmp/PSP/GAME/
	cp -r PSP/res/LIBS/ipl_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp -r PSP/res/LIBS/kbooti_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp -r PSP/res/LIBS/kpspident.prx dist/tmp/PSP/GAME/CustomIPL/
	cp PSP/EBOOT.PBP dist/tmp/PSP/GAME/FasterARK/
	cp -r PSP/res/LIBS/usbdevice.prx dist/tmp/PSP/GAME/FasterARK/
	cp -r PSP/res/LIBS/idsregeneration.prx dist/tmp/PSP/GAME/FasterARK/
	cd dist/tmp/ && zip -m -r FasterARK_psp_lite.zip * && cd ../../ && mv dist/tmp/FasterARK_psp_lite.zip dist/
#	PSP Full Install
	mkdir -p dist/tmp/PSP/GAME/FasterARK/
	mkdir -p dist/tmp/PSP/SAVEDATA/
	cp -r PSP/res/ARK_01234 dist/tmp/PSP/SAVEDATA/
	cp PSP/res/Extras/* dist/tmp/PSP/SAVEDATA/ARK_01234/
	cp -r PSP/res/CustomIPL dist/tmp/PSP/GAME/
	cp -r PSP/res/LIBS/ipl_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp -r PSP/res/LIBS/kbooti_update.prx dist/tmp/PSP/GAME/CustomIPL/
	cp -r PSP/res/LIBS/kpspident.prx dist/tmp/PSP/GAME/CustomIPL/
	cp PSP/EBOOT.PBP dist/tmp/PSP/GAME/FasterARK/
	cp -r PSP/res/LIBS/usbdevice.prx dist/tmp/PSP/GAME/FasterARK/
	cp -r PSP/res/LIBS/idsregeneration.prx dist/tmp/PSP/GAME/FasterARK/
	cd dist/tmp/ && zip -m -r FasterARK_psp_full.zip * && cd ../../ && mv dist/tmp/FasterARK_psp_full.zip dist/
	rm -r dist/tmp/
#	PS Vita Standalone Installer
	mkdir -p PSVita/build
	cd PSVita/build && cmake .. && make && cd ../../
	cp PSVita/build/FasterARK.vpk dist/FasterARK_psvita.vpk

clean:
	rm -rf dist
	rm -rf PSVita/build
	make -C PSP clean
