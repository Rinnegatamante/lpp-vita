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
#include <vitasdk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include "include/luaplayer.h"

// Internal recorder state
#define MIC_SAMPLERATE 16000
#define MIC_GRAIN      256
#define MIC_GRAIN_X_2  512
volatile bool isSampling = false;
volatile bool termSampling = false;
volatile bool termMic = false;
uint16_t* audioBuf = NULL;
uint16_t resampleBuffer[256];
uint32_t bufSize = 0;
uint32_t resampleNum = 0;
uint32_t bufPos = 0;
int samplerate = 0;
SceUID Mic_Mutex;

int micThread(SceSize args, void* argc){
	
	for (;;){
		
		if (termMic){
			sceKernelExitDeleteThread(0);
			termMic = false;
		}
		
		// Waiting for an audio input request
		sceKernelWaitSema(Mic_Mutex, 1, NULL);
		
		if (termMic){
			sceKernelExitDeleteThread(0);
			termMic = false;
		}
		
		// Initializing audio port
		isSampling = true;
		int ch = sceAudioInOpenPort(SCE_AUDIO_IN_PORT_TYPE_VOICE, MIC_GRAIN, MIC_SAMPLERATE, SCE_AUDIO_IN_PARAM_FORMAT_S16_MONO);
		
		// Recording loop
		for (;;){
			
			// Recorder is sampling
			if (isSampling){
				if (bufPos >= (bufSize>>1)) isSampling = false;
				if (samplerate == 16000){
					sceAudioInInput(ch, &audioBuf[bufPos]);
					bufPos += MIC_GRAIN;
				}else{
					sceAudioInInput(ch, resampleBuffer);
					for (int i=0; i < MIC_GRAIN; i++){
						for (int j=0; j < resampleNum; j++){
							audioBuf[bufPos+j] = resampleBuffer[i];
						}
						bufPos += resampleNum;
					}
				}
			}
			
			// Recorder finished sampling
			if (bufPos >= (bufSize>>1)) isSampling = false;
			
			// Called a sampling stop
			if (termSampling){
				free(audioBuf);
				audioBuf = NULL;
				isSampling = false;
				bufPos = 0;
				bufSize = 0;
				termSampling = false;
				
				// Freeing audio port
				sceAudioInReleasePort(ch);
				
				break;
			}
			
		}
		
	}
		
}

static int lua_regsound(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	uint32_t time = luaL_checkinteger(L, 1);
	samplerate = luaL_checkinteger(L, 2);
	if ((samplerate % MIC_SAMPLERATE) != 0) return luaL_error(L, "samplerate must be a multiple of 16000");
	if (samplerate > 48000) return luaL_error(L, "too high samplerate");
	
	// Initializing audio buffer
	if (audioBuf != NULL) free(audioBuf);
	bufSize = samplerate * 2 * time;
	bufPos = 0;
	audioBuf = (uint16_t*)memalign(MIC_GRAIN, bufSize);
	resampleNum = samplerate / MIC_SAMPLERATE;
	
	// Sending request to Mic thread
	isSampling = true;
	sceKernelSignalSema(Mic_Mutex, 1);
	
	return 0;
}

static int lua_micplaying(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	lua_pushboolean(L, isSampling);
	return 1;
}

static int lua_stoprec(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	const char* file = luaL_checkstring(L, 1);
	uint32_t four_bytes;
	uint16_t two_bytes;
	isSampling = false;
	
	// Saving the output file
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT, 0777);
	sceIoWrite(fd,"RIFF", 4);
	four_bytes = bufSize + 36;
	sceIoWrite(fd, &four_bytes, 4);
	sceIoWrite(fd, "WAVEfmt ", 8);
	four_bytes = 0x10;
	sceIoWrite(fd, &four_bytes, 4);
	two_bytes = 0x01;
	sceIoWrite(fd, &two_bytes, 2);
	sceIoWrite(fd, &two_bytes, 2);
	sceIoWrite(fd, &samplerate, 4);
	four_bytes = samplerate<<1;
	sceIoWrite(fd, &four_bytes, 4);
	two_bytes = 0x02;
	sceIoWrite(fd, &two_bytes, 2);
	two_bytes = 0x10;
	sceIoWrite(fd, &two_bytes, 2);
	sceIoWrite(fd,"data", 4);
	sceIoWrite(fd, &bufSize, 4);
	sceIoWrite(fd, audioBuf, bufSize);
	sceIoClose(fd);
	
	// Resetting recorder state
	termSampling = true;
	
	return 0;
}

static int lua_pausemic(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	if (isSampling) isSampling = false;
	return 0;
}

static int lua_resumemic(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	if (!isSampling) isSampling = true;
	return 0;
}


//Register our Mic Functions
static const luaL_Reg Mic_functions[] = {
	{"start",        lua_regsound},
	{"isRecording",  lua_micplaying},	
	{"stop",         lua_stoprec},	
	{"pause",        lua_pausemic},	
	{"resume",       lua_resumemic},	
	{0, 0}
};

void luaMic_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Mic_functions, 0);
	lua_setglobal(L, "Mic");
}