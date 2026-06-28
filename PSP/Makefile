TARGET = fasterark
OBJS = main.o kernel.o

UNAME := $(shell uname)

CFLAGS = -O2 -Os -G0 -Wall -std=c99
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS =

BUILD_PRX = 1
ENCRYPT = 1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP FasterARK
PSP_EBOOT_ICON = res/icon0.png
PSP_EBOOT_PIC0 = res/pic0.png
PSP_EBOOT_PIC1 = res/pic1.png
PSP_EBOOT_ICON1 = res/icon1.pmf
PSP_EBOOT_SND0 = res/snd0.at3

LIBDIR = 
LIBS = -lpspexploit -lya2d -ltinyfont -lpng -lz -lpspmath -lpspvram -lpspgum -lpspgu -lpsprtc

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
