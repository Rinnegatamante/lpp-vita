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
	#include "include/sqlite3.h"
}
#include "include/luaplayer.h"
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

int callback_results = 1;
static int sqlite_callback(void *data, int argc, char **argv, char **azColName){
	lua_State *L = (lua_State*)data;
	lua_pushnumber(L, callback_results++);
	lua_newtable(L);
	for (int i = 0; i < argc; i++){
		lua_pushstring(L,  azColName[i]);
		if (argv[i] != NULL) lua_pushstring(L, argv[i]);
		else lua_pushnil(L);
		lua_settable(L, -3);
	}
	lua_settable(L, -3);
	return 0;
}
	
static int lua_opendb(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	const char *file = luaL_checkstring(L, 1);
	sqlite3 *db;
	if (sqlite3_open(file, &db) != SQLITE_OK){
		return luaL_error(L, sqlite3_errmsg(db));
	}
	lua_pushinteger(L, (uint32_t)db);
	return 1;
}

static int lua_closedb(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	sqlite3 *db = (sqlite3*)luaL_checkinteger(L, 1);
	sqlite3_close(db);
	return 0;
}

static int lua_query(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	sqlite3 *db = (sqlite3*)luaL_checkinteger(L, 1);
	const char *query = luaL_checkstring(L, 2);
	callback_results = 1;
	char *zErrMsg = NULL;
	lua_newtable(L);
	int fd = sqlite3_exec(db, query, sqlite_callback, L, &zErrMsg);
	if (fd != SQLITE_OK){
		return luaL_error(L, sqlite3_errmsg(db));
	}
	return 1;
}
	
//Register our Database Functions
static const luaL_Reg Database_functions[] = {
  {"open",                  lua_opendb},
  {"close",                 lua_closedb},
  {"execQuery",             lua_query},
  {0, 0}
};

void luaDatabase_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Database_functions, 0);
	lua_setglobal(L, "Database");
}
