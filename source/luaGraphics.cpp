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

#include <string.h>
#include <vitasdk.h>
#include <vita2d.h>
#include <utils.h>
#include "include/luaplayer.h"

extern "C"{
#include <png.h>
#include <libimagequant.h>
#include "include/gifdec.h"
};

#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

vita2d_pgf* debug_font;
struct ttf {
	uint32_t magic;
	vita2d_font* f;
	vita2d_pgf* f2;
	vita2d_pvf* f3;
	int size;
	float scale;
};

struct rescaler {
	vita2d_texture* fbo;
	int x;
	int y;
	float x_scale;
	float y_scale;
};

struct animated_texture {
	void *frames;
	uint32_t num_frames;
};

static bool isRescaling = false;
static rescaler scaler;

static char asyncImagePath[512];

extern int FORMAT_BMP;
extern int FORMAT_PNG;
extern int FORMAT_JPG;

#ifdef PARANOID
static bool draw_state = false;
#endif

static int imgThread(unsigned int args, void* arg)
{
	char* text = asyncImagePath;
	SceUID file = sceIoOpen(text, SCE_O_RDONLY, 0777);
	uint16_t magic;
	sceIoRead(file, &magic, 2);
	sceIoClose(file);
	vita2d_texture* result;
	if (magic == 0x4D42) result = vita2d_load_BMP_file(text);
	else if (magic == 0xD8FF) result = vita2d_load_JPEG_file(text);
	else if (magic == 0x5089) result = vita2d_load_PNG_file(text);
	else 
	{
		async_task_num--;
		asyncResult = 1;
		sceKernelExitDeleteThread(0);
		return 0;
	}
	lpp_texture* ret = (lpp_texture*)malloc(sizeof(lpp_texture));
	ret->magic = 0xABADBEEF;
	ret->text = result;
	ret->data = NULL;
	char* buffer = (char*)malloc(16);
	memset(buffer, 0, 16);
	sprintf(buffer, "%i", ret);
	asyncStrRes = (unsigned char*)buffer;
	asyncResSize = strlen(buffer);
	async_task_num--;
	asyncResult = 1;
	sceKernelExitDeleteThread(0);
	return 0;
}

static int lua_print(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 4 && argc != 5) return luaL_error(L, "wrong number of arguments.");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "debugPrint can't be called outside a blending phase.");
#endif
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	char* text = (char*)luaL_checkstring(L, 3);
	int color = luaL_checkinteger(L, 4);
	float scale = 1.0;
	if (argc == 5) scale = luaL_checknumber(L, 5);
	vita2d_pgf_draw_text(debug_font, x, y + 17.402 * scale, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF), scale, text);
	return 0;
}

static int lua_pixel(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3 && argc != 4) return luaL_error(L, "wrong number of arguments.");
#endif
#ifdef PARANOID
	if (!draw_state && argc == 3) return luaL_error(L, "drawPixel can't be called outside a blending phase for on screen drawing.");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	uint32_t color = luaL_checkinteger(L, 3);
	if (argc == 3) vita2d_draw_rectangle(x, y, 1, 1, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	else{
		lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 4));
	#ifndef SKIP_ERROR_HANDLING
		if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
	#endif
		int intx = x;
		int inty = y;
		uint32_t* data = (uint32_t*)vita2d_texture_get_datap(text->text);
		uint32_t pitch = vita2d_texture_get_stride(text->text)>>2;
		data[intx+inty*pitch] = color;
	}
	return 0;
}

static int lua_rect(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 5) return luaL_error(L, "wrong number of arguments.");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "drawRect can't be called outside a blending phase.");
#endif
	float x1 = luaL_checknumber(L, 1);
	float x2 = luaL_checknumber(L, 2);
	float y1 = luaL_checknumber(L, 3);
	float y2 = luaL_checknumber(L, 4);
	int color = luaL_checkinteger(L,5);
	if (x2 < x1) {
		int tmp = x2;
		x2 = x1;
		x1 = tmp;
	}
	if (y2 < y1) {
		int tmp = y2;
		y2 = y1;
		y1 = tmp;
	}
	vita2d_draw_rectangle(x1, y1, x2-x1, y2-y1, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}

static int lua_emptyrect(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 5) return luaL_error(L, "wrong number of arguments.");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "drawEmptyRect can't be called outside a blending phase.");
#endif
	float x1 = luaL_checknumber(L, 1);
	float x2 = luaL_checknumber(L, 2);
	float y1 = luaL_checknumber(L, 3);
	float y2 = luaL_checknumber(L, 4);
	int color = luaL_checkinteger(L,5);
	if (x2 < x1) {
		int tmp = x2;
		x2 = x1;
		x1 = tmp;
	}
	if (y2 < y1) {
		int tmp = y2;
		y2 = y1;
		y1 = tmp;
	}
	vita2d_draw_line(x1, y1, x2, y1, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	vita2d_draw_line(x1, y2, x2, y2, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	vita2d_draw_line(x1, y1, x1, y2, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	vita2d_draw_line(x2, y1, x2, y2, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}

static int lua_line(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 5) return luaL_error(L, "wrong number of arguments.");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "drawLine can't be called outside a blending phase.");
#endif
	float x1 = luaL_checknumber(L, 1);
	float x2 = luaL_checknumber(L, 2);
	float y1 = luaL_checknumber(L, 3);
	float y2 = luaL_checknumber(L, 4);
	int color = luaL_checkinteger(L,5);
	vita2d_draw_line(x1, y1, x2, y2, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}

static int lua_circle(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 4) return luaL_error(L, "wrong number of arguments.");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "drawCircle can't be called outside a blending phase.");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	float radius = luaL_checknumber(L, 3);
	int color = luaL_checkinteger(L,4);
	vita2d_draw_fill_circle(x, y, radius, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}

static int lua_init(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
#ifdef PARANOID
	if (draw_state) return luaL_error(L, "initBlend can't be called inside a blending phase.");
	else draw_state = true;
#endif
	if (isRescaling) {
		vita2d_pool_reset();
		vita2d_start_drawing_advanced(scaler.fbo, SCE_GXM_SCENE_FRAGMENT_SET_DEPENDENCY);
	}else vita2d_start_drawing();
	return 0;
}

static int lua_term(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "termBlend can't be called outside a blending phase.");
	else draw_state = false;
#endif
	vita2d_end_drawing();
	if (isRescaling) {
		vita2d_start_drawing_advanced(NULL, SCE_GXM_SCENE_VERTEX_WAIT_FOR_DEPENDENCY);
		vita2d_draw_texture_scale(scaler.fbo,scaler.x,scaler.y,scaler.x_scale,scaler.y_scale);
		vita2d_end_drawing();
	}
	if (keyboardStarted || messageStarted) vita2d_common_dialog_update();
	vita2d_wait_rendering_done();
	return 0;
}

static int lua_loadimg(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char* text = (char*)(luaL_checkstring(L, 1));
	SceUID file = sceIoOpen(text, SCE_O_RDONLY, 0777);
	uint16_t magic;
	sceIoRead(file, &magic, 2);
	sceIoClose(file);
	vita2d_texture *result = NULL;
	if (magic == 0x4D42) result = vita2d_load_BMP_file(text);
	else if (magic == 0xD8FF) result = vita2d_load_JPEG_file(text);
	else if (magic == 0x5089) result = vita2d_load_PNG_file(text);
	else return luaL_error(L, "Error loading image (invalid magic).");
#ifndef SKIP_ERROR_HANDLING
	if (result == NULL) return luaL_error(L, "Error loading image.");
#endif
	lpp_texture *ret = (lpp_texture*)malloc(sizeof(lpp_texture));
	ret->magic = 0xABADBEEF;
	ret->text = result;
	ret->data = NULL;
	
	lua_pushinteger(L, (uint32_t)(ret));
	return 1;
}

static int lua_saveimg(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2 && argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 1));
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	const char *filename = luaL_checkstring(L, 2);
	int format = (argc > 2) ? luaL_checkinteger(L, 3) : FORMAT_BMP;
	int w = vita2d_texture_get_width(text->text);
	int h = vita2d_texture_get_height(text->text);
	SceGxmTextureFormat fmt = vita2d_texture_get_format(text->text);
	int bpp;
	switch (fmt) {
	case SCE_GXM_TEXTURE_FORMAT_U8U8U8_BGR:
		bpp = 3;
		break;
	default:
		bpp = 4;
		break;
	}
	int pitch = vita2d_texture_get_stride(text->text) / bpp;
	if (format == FORMAT_BMP) {
		SceUID fd = sceIoOpen(filename, SCE_O_CREAT|SCE_O_WRONLY|SCE_O_TRUNC, 0777);
		uint8_t *bmp_content = (uint8_t*)malloc((w * h * 4)+0x36);
		sceClibMemset(bmp_content, 0, 0x36);
		*(uint16_t*)&bmp_content[0x0] = 0x4D42;
		*(uint32_t*)&bmp_content[0x2] = (w * h * 4)+0x36;
		*(uint32_t*)&bmp_content[0xA] = 0x36;
		*(uint32_t*)&bmp_content[0xE] = 0x28;
		*(uint32_t*)&bmp_content[0x12] = w;
		*(uint32_t*)&bmp_content[0x16] = h;
		*(uint32_t*)&bmp_content[0x1A] = 0x00200001;
		*(uint32_t*)&bmp_content[0x22] = w * h * 4;
		int x, y;
		uint8_t* buffer = (uint8_t*)&bmp_content[0x36];
		uint8_t* framebuf = (uint8_t*)vita2d_texture_get_datap(text->text);
		for (y = 0; y < h; y++){
			for (x = 0; x < w; x++){
				if (bpp == 3) {
					buffer[(x + y * w) * 4] = framebuf[(x+(h-y)*pitch)*bpp + 2];
					buffer[(x + y * w) * 4 + 1] = framebuf[(x+(h-y)*pitch)*bpp + 1];
					buffer[(x + y * w) * 4 + 2] = framebuf[(x+(h-y)*pitch)*bpp];
					buffer[(x + y * w) * 4 + 3] = 0xFF;
				} else {
					buffer[(x + y * w) * 4] = framebuf[(x+(h-y)*pitch)*bpp + 2];
					buffer[(x + y * w) * 4 + 1] = framebuf[(x+(h-y)*pitch)*bpp + 1];
					buffer[(x + y * w) * 4 + 2] = framebuf[(x+(h-y)*pitch)*bpp];
					buffer[(x + y * w) * 4 + 3] = framebuf[(x+(h-y)*pitch)*bpp + 3];
				}
			}
		}
		sceIoWrite(fd, bmp_content, (w * h * 4)+0x36);
		free(bmp_content);
		sceIoClose(fd);
	} else if (format == FORMAT_PNG) {
		FILE *fh = fopen(filename, "wb");
		uint8_t *raw_data = (uint8_t*)malloc(w * h * 4);
		uint8_t* buffer = (uint8_t*)raw_data;
		uint8_t* framebuf = (uint8_t*)vita2d_texture_get_datap(text->text);
		int x, y;
		for (y = 0; y < h; y++){
			for (x = 0; x < w; x++){
				if (bpp == 3) {
					buffer[(x + y * w) * 4] = framebuf[(x+y*pitch)*bpp];
					buffer[(x + y * w) * 4 + 1] = framebuf[(x+y*pitch)*bpp + 1];
					buffer[(x + y * w) * 4 + 2] = framebuf[(x+y*pitch)*bpp + 2];
					buffer[(x + y * w) * 4 + 3] = 0xFF;
				} else {
					buffer[(x + y * w) * 4] = framebuf[(x+y*pitch)*bpp];
					buffer[(x + y * w) * 4 + 1] = framebuf[(x+y*pitch)*bpp + 1];
					buffer[(x + y * w) * 4 + 2] = framebuf[(x+y*pitch)*bpp + 2];
					buffer[(x + y * w) * 4 + 3] = framebuf[(x+y*pitch)*bpp + 3];
				}
			}
		}
		liq_attr *handle = liq_attr_create();
		liq_image *input_image = liq_image_create_rgba(handle, raw_data, w, h, 0);
		liq_result *res;
		liq_image_quantize(input_image, handle, &res);
		uint8_t *quant_raw = (uint8_t*)malloc(w * h);
		liq_set_dithering_level(res, 1.0);
		liq_write_remapped_image(res, input_image, quant_raw, w * h);
		const liq_palette *palette = liq_get_palette(res);
		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);;
		png_infop info_ptr = png_create_info_struct(png_ptr);
		setjmp(png_jmpbuf(png_ptr));
		png_init_io(png_ptr, fh);
		png_set_IHDR(png_ptr, info_ptr, w, h,
			8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		png_color *pal = (png_color*)png_malloc(png_ptr, palette->count*sizeof(png_color));
		for (int i = 0; i < palette->count; i++){
			png_color *col = &pal[i];
			col->red = palette->entries[i].r;
			col->green = palette->entries[i].g;
			col->blue = palette->entries[i].b;
		}
		png_set_PLTE(png_ptr, info_ptr, pal, palette->count);
		png_write_info(png_ptr, info_ptr);
		for (y = 0; y < h; y++) {
			png_write_row(png_ptr, &quant_raw[y * w]);
		}
		png_write_end(png_ptr, NULL);
		fclose(fh);
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		free(quant_raw);
		free(raw_data);
		liq_result_destroy(res);
		liq_image_destroy(input_image);
		liq_attr_destroy(handle);
	}
	return 0;
}

static int lua_loadimgasync(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char* text = (char*)(luaL_checkstring(L, 1));
	sprintf(asyncImagePath, text);
	async_task_num++;
	SceUID thd = sceKernelCreateThread("Image loader Thread", &imgThread, 0x10000100, 0x100000, 0, 0, NULL);
	if (thd < 0)
	{
		asyncResult = -1;
		return 0;
	}
	asyncResult = 0;
	sceKernelStartThread(thd, 0, NULL);
	return 0;
}

static int lua_loadanimg(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char* text = (char*)(luaL_checkstring(L, 1));
	SceUID file = sceIoOpen(text, SCE_O_RDONLY, 0777);
	uint16_t magic;
	sceIoRead(file, &magic, 2);
	sceIoClose(file);
	vita2d_texture *result = NULL;
	uint32_t *frames = NULL;
	uint32_t num_frames = 0;
	if (magic == 0x4947) {
		gd_GIF *f = gd_open_gif(text);
		result = (vita2d_texture*)malloc(sizeof(vita2d_texture));
		result->palette_UID = 0;
		while (gd_get_frame(f)) {
			num_frames++;
		}
		gd_rewind(f);
		frames = (uint32_t *)malloc(f->width * f->height * 4 * num_frames);
		uint32_t *frame = frames;
		uint8_t *rgb_data = (uint8_t *)malloc(f->width * f->height * 3);
		while (gd_get_frame(f)) {
			uint8_t *pixel_data = rgb_data;
			gd_render_frame(f, rgb_data);
			for (int i = 0; i < f->width * f->height; i++) {
				frame[i] = pixel_data[0] | pixel_data[1] << 8 | pixel_data[2] << 16 | (gd_is_bgcolor(f, pixel_data) ? 0x00 : 0xFF) << 24;
				pixel_data += 3;
			}
			frame += f->width * f->height;
			sceGxmTextureInitLinear(&result->gxm_tex, frames, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8, f->width, f->height, 0);
		}
		gd_close_gif(f);
		free(rgb_data);
	} else return luaL_error(L, "Error loading image (invalid magic).");
#ifndef SKIP_ERROR_HANDLING
	if (result == NULL) return luaL_error(L, "Error loading image.");
#endif
	lpp_texture *ret = (lpp_texture*)malloc(sizeof(lpp_texture));
	ret->magic = 0xABADBEEF;
	ret->text = result;
	
	animated_texture *anim_data = (animated_texture*)malloc(sizeof(animated_texture));
	anim_data->frames = frames;
	anim_data->num_frames = num_frames;
	ret->data = anim_data;
	
	lua_pushinteger(L, (uint32_t)(ret));
	return 1;
}

static int lua_getnumframes(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 1));
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
	if (text->data == NULL) luaL_error(L, "attempt to access a static image.");
#endif
	animated_texture *d = (animated_texture *)text->data;
	lua_pushinteger(L, d->num_frames);
	return 1;
}

static int lua_setframe(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 1));
	int idx = luaL_checkinteger(L, 2);
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
	if (text->data == NULL) luaL_error(L, "attempt to access a static image.");
#endif
	animated_texture *d = (animated_texture *)text->data;
	uint32_t *frames = (uint32_t *)d->frames;
	sceClibPrintf("frames %X\n", frames);
	if (idx >= d->num_frames) idx = d->num_frames - 1;
	int w = vita2d_texture_get_width(text->text);
	int h = vita2d_texture_get_height(text->text);
	sceGxmTextureInitLinear(&text->text->gxm_tex, &frames[w * h * idx], SCE_GXM_TEXTURE_FORMAT_A8B8G8R8, w, h, 0);
	return 0;
}

static int lua_drawimg(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3 && argc != 4) return luaL_error(L, "wrong number of arguments");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "drawImage can't be called outside a blending phase.");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 3));
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	if (argc == 4) {
		uint32_t color = luaL_checkinteger(L, 4);
		vita2d_draw_texture_tint(text->text, x, y, color);
	}else vita2d_draw_texture(text->text, x, y);
	return 0;
}

static int lua_drawimg_rotate(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 4 && argc != 5) return luaL_error(L, "wrong number of arguments");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "drawRotateImage can't be called outside a blending phase.");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 3));
	float radius = luaL_checknumber(L, 4);
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	if (argc == 5) {
		uint32_t color = luaL_checkinteger(L, 5);
		vita2d_draw_texture_tint_rotate(text->text, x, y, radius, color);
	}else vita2d_draw_texture_rotate(text->text, x, y, radius);
	return 0;
}

static int lua_drawimg_scale(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "drawScaleImage can't be called outside a blending phase.");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 3));
	float x_scale = luaL_checknumber(L, 4);
	float y_scale = luaL_checknumber(L, 5);
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	if (argc == 6) {
		uint32_t color = luaL_checkinteger(L, 6);
		vita2d_draw_texture_tint_scale(text->text, x, y, x_scale, y_scale, color);
	}else vita2d_draw_texture_scale(text->text, x, y, x_scale, y_scale);
	return 0;
}

static int lua_drawimg_part(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 7 && argc != 8) return luaL_error(L, "wrong number of arguments");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "drawPartialImage can't be called outside a blending phase.");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 3));
	int st_x = luaL_checkinteger(L, 4);
	int st_y = luaL_checkinteger(L, 5);
	float width = luaL_checknumber(L, 6);
	float height = luaL_checknumber(L, 7);
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	if (argc == 8) {
		uint32_t color = luaL_checkinteger(L, 8);
		vita2d_draw_texture_tint_part(text->text, x, y, st_x, st_y, width, height, color);
	}else vita2d_draw_texture_part(text->text, x, y, st_x, st_y, width, height);
	return 0;
}

static int lua_drawimg_full(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 11 && argc != 10) return luaL_error(L, "wrong number of arguments");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "drawImageExtended can't be called outside a blending phase.");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 3));
	int st_x = luaL_checkinteger(L, 4);
	int st_y = luaL_checkinteger(L, 5);
	float width = luaL_checknumber(L, 6);
	float height = luaL_checknumber(L, 7);
	float radius = luaL_checknumber(L, 8);
	float x_scale = luaL_checknumber(L, 9);
	float y_scale = luaL_checknumber(L, 10);
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	if (argc == 11) {
		uint32_t color = luaL_checkinteger(L, 11);
		vita2d_draw_texture_part_tint_scale_rotate(text->text, x, y, st_x, st_y, width, height, x_scale, y_scale, radius, color);
	}else vita2d_draw_texture_part_scale_rotate(text->text, x, y, st_x, st_y, width, height, x_scale, y_scale, radius);
	return 0;
}

static int lua_width(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 1));
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	lua_pushinteger(L, vita2d_texture_get_width(text->text));
	return 1;
}

static int lua_height(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 1));
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	lua_pushinteger(L, vita2d_texture_get_height(text->text));
	return 1;
}

static int lua_free(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 1));
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	vita2d_free_texture(text->text);
	if (text->data) {
		animated_texture *d = (animated_texture *)text->data;
		free(d->frames);
		free(text->data);
	}
	free(text);
	return 0;
}

static int lua_createimage(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2 && argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	int w = luaL_checkinteger(L, 1);
	int h = luaL_checkinteger(L, 2);
	uint32_t color = 0xFFFFFFFF;
	if (argc == 3) color = luaL_checkinteger(L, 3);
	lpp_texture* text = (lpp_texture*)malloc(sizeof(lpp_texture));
	text->magic = 0xABADBEEF;
	text->text = vita2d_create_empty_texture_rendertarget(w, h, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
	memset(vita2d_texture_get_datap(text->text), color, vita2d_texture_get_stride(text->text) * h);
	lua_pushinteger(L, (uint32_t)text);
	return 1;
}

static int lua_filters(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 1));
	SceGxmTextureFilter min_filter = (SceGxmTextureFilter)(luaL_checkinteger(L, 2));
	SceGxmTextureFilter mag_filter = (SceGxmTextureFilter)(luaL_checkinteger(L, 3));
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	vita2d_texture_set_filters(text->text, min_filter, mag_filter);
	return 0;
}

static int lua_loadFont(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char* text = (char*)(luaL_checkstring(L, 1));
	ttf* result = (ttf*)malloc(sizeof(ttf));
	memset(result, 0, sizeof(ttf));
	result->size = 16;
	result->scale = 0.919f;
	result->f = vita2d_load_font_file(text); // TTF font
	if (result->f == NULL) { 
		result->f2 = vita2d_load_custom_pgf(text);
		if (result->f2 == NULL) { 
			result->f3 = vita2d_load_custom_pvf(text);
		#ifndef SKIP_ERROR_HANDLING
			if (result->f3 == NULL) {
				free(result);
				return luaL_error(L, "cannot load font file");
			}
		#endif
		}
	}
	result->magic = 0x4C464E54;
	lua_pushinteger(L,(uint32_t)result);
	return 1;
}

static int lua_fsize(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	ttf* font = (ttf*)(luaL_checkinteger(L, 1));
	int size = luaL_checkinteger(L,2);
#ifndef SKIP_ERROR_HANDLING
	if (font->magic != 0x4C464E54) return luaL_error(L, "attempt to access wrong memory block type");
#endif
	font->size = size;
	font->scale = size / 17.402f;
	return 0;
}

static int lua_unloadFont(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	ttf* font = (ttf*)(luaL_checkinteger(L, 1));
#ifndef SKIP_ERROR_HANDLING
	if (font->magic != 0x4C464E54) return luaL_error(L, "attempt to access wrong memory block type");
#endif
	if (font->f != NULL) vita2d_free_font(font->f);
	else if (font->f2 != NULL) vita2d_free_pgf(font->f2);
	else vita2d_free_pvf(font->f3);
	free(font);
	return 0;
}

static int lua_fprint(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 5) return luaL_error(L, "wrong number of arguments");
#endif
#ifdef PARANOID
	if (!draw_state) return luaL_error(L, "print can't be called outside a blending phase.");
#endif
	ttf *font = (ttf *)(luaL_checkinteger(L, 1));
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	char *text = (char *)(luaL_checkstring(L, 4));
	uint32_t color = luaL_checkinteger(L,5);
#ifndef SKIP_ERROR_HANDLING
	if (font->magic != 0x4C464E54) return luaL_error(L, "attempt to access wrong memory block type");
#endif
	if (font->f != NULL)
		vita2d_font_draw_text(font->f, x, y + font->size, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF), font->size, text);
	else if (font->f2 != NULL)
		vita2d_pgf_draw_text(font->f2, x, y + 17.402 * font->scale, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF), font->scale, text);
	else
		vita2d_pvf_draw_text(font->f3, x, y + 17.402 * font->scale, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF), font->scale, text);
	return 0;
}

static int lua_fwidth(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	ttf *font = (ttf *)(luaL_checkinteger(L, 1));
	char *text = (char *)(luaL_checkstring(L, 2));
#ifndef SKIP_ERROR_HANDLING
	if (font->magic != 0x4C464E54) return luaL_error(L, "attempt to access wrong memory block type");
#endif
	if (font->f != NULL)
		lua_pushinteger(L, vita2d_font_text_width(font->f, font->size, text));
	else if (font->f2 != NULL)
		lua_pushinteger(L, vita2d_pgf_text_width(font->f2, font->scale, text));
	else
		lua_pushinteger(L, vita2d_pvf_text_width(font->f3, font->scale, text));
	return 1;
}

static int lua_fheight(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	ttf *font = (ttf*)(luaL_checkinteger(L, 1));
	char *text = (char*)(luaL_checkstring(L, 2));
#ifndef SKIP_ERROR_HANDLING
	if (font->magic != 0x4C464E54) return luaL_error(L, "attempt to access wrong memory block type");
#endif
	if (font->f != NULL)
		lua_pushinteger(L, vita2d_font_text_height(font->f, font->size, text));
	else if (font->f2 != NULL)
		lua_pushinteger(L, vita2d_pgf_text_height(font->f2, font->scale, text));
	else
		lua_pushinteger(L, vita2d_pvf_text_height(font->f3, font->scale, text));
	return 1;
}

static int lua_rescaleron(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 4) return luaL_error(L, "wrong number of arguments");
	if (isRescaling) return luaL_error(L, "cannot start two different rescalers together");
#endif
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	float x_scale = luaL_checknumber(L, 3);
	float y_scale = luaL_checknumber(L, 4);
	scaler.x = x;
	scaler.y = y;
	scaler.x_scale = x_scale;
	scaler.y_scale = y_scale;
	scaler.fbo = vita2d_create_empty_texture_format(960,544,SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
	isRescaling = true;
	return 0;
}

static int lua_rescaleroff(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!isRescaling) return luaL_error(L, "no rescaler available");
#endif
	vita2d_free_texture(scaler.fbo);
	isRescaling = false;
	return 0;
}

static int lua_gpixel(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 3));
#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	uint32_t *buff = (uint32_t*)vita2d_texture_get_datap(text->text);
	lua_pushinteger(L, buff[ALIGN(vita2d_texture_get_width(text->text), 8) * y + x]);
	return 1;
}

//Register our Graphics Functions
static const luaL_Reg Graphics_functions[] = {
  {"initBlend",           lua_init},
  {"termBlend",           lua_term},
  {"debugPrint",          lua_print},
  {"drawPixel",           lua_pixel},
  {"getPixel",            lua_gpixel},
  {"drawLine",            lua_line},
  {"fillRect",            lua_rect},
  {"fillEmptyRect",       lua_emptyrect},
  {"fillCircle",          lua_circle},
  {"loadImage",           lua_loadimg},
  {"saveImage",           lua_saveimg},
  {"loadImageAsync",      lua_loadimgasync},
  {"loadAnimatedImage",   lua_loadanimg},
  {"getImageFramesNum",   lua_getnumframes},
  {"setImageFrame",       lua_setframe},
  {"drawImage",           lua_drawimg},
  {"drawRotateImage",     lua_drawimg_rotate},
  {"drawScaleImage",      lua_drawimg_scale},
  {"drawPartialImage",    lua_drawimg_part},
  {"drawImageExtended",   lua_drawimg_full},
  {"createImage",         lua_createimage},
  {"setImageFilters",     lua_filters},
  {"getImageWidth",       lua_width},
  {"getImageHeight",      lua_height},
  {"freeImage",           lua_free},
  {"initRescaler",        lua_rescaleron},
  {"termRescaler",        lua_rescaleroff},
  {0, 0}
};

//Register our Font Functions
static const luaL_Reg Font_functions[] = {
  {"load",            lua_loadFont}, 
  {"print",           lua_fprint},
  {"getTextWidth",    lua_fwidth},
  {"getTextHeight",   lua_fheight}, 
  {"setPixelSizes",   lua_fsize}, 
  {"unload",          lua_unloadFont}, 
  {0, 0}
};

void luaGraphics_init(lua_State *L) {
	uint32_t FILTER_POINT = (uint32_t)SCE_GXM_TEXTURE_FILTER_POINT;
	uint32_t FILTER_LINEAR = (uint32_t)SCE_GXM_TEXTURE_FILTER_LINEAR;
	VariableRegister(L,FILTER_POINT);
	VariableRegister(L,FILTER_LINEAR);
	lua_newtable(L);
	luaL_setfuncs(L, Graphics_functions, 0);
	lua_setglobal(L, "Graphics");
	lua_newtable(L);
	luaL_setfuncs(L, Font_functions, 0);
	lua_setglobal(L, "Font");
	lua_newtable(L);
}