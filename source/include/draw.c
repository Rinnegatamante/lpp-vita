/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/kernel/sysmem.h>
#include "draw.h"
#include "utils.h"

extern const unsigned char msx_font[];

static SceDisplayFrameBuf fb[2];
static SceUID fb_memuid[2];
static int cur_fb = 0;

static void *alloc_gpu_mem(uint32_t type, uint32_t size, uint32_t attribs, SceUID *uid)
{
	int ret;
	void *mem = NULL;

	if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW)
		size = align_mem(size, 256 * 1024);
	else
		size = align_mem(size, 4 * 1024);

	*uid = sceKernelAllocMemBlock("gxm", type, size, NULL);

	printf("MemBlock uid: 0x%08X\n", *uid);

	ret = sceKernelGetMemBlockBase(*uid, &mem);
	printf("sceKernelGetMemBlockBase(): 0x%08X\n", ret);
	printf("MemBlockBase addr: %p\n", mem);
	if (ret != 0) {
		return NULL;
	}

	ret = sceGxmMapMemory(mem, size, attribs);
	printf("sceGxmMapMemory(): 0x%08X\n", ret);
	if (ret != 0) {
		return NULL;
	}

	return mem;
}

void init_video()
{
	int ret;

	SceGxmInitializeParams params;

	params.flags                        = 0x0;
	params.displayQueueMaxPendingCount  = 0x2; //Double buffering
	params.displayQueueCallback         = 0x0;
	params.displayQueueCallbackDataSize = 0x0;
	params.parameterBufferSize          = (16 * 1024 * 1024);

	/* Initialize the GXM */
	ret = sceGxmInitialize(&params);
	printf("sceGxmInitialize(): 0x%08X\n", ret);

	/* Setup framebuffers */
	fb[0].size        = sizeof(fb[0]);
	fb[0].pitch       = SCREEN_W;
	fb[0].pixelformat = PSP2_DISPLAY_PIXELFORMAT_A8B8G8R8;
	fb[0].width       = SCREEN_W;
	fb[0].height      = SCREEN_H;

	fb[1].size        = sizeof(fb[1]);
	fb[1].pitch       = SCREEN_W;
	fb[1].pixelformat = PSP2_DISPLAY_PIXELFORMAT_A8B8G8R8;
	fb[1].width       = SCREEN_W;
	fb[1].height      = SCREEN_H;

	/* Allocate memory for the framebuffers */
	fb[0].base = alloc_gpu_mem(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
		SCREEN_W * SCREEN_H * 4, SCE_GXM_MEMORY_ATTRIB_RW, &fb_memuid[0]);

	if (fb[0].base == NULL) {
		printf("Could not allocate memory for fb[0]. %p", fb[0].base);
		return;
	}

	fb[1].base = alloc_gpu_mem(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
		SCREEN_W * SCREEN_H * 4, SCE_GXM_MEMORY_ATTRIB_RW, &fb_memuid[1]);

	if (fb[1].base == NULL) {
		printf("Could not allocate memory for fb[1]. %p", fb[1].base);
		return;
	}

	/* Display the framebuffer 0 */
	cur_fb = 0;
	swap_buffers();

	printf(
		"\nframebuffer 0:\n"
		"\tsize:           0x%08X\n"
		"\tbase:           0x%08X\n"
		"\tpitch:          0x%08X\n"
		"\tpixelformat:    0x%08X\n"
		"\twidth:          0x%08X\n"
		"\theight          0x%08X\n",
		fb[0].size, (uintptr_t)fb[0].base,
		fb[0].pitch, fb[0].pixelformat, fb[0].width, fb[0].height);

	printf(
		"\nframebuffer 1:\n"
		"\tsize:           0x%08X\n"
		"\tbase:           0x%08X\n"
		"\tpitch:          0x%08X\n"
		"\tpixelformat:    0x%08X\n"
		"\twidth:          0x%08X\n"
		"\theight          0x%08X\n",
		fb[1].size, (uintptr_t)fb[1].base,
		fb[1].pitch, fb[1].pixelformat, fb[1].width, fb[1].height);
}

void end_video()
{
	sceGxmUnmapMemory(fb[0].base);
	sceGxmUnmapMemory(fb[1].base);
	sceGxmTerminate();
}

void swap_buffers()
{
	sceDisplaySetFrameBuf(&fb[cur_fb], PSP2_DISPLAY_SETBUF_NEXTFRAME);
	cur_fb ^= 1;
}

void clear_screen()
{
	memset(fb[cur_fb].base, 0x00, SCREEN_W*SCREEN_H*4);
}

void draw_pixel(uint32_t x, uint32_t y, uint32_t color)
{
	((uint32_t *)fb[cur_fb].base)[x + y*fb[cur_fb].pitch] = color;
}

void draw_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
	int i, j;
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			((uint32_t *)fb[cur_fb].base)[(x + j) + (y + i)*fb[cur_fb].pitch] = color;
		}
	}
}

void draw_circle(uint32_t x, uint32_t y, uint32_t radius, uint32_t color)
{
	int r2 = radius * radius;
	int area = r2 << 2;
	int rr = radius << 1;

	int i;
	for (i = 0; i < area; i++) {
		int tx = (i % rr) - radius;
		int ty = (i / rr) - radius;

		if (tx * tx + ty * ty <= r2) {
			draw_pixel(x + tx, y + ty, color);
		}
	}
}

void font_draw_char(int x, int y, uint32_t color, char c)
{
	unsigned char *font = (unsigned char *)(msx_font + (c - (uint32_t)' ') * 8);
	int i, j, pos_x, pos_y;
	for (i = 0; i < 8; ++i) {
		pos_y = y + i*2;
		for (j = 0; j < 8; ++j) {
			pos_x = x + j*2;
			if ((*font & (128 >> j))) {
				draw_pixel(pos_x + 0, pos_y + 0, color);
				draw_pixel(pos_x + 1, pos_y + 0, color);
				draw_pixel(pos_x + 0, pos_y + 1, color);
				draw_pixel(pos_x + 1, pos_y + 1, color);
			}
		}
		++font;
	}
}

void font_draw_string(int x, int y, uint32_t color, const char *string)
{
	if (string == NULL) return;

	int startx = x;
	const char *s = string;

	while (*s) {
		if (*s == '\n') {
			x = startx;
			y += 16;
		} else if (*s == ' ') {
			x += 16;
		} else if(*s == '\t') {
			x += 16*4;
		} else {
			font_draw_char(x, y, color, *s);
			x += 16;
		}
		++s;
	}
}

void font_draw_stringf(int x, int y, uint32_t color, const char *s, ...)
{
	char buf[256];
	va_list argptr;
	va_start(argptr, s);
	vsnprintf(buf, sizeof(buf), s, argptr);
	va_end(argptr);
	font_draw_string(x, y, color, buf);
}

