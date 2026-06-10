PSPSDK=$(shell psp-config --pspsdk-path)
CFWSDK = $(PSPDEV)/share/psp-cfw-sdk
INCDIR = $(PSPSDK)/include
LIBS = -L $(PSPSDK)/lib -lpspexploit -lpopsdisplay -lansic
TARGET = ARKX.BIN

PYTHON=$(shell which python3)

all: pre-build $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR))

C_OBJS = main.o ../../vitaflash.o ../../reboot.o


LINKFILE = link.l

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy

pre-build: link.l

cmd_genlink = $(PYTHON) $(CFWSDK)/build-tools/genlink.py 0x88380000 exploitEntry > $(LINKFILE)

link.l:
	$(cmd_genlink)

main.elf: $(C_OBJS)

$(TARGET): main.elf
	$(STRIP) -s $<
	$(OBJCOPY) -O binary $< $@
	@echo GET $@

clean:
	rm -rf *~ *.s *.o *.elf $(TARGET) exploit.h link.l

include $(CFWSDK)/build-tools/beauty_bin.mak