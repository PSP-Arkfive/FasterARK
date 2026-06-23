#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pspsdk.h>
#include <pspumd.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>
#include <bootloadex.h>
#include <rebootexconfig.h>
#include <popsdisplay.h>
#include <libpspexploit.h>
#include <tinyfont.h>
#include <colordebugger.h>

#include "payload.h"


ARKConfig ark_config = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH, // only ms0 available
    .launcher = ARK_XMENU, // use xMenu
    .exec_mode = PSV_POPS, // set to VitaPops mode
    .exploit_id = "ePSX", // ps1 loader name
    .recovery = 0, // no recovery available
};

KernelFunctions k_tbl;
u8* rebootbuffer = rebootbuffer_vitapops;

extern int patchKermitPeripheral();
extern void patchLoadExec();
extern int rebootLauncher();


// Fake K1 Kernel Setting
void setK1Kernel(void){
    // Set K1 to Kernel Value
    __asm__ (
        "nop\n"
        "lui $k1,0x0\n"
    );
}

void clearBSS(void){
    // BSS Start and End Address from Linkfile
    extern char __bss_start, __bss_end;
    // Clear Memory
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}

// load and start pops module
void loadstart_pops(){
    int (*LoadModule)() = (void*)pspXploitFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x939E4270);
    int (*StartModule)() = (void*)pspXploitFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x3FF74DF1);

    int modid = LoadModule("flash0:/kd/pops_01g.prx", 0, NULL);
    StartModule(modid, 0, NULL, NULL, NULL);
}

// kill pops before it crashes
void kill_pops(){
    // popsmain, mcworker and cdworker
    int (*KernelTerminateDeleteThread)(int) = (void*)pspXploitFindFunction("sceThreadManager", "ThreadManForUser", 0x383F7BCC);
    
    int popsmain = pspXploitGetThreadUIDByName("popsmain");
    KernelTerminateDeleteThread(popsmain);

    int mcworker = pspXploitGetThreadUIDByName("mcworker");
    KernelTerminateDeleteThread(mcworker);

    int cdworker = pspXploitGetThreadUIDByName("cdworker");
    KernelTerminateDeleteThread(cdworker);
}

int exploitEntry() __attribute__((section(".text.startup")));
int exploitEntry(){

    clearBSS();

    // Switch to Kernel Permission Level
    setK1Kernel();

    // resolve some useful functions
    pspXploitScanKernelFunctions(&k_tbl);

    // Extremely nasty solution to get screen working fine
    k_tbl.KernelDelayThread(1000000); // wait for system to finish booting up
    loadstart_pops(); // load and start pops module
    k_tbl.KernelDelayThread(1000000); // wait for pops to set up things
    kill_pops(); // kill pops threads to prevent crash

    // initialize screen
    popsDisplayInit();

    // patch and reboot
    patchKermitPeripheral();
    patchLoadExec();
    rebootLauncher();

    return 0;
}
