#include <pspsdk.h>
#include <psploadexec.h>

#include "kxploit.h"
#include "loader.h"

PSP_MODULE_INFO("ARK Vita Loader PBOOT", 0, 1, 0);


int main(int argc, char** args){

    initKernelExploit();
    doKernelExploit();
    executeKernel((u32)kmain);

    sceKernelExitGame();
    return 0;
}
