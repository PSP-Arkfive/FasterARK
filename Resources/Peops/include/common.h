#ifndef COMMON_H
#define COMMON_H    

#include <pspkernel.h>
#include <psploadcore.h>
#include <pspaudio.h>

enum ReverbType{
    REVERB_OFF = 0x0000,
    REVERB_ROOM = 0x007D,
    REVERB_STUDIO_SMALL = 0x0033,
    REVERB_STUDIO_MEDIUM = 0x00B1,
    REVERB_STUDIO_LARGE = 0x00E3,
    REVERB_HALL = 0x01A5,
    REVERB_SPACE_ECHO = 0x033D,
    REVERB_ECHO_DELAY = 0x0001,
    REVERB_HALF_ECHO = 0x0017,
};

enum InterpolationType{
    INTERPOLATION_OFF,
    INTERPOLATION_SIMPLE,
    INTERPOLATION_GAUSS,
};

enum SPU_UpdateMode{
    SPU_WAITVBLANK = 0,
    SPU_DELAY = 100,
};

enum SPU_Priorities{
    SPU_PRIORITY_VERY_LOW = 17,
    SPU_PRIORITY_LOW = 24,
    SPU_PRIORITY_MEDIUM = 32,
    SPU_PRIORITY_HIGH = 40,
    SPU_PRIORITY_VERY_HIGH = 48,
};

#endif
