#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pspsdk.h>
#include <pspumd.h>
#include <pspkermit.h>
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


// Vita Buffered RAM flash0 Filesystem Structure
typedef struct VitaFlashBufferFile
{
    char * name;
    void * content;
    unsigned int size;
} VitaFlashBufferFile;

extern ARKConfig ark_config;
extern KernelFunctions k_tbl;

int (* Kermit_driver_4F75AA05)(void* kermit_packet, u32 cmd_mode, u32 cmd, u32 argc, u32 allow_callback, u64 *resp) = NULL;


int installFlash0Archive(char* path)
{
    int fd;

    // Base Address
    uint32_t procfw = VITA_FLASH_ARK;
    uint32_t sony = VITA_FLASH_SONY;

    // Cast PROCFW flash0 Filesystem
    VitaFlashBufferFile * prof0 = (VitaFlashBufferFile *)procfw;
    
    // Cast Sony flash0 Filesystem
    VitaFlashBufferFile * f0 = (VitaFlashBufferFile *)sony;

    // flash0 Filecounts
    uint32_t procfw_filecount = 0;
    uint32_t flash0_filecount = 0;

    fd = k_tbl.KernelIOOpen(path, PSP_O_RDONLY, 0777);

    if(fd < 0){
        return fd;
    }

    k_tbl.KernelIORead(fd, &procfw_filecount, sizeof(procfw_filecount));
    k_tbl.KernelIOClose(fd);

    // Count Sony flash0 Files
    while(f0[flash0_filecount].content != NULL) flash0_filecount++;

    // Copy Sony flash0 Filesystem into PROCFW flash0
    memcpy(&prof0[procfw_filecount], f0, (flash0_filecount + 1) * sizeof(VitaFlashBufferFile));
    
    // Cast Namebuffer
    char * namebuffer = (char *)sony;
    
    // Cast Contentbuffer
    unsigned char * contentbuffer = (unsigned char *)&prof0[procfw_filecount + flash0_filecount + 1];
    
    // Ammount of linked in Files
    unsigned int linked = 0;
    
    fd = k_tbl.KernelIOOpen(path, PSP_O_RDONLY, 0777);

    if(fd < 0)
        return fd;

    int fileSize, ret, i;
    unsigned char lenFilename;

    // skip file counter
    k_tbl.KernelIORead(fd, &fileSize, sizeof(fileSize));

    for(i=0; i<procfw_filecount; ++i)
    {
        ret = k_tbl.KernelIORead(fd, &fileSize, sizeof(fileSize));

        if(ret != sizeof(fileSize))
            break;

        ret = k_tbl.KernelIORead(fd, &lenFilename, sizeof(lenFilename));

        if(ret != sizeof(lenFilename))
            break;

        ret = k_tbl.KernelIORead(fd, namebuffer, lenFilename);

        if(ret != lenFilename)
            break;

        namebuffer[lenFilename] = '\0';

        // Content Buffer 64 Byte Alignment required
        // (if we don't align this buffer by 64 PRXDecrypt will fail on 1.67+ FW!)
        if((((unsigned int)contentbuffer) % 64) != 0)
        {
            // Align Content Buffer
            contentbuffer += 64 - (((unsigned int)contentbuffer) % 64);
        }
        
        ret = k_tbl.KernelIORead(fd, contentbuffer, fileSize);

        if(ret != fileSize)
            break;

        // Link File into virtual flash0 Filesystem
        prof0[linked].name = namebuffer;
        prof0[linked].content = contentbuffer;
        prof0[linked++].size = fileSize;

        // Move Buffer
        namebuffer += lenFilename + 1;
        contentbuffer += fileSize;
    }

    k_tbl.KernelIOClose(fd);

    // Injection Error
    if(procfw_filecount == 0 || linked != procfw_filecount) return -1;
    
    // Return Number of Injected Files
    return linked;
}

// kermit_peripheral's sub_000007CC clone, called by loadexec + 0x0000299C with a0=8 (was a0=7 for fw <210)
// Returns 0 on success
u64 kermit_flash_load(int cmd)
{
    u8 buf[128];
    u64 resp;
    void *alignedBuf = (void*)ALIGN_64((int)buf + 63);
    k_tbl.KernelDcacheInvalidateRange(alignedBuf, 0x40);
    KermitPacket *packet = (KermitPacket *)KERMIT_PACKET((int)alignedBuf);
    u32 argc = 0;
    Kermit_driver_4F75AA05(packet, KERMIT_MODE_PERIPHERAL, cmd, argc, KERMIT_CALLBACK_DISABLE, &resp);
    return resp;
}

int flashLoadPatch(int cmd)
{
    int ret = kermit_flash_load(cmd);
    // Custom handling on loadFlash mode, else nothing
    if ( cmd == KERMIT_CMD_ERROR_EXIT || cmd == KERMIT_CMD_ERROR_EXIT_2 )
    {
        int linked;
        // Wait for flash to load
        k_tbl.KernelDelayThread(10000);
        // Patch flash0 Filesystem Driver
        char path[ARK_PATH_SIZE];
        strcpy(path, ark_config.arkpath);
        strcat(path, FLASH0_ARK);
        linked = installFlash0Archive(path);
        k_tbl.KernelIcacheInvalidateAll();
        k_tbl.KernelDcacheWritebackInvalidateAll();
    }
    return ret;
}

void* findKermitFlashDriver(){
    u32 nids[] = {0x4F75AA05, 0x36666181};
    for (int i=0; i<NELEMS(nids) && !Kermit_driver_4F75AA05; i++){
        Kermit_driver_4F75AA05 = (void*)pspXploitFindFunction("sceKermit_Driver", "sceKermit_driver", nids[i]);
    }
    return Kermit_driver_4F75AA05;
}

int patchKermitPeripheral()
{
    findKermitFlashDriver();
    // Redirect KERMIT_CMD_ERROR_EXIT loadFlash function
    u32 knownnids[2] = { 0x3943440D, 0x0648E1A3 /* 3.3X */ };
    u32 swaddress = 0;
    u32 i;
    for (i = 0; i < 2; i++)
    {
        swaddress = pspXploitFindFirstJAL(pspXploitFindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral_driver", knownnids[i]));
        if (swaddress != 0)
            break;
    }
    _sw(JUMP(flashLoadPatch), swaddress);
    _sw(NOP, swaddress+4);
    
    return 0;
}
