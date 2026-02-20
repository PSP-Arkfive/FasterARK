TARGET = fasterark
OBJS = main.o kernel.o

UNAME := $(shell uname)

CFLAGS = -O2 -Os -G0 -Wall -std=c99
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
ENCRYPT = 1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP FasterARK
PSP_EBOOT_ICON = res/icon0.png
PSP_EBOOT_PIC1 = res/pic1.png

LIBDIR = 
LIBS = -lpspexploit -lpsprtc -lmini2d -lpng -lz -lpspgu -lpspgum

all: $(EXTRA_TARGETS)
	mkdir -p dist/PSP/GAME/FasterARK/
	mkdir -p dist/PSP/SAVEDATA/
	cp -r res/ARK_01234 dist/PSP/SAVEDATA/
	cp -r res/LIBS dist/PSP/
	cp EBOOT.PBP dist/PSP/GAME/FasterARK/
	cd dist && zip -r FasterARK_psp.zip * && cd ..

distclean: clean
	rm -rf dist

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak