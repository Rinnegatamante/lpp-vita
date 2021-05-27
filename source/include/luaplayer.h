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
#- Smealum for ctrulib and ftpony src ----------------------------------------------------------------------------------#
#- StapleButter for debug font -----------------------------------------------------------------------------------------#
#- Lode Vandevenne for lodepng -----------------------------------------------------------------------------------------#
#- Jean-loup Gailly and Mark Adler for zlib ----------------------------------------------------------------------------#
#- Special thanks to Aurelio for testing, bug-fixing and various help with codes and implementations -------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

#ifndef __LUAPLAYER_H
#define __LUAPLAYER_H

#include <stdlib.h>
#include <lua.hpp>
#include <vitasdk.h>
#include <vita2d.h>

extern void luaC_collectgarbage (lua_State *L);

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(val, min, max) ((val)>(max)?(max):((val)<(min)?(min):(val)))

#define ASYNC_TASKS_MAX 1

const char *runScript(const char* script, bool isStringBuffer);
void luaC_collectgarbage (lua_State *L);

void luaControls_init(lua_State *L);
void luaCamera_init(lua_State *L);
void luaScreen_init(lua_State *L);
void luaGraphics_init(lua_State *L);
void luaSound_init(lua_State *L);
void luaSystem_init(lua_State *L);
void luaNetwork_init(lua_State *L);
void luaTimer_init(lua_State *L);
void luaKeyboard_init(lua_State *L);
void luaRender_init(lua_State *L);
void luaMic_init(lua_State *L);
void luaVideo_init(lua_State *L);
void luaDatabase_init(lua_State *L);
void luaRegistry_init(lua_State *L);
void luaGui_init(lua_State *L);
void bitlib_init(lua_State *L);

// lua-compat
void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);
#define lua_callk(L, na, nr, ctx, cont) ((void)(ctx), (void)(cont), lua_call(L, na, nr))
#define lua_rawlen(L, i) lua_objlen(L, i)
#define LUA_OK 0

// Internal variables
extern int clr_color;
extern char errorMex[];
extern vita2d_pgf* debug_font;
extern bool unsafe_mode;
extern bool keyboardStarted;
extern bool messageStarted;
extern SceCommonDialogConfigParam cmnDlgCfgParam;
extern volatile bool termMic;
extern int micThread(SceSize args, void* argc);
extern SceUID Mic_Mutex;
extern volatile int asyncResult;
extern uint8_t async_task_num;
extern unsigned char* asyncStrRes;
extern uint32_t asyncResSize;
extern float video_audio_tick;

// Internal structs
struct lpp_texture{
	uint32_t magic;
	vita2d_texture *text;
	void *data;
};

struct DecodedMusic{
	uint8_t* audiobuf;
	uint8_t* audiobuf2;
	uint8_t* cur_audiobuf;
	FILE* handle;
	volatile bool isPlaying;
	bool loop;
	volatile bool pauseTrigger;
	volatile bool closeTrigger;
	volatile uint8_t audioThread;
	volatile int volume;
	char filepath[256];
	char title[256];
	char author[256];
	bool tempBlock;
};

#endif
