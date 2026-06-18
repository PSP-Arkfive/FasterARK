PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = psploader
OBJS = main.o loader.o ../../vitaflash.o ../../reboot.o

CFLAGS = -std=c99 -Os -G0 -Wall
LDFLAGS =
LIBS = -lpspexploit

PSP_FW_VERSION=660
EXTRA_TARGETS = PBOOT.PBP
BUILD_PRX=1
ENCRYPT=2
PSP_EBOOT_TITLE = ARK
PSP_EBOOT_ICON = ICON0.PNG

include $(PSPSDK)/lib/build.mak
