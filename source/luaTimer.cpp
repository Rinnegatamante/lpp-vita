/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____	_____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |	  | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|	  |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |	  | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|	  |_| \____|(___/   ------------------------------------------------------------------#
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
#include <vitasdk.h>
#include "include/luaplayer.h"

struct Timer {
	uint32_t magic;
	bool isPlaying;
	uint64_t tick;
};

static inline __attribute__((always_inline)) uint64_t osGetTime(void) {
	return (sceKernelGetProcessTimeWide() / 1000);
}

static int lua_newT(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0)
		return luaL_error(L, "wrong number of arguments");
#endif
	Timer* new_timer = (Timer*)malloc(sizeof(Timer));
	new_timer->tick = osGetTime();
	new_timer->magic = 0x4C544D52;
	new_timer->isPlaying = true;
	lua_pushinteger(L, (uint32_t)new_timer);
	return 1;
}

static int lua_time(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1)
		return luaL_error(L, "wrong number of arguments");
#endif
	Timer* src = (Timer*)luaL_checkinteger(L,1);
#ifndef SKIP_ERROR_HANDLING
	if (src->magic != 0x4C544D52)
		return luaL_error(L, "attempt to access wrong memory block type");
#endif
	if (src->isPlaying) {
		lua_pushinteger(L, osGetTime() - src->tick);
	} else {
		lua_pushinteger(L, src->tick);
	}
	return 1;
}

static int lua_pause(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1)
		return luaL_error(L, "wrong number of arguments");
#endif
	Timer* src = (Timer*)luaL_checkinteger(L, 1);
#ifndef SKIP_ERROR_HANDLING
	if (src->magic != 0x4C544D52)
		return luaL_error(L, "attempt to access wrong memory block type");
#endif
	if (src->isPlaying) {
		src->isPlaying = false;
		src->tick = osGetTime() - src->tick;
	}
	return 0;
}

static int lua_resume(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1)
		return luaL_error(L, "wrong number of arguments");
#endif
	Timer* src = (Timer*)luaL_checkinteger(L, 1);
#ifndef SKIP_ERROR_HANDLING
	if (src->magic != 0x4C544D52)
		return luaL_error(L, "attempt to access wrong memory block type");
#endif
	if (!src->isPlaying) {
		src->isPlaying = true;
		src->tick = (osGetTime()-src->tick);
	}
	return 0;
}

static int lua_reset(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1)
		return luaL_error(L, "wrong number of arguments");
#endif
	Timer* src = (Timer*)luaL_checkinteger(L, 1);
#ifndef SKIP_ERROR_HANDLING
	if (src->magic != 0x4C544D52)
		return luaL_error(L, "attempt to access wrong memory block type");
#endif
	if (src->isPlaying)
		src->tick = osGetTime();
	else
		src->tick = 0;
	return 0;
}

static int lua_set(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2)
		return luaL_error(L, "wrong number of arguments");
#endif
	Timer* src = (Timer*)luaL_checkinteger(L, 1);
	uint32_t val = (uint32_t)luaL_checkinteger(L, 2);
#ifndef SKIP_ERROR_HANDLING
	if (src->magic != 0x4C544D52)
		return luaL_error(L, "attempt to access wrong memory block type");
#endif
	if (src->isPlaying)
		src->tick = osGetTime() + val;
	else
		src->tick = val;
	return 0;
}

static int lua_wisPlaying(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1)
		return luaL_error(L, "wrong number of arguments");
#endif
	Timer* src = (Timer*)luaL_checkinteger(L, 1);
#ifndef SKIP_ERROR_HANDLING
	if (src->magic != 0x4C544D52)
		return luaL_error(L, "attempt to access wrong memory block type");
#endif
	lua_pushboolean(L, src->isPlaying);
	return 1;
}

static int lua_destroy(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1)
		return luaL_error(L, "wrong number of arguments");
#endif
	Timer* timer = (Timer*)luaL_checkinteger(L,1);
#ifndef SKIP_ERROR_HANDLING
	if (timer->magic != 0x4C544D52)
		return luaL_error(L, "attempt to access wrong memory block type");
#endif
	free(timer);
	return 1;
}

//Register our Timer Functions
static const luaL_Reg Timer_functions[] = {
	{"new",        lua_newT},
	{"getTime",    lua_time},
	{"setTime",    lua_set},
	{"destroy",    lua_destroy},
	{"pause",      lua_pause},
	{"resume",     lua_resume},
	{"reset",      lua_reset},
	{"isPlaying",  lua_wisPlaying},
	{0, 0}
};

void luaTimer_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Timer_functions, 0);
	lua_setglobal(L, "Timer");
}