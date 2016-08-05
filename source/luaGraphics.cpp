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

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/moduleinfo.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <vita2d.h>
#include "include/luaplayer.h"
extern "C"{
	#include "include/draw/font.h"
}

struct ttf{
	uint32_t magic;
	vita2d_font* f;
	int size;
};


static int lua_print(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 4) return luaL_error(L, "wrong number of arguments.");
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	char* text = (char*)luaL_checkstring(L, 3);
	int color = luaL_checkinteger(L, 4);
	font_draw_string(x, y, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF), text);
	return 0;
}

static int lua_pixel(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3) return luaL_error(L, "wrong number of arguments.");
	float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
	int color = luaL_checkinteger(L, 3);
	vita2d_draw_pixel(x, y, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}

static int lua_rect(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 5) return luaL_error(L, "wrong number of arguments.");
	float x1 = luaL_checknumber(L, 1);
    float x2 = luaL_checknumber(L, 2);
	float y1 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
	int color = luaL_checkinteger(L,5);
	if (x2 < x1){
		int tmp = x2;
		x2 = x1;
		x1 = tmp;
	}
	if (y2 < y1){
		int tmp = y2;
		y2 = y1;
		y1 = tmp;
	}
	vita2d_draw_rectangle(x1, y1, x2-x1, y2-y1, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}

static int lua_line(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 5) return luaL_error(L, "wrong number of arguments.");
	float x1 = luaL_checknumber(L, 1);
    float x2 = luaL_checknumber(L, 2);
	float y1 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
	if (x2 < x1){
		int tmp = x2;
		x2 = x1;
		x1 = tmp;
	}
	if (y2 < y1){
		int tmp = y2;
		y2 = y1;
		y1 = tmp;
	}
	int color = luaL_checkinteger(L,5);
	vita2d_draw_line(x1, y1, x2, y2, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}

static int lua_circle(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 4) return luaL_error(L, "wrong number of arguments.");
	float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
	float radius = luaL_checknumber(L, 3);
	int color = luaL_checkinteger(L,4);
	vita2d_draw_fill_circle(x, y, radius, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}

static int lua_init(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");	
    vita2d_start_drawing();
    return 0;
}

static int lua_term(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
    vita2d_end_drawing();
	vita2d_wait_rendering_done();
    return 0;
}

static int lua_loadimg(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	char* text = (char*)(luaL_checkstring(L, 1));
	SceUID file = sceIoOpen(text, SCE_O_RDONLY, 0777);
	uint16_t magic;
	sceIoRead(file, &magic, 2);
	sceIoClose(file);
	vita2d_texture* result;
	if (magic == 0x4D42) result = vita2d_load_BMP_file(text);
	else if (magic == 0xD8FF) result = vita2d_load_JPEG_file(text);
	else if (magic == 0x5089) result = vita2d_load_PNG_file(text);
	else return luaL_error(L, "Error loading image.");
    lua_pushinteger(L, (uint32_t)(result));
	return 1;
}

static int lua_drawimg(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
	vita2d_texture* text = (vita2d_texture*)(luaL_checkinteger(L, 3));
	vita2d_draw_texture(text, x, y);
	return 0;
}

static int lua_drawimg_rotate(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 4) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
	float radius = luaL_checknumber(L, 3);
	vita2d_texture* text = (vita2d_texture*)(luaL_checkinteger(L, 4));
	vita2d_draw_texture_rotate(text, x, y, radius);
	return 0;
}

static int lua_drawimg_scale(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 5) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
	float x_scale = luaL_checknumber(L, 3);
    float y_scale = luaL_checknumber(L, 4);
	vita2d_texture* text = (vita2d_texture*)(luaL_checkinteger(L, 5));
	vita2d_draw_texture_scale(text, x, y, x_scale, y_scale);
	return 0;
}

static int lua_drawimg_part(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 7) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
	int st_x = luaL_checkinteger(L, 3);
    int st_y = luaL_checkinteger(L, 4);
	float width = luaL_checknumber(L, 5);
    float height = luaL_checknumber(L, 6);
	vita2d_texture* text = (vita2d_texture*)(luaL_checkinteger(L, 7));
	vita2d_draw_texture_part(text, x, y, st_x, st_y, width, height);
	return 0;
}

static int lua_drawimg_full(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 9) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
	int st_x = luaL_checkinteger(L, 3);
    int st_y = luaL_checkinteger(L, 4);
	float width = luaL_checknumber(L, 5);
    float height = luaL_checknumber(L, 6);
	float x_scale = luaL_checknumber(L, 7);
    float y_scale = luaL_checknumber(L, 8);
	vita2d_texture* text = (vita2d_texture*)(luaL_checkinteger(L, 9));
	vita2d_draw_texture_part_scale(text, x, y, st_x, st_y, width, height, x_scale, y_scale);
	return 0;
}

static int lua_width(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	vita2d_texture* text = (vita2d_texture*)(luaL_checkinteger(L, 1));
	lua_pushinteger(L, vita2d_texture_get_width(text));
	return 1;
}

static int lua_height(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	vita2d_texture* text = (vita2d_texture*)(luaL_checkinteger(L, 1));
	lua_pushinteger(L, vita2d_texture_get_height(text));
	return 1;
}

static int lua_free(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	vita2d_texture* text = (vita2d_texture*)(luaL_checkinteger(L, 1));
	vita2d_free_texture(text);
	return 0;
}

static int lua_createimage(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
	int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);
	vita2d_texture* text = vita2d_create_empty_texture(w, h);
	lua_pushinteger(L, vita2d_texture_get_height(text));
	return 1;
}

static int lua_loadFont(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	char* text = (char*)(luaL_checkstring(L, 1));
	ttf* result = (ttf*)malloc(sizeof(ttf));
	result->size = 16;
	result->f = vita2d_load_font_file(text);
	result->magic = 0x4C464E54;
	lua_pushinteger(L,(uint32_t)result);
    return 1;
}

static int lua_fsize(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
	ttf* font = (ttf*)(luaL_checkinteger(L, 1));
	int size = luaL_checkinteger(L,2);
	#ifndef SKIP_ERROR_HANDLING
		if (font->magic != 0x4C464E54) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	font->size = size;
    return 0;
}

static int lua_unloadFont(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	ttf* font = (ttf*)(luaL_checkinteger(L, 1));
	#ifndef SKIP_ERROR_HANDLING
		if (font->magic != 0x4C464E54) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	vita2d_free_font(font->f);
	free(font);
    return 0;
}

static int lua_fprint(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5) return luaL_error(L, "wrong number of arguments");
	ttf* font = (ttf*)(luaL_checkinteger(L, 1));
	float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
	char* text = (char*)(luaL_checkstring(L, 4));
	uint32_t color = luaL_checkinteger(L,5);
	#ifndef SKIP_ERROR_HANDLING
		if (font->magic != 0x4C464E54) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	vita2d_font_draw_text(font->f, x, y, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF), font->size, text);
    return 0;
}

//Register our Graphics Functions
static const luaL_Reg Graphics_functions[] = {
  {"initBlend",							lua_init},
  {"termBlend",							lua_term},
  {"debugPrint",						lua_print},
  {"drawPixel",							lua_pixel},
  {"drawLine",							lua_line},
  {"fillRect",							lua_rect},
  {"fillCircle",						lua_circle},
  {"loadImage",							lua_loadimg},
  {"drawImage",							lua_drawimg},
  {"drawRotateImage",					lua_drawimg_rotate},
  {"drawScaleImage",					lua_drawimg_scale},
  {"drawPartialImage",					lua_drawimg_part},
  {"drawImageExtended",					lua_drawimg_full},
  {"createImage",						lua_createimage},
  {"getImageWidth",						lua_width},
  {"getImageHeight",					lua_height},
  {"freeImage",							lua_free},
  {0, 0}
};

//Register our Font Functions
static const luaL_Reg Font_functions[] = {
  {"load",					lua_loadFont}, 
  {"print",					lua_fprint}, 
  {"setPixelSizes",			lua_fsize}, 
  {"unload",				lua_unloadFont}, 
  {0, 0}
};

void luaGraphics_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Graphics_functions, 0);
	lua_setglobal(L, "Graphics");
	lua_newtable(L);
	luaL_setfuncs(L, Font_functions, 0);
	lua_setglobal(L, "Font");
	lua_newtable(L);
}