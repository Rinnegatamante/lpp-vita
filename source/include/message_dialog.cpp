/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____    _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |      | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|      |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   ------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License --------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <lpp.nanni@gmail.com> ---------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@gmail.com> -------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- All the devs involved in Rejuvenate and vita-toolchain --------------------------------------------------------------#
#- xerpi for drawing libs and for FTP server code ----------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <vitasdk.h>
#include <vita2d.h>

#define MSGDIALOG_WIDTH 480.0f
#define MSGDIALOG_X     240.0f 
#define FONT_HEIGHT     23.0f
#define FONT_WIDTH      15.0f
#define LINE_MAX_WIDTH  434.0f
extern SceMsgDialogProgressBarParam barParam;
extern SceMsgDialogUserMessageParam msgParam;
extern bool messageStarted;
extern char messageText[512];
extern vita2d_pgf* debug_font;
extern SceCommonDialogConfigParam cmnDlgCfgParam;

struct MsgDialog{
	char msg[512];
	char submsg[128];
	int btnType;
	int mode;
	SceUInt32 progress;
	SceCommonDialogStatus state;
	SceCommonDialogStatus intstate;
	int btn;
	float y;
	float height;
	float anim_percent;
	float submsg_x;
};

MsgDialog dlg;
uint32_t oldpad;

void sceMsgDialogDraw(){
	if (!messageStarted) return;
	if (dlg.anim_percent >= 1.0f){
		switch (dlg.mode){
			case SCE_MSG_DIALOG_MODE_USER_MSG:
				vita2d_draw_rectangle(MSGDIALOG_X, dlg.y, MSGDIALOG_WIDTH, dlg.height, RGBA8(0x29, 0x26, 0x39, 0xFF));
				vita2d_pgf_draw_text(debug_font, MSGDIALOG_X + FONT_WIDTH, dlg.y + 17.402f + FONT_HEIGHT / 2, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), 1.0f, dlg.msg);
				break;
			case SCE_MSG_DIALOG_MODE_PROGRESS_BAR:
				vita2d_draw_rectangle(MSGDIALOG_X, dlg.y, MSGDIALOG_WIDTH, dlg.height, RGBA8(0x29, 0x26, 0x39, 0xFF));
				vita2d_pgf_draw_text(debug_font, MSGDIALOG_X + FONT_WIDTH, dlg.y + 17.402f + FONT_HEIGHT / 2, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), 1.0f, dlg.msg);
				vita2d_draw_rectangle(MSGDIALOG_X + FONT_WIDTH * 3, dlg.y + 17.402f + FONT_HEIGHT / 2 + FONT_HEIGHT * 2, MSGDIALOG_WIDTH - FONT_WIDTH * 6, 8.0f, RGBA8(0x08, 0x08, 0x08, 0xFF));
				vita2d_draw_rectangle(MSGDIALOG_X + FONT_WIDTH * 3, dlg.y + 17.402f + FONT_HEIGHT / 2 + FONT_HEIGHT * 2, (MSGDIALOG_WIDTH - FONT_WIDTH * 6) * (dlg.progress / 100.0f), 8.0f, RGBA8(0x87, 0xB7, 0x2D, 0xFF));
				vita2d_pgf_draw_text(debug_font, dlg.submsg_x, dlg.y + 17.402f + FONT_HEIGHT / 2 + FONT_HEIGHT * 3, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), 0.8f, dlg.submsg);
				break;
		}
	}else{
		if (dlg.intstate == SCE_COMMON_DIALOG_STATUS_RUNNING) dlg.anim_percent += 0.01f;
		else dlg.anim_percent -= 0.01f;
		float MSGDIALOG_Y = dlg.height / 2;
		vita2d_draw_rectangle(MSGDIALOG_X + (MSGDIALOG_X - dlg.anim_percent * MSGDIALOG_X), dlg.y + (MSGDIALOG_Y - dlg.anim_percent * MSGDIALOG_Y), (dlg.anim_percent * MSGDIALOG_X) * 2, (dlg.anim_percent * MSGDIALOG_Y) * 2, RGBA8(0x29, 0x26, 0x39, 0xFF));
		if ((dlg.intstate == SCE_COMMON_DIALOG_STATUS_FINISHED) && (dlg.anim_percent <= 0.0f)) dlg.state = SCE_COMMON_DIALOG_STATUS_FINISHED;
	}
}

int sceMsgDialogInit(const SceMsgDialogParam *param) {
	
	// Setting Dialog mode
	if (param == NULL) return SCE_MSG_DIALOG_ERROR_PARAM;
	dlg.mode = param->mode;

	// Populating MsgDialog struct, calculating extra lines for buttons
	dlg.btn = 0;
	dlg.anim_percent = 0.0f;
	uint8_t extra_lines = 0;
	switch (dlg.mode){
		case SCE_MSG_DIALOG_MODE_PROGRESS_BAR:
			sprintf(dlg.msg, (char*)param->progBarParam->msg);
			sprintf(dlg.submsg, " ");
			dlg.progress = 0;
			break;
		case SCE_MSG_DIALOG_MODE_USER_MSG:
			sprintf(dlg.msg, (char*)param->userMsgParam->msg);
			dlg.btnType = param->userMsgParam->buttonType;
			switch (dlg.btnType){
				case SCE_MSG_DIALOG_BUTTON_TYPE_OK:
				case SCE_MSG_DIALOG_BUTTON_TYPE_CANCEL:
					extra_lines = 2;
					break;
				case SCE_MSG_DIALOG_BUTTON_TYPE_YESNO:
				case SCE_MSG_DIALOG_BUTTON_TYPE_OK_CANCEL:
					extra_lines = 3;
					break;
				default:
					extra_lines = 0;
					break;
			}
			break;
	}
	
	// Nulling starting buttons
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
	oldpad = pad.buttons;
	
	// Calculating message size
	char* str = dlg.msg;
	char* spc = strstr(str, " ");
	char* last_spc = NULL;
	char* next_newline = strstr(str, "\n");
	char buf[512];
	float len = 0.0f;
	if (spc != NULL){
		spc[0] = 0;
		sprintf(buf, str);
		spc[0] = ' ';
		len = vita2d_pgf_text_width(debug_font, 1.0f, buf);
	}
	uint8_t lines = 1;
	while (spc != NULL){
		if ((next_newline != NULL) && (next_newline < spc)){
			str = next_newline;
			next_newline = strstr(str + 1, "\n");
			spc[0] = 0;
			sprintf(buf, str);
			spc[0] = ' ';
			len = vita2d_pgf_text_width(debug_font, 1.0f, buf);
			lines++;
		}else if (len < LINE_MAX_WIDTH){
			last_spc = spc;
			spc = strstr(spc + 1, " ");
			if (spc != NULL){
				spc[0] = 0;
				sprintf(buf, str);
				spc[0] = ' ';
				len = vita2d_pgf_text_width(debug_font, 1.0f, buf);
			}
		}else{
			last_spc[0] = '\n';
			str = last_spc;
			spc = strstr(str, " ");
			if (spc != NULL){
				spc[0] = 0;
				sprintf(buf, str);
				spc[0] = ' ';
				len = vita2d_pgf_text_width(debug_font, 1.0f, buf);
			}
			lines++;
		}
	}
	
	// Appending extra lines to the message
	if (dlg.mode == SCE_MSG_DIALOG_MODE_USER_MSG){
		switch (dlg.btnType){
			case SCE_MSG_DIALOG_BUTTON_TYPE_OK:
				sprintf(dlg.msg, "%s\n\nPress %s to confirm.", dlg.msg, (cmnDlgCfgParam.enterButtonAssign == 0) ? "O" : "X");
				break;
			case SCE_MSG_DIALOG_BUTTON_TYPE_CANCEL:
				sprintf(dlg.msg, "%s\n\nPress %s to cancel.", dlg.msg, (cmnDlgCfgParam.enterButtonAssign == 0) ? "X" : "O");
				break;
			case SCE_MSG_DIALOG_BUTTON_TYPE_YESNO:
			case SCE_MSG_DIALOG_BUTTON_TYPE_OK_CANCEL:
				sprintf(dlg.msg, "%s\n\nPress %s to confirm.\nPress %s to cancel.", dlg.msg, (cmnDlgCfgParam.enterButtonAssign == 0) ? "O" : "X", (cmnDlgCfgParam.enterButtonAssign == 0) ? "X" : "O");
				break;
			default:
				break;
		}
	}
	
	dlg.height = FONT_HEIGHT * (lines + extra_lines) + 20.0f;
	if (dlg.mode == SCE_MSG_DIALOG_MODE_PROGRESS_BAR) dlg.height += FONT_HEIGHT * 3; // Some extra space for the progressbar
	dlg.y = (544.0f - dlg.height) / 2;
	dlg.intstate = dlg.state = SCE_COMMON_DIALOG_STATUS_RUNNING;
	return 0;
}

int sceMsgDialogTerm(void) {
	dlg.state = SCE_COMMON_DIALOG_STATUS_NONE;
	return 0;
}

int sceMsgDialogClose(void) {
	dlg.intstate = SCE_COMMON_DIALOG_STATUS_FINISHED;
	dlg.anim_percent = 0.99f;
	return 0;
}

int sceMsgDialogGetResult(SceMsgDialogResult *result) {
	if (result == NULL) return -1;
	result->buttonId = dlg.btn;
	return 0;
}

SceCommonDialogStatus sceMsgDialogGetStatus(void) {
	if ((dlg.state == SCE_COMMON_DIALOG_STATUS_RUNNING) && (dlg.intstate == dlg.state)){
		if (dlg.mode == SCE_MSG_DIALOG_MODE_USER_MSG){
			SceCtrlData pad;
			sceCtrlPeekBufferPositive(0, &pad, 1);
			uint32_t SCE_CTRL_CONFIRM = (cmnDlgCfgParam.enterButtonAssign == 0) ? SCE_CTRL_CIRCLE : SCE_CTRL_CROSS;
			uint32_t SCE_CTRL_CANCEL = (cmnDlgCfgParam.enterButtonAssign == 0) ? SCE_CTRL_CROSS : SCE_CTRL_CIRCLE;
			switch (dlg.btnType) {
				case SCE_MSG_DIALOG_BUTTON_TYPE_OK:
					if ((pad.buttons & SCE_CTRL_CONFIRM) && (!(oldpad & SCE_CTRL_CONFIRM))){
						sceMsgDialogClose();
						dlg.btn = SCE_MSG_DIALOG_BUTTON_ID_OK;
					}
					break;
				case SCE_MSG_DIALOG_BUTTON_TYPE_YESNO:
				case SCE_MSG_DIALOG_BUTTON_TYPE_OK_CANCEL:
					if (pad.buttons & SCE_CTRL_CONFIRM && (!(oldpad & SCE_CTRL_CONFIRM))){
						sceMsgDialogClose();
						dlg.btn = SCE_MSG_DIALOG_BUTTON_ID_OK;
					}else if (pad.buttons & SCE_CTRL_CANCEL && (!(oldpad & SCE_CTRL_CANCEL))){
						sceMsgDialogClose();
						dlg.btn = SCE_MSG_DIALOG_BUTTON_ID_NO;
					}
					break;
				case SCE_MSG_DIALOG_BUTTON_TYPE_CANCEL:
					if (pad.buttons & SCE_CTRL_CANCEL && (!(oldpad & SCE_CTRL_CANCEL))){
						sceMsgDialogClose();
						dlg.btn = SCE_MSG_DIALOG_BUTTON_ID_NO;
					}
					break;
				default:
					break;
			}
			oldpad = pad.buttons;
		}
	}
	return dlg.state;
}

int sceMsgDialogProgressBarInc(SceMsgDialogProgressBarTarget target, SceUInt32 delta){
	dlg.progress += delta;
	if (dlg.progress > 100) dlg.progress = 100;
	return 0;
}

int sceMsgDialogProgressBarSetValue(SceMsgDialogProgressBarTarget target, SceUInt32 rate){
	dlg.progress = rate;
	if (dlg.progress > 100) dlg.progress = 100;
	return 0;
}

int sceMsgDialogProgressBarSetMsg( SceMsgDialogProgressBarTarget target, const SceChar8 *barMsg ){
	sprintf(dlg.submsg, (char*)barMsg);
	float submsg_width = vita2d_pgf_text_width(debug_font, 0.8f, dlg.submsg);
	dlg.submsg_x = MSGDIALOG_X * 2 - submsg_width / 2;
	return 0;
}