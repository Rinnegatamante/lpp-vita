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
#include <vitasdk.h>
#include "include/luaplayer.h"
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)
#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)

SceCtrlActuator pad1, pad2;

static int lua_readC(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0 && argc != 1) return luaL_error(L, "wrong number of arguments.");
	int port = 0;
	if (argc == 1) port = luaL_checkinteger(L, 1); 
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(port, &pad, 1);
	lua_pushinteger(L, pad.buttons);
	return 1;
}

static int lua_readleft(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0 && argc != 1) return luaL_error(L, "wrong number of arguments.");
	int port = 0;
	if (argc == 1) port = luaL_checkinteger(L, 1); 
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(port, &pad, 1);
	lua_pushinteger(L, pad.lx);
	lua_pushinteger(L, pad.ly);
	return 2;
}

static int lua_readright(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0 && argc != 1) return luaL_error(L, "wrong number of arguments.");
	int port = 0;
	if (argc == 1) port = luaL_checkinteger(L, 1); 
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(port, &pad, 1);
	lua_pushinteger(L, pad.rx);
	lua_pushinteger(L, pad.ry);
	return 2;
}

static int lua_check(lua_State *L)
{
	if (lua_gettop(L) != 2) return luaL_error(L, "wrong number of arguments.");
	int pad = luaL_checkinteger(L, 1);
	int button = luaL_checkinteger(L, 2);
	lua_pushboolean(L, (pad & button));
	return 1;
}

static int lua_touchpad(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments.");
	SceTouchData touch;
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
	for (SceUInt32 i=0;i<touch.reportNum;i++){
		lua_pushinteger(L, lerp(touch.report[i].x, 1920, 960));
		lua_pushinteger(L, lerp(touch.report[i].y, 1088, 544));
	}
	return touch.reportNum<<1;
}

static int lua_rumble(lua_State *L)
{
	if (lua_gettop(L) != 3) return luaL_error(L, "wrong number of arguments.");
	int port = luaL_checkinteger(L, 1);
	uint8_t int_small = luaL_checkinteger(L, 2);
	uint8_t int_large = luaL_checkinteger(L, 3);
	switch (port){
		case 0:
		case 1:
			pad1.small = int_small;
			pad1.large = int_large;
			sceCtrlSetActuator(1, &pad1);
			break;
		case 2:
			pad2.small = int_small;
			pad2.large = int_large;
			sceCtrlSetActuator(2, &pad2);
			break;
		default:
			return luaL_error(L, "wrong port number.");
			break;
	}	
	return 0;
}

static int lua_lightbar(lua_State *L)
{
	if (lua_gettop(L) != 2) return luaL_error(L, "wrong number of arguments.");
	int port = luaL_checkinteger(L, 1);
	uint32_t color = luaL_checkinteger(L, 2);
	switch (port){
		case 0:
		case 1:
			sceCtrlSetLightBar(1, color & 0xFF, (color>>8) & 0xFF, (color>>16) & 0xFF);
			break;
		case 2:
			sceCtrlSetLightBar(2, color & 0xFF, (color>>8) & 0xFF, (color>>16) & 0xFF);
			break;
		default:
			return luaL_error(L, "wrong port number.");
			break;
	}	
	return 0;
}

static int lua_touchpad2(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments.");
	SceTouchData touch;
	sceTouchPeek(SCE_TOUCH_PORT_BACK, &touch, 1);
	for (SceUInt32 i=0;i<touch.reportNum;i++){
		lua_pushinteger(L, lerp(touch.report[i].x, 1920, 960));
		lua_pushinteger(L, lerp(touch.report[i].y, 1088, 544));
	}
	return touch.reportNum<<1;
}

//Register our Controls Functions
static const luaL_Reg Controls_functions[] = {
  {"read",								lua_readC},	
  {"readLeftAnalog",					lua_readleft},	  
  {"readRightAnalog",					lua_readright},	
  {"rumble",							lua_rumble},
  {"setLightbar",						lua_lightbar},
  {"check",								lua_check},	
  {"readTouch",							lua_touchpad},	
  {"readRetroTouch",					lua_touchpad2},	
  {0, 0}
};

void luaControls_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Controls_functions, 0);
	lua_setglobal(L, "Controls");
	uint8_t FIRST_CTRL = 0;
	uint8_t SECOND_CTRL = 2;
	VariableRegister(L,FIRST_CTRL);
	VariableRegister(L,SECOND_CTRL);
	VariableRegister(L,SCE_CTRL_UP);
	VariableRegister(L,SCE_CTRL_DOWN);
	VariableRegister(L,SCE_CTRL_LEFT);
	VariableRegister(L,SCE_CTRL_RIGHT);
	VariableRegister(L,SCE_CTRL_CROSS);
	VariableRegister(L,SCE_CTRL_CIRCLE);
	VariableRegister(L,SCE_CTRL_SQUARE);
	VariableRegister(L,SCE_CTRL_TRIANGLE);
	VariableRegister(L,SCE_CTRL_LTRIGGER);
	VariableRegister(L,SCE_CTRL_RTRIGGER);
	VariableRegister(L,SCE_CTRL_START);
	VariableRegister(L,SCE_CTRL_SELECT);
}