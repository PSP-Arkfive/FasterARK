#include <pspsdk.h>
#include <pspdisplay.h>
#include <pspgu.h>

#include <ark.h>
#include <rebootconfig.h>
#include <libpspexploit.h>
#include <mini2d.h>

PSP_MODULE_INFO("Flash Dumper", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);

static KernelFunctions _ktbl;
KernelFunctions* k_tbl = &_ktbl;

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

int working = 1;
char* curtext = NULL;

Image* background;
Image* icon;

extern void kmain();

int drawthread(SceSize args, void *argp){
    
    while (working){
        clearScreen(CLEAR_COLOR);
        
        blitAlphaImageToScreen(0, 0, 480, 272, background, 0, 0);
        blitAlphaImageToScreen(0, 0, 80, 80, icon, 0, 0);

        if (curtext)
            printTextScreen(100, 100, curtext, WHITE_COLOR);

        flipScreen();
    }

    return 0;
}

void loadGraphics(int argc, char** argv){
    initGraphics();

    PBPHeader header;
    
    int fd = sceIoOpen(argv[0], PSP_O_RDONLY, 0777);
    sceIoRead(fd, &header, sizeof(PBPHeader));
    sceIoClose(fd);
    
    background = loadImage(argv[0], header.pic1_offset);
    icon = loadImage(argv[0], header.icon0_offset);

    SceUID thid = sceKernelCreateThread("draw_thread", &drawthread, 0x10, 0x20000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    if (thid >= 0){
        // start thread and wait for it to end
        sceKernelStartThread(thid, 0, NULL);
    }
}

int main(int argc, char** argv){

    int res = 0;

    loadGraphics(argc, argv);

    curtext = "ARK-5 Loader Started.";
    
    curtext = "Initializing kernel exploit...";
    res = pspXploitInitKernelExploit();

    if (res == 0){

        curtext = "Corrupting kernel...";
        res = pspXploitDoKernelExploit();
        
        if (res == 0){
            pspXploitExecuteKernel((u32)kmain);
        }
        else {
            curtext = "ERROR executing kernel exploit";
        }
    
    }
    else{
        curtext = "ERROR initializing libpspexploit";
    }

    sceKernelExitGame();
    return 0;
}