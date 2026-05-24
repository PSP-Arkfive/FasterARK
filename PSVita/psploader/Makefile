PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = psploader
OBJS = main.o

CFLAGS = -std=c99 -Os -G0 -Wall

ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG=1
endif

LDFLAGS = -nostartfiles
LIBS = -lcolordebugger -ltinyfont
PSP_FW_VERSION=660

BUILD_PRX=1
ENCRYPT_PRX=1

include $(PSPSDK)/lib/build.mak

all: $(TARGET)
	$(Q)cp $(TARGET).elf iso_files/psp_game/sysdir/boot.bin
	$(Q)cp $(TARGET).prx iso_files/psp_game/sysdir/eboot.bin
	$(Q)mkisofs -o psploader.iso iso_files/

clean:
	$(Q)rm -f *.elf *.prx *.o *.bin *.cue

# To generate the EBOOT.PBP:
# ChovySign-CLI --psp psploader.iso --no-psvimg --nopspemudrm EP0099-NPUZ01234_00-CHOVYSIGN0000000