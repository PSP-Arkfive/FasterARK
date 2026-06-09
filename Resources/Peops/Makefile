PSPSDK=$(shell psp-config --pspsdk-path)

TARGET = peops
SPU_OBJS = spu/src/decode_xa.o spu/src/spu.o spu/src/registers.o spu/src/xa.o
OBJS = main.o exports.o src/psp.o src/audio.o $(SPU_OBJS)

INCDIR = include spu/include
CFLAGS = -O3 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_PSPSDK_LIBS = 1
USE_PSPSDK_LIBC = 1

LIBDIR = 
LIBS = -lpspsystemctrl_user -lpspaudio -lpspsysc_user

include $(PSPSDK)/lib/build_prx.mak
