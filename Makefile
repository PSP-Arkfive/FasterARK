all:
#	PSP
	make -C PSP
	mkdir -p dist/PSP/GAME/FasterARK/
	mkdir -p dist/PSP/SAVEDATA/
	cp -r PSP/res/ARK_01234 dist/PSP/SAVEDATA/
	cp -r PSP/res/LIBS dist/PSP/
	cp -r PSP/res/CustomIPL dist/PSP/GAME/
	cp PSP/EBOOT.PBP dist/PSP/GAME/FasterARK/
	cd dist && zip -m -r FasterARK_psp.zip * && cd ..
#	PS Vita
	mkdir -p PSVita/build
	cd PSVita/build && cmake .. && make && cd ../../
	cp PSVita/build/FasterARK.vpk dist/FasterARK_psv.vpk

clean:
	rm -rf dist
	rm -rf PSVita/build
	make -C PSP clean
