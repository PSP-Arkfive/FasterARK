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

extern int patchKermitPeripheral();

// Sony Reboot Buffer Loader
int (* _LoadReboot)(void *, unsigned int, void *, unsigned int) = NULL;
int (* _KernelLoadExecVSHWithApitype)(int, char *, struct SceKernelLoadExecVSHParam *, int);



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

void PS1PRT(char *fmt, ...) 
{
    static char msg[256];
    static int cur_y = 10;
    va_list list;
    va_start(list, fmt);
    vsnprintf(msg, sizeof(msg), fmt, list);
    va_end(list);
    tinyFontPrintTextScreen(msx, 10, cur_y, msg, -1, NULL);
    popsDisplaySoftRelocateVram((void*)0x44000000, NULL);
    cur_y+=10;
    if (cur_y+8>=272){
        colorDebug(0);
        cur_y = 10;
    }
}

// Build Reboot Configuration
void buildRebootBufferConfig(int rebootBufferSize)
{
    // Fetch Memory Range
    RebootexConfigARK* conf = (RebootexConfigARK*)(REBOOTEX_CONFIG);
    
    // Clear Reboot Configuration
    memset((void*)REBOOTEX_CONFIG, 0, sizeof(RebootexConfigARK));
    
    // Write Configuration Magic
    conf->magic = ARK_CONFIG_MAGIC;
    
    // Write PROCFW Reboot Buffer Size (for backup in System Control)
    conf->reboot_buffer_size = rebootBufferSize;

    // Default ISO driver for homebrew and ISO
    conf->iso_mode = 0;
    // Default ISO disc type
    conf->iso_disc_type = PSP_UMD_TYPE_GAME;
    
    // backup runtime ARK config
    memcpy((void*)ARK_CONFIG, &ark_config, sizeof(ARKConfig));
}

// PROCFW Reboot Buffer Loader
int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{
    // Copy PROCFW Reboot Buffer into Memory
    memset((void*)REBOOTEX_TEXT, 0, REBOOTEX_MAX_SIZE);
    int rebootBufferSize = REBOOTEX_MAX_SIZE;

    if (rebootbuffer_vitapops[0] == 0x1F && rebootbuffer_vitapops[1] == 0x8B) // gzip packed rebootex
        rebootBufferSize = k_tbl.KernelGzipDecompress((unsigned char *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE, rebootbuffer_vitapops, NULL);
    else // plain payload
        memcpy((void*)REBOOTEX_TEXT, rebootbuffer_vitapops, REBOOTEX_MAX_SIZE);
        
    // Build Configuration
    buildRebootBufferConfig(rebootBufferSize);
    
    // Load Sony Reboot Buffer
    return _LoadReboot(arg1, arg2, arg3, arg4);
}

// reboot to launcher
int reboot_launcher(){

    char menupath[ARK_PATH_SIZE];
    strcpy(menupath, ark_config.arkpath);
    strcat(menupath, ark_config.launcher);

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(menupath) + 1;
    param.argp = menupath;
    param.key = "game";

    PS1PRT("Running Menu at %s", menupath);
    return _KernelLoadExecVSHWithApitype(0x141, menupath, &param, 0x10000);
}

// load and start pops module
void loadstart_pops(){
    int (*LoadModule)() = (void*)pspXploitFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x939E4270);
    int (*StartModule)() = (void*)pspXploitFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x3FF74DF1);

    int modid = LoadModule("flash0:/kd/pops_01g.prx", 0, NULL);
    if (modid < 0){
        PS1PRT("modid: %p", modid);
        _sw(0, 0);
    }
    int res = StartModule(modid, 0, NULL, NULL, NULL);
    if (res < 0){
        PS1PRT("res: %p\n", res);
        _sw(0, 0);
    }
}

// kill pops before it crashes
void kill_pops(){
    // popsmain, mcworker and cdworker
    int (*KernelTerminateDeleteThread)(int) = (void*)pspXploitFindFunction("sceThreadManager", "ThreadManForUser", 0x383F7BCC);
    
    int popsmain = sctrlGetThreadUIDByName("popsmain");
    KernelTerminateDeleteThread(popsmain);

    int mcworker = sctrlGetThreadUIDByName("mcworker");
    KernelTerminateDeleteThread(mcworker);

    int cdworker = sctrlGetThreadUIDByName("cdworker");
    KernelTerminateDeleteThread(cdworker);
}

void patchLoadExec(SceModule *loadexec, u32 LoadReboot, u32 GetUserLevel, int k1_patches) {
  u32 addr = 0;
  u32 text_addr = loadexec->text_addr;
  u32 topaddr = text_addr + loadexec->text_size;

  u32 rebootcall = JAL(text_addr);
  u32 GetUserLevelJump = JUMP(GetUserLevel);

  int patches = 3 + k1_patches;
  for (addr = text_addr; addr < topaddr && patches; addr += 4) {
    u32 data = _lw(addr);
    if (data == 0x3C018860) {
      _sb(0xFC, addr); // Patch Reboot Buffer Entry Point
      patches--;
    } else if (data == rebootcall) {
      _sw(JAL(LoadReboot), addr); // Patch Reboot Buffer Loader
      patches--;
    } else if (data == GetUserLevelJump) {
      _sw(JR_RA, addr); // patch sceKernelGetUserLevel stub to make it return 4
      _sw(LI_V0(4), addr + 4);
      patches--;
    } else if ((data & 0xFFFF0000) == 0x07600000 && k1_patches) {
      _sh(0x1000, addr + 2); // patch k1 check
      k1_patches--;
      patches--;
    }
  }
}

// old code from original ARK that I finally found a use for
int sctrlGetThreadUIDByName(const char * name)
{
    // Invalid Arguments
    if(name == NULL) return -1;
    
    // Thread UID List
    int ids[100];
    
    // Clear Memory
    memset(ids, 0, sizeof(ids));
    
    // Thread Counter
    int count = 0;

    int (*KernelGetThreadmanIdList)() = (void*)pspXploitFindFunction("sceThreadManager", "ThreadManForUser", 0x94416130);
    int (*KernelReferThreadStatus)() = (void*)pspXploitFindFunction("sceThreadManager", "ThreadManForUser", 0x17C1684E);
    
    // Get Thread UIDs
    if(KernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, ids, NELEMS(ids), &count) >= 0)
    {
        // Iterate Results
        int i = 0; for(; i < count; i++)
        {
        	// Thread Information
        	SceKernelThreadInfo info;
        	
        	// Clear Memory
        	memset(&info, 0, sizeof(info));
        	
        	// Initialize Structure Size
        	info.size = sizeof(info);
        	
        	// Fetch Thread Status
        	if(KernelReferThreadStatus(ids[i], &info) == 0)
        	{
        		// Matching Name
        		if(strcmp(info.name, name) == 0)
        		{
        			
        			// Return Thread UID
        			return ids[i];
        		}
        	}
        }
    }
    
    // Thread not found
    return -2;
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
    colorDebug(0);

    // now we can draw things!
    PS1PRT("Loading ARK in ePSX mode");

    PS1PRT("Patching FLASH0");
    patchKermitPeripheral();

    // Invalidate Cache
    k_tbl.KernelDcacheWritebackInvalidateAll();
    k_tbl.KernelIcacheInvalidateAll();


    PS1PRT("Patching Loadexec");

    // Find LoadExec Module
    SceModule * loadexec = (SceModule*)pspXploitFindModuleByName("sceLoadExec");
    // Find Reboot Loader Function
    _LoadReboot = (void *)loadexec->text_addr;

    u32 getuserlevel = pspXploitFindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665);
    patchLoadExec(loadexec, (u32)LoadReboot, getuserlevel, 3);

    // Invalidate Cache
    k_tbl.KernelDcacheWritebackInvalidateAll();
    k_tbl.KernelIcacheInvalidateAll();

    PS1PRT("Preparing reboot...");

    _KernelLoadExecVSHWithApitype = (void *)pspXploitFindFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);
    
    SceUID kthreadID = k_tbl.KernelCreateThread( "ark-x-loader", &reboot_launcher, 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
    k_tbl.KernelStartThread(kthreadID, 0, NULL);
    k_tbl.waitThreadEnd(kthreadID, NULL);

    return 0;
}
