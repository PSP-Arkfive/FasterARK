#include <pspsdk.h>
#include <pspctrl.h>

#include <colordebugger.h>
#include <tinyfont.h>

PSP_MODULE_INFO("ARK Vita Loader", 0, 1, 0);


int main(int argc, char** argv){
    colorDebug(0xFF00);
    tinyFontPrintTextScreen(msx, 10, 10, "Hello World from ARK-5 Vita Loader!", -1, NULL);

    while (1)
    {
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);
        if (pad.Buttons & (PSP_CTRL_CROSS|PSP_CTRL_CIRCLE))
        {
            break;
        }
        sceKernelDelayThread(10000);
    }
}
