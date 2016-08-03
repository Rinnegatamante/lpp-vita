#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/types.h>
#include <psp2/moduleinfo.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/apputil.h>
#include <psp2/sysmodule.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include "include/luaplayer.h"
extern "C"{
	#include <vita2d.h>
	#include "include/draw/font.h"
	#include "include/ftp/ftp.h"
}

const char *errMsg;
unsigned char *script;
int script_files = 0;
int clr_color;

int main()
{

	// Initializing touch screens and analogs
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 1);
	
	// Starting secondary modules and mounting secondary filesystems
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	SceAppUtilInitParam init_param;
	SceAppUtilBootParam boot_param;
	memset(&init_param, 0, sizeof(SceAppUtilInitParam));
	memset(&boot_param, 0, sizeof(SceAppUtilBootParam));
	sceAppUtilInit(&init_param, &boot_param);
	sceAppUtilMusicMount();
	sceAppUtilPhotoMount();
	
	char vita_ip[16];
	unsigned short int vita_port = 0;
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	clr_color = 0x000000FF;
	SceCtrlData pad;
	SceCtrlData oldpad;
	while (1) {
		
		// Load main script
		SceUID main_file = sceIoOpen("app0:/index.lua", SCE_O_RDONLY, 0777);
		if (main_file < 0) errMsg = "index.lua not found.";
		else{
			SceOff size = sceIoLseek(main_file, 0, SEEK_END);
			if (size < 1) errMsg = "Invalid main script.";
			else{
				sceIoLseek(main_file, 0, SEEK_SET);
				script = (unsigned char*)malloc(size + 1);
				sceIoRead(main_file, script, size);
				script[size] = 0;
				sceIoClose(main_file);
				errMsg = runScript((const char*)script, true);
				free(script);
			}
		}

		if (errMsg != NULL){
			if (strstr(errMsg, "lpp_shutdown")) break;
			else{
				int restore = 0;
				bool s = true;
				while (restore == 0){
					vita2d_start_drawing();
					vita2d_clear_screen();
					font_draw_string(10, 10, RGBA8(255, 255, 255, 255), "An error occurred:");
					font_draw_string(10, 30, RGBA8(255, 255, 255, 255), errMsg);
					font_draw_string(10, 70, RGBA8(255, 255, 255, 255), "Press X to restart.");
					font_draw_string(10, 90, RGBA8(255, 255, 255, 255), "Press O to enable/disable FTP.");
					if (vita_port != 0){
						font_draw_stringf(10, 150, RGBA8(255, 255, 255, 255), "PSVITA listening on IP %s , Port %u", vita_ip, vita_port);
					}
					vita2d_end_drawing();
					vita2d_swap_buffers();
					if (s){
						sceKernelDelayThread(800000);
						s = false;
					}
					sceCtrlPeekBufferPositive(0, &pad, 1);
					if (pad.buttons & SCE_CTRL_CROSS) {
						errMsg = NULL;
						restore = 1;
						if (vita_port != 0){
							ftpvita_fini();
							vita_port = 0;
						}
						sceKernelDelayThread(800000);
					}else if ((pad.buttons & SCE_CTRL_CIRCLE) && (!(oldpad.buttons & SCE_CTRL_CIRCLE))){
						if (vita_port == 0){
							ftpvita_init(vita_ip, &vita_port);
							ftpvita_add_device("app0:");
							ftpvita_add_device("ux0:");
							ftpvita_add_device("ur0:");
							ftpvita_add_device("music0:");
							ftpvita_add_device("photo0:");
						}else{
							ftpvita_fini();
							vita_port = 0;
						}
					}
					oldpad = pad;
				}
			}
		}
	}
	
	sceAppUtilPhotoUmount();
	sceAppUtilMusicUmount();
	sceAppUtilShutdown();
	vita2d_fini();
	sceKernelExitProcess(0);
	return 0;
}
