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

#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "include/luaplayer.h"
#include "include/audiodec/audio_decoder.h"
#define stringify(str) #str
#define BooleanRegister(lua, value) do { lua_pushboolean(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

#define BUFSIZE 8192 // Max dimension of audio buffer size
#define NSAMPLES 2048 // Number of samples for output
#define AUDIO_CHANNELS 8 // PSVITA has 8 available audio channels

// Music block struct
struct DecodedMusic{
	uint8_t* audiobuf;
	uint8_t* audiobuf2;
	uint8_t* cur_audiobuf;
	FILE* handle;
	volatile bool isPlaying;
	bool loop;
	volatile bool pauseTrigger;
	volatile bool closeTrigger;
	volatile uint8_t audioThread;
	char filepath[256];
	bool tempBlock;
};

SceUID AudioThreads[AUDIO_CHANNELS], Audio_Mutex, NewTrack_Mutex;
DecodedMusic* new_track = NULL;
bool initialized = false;
bool availThreads[AUDIO_CHANNELS];
std::unique_ptr<AudioDecoder> audio_decoder[AUDIO_CHANNELS];
volatile bool mustExit = false;
uint8_t ids[] = {0, 1, 2, 3, 4, 5, 6 ,7}; // Change this if you edit AUDIO_CHANNELS macro

// Audio thread code
static int audioThread(unsigned int args, void* arg){

	// Getting thread id
	uint8_t* argv = (uint8_t*)arg;
	uint8_t id = argv[0];
	
	// Initializing audio port
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, NSAMPLES, 48000, SCE_AUDIO_OUT_MODE_STEREO);
	sceAudioOutSetConfig(ch, -1, -1, -1);
	int vol_stereo[] = {32767, 32767};
	sceAudioOutSetVolume(ch, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol_stereo);
	
	DecodedMusic* mus;
	for (;;){
		
		// Waiting for an audio output request
		sceKernelWaitSema(Audio_Mutex, 1, NULL);
		
		// Setting thread as busy
		availThreads[id] = false;
		
		// Fetching track
		mus = new_track;
		mus->audioThread = id;
		sceKernelSignalSema(NewTrack_Mutex, 1);
		
		// Checking if a new track is available
		if (mus == NULL){
			
			//If we enter here, we probably are in the exiting procedure
			if (mustExit){
				sceKernelSignalSema(Audio_Mutex, 1);
				sceAudioOutReleasePort(ch);
				sceKernelExitDeleteThread(0);
			}
		
		}
		
		// Initializing audio decoder
		audio_decoder[id] = AudioDecoder::Create(mus->handle, "Track");
		if (audio_decoder[id] == NULL) continue; // TODO: Find why this case apparently can happen
		audio_decoder[id]->Open(mus->handle);
		audio_decoder[id]->SetLooping(mus->loop);
		audio_decoder[id]->SetFormat(48000, AudioDecoder::Format::S16, 2);
		
		// Initializing audio buffers
		mus->audiobuf = (uint8_t*)malloc(BUFSIZE);
		mus->audiobuf2 = (uint8_t*)malloc(BUFSIZE);
		mus->cur_audiobuf = mus->audiobuf;
		
		// Audio playback loop
		for (;;){
		
			// Check if the music must be paused
			if (mus->pauseTrigger || mustExit){
			
				// Check if the music must be closed
				if (mus->closeTrigger){
					free(mus->audiobuf);
					free(mus->audiobuf2);
					audio_decoder[id].reset();
					free(mus);
					mus = NULL;
					availThreads[id] = true;
					if (!mustExit) break;
				}
				
				// Check if the thread must be closed
				if (mustExit){
				
					// Check if the audio stream has already been closed
					if (mus != NULL){
						mus->closeTrigger = true;
						continue;
					}
					
					// Recursively closing all the threads
					sceKernelSignalSema(Audio_Mutex, 1);
					sceAudioOutReleasePort(ch);
					sceKernelExitDeleteThread(0);
					
				}
			
				mus->isPlaying = !mus->isPlaying;
				mus->pauseTrigger = false;
			}
			
			if (mus->isPlaying){
				
				// Check if audio playback finished
				if ((!mus->loop) && audio_decoder[id]->IsFinished()) mus->isPlaying = false;
				
				// Update audio output
				if (mus->cur_audiobuf == mus->audiobuf) mus->cur_audiobuf = mus->audiobuf2;
				else mus->cur_audiobuf = mus->audiobuf;
				audio_decoder[id]->Decode(mus->cur_audiobuf, BUFSIZE);	
				sceAudioOutOutput(ch, mus->cur_audiobuf);
				
			}else{
				
				// Check if we finished a non-looping audio playback
				if ((!mus->loop) && audio_decoder[id]->IsFinished()){
					
					// Releasing the audio file
					audio_decoder[id].reset();
					if (mus->tempBlock){
						free(mus->audiobuf);
						free(mus->audiobuf2);
						free(mus);
						mus = NULL;
					}else{
						mus->handle = fopen(mus->filepath,"rb");
						mus->audioThread = 0xFF;
					}
					availThreads[id] = true;
					break;
					
				}
				
			}
			
		}
		
	}
	
}

static int lua_init(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	
	if (!initialized){
	
		// Creating audio mutexs
		Audio_Mutex = sceKernelCreateSema("Audio Mutex", 0, 0, 1, NULL);
		NewTrack_Mutex = sceKernelCreateSema("NewTrack Mutex", 0, 1, 1, NULL);
		
		// Starting audio threads
		for (int i=0;i < AUDIO_CHANNELS; i++){
			availThreads[i] = true;
			AudioThreads[i] = sceKernelCreateThread("Audio Thread", &audioThread, 0x10000100, 0x10000, 0, 0, NULL);
			int res = sceKernelStartThread(AudioThreads[i], sizeof(ids[i]), &ids[i]);
			if (res != 0) return luaL_error(L, "Failed to init an audio thread.");
		}
	
		initialized = true;
	}
	return 0;
}

static int lua_term(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	
	if (initialized){
		
		// Starting exit procedure for audio threads
		mustExit = true;
		sceKernelSignalSema(Audio_Mutex, 1);
		for (int i=0;i<AUDIO_CHANNELS;i++){
			sceKernelWaitThreadEnd(AudioThreads[i], NULL, NULL);
		}
		mustExit = false;
		
		// Deleting audio mutex
		sceKernelDeleteSema(Audio_Mutex);
		
		initialized = false;
	}
	return 0;
}

static int lua_closesong(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	mus->closeTrigger = true;
	mus->pauseTrigger = true;
	return 0;
}

static int lua_pause(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	if (mus->isPlaying) mus->pauseTrigger = true;
	
	return 0;
}

static int lua_isplaying(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	lua_pushboolean(L,mus->isPlaying);	
	return 1;
}

static int lua_resume(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	if (!mus->isPlaying) mus->pauseTrigger = true;
	
	return 0;
}

static int lua_openMp3(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char* path = luaL_checkstring(L, 1);
	
	// Opening file and checking for magic
	FILE* f = fopen(path, "rb");
	if (f == NULL) return luaL_error(L, "file doesn't exist.");
	uint32_t magic;
	fread(&magic,1,4,f);
	if (magic != 0x03334449) return luaL_error(L, "Corrupted mp3 file.");
	fseek(f, 0, SEEK_SET);
	
	// Allocating and pushing music block
	DecodedMusic* memblock = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	memblock->handle = f;
	memblock->isPlaying = false;
	memblock->audioThread = 0xFF;
	memblock->tempBlock = false;
	sprintf(memblock->filepath, "%s", path);
	lua_pushinteger(L,(uint32_t)memblock);
	return 1;
}

static int lua_openMidi(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char* path = luaL_checkstring(L, 1);
	
	// Opening file and checking for magic
	FILE* f = fopen(path, "rb");
	if (f == NULL) return luaL_error(L, "file doesn't exist.");
	uint32_t magic;
	fread(&magic,1,4,f);
	if (magic != 0x6468544d) return luaL_error(L, "Corrupted midi file.");
	fseek(f, 0, SEEK_SET);
	
	// Allocating and pushing music block
	DecodedMusic* memblock = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	memblock->handle = f;
	memblock->isPlaying = false;
	memblock->audioThread = 0xFF;
	memblock->tempBlock = false;
	sprintf(memblock->filepath, "%s", path);
	lua_pushinteger(L,(uint32_t)memblock);
	return 1;
}

static int lua_openOgg(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char* path = luaL_checkstring(L, 1);
	
	// Opening file and checking for magic
	FILE* f = fopen(path, "rb");
	if (f == NULL) return luaL_error(L, "file doesn't exist.");
	uint32_t magic;
	fread(&magic,1,4,f);
	if (magic != 0x5367674f) return luaL_error(L, "Corrupted ogg file.");
	fseek(f, 0, SEEK_SET);
	
	// Allocating and pushing music block
	DecodedMusic* memblock = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	memblock->handle = f;
	memblock->isPlaying = false;
	memblock->audioThread = 0xFF;
	memblock->tempBlock = false;
	sprintf(memblock->filepath, "%s", path);
	lua_pushinteger(L,(uint32_t)memblock);
	return 1;
}

static int lua_openWav(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char* path = luaL_checkstring(L, 1);
	
	// Opening file and checking for magic
	FILE* f = fopen(path, "rb");
	if (f == NULL) return luaL_error(L, "file doesn't exist.");
	uint32_t magic;
	fread(&magic,1,4,f);
	if (magic != 0x46464952) return luaL_error(L, "Corrupted wav file.");
	fseek(f, 0, SEEK_SET);
	
	// Allocating and pushing music block
	DecodedMusic* memblock = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	memblock->handle = f;
	memblock->isPlaying = false;
	memblock->audioThread = 0xFF;
	memblock->tempBlock = false;
	sprintf(memblock->filepath, "%s", path);
	lua_pushinteger(L,(uint32_t)memblock);
	return 1;
}

static int lua_play(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 2) return luaL_error(L, "wrong number of arguments");
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	bool loop = lua_toboolean(L, 2);
	
	// Check if the music is already loaded into an audio thread
	if (mus->audioThread != 0xFF){
	
		// We create a temporary duplicated and play it instead of the original one
		DecodedMusic* dup = (DecodedMusic*)malloc(sizeof(DecodedMusic));
		memcpy(dup, mus, sizeof(DecodedMusic));
		dup->handle = fopen(dup->filepath, "rb");
		dup->tempBlock = true;
		mus = dup;
		
	}
	
	// Properly setting music memory block info
	mus->loop = loop;
	mus->pauseTrigger = false;
	mus->closeTrigger = false;
	mus->isPlaying = true;
	
	// Wait till a thread is available
	bool found = false;
	while (!found){
		for (int i=0; i<AUDIO_CHANNELS; i++){
			found = availThreads[i];
			if (found) break;
		}
	}
	
	// Waiting till track slot is free
	sceKernelWaitSema(NewTrack_Mutex, 1, NULL);
	
	// Passing music to an audio thread
	new_track = mus;
	sceKernelSignalSema(Audio_Mutex, 1);
	
	return 0;
}

//Register our Sound Functions
static const luaL_Reg Sound_functions[] = {
	{"init",						lua_init},
	{"term",						lua_term},
	{"openWav",						lua_openWav},
	{"openMp3",						lua_openMp3},
	{"openMidi",					lua_openMidi},
	{"openOgg",						lua_openOgg},
	{"play",						lua_play},
	{"pause",						lua_pause},
	{"resume",						lua_resume},
	{"isPlaying",					lua_isplaying},
	{"close",						lua_closesong},
	{0, 0}
};

void luaSound_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Sound_functions, 0);
	lua_setglobal(L, "Sound");
	bool LOOP = true;
	bool NO_LOOP = false;
	BooleanRegister(L, NO_LOOP);
	BooleanRegister(L, LOOP);
}