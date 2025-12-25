#include <pspsdk.h>

#include <ark.h>
#include <bootloadex.h>
#include <rebootconfig.h>
#include <libpspexploit.h>

#include "payload.h"


#define BUF_SIZE 16*1024


unsigned char buf[BUF_SIZE];

extern int working ;
extern char* curtext;
extern KernelFunctions* k_tbl;
SceModule* loadexec = NULL;

ARKConfig arkconf = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = "ms0:/PSP/SAVEDATA/ARK_01234/", // default path for ARK files
    .exploit_id = "Live",
    .launcher = {0},
    .exec_mode = PSP_ORIG, // run ARK in PSP mode
    .recovery = 0,
};


int isVitaFile(char* filename){
    return  (  strstr(filename, "psv")  != NULL // PS Vita btcnf replacement, not used on PSP
            || strstr(filename, "660")  != NULL // PSP 6.60 modules can be used on Vita, not needed for PSP
            || strstr(filename, "vita") != NULL // Vita modules
    );
}

void open_flash(){
    while(k_tbl->IoUnassign("flash0:") < 0) {
        k_tbl->KernelDelayThread(500000);
    }
    while (k_tbl->IoAssign("flash0:", "lflash0:0,0", "flashfat0:", 0, NULL, 0)<0){
        k_tbl->KernelDelayThread(500000);
    }
}

int CopyFile(char* orig, char* dest){
    int fdr = k_tbl->KernelIOOpen(orig, PSP_O_RDONLY, 0777);
    if (fdr < 0) return fdr;
    int fdw = k_tbl->KernelIOOpen(dest, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    if (fdw < 0){
        k_tbl->KernelIOClose(fdr);
        return fdw;
    }
    while (1){
        int read = k_tbl->KernelIORead(fdr, buf, BUF_SIZE);
        if (read <= 0) break;
        k_tbl->KernelIOWrite(fdw, buf, read);
    }
    k_tbl->KernelIOClose(fdr);
    k_tbl->KernelIOClose(fdw);
    return 0;
}

void extractFlash0Archive(char* archive, char* dest_path){

    int path_len = strlen(dest_path);
    static char filepath[ARK_PATH_SIZE];
    static char filename[ARK_PATH_SIZE];
    strcpy(filepath, dest_path);

    curtext = "Extracting FLASH0.ARK";

    if (strncmp(dest_path, "flash", 4) == 0) open_flash();
    
    int fdr = k_tbl->KernelIOOpen(archive, PSP_O_RDONLY, 0777);
    
    if (fdr>=0){
        int filecount;
        k_tbl->KernelIORead(fdr, &filecount, sizeof(filecount));
        for (int i=0; i<filecount; i++){
            filepath[path_len] = '\0';
            int filesize;
            k_tbl->KernelIORead(fdr, &filesize, sizeof(filesize));

            char namelen;
            k_tbl->KernelIORead(fdr, &namelen, sizeof(namelen));

            k_tbl->KernelIORead(fdr, filename, namelen);
            filename[namelen] = '\0';
            
            if (isVitaFile(filename)){ // check if file is not needed on PSP
                k_tbl->KernelIOLSeek(fdr, filesize, 1); // skip file
            }
            else{
                strcat(filepath, (filename[0]=='/')?filename+1:filename);
                curtext = filepath;
                int fdw = k_tbl->KernelIOOpen(filepath, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
                if (fdw < 0){
                    curtext = "ERROR: could not open file for writing";
                    k_tbl->KernelIOClose(fdr);
                    while(1){};
                    return;
                }
                while (filesize>0){
                    int read = k_tbl->KernelIORead(fdr, buf, (filesize>BUF_SIZE)?BUF_SIZE:filesize);
                    k_tbl->KernelIOWrite(fdw, buf, read);
                    filesize -= read;
                }
                k_tbl->KernelIOClose(fdw);
            }
        }
        k_tbl->KernelIOClose(fdr);
    }
    else{
        curtext = "Nothing to be done";
    }

    curtext = "Finished!";
}

void copyLibraryFiles(){
    if (CopyFile("usbdevice.prx", "flash0:/kd/usbdevice.prx") < 0
            && CopyFile("ms0:/PSP/LIBS/usbdevice.prx", "flash0:/kd/usbdevice.prx") < 0){
        curtext = "ERROR copying usbdevice.prx";
    }
    if (CopyFile("idsregeneration.prx", "flash0:/kd/idsregeneration.prx") < 0
            && CopyFile("ms0:/PSP/LIBS/idsregeneration.prx", "flash0:/kd/idsregeneration.prx") < 0){
        curtext = "ERROR copying idsregeneration.prx";
    }
}

int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{
    // Copy Rebootex into Memory
    memset((char *)REBOOTEX_TEXT, 0, REBOOTEX_MAX_SIZE);
    if (rebootbuffer_psp[0] == 0x1F && rebootbuffer_psp[1] == 0x8B) // gzip packed rebootex
        k_tbl->KernelGzipDecompress((unsigned char *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE, rebootbuffer_psp, NULL);
    else // plain payload
        memcpy((void*)REBOOTEX_TEXT, rebootbuffer_psp, REBOOTEX_MAX_SIZE);
        
    // Build Configuration
    RebootConfigARK* conf = (RebootConfigARK*)(REBOOTEX_CONFIG);
    memset((char *)REBOOTEX_CONFIG, 0, sizeof(RebootConfigARK));
    conf->magic = ARK_CONFIG_MAGIC;
    memcpy((void*)ARK_CONFIG, &arkconf, sizeof(ARKConfig));
    
    // Load Sony Reboot Buffer
    int (*sceLoadReboot)(void *, unsigned int, void *, unsigned int) = (void*)loadexec->text_addr;
    return sceLoadReboot(arg1, arg2, arg3, arg4);
}

void cfwReboot(){

    loadexec = (SceModule*)pspXploitFindModuleByName("sceLoadExec");

    u32 addr = 0;
    u32 text_addr = loadexec->text_addr;
    u32 topaddr = text_addr + loadexec->text_size;
    u32 rebootcall = JAL(text_addr);

    // patch rebootex into loadexec
    int patches = 2;
    for (addr = text_addr; addr < topaddr && patches; addr += 4) {
        u32 data = _lw(addr);
        if (data == 0x3C018860) {
            _sb(0xFC, addr); // Patch Reboot Buffer Entry Point
            patches--;
        } else if (data == rebootcall) {
            _sw(JAL(LoadReboot), addr); // Patch Reboot Buffer Loader
            patches--;
        }
    }

    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();
}

int kthread(SceSize args, void *argp){
    
    static char* flash0_paths[] = {
        "FLASH0.ARK",
        "ms0:/PSP/SAVEDATA/ARK_01234/FLASH0.ARK"
    };

    for (int i=0; i<NELEMS(flash0_paths); i++){
        SceIoStat stat;
        int res = k_tbl->KernelIOGetStat(flash0_paths[i], &stat);
        if (res >= 0){
            extractFlash0Archive(flash0_paths[i], "flash0:/");
            copyLibraryFiles();
            cfwReboot();
        }
    }

    curtext = "Finished!";

    working = 0;

    return 0;
}

void doKernelThread(){
    SceUID kthreadID = k_tbl->KernelCreateThread("arkflasher", (void*)KERNELIFY(&kthread), 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
    if (kthreadID >= 0){
        // start thread and wait for it to end
        k_tbl->KernelStartThread(kthreadID, 0, NULL);
    }
}

void kmain(){
    int k1 = pspSdkSetK1(0);
    curtext = "Got Kernel Access!";
    pspXploitScanKernelFunctions(k_tbl);
    pspXploitRepairKernel();
    doKernelThread();
    curtext = "All Done!";
    pspSdkSetK1(k1);
}
