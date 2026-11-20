#include <vitasdk.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "timemachine.h"
#include "ui.h"

#define TM_PACKAGE "app0:timemachine/TM.PKG"
#define TN_PACKAGE "app0:timemachine/FLASH0.TN"

static const struct {
    const char *name;
    const char *destination;
} modules[] = {
    {"/TM/tmvita.prx", "ux0:pspemu/TM/tmvita.prx"},
    {"/TM/V661ARK/reboot.prx", "ux0:pspemu/TM/V661ARK/reboot.prx"},
    {"/TM/V661EPI/reboot.prx", "ux0:pspemu/TM/V661EPI/reboot.prx"},
    {"/TM/V661TN/reboot.prx", "ux0:pspemu/TM/V661TN/reboot.prx"},
    {"/TM/V661TN/tmhlpr.prx", "ux0:pspemu/TM/V661TN/tmhlpr.prx"},
};

#define MODULE_COUNT (sizeof(modules) / sizeof(modules[0]))

static int ensureDirectory(const char *path)
{
    SceIoStat stat;
    if (sceIoGetstat(path, &stat) >= 0)
        return SCE_S_ISDIR(stat.st_mode) ? 0 : -1;
    return sceIoMkdir(path, 0777);
}

static int copyBytes(SceUID input, const char *destination, uint32_t remaining)
{
    char buffer[16 * 1024];
    SceUID output = sceIoOpen(destination, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (output < 0)
        return output;
    int result = 0;
    while (remaining > 0) {
        int amount = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        int read = sceIoRead(input, buffer, amount);
        if (read <= 0) {
            result = read < 0 ? read : -1;
            break;
        }
        int written = sceIoWrite(output, buffer, read);
        if (written != read) {
            result = written < 0 ? written : -1;
            break;
        }
        remaining -= read;
    }
    int close_result = sceIoClose(output);
    return result < 0 ? result : close_result;
}

static int scanPackage(SceUID input, SceOff *offsets, uint32_t *sizes)
{
    uint32_t count, found = 0;
    SceOff total = sceIoLseek(input, 0, SCE_SEEK_END);
    if (total < 4 || sceIoLseek(input, 0, SCE_SEEK_SET) != 0 ||
        sceIoRead(input, &count, sizeof(count)) != sizeof(count) ||
        count > (total - 4) / 5)
        return -1;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t size;
        unsigned char length;
        char name[256];
        if (sceIoRead(input, &size, sizeof(size)) != sizeof(size) ||
            sceIoRead(input, &length, sizeof(length)) != sizeof(length) || !length ||
            sceIoRead(input, name, length) != length || memchr(name, '\0', length))
            return -1;
        name[length] = '\0';
        SceOff position = sceIoLseek(input, 0, SCE_SEEK_CUR);
        if (position < 0 || position > total || size > total - position)
            return -1;
        for (size_t j = 0; j < MODULE_COUNT; j++) {
            if (strcmp(name, modules[j].name) == 0) {
                if (!size || (found & (1U << j)))
                    return -1;
                offsets[j] = position;
                sizes[j] = size;
                found |= 1U << j;
                break;
            }
        }
        if (sceIoLseek(input, size, SCE_SEEK_CUR) != position + size)
            return -1;
    }
    return found == (1U << MODULE_COUNT) - 1 ? 0 : -1;
}

int installTimeMachine(void)
{
    static const char *directories[] = {
        "ux0:pspemu", "ux0:pspemu/TM", "ux0:pspemu/TM/V661ARK",
        "ux0:pspemu/TM/V661EPI", "ux0:pspemu/TM/V661TN"
    };
    SceOff offsets[MODULE_COUNT];
    uint32_t sizes[MODULE_COUNT];
    const char *step = "opening TM.PKG";
    int result;
    SceUID package = sceIoOpen(TM_PACKAGE, SCE_O_RDONLY, 0);
    SceUID tn = -1;
    if (package < 0) {
        result = package;
        goto done;
    }
    step = "validating TM.PKG";
    result = scanPackage(package, offsets, sizes);
    if (result < 0)
        goto done;
    step = "opening FLASH0.TN";
    tn = sceIoOpen(TN_PACKAGE, SCE_O_RDONLY, 0);
    if (tn < 0) {
        result = tn;
        goto done;
    }
    SceOff tn_size = sceIoLseek(tn, 0, SCE_SEEK_END);
    if (tn_size <= 0 || tn_size > UINT32_MAX || sceIoLseek(tn, 0, SCE_SEEK_SET) != 0) {
        result = -1;
        goto done;
    }
    step = "creating Time Machine directories";
    for (size_t i = 0; i < sizeof(directories) / sizeof(directories[0]); i++) {
        result = ensureDirectory(directories[i]);
        if (result < 0)
            goto done;
    }
    displayMsg("Installing Time Machine", "Copying FLASH0.TN...");
    step = "copying FLASH0.TN";
    result = copyBytes(tn, "ux0:pspemu/TM/V661TN/FLASH0.TN", (uint32_t)tn_size);
    if (result < 0)
        goto done;
    displayMsg("Installing Time Machine", "Installing CFW modules...");
    step = "installing Time Machine modules";
    for (size_t i = 0; i < MODULE_COUNT; i++) {
        if (sceIoLseek(package, offsets[i], SCE_SEEK_SET) != offsets[i]) {
            result = -1;
            goto done;
        }
        result = copyBytes(package, modules[i].destination, sizes[i]);
        if (result < 0)
            goto done;
    }
    result = 0;
done:
    if (tn >= 0 && sceIoClose(tn) < 0 && result == 0)
        result = -1;
    if (package >= 0 && sceIoClose(package) < 0 && result == 0)
        result = -1;
    if (result < 0) {
        char message[160];
        snprintf(message, sizeof(message), "Failed %s (0x%08X).", step, (unsigned int)result);
        displayMsg("Time Machine error", message);
    }
    return result;
}
