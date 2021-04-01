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

static SceCtrlActuator actuators[4];
static bool sensors_enabled = false;

static int lua_readC(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0 && argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif
	int port = 0;
	if (argc == 1){
		port = luaL_checkinteger(L, 1);
#ifndef SKIP_ERROR_HANDLING
		if (port > 5) return luaL_error(L, "wrong port number.");
#endif
	}
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(port, &pad, 1);
	lua_pushinteger(L, pad.buttons);
	return 1;
}

static int lua_readleft(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0 && argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif
	int port = 0;
	if (argc == 1){
		port = luaL_checkinteger(L, 1);
#ifndef SKIP_ERROR_HANDLING
		if (port > 5) return luaL_error(L, "wrong port number.");
#endif
	}
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(port, &pad, 1);
	lua_pushinteger(L, pad.lx);
	lua_pushinteger(L, pad.ly);
	return 2;
}

static int lua_readright(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0 && argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif
	int port = 0;
	if (argc == 1){
		port = luaL_checkinteger(L, 1);
#ifndef SKIP_ERROR_HANDLING
		if (port > 5) return luaL_error(L, "wrong port number.");
#endif
	}
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(port, &pad, 1);
	lua_pushinteger(L, pad.rx);
	lua_pushinteger(L, pad.ry);
	return 2;
}

static int lua_check(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments.");
#endif
	int pad = luaL_checkinteger(L, 1);
	int button = luaL_checkinteger(L, 2);
	lua_pushboolean(L, (pad & button));
	return 1;
}

static int lua_touchpad(lua_State *L){
	int argc = lua_gettop(L);
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	SceTouchData touch;
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
	for (SceUInt32 i=0;i<touch.reportNum;i++){
		lua_pushinteger(L, lerp(touch.report[i].x, 1920, 960));
		lua_pushinteger(L, lerp(touch.report[i].y, 1088, 544));
	}
	return touch.reportNum<<1;
}

static int lua_rumble(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments.");
#endif
	int port = luaL_checkinteger(L, 1);
#ifndef SKIP_ERROR_HANDLING
	if (port > 5) return luaL_error(L, "wrong port number.");
#endif
	if (port == 0) port = 1;
	uint8_t int_small = luaL_checkinteger(L, 2);
	uint8_t int_large = luaL_checkinteger(L, 3);
	actuators[port-1].small = int_small;
	actuators[port-1].small = int_large;
	sceCtrlSetActuator(port, &actuators[port-1]);
	return 0;
}

static int lua_lightbar(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments.");
#endif
	int port = luaL_checkinteger(L, 1);
#ifndef SKIP_ERROR_HANDLING
	if (port > 5) return luaL_error(L, "wrong port number.");
#endif
	if (port == 0) port = 1;
	uint32_t color = luaL_checkinteger(L, 2);
	sceCtrlSetLightBar(port, color & 0xFF, (color>>8) & 0xFF, (color>>16) & 0xFF);
	return 0;
}

static int lua_touchpad2(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
#endif
	SceTouchData touch;
	sceTouchPeek(SCE_TOUCH_PORT_BACK, &touch, 1);
	for (SceUInt32 i=0;i<touch.reportNum;i++){
		lua_pushinteger(L, lerp(touch.report[i].x, 1920, 960));
		lua_pushinteger(L, lerp(touch.report[i].y, 1088, 544));
	}
	return touch.reportNum<<1;
}

static int lua_lock(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
#endif
	sceShellUtilLock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN | SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU));
	return 0;
}

static int lua_unlock(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
#endif
	sceShellUtilUnlock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN | SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU));
	return 0;
}

static int lua_gettype(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	SceCtrlPortInfo pinfo;
	sceCtrlGetControllerPortInfo(&pinfo);
	lua_newtable(L);
	int i = 1;
	while (i <= 5) {
		lua_pushnumber(L, i);
		lua_newtable(L);
		lua_pushstring(L, "type");
		lua_pushinteger(L, pinfo.port[i-1]);
		lua_settable(L, -3);
		lua_settable(L, -3);
		i++;
	}
	return 1;
}

static int lua_headset(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
	lua_pushboolean(L, (pad.buttons & SCE_CTRL_HEADPHONE));
	return 1;
}

static int lua_accel(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!sensors_enabled) return luaL_error(L, "you must enable sensors reading to use this function.");
#endif
	SceMotionSensorState sensor;
	sceMotionGetSensorState(&sensor, 1);
	lua_pushnumber(L, sensor.accelerometer.x);
	lua_pushnumber(L, sensor.accelerometer.y);
	lua_pushnumber(L, sensor.accelerometer.z);
	return 3;
}

static int lua_gyro(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!sensors_enabled) return luaL_error(L, "you must enable sensors reading to use this function.");
#endif
	SceMotionSensorState sensor;
	sceMotionGetSensorState(&sensor, 1);
	lua_pushnumber(L, sensor.gyro.x);
	lua_pushnumber(L, sensor.gyro.y);
	lua_pushnumber(L, sensor.gyro.z);
	return 3;
}

static int lua_enablesensors(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	if (!sensors_enabled){
		sceMotionStartSampling();
		sceMotionMagnetometerOn();
		sensors_enabled = true;
	}
	return 0;
}

static int lua_disablesensors(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	if (sensors_enabled){
		sceMotionMagnetometerOff();
		sceMotionStopSampling();
		sensors_enabled = false;
	}
	return 0;
}

static int lua_getenter(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	int val;
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &val);
	if (val == 0) lua_pushinteger(L, SCE_CTRL_CIRCLE);
	else lua_pushinteger(L, SCE_CTRL_CROSS);
	return 1;
}

//Register our Controls Functions
static const luaL_Reg Controls_functions[] = {
  {"read",             lua_readC},	
  {"readLeftAnalog",   lua_readleft},	  
  {"readRightAnalog",  lua_readright},	
  {"rumble",           lua_rumble},
  {"setLightbar",      lua_lightbar},
  {"check",            lua_check},	
  {"readTouch",        lua_touchpad},	
  {"readRetroTouch",   lua_touchpad2},	
  {"lockHomeButton",   lua_lock},	
  {"unlockHomeButton", lua_unlock},	
  {"getDeviceInfo",    lua_gettype},
  {"headsetStatus",    lua_headset},
  {"readAccel",        lua_accel},
  {"readGyro",         lua_gyro},
  {"enableGyro",       lua_enablesensors},	
  {"enableAccel",      lua_enablesensors},	
  {"disableGyro",      lua_disablesensors},	
  {"disableAccel",     lua_disablesensors},
  {"getEnterButton",   lua_getenter}, 
  {0, 0}
};

void luaControls_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Controls_functions, 0);
	lua_setglobal(L, "Controls");
	uint8_t UNPAIRED_DEV = 0;
	uint8_t VITA_DEV = 1;
	uint8_t VIRTUAL_DEV = 2;
	uint8_t DS3_DEV = 4;
	uint8_t DS4_DEV = 8;
	VariableRegister(L,UNPAIRED_DEV);
	VariableRegister(L,VITA_DEV);
	VariableRegister(L,VIRTUAL_DEV);
	VariableRegister(L,DS3_DEV);
	VariableRegister(L,DS4_DEV);
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
	VariableRegister(L,SCE_CTRL_POWER);
	VariableRegister(L,SCE_CTRL_VOLUP);
	VariableRegister(L,SCE_CTRL_VOLDOWN);
	VariableRegister(L,SCE_CTRL_PSBUTTON);
}