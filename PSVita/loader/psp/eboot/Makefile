PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = psploader
OBJS = main.o

CFLAGS = -std=c99 -Os -G0 -Wall
LDFLAGS = -nostartfiles
LIBS =

PSP_FW_VERSION=660
EXTRA_TARGETS = EBOOT.PBP
BUILD_PRX=1

include $(PSPSDK)/lib/build.mak
