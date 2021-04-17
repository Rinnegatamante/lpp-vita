/*---------------------------------------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ---------------------------------------------------------------------------------------#
#------- _  -------------------  ______  _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \| |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____   _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/| | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |     | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|     |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/ -------------------------------------------------------------#
#------------------------   ______   _   ------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  ------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   -----------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  -----------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  -----------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   -----------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License -------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <lpp.nanni@gmail.com> --------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@gmail.com> ------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#- Credits : ----------------------------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#- All the devs involved in Rejuvenate and vita-toolchain -------------------------------------------------------------#
#- xerpi for drawing libs and for FTP server code ---------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern "C"{
	#include <vitasdk.h>
}
#include "include/luaplayer.h"
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

enum {
	TYPE_STRING,
	TYPE_BINARY,
	TYPE_NUMBER
};

static int lua_getkey(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char buf[256];
	int val;
	const char *cat = luaL_checkstring(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int type = luaL_checkinteger(L, 3);
	switch (type) {
	case TYPE_STRING:
		sceRegMgrGetKeyStr(cat, name, buf, 256);
		lua_pushstring(L, buf);
		break;
	case TYPE_BINARY:
		sceRegMgrGetKeyBin(cat, name, buf, 256);
		lua_pushlstring(L, buf, 256);
		break;
	default:
		sceRegMgrGetKeyInt(cat, name, &val);
		lua_pushinteger(L, val);
		break;
	}
	return 1;
}

static int lua_setkey(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 4 && argc != 5) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char *buf;
	int val;
	const char *cat = luaL_checkstring(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int type = luaL_checkinteger(L, 3);
	int size = 0;
	if (argc == 5) size = luaL_checkinteger(L, 5);
	switch (type) {
	case TYPE_NUMBER:
		val = luaL_checkinteger(L, 4);
		sceRegMgrSetKeyInt(cat, name, val);
		break;
	case TYPE_STRING:
		buf = size ? luaL_checkstring(L, 4) : luaL_checklstring(L, 4, &size);
		sceRegMgrSetKeyStr(cat, name, buf, size);
		break;
	default:
		buf = size ? luaL_checkstring(L, 4) : luaL_checklstring(L, 4, &size);
		sceRegMgrSetKeyBin(cat, name, buf, size);
		break;
	}
	
	return 0;
}

static int lua_getsyskey(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char buf[256];
	int val;
	int id = luaL_checkinteger(L, 1);
	int type = luaL_checkinteger(L, 2);
	switch (type) {
	case TYPE_STRING:
		sceRegMgrSystemParamGetStr(id, buf, 256);
		lua_pushstring(L, buf);
		break;
	default:
		sceRegMgrSystemParamGetInt(id, &val);
		lua_pushinteger(L, val);
		break;
	}
	return 1;
}
	
//Register our Registry Functions
static const luaL_Reg Registry_functions[] = {
  {"getKey",                 lua_getkey},
  {"setKey",                 lua_setkey},
  {"getSysKey",              lua_getsyskey},
  {0, 0}
};

void luaRegistry_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Registry_functions, 0);
	lua_setglobal(L, "Registry");
	VariableRegister(L, TYPE_STRING);
	VariableRegister(L, TYPE_BINARY);
	VariableRegister(L, TYPE_NUMBER);
}
