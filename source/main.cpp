#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/moduleinfo.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include "include/luaplayer.h"
extern "C"{
	#include "include/utils.h"
	#include "include/draw.h"
}
PSP2_MODULE_INFO(0, 0, "lpp-vita")

const char *errMsg;
unsigned char *script;

int main()
{
	init_video();
	SceCtrlData pad;
	
	while (1) {
	
		// Load main script
		SceUID main_file = sceIoOpen("cache0:/index.lua", PSP2_O_RDONLY, 0777);
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
			int restore = 0;
			while (restore == 0){
				sceCtrlPeekBufferPositive(0, &pad, 1);
				clear_screen();
				font_draw_string(10, 10, RGBA8(255, 255, 255, 255), "An error occurred:");
				font_draw_string(10, 30, RGBA8(255, 255, 255, 255), errMsg);
				font_draw_string(10, 70, RGBA8(255, 255, 255, 255), "Press X to restart.");
				swap_buffers();
				sceDisplayWaitVblankStart();
				if (pad.buttons & PSP2_CTRL_CROSS) {
					errMsg = NULL;
					restore = 1;
				}
			}
		}
	}

	end_video();
	return 0;
}
