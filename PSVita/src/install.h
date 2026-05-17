#ifndef _INSTALL_H
#define _INSTALL_H

#define TITLE_ID "NPUZ01234"
#define ARK_X "SCPS10084"
#define CONTENT_ID "IP9100-PCSI00011_00-PSMRUNTIME000000"
#define CONTENT_ID_ARK "IP9100-PCSI00011_00-PSMRUNTIME000001"

#include <stddef.h>
#include <stdint.h>

size_t GetTotalNeededDirectories(int _ARK_X);
void createPspEmuDirectories(int _ARK_X);
void placePspGameData(char* gameID);
void createBubble(char* gameID);
void copySaveFiles();

void doInstall(void);
void installARK4Only(void);
void installARKXOnly(void);

int checkPS1Plugin(void);
int installAnalogPlugin(void);
void taiReloadConfig(void);

// Storage functions
int64_t getFreeSpace(const char* path);
int64_t getTotalSpace(const char* path);
int deviceExists(const char* path);
int isUx0Internal(void);
int hasUma0(void);
int checkSpaceBeforeInstall(void);

#endif