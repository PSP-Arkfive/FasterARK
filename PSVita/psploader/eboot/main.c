#include <pspsdk.h>
#include <psploadexec.h>

PSP_MODULE_INFO("ARK Vita Loader Eboot", 0, 1, 0);


int module_start(int argc, void* argp){
    sceKernelExitGame();
    return 0;
}
