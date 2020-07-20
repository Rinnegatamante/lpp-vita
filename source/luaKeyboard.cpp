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
#include "include/luaplayer.h"
#include "include/utils.h"
#include <vitasdk.h>

#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

static uint16_t title[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
static uint16_t initial_text[SCE_IME_DIALOG_MAX_TEXT_LENGTH];
static uint16_t input_text[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
static uint8_t FINISHED = SCE_COMMON_DIALOG_STATUS_FINISHED;
static uint8_t RUNNING = SCE_COMMON_DIALOG_STATUS_RUNNING;
static uint8_t CANCELED = 3;
bool keyboardStarted = false;

static int lua_setup(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 2 && argc != 3 && argc != 4 && argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
	#endif
	char* title_ascii = (char*)luaL_checkstring(L, 1);
	char* text = (char*)luaL_checkstring(L, 2);
	SceUInt32 length = SCE_IME_DIALOG_MAX_TEXT_LENGTH;
	SceUInt32 type = SCE_IME_TYPE_BASIC_LATIN;
	SceUInt32 mode = SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT;
	SceUInt32 option = 0;
	if (argc >= 3) length = luaL_checkinteger(L, 3);
	if (argc >= 4) type = luaL_checkinteger(L, 4);
	if (argc >= 5) mode = luaL_checkinteger(L, 5);
	if (argc == 6) option = luaL_checkinteger(L, 6);
	#ifndef SKIP_ERROR_HANDLING
	if (length > SCE_IME_DIALOG_MAX_TEXT_LENGTH) length = SCE_IME_DIALOG_MAX_TEXT_LENGTH;
	if (type > 3) return luaL_error(L, "invalid keyboard type");
	if (mode > 1) return luaL_error(L, "invalid keyboard mode");
	if (strlen(title_ascii) > SCE_IME_DIALOG_MAX_TITLE_LENGTH) return luaL_error(L, "title is too long");
	#endif
	
	// Converting input to UTF16
	ascii2utf(initial_text, text);
	ascii2utf(title, title_ascii);
	
	// Initializing OSK
	SceImeDialogParam param;
	sceImeDialogParamInit(&param);
	param.supportedLanguages = 0x0001FFFF;
	param.languagesForced = SCE_TRUE;
	param.type = type;
	param.title = title;
	param.textBoxMode = mode;
	param.maxTextLength = length;
	param.initialText = initial_text;
	param.inputTextBuffer = input_text;
	if (option > 0) param.option = option;
	sceImeDialogInit(&param);
	keyboardStarted = true;
	
	return 0;
}

static int lua_state(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	SceCommonDialogStatus status = sceImeDialogGetStatus();
	if (status == SCE_COMMON_DIALOG_STATUS_FINISHED){
		SceImeDialogResult result;
		memset(&result, 0, sizeof(SceImeDialogResult));
		sceImeDialogGetResult(&result);
		if (result.button != SCE_IME_DIALOG_BUTTON_ENTER) {
			status = (SceCommonDialogStatus)CANCELED;
		}
	}
	lua_pushinteger(L, (uint32_t)status);
	return 1;
}

static int lua_input(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	char res[SCE_IME_DIALOG_MAX_TEXT_LENGTH+1];
	utf2ascii(res, input_text);
	lua_pushstring(L, res);
	return 1;
}

static int lua_clear(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	keyboardStarted = false;
	sceImeDialogTerm();
	return 0;
}

//Register our Keyboard Functions
static const luaL_Reg Keyboard_functions[] = {
	{"start",     lua_setup},
	{"getState",  lua_state},
	{"getInput",  lua_input},
	{"clear",     lua_clear},
	{0, 0}
};

void luaKeyboard_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Keyboard_functions, 0);
	lua_setglobal(L, "Keyboard");
	uint8_t MODE_TEXT = SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT;
	uint8_t MODE_PASSWORD = SCE_IME_DIALOG_TEXTBOX_MODE_PASSWORD;
	uint8_t TYPE_DEFAULT = SCE_IME_TYPE_DEFAULT;
	uint8_t TYPE_LATIN = SCE_IME_TYPE_BASIC_LATIN;
	uint8_t TYPE_NUMBER = SCE_IME_TYPE_NUMBER;
	uint8_t TYPE_EXT_NUMBER = SCE_IME_TYPE_EXTENDED_NUMBER;
	uint8_t OPT_MULTILINE = SCE_IME_OPTION_MULTILINE;
	uint8_t OPT_NO_AUTOCAP = SCE_IME_OPTION_NO_AUTO_CAPITALIZATION;
	uint8_t OPT_NO_ASSISTANCE = SCE_IME_OPTION_NO_ASSISTANCE;
	VariableRegister(L, MODE_TEXT);
	VariableRegister(L, MODE_PASSWORD);
	VariableRegister(L, TYPE_DEFAULT);
	VariableRegister(L, TYPE_LATIN);
	VariableRegister(L, TYPE_NUMBER);
	VariableRegister(L, TYPE_EXT_NUMBER);
	VariableRegister(L, RUNNING);
	VariableRegister(L, FINISHED);
	VariableRegister(L, CANCELED);
	VariableRegister(L, OPT_MULTILINE);
	VariableRegister(L, OPT_NO_AUTOCAP);
	VariableRegister(L, OPT_NO_ASSISTANCE);
}
