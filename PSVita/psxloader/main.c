#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspsuspend.h>
#include <psputilsforkernel.h>
#include <psppower.h>

#include <systemctrl_ark.h>
#include <colordebugger.h>
#include <tinyfont.h>
#include <popsdisplay.h>

PSP_MODULE_INFO("ARK VitaPOPS Loader", 0, 1, 0);

void printScreen(const char* text){
    static int cur_y = 10;
    tinyFontPrintTextScreenBuf(g_vram_base, msx, 10, cur_y, text, -1, NULL);
    popsDisplaySoftRelocateVram(g_vram_base, NULL);
    cur_y += 10;
}

int psxloader_thread(unsigned int argc, void* argv){

    // initialize screen
    colorDebug(0);
    popsDisplayInit();

    printScreen("ARK-X loader started!");

    // trigger virtual exploit in ps1cfw_enabler
    int res = sceIoOpen("ms0:/__dokxploit__", 0, 0);

    if (res < 0){
        sceKernelExitGame();
        return 0;
    }

    printScreen("Opening ARKX.BIN");

    // open ARKX binloader
    SceUID fd = sceIoOpen(DEFAULT_ARK_PATH ARKX_BIN, PSP_O_RDONLY, 0);

    if (fd < 0){
        sceKernelExitGame();
        return 0;
    }

    printScreen("Reading ARKX.BIN into kram");

    // read binloader into KRAM at 0x88380000
    sceIoRead(fd, (void *)0x88380000, 0x80000);
    sceIoClose(fd);
    sceKernelDcacheWritebackAll();

    printScreen("Executing ARKX.BIN");

    // execute main function
    int (*libctime)(u32, u32) = (void*)&sceKernelLibcTime;
    libctime(0x08800000, 0x88380000);

    return 0;
}

int module_start(SceSize args, void* argp)
{

    // loader needs to be in a thread
    int thid = sceKernelCreateThread("psxloader", &psxloader_thread, 0x10, 0x20000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(thid, 0, NULL);

    return 0;
}
