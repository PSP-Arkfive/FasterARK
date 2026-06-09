PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = psploader
OBJS = main.o kxploit.o loader.o ../../vitaflash.o ../../reboot.o stubs.o

CFLAGS = -std=c99 -Os -G0 -Wall
LDFLAGS =
LIBS = -lpspexploit

PSP_FW_VERSION=660
EXTRA_TARGETS = EBOOT.PBP
BUILD_PRX=1

include $(PSPSDK)/lib/build.mak
