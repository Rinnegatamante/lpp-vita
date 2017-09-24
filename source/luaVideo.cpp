/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____	_____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |	  | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|	  |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |	  | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|	  |_| \____|(___/   ------------------------------------------------------------------#
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
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <vitasdk.h>
extern "C"{
	#include <utils.h> // utils.h file from vita2d
}
#include "include/luaplayer.h"
#include "include/videodec.h"

#define DECODER_BUFSIZE        0x5FA00
#define FRAMEBUFFER_SIZE       0x200000
#define FRAMEBUFFER_ALIGNMENT  0x40000
#define VIDEO_BUFFERING        5

SceUID decoderThread, decoderBlock;
SceAvcdecCtrl decoder = {0};
static uint8_t* auBuf;
static bool isPlayerReady = false;
static bool decoderRunning = false;
static bool looping = false;
controller videoDecoder;
void* videobuf[VIDEO_BUFFERING];
uint8_t videobuf_idx = 0;
float deltaTick = 0.0f;
uint8_t busy_buffers = 0;
uint8_t cur_videobuf_idx = 0;
bool isPlaying = false;

struct texture{
	uint32_t magic;
	vita2d_texture* text;
};
texture out_text[VIDEO_BUFFERING];
texture* cur_text = NULL;

static int decoderThreadFunc(unsigned int args, void* arg){
	int ret = 0;
	uint32_t size;
	uint8_t* dst;
	SceAvcdecArrayPicture array_picture = {0};
	SceAvcdecPicture picture = {0};
	SceAvcdecPicture *pictures = { &picture };
	SceAvcdecAu au = {0};
	picture.size = sizeof(SceAvcdecPicture);
	decoderRunning = true;
	float tick = 0.0f;
	float cur_tick = 0.0f;
	float pause_delta_tick = 0.0f;
	bool _isPlaying = true;
	
	while (decoderRunning){
		
		if (!isPlaying){
			if (_isPlaying){
				_isPlaying = false;
				pause_delta_tick = (sceKernelGetProcessTimeWide() / 1000000.0f);
			}
			continue;
		}else{
			if (!_isPlaying){
				_isPlaying = true;
				tick += ((sceKernelGetProcessTimeWide() / 1000000.0f) - pause_delta_tick);
			}
		}
		
		if ((busy_buffers > (VIDEO_BUFFERING - 1)) && (((sceKernelGetProcessTimeWide() / 1000000.0f) - tick) < (deltaTick / 2))) continue;
		
		ret = ctrlReadFrame(&videoDecoder, auBuf, &size);
		if (ret > 0){
			
			picture.frame.pixelType = 0;
			picture.frame.framePitch = 960;
			picture.frame.frameWidth = 960;
			picture.frame.frameHeight = 544;
			picture.frame.pPicture[0] = videobuf[videobuf_idx];
			
			au.es.pBuf = auBuf;
			au.es.size = size;
			au.dts.lower = 0xFFFFFFFF;
			au.dts.upper = 0xFFFFFFFF;
			au.pts.lower = 0xFFFFFFFF;
			au.pts.upper = 0xFFFFFFFF;
			
			array_picture.numOfElm = 1;
			array_picture.pPicture = &pictures;
			
			sceAvcdecDecode(&decoder, &au, &array_picture);
			
			if (array_picture.numOfOutput == 1){
				
				busy_buffers++;
				videobuf_idx = (videobuf_idx + 1) % VIDEO_BUFFERING;
				cur_tick = sceKernelGetProcessTimeWide() / 1000000.0f;
				
				if (cur_tick - tick > deltaTick){
					cur_text = &out_text[cur_videobuf_idx];
					cur_videobuf_idx = videobuf_idx;
					busy_buffers--;
					tick = cur_tick;
				}
				
			}
			
		}else{
			if (looping) ctrlRewind(&videoDecoder);
			else decoderRunning = false;
		}

		sceKernelDelayThread(100);
	}

	return sceKernelExitDeleteThread(0);

}

static int lua_init(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (isPlayerReady) return 0;
	#endif
	
	auBuf = (uint8_t*)memalign(0x100, DECODER_BUFSIZE);
	
	SceVideodecQueryInitInfoHwAvcdec init = {0};
	init.size = sizeof(init);
	init.horizontal = 960;
	init.vertical = 544;
	init.numOfRefFrames = 3;
	init.numOfStreams = 1;
	
	SceAvcdecQueryDecoderInfo	decoder_info = {0};
	decoder_info.horizontal = init.horizontal;
	decoder_info.vertical = init.vertical;
	decoder_info.numOfRefFrames = init.numOfRefFrames;
	
	SceAvcdecDecoderInfo decoder_info_out = {0};
	
	sceVideodecInitLibrary(SCE_VIDEODEC_TYPE_HW_AVCDEC, &init);
	sceAvcdecQueryDecoderMemSize(SCE_VIDEODEC_TYPE_HW_AVCDEC, &decoder_info, &decoder_info_out);
	
	size_t sz = (decoder_info_out.frameMemSize + 0xFFFFF) & ~0xFFFFF;
	decoder.frameBuf.size = sz;
	decoderBlock = sceKernelAllocMemBlock("decoder", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW, sz, NULL);
	sceKernelGetMemBlockBase(decoderBlock, &decoder.frameBuf.pBuf);
	sceAvcdecCreateDecoder(SCE_VIDEODEC_TYPE_HW_AVCDEC, &decoder, &decoder_info);
	
	SceKernelMemBlockType orig = vita2d_texture_get_alloc_memblock_type();
	vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW);
	for (int i=0; i < VIDEO_BUFFERING; i++){
		out_text[i].magic = 0xABADBEEF;
		out_text[i].text = vita2d_create_empty_texture(960, 544);
		videobuf[i] = vita2d_texture_get_datap(out_text[i].text);
	}
	vita2d_texture_set_alloc_memblock_type(orig);
	
	isPlayerReady = true;
	return 0;
}

static int lua_openavc(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif
	
	// Opening given file
	char* file = (char*)luaL_checkstring(L, 1);
	float fps = (float)luaL_checknumber(L, 2);
	looping = lua_toboolean(L, 3);
	ctrlInit(&videoDecoder, file, DECODER_BUFSIZE);
	
	deltaTick = 1.0f / fps;
	isPlaying = true;
	
	// Starting decoder thread
	decoderThread = sceKernelCreateThread("Videodec Thread", &decoderThreadFunc, 0x10000100, 0x10000, 0, 0, NULL);
	int res = sceKernelStartThread(decoderThread, 0, NULL);
	
	return 0;
}

static int lua_output(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif	
	lua_pushinteger(L, (uint32_t)cur_text);
	return 1;
}

static int lua_pause(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif	
	isPlaying = false;
	return 0;
}

static int lua_resume(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif	
	isPlaying = true;
	return 0;
}

static int lua_playing(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif	
	lua_pushboolean(L, isPlaying);
	return 1;
}

static int lua_term(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif
	decoderRunning = false;
	sceKernelWaitThreadEnd(decoderThread, NULL, NULL);
	sceAvcdecDeleteDecoder(&decoder);
	sceVideodecTermLibrary(SCE_VIDEODEC_TYPE_HW_AVCDEC);
	isPlayerReady = false;
	for (int i=0; i < VIDEO_BUFFERING; i++){
		vita2d_free_texture(out_text[i].text);
	}
	sceKernelFreeMemBlock(decoderBlock);
	return 0;
}

//Register our Video Functions
static const luaL_Reg Video_functions[] = {
  {"init",        lua_init},
  {"term",        lua_term},
  {"openAvc",     lua_openavc},
  {"getOutput",   lua_output},
  {"pause",       lua_pause},
  {"resume",      lua_resume},
  {"isPlaying",   lua_playing},
  {0, 0}
};

void luaVideo_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Video_functions, 0);
	lua_setglobal(L, "Video");
}