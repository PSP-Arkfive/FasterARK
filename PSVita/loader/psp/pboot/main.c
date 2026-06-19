#include <pspsdk.h>
#include <psploadexec.h>

#include <libpspexploit.h>

#include "loader.h"

PSP_MODULE_INFO("ARK Vita Loader PBOOT", 0, 1, 0);


int main(int argc, char** args){

    pspXploitConfigure(&kreader_netmcopyback, &kwriter_fakeuid);
    pspXploitInitKernelExploit();
    pspXploitDoKernelExploit();
    pspXploitExecuteKernel(kmain);

    sceKernelExitGame();
    return 0;
}
