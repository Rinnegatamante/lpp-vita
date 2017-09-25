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
#include "include/videodec.h"

#define DECODER_BUFSIZE        0x5FA00
#define FRAMEBUFFER_SIZE       0x200000
#define FRAMEBUFFER_ALIGNMENT  0x40000
#define VIDEO_BUFFERING        5
#define AUDIO_CHANNELS         8    // PSVITA has 8 available audio channels

SceUID decoderThread, decoderBlock, deployerThread, Buffers_Mutex;
SceAvcdecCtrl decoder = {0};
static uint8_t* auBuf;
static bool isPlayerReady = false;
static bool decoderRunning = false;
static bool looping = false;
controller videoDecoder;
void* videobuf[VIDEO_BUFFERING];
uint8_t videobuf_idx = 0;
float deltaTick = 0.0f;
volatile uint8_t busy_buffers = 0;
uint8_t cur_videobuf_idx = 0;
bool isPlaying = false;
float video_audio_tick = 0.0f;
DecodedMusic* audio_track;

extern bool availThreads[];
extern DecodedMusic* new_track;
extern SceUID NewTrack_Mutex;
extern SceUID Audio_Mutex;

lpp_texture out_text[VIDEO_BUFFERING];
lpp_texture* cur_text = NULL;

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
	
	// Decoder main loop
	while (decoderRunning){
		
		// Check if we have free buffers to decode a new frame
		if (busy_buffers > (VIDEO_BUFFERING - 2)) continue;
		
		// Read next Access Unit from file
		ret = ctrlReadFrame(&videoDecoder, auBuf, &size);
		if (ret > 0){
			
			picture.frame.pixelType = 0;
			picture.frame.framePitch = 960;
			picture.frame.frameWidth = 960;
			picture.frame.frameHeight = 544;
			picture.frame.pPicture[0] = videobuf[videobuf_idx];
			picture.frame.pPicture[1] = NULL;
			
			au.es.pBuf = auBuf;
			au.es.size = size;
			au.dts.lower = 0;
			au.dts.upper = 0;
			au.pts.lower = 0;
			au.pts.upper = 0;
			
			array_picture.numOfElm = 1;
			array_picture.pPicture = &pictures;
			
			// Decoding current Access Unit
			sceKernelWaitSema(Buffers_Mutex, 1, NULL);
			sceAvcdecDecode(&decoder, &au, &array_picture);
			
			if (array_picture.numOfOutput >= 1){
				busy_buffers++;
				videobuf_idx = (videobuf_idx + 1) % VIDEO_BUFFERING;
			}
			sceKernelSignalSema(Buffers_Mutex, 1);
		
		// File reached EOF, closing decoder or restarting the playback
		}else{
			if (looping) ctrlRewind(&videoDecoder);
			else decoderRunning = false;
		}

		// Invoking scheduler
		sceKernelDelayThread(100);
		
	}
	
	return sceKernelExitDeleteThread(0);

}

static int frameDeployer(unsigned int args, void* arg){
	
	float tick = 0.0f;
	float cur_tick = 0.0f;
	float pause_delta_tick = 0.0f;
	bool _isPlaying = true;
	
	// Frame deployer main loop
	while (isPlayerReady){
		
		// Handling player status changes
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
		
		// Updating current frame if required
		sceKernelWaitSema(Buffers_Mutex, 1, NULL);
		if (cur_tick == 0.0f){
			cur_tick = tick = video_audio_tick;
		}else cur_tick = (sceKernelGetProcessTimeWide() / 1000000.0f);
		if ((cur_tick - tick > deltaTick) && (busy_buffers > 0)){
			cur_text = &out_text[cur_videobuf_idx];
			cur_videobuf_idx = (cur_videobuf_idx + 1) % VIDEO_BUFFERING;			
			busy_buffers--;
			tick += deltaTick;
		}
		sceKernelSignalSema(Buffers_Mutex, 1);
		
		// Invoking scheduler
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
	
	// Allocating Access Unit buffer for the decoder
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
	
	// Initializing sceVideodec
	sceVideodecInitLibrary(SCE_VIDEODEC_TYPE_HW_AVCDEC, &init);
	sceAvcdecQueryDecoderMemSize(SCE_VIDEODEC_TYPE_HW_AVCDEC, &decoder_info, &decoder_info_out);
	
	// Creating a sceAvcdec decoder
	decoder.handle = 0;
	decoder.frameBuf.pBuf = NULL;
	decoder.frameBuf.size = 0;
	sceAvcdecCreateDecoder(SCE_VIDEODEC_TYPE_HW_AVCDEC, &decoder, &decoder_info);
	
	// Allocating enough textures to handle our video playback
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

static int lua_openpshv(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif
	
	char* file = (char*)luaL_checkstring(L, 1);
	bool looping = lua_toboolean(L, 2);
	
	// Opening given file for audio decoding
	FILE* f = fopen(file, "rb");
	#ifndef SKIP_ERROR_HANDLING
	if (f == NULL) return luaL_error(L, "file doesn't exist.");
	uint32_t magic;
	fread(&magic,1,4,f);	
	if (magic != 0x56485350 /* PSHV */){
		fclose(f);
		return luaL_error(L, "unknown video file format.");
	}
	fseek(f, 0, SEEK_SET);
	#endif
	DecodedMusic* memblock = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	memblock->handle = f;
	memblock->isPlaying = false;
	memblock->audioThread = 0xFF;
	memblock->tempBlock = false;
	sprintf(memblock->filepath, "%s", file);
	
	// Opening given file for video decoding
	float fps;
	ctrlInit(&videoDecoder, file, DECODER_BUFSIZE, &fps);
	deltaTick = 1.0f / fps;
	isPlaying = true;
	
	// Creating a mutex to avoid race conditions between decoder and frame deployer threads
	Buffers_Mutex = sceKernelCreateSema("Buffers Mutex", 0, 1, 1, NULL);
	
	// Starting decoder and frames deployer thread
	decoderThread = sceKernelCreateThread("Videodec Thread", &decoderThreadFunc, 0x10000100, 0x10000, 0, 0, NULL);
	sceKernelStartThread(decoderThread, 0, NULL);
	deployerThread = sceKernelCreateThread("Frames Thread", &frameDeployer, 0x10000100, 0x10000, 0, 0, NULL);
	sceKernelStartThread(deployerThread, 0, NULL);
	
	// Wait till a thread is available
	bool found = false;
	for (int i=0; i<AUDIO_CHANNELS; i++){
		found = availThreads[i];
		if (found) break;
	}
	if (!found) return 0;
	
	// Check if the music is already loaded into an audio thread
	if (memblock->audioThread != 0xFF){
	
		// We create a temporary duplicated and play it instead of the original one
		DecodedMusic* dup = (DecodedMusic*)malloc(sizeof(DecodedMusic));
		memcpy(dup, memblock, sizeof(DecodedMusic));
		dup->handle = fopen(dup->filepath, "rb");
		dup->tempBlock = true;
		memblock = dup;
		
	}
	
	// Properly setting music memory block info
	memblock->loop = looping;
	memblock->pauseTrigger = false;
	memblock->closeTrigger = false;
	memblock->isPlaying = true;
	memblock->isVideoTrack = true;
	memblock->volume = 32767;
	
	// Waiting till track slot is free
	sceKernelWaitSema(NewTrack_Mutex, 1, NULL);
	
	// Passing music to an audio thread
	new_track = audio_track = memblock;
	sceKernelSignalSema(Audio_Mutex, 1);
	
	return 0;
}

static int lua_output(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif	
	sceKernelWaitSema(Buffers_Mutex, 1, NULL);
	lua_pushinteger(L, (uint32_t)cur_text);
	sceKernelSignalSema(Buffers_Mutex, 1);
	return 1;
}

/*
	FIXME: Resume/pause may actually desync video/audio, especially if spammed.
*/
static int lua_pause(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif	
	isPlaying = false;
	if (audio_track->isPlaying) audio_track->pauseTrigger = true;
	return 0;
}

static int lua_resume(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif	
	isPlaying = true;
	if (!audio_track->isPlaying) audio_track->pauseTrigger = true;
	return 0;
}

static int lua_playing(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
	#endif	
	lua_pushinteger(L, isPlaying);
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
	isPlayerReady = false;
	sceKernelWaitThreadEnd(deployerThread, NULL, NULL);
	sceAvcdecDeleteDecoder(&decoder);
	sceVideodecTermLibrary(SCE_VIDEODEC_TYPE_HW_AVCDEC);
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
  {"open",        lua_openpshv},
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