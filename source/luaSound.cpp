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
#include <mpg123.h>
#include "include/luaplayer.h"
#include "include/audiodec/audio_decoder.h"
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

#define BUFSIZE        8192 // Max dimension of audio buffer size
#define BUFSIZE_MONO   4096 // Dimension of audio buffer files for mono tracks
#define NSAMPLES       2048 // Number of samples for output
#define AUDIO_CHANNELS 8    // PSVITA has 8 available MAIN audio channels

SceUID AudioThreads[AUDIO_CHANNELS], MicThread, Audio_Mutex, NewTrack_Mutex;
DecodedMusic* new_track = NULL;
static bool initialized = false;
static bool mpg123_inited = false;
bool availThreads[AUDIO_CHANNELS];
std::unique_ptr<AudioDecoder> audio_decoder[AUDIO_CHANNELS];
static volatile bool mustExit = false;
static uint8_t ids[] = {0, 1, 2, 3, 4, 5, 6 ,7}; // Change this if you edit AUDIO_CHANNELS macro

extern void replace_header_handle(mpg123_handle *handle);

// Audio thread code
static int audioThread(unsigned int args, void* arg){

	// Getting thread id
	uint8_t* argv = (uint8_t*)arg;
	uint8_t id = argv[0];
	
	// Initializing audio port
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, NSAMPLES, 48000, SCE_AUDIO_OUT_MODE_STEREO);
	sceAudioOutSetConfig(ch, -1, -1, (SceAudioOutMode)-1);
	
	DecodedMusic* mus;
	int old_volume;
	for (;;){
		
		// Waiting for an audio output request
		sceKernelWaitSema(Audio_Mutex, 1, NULL);
		
		// Setting thread as busy
		availThreads[id] = false;
		
		// Fetching track
		mus = new_track;
		
		// Checking if a new track is available
		if (mus == NULL){
			
			//If we enter here, we probably are in the exiting procedure
			if (mustExit){
				sceKernelSignalSema(Audio_Mutex, 1);
				sceAudioOutReleasePort(ch);
				return sceKernelExitDeleteThread(0);
			}
		
		}
		
		mus->audioThread = id;
		sceKernelSignalSema(NewTrack_Mutex, 1);
		
		// Setting audio channel volume
		int vol_stereo[] = {32767, 32767};
		sceAudioOutSetVolume(ch, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol_stereo);
		old_volume = 32767;
		
		// Initializing audio decoder
		audio_decoder[id] = AudioDecoder::Create(mus->handle, "Track");
		if (audio_decoder[id] == NULL){ // TODO: Find why this case apparently can happen
			if (mus->tempBlock){
				free(mus);
				mus = NULL;
			}else{
				mus->handle = fopen(mus->filepath,"rb");
				mus->audioThread = 0xFF;
			}
			availThreads[id] = true;
			continue; 
		}
		audio_decoder[id]->Open(mus->handle);
		audio_decoder[id]->SetLooping(mus->loop);
		audio_decoder[id]->SetFormat(48000, AudioDecoder::Format::S16, 2);
		
		// Checking resampler output mode
		int rate, chns;
		AudioDecoder::Format fmt;
		audio_decoder[id]->GetFormat(rate, fmt, chns);
		
		if (rate != 48000){ // That should not happen
			audio_decoder[id].reset();
			if (mus->tempBlock){
				free(mus);
				mus = NULL;
			}else{
				mus->handle = fopen(mus->filepath,"rb");
				mus->audioThread = 0xFF;
			}
			availThreads[id] = true;
			continue;
		}
		
		sceAudioOutSetConfig(ch, -1, -1, (SceAudioOutMode)(chns-1));
		
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
					audio_decoder[id].reset();
					free(mus->audiobuf);
					free(mus->audiobuf2);
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
					return sceKernelExitDeleteThread(0);
					
				}
			
				mus->isPlaying = !mus->isPlaying;
				mus->pauseTrigger = false;
				
			}
			
			// Check if a volume change request arrived
			if (mus->volume != old_volume){
				int vol_stereo_new[] = {mus->volume, mus->volume};
				sceAudioOutSetVolume(ch, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol_stereo_new);
				old_volume = mus->volume;
			}
			
			if (mus->isPlaying){
				
				// Check if audio playback finished
				if ((!mus->loop) && audio_decoder[id]->IsFinished()) mus->isPlaying = false;
				
				// Update audio output
				if (mus->cur_audiobuf == mus->audiobuf) mus->cur_audiobuf = mus->audiobuf2;
				else mus->cur_audiobuf = mus->audiobuf;
				audio_decoder[id]->Decode(mus->cur_audiobuf, (chns > 1) ? BUFSIZE : BUFSIZE_MONO);
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
					
				}else sceKernelDelayThread(100); // Tricky way to call a re-scheduling
				
			}
			
		}
		
	}
	
}

static int lua_init(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	
	if (!initialized){
	
		// Creating audio mutexs
		Audio_Mutex = sceKernelCreateSema("Audio Mutex", 0, 0, 1, NULL);
		NewTrack_Mutex = sceKernelCreateSema("NewTrack Mutex", 0, 1, 1, NULL);
		Mic_Mutex = sceKernelCreateSema("Mic Mutex", 0, 0, 1, NULL);
		
		// Starting audio threads
		for (int i=0;i < AUDIO_CHANNELS; i++){
			availThreads[i] = true;
			AudioThreads[i] = sceKernelCreateThread("Audio Thread", &audioThread, 0x10000100, 0x10000, 0, 0, NULL);
			int res = sceKernelStartThread(AudioThreads[i], sizeof(ids[i]), &ids[i]);
			if (res != 0) return luaL_error(L, "Failed to init an audio thread.");
		}
		
		// Starting mic thread
		MicThread = sceKernelCreateThread("Mic Thread", &micThread, 0x10000100, 0x10000, 0, 0, NULL);
		int res = sceKernelStartThread(MicThread, 0, NULL);
		
		// Loading shutter sound module
		sceSysmoduleLoadModule(SCE_SYSMODULE_SHUTTER_SOUND);
		
		initialized = true;
	}
	return 0;
}

static int lua_term(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	#endif
	
	if (initialized){
		
		// Starting exit procedure for audio threads
		mustExit = true;
		sceKernelSignalSema(Audio_Mutex, 1);
		for (int i=0;i<AUDIO_CHANNELS;i++){
			sceKernelWaitThreadEnd(AudioThreads[i], NULL, NULL);
		}
		mustExit = false;
		
		// Starting exit procedure for mic thread
		termMic = true;
		sceKernelSignalSema(Mic_Mutex, 1);
		sceKernelWaitThreadEnd(MicThread, NULL, NULL);
		termMic = false;
		
		// Deleting audio mutexes
		sceKernelDeleteSema(Audio_Mutex);
		sceKernelDeleteSema(Mic_Mutex);
		sceKernelDeleteSema(NewTrack_Mutex);
		
		// Unloading shutter sound module
		sceSysmoduleUnloadModule(SCE_SYSMODULE_SHUTTER_SOUND);
		
		initialized = false;
	}
	return 0;
}

static int lua_closesong(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	mus->closeTrigger = true;
	mus->pauseTrigger = true;
	return 0;
}

static int lua_pause(lua_State *L){
	#ifndef SKIP_ERROR_HANDLING
	int argc = lua_gettop(L);
	#endif
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	if (mus->isPlaying) mus->pauseTrigger = true;

	return 0;
}

static int lua_setvol(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	int vol = luaL_checkinteger(L, 2);
	vol = (vol < 0) ? 0 : ((vol > 32767) ? 32767 : vol);
	mus->volume = vol;
	return 0;
}

static int lua_getvol(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	lua_pushinteger(L, mus->volume);
	return 1;
}

static int lua_isplaying(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	lua_pushboolean(L,mus->isPlaying);	
	return 1;
}

static int lua_resume(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	if (!mus->isPlaying) mus->pauseTrigger = true;
	
	return 0;
}

static int lua_opensound(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	const char* path = luaL_checkstring(L, 1);
	
	// Allocating music block
	DecodedMusic* memblock = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	
	// Opening file and checking for magic
	FILE* f = fopen(path, "rb");
	#ifndef SKIP_ERROR_HANDLING
	if (f == NULL) return luaL_error(L, "file doesn't exist.");
	uint32_t magic;
	fread(&magic, 1, 4, f);	
	if (magic != 0x03334449 /* MP3 */ && magic != 0x6468544D /* MIDI */ && magic != 0x5367674F /* OGG */ && magic != 0x46464952 /* WAV */ && magic != 0x4D524F46 /* AIF */ && magic != 0x56485350 /* PSHV */){
		fclose(f);
		free(memblock);
		return luaL_error(L, "unknown audio file format.");
	}
	fseek(f, 0, SEEK_SET);
	#endif
	
	// Metadata extraction
	uint32_t half_magic = 0xDEADBEEF, info_size = 0xDEADBEEF, offset;
	uint32_t jump, chunk = 0, pos = 16;
	mpg123_handle *handle;
	mpg123_id3v1 *v1;
	mpg123_id3v2 *v2;
	uint8_t header[256];
	char info_type[7];
	int i = 0;
	strcpy(memblock->author, "");
	strcpy(memblock->title, "");
	switch(magic){
	case 0x03334449: // MP3
		if (!mpg123_inited){
			mpg123_init();
			mpg123_inited = true;
		}
		handle = mpg123_new(nullptr, nullptr);
		replace_header_handle(handle);
		if ((mpg123_open_handle(handle, f)) != MPG123_OK){
			fclose(f);
			free(memblock);
			return luaL_error(L, mpg123_strerror(handle));
		}
		mpg123_scan(handle);
		i = mpg123_meta_check(handle);
		if (i & MPG123_ID3 && mpg123_id3(handle, &v1, &v2) == MPG123_OK){
			if (v2 == nullptr){
				strcpy(memblock->author, v1->artist);
				strcpy(memblock->title, v1->title);
			}else{
				strcpy(memblock->author, v2->artist->p);
				strcpy(memblock->title, v2->title->p);
			}
		}
		mpg123_close(handle);
		mpg123_delete(handle);
		f = fopen(path, "rb");
		break;
	case 0x5367674F: // OGG
		fseek(f, 0x60, SEEK_SET);
		while (half_magic != 0x726F7603){
			i++;
			fread(&half_magic, 4, 1, f);
			fseek(f, 0x60+i, SEEK_SET);
		}
		fseek(f, 0x06, SEEK_CUR);
		fread(&info_size, 4, 1, f);
		fseek(f, info_size + 4, SEEK_CUR);
		fread(&info_size, 4, 1, f);
		while (info_size != 0x6F760501){
			offset = ftell(f);
			if (offset > 0x200) break; // Temporary patch for files without COMMENT section
			fread(&info_type, 6, 1, f);
			if (strcasecmp((const char*)&info_type, "ARTIST") == 0){
				fseek(f, 0x01, SEEK_CUR);
				fread(memblock->author, info_size - 7, 1, f);
				memblock->author[info_size - 7] = 0;
			}else if (strcasecmp((const char*)&info_type,"TITLE=") == 0){
				fread(memblock->title, info_size - 6, 1, f);
				memblock->title[info_size - 6] = 0;
			}
			fseek(f, offset + info_size, SEEK_SET);
			fread(&info_size, 4, 1, f);
		}
		fseek(f, 0, SEEK_SET);
		break;
	case 0x46464952: // WAV
		fread(header, 1, 256, f);
		while (chunk != 0x61746164){
			memcpy(&jump, &header[pos], 4);
			pos=pos+4+jump;
			memcpy(&chunk, &header[pos], 4);
			pos=pos+4;
			
			//Chunk LIST detection
			if (chunk == 0x5453494C){
				uint32_t chunk_size;
				uint32_t subchunk;
				uint32_t subchunk_size;
				uint32_t sub_pos = pos+4;
				memcpy(&subchunk, &header[sub_pos], 4);
				if (subchunk == 0x4F464E49){
					sub_pos = sub_pos+4;
					memcpy(&chunk_size, &header[pos], 4);
					while (sub_pos < (chunk_size + pos + 4)){
						memcpy(&subchunk, &header[sub_pos], 4);
						memcpy(&subchunk_size, &header[sub_pos + 4], 4);
						if (subchunk == 0x54524149){
							strncpy(memblock->author, (char*)&header[sub_pos + 8], subchunk_size);
							memblock->author[subchunk_size] = 0;
						}else if (subchunk == 0x4D414E49){
							strncpy(memblock->title, (char*)&header[sub_pos + 8], subchunk_size);
							memblock->title[subchunk_size] = 0;
						}
						sub_pos = sub_pos + 8 + subchunk_size;
						uint8_t checksum;
						memcpy(&checksum, &header[sub_pos], 1);
						if (checksum == 0) sub_pos++;
					}
				}
			}
			
		}
		fseek(f, 0, SEEK_SET);
		break;
	default:
		break;
	}
	
	// Pushing music block
	memblock->handle = f;
	memblock->isPlaying = false;
	memblock->audioThread = 0xFF;
	memblock->tempBlock = false;
	sprintf(memblock->filepath, "%s", path);
	lua_pushinteger(L,(uint32_t)memblock);
	return 1;
}

static int lua_play(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	bool loop = false;
	if (argc == 2) {
		loop = lua_toboolean(L, 2);
	}
	
	// Wait till a thread is available
	bool found = false;
	for (int i=0; i<AUDIO_CHANNELS; i++){
		found = availThreads[i];
		if (found) break;
	}
	if (!found) return 0;
	
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
	mus->volume = 32767;
	
	// Waiting till track slot is free
	sceKernelWaitSema(NewTrack_Mutex, 1, NULL);
	
	// Passing music to an audio thread
	new_track = mus;
	sceKernelSignalSema(Audio_Mutex, 1);
	
	return 0;
}

static int lua_playshutter(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	uint32_t type = (uint32_t)luaL_checkinteger(L, 1);
	#ifndef SKIP_ERROR_HANDLING
	if ((type > 2) || (type < 0)) return luaL_error(L, "invalid shutter sound id.");
	#endif
	sceShutterSoundPlay(type);
	return 0;
}

static int lua_getTitle(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	lua_pushstring(L, mus->title);
	return 1;
}

static int lua_getAuthor(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	DecodedMusic* mus = (DecodedMusic*)luaL_checkinteger(L, 1);
	lua_pushstring(L, mus->author);
	return 1;
}

//Register our Sound Functions
static const luaL_Reg Sound_functions[] = {
	{"init",         lua_init},
	{"term",         lua_term},
	{"open",         lua_opensound},
	{"play",         lua_play},
	{"playShutter",  lua_playshutter},
	{"setVolume",    lua_setvol},
	{"getVolume",    lua_getvol},
	{"getTitle",     lua_getTitle},
	{"getAuthor",    lua_getAuthor},
	{"pause",        lua_pause},
	{"resume",       lua_resume},
	{"isPlaying",    lua_isplaying},
	{"close",        lua_closesong},
	{0, 0}
};

void luaSound_init(lua_State *L) {
	uint8_t IMAGE_CAPTURE = (uint8_t)SCE_SHUTTER_SOUND_TYPE_SAVE_IMAGE;
	uint8_t VIDEO_CAPTURE_START = (uint8_t)SCE_SHUTTER_SOUND_TYPE_SAVE_VIDEO_START;
	uint8_t VIDEO_CAPTURE_END = (uint8_t)SCE_SHUTTER_SOUND_TYPE_SAVE_VIDEO_END;
	lua_newtable(L);
	luaL_setfuncs(L, Sound_functions, 0);
	lua_setglobal(L, "Sound");
	VariableRegister(L, IMAGE_CAPTURE);
	VariableRegister(L, VIDEO_CAPTURE_START);
	VariableRegister(L, VIDEO_CAPTURE_END);
}