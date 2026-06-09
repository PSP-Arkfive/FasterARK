#ifndef EPSP_KXPLOIT_H
#define EPSP_KXPLOIT_H

#include <pspsdk.h>

int initKernelExploit();
int doKernelExploit(void);
void executeKernel(u32 kernelContentFunction);
void repairKernel();

#endif