#include <pspsdk.h>
#include <pspdisplay.h>
#include <pspgu.h>

#include <systemctrl_ark.h>
#include <rebootexconfig.h>
#include <libpspexploit.h>
#include <ya2d.h>
#include <tinyfont.h>

PSP_MODULE_INFO("FasterARK PSP", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);

static KernelFunctions _ktbl;
KernelFunctions* k_tbl = &_ktbl;

#define CLEAR_COLOR 0x00000000
#define WHITE_COLOR 0xFFFFFFFF

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


struct ya2d_texture* background;
struct ya2d_texture* icon;

extern void kmain();

int drawthread(SceSize args, void *argp){
    
    while (working){
        ya2d_start_drawing();
        ya2d_clear_screen(CLEAR_COLOR);
        
        ya2d_draw_texture(background, 0, 0);
        ya2d_draw_texture(icon, 0, 0);

        if (curtext){
            ya2d_draw_rect(100, 100, 8*strlen(curtext), 8, 0x8000ff00, 1);
            tinyFontPrintTextScreenBuf(ya2d_get_drawbuffer(), msx, 100, 100, curtext, WHITE_COLOR, NULL);
        }

        ya2d_finish_drawing();
        ya2d_swapbuffers();
    }

    return 0;
}

void loadGraphics(int argc, char** argv){

    PBPHeader header;
    
    int fd = sceIoOpen(argv[0], PSP_O_RDONLY, 0777);
    sceIoRead(fd, &header, sizeof(PBPHeader));
    sceIoClose(fd);
    
    ya2d_init();
    ya2d_set_vsync(1);
    background = ya2d_load_PNG_file_offset(argv[0], YA2D_PLACE_RAM, header.pic1_offset);
    icon = ya2d_load_PNG_file_offset(argv[0], YA2D_PLACE_RAM, header.icon0_offset);

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
            pspXploitExecuteKernel(kmain);
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