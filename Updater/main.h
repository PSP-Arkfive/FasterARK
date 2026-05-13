#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>

#define RIGHT  345
#define TOP    2

#define CLEAR_COLOR 0x00000000
#define WHITE_COLOR 0xFFFFFFFF
#define RED_COLOR    0x000000FF
#define GREEN_COLOR  0xFF00FF00
#define YELLOW_COLOR 0x00FFFF00

typedef struct
{
    u32 magic;
    u32 version;
    u32 param_offset;
    u32 icon0_offset;
    u32 icon1_offset;
    u32 pic0_offset;
    u32 pic1_offset;
    u32 snd0_offset;
    u32 elf_offset;
    u32 psar_offset;
} PBPHeader;

enum {
    INFO_MSG,
    WARNING_MSG,
    ERROR_MSG
};

extern PBPHeader pbp_header;
extern char* eboot_path;

extern char msg[256];
extern int msg_type;

extern char* cipl_type;
extern char** options;
extern int nopts;

int updateARK();

void setInfoMsg(int type, char* txt);
void ExitWithMessage(int type, int reboot, int milisecs, char *fmt, ...);
#define ErrorExit(_m_, _f_, ...)  ExitWithMessage(ERROR_MSG, 0, _m_, _f_, ##__VA_ARGS__)
#define NormalExit(_m_, _f_, ...) ExitWithMessage(INFO_MSG,  0, _m_, _f_, ##__VA_ARGS__)
#define RebootExit(_m_, _f_, ...) ExitWithMessage(INFO_MSG,  1, _m_, _f_, ##__VA_ARGS__)


#endif