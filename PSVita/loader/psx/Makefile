PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = psxloader
OBJS = main.o

CFLAGS = -std=c99 -Os -G0 -Wall

ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG=1
endif

LDFLAGS = -nostartfiles
LIBS = 
PSP_FW_VERSION=660

BUILD_PRX=1

include $(PSPSDK)/lib/build.mak

psxloader.cue:
	mkpsxiso psxloader.xml

main.c: psxloader.cue

clean:
	rm -f *.elf *.prx *.o *.bin *.cue
