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

#define FRAMEBUFFER_ALIGNMENT  0x40000
#define VIDEO_BUFFERING        2
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

vita2d_texture *videobuf[VIDEO_BUFFERING];
SceAvPlayerFrameInfo videoFrame[VIDEO_BUFFERING];
uint8_t videobuf_idx = 0;
SceAvPlayerHandle avplayer;
static bool isPlayerReady = false;
static bool isPlaying = false;
volatile int audio_vol = 32767;

lpp_texture out_text[VIDEO_BUFFERING];
lpp_texture* cur_text = nullptr;

FILE *sub_handle = nullptr;
int64_t sub_time_start = 0;
int64_t sub_time_end = 0;
char sub_string[16384];
bool sub_eof = true;
bool sub_vtt = false;

static int audioThread(unsigned int args, void* arg){
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, 1024, 48000, SCE_AUDIO_OUT_MODE_STEREO);
	sceAudioOutSetConfig(ch, -1, -1, (SceAudioOutMode)-1);
	
	SceAvPlayerFrameInfo audio_frame;
	memset(&audio_frame, 0, sizeof(SceAvPlayerFrameInfo));
	
	// Setting audio channel volume
	int vol_stereo[] = {audio_vol, audio_vol};
	sceAudioOutSetVolume(ch, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol_stereo);
	
	while (isPlayerReady) {
		if (vol_stereo[0] != audio_vol) {
			vol_stereo[0] = vol_stereo[1] = audio_vol;
			sceAudioOutSetVolume(ch, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol_stereo);
		}
		if (sceAvPlayerIsActive(avplayer)) {
			if (sceAvPlayerGetAudioData(avplayer, &audio_frame)) {
				sceAudioOutSetConfig(ch, -1, audio_frame.details.audio.sampleRate, audio_frame.details.audio.channelCount == 1 ? SCE_AUDIO_OUT_MODE_MONO : SCE_AUDIO_OUT_MODE_STEREO);
				sceAudioOutOutput(ch, audio_frame.pData);
			} else {
				sceKernelDelayThread(1000);
			}
		} else {
			sceKernelDelayThread(1000);
		}
	}
	
	return sceKernelExitDeleteThread(0);
}

void *memalloc(void *p, uint32_t alignment, uint32_t size) {
	return memalign(alignment, size);
}

void dealloc(void *p, void *ptr) {
	free(ptr);
}

void *gpu_alloc(void *p, uint32_t alignment, uint32_t size) {
	void *res = nullptr;
	if (alignment < FRAMEBUFFER_ALIGNMENT) {
		alignment = FRAMEBUFFER_ALIGNMENT;
	}
	size = ALIGN(size, alignment);
#ifdef SYS_APP_MODE
	size = ALIGN(size, 1024 * 1024);
	SceUID memblock = sceKernelAllocMemBlock("Video Memblock", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW, size, nullptr);
#else
	SceKernelAllocMemBlockOpt opt;
	memset(&opt, 0, sizeof(opt));
	opt.size = sizeof(SceKernelAllocMemBlockOpt);
	opt.attr = SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_HAS_ALIGNMENT;
	opt.alignment = alignment;
	SceUID memblock = sceKernelAllocMemBlock("Video Memblock", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, size, &opt);
#endif
	sceKernelGetMemBlockBase(memblock, &res);
	sceGxmMapMemory(res, size, (SceGxmMemoryAttribFlags)(SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE));
	return res;
}

void gpu_dealloc(void *p, void *ptr) {
	SceUID memblock = sceKernelFindMemBlockByAddr(ptr, 0);
	sceKernelFreeMemBlock(memblock);
}

static int lua_init(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (isPlayerReady) return 0;
#endif
	
	for (int i=0; i < VIDEO_BUFFERING; i++) {
		videobuf[i] = (vita2d_texture*)malloc(sizeof(vita2d_texture));
		videobuf[i]->palette_UID = 0;
		memset(&videoFrame[i], 0, sizeof(SceAvPlayerFrameInfo));
		out_text[i].magic = 0xABADBEEF;
		out_text[i].text = videobuf[i];
		out_text[i].data = NULL;
	}
	
	sceSysmoduleLoadModule(SCE_SYSMODULE_AVPLAYER);
	
	SceAvPlayerInitData init_data;
	memset(&init_data, 0, sizeof(SceAvPlayerInitData));
	init_data.memoryReplacement.allocate            = memalloc;
	init_data.memoryReplacement.deallocate          = dealloc;
	init_data.memoryReplacement.allocateTexture 	= gpu_alloc;
	init_data.memoryReplacement.deallocateTexture 	= gpu_dealloc;
	init_data.basePriority = 0xA0;
	init_data.numOutputVideoFrameBuffers = 2;
	init_data.autoStart = true;
	avplayer = sceAvPlayerInit(&init_data);
	
	isPlayerReady = true;
	return 0;
}

static int lua_openvid(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
#endif
	
	char *file = (char*)luaL_checkstring(L, 1);
	bool looping = false;
	if (argc == 2) {
		looping = lua_toboolean(L, 2);
	}
	
#ifndef SKIP_ERROR_HANDLING
	FILE* f = fopen(file, "rb");
	if (f == NULL) return luaL_error(L, "file doesn't exist.");
	else fclose(f);
#endif
	
	sceAvPlayerAddSource(avplayer, file);
	sceAvPlayerSetLooping(avplayer, looping);
	sceAvPlayerSetTrickSpeed(avplayer, 100);
	
	SceUID audio_thread = sceKernelCreateThread("Vid Audio Thread", &audioThread, 0x10000100, 0x10000, 0, 0, NULL);
	sceKernelStartThread(audio_thread, 0, NULL);
	
	isPlaying = true;
	return 0;
}

static int lua_output(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
#endif
	if (sceAvPlayerGetVideoData(avplayer, &videoFrame[videobuf_idx])) {
		sceGxmTextureInitLinear(
			&videobuf[videobuf_idx]->gxm_tex,
			videoFrame[videobuf_idx].pData,
			SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1,
			videoFrame[videobuf_idx].details.video.width,
			videoFrame[videobuf_idx].details.video.height, 0);
		cur_text = &out_text[videobuf_idx];
		videobuf_idx = (videobuf_idx + 1) % VIDEO_BUFFERING;
	}
	lua_pushinteger(L, (uint32_t)cur_text);
	return 1;
}

static int lua_pause(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
#endif	
	sceAvPlayerPause(avplayer);
	isPlaying = false;
	return 0;
}

static int lua_resume(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
#endif	
	sceAvPlayerResume(avplayer);
	isPlaying = true;
	return 0;
}

static int lua_playing(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
#endif
	lua_pushboolean(L, sceAvPlayerIsActive(avplayer) && isPlaying);
	return 1;
}

static int lua_closevid(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
#endif
	sceAvPlayerStop(avplayer);
	return 0;
}

static int lua_term(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
#endif
	isPlayerReady = false;
	for (int i=0; i < VIDEO_BUFFERING; i++){
		vita2d_free_texture(out_text[i].text);
	}
	sceAvPlayerClose(avplayer);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_AVPLAYER);
	return 0;
}

static int lua_timing(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
#endif
	lua_pushnumber(L, sceAvPlayerCurrentTime(avplayer));
	return 1;
}

static int lua_jump(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int64_t timing = luaL_checknumber(L, 1);
	sceAvPlayerJumpToTime(avplayer, timing);
	if (sub_handle && timing < sceAvPlayerCurrentTime(avplayer)) {
		fseek(sub_handle, 0, SEEK_SET);
		sub_string[0] = 0;
		sub_time_start = 0;
		sub_time_end = 0;
		sub_eof = false;
		sceKernelDelayThread(100000); // We let sceAvPlayer actually rewind the video before resuming main thread
	}
	return 0;
}

static int lua_mode(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int mode = luaL_checknumber(L, 1);
	sceAvPlayerSetTrickSpeed(avplayer, mode);
	return 0;
}

static int lua_setvol(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int vol = luaL_checkinteger(L, 1);
	audio_vol = (vol < 0) ? 0 : ((vol > 32767) ? 32767 : vol);
	return 0;
}

static int lua_getvol(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, audio_vol);
	return 1;
}

static int lua_subopen(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char *file = luaL_checkstring(L, 1);
	sub_handle = fopen(file, "rb");
#ifndef SKIP_ERROR_HANDLING
	if (sub_handle == NULL) return luaL_error(L, "file doesn't exist.");
#endif
	sub_string[0] = 0;
	sub_time_start = 0;
	sub_time_end = 0;
	sub_eof = false;
	char header[256];
	fgets(header, 256, sub_handle);
	sub_vtt = !strncmp(header, "WEBVTT", 6);
	return 0;
}

static int lua_subclose(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	if (sub_handle) fclose(sub_handle);
	sub_handle = nullptr;
	return 0;
}

static int lua_getsub(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	int64_t cur_time = sceAvPlayerCurrentTime(avplayer);
	if (sub_handle && !sub_eof && cur_time > sub_time_end) {
		char str[2048];
		while (fgets(str, 2048, sub_handle)) {
			char *s = strstr(str, " --> ");
			if (s) {
				int times_start[4];
				int times_end[4];
				int64_t start;
				int64_t end;
				if (s - str > 10) { // Has hours
					if (sub_vtt) // VTT
						sscanf(str, "%d:%2d:%2d.%3d --> %d:%2d:%2d.%3d", &times_start[3], &times_start[0], &times_start[1], &times_start[2], &times_end[3], &times_end[0], &times_end[1], &times_end[2]);
					else // SRT
						sscanf(str, "%d:%2d:%2d,%3d --> %d:%2d:%2d,%3d", &times_start[3], &times_start[0], &times_start[1], &times_start[2], &times_end[3], &times_end[0], &times_end[1], &times_end[2]);
					start = times_start[2] + times_start[1] * 1000 + times_start[0] * 1000 * 60 + times_start[3] * 1000 * 60 * 60;
					end = times_end[2] + times_end[1] * 1000 + times_end[0] * 1000 * 60 + times_end[3] * 1000 * 60 * 60;
				} else {
					if (sub_vtt) // VTT
						sscanf(str, "%2d:%2d.%3d --> %2d:%2d.%3d", &times_start[0], &times_start[1], &times_start[2], &times_end[0], &times_end[1], &times_end[2]);
					else // SRT
						sscanf(str, "%2d:%2d,%3d --> %2d:%2d,%3d", &times_start[0], &times_start[1], &times_start[2], &times_end[0], &times_end[1], &times_end[2]);
					start = times_start[2] + times_start[1] * 1000 + times_start[0] * 1000 * 60;
					end = times_end[2] + times_end[1] * 1000 + times_end[0] * 1000 * 60;
				}
				if (cur_time < end) {
					sub_string[0] = 0;
					sub_time_end = end;
					sub_time_start = start;
					while (fgets(str, 2048, sub_handle)) {
						if (strlen(str) == 1) break;
						strcat(sub_string, str);
					}
					lua_pushstring(L, cur_time > sub_time_start ? sub_string : "");
					return 1;
				}
			}
		}
		sub_eof = true;
	}
	lua_pushstring(L, cur_time > sub_time_start ? sub_string : "");
	return 1;
}

//Register our Video Functions
static const luaL_Reg Video_functions[] = {
  {"init",          lua_init},
  {"term",          lua_term},
  {"open",          lua_openvid},
  {"close",         lua_closevid},
  {"setVolume",     lua_setvol},
  {"getVolume",     lua_getvol},
  {"getOutput",     lua_output},
  {"pause",         lua_pause},
  {"resume",        lua_resume},
  {"isPlaying",     lua_playing},
  {"getTime",       lua_timing},
  {"jumpToTime",    lua_jump},
  {"setPlayMode",   lua_mode},
  {"openSubs",      lua_subopen},
  {"closeSubs",     lua_subclose},
  {"getSubs",       lua_getsub},
  {0, 0}
};

void luaVideo_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Video_functions, 0);
	lua_setglobal(L, "Video");
	int NORMAL_MODE = 100;
	int FAST_FORWARD_2X_MODE = 200;
	int FAST_FORWARD_4X_MODE = 400;
	int FAST_FORWARD_8X_MODE = 800;
	int FAST_FORWARD_16X_MODE = 1600;
	int FAST_FORWARD_32X_MODE = 3200;
	int REWIND_8X_MODE = -800;
	int REWIND_16X_MODE = -1600;
	int REWIND_32X_MODE = -3200;
	VariableRegister(L, NORMAL_MODE);
	VariableRegister(L, FAST_FORWARD_2X_MODE);
	VariableRegister(L, FAST_FORWARD_4X_MODE);
	VariableRegister(L, FAST_FORWARD_8X_MODE);
	VariableRegister(L, FAST_FORWARD_16X_MODE);
	VariableRegister(L, FAST_FORWARD_32X_MODE);
}