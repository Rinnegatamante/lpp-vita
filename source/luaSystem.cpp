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
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/power.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/dirent.h>
#include "include/luaplayer.h"
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

int FREAD = SCE_O_RDONLY;
int FWRITE = SCE_O_WRONLY;
int FCREATE = SCE_O_CREAT | SCE_O_WRONLY;
int FRDWR = SCE_O_RDWR;
int SET = 1;
int CUR = 2;
int END = 3;
extern int script_files;

static int lua_dofile(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	char* file = (char*)luaL_checkstring(L,1);
	unsigned char* buffer;
	SceUID script = sceIoOpen(file, SCE_O_RDONLY, 0777);
	if (script < 0) return luaL_error(L, "error opening file.");
	else{
		SceOff size = sceIoLseek(script, 0, SEEK_END);
		sceIoLseek(script, 0, SEEK_SET);
		buffer = (unsigned char*)malloc(size + 1);
		sceIoRead(script, buffer, size);
		buffer[size] = 0;
		sceIoClose(script);
	}
	lua_settop(L, 1);
	if (luaL_loadbuffer(L, (const char*)buffer, strlen((const char*)buffer), NULL) != LUA_OK)	return lua_error(L);
	lua_KFunction dofilecont = (lua_KFunction)(lua_gettop(L) - 1);
	lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
	return (int)dofilecont;
}

static int lua_openfile(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
	const char *file_tbo = luaL_checkstring(L, 1);
	int type = luaL_checkinteger(L, 2);
	SceUID fileHandle = sceIoOpen(file_tbo, type, 0777);
	lua_pushinteger(L,fileHandle);
	return 1;
}

static int lua_readfile(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
	int file = luaL_checkinteger(L, 1);
	int size = luaL_checkinteger(L, 2);
	unsigned char *buffer = (unsigned char*)(malloc((size+1) * sizeof (char)));
	sceIoRead(file,buffer, size);
	buffer[size] = 0;
	lua_pushlstring(L,(const char*)buffer,size);
	free(buffer);
	return 1;
}

static int lua_writefile(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
	int fileHandle = luaL_checkinteger(L, 1);
	const char *text = luaL_checkstring(L, 2);
	int size = luaL_checknumber(L, 3);
	sceIoWrite(fileHandle, text, size);
	return 0;
}

static int lua_closefile(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	int fileHandle = luaL_checkinteger(L, 1);
	sceIoClose(fileHandle);
	return 0;
}

static int lua_seekfile(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
	int fileHandle = luaL_checkinteger(L, 1);
	int pos = luaL_checkinteger(L, 2);
	int type = luaL_checkinteger(L, 3);
	sceIoLseek(fileHandle, pos, type);	
	return 0;
}

static int lua_sizefile(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	int fileHandle = luaL_checkinteger(L, 1);
	SceOff size = sceIoLseek(fileHandle, 0, SEEK_END);
	sceIoLseek(fileHandle, 0, SEEK_SET);
	lua_pushinteger(L, size);
	return 1;
}

static int lua_checkexist(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char *file_tbo = luaL_checkstring(L, 1);
	SceUID fileHandle = sceIoOpen(file_tbo, SCE_O_RDONLY, 0777);
	if (fileHandle < 0) lua_pushboolean(L, false);
	else{
		sceIoClose(fileHandle);
		lua_pushboolean(L,true);
	}
	return 1;
}

static int lua_rename(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char *old_file = luaL_checkstring(L, 1);
	const char *new_file = luaL_checkstring(L, 2);
	sceIoRename(old_file, new_file);
	return 0;
}

static int lua_removef(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char *old_file = luaL_checkstring(L, 1);
	sceIoRemove(old_file);
	return 0;
}

static int lua_removef2(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char *old_file = luaL_checkstring(L, 1);
	sceIoRmdir(old_file);
	return 0;
}

static int lua_newdir(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char *newdir = luaL_checkstring(L, 1);
	sceIoMkdir(newdir, 0777);
	return 0;
}

static int lua_exit(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	char stringbuffer[256];
	strcpy(stringbuffer,"lpp_shutdown");
	luaL_dostring(L, "collectgarbage()");
	return luaL_error(L, stringbuffer); //Fake LUA error
}

static int lua_wait(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	int microsecs = luaL_checkinteger(L, 1);
	sceKernelDelayThread(microsecs);
	return 0;
}


SceIoDirent g_dir;

static int lua_dir(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0 && argc != 1) return luaL_error(L, "System.listDirectory([path]) takes zero or one argument");


    const char *path = "";
    if (argc == 0) {
        path = "";
    } else {
        path = luaL_checkstring(L, 1);
    }
    int fd = sceIoDopen(path);

    if (fd < 0) {
        lua_pushnil(L);  /* return nil */
        return 1;
    }
    lua_newtable(L);
    int i = 1;
    while (sceIoDread(fd, &g_dir) > 0) {
        lua_pushnumber(L, i++);  /* push key for file entry */

        lua_newtable(L);
            lua_pushstring(L, "name");
            lua_pushstring(L, g_dir.d_name);
            lua_settable(L, -3);

            lua_pushstring(L, "size");
            lua_pushnumber(L, g_dir.d_stat.st_size);
            lua_settable(L, -3);

            lua_pushstring(L, "directory");
            lua_pushboolean(L, SCE_S_ISDIR(g_dir.d_stat.st_mode));
            lua_settable(L, -3);

        lua_settable(L, -3);
    }

    sceIoDclose(fd);

    return 1;  /* table is already on top */
}

static int lua_charging(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushboolean(L, scePowerIsBatteryCharging());
	return 1;
}

static int lua_percent(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushinteger(L, scePowerGetBatteryLifePercent());
	return 1;
}

static int lua_lifetime(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushinteger(L, scePowerGetBatteryLifeTime());
	return 1;
}

static int lua_nopower(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	sceKernelPowerTick(0);
	return 0;
}

//Register our System Functions
static const luaL_Reg System_functions[] = {

  // Dofile & I/O Library patched functions
  {"doNotUse",							lua_dofile},
  {"doNotOpen",							lua_openfile},
  {"doNotRead",							lua_readfile},
  {"doNotWrite",						lua_writefile},
  {"doNotClose",						lua_closefile},  
  {"doNotSeek",							lua_seekfile},  
  {"doNotSize",							lua_sizefile},  
  {"doesFileExist",						lua_checkexist},
  {"exit",							lua_exit},
  {"rename",							lua_rename},
  {"deleteFile",						lua_removef},
  {"deleteDirectory",						lua_removef2},
  {"createDirectory",						lua_newdir},
  {"listDirectory",						lua_dir},
  {"wait",							lua_wait},
  {"isBatteryCharging",						lua_charging},
  {"getBatteryPercentage",					lua_percent},
  {"getBatteryLifetime",					lua_lifetime},
  {"powerTick",							lua_nopower},
  {0, 0}
};

void luaSystem_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, System_functions, 0);
	lua_setglobal(L, "System");
	VariableRegister(L,FREAD);
	VariableRegister(L,FWRITE);
	VariableRegister(L,FCREATE);
	VariableRegister(L,FRDWR);
	VariableRegister(L,SET);
	VariableRegister(L,END);
	VariableRegister(L,CUR);
}
