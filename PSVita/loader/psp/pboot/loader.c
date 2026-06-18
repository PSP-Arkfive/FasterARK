#include <pspsdk.h>

#include <cfwmacros.h>
#include <systemctrl_ark.h>
#include <libpspexploit.h>
#include <colordebugger.h>
#include <tinyfont.h>

#include "kxploit.h"
#include "payload.h"


ARKConfig ark_config = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH, // only ms0 available
    .launcher = VBOOT_PBP, // use xMenu
    .exec_mode = PS_VITA, // set to VitaPops mode
    .exploit_id = "ePSP", // ps1 loader name
    .recovery = 0, // no recovery available
};

KernelFunctions k_tbl;
u8* rebootbuffer = rebootbuffer_vita;

extern int patchKermitPeripheral();
extern void patchLoadExec();
extern int reboot_launcher();


void kmain(){
    pspSdkSetK1(0);
    pspXploitRepairKernel();
    pspXploitScanKernelFunctions(&k_tbl);
    patchKermitPeripheral();
    patchLoadExec();
    reboot_launcher();
}
