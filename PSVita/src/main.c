#include <vitasdk.h>
#include <vita2d.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <psp2/ctrl.h>
#include <psp2/appmgr.h>

#include "install.h"
#include "ui.h"

int main(int argc, const char *argv[]) {
    uiInit();
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    SceCtrlData pad;
    int selection = 0;
    const char *options[] = {
        "Install ARK-4, ARK-X and Analog Plugin",
        "Install ARK-4 Only (No Plugins)",
        "Install ARK-4 and Analog Plugin",
        "Install ARK-X Only",
        "Install Analog Plugin Only",
        "Install Savedata Only",
        "Exit"
    };
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        startDraw();
        drawLines();

        // Draw menu box
        vita2d_draw_rectangle(100, 100, 760, 300, RGBA8(0x20, 0x20, 0x40, 180));
        vita2d_draw_line(100, 100, 860, 100, RGBA8(0x40, 0x80, 0xFF, 255));
        vita2d_draw_line(100, 400, 860, 400, RGBA8(0x40, 0x80, 0xFF, 255));
        vita2d_draw_line(100, 100, 100, 400, RGBA8(0x40, 0x80, 0xFF, 255));
        vita2d_draw_line(860, 100, 860, 400, RGBA8(0x40, 0x80, 0xFF, 255));

        drawTextCenterColored(130, "Select an option with Up/Down, press X to confirm:", 0x40, 0x80, 0xFF);

        for (int i = 0; i < num_options; i++) {
            uint32_t color = (i == selection) ? RGBA8(255, 0, 0, 255) : RGBA8(255, 255, 255, 255);
            vita2d_pgf_draw_textf(uiGetFont(), 140, 170 + i * 30, color, 1.0f, "%s", options[i]);
        }

        vita2d_pgf_draw_textf(uiGetFont(), 110, 170 + selection * 30, RGBA8(255, 0, 0, 255), 1.0f, "→");

        endDraw();

        sceCtrlPeekBufferPositive(0, &pad, 1);

        if (pad.buttons & SCE_CTRL_DOWN) {
            selection = (selection + 1) % num_options;
            sceKernelDelayThread(200 * 1000);
        } else if (pad.buttons & SCE_CTRL_UP) {
            selection = (selection - 1 + num_options) % num_options;
            sceKernelDelayThread(200 * 1000);
        } else if (pad.buttons & SCE_CTRL_CROSS) {
            break;
        }
    }

    // Pre-install space check for install operations
    if (selection >= 0 && selection <= 3) {
        if (checkSpaceBeforeInstall() != 0) {
            sceKernelExitProcess(0);
        }
    }

    switch (selection) {
        case 0:
            displayMsg("Installing ARK-4 and ARK-X", "Installing full package...");
            doInstall();

            {
                const char *launch_options[] = { "Launch ARK-4", "Launch ARK-X", "Exit" };
                int launch_sel = 0;
                int launch_num = sizeof(launch_options) / sizeof(launch_options[0]);

                while (1) {
                    startDraw();
                    drawLines();

                    // Draw launch box
                    vita2d_draw_rectangle(100, 120, 760, 200, RGBA8(0x20, 0x20, 0x40, 180));
                    vita2d_draw_line(100, 120, 860, 120, RGBA8(0x40, 0x80, 0xFF, 255));
                    vita2d_draw_line(100, 320, 860, 320, RGBA8(0x40, 0x80, 0xFF, 255));
                    vita2d_draw_line(100, 120, 100, 320, RGBA8(0x40, 0x80, 0xFF, 255));
                    vita2d_draw_line(860, 120, 860, 320, RGBA8(0x40, 0x80, 0xFF, 255));

                    drawTextCenterColored(150, "Installation complete! Choose what to launch:", 0x40, 0x80, 0xFF);

                    for (int i = 0; i < launch_num; i++) {
                        uint32_t color = (i == launch_sel) ? RGBA8(255, 0, 0, 255) : RGBA8(255, 255, 255, 255);
                        vita2d_pgf_draw_textf(uiGetFont(), 140, 190 + i * 30, color, 1.0f, "%s", launch_options[i]);
                    }

                    vita2d_pgf_draw_textf(uiGetFont(), 110, 190 + launch_sel * 30, RGBA8(255, 0, 0, 255), 1.0f, "→");

                    endDraw();

                    sceCtrlPeekBufferPositive(0, &pad, 1);

                    if (pad.buttons & SCE_CTRL_DOWN) {
                        launch_sel = (launch_sel + 1) % launch_num;
                        sceKernelDelayThread(200 * 1000);
                    } else if (pad.buttons & SCE_CTRL_UP) {
                        launch_sel = (launch_sel - 1 + launch_num) % launch_num;
                        sceKernelDelayThread(200 * 1000);
                    } else if (pad.buttons & SCE_CTRL_CROSS) {
                        break;
                    }
                }

                if (launch_sel == 0) {
                    sceAppMgrLaunchAppByUri(0, "psgm:play?titleid=NPUZ01234"); // ARK-4
                    sceKernelDelayThread(1000 * 1000);
                    sceKernelExitProcess(0);
                } else if (launch_sel == 1){
                    sceAppMgrLaunchAppByUri(0, "psgm:play?titleid=SCPS10084"); // ARK-X
                    sceKernelDelayThread(1000 * 1000);
                    sceKernelExitProcess(0);
                }
                else {
                    sceKernelExitProcess(0);
                }
            }
            return 0;

        case 1:
            displayMsg("Installing ARK-4", "Installing ARK-4 only (no plugins)...");
            copySaveFiles();
            installARK4Only();
            break;

        case 2:
            displayMsg("Installing ARK-4 and Analog Plugin", "Installing ARK-4 and analog plugin...");
            copySaveFiles();
            installARK4Only();
            installAnalogPlugin();
            taiReloadConfig();
            break;

        case 3:
            displayMsg("Installing Only ARK-X", "Installing Only ARK-X...");
            copySaveFiles();
            installARKXOnly();
            checkPS1Plugin();
            taiReloadConfig();
            break;

        case 4:
            displayMsg("Installing Analog Plugin", "Installing Analog plugin...");
            installAnalogPlugin();
            taiReloadConfig();
            break;

        case 5:
            displayMsg("Installing Savedata Folder", "Installing Savedata Folder...");
            copySaveFiles();
            taiReloadConfig();
            break;

        default:
            displayMsg("Exit", "Exiting application...");
            return 0;
    }

    if (selection == 4 || selection == 5) {
        // For "Install Only Analog Plugin", don't launch since ARK-4 may not be installed
        vita2d_start_drawing();
        vita2d_clear_screen();
        vita2d_pgf_draw_textf(uiGetFont(), 20, 100, RGBA8(255, 255, 255, 255), 1.0f, "Installation complete!");
        vita2d_pgf_draw_textf(uiGetFont(), 20, 140, RGBA8(255, 255, 255, 255), 1.0f, "Press X to exit.");
        drawBatteryAndStorage();
        vita2d_end_drawing();
        vita2d_swap_buffers();
        waitCross();
    } else {
        // Launch menu
        const char *launch_opts[2];
        int num_launch = 0;
        if (selection == 1) {
            launch_opts[0] = "Launch ARK-4";
            num_launch = 1;
        } else if (selection == 2) {
            launch_opts[0] = "Launch ARK-4";
            num_launch = 1;
        } else if (selection == 3) {
            launch_opts[0] = "Launch ARK-X";
            num_launch = 1;
        }
        int launch_sel = 0;
        while (1) {
            startDraw();
            drawLines();

            // Draw launch box
            vita2d_draw_rectangle(100, 120, 760, 200, RGBA8(0x20, 0x20, 0x40, 180));
            vita2d_draw_line(100, 120, 860, 120, RGBA8(0x40, 0x80, 0xFF, 255));
            vita2d_draw_line(100, 320, 860, 320, RGBA8(0x40, 0x80, 0xFF, 255));
            vita2d_draw_line(100, 120, 100, 320, RGBA8(0x40, 0x80, 0xFF, 255));
            vita2d_draw_line(860, 120, 860, 320, RGBA8(0x40, 0x80, 0xFF, 255));

            drawTextCenterColored(150, "Installation complete! Choose what to launch:", 0x40, 0x80, 0xFF);

            for (int i = 0; i < num_launch; i++) {
                uint32_t color = (i == launch_sel) ? RGBA8(255, 0, 0, 255) : RGBA8(255, 255, 255, 255);
                vita2d_pgf_draw_textf(uiGetFont(), 140, 190 + i * 30, color, 1.0f, "%s", launch_opts[i]);
            }

            vita2d_pgf_draw_textf(uiGetFont(), 110, 190 + launch_sel * 30, RGBA8(255, 0, 0, 255), 1.0f, "→");

            endDraw();
            sceCtrlPeekBufferPositive(0, &pad, 1);
            if (pad.buttons & SCE_CTRL_DOWN) {
                launch_sel = (launch_sel + 1) % num_launch;
                sceKernelDelayThread(200 * 1000);
            } else if (pad.buttons & SCE_CTRL_UP) {
                launch_sel = (launch_sel - 1 + num_launch) % num_launch;
                sceKernelDelayThread(200 * 1000);
            } else if (pad.buttons & SCE_CTRL_CROSS) {
                break;
            }
        }
        if (num_launch == 1) {
            if (selection == 1 || selection == 2) {
                sceAppMgrLaunchAppByUri(0, "psgm:play?titleid=NPUZ01234"); // ARK-4
            } else if (selection == 3) {
                sceAppMgrLaunchAppByUri(0, "psgm:play?titleid=SCPS10084"); // ARK-X
            }
        } else if (num_launch == 2) {
            if (launch_sel == 0) {
                sceAppMgrLaunchAppByUri(0, "psgm:play?titleid=NPUZ01234"); // ARK-4
            } else {
                sceAppMgrLaunchAppByUri(0, "psgm:play?titleid=SCPS10084"); // ARK-X
            }
        }
        sceKernelDelayThread(1000 * 1000);
        sceKernelExitProcess(0);
    }
    return 0;
}