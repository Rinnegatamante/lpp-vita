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
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include "include/luaplayer.h"
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

static int lua_readC(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
	lua_pushnumber(L, pad.buttons);
	return 1;
}

static int lua_readleft(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
	lua_pushnumber(L, pad.lx);
	lua_pushnumber(L, pad.ly);
	return 2;
}

static int lua_readright(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
	lua_pushnumber(L, pad.rx);
	lua_pushnumber(L, pad.ry);
	return 2;
}

static int lua_check(lua_State *L)
{
        if (lua_gettop(L) != 2) return luaL_error(L, "wrong number of arguments.");
		int pad = luaL_checknumber(L, 1);
		int button = luaL_checknumber(L, 2);
		lua_pushboolean(L, (pad & button));
        return 1;
}

static int lua_touchpad(lua_State *L)
{
        if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments.");
		SceTouchData touch;
		sceTouchPeek(0, &touch, 1);
		lua_pushnumber(L, touch.report[0].x);
		lua_pushnumber(L, touch.report[0].y);
        return 2;
}

//Register our Controls Functions
static const luaL_Reg Controls_functions[] = {
  {"read",								lua_readC},	
  {"readLeftAnalog",					lua_readleft},	  
  {"readRightAnalog",					lua_readright},	
  {"check",								lua_check},	
  {"readTouch",							lua_touchpad},	
  {0, 0}
};

void luaControls_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Controls_functions, 0);
	lua_setglobal(L, "Controls");
	VariableRegister(L,PSP2_CTRL_UP);
	VariableRegister(L,PSP2_CTRL_DOWN);
	VariableRegister(L,PSP2_CTRL_LEFT);
	VariableRegister(L,PSP2_CTRL_RIGHT);
	VariableRegister(L,PSP2_CTRL_CROSS);
	VariableRegister(L,PSP2_CTRL_CIRCLE);
	VariableRegister(L,PSP2_CTRL_SQUARE);
	VariableRegister(L,PSP2_CTRL_TRIANGLE);
	VariableRegister(L,PSP2_CTRL_LTRIGGER);
	VariableRegister(L,PSP2_CTRL_RTRIGGER);
	VariableRegister(L,PSP2_CTRL_START);
	VariableRegister(L,PSP2_CTRL_SELECT);
}