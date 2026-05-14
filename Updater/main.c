#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <psppower.h>

#include <ya2d.h>
#include <tinyfont.h>
#include <systemctrl_ark.h>

#include "main.h"

PSP_MODULE_INFO("ARKUpdater", 0x800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH | PSP_THREAD_ATTR_VFPU);
/*
PSP_DISABLE_NEWLIB();
PSP_DISABLE_NEWLIB_PIPE_SUPPORT();
PSP_DISABLE_NEWLIB_SOCKET_SUPPORT();
PSP_DISABLE_NEWLIB_TIMEZONE_SUPPORT();
PSP_DISABLE_NEWLIB_CWD_SUPPORT();
PSP_DISABLE_AUTOSTART_PTHREAD();
*/

PBPHeader pbp_header;
char* eboot_path = NULL;

int working = 1;
char* curtext = NULL;

struct ya2d_texture* background;
struct ya2d_texture* icon;

char** options = NULL;
int nopts = 0;
char msg[256];
int msg_type = INFO_MSG;
int msg_colors[] = { GREEN_COLOR, YELLOW_COLOR, RED_COLOR };
char header[64];


void setInfoMsg(int type, char* txt){
    msg_type = type;
    strcpy(msg, txt);
}

void ExitWithMessage(int type, int reboot, int milisecs, char *fmt, ...) 
{
    va_list list;
    va_start(list, fmt);
    msg_type = type;
    vsprintf(msg, fmt, list);
    va_end(list);
    sceKernelDelayThread(milisecs*1000);
    if (reboot)
        scePowerRequestColdReset(0);
    else
        sceKernelExitGame(); 
}

void drawMenu(){
    // now draw our menu stuff
    int w = 260;
    int h = 100;
    int x = (480-w)/2;
    int y = (272-h)/2;

    u32 color = 0x80808000;

    // menu window
    ya2d_draw_rect(x, y, w, h, color, 1);

    int tl = strlen(header);
    int dx = ((w-8*tl)/2);
    ya2d_draw_rect(x+dx, y+5, 8*tl, 8, 0x8000ff00, 1);
    tinyFontPrintTextScreenBuf(ya2d_get_drawbuffer(), msx, x + dx, y+5, header, WHITE_COLOR, NULL);

    // menu items
    int cur_x;
    int cur_y = y + (h-(10*nopts))/2;
    for (int i=0; i<nopts; i++){
        int tl = strlen(options[i]);
        cur_x = x + ((w-(8*tl))/2);
        ya2d_draw_rect(cur_x, cur_y+4, 8*tl, 8, color&0x00FFFFFF, 1);
        if(i==0)
            tinyFontPrintTextScreenBuf(ya2d_get_drawbuffer(), msx, cur_x, cur_y+4, options[i], WHITE_COLOR, NULL);
        else
            tinyFontPrintTextScreenBuf(ya2d_get_drawbuffer(), msx, cur_x, cur_y+5, options[i], WHITE_COLOR, NULL);
        cur_y += 10;
    }

    if (msg[0]){
        tinyFontPrintTextScreenBuf(ya2d_get_drawbuffer(), msx, 480-8*strlen(msg), TOP+15, msg, msg_colors[msg_type], NULL);
    }
}

int drawthread(SceSize args, void *argp){
    
    while (working){
        ya2d_start_drawing();
        ya2d_clear_screen(CLEAR_COLOR);
        
        ya2d_draw_texture(background, 0, 0);
        ya2d_draw_texture(icon, 0, 272-icon->height);

        if (options != NULL && nopts > 0)
            drawMenu();

        ya2d_finish_drawing();
        ya2d_swapbuffers();
    }

    return 0;
}

void loadGraphics(int argc, char** argv){

    eboot_path = argv[0];
    
    int fd = sceIoOpen(argv[0], PSP_O_RDONLY, 0777);
    sceIoRead(fd, &pbp_header, sizeof(PBPHeader));
    sceIoClose(fd);
    
    ya2d_init();
    ya2d_set_vsync(1);
    background = ya2d_load_PNG_file_offset(argv[0], YA2D_PLACE_RAM, pbp_header.pic1_offset);
    icon = ya2d_load_PNG_file_offset(argv[0], YA2D_PLACE_RAM, pbp_header.icon0_offset);

    snprintf(header, sizeof(header), "ARK Updater %d.%d.%d", ARK_MAJOR_VERSION, ARK_MINOR_VERSION, ARK_MICRO_VERSION);

    SceUID thid = sceKernelCreateThread("draw_thread", &drawthread, 0x10, 0x20000, PSP_THREAD_ATTR_VSH|PSP_THREAD_ATTR_VFPU, NULL);
    if (thid >= 0){
        // start thread and wait for it to end
        sceKernelStartThread(thid, 0, NULL);
    }
}


int main(int argc, char** argv){

    loadGraphics(argc, argv);
    
    updateARK();

    sceKernelExitGame();
    
    return 0;
}