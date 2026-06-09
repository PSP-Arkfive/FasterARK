#include <pspsdk.h>
#include <pspumd.h>
#include <psploadcore.h>
#include <psploadexec.h>

#include <cfwmacros.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>
#include <bootloadex.h>
#include <rebootexconfig.h>
#include <libpspexploit.h>


extern ARKConfig ark_config;
extern KernelFunctions k_tbl;

extern u8* rebootbuffer;

int (* _LoadReboot)(void *, unsigned int, void *, unsigned int) = NULL;


// Build Reboot Configuration
static void buildRebootBufferConfig(int rebootBufferSize)
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
    conf->iso_mode = MODE_INFERNO;
    // Default ISO disc type
    conf->iso_disc_type = PSP_UMD_TYPE_GAME;
    
    // backup runtime ARK config
    memcpy((void*)ARK_CONFIG, &ark_config, sizeof(ARKConfig));
}

// PROCFW Reboot Buffer Loader
static int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{
    // Copy PROCFW Reboot Buffer into Memory
    memset((void*)REBOOTEX_TEXT, 0, REBOOTEX_MAX_SIZE);
    int rebootBufferSize = REBOOTEX_MAX_SIZE;

    if (rebootbuffer[0] == 0x1F && rebootbuffer[1] == 0x8B) // gzip packed rebootex
        rebootBufferSize = k_tbl.KernelGzipDecompress((unsigned char *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE, rebootbuffer, NULL);
    else // plain payload
        memcpy((void*)REBOOTEX_TEXT, rebootbuffer, REBOOTEX_MAX_SIZE);
        
    // Build Configuration
    buildRebootBufferConfig(rebootBufferSize);
    
    // Load Sony Reboot Buffer
    return _LoadReboot(arg1, arg2, arg3, arg4);
}

static void patchLoadExecModule(SceModule *loadexec, u32 LoadReboot, u32 GetUserLevel, int k1_patches) {
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

void patchLoadExec(){
    // Find LoadExec Module
    SceModule * loadexec = (SceModule*)pspXploitFindModuleByName("sceLoadExec");
    // Find Reboot Loader Function
    _LoadReboot = (void *)loadexec->text_addr;

    u32 getuserlevel = pspXploitFindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665);
    patchLoadExecModule(loadexec, (u32)LoadReboot, getuserlevel, 3);

    // Invalidate Cache
    k_tbl.KernelDcacheWritebackInvalidateAll();
    k_tbl.KernelIcacheInvalidateAll();
}

// reboot to launcher
int reboot_launcher(){
    static char menupath[ARK_PATH_SIZE];
    strcpy(menupath, ark_config.arkpath);
    strcat(menupath, ark_config.launcher);

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(menupath) + 1;
    param.argp = menupath;
    param.key = "game";

    int (* _KernelLoadExecVSHWithApitype)(int, char *, struct SceKernelLoadExecVSHParam *, int);
    _KernelLoadExecVSHWithApitype = (void *)pspXploitFindFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);
    return _KernelLoadExecVSHWithApitype(0x141, menupath, &param, 0x10000);
}
