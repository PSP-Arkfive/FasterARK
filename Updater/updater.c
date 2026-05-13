#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>

#include <cfwmacros.h>
#include <kubridge.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>

#include "main.h"


#define BUF_SIZE 16*1024

enum {
    LITE_INSTALL,
    FULL_INSTALL,
};

char* savedata_files_full[] = {
    "ICON0.PNG",
    "PARAM.SFO",
    "SAVEDATA.BIN",
    "FLASH0.ARK",
    "FLASH150.ARK",
    "DC10.ARK",
    "LANG.ARK",
    "THEME.ARK",
    "VBOOT.PBP",
    "VSHMENU.PRX",
    "XBOOT.PBP",
    "PS1SPU.PRX",
    "H.BIN",
    "K.BIN",
    "ARK.BIN",
    "ARK4.BIN",
    "ARKX.BIN",
    "POPS.PRX",
    "POPSMAN.PRX",
    "MEDIASYN.PRX",
    "LEDA.PRX",
    "PLUGINS.TXT",
    "SETTINGS.TXT",
    "ARKMENU.BIN",
    "UPDATER.TXT",
    "IDSREG.PRX",
    "USBDEV.PRX",
};

char* savedata_files_lite[] = {
    "ICON0.PNG",
    "PARAM.SFO",
    "SAVEDATA.BIN",
    "FLASH0.ARK",
    "SETTINGS.TXT",
    "ARKMENU.BIN",
    "UPDATER.TXT",
};

char* cleanup_files[] = {
    "IDSREG.PRX",
    "USBDEV.PRX",
    "FLASH150.ARK",
    "DC10.ARK",
};

int install_type = FULL_INSTALL;

struct {
    char* orig;
    char* dest;
} flash_files[] = {
    {IDSREG_PRX, IDSREG_PRX_FLASH},
    {USBDEV_PRX, USBDEV_PRX_FLASH},
    {VSH_MENU, VSH_MENU_FLASH},
    {ARK_SETTINGS, ARK_SETTINGS_FLASH},
};

ARKConfig ark_config;
int psp_model = 0;

unsigned char buf[BUF_SIZE];


void open_flash(){
    while(sceIoUnassign("flash0:") < 0) {
        sceKernelDelayThread(500000);
    }
    while (sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", 0, NULL, 0)<0){
        sceKernelDelayThread(500000);
    }
    while(sceIoUnassign("flash1:") < 0) {
        sceKernelDelayThread(500000);
    }
    while (sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", 0, NULL, 0)<0){
        sceKernelDelayThread(500000);
    }
}

void copy_file(char* orig, char* dest){
    static u8 buf[BUF_SIZE];
    int fdr = sceIoOpen(orig, PSP_O_RDONLY, 0777);
    int fdw = sceIoOpen(dest, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    while (1){
        int read = sceIoRead(fdr, buf, BUF_SIZE);
        if (read <= 0) break;
        sceIoWrite(fdw, buf, read);
    }
    sceIoClose(fdr);
    sceIoClose(fdw);
}


void checkSavedataFolder(){
    SceIoStat stat;
    char path[ARK_PATH_SIZE]; strcpy(path, ark_config.arkpath); path[strlen(path)-1] = 0; // remove trailing '/' or else sceIoGetstat won't work
    if (strcmp(ark_config.arkpath+4, "/SEPLUGINS/") == 0 || sceIoGetstat(path, &stat) < 0){
        
        int res = -1;

        // create savedata folder, first attempt on ef0 for PSP Go
        if (psp_model == PSP_GO){
            strcpy(ark_config.arkpath, SAVEDATA_EF0 DEFAULT_ARK_FOLDER); // ef0:/PSP/SAVEDATA/ARK_01234
            res = sceIoMkdir(ark_config.arkpath, 0777);
        }

        if (res < 0){
            strcpy(ark_config.arkpath, SAVEDATA_MS0 DEFAULT_ARK_FOLDER); // ms0:/PSP/SAVEDATA/ARK_01234
            res = sceIoMkdir(ark_config.arkpath, 0777);
        }

        // add trailing / to facilitate concatenations
        strcat(ark_config.arkpath, "/");

        // notify SystemControl of the new arkpath
        sctrlArkSetConfig(&ark_config);

		install_type = LITE_INSTALL; // lite installation
    }
}

int filter_vita_files(char* filename){
    return (   strstr(filename, "psv")!=NULL // PS Vita btcnf replacement, not used on PSP
            || strstr(filename, "psx")!=NULL // PS Vita ePSX btcnf replacement, not used on PSP
            || strstr(filename, "660")!=NULL // PSP 6.60 modules can be used on Vita, not needed for PSP
            || strstr(filename, "vita")!=NULL // Vita modules
    );
}

int filter_savedata_full_install(char* filename){
    for (int i=0; i<NELEMS(savedata_files_full); i++){
        if (strcmp(filename, savedata_files_full[i]) == 0){
            SceIoStat stat;
            char path[ARK_PATH_SIZE];
            strcpy(path, ark_config.arkpath);
            strcat(path, filename);
            if (sceIoGetstat(path, &stat) < 0) return 1; // not previously there? do not install
            return 0; // install
        }
    }
    return 1; // not in savedata list: disallow
}

int filter_savedata_lite_install(char* filename){
    for (int i=0; i<NELEMS(savedata_files_lite); i++){
        if (strcmp(filename, savedata_files_lite[i]) == 0){
            return 0; // install
        }
    }
    return 1; // not in savedata list: disallow
}

void extractArchive(int fdr, char* dest_path, int (*filter)(char*)){

    int path_len = strlen(dest_path);
    char filepath[ARK_PATH_SIZE];
    char filename[ARK_PATH_SIZE];
    strcpy(filepath, dest_path);
    
    if (fdr>=0){
        int filecount;
        sceIoRead(fdr, &filecount, sizeof(filecount));
        for (int i=0; i<filecount; i++){
            filepath[path_len] = '\0';
            int filesize;
            sceIoRead(fdr, &filesize, sizeof(filesize));

            char namelen;
            sceIoRead(fdr, &namelen, sizeof(namelen));

            sceIoRead(fdr, filename, namelen);
            filename[namelen] = '\0';
            
            if (filter && filter(filename)){ // check if file is not needed on PSP
                sceIoLseek32(fdr, filesize, 1); // skip file
            }
            else{
                strcat(filepath, (filename[0]=='/')?filename+1:filename);
                setInfoMsg(INFO_MSG, filepath);
                int fdw = sceIoOpen(filepath, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
                if (fdw < 0){
                    ErrorExit(5000, "ERROR: could not open file for writing");
                }
                while (filesize>0){
                    int read = sceIoRead(fdr, buf, (filesize>BUF_SIZE)?BUF_SIZE:filesize);
                    sceIoWrite(fdw, buf, read);
                    filesize -= read;
                }
                sceIoClose(fdw);
            }
        }
    }
    else{
        setInfoMsg(WARNING_MSG, "File not opened");
    }

    setInfoMsg(INFO_MSG, "Finished!");
}

void doLiteInstall(){
    SceUID fd = sceIoOpen(eboot_path, PSP_O_RDONLY, 0777);
    sceIoLseek32(fd, pbp_header.psar_offset, PSP_SEEK_SET);
    extractArchive(fd, ark_config.arkpath, filter_savedata_lite_install);
    sceIoClose(fd);
}

void doFullInstall(){
    SceUID fd = sceIoOpen(eboot_path, PSP_O_RDONLY, 0777);
    sceIoLseek32(fd, pbp_header.psar_offset, PSP_SEEK_SET);
    extractArchive(fd, ark_config.arkpath, filter_savedata_full_install);
    sceIoClose(fd);
}

void installFlash0Files(){
    char flash0_ark[ARK_PATH_SIZE];
    strcpy(flash0_ark, ark_config.arkpath);
    strcat(flash0_ark, FLASH0_ARK);
    SceUID fd = sceIoOpen(flash0_ark, PSP_O_RDONLY, 0777);
    open_flash();
    extractArchive(fd, "flash0:/", filter_vita_files);
    sceIoClose(fd);

    // install extra files
    for (int i=0; i<NELEMS(flash_files); i++){
        char path[ARK_PATH_SIZE];
        strcpy(path, ark_config.arkpath);
        strcat(path, flash_files[i].orig);
        setInfoMsg(INFO_MSG, flash_files[i].dest);
        copy_file(path, flash_files[i].dest);
    }
}

void installDCFiles(){

}

void installDC150Files(){
    char flash150_ark[ARK_PATH_SIZE];
    strcpy(flash150_ark, ark_config.arkpath);
    strcat(flash150_ark, FLASH150_ARK);
    SceUID fd = sceIoOpen(flash150_ark, PSP_O_RDONLY, 0777);
    extractArchive(fd, ARK_DC_PATH_150 "/", NULL);
    sceIoClose(fd);
}

void cleanupFiles(){
    for (int i=0; i<NELEMS(cleanup_files); i++){
        char path[ARK_PATH_SIZE];
        strcpy(path, ark_config.arkpath);
        strcat(path, cleanup_files[i]);
        setInfoMsg(INFO_MSG, cleanup_files[i]);
        sceIoRemove(path);
    }
}

int updateARK(){

    static char* menuopts[] = {
        "X/O - Install",
        "/\\ - Cancel and Exit",
    };
    static char* waiting[] = {
        "Please Wait..."
    };
    static char* exitopts[] = {
        "[] - Delete Updater and Exit",
        "/\\ - Exit",
    };

    options = menuopts;
    nopts = NELEMS(menuopts);

    sctrlArkGetConfig(&ark_config);
    psp_model = kuKernelGetModel();

    while (1)
    {
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);
        if (pad.Buttons & (PSP_CTRL_CROSS|PSP_CTRL_CIRCLE))
        {
            break;
        }
        else if (pad.Buttons & PSP_CTRL_TRIANGLE)
        {
            ErrorExit(5000, "Cancelled by user.");
        }
        sceKernelDelayThread(10000);
    }

    options = waiting;
    nopts = NELEMS(waiting);

    setInfoMsg(INFO_MSG, "Starting Update...");
    checkSavedataFolder();

    setInfoMsg(INFO_MSG, "Extracting files...");
    switch (install_type){
        case LITE_INSTALL: setInfoMsg(INFO_MSG, "Lite Installation");   doLiteInstall(); break;
        case FULL_INSTALL: setInfoMsg(INFO_MSG, "Normal Installation"); doFullInstall(); break;
    }

    if (IS_PSP(&ark_config)){
        setInfoMsg(INFO_MSG, "Installing flash0 files...");
        installFlash0Files();

        SceIoStat stat;
        if (sceIoGetstat(ARK_DC_PATH, &stat)>=0){
            setInfoMsg(INFO_MSG, "Installing DC files...");
            installDCFiles();

            if (psp_model == PSP_1000 && sceIoGetstat(ARK_DC_PATH_150, &stat)>=0){
                setInfoMsg(INFO_MSG, "Installing DC ARK-150 files...");
                installDC150Files();
            }    
        }
    }

    setInfoMsg(INFO_MSG, "Cleaning Files");
    cleanupFiles();

    setInfoMsg(INFO_MSG, "Update finished!");
    sceKernelDelayThread(2000000);

    options = exitopts;
    nopts = NELEMS(exitopts);

    while (1)
    {
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_SQUARE)
        {
            sceIoRemove(eboot_path);
            char* c = strrchr(eboot_path, '/');
            *c = 0;
            sceIoRmdir(eboot_path);
            setInfoMsg(INFO_MSG, "Updater deleted, exiting...");
            break;
        }
        else if (pad.Buttons & PSP_CTRL_TRIANGLE)
        {
            setInfoMsg(INFO_MSG, "Exiting...");
            break;
        }
        sceKernelDelayThread(10000);
    }

    sceKernelDelayThread(1000000);
    return 0;
}
