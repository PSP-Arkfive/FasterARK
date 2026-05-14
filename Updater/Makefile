PYTHON = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
BUILDTOOLS = $(PSPDEV)/share/psp-cfw-sdk/build-tools

TARGET = updater
C_OBJS = main.o updater.o
OBJS = $(C_OBJS)
all: $(TARGET).prx
CFLAGS = -std=c99 -Os -G0 -Wall
INCDIR = 
LIBDIR = 

LIBS = -ltinyfont -lya2d -lpng -lz -lpspgum -lpspgu -lpspvram -lpspmath -lpspsystemctrl_user -lpspkubridge -lpsppower

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS =
BUILD_PRX = 1

PSP_EBOOT_TITLE = ARK Updater
PSP_EBOOT_ICON = Resources/ICON0.PNG
PSP_EBOOT_PIC1 = Resources/PIC1.PNG

EXTRA_TARGETS = EBOOT.PBP

PSP_EBOOT_PSAR = ARK_01234.PKG

ARK_01234.PKG:
	$(Q)$(PYTHON) $(BUILDTOOLS)/pack/pack.py -p ARK_01234.PKG packlist.txt -s

all: ARK_01234.PKG $(EXTRA_TARGETS)
	$(Q)rm PARAM.SFO ARK_01234.PKG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

PARAM.SFO:
	$(Q)cp PARAM_UPDATER.SFO PARAM.SFO
