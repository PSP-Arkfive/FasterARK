PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = psxloader
OBJS = main.o

CFLAGS = -std=c99 -Os -G0 -Wall

ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG=1
endif

LDFLAGS = -nostartfiles
LIBS = -lcolordebugger -ltinyfont -lpopsdisplay
PSP_FW_VERSION=660

BUILD_PRX=1

include $(PSPSDK)/lib/build.mak

psxloader.cue:
	$(Q)mkpsxiso psxloader.xml

main.c: psxloader.cue

clean:
	$(Q)rm -f *.elf *.prx *.o *.bin *.cue

# To generate the EBOOT.PBP:
# ChovySign-CLI --pops psxloader.cue --pops-info "ARK-X" ICON0.PNG --pops-eboot psxloader.prx --no-psvimg --nopspemudrm EP0099-SCPS10084_00-CHOVYSIGN0000000