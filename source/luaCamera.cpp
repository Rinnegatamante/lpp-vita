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
#include <vita2d.h>

#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

#define FRAMERATES_NUM 9 // Number of available framerates

SceCameraInfo cam_info;
SceCameraRead cam_info_read;
lpp_texture* cam_buf;
uint8_t cam_type = 0;
uint8_t cur_buf = 0;
bool isCamOn = false;

// Camera resolutions
uint8_t VGA_RES = SCE_CAMERA_RESOLUTION_640_480;
uint8_t QVGA_RES = SCE_CAMERA_RESOLUTION_320_240;
uint8_t QQVGA_RES = SCE_CAMERA_RESOLUTION_160_120;
uint8_t CIF_RES = SCE_CAMERA_RESOLUTION_352_288;
uint8_t QCIF_RES = SCE_CAMERA_RESOLUTION_176_144;
uint8_t PSP_RES = SCE_CAMERA_RESOLUTION_480_272;
uint8_t NGP_RES = SCE_CAMERA_RESOLUTION_640_360;

// Camera framerates
uint8_t framerates[FRAMERATES_NUM] = {3, 5, 7, 10, 15, 20, 30, 60, 120};

void initCam(uint8_t type, uint8_t res, uint8_t fps){
	uint16_t width;
	uint16_t height;
	bool high_fps_ready = false;
	if (res == VGA_RES){
		width = 640;
		height = 480;
	}else if (res == QVGA_RES){
		width = 320;
		height = 240;
		high_fps_ready = true;
	}else if (res == QQVGA_RES){
		width = 160;
		height = 120;
		high_fps_ready = true;
	}else if (res == CIF_RES){
		width = 352;
		height = 288;
		high_fps_ready = true;
	}else if (res == QCIF_RES){
		width = 176;
		height = 144;
	}else if (res == PSP_RES){
		width = 480;
		height = 272;
	}else if (res == NGP_RES){
		width = 640;
		height = 360;
	}
	
	// Initializing camera buffers
	SceKernelMemBlockType orig = vita2d_texture_get_alloc_memblock_type();
#ifndef SYS_APP_MODE
	vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW);
#endif
	cam_buf = (lpp_texture*)malloc(sizeof(lpp_texture));
	cam_buf->text = vita2d_create_empty_texture(width, height);
	cam_buf->magic = 0xABADBEEF;
	vita2d_texture_set_alloc_memblock_type(orig);
	
	// Initializing camera structs
	cam_info.size = sizeof(SceCameraInfo);
	cam_info.format = SCE_CAMERA_FORMAT_ABGR;
	cam_info.resolution = res;
	cam_info.pitch = vita2d_texture_get_stride(cam_buf->text) - (width << 2);
	cam_info.sizeIBase = (width * height) << 2;
	cam_info.pIBase = vita2d_texture_get_datap(cam_buf->text);
	
	cam_info_read.size = sizeof(SceCameraRead);
	cam_info_read.mode = 0;
	
	// Setting framerate
	if ((fps == 120) && (!high_fps_ready)) cam_info.framerate = 60;
	else cam_info.framerate = fps;
	bool framerateFound = false;
	for (int i = 0; i < FRAMERATES_NUM; i++){
		if (fps == framerates[i]){
			framerateFound = true;
			break;
		}
	}
	if (!framerateFound) cam_info.framerate = 30;
	
	sceCameraOpen(type, &cam_info);
	sceCameraStart(type);
	cam_type = type;
}

static int lua_caminit(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments.");
	if (isCamOn) return luaL_error(L, "cannot start two camera instances together");
	#endif
	uint8_t type = (uint8_t)luaL_checkinteger(L, 1);
	uint8_t resolution = (uint8_t)luaL_checkinteger(L,2);
	uint8_t framerate = (uint8_t)luaL_checkinteger(L,3);
	initCam(type, resolution, framerate);
	isCamOn = true;
	return 0;
}

static int lua_camoutput(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	sceCameraRead(cam_type, &cam_info_read);
	uint32_t res = (uint32_t)cam_buf;
	lua_pushinteger(L, res);
	return 1;
}

static int lua_camexit(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	isCamOn = false;
	sceCameraStop(cam_type);
	sceCameraClose(cam_type);
	vita2d_free_texture(cam_buf->text);
	free(cam_buf);
	return 0;
}

static int lua_sbright(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	if (val < 0) val = 0;
	else if (val > 255) val = 255;
	sceCameraSetBrightness(cam_type, val);
	return 0;
}

static int lua_ssat(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	float val = luaL_checknumber(L, 1);
	int val_int = val * 10;
	sceCameraSetSaturation(cam_type, val_int);
	return 0;
}

static int lua_ssharp(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	sceCameraSetSharpness(cam_type, val / 100);
	return 0;
}

static int lua_scontrast(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	if (val < 0) val = 0;
	else if (val > 255) val = 255;
	sceCameraSetContrast(cam_type, val);
	return 0;
}

static int lua_srev(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	sceCameraSetReverse(cam_type, val);
	return 0;
}

static int lua_seffect(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	sceCameraSetEffect(cam_type, val);
	return 0;
}

static int lua_sexp(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	float val = luaL_checknumber(L, 1);
	int val_int = val * 10;
	sceCameraSetEV(cam_type, val_int);
	return 0;
}

static int lua_szoom(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	sceCameraSetZoom(cam_type, val);
	return 0;
}

static int lua_santiflicker(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	sceCameraSetAntiFlicker(cam_type, val);
	return 0;
}

static int lua_siso(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	sceCameraSetISO(cam_type, val);
	return 0;
}

static int lua_sgain(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	if (val < 0) val = 0;
	else if (val > 16) val = 16;
	sceCameraSetGain(cam_type, val);
	return 0;
}

static int lua_swhite(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	sceCameraSetWhiteBalance(cam_type, val);
	return 0;
}

static int lua_slight(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	sceCameraSetBacklight(cam_type, val);
	return 0;
}

static int lua_snight(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val = luaL_checkinteger(L, 1);
	sceCameraSetNightmode(cam_type, val);
	return 0;
}

static int lua_gbright(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetNightmode(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_gsat(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetSaturation(cam_type, &val);
	float val_num = val / 10.0f;
	lua_pushnumber(L, val_num);
	return 1;
}

static int lua_gsharp(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetSharpness(cam_type, &val);
	lua_pushinteger(L, val * 100);
	return 1;
}

static int lua_gcontrast(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetContrast(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_grev(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetReverse(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_geffect(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetEffect(cam_type, &val);
	float val_num = val / 10.0f;
	lua_pushnumber(L, val_num);
	return 1;
}

static int lua_gexp(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetEV(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_gzoom(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetZoom(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_gantiflicker(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetAntiFlicker(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_giso(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetISO(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_ggain(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetGain(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_gwhite(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetWhiteBalance(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_glight(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetBacklight(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

static int lua_gnight(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isCamOn) return luaL_error(L, "camera has not been initialized.");
	#endif
	int val;
	sceCameraGetNightmode(cam_type, &val);
	lua_pushinteger(L, val);
	return 1;
}

//Register our Camera Functions
static const luaL_Reg Camera_functions[] = {
	{"init",              lua_caminit},
	{"getOutput",         lua_camoutput},
	{"setBrightness",     lua_sbright},
	{"setSaturation",     lua_ssat},
	{"setSharpness",      lua_ssharp},
	{"setContrast",       lua_scontrast},
	{"setReverse",        lua_srev},
	{"setEffect",         lua_seffect},
	{"setExposure",       lua_sexp},
	{"setZoom",           lua_szoom},
	{"setAntiFlicker",    lua_santiflicker},
	{"setISO",            lua_siso},
	{"setGain",           lua_sgain},
	{"setWhiteBalance",   lua_swhite},
	{"setBacklight",      lua_slight},
	{"setNightmode",      lua_snight},
	{"getBrightness",     lua_gbright},
	{"getSaturation",     lua_gsat},
	{"getSharpness",      lua_gsharp},
	{"getContrast",       lua_gcontrast},
	{"getReverse",        lua_grev},
	{"getEffect",         lua_geffect},
	{"getExposure",       lua_gexp},
	{"getZoom",           lua_gzoom},
	{"getAntiFlicker",    lua_gantiflicker},
	{"getISO",            lua_giso},
	{"getGain",           lua_ggain},
	{"getWhiteBalance",   lua_gwhite},
	{"getBacklight",      lua_glight},
	{"getNightmode",      lua_gnight},
	{"term",              lua_camexit},
	{0, 0}
};

void luaCamera_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Camera_functions, 0);
	lua_setglobal(L, "Camera");
	
	// Camera combinations
	uint8_t INNER_CAM = SCE_CAMERA_DEVICE_FRONT;
	uint8_t OUTER_CAM = SCE_CAMERA_DEVICE_BACK;
	VariableRegister(L,INNER_CAM);
	VariableRegister(L,OUTER_CAM);
	
	// Internal camera settings
	uint8_t ANTIFLICKER_AUTO = SCE_CAMERA_ANTIFLICKER_AUTO;
	uint8_t ANTIFLICKER_50HZ = SCE_CAMERA_ANTIFLICKER_50HZ;
	uint8_t ANTIFLICKER_60HZ = SCE_CAMERA_ANTIFLICKER_60HZ;
	uint16_t ISO_AUTO = SCE_CAMERA_ISO_AUTO;
	uint16_t ISO_100 = SCE_CAMERA_ISO_100;
	uint16_t ISO_200 = SCE_CAMERA_ISO_200;
	uint16_t ISO_400 = SCE_CAMERA_ISO_400;
	uint8_t WB_AUTO = SCE_CAMERA_WB_AUTO;
	uint8_t WB_DAYLIGHT = SCE_CAMERA_WB_DAY;
	uint8_t WB_CWF = SCE_CAMERA_WB_CWF;
	uint8_t WB_SLSA = SCE_CAMERA_WB_SLSA;
	uint8_t BACKLIGHT_OFF = SCE_CAMERA_BACKLIGHT_OFF;
	uint8_t BACKLIGHT_ON = SCE_CAMERA_BACKLIGHT_ON;
	uint8_t NIGHTMODE_OFF = SCE_CAMERA_NIGHTMODE_OFF;
	uint8_t NIGHTMODE_LOW = SCE_CAMERA_NIGHTMODE_LESS10;
	uint8_t NIGHTMODE_MED = SCE_CAMERA_NIGHTMODE_LESS100;
	uint8_t NIGHTMODE_HIGH = SCE_CAMERA_NIGHTMODE_OVER100;
	uint8_t REVERSE_OFF = SCE_CAMERA_REVERSE_OFF;
	uint8_t REVERSE_MIRROR = SCE_CAMERA_REVERSE_MIRROR;
	uint8_t REVERSE_FLIP = SCE_CAMERA_REVERSE_FLIP;
	uint8_t REVERSE_BOTH = SCE_CAMERA_REVERSE_MIRROR_FLIP;
	uint8_t EFFECT_NONE = SCE_CAMERA_EFFECT_NORMAL;
	uint8_t EFFECT_NEGATIVE = SCE_CAMERA_EFFECT_NEGATIVE;
	uint8_t EFFECT_BLACKWHITE = SCE_CAMERA_EFFECT_BLACKWHITE;
	uint8_t EFFECT_SEPIA = SCE_CAMERA_EFFECT_SEPIA;
	uint8_t EFFECT_BLUE = SCE_CAMERA_EFFECT_BLUE;
	uint8_t EFFECT_RED = SCE_CAMERA_EFFECT_RED;
	uint8_t EFFECT_GREEN = SCE_CAMERA_EFFECT_GREEN;
	VariableRegister(L,ANTIFLICKER_AUTO);
	VariableRegister(L,ANTIFLICKER_50HZ);
	VariableRegister(L,ANTIFLICKER_60HZ);
	VariableRegister(L,ISO_AUTO);
	VariableRegister(L,ISO_100);
	VariableRegister(L,ISO_200);
	VariableRegister(L,ISO_400);
	VariableRegister(L,WB_AUTO);
	VariableRegister(L,WB_DAYLIGHT);
	VariableRegister(L,WB_CWF);
	VariableRegister(L,WB_SLSA);
	VariableRegister(L,BACKLIGHT_OFF);
	VariableRegister(L,BACKLIGHT_ON);
	VariableRegister(L,NIGHTMODE_OFF);
	VariableRegister(L,NIGHTMODE_LOW);
	VariableRegister(L,NIGHTMODE_MED);
	VariableRegister(L,NIGHTMODE_HIGH);
	VariableRegister(L,REVERSE_OFF);
	VariableRegister(L,REVERSE_MIRROR);
	VariableRegister(L,REVERSE_FLIP);
	VariableRegister(L,REVERSE_BOTH);
	VariableRegister(L,EFFECT_NONE);
	VariableRegister(L,EFFECT_NEGATIVE);
	VariableRegister(L,EFFECT_BLACKWHITE);
	VariableRegister(L,EFFECT_SEPIA);
	VariableRegister(L,EFFECT_BLUE);
	VariableRegister(L,EFFECT_RED);
	VariableRegister(L,EFFECT_GREEN);
	
	// Camera Resolutions
	VariableRegister(L,VGA_RES);
	VariableRegister(L,QVGA_RES);
	VariableRegister(L,QQVGA_RES);
	VariableRegister(L,CIF_RES);
	VariableRegister(L,QCIF_RES);
	VariableRegister(L,PSP_RES);
	VariableRegister(L,NGP_RES);

}