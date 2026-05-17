#include <vitasdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "promote.h"
#include "pbp.h"
#include "install.h"
#include "ui.h"

static char* NeededDirectories[] = {
    "ux0:pspemu", 
    "ux0:pspemu/PSP",
    "ux0:pspemu/PSP/SAVEDATA",
    "ux0:pspemu/temp",
    "ux0:pspemu/temp/game",
    "ux0:pspemu/temp/game/PSP",
    "ux0:pspemu/temp/game/PSP/GAME",
    "ux0:pspemu/temp/game/PSP/GAME/" TITLE_ID,
    "ux0:pspemu/temp/game/PSP/LICENSE"
};

static char* NeededDirectoriesARKX[] = {
    "ux0:pspemu", 
    "ux0:pspemu/PSP",
    "ux0:pspemu/temp",
    "ux0:pspemu/temp/game",
    "ux0:pspemu/temp/game/PSP",
    "ux0:pspemu/temp/game/PSP/GAME",
    "ux0:pspemu/temp/game/PSP/GAME/" ARK_X,
    "ux0:pspemu/temp/game/PSP/LICENSE"
};

#define MIN_SPACE_NEEDED (100LL * 1024LL * 1024LL)

int64_t getFreeSpace(const char* path) {
    SceIoDevInfo info;
    int res = sceIoDevctl(path, 0x3001, NULL, 0, &info, sizeof(info));
    if (res < 0) return -1;
    return (int64_t)info.free_size;
}

int64_t getTotalSpace(const char* path) {
    SceIoDevInfo info;
    int res = sceIoDevctl(path, 0x3001, NULL, 0, &info, sizeof(info));
    if (res < 0) return -1;
    return (int64_t)info.max_size;
}

int deviceExists(const char* path) {
    SceIoDevInfo info;
    return (sceIoDevctl(path, 0x3001, NULL, 0, &info, sizeof(info)) >= 0);
}

int isUx0Internal() {
    int64_t ur0Total = getTotalSpace("ur0:");
    int64_t ux0Total = getTotalSpace("ux0:");
    if (ur0Total < 0 || ux0Total < 0) return 1;
    return (ux0Total < 10LL * 1024LL * 1024LL * 1024LL);
}

int hasUma0(void) {
    return deviceExists("uma0:");
}

int checkSpaceBeforeInstall(void) {
    int64_t freeUx0 = getFreeSpace("ux0:");
    
    if (freeUx0 < 0) {
        displayMsg("ERROR", "Cannot check free space on ux0:!");
        sceKernelDelayThread(3000000);
        return 1;
    }
    
    if (freeUx0 < MIN_SPACE_NEEDED) {
        char msg[128];
        float freeGB = (float)freeUx0 / (1024.0f * 1024.0f * 1024.0f);
        snprintf(msg, sizeof(msg), "Low space on ux0:\nOnly %.1f GB free\nNeed at least 100 MB.", freeGB);
        displayMsg("WARNING", msg);
        sceKernelDelayThread(5000000);
        return 1;
    }
    
    return 0;
}

int checkTaiConfig() {
    char c = 0;
    int fd = sceIoOpen("ur0:tai/config.txt", SCE_O_RDONLY, 0777);
    if (fd < 0) return 1; 
    SceIoStat stat;
    sceIoGetstatByFd(fd, &stat);
    if (stat.st_size == 0) {
        sceIoClose(fd);
        return 1; 
    }
    sceIoLseek(fd, -1, SCE_SEEK_END);
    sceIoRead(fd, &c, 1);
    sceIoClose(fd);
    return (c == '\n');
}

int installAnalogPlugin() {
    sceIoMkdir("ur0:tai", 0777);
    updateUi("Checking for ARK Right Analog Plugin ...");
    int pluginCheck = sceIoOpen("ur0:tai/arkrightanalog.suprx", SCE_O_RDONLY, 0777);
    if(pluginCheck < 0) {
        updateUi("ARK Right Analog Plugin not found adding to config ...");
        CopyFileAndUpdateUi("app0:psp/arkrightanalog.suprx", "ur0:tai/arkrightanalog.suprx");
        int hasNewLine = checkTaiConfig();
        int addPlugin = sceIoOpen("ur0:tai/config.txt", SCE_O_CREAT | SCE_O_WRONLY | SCE_O_APPEND, 0777);
        static char pluginLine[] = "# Add second analog support to ARK\n*NPUZ01234\nur0:tai/arkrightanalog.suprx";
        if (!hasNewLine) sceIoWrite(addPlugin, "\n", 1);
        sceIoWrite(addPlugin, pluginLine, sizeof(pluginLine)-1);
        sceIoClose(addPlugin);
        return 1;
    }
    else {
        sceIoClose(pluginCheck);
        updateUi("ARK Right Analog Plugin found updating plugin and base game only ...");
        CopyFileAndUpdateUi("app0:psp/arkrightanalog.suprx", "ur0:tai/arkrightanalog.suprx");
        return 0;
    }
}

int checkPS1Plugin() {
    // Check for old ARK-X PS1 plugin in ur0:tai and ux0:tai
    int oldPluginUr0 = sceIoOpen("ur0:tai/ps1cfw_enabler.suprx", SCE_O_RDONLY, 0777);
    int oldPluginUx0 = sceIoOpen("ux0:tai/ps1cfw_enabler.suprx", SCE_O_RDONLY, 0777);
    int oldPluginFound = (oldPluginUr0 >= 0 || oldPluginUx0 >= 0);
    if(oldPluginUr0 >= 0) sceIoClose(oldPluginUr0);
    if(oldPluginUx0 >= 0) sceIoClose(oldPluginUx0);
    
    // Check for NoPspEmuDrm_mod in both ur0:tai and ux0:tai
    int noPspEmuKern = sceIoOpen("ur0:tai/NoPspEmuDrm_kern.skprx", SCE_O_RDONLY, 0777);
    int noPspEmuUser = sceIoOpen("ur0:tai/NoPspEmuDrm_user.suprx", SCE_O_RDONLY, 0777);
    int noPspEmuKernUx = sceIoOpen("ux0:tai/NoPspEmuDrm_kern.skprx", SCE_O_RDONLY, 0777);
    int noPspEmuUserUx = sceIoOpen("ux0:tai/NoPspEmuDrm_user.suprx", SCE_O_RDONLY, 0777);
    int noPspEmuFound = (noPspEmuKern >= 0 || noPspEmuUser >= 0 || 
                          noPspEmuKernUx >= 0 || noPspEmuUserUx >= 0);
    int noPspEmuBothLocations = (noPspEmuKern >= 0 || noPspEmuUser >= 0) && 
                                 (noPspEmuKernUx >= 0 || noPspEmuUserUx >= 0);
    if(noPspEmuKern >= 0) sceIoClose(noPspEmuKern);
    if(noPspEmuUser >= 0) sceIoClose(noPspEmuUser);
    if(noPspEmuKernUx >= 0) sceIoClose(noPspEmuKernUx);
    if(noPspEmuUserUx >= 0) sceIoClose(noPspEmuUserUx);
    
    // Conflict detection: old plugin AND new one both present
    if(oldPluginFound && noPspEmuFound) {
        displayMsg("CONFLICT", "Both old PS1 plugin and\nNoPspEmuDrm_mod detected!\nRemove the old ps1cfw_enabler.suprx.");
        sceKernelDelayThread(5000000);
    }
    else if(oldPluginFound) {
        displayMsg("WARNING", "Old ARK-X PS1 Plugin found!\nIt is recommended to uninstall it\nand update to latest NoPspEmuDrm_mod.");
        sceKernelDelayThread(5000000);
    }
    
    if(!noPspEmuFound) {
        displayMsg("WARNING", "NoPspEmuDrm_mod not found!\nPlease install the latest\nNoPspEmuDrm_mod for PS1 support.");
        sceKernelDelayThread(5000000);
    }
    
    // Warn if files are in both ur0 and ux0 (should only be in one)
    if(noPspEmuBothLocations) {
        displayMsg("NOTE", "NoPspEmuDrm_mod found in both\nur0:tai/ and ux0:tai/!\nKeep only one copy to avoid issues.");
        sceKernelDelayThread(5000000);
    }
    
    return 0;
}

size_t GetTotalNeededDirectories(int _ARK_X) {
    if(_ARK_X)
        return (sizeof(NeededDirectoriesARKX) / sizeof(char*));
    else
        return (sizeof(NeededDirectories) / sizeof(char*));
}

void createPspEmuDirectories(int _ARK_X) {
    if(_ARK_X) {
        for(size_t i = 0; i < GetTotalNeededDirectories(_ARK_X); i++){
            CreateDirAndUpdateUi(NeededDirectoriesARKX[i]);
        }
    }
    else {
        for(size_t i = 0; i < GetTotalNeededDirectories(0); i++){
            CreateDirAndUpdateUi(NeededDirectories[i]);
        }
    }
}

void genEbootSignature(char* ebootPath, char *gameID) {
    char ebootSigFilePath[MAX_PATH];
    char ebootSig[0x200];    
    unsigned char pbpHash[0x20];

    int swVer = 0;

    memset(ebootSig, 0x00, sizeof(ebootSig));
    memset(pbpHash, 0x00, sizeof(pbpHash));

    if(gameID != NULL)
        snprintf(ebootSigFilePath, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/__sce_ebootpbp", gameID);
    else
        snprintf(ebootSigFilePath, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/__sce_ebootpbp", TITLE_ID);

    
    updateUi("Calculating EBOOT.PBP Sha256 ...");
    HashPbp(ebootPath, pbpHash);

    updateUi("Generating EBOOT.PBP Signature ...");
    int res = _vshNpDrmEbootSigGenPsp(ebootPath, pbpHash, ebootSig, &swVer);
    if(res >= 0) {
        WriteFile(ebootSigFilePath, ebootSig, sizeof(ebootSig));
    }
}

void placePspGameData(char *gameID) {
    char ebootFile[MAX_PATH] = {0};
    char pbootFile[MAX_PATH] = {0};
    char rifFile[MAX_PATH] = {0};

    if(gameID != NULL) {
        snprintf(rifFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/LICENSE/%s.rif", CONTENT_ID_ARK);
        snprintf(ebootFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/EBOOT.PBP", gameID);
        CopyFileAndUpdateUi("app0:psx/EBOOT.PBP", ebootFile);
        CopyFileAndUpdateUi("app0:rif/psx.rif", rifFile);
    } else {
        snprintf(rifFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/LICENSE/%s.rif", CONTENT_ID);
        snprintf(ebootFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/EBOOT.PBP", TITLE_ID);
        snprintf(pbootFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/PBOOT.PBP", TITLE_ID);
        CopyFileAndUpdateUi("app0:psp/EBOOT.PBP", ebootFile);
        CopyFileAndUpdateUi("app0:psp/PBOOT.PBP", pbootFile);
        CopyFileAndUpdateUi("app0:rif/game.rif", rifFile);
    }

    genEbootSignature(ebootFile, gameID);
}

void createBubble(char *gameID) {
    updateUi("Promoting ...");
    if(gameID != NULL)
        promoteCma("ux0:pspemu/temp/game", gameID, SCE_PKG_TYPE_PSP);
    else
        promoteCma("ux0:pspemu/temp/game", TITLE_ID, SCE_PKG_TYPE_PSP);
}

int hasExistingSaveData() {
    char existingPath[MAX_PATH];
    snprintf(existingPath, sizeof(existingPath), "ux0:/pspemu/PSP/SAVEDATA/ARK_01234");
    
    int dir = sceIoDopen(existingPath);
    if (dir < 0) return 0;
    
    int hasFiles = 0;
    SceIoDirent dent;
    memset(&dent, 0, sizeof(dent));
    while (sceIoDread(dir, &dent) > 0) {
        if (strcmp(dent.d_name, ".") != 0 && strcmp(dent.d_name, "..") != 0) {
            hasFiles = 1;
            break;
        }
    }
    sceIoDclose(dir);
    
    return hasFiles;
}

int askBackupSaveData() {
    SceCtrlData pad;
    
    // Flush any stale input from menu selection
    sceKernelDelayThread(200 * 1000);
    for (int i = 0; i < 10; i++) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        sceKernelDelayThread(1000);
    }
    int backupSel = 0; 
    
    while (1) {
        startDraw();
        drawLines();

        // Draw dialog box
        vita2d_draw_rectangle(120, 140, 720, 180, RGBA8(0x20, 0x20, 0x40, 200));
        vita2d_draw_line(120, 140, 840, 140, RGBA8(0x40, 0x80, 0xFF, 255));
        vita2d_draw_line(120, 320, 840, 320, RGBA8(0x40, 0x80, 0xFF, 255));
        vita2d_draw_line(120, 140, 120, 320, RGBA8(0x40, 0x80, 0xFF, 255));
        vita2d_draw_line(840, 140, 840, 320, RGBA8(0x40, 0x80, 0xFF, 255));

        drawTextCenterColored(180, "Existing save data found!", 0x40, 0x80, 0xFF);
        drawTextCenterColored(215, "Backup before overwriting?", 255, 255, 255);
        
        int yesX = 300, noX = 580, y = 260;
        
        uint32_t selColor = RGBA8(255, 0, 0, 255);
        uint32_t normColor = RGBA8(255, 255, 255, 255);
        
        // Read input
        sceCtrlPeekBufferPositive(0, &pad, 1);
        
        if (pad.buttons & SCE_CTRL_LEFT) {
            backupSel = 0;
            sceKernelDelayThread(200 * 1000);
        } else if (pad.buttons & SCE_CTRL_RIGHT) {
            backupSel = 1;
            sceKernelDelayThread(200 * 1000);
        } else if (pad.buttons & SCE_CTRL_CROSS) {
            sceKernelDelayThread(200 * 1000);
            return (backupSel == 0) ? 1 : 0; 
        }
        
        vita2d_pgf_draw_textf(uiGetFont(), yesX, y, (backupSel == 0) ? selColor : normColor, 1.0f, "  [ YES ]");
        vita2d_pgf_draw_textf(uiGetFont(), noX, y, (backupSel == 1) ? selColor : normColor, 1.0f, "  [ NO ]");
        
        // Draw arrows around selected
        if (backupSel == 0)
            vita2d_pgf_draw_textf(uiGetFont(), yesX - 20, y, selColor, 1.0f, "→");
        else
            vita2d_pgf_draw_textf(uiGetFont(), noX - 20, y, selColor, 1.0f, "→");

        endDraw();
        sceKernelDelayThread(10000);
    }
}

void backupSaveData() {
    char backupPath[MAX_PATH];
    snprintf(backupPath, sizeof(backupPath), "ux0:/pspemu/PSP/SAVEDATA/ARK_01234_BACKUP");
    
    sceIoRemove(backupPath);
    
    char existingPath[MAX_PATH];
    snprintf(existingPath, sizeof(existingPath), "ux0:/pspemu/PSP/SAVEDATA/ARK_01234");
    
    startDraw();
    drawLines();
    vita2d_draw_rectangle(80, 160, 800, 100, RGBA8(0x15, 0x15, 0x30, 200));
    vita2d_draw_line(80, 160, 880, 160, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(80, 260, 880, 260, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(80, 160, 80, 260, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(880, 160, 880, 260, RGBA8(0x40, 0x80, 0xFF, 255));
    drawTextCenterColored(190, "Backing up ...", 0x60, 0xA0, 0xFF);
    drawTextCenterColored(230, "Backing up existing ARK save data...", 255, 255, 255);
    endDraw();
    
    CopyTreeSilent(existingPath, backupPath);
    
    displayMsg("BACKUP", "Existing ARK save data\nbacked up to:\nARK_01234_BACKUP");
    sceKernelDelayThread(3000000);
}

void copySaveFiles() {
    if (hasExistingSaveData()) {
        if (askBackupSaveData()) {
            backupSaveData();
        }
    }
    sceIoMkdir("ux0:/pspemu/PSP/SAVEDATA/ARK_01234", 0006);
    CopyTree("app0:save/ARK_01234", "ux0:/pspemu/PSP/SAVEDATA/ARK_01234");
}

void installARK4Only() {
    createPspEmuDirectories(0);
    placePspGameData(NULL);
    createBubble(NULL);
}

void installARKXOnly() {
    createPspEmuDirectories(1);
    placePspGameData("SCPS10084");
    createBubble("SCPS10084");
}

void doInstall() {
    copySaveFiles();
    installARK4Only();
    installARKXOnly();
    installAnalogPlugin();
    checkPS1Plugin();
    taiReloadConfig();
}

void taiReloadConfig(void) {
    updateUi("Reloading tai config...");
    sceKernelDelayThread(1000000); 
}