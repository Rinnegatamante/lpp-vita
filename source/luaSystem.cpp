/*---------------------------------------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ---------------------------------------------------------------------------------------#
#------- _  -------------------  ______  _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \| |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____   _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/| | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |     | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|     |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/ -------------------------------------------------------------#
#------------------------   ______   _   ------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  ------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   -----------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  -----------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  -----------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   -----------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License -------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <lpp.nanni@gmail.com> --------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@gmail.com> ------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#- Credits : ----------------------------------------------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------#
#- All the devs involved in Rejuvenate and vita-toolchain -------------------------------------------------------------#
#- xerpi for drawing libs and for FTP server code ---------------------------------------------------------------------#
#----------------------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern "C"{
#include <png.h>
#include <libimagequant.h>
#include <vitasdk.h>
#include <taihen.h>
}
#include "include/zip.h"
#include "include/unzip.h"
#include "include/luaplayer.h"
#include "include/head.h"
#include "include/sha1.h"
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

#define FULL_EXTRACT 0
#define FILE_EXTRACT 1
#define EXTRACT_END  2

#define COPY_BUFFER_SIZE 1024 * 1024

int FORMAT_BMP = 0;
int FORMAT_PNG = 1;
int FORMAT_JPG = 2;
static int FREAD = SCE_O_RDONLY;
static int FWRITE = SCE_O_WRONLY;
static int FCREATE = SCE_O_CREAT | SCE_O_WRONLY;
static int FRDWR = SCE_O_RDWR;
static uint32_t SET = SEEK_SET;
static uint32_t CUR = SEEK_CUR;
static uint32_t END = SEEK_END;
static uint32_t AUTO_SUSPEND_TIMER = SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND;
static uint32_t SCREEN_OFF_TIMER = SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF;
static uint32_t SCREEN_DIMMING_TIMER = SCE_KERNEL_POWER_TICK_DISABLE_OLED_DIMMING;
static SceMsgDialogProgressBarParam barParam;
static SceMsgDialogUserMessageParam msgParam;
bool messageStarted = false;
static char messageText[512];
static char fname[512], ext_fname[512], read_buffer[8192];

static unzFile asyncHandle;
static char asyncDest[512];
static char asyncName[512];
static volatile uint8_t asyncMode = EXTRACT_END;
volatile int asyncResult = 1;
uint8_t async_task_num = 0;
unsigned char* asyncStrRes = NULL;
uint32_t asyncResSize = 0;

void *work_buf = nullptr;

typedef struct{
	uint32_t magic;
	uint32_t version;
	uint32_t keyTableOffset;
	uint32_t dataTableOffset;
	uint32_t indexTableEntries;
} sfo_header_t;

typedef struct{
	uint16_t keyOffset;
	uint16_t param_fmt;
	uint32_t paramLen;
	uint32_t paramMaxLen;
	uint32_t dataOffset;
} sfo_entry_t;

static void addFileToZip(zipFile zFile, const char *fname, char *src, int compression_level) {
	zip_fileinfo info = {0};
	info.tmz_date.tm_sec = info.tmz_date.tm_min = info.tmz_date.tm_hour = 0;
	info.tmz_date.tm_mday = info.tmz_date.tm_mon = info.tmz_date.tm_year = 0;
	info.dosDate = 0;
	info.internal_fa = 0;
	info.external_fa = 0;
	
	char *last_slash = src;
	char *p;
	do {
		p = last_slash + 1;
		last_slash = strstr(p, "/");
	} while(last_slash);
	char dst[256];
	sprintf(dst, "%s%s%s", fname, strlen(fname) > 0 ? "/" : "", p);
	zipOpenNewFileInZip(zFile, dst, &info, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

	FILE *f = fopen(src, "rb");
	uint8_t *data = (uint8_t*)malloc(COPY_BUFFER_SIZE);
	for (;;) {
		uint32_t len = fread(data, 1, COPY_BUFFER_SIZE, f);
		if (!len) break;
		zipWriteInFileInZip(zFile, data, len);
	}
	fclose(f);
	free(data);
	
	zipCloseFileInZip(zFile);
}

static void addDirToZip(zipFile zFile, const char *fname, char *src, int compression_level) {
	SceIoDirent g_dir;
	int fd = sceIoDopen(src);
	char path[512], path2[512];
	while (sceIoDread(fd, &g_dir) > 0) {
		if (SCE_S_ISDIR(g_dir.d_stat.st_mode)) {
			sprintf(path2, "%s/%s", src, g_dir.d_name);
			sprintf(path, "%s%s%s", fname, strlen(fname) > 0 ? "/" : "", g_dir.d_name);
			addDirToZip(zFile, path, path2, compression_level);
		} else {
			sprintf(path2, "%s/%s", src, g_dir.d_name);
			addFileToZip(zFile, fname, path2, compression_level);
		}
	}
	sceIoDclose(fd);
}

void loadPromoter() {
	uint32_t ptr[0x100] = { 0 };
	ptr[0] = 0;
	ptr[1] = (uint32_t)&ptr[0];
	uint32_t scepaf_argp[] = { 0x400000, 0xEA60, 0x40000, 0, 0 };
	sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(scepaf_argp), scepaf_argp, (SceSysmoduleOpt *)ptr);
	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
	scePromoterUtilityInit();
}

void unloadPromoter() {
	scePromoterUtilityExit();
	sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
	SceSysmoduleOpt opt;
	sceClibMemset(&opt.flags, 0, sizeof(opt));
	sceSysmoduleUnloadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, 0, NULL, &opt);
}

void recursive_mkdir(char *dir) {
	char *p = dir;
	while (p) {
		char *p2 = strstr(p, "/");
		if (p2) {
			p2[0] = 0;
			sceIoMkdir(dir, 0777);
			p = p2 + 1;
			p2[0] = '/';
		} else break;
	}
}

static int zipThread(unsigned int args, void* arg){
	unz_global_info global_info;
	unz_file_info file_info;
	uint64_t curr_file_bytes = 0;
	int num_files = 0;
	switch (asyncMode){
		case FULL_EXTRACT:
			unzGetGlobalInfo(asyncHandle, &global_info);
			unzGoToFirstFile(asyncHandle);
			num_files = global_info.number_entry;
			for (int zip_idx = 0; zip_idx < num_files; ++zip_idx) {
				unzGetCurrentFileInfo(asyncHandle, &file_info, fname, 512, NULL, 0, NULL, 0);
				sprintf(ext_fname, "%s%s", asyncDest, fname); 
				const size_t filename_length = strlen(ext_fname);
				if (ext_fname[filename_length - 1] != '/') {
					curr_file_bytes = 0;
					unzOpenCurrentFile(asyncHandle);
					recursive_mkdir(ext_fname);
					FILE *f = fopen(ext_fname, "wb");
					while (curr_file_bytes < file_info.uncompressed_size) {
						int rbytes = unzReadCurrentFile(asyncHandle, read_buffer, 8192);
						if (rbytes > 0) {
							fwrite(read_buffer, 1, rbytes, f);
							curr_file_bytes += rbytes;
						}
					}
					fclose(f);
					unzCloseCurrentFile(asyncHandle);
				}
				if ((zip_idx + 1) < num_files) unzGoToNextFile(asyncHandle);
			}
			unzClose(asyncHandle);
			asyncResult = 1;
			break;
		case FILE_EXTRACT:
			unzGetGlobalInfo(asyncHandle, &global_info);
			unzGoToFirstFile(asyncHandle);
			num_files = global_info.number_entry;
			for (int zip_idx = 0; zip_idx < num_files; ++zip_idx) {
				unzGetCurrentFileInfo(asyncHandle, &file_info, fname, 512, NULL, 0, NULL, 0);
				if (!strcmp(fname, asyncName)) {
					curr_file_bytes = 0;
					unzOpenCurrentFile(asyncHandle);
					FILE *f = fopen(asyncDest, "wb");
					while (curr_file_bytes < file_info.uncompressed_size) {
						int rbytes = unzReadCurrentFile(asyncHandle, read_buffer, 8192);
						if (rbytes > 0) {
							fwrite(read_buffer, 1, rbytes, f);
							curr_file_bytes += rbytes;
						}
					}
					fclose(f);
					unzCloseCurrentFile(asyncHandle);
					break;
				}
				if ((zip_idx + 1) < num_files) unzGoToNextFile(asyncHandle);
			}
			unzClose(asyncHandle);
			asyncResult = 1;
			break;
	}
	asyncMode = EXTRACT_END;
	async_task_num--;
	sceKernelExitDeleteThread(0);
	return 0;
}

// Taken from modoru, thanks to TheFloW
void firmware_string(char string[8], unsigned int version) {
	char a = (version >> 24) & 0xf;
	char b = (version >> 20) & 0xf;
	char c = (version >> 16) & 0xf;
	char d = (version >> 12) & 0xf;

	sceClibMemset(string, 0, 8);
	string[0] = '0' + a;
	string[1] = '.';
	string[2] = '0' + b;
	string[3] = '0' + c;
	string[4] = '\0';

	if (d) {
		string[4] = '0' + d;
		string[5] = '\0';
	}
}

// Taken from VHBB, thanks to devnoname120
static void fpkg_hmac(const uint8_t* data, unsigned int len, uint8_t hmac[16]) {
	
	SHA1_CTX ctx;
	char sha1[20];
	char buf[64];

	sha1_init(&ctx);
	sha1_update(&ctx, (BYTE*)data, len);
	sha1_final(&ctx, (BYTE*)sha1);

	sceClibMemset(buf, 0, 64);
	sceClibMemcpy(&buf[0], &sha1[4], 8);
	sceClibMemcpy(&buf[8], &sha1[4], 8);
	sceClibMemcpy(&buf[16], &sha1[12], 4);
	buf[20] = sha1[16];
	buf[21] = sha1[1];
	buf[22] = sha1[2];
	buf[23] = sha1[3];
	sceClibMemcpy(&buf[24], &buf[16], 8);

	sha1_init(&ctx);
	sha1_update(&ctx, (BYTE*)buf, 64);
	sha1_final(&ctx, (BYTE*)sha1);
	sceClibMemcpy(hmac, sha1, 16);
}

void makeHeadBin(const char *dir) {
	uint8_t hmac[16];
	uint32_t off;
	uint32_t len;
	uint32_t out;

	char head_path[256];
	char param_path[256];
	sprintf(head_path, "%s/sce_sys/package/head.bin", dir);
	sprintf(param_path, "%s/sce_sys/param.sfo", dir);
	
	SceUID fileHandle = sceIoOpen(head_path, SCE_O_RDONLY, 0777);
	if (fileHandle >= 0) {
		sceIoClose(fileHandle);
		return;
	}

	FILE* f = fopen(param_path,"rb");
	
	if (f == NULL)
		return;
	
	sfo_header_t hdr;
	fread(&hdr, sizeof(sfo_header_t), 1, f);
	
	if (hdr.magic != 0x46535000){
		fclose(f);
		return;
	}
	
	uint8_t* idx_table = (uint8_t*)malloc((sizeof(sfo_entry_t)*hdr.indexTableEntries));
	fread(idx_table, sizeof(sfo_entry_t)*hdr.indexTableEntries, 1, f);
	sfo_entry_t* entry_table = (sfo_entry_t*)idx_table;
	fseek(f, hdr.keyTableOffset, SEEK_SET);
	uint8_t* key_table = (uint8_t*)malloc(hdr.dataTableOffset - hdr.keyTableOffset);
	fread(key_table, hdr.dataTableOffset - hdr.keyTableOffset, 1, f);
	
	char titleid[12];
	char contentid[48];
	
	for (int i=0; i < hdr.indexTableEntries; i++) {
		char param_name[256];
		sprintf(param_name, "%s", (char*)&key_table[entry_table[i].keyOffset]);
			
		if (strcmp(param_name, "TITLE_ID") == 0){ // Application Title ID
			fseek(f, hdr.dataTableOffset + entry_table[i].dataOffset, SEEK_SET);
			fread(titleid, entry_table[i].paramLen, 1, f);
		} else if (strcmp(param_name, "CONTENT_ID") == 0) { // Application Content ID
			fseek(f, hdr.dataTableOffset + entry_table[i].dataOffset, SEEK_SET);
			fread(contentid, entry_table[i].paramLen, 1, f);
		}
	}

	// Free sfo buffer
	free(idx_table);
	free(key_table);

	// Allocate head.bin buffer
	uint8_t* head_bin = (uint8_t*)malloc(size_head);
	sceClibMemcpy(head_bin, head, size_head);

	// Write full title id
	char full_title_id[48];
	snprintf(full_title_id, sizeof(full_title_id), "EP9000-%s_00-0000000000000000", titleid);
	strncpy((char*)&head_bin[0x30], strlen(contentid) > 0 ? contentid : full_title_id, 48);

	// hmac of pkg header
	len = __builtin_bswap32(*(uint32_t*)&head_bin[0xD0]);
	fpkg_hmac(&head_bin[0], len, hmac);
	sceClibMemcpy(&head_bin[len], hmac, 16);

	// hmac of pkg info
	off = __builtin_bswap32(*(uint32_t*)&head_bin[0x8]);
	len = __builtin_bswap32(*(uint32_t*)&head_bin[0x10]);
	out = __builtin_bswap32(*(uint32_t*)&head_bin[0xD4]);
	fpkg_hmac(&head_bin[off], len - 64, hmac);
	sceClibMemcpy(&head_bin[out], hmac, 16);

	// hmac of everything
	len = __builtin_bswap32(*(uint32_t*)&head_bin[0xE8]);
	fpkg_hmac(&head_bin[0], len, hmac);
	sceClibMemcpy(&head_bin[len], hmac, 16);

	// Make dir
	char pkg_dir[256];
	sprintf(pkg_dir, "%s/sce_sys/package", dir);
	sceIoMkdir(pkg_dir, 0777);

	// Write head.bin
	fclose(f);
	f = fopen(head_path, "wb");
	fwrite(head_bin, 1, size_head, f);
	fclose(f);

	free(head_bin);
}

static void pushDateToTable(lua_State *L, SceDateTime date) {
	lua_pushstring(L, "year");
	lua_pushinteger(L, date.year);
	lua_settable(L, -3);
	lua_pushstring(L, "month");
	lua_pushinteger(L, date.month);
	lua_settable(L, -3);
	lua_pushstring(L, "day");
	lua_pushinteger(L, date.day);
	lua_settable(L, -3);
	lua_pushstring(L, "hour");
	lua_pushinteger(L, date.hour);
	lua_settable(L, -3);
	lua_pushstring(L, "minute");
	lua_pushinteger(L, date.minute);
	lua_settable(L, -3);
	lua_pushstring(L, "second");
	lua_pushinteger(L, date.second);
	lua_settable(L, -3);
}

static void pushStatToTable(lua_State *L, SceIoStat stat) {
	lua_newtable(L);
	lua_pushstring(L, "access_time");
	lua_newtable(L);
	pushDateToTable(L, stat.st_atime);
	lua_settable(L, -3);
	lua_pushstring(L, "creation_time");
	lua_newtable(L);
	pushDateToTable(L, stat.st_ctime);
	lua_settable(L, -3);
	lua_pushstring(L, "mod_time");
	lua_newtable(L);
	pushDateToTable(L, stat.st_mtime);
	lua_settable(L, -3);
	lua_pushstring(L, "size");
	lua_pushnumber(L, stat.st_size);
	lua_settable(L, -3);
	lua_pushstring(L, "directory");
	lua_pushboolean(L, SCE_S_ISDIR(stat.st_mode));
	lua_settable(L, -3);
}



static int lua_launch(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif
	char *file = (char*)luaL_checkstring(L,1);
	unsigned char *buffer;
	SceUID bin = sceIoOpen(file, SCE_O_RDONLY, 0777);
#ifndef SKIP_ERROR_HANDLING
	if (bin < 0) return luaL_error(L, "error opening file.");
#endif
	else sceIoClose(bin);
	sceAppMgrLoadExec(file, NULL, NULL);
	return 0;
}

static int lua_launch2(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif
	char *titleid = (char*)luaL_checkstring(L,1);
	char uri[32];
	sprintf(uri, "psgm:play?titleid=%s", titleid);
	int i;
	for (i=0;i<2;i++){
		sceKernelDelayThread(10000);
		sceAppMgrLaunchAppByUri(0xFFFFF, uri);
	}
	return 0;
}

static int lua_openfile(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	const char *file_tbo = luaL_checkstring(L, 1);
	int type = luaL_checkinteger(L, 2);
	SceUID fileHandle = sceIoOpen(file_tbo, type, 0777);
#ifndef SKIP_ERROR_HANDLING
	if (fileHandle < 0) return luaL_error(L, "cannot open requested file.");
#endif
	lua_pushinteger(L,fileHandle);
	return 1;
}

static int lua_statfile(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	const char *file = luaL_checkstring(L, 1);
	SceIoStat stat;
	if (sceIoGetstat(file, &stat) < 0) {
		lua_pushnil(L);  /* return nil */
		return 1;
	}
	pushStatToTable(L, stat);
	return 1;
}

static int lua_statfilehandle(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	SceUID file = luaL_checkinteger(L, 1);
	SceIoStat stat;
	if (sceIoGetstatByFd(file, &stat) < 0) {
		lua_pushnil(L);  /* return nil */
		return 1;
	}
	pushStatToTable(L, stat);
	return 1;
}

static int lua_readfile(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	SceUID file = luaL_checkinteger(L, 1);
	uint32_t size = luaL_checkinteger(L, 2);
	uint8_t *buffer = (uint8_t*)malloc(size + 1);
	int len = sceIoRead(file,buffer, size);
	buffer[len] = 0;
	lua_pushlstring(L,(const char*)buffer,len);
	free(buffer);
	return 1;
}

static int lua_writefile(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	SceUID fileHandle = luaL_checkinteger(L, 1);
	const char *text = luaL_checkstring(L, 2);
	int size = luaL_checknumber(L, 3);
	sceIoWrite(fileHandle, text, size);
	return 0;
}

static int lua_closefile(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	SceUID fileHandle = luaL_checkinteger(L, 1);
	sceIoClose(fileHandle);
	return 0;
}

static int lua_seekfile(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	SceUID fileHandle = luaL_checkinteger(L, 1);
	int pos = luaL_checkinteger(L, 2);
	uint32_t type = luaL_checkinteger(L, 3);
	sceIoLseek(fileHandle, pos, type);	
	return 0;
}

static int lua_sizefile(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	SceUID fileHandle = luaL_checkinteger(L, 1);
	uint32_t cur_off = sceIoLseek(fileHandle, 0, SEEK_CUR);
	uint32_t size = sceIoLseek(fileHandle, 0, SEEK_END);
	sceIoLseek(fileHandle, cur_off, SEEK_SET);
	lua_pushinteger(L, size);
	return 1;
}

static int lua_checkexist(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	const char *file_tbo = luaL_checkstring(L, 1);
	SceUID fileHandle = sceIoOpen(file_tbo, SCE_O_RDONLY, 0777);
	if (fileHandle < 0) lua_pushboolean(L, false);
	else{
		sceIoClose(fileHandle);
		lua_pushboolean(L,true);
	}
	return 1;
}

static int lua_checkexist2(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	const char *dir = luaL_checkstring(L, 1);
	SceUID fd = sceIoDopen(dir);
	if (fd < 0) lua_pushboolean(L, false);
	else{
		sceIoDclose(fd);
		lua_pushboolean(L,true);
	}
	return 1;
}

static int lua_rename(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	const char *old_file = luaL_checkstring(L, 1);
	const char *new_file = luaL_checkstring(L, 2);
	sceIoRename(old_file, new_file);
	return 0;
}

static int lua_copy(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	const char *old_file = luaL_checkstring(L, 1);
	const char *new_file = luaL_checkstring(L, 2);
	FILE *f = fopen(old_file, "rb");
#ifndef SKIP_ERROR_HANDLING
	if (!f) return luaL_error(L, "the file doesn't exist");
#endif
	FILE *f2 = fopen(new_file, "wb");
	uint8_t *data = (uint8_t*)malloc(COPY_BUFFER_SIZE);
	for (;;) {
		uint32_t size = fread(data, 1, COPY_BUFFER_SIZE, f);
		if (size) {
			fwrite(data, 1, size, f2);
		} else {
			break;
		}
	}
	fclose(f);
	fclose(f2);
	free(data);
	return 0;
}

static int lua_removef(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	const char *old_file = luaL_checkstring(L, 1);
	sceIoRemove(old_file);
	return 0;
}

static int lua_removef2(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	const char *old_file = luaL_checkstring(L, 1);
	sceIoRmdir(old_file);
	return 0;
}

static int lua_newdir(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	const char *newdir = luaL_checkstring(L, 1);
	sceIoMkdir(newdir, 0777);
	return 0;
}

static int lua_exit(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	char stringbuffer[256];
	strcpy(stringbuffer,"lpp_shutdown");
	luaL_dostring(L, "collectgarbage()");
	return luaL_error(L, stringbuffer); //Fake LUA error
}

static int lua_wait(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int microsecs = luaL_checkinteger(L, 1);
	sceKernelDelayThread(microsecs);
	return 0;
}

static int lua_screenshot(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 2 && argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	const char *filename = luaL_checkstring(L, 1);
	int format = (argc > 1) ? luaL_checkinteger(L, 2) : FORMAT_BMP;
	int ratio = (argc == 3) ? luaL_checkinteger(L, 3) : 127;
	SceDisplayFrameBuf param;
	param.size = sizeof(SceDisplayFrameBuf);
	sceDisplayWaitVblankStart();
	sceDisplayGetFrameBuf(&param, SCE_DISPLAY_SETBUF_NEXTFRAME);
	if (format == FORMAT_BMP) {
		SceUID fd = sceIoOpen(filename, SCE_O_CREAT|SCE_O_WRONLY|SCE_O_TRUNC, 0777);
		uint8_t *bmp_content = (uint8_t*)malloc(((param.pitch*param.height)<<2)+0x36);
		sceClibMemset(bmp_content, 0, 0x36);
		*(uint16_t*)&bmp_content[0x0] = 0x4D42;
		*(uint32_t*)&bmp_content[0x2] = ((param.pitch*param.height)<<2)+0x36;
		*(uint32_t*)&bmp_content[0xA] = 0x36;
		*(uint32_t*)&bmp_content[0xE] = 0x28;
		*(uint32_t*)&bmp_content[0x12] = param.pitch;
		*(uint32_t*)&bmp_content[0x16] = param.height;
		*(uint32_t*)&bmp_content[0x1A] = 0x00200001;
		*(uint32_t*)&bmp_content[0x22] = ((param.pitch*param.height)<<2);
		int x, y;
		uint32_t* buffer = (uint32_t*)&bmp_content[0x36];
		uint32_t* framebuf = (uint32_t*)param.base;
		for (y = 0; y<param.height; y++){
			for (x = 0; x<param.pitch; x++){
				buffer[x+y*param.pitch] = framebuf[x+(param.height-y)*param.pitch];
				uint8_t *clr = (uint8_t*)&buffer[x+y*param.pitch];
				uint8_t r = clr[0];
				clr[0] = clr[2];
				clr[2] = r;
			}
		}
		sceIoWrite(fd, bmp_content, ((param.pitch*param.height)<<2)+0x36);
		free(bmp_content);
		sceIoClose(fd);
	} else if (format == FORMAT_JPG) {
		SceUID fd = sceIoOpen(filename, SCE_O_CREAT|SCE_O_WRONLY|SCE_O_TRUNC, 0777);
		uint32_t in_size = ALIGN((param.width * param.height)<<1, 256);
		uint32_t out_size = ALIGN(param.width * param.height, 256);
		uint32_t buf_size = ALIGN(in_size + out_size, 0x40000);
		SceUID memblock = sceKernelAllocMemBlock("encoderBuffer", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, buf_size, NULL);
		void *buf_addr = NULL;
		sceKernelGetMemBlockBase(memblock, &buf_addr);
		SceJpegEncoderContext context = malloc(sceJpegEncoderGetContextSize());
		sceJpegEncoderInit(context, param.width, param.height, (SceJpegEncoderPixelFormat)(SCE_JPEGENC_PIXELFORMAT_YCBCR420 | SCE_JPEGENC_PIXELFORMAT_CSC_ARGB_YCBCR), (void*)((uint32_t)buf_addr + in_size), out_size);
		sceJpegEncoderSetValidRegion(context, param.width, param.height);
		sceJpegEncoderSetCompressionRatio(context, ratio);
		sceJpegEncoderSetOutputAddr(context, (void*)((uint32_t)buf_addr + in_size), out_size);
		sceJpegEncoderCsc(context, buf_addr, param.base, param.pitch, SCE_JPEGENC_PIXELFORMAT_ARGB8888);
		int filesize = sceJpegEncoderEncode(context, buf_addr);
		sceIoWrite(fd, (void*)((uint32_t)buf_addr + in_size), filesize);
		sceJpegEncoderEnd(context);
		free(context);
		sceKernelFreeMemBlock(memblock);
		sceIoClose(fd);
	} else if (format == FORMAT_PNG) {
		FILE *fh = fopen(filename, "wb");
		uint8_t *raw_data = (uint8_t*)malloc((param.width*param.height)<<2);
		uint32_t *buffer = (uint32_t*)raw_data;
		uint32_t *framebuf = (uint32_t*)param.base;
		int y;
		for (y = 0; y < 544; y++) {
			memcpy(buffer, framebuf, 960 * 4);
			buffer += 960;
			framebuf += param.pitch;
		}
		liq_attr *handle = liq_attr_create();
		liq_image *input_image = liq_image_create_rgba(handle, raw_data, 960, 544, 0);
		liq_result *res;
		liq_image_quantize(input_image, handle, &res);
		uint8_t *quant_raw = (uint8_t*)malloc(960 * 544);
		liq_set_dithering_level(res, 1.0);
		liq_write_remapped_image(res, input_image, quant_raw, 960 * 544);
		const liq_palette *palette = liq_get_palette(res);
		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);;
		png_infop info_ptr = png_create_info_struct(png_ptr);
		setjmp(png_jmpbuf(png_ptr));
		png_init_io(png_ptr, fh);
		png_set_IHDR(png_ptr, info_ptr, 960, 544,
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
		for (y = 0; y < 544; y++) {
			png_write_row(png_ptr, &quant_raw[y * 960]);
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

static int lua_dir(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0 && argc != 1) return luaL_error(L, "System.listDirectory([path]) takes zero or one argument");
#endif
	const char *path = "";
	if (argc == 0) path = "";
	else path = luaL_checkstring(L, 1);
	int fd = sceIoDopen(path);
	if (fd < 0) {
		lua_pushnil(L);  /* return nil */
		return 1;
	}
	lua_newtable(L);
	int i = 1;
	SceIoDirent g_dir;
	while (sceIoDread(fd, &g_dir) > 0) {
		lua_pushnumber(L, i++);  /* push key for file entry */
		lua_newtable(L);
		lua_pushstring(L, "name");
		lua_pushstring(L, g_dir.d_name);
		lua_settable(L, -3);
		lua_pushstring(L, "size");
		lua_pushnumber(L, g_dir.d_stat.st_size);
		lua_settable(L, -3);
		lua_pushstring(L, "directory");
		lua_pushboolean(L, SCE_S_ISDIR(g_dir.d_stat.st_mode));
		lua_settable(L, -3);
		lua_settable(L, -3);
	}
	sceIoDclose(fd);
	return 1;  /* table is already on top */
}

static int lua_charging(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushboolean(L, scePowerIsBatteryCharging());
	return 1;
}

static int lua_percent(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetBatteryLifePercent());
	return 1;
}

static int lua_lifetime(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetBatteryLifeTime());
	return 1;
}

static int lua_voltbatt(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetBatteryVolt());
	return 1;
}

static int lua_healthbatt(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetBatterySOH());
	return 1;
}

static int lua_cyclebatt(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetBatteryCycleCount());
	return 1;
}

static int lua_tempbatt(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetBatteryTemp() / 100);
	return 1;
}

static int lua_fullbatt(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetBatteryFullCapacity());
	return 1;
}

static int lua_remainbatt(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetBatteryRemainCapacity());
	return 1;
}

static int lua_tickpower(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int tmr = luaL_checkinteger(L, 1);
	sceKernelPowerTick((SceKernelPowerTickType)tmr);
	return 0;
}

static int lua_offpower(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int tmr = luaL_checkinteger(L, 1);
	sceKernelPowerLock((SceKernelPowerTickType)tmr);
	return 0;
}

static int lua_onpower(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int tmr = luaL_checkinteger(L, 1);
	sceKernelPowerUnlock((SceKernelPowerTickType)tmr);
	return 0;
}

static int lua_setcpu(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int freq = luaL_checkinteger(L, 1);
	scePowerSetArmClockFrequency(freq);
	return 0;
}

static int lua_setbus(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int freq = luaL_checkinteger(L, 1);
	scePowerSetBusClockFrequency(freq);
	return 0;
}

static int lua_setgpu(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int freq = luaL_checkinteger(L, 1);
	scePowerSetGpuClockFrequency(freq);
	return 0;
}

static int lua_setgpu2(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int freq = luaL_checkinteger(L, 1);
	scePowerSetGpuXbarClockFrequency(freq);
	return 0;
}

static int lua_getcpu(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetArmClockFrequency());
	return 1;
}

static int lua_getbus(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetBusClockFrequency());
	return 1;
}

static int lua_getgpu(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetGpuClockFrequency());
	return 1;
}

static int lua_getgpu2(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L, scePowerGetGpuXbarClockFrequency());
	return 1;
}

static int lua_gettime(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);
	lua_pushinteger(L,time.hour);
	lua_pushinteger(L,time.minute);
	lua_pushinteger(L,time.second);
	return 3;
}

static int lua_getdate(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	SceDateTime time;
	sceRtcGetCurrentClockLocalTime(&time);
	lua_pushinteger(L, sceRtcGetDayOfWeek(time.year, time.month, time.day));
	lua_pushinteger(L,time.day);
	lua_pushinteger(L,time.month);
	lua_pushinteger(L,time.year);
	return 4;
}

static int lua_nickname(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	SceChar8 nick[SCE_SYSTEM_PARAM_USERNAME_MAXSIZE];
	sceAppUtilSystemParamGetString(SCE_SYSTEM_PARAM_ID_USERNAME, nick, SCE_SYSTEM_PARAM_USERNAME_MAXSIZE);
	lua_pushstring(L,(char*)nick);
	return 1;
}

static int lua_lang(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	int lang;
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &lang);
	lua_pushinteger(L,lang);
	return 1;
}

static int lua_title(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	char title[256];
	sceAppMgrAppParamGetString(0, 9, title , 256);
	lua_pushstring(L,title);
	return 1;
}

static int lua_titleid(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	char title[16];
	sceAppMgrAppParamGetString(0, 12, title , 256);
	lua_pushstring(L,title);
	return 1;
}

static int lua_model(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushinteger(L,sceKernelGetModel());
	return 1;
}

static int lua_ZipExtract(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if(argc != 2) return luaL_error(L, "wrong number of arguments.");
#endif
	const char* FileToExtract = luaL_checkstring(L, 1);
	const char* DirTe = luaL_checkstring(L, 2);
	char DirToExtract[512];
	if (DirTe[strlen(DirTe)-1] != '/')
		sprintf(DirToExtract, "%s/", DirTe);
	else
		strcpy(DirToExtract, DirTe);
	sceIoMkdir(DirToExtract, 0777);
	unz_global_info global_info;
	unz_file_info file_info;
	unzFile zipfile = unzOpen(FileToExtract);
	unzGetGlobalInfo(zipfile, &global_info);
	unzGoToFirstFile(zipfile);
	uint64_t curr_file_bytes = 0;
	int num_files = global_info.number_entry;
	for (int zip_idx = 0; zip_idx < num_files; ++zip_idx) {
		unzGetCurrentFileInfo(zipfile, &file_info, fname, 512, NULL, 0, NULL, 0);
		sprintf(ext_fname, "%s%s", DirToExtract, fname); 
		const size_t filename_length = strlen(ext_fname);
		if (ext_fname[filename_length - 1] != '/') {
			curr_file_bytes = 0;
			unzOpenCurrentFile(zipfile);
			recursive_mkdir(ext_fname);
			FILE *f = fopen(ext_fname, "wb");
			while (curr_file_bytes < file_info.uncompressed_size) {
				int rbytes = unzReadCurrentFile(zipfile, read_buffer, 8192);
				if (rbytes > 0) {
					fwrite(read_buffer, 1, rbytes, f);
					curr_file_bytes += rbytes;
				}
			}
			fclose(f);
			unzCloseCurrentFile(zipfile);
		}
		if ((zip_idx + 1) < num_files) unzGoToNextFile(zipfile);
	}
	unzClose(zipfile);
	return 0;
}

static int lua_zip(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if(argc != 2 && argc != 3) return luaL_error(L, "wrong number of arguments.");
#endif
	char *ToCompress = luaL_checkstring(L, 1);
	const char *FileName = luaL_checkstring(L, 2);
	int compression_level = Z_DEFAULT_COMPRESSION;
	if (argc == 3) compression_level = luaL_checkinteger(L, 3);
	zipFile zFile = zipOpen(FileName, APPEND_STATUS_CREATE);
	FILE *f = fopen(ToCompress, "r");
	if (f) {
		fclose(f);
		addFileToZip(zFile, "", ToCompress, compression_level);
	} else {
		addDirToZip(zFile, "", ToCompress, compression_level);
	}
	zipClose(zFile, nullptr);
	return 0;
}

static int lua_addzip(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if(argc != 3 && argc != 4) return luaL_error(L, "wrong number of arguments.");
#endif
	char *ToCompress = luaL_checkstring(L, 1);
	const char *FileName = luaL_checkstring(L, 2);
	char *Parent = luaL_checkstring(L, 3);
	int compression_level = Z_DEFAULT_COMPRESSION;
	if (argc == 4) compression_level = luaL_checkinteger(L, 4);
	zipFile zFile = zipOpen(FileName, APPEND_STATUS_ADDINZIP);
	FILE *f = fopen(ToCompress, "r");
	if (f) {
		fclose(f);
		addFileToZip(zFile, Parent, ToCompress, compression_level);
	} else {
		addDirToZip(zFile, Parent, ToCompress, compression_level);
	}
	zipClose(zFile, nullptr);
	return 0;
}

static int lua_ZipExtractAsync(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments.");
	if (async_task_num == ASYNC_TASKS_MAX) return luaL_error(L, "cannot start more async tasks.");
#endif
	const char* FileToExtract = luaL_checkstring(L, 1);
	const char* DirTe = luaL_checkstring(L, 2);
	if (DirTe[strlen(DirTe)-1] != '/')
		sprintf(asyncDest, "%s/", DirTe);
	else
		strcpy(asyncDest, DirTe);
	sceIoMkdir(asyncDest, 0777);
	asyncHandle = unzOpen(FileToExtract);
	asyncMode = FULL_EXTRACT;
	async_task_num++;
	SceUID thd = sceKernelCreateThread("Zip Extract Thread", &zipThread, 0x10000100, 0x100000, 0, 0, NULL);
	if (thd < 0)
	{
		asyncResult = -1;
		return 0;
	}
	asyncResult = 0;
	sceKernelStartThread(thd, 0, NULL);
	return 0;
}

static int lua_getfilefromzip(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if(argc != 3) return luaL_error(L, "wrong number of arguments.");
#endif
	const char* FileToExtract = luaL_checkstring(L, 1);
	const char* FileToExtract2 = luaL_checkstring(L, 2);
	const char* Dest = luaL_checkstring(L, 3);
	unz_global_info global_info;
	unz_file_info file_info;
	unzFile zipfile = unzOpen(FileToExtract);
	unzGetGlobalInfo(zipfile, &global_info);
	unzGoToFirstFile(zipfile);
	uint64_t curr_file_bytes = 0;
	int num_files = global_info.number_entry;
	for (int zip_idx = 0; zip_idx < num_files; ++zip_idx) {
		unzGetCurrentFileInfo(zipfile, &file_info, fname, 512, NULL, 0, NULL, 0);
		if (!strcmp(fname, FileToExtract2)) {
			curr_file_bytes = 0;
			unzOpenCurrentFile(zipfile);
			FILE *f = fopen(Dest, "wb");
			while (curr_file_bytes < file_info.uncompressed_size) {
				int rbytes = unzReadCurrentFile(zipfile, read_buffer, 8192);
				if (rbytes > 0) {
					fwrite(read_buffer, 1, rbytes, f);
					curr_file_bytes += rbytes;
				}
			}
			fclose(f);
			unzCloseCurrentFile(zipfile);
			break;
		}
		if ((zip_idx + 1) < num_files) unzGoToNextFile(zipfile);
	}
	unzClose(zipfile);
	return 0;
}

static int lua_getfilefromzipasync(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if(argc != 3) return luaL_error(L, "wrong number of arguments.");
	if (async_task_num == ASYNC_TASKS_MAX) return luaL_error(L, "cannot start more async tasks.");
#endif
	const char* FileToExtract = luaL_checkstring(L, 1);
	const char* FileToExtract2 = luaL_checkstring(L, 2);
	const char* Dest = luaL_checkstring(L, 3);
	asyncHandle = unzOpen(FileToExtract);
	asyncMode = FILE_EXTRACT;
	sprintf(asyncDest, Dest);
	sprintf(asyncName, FileToExtract2);
	async_task_num++;
	SceUID thd = sceKernelCreateThread("Zip Extract Thread", &zipThread, 0x10000100, 0x100000, 0, 0, NULL);
	if (thd < 0)
	{
		asyncResult = -1;
		return 0;
	}
	asyncResult = 0;
	sceKernelStartThread(thd, 0, NULL);
	return 0;
}

static int lua_getasyncstate(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if(argc != 0) return luaL_error(L, "wrong number of arguments.");
#endif
	lua_pushinteger(L, asyncResult);
	return 1;
}

static int lua_getasyncres(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if(argc != 0) return luaL_error(L, "wrong number of arguments.");
#endif
	if (asyncStrRes != NULL){
		lua_pushlstring(L,(const char*)asyncStrRes,asyncResSize);
		free(asyncStrRes);
		asyncStrRes = NULL;
		return 1;
	}else return 0;
}

static int lua_extractsfo(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if(argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif
	const char* file = luaL_checkstring(L, 1);
	FILE* f = fopen(file,"rb");
	if (f == NULL) return luaL_error(L, "error opening SFO file.");
	else{
		sfo_header_t hdr;
		fread(&hdr, sizeof(sfo_header_t), 1, f);
		if (hdr.magic != 0x46535000){
			fclose(f);
			return luaL_error(L, "SFO file is corrupted.");
		}
		uint8_t* idx_table = (uint8_t*)malloc((sizeof(sfo_entry_t)*hdr.indexTableEntries));
		fread(idx_table, sizeof(sfo_entry_t)*hdr.indexTableEntries, 1, f);
		sfo_entry_t* entry_table = (sfo_entry_t*)idx_table;
		fseek(f, hdr.keyTableOffset, SEEK_SET);
		uint8_t* key_table = (uint8_t*)malloc(hdr.dataTableOffset - hdr.keyTableOffset);
		fread(key_table, hdr.dataTableOffset - hdr.keyTableOffset, 1, f);
		lua_newtable(L);
		uint8_t return_indexes = 0;
		for (int i=0; i < hdr.indexTableEntries; i++){
			char param_name[256];
			sprintf(param_name, "%s", (char*)&key_table[entry_table[i].keyOffset]);
			
			// Returning only relevant info
			if (strcmp(param_name, "APP_VER") == 0){ // Application Version
				fseek(f, hdr.dataTableOffset + entry_table[i].dataOffset, SEEK_SET);
				lua_pushstring(L, "version");
				char ver[0x08];
				fread(ver, entry_table[i].paramLen, 1, f);
				lua_pushstring(L, ver);
				lua_settable(L, -3);
				return_indexes++;
			}else if (strcmp(param_name, "STITLE") == 0){ // Application Title
				fseek(f, hdr.dataTableOffset + entry_table[i].dataOffset, SEEK_SET);
				lua_pushstring(L, "title");
				char title[0x80];
				fread(title, entry_table[i].paramLen > 1 ? entry_table[i].paramLen : (entry_table[i+1].dataOffset - entry_table[i].dataOffset), 1, f);
				lua_pushstring(L, title);
				lua_settable(L, -3);
				return_indexes++;
			}else if (strcmp(param_name, "CATEGORY") == 0){ // Application Category
				fseek(f, hdr.dataTableOffset + entry_table[i].dataOffset, SEEK_SET);
				lua_pushstring(L, "category");
				char category[0x04];
				fread(category, entry_table[i].paramLen, 1, f);
				lua_pushstring(L, category);
				lua_settable(L, -3);
				return_indexes++;
			}else if (strcmp(param_name, "TITLE_ID") == 0){ // Application Title ID
				fseek(f, hdr.dataTableOffset + entry_table[i].dataOffset, SEEK_SET);
				lua_pushstring(L, "titleid");
				char id[0x0C];
				fread(id, entry_table[i].paramLen, 1, f);
				lua_pushstring(L, id);
				lua_settable(L, -3);
				return_indexes++;
			}
			
		}
		fclose(f);
		if (return_indexes == 0) lua_settable(L, -3);
		free(idx_table);
		free(key_table);
		return 1;
	}
}

static int lua_executeuri(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	const char *uri_string = luaL_checkstring(L, 1);
	sceAppMgrLaunchAppByUri(0xFFFFF, (char*)uri_string);
	return 0;
}

static int lua_reboot(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	scePowerRequestColdReset();
	return 0;
}

static int lua_shutdown(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	scePowerRequestStandby();
	return 0;
}

static int lua_standby(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	scePowerRequestSuspend();
	return 0;
}

static int lua_issafe(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushboolean(L, !unsafe_mode);
	return 1;
}

static int lua_setmsg(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (messageStarted) return luaL_error(L, "cannot start multiple message instances.");
	if (argc != 3 && argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	const char* text = luaL_checkstring(L, 1);
	bool progressbar = lua_toboolean(L, 2);
	int buttons = 0;
	if (argc == 3) buttons = luaL_checkinteger(L, 3);
#ifndef SKIP_ERROR_HANDLING
	if (buttons > 4 || buttons < 0) return luaL_error(L, "invalid buttons type argument.");
#endif
	sprintf(messageText, text);
	SceMsgDialogParam param;
	sceMsgDialogParamInit(&param);
	if (progressbar){
		param.mode = SCE_MSG_DIALOG_MODE_PROGRESS_BAR;
		sceClibMemset(&barParam, 0, sizeof(SceMsgDialogProgressBarParam));
		barParam.msg = (const SceChar8*)messageText;
		barParam.barType = SCE_MSG_DIALOG_PROGRESSBAR_TYPE_PERCENTAGE;
		param.progBarParam = &barParam;
	}else{
		param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
		sceClibMemset(&msgParam, 0, sizeof(SceMsgDialogUserMessageParam));
		msgParam.msg = (const SceChar8*)messageText;
		msgParam.buttonType = buttons;
		param.userMsgParam = &msgParam;
	}
	if (sceMsgDialogInit(&param) < 0) return luaL_error(L, "an error occurred when starting message instance.");
	messageStarted = true;
	return 0;
}

static int lua_getmsg(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	SceCommonDialogStatus status = sceMsgDialogGetStatus();
	if (!messageStarted) status = SCE_COMMON_DIALOG_STATUS_FINISHED;
	if (status == SCE_COMMON_DIALOG_STATUS_FINISHED) {
		SceMsgDialogResult result;
		sceClibMemset(&result, 0, sizeof(SceMsgDialogResult));
		sceMsgDialogGetResult(&result);
		if (result.buttonId == SCE_MSG_DIALOG_BUTTON_ID_NO) status = (SceCommonDialogStatus)3; // CANCELED status, look at luaKeyboard.cpp
		sceMsgDialogTerm();
		messageStarted = false;
	}else status = SCE_COMMON_DIALOG_STATUS_RUNNING;
	lua_pushinteger(L, (uint32_t)status);
	return 1;
}

static int lua_setprogmsg(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (!messageStarted) return luaL_error(L, "no message instances available.");
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	const char* msg = luaL_checkstring(L, 1);
	sceMsgDialogProgressBarSetMsg(SCE_MSG_DIALOG_PROGRESSBAR_TARGET_BAR_DEFAULT, (const SceChar8*)msg);
	return 0;
}

static int lua_setprogbar(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (!messageStarted) return luaL_error(L, "no message instances available.");
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int val = luaL_checkinteger(L, 1);
	sceMsgDialogProgressBarSetValue(SCE_MSG_DIALOG_PROGRESSBAR_TARGET_BAR_DEFAULT, val);
	return 0;
}

static int lua_closemsg(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (!messageStarted) return luaL_error(L, "no message instances available.");
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	sceMsgDialogClose();
	return 0;
}

static int lua_getpsid(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif	
	SceKernelOpenPsId id;
	sceKernelGetOpenPsId(&id);
	char psid[64];
	sprintf(psid, "%02X", id.id[0]);
	for (int i = 1; i < 16; i++){
		sprintf(&psid[2 + ((i-1) * 3)], ":%02X", id.id[i]);
	}
	lua_pushstring(L, psid);
	return 1;
}

static int lua_freespace(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	uint64_t free_storage = 0;
	uint64_t dummy;
	char *dev_name = luaL_checkstring(L, 1);
	SceIoDevInfo info;
	sceClibMemset(&info, 0, sizeof(SceIoDevInfo));
	int res = sceIoDevctl(dev_name, 0x3001, NULL, 0, &info, sizeof(SceIoDevInfo));
	if (res >= 0) free_storage = info.free_size;
	else sceAppMgrGetDevInfo(dev_name, &dummy, &free_storage);
	lua_pushnumber(L, free_storage);
	return 1;
}

static int lua_totalspace(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	uint64_t total_storage = 0;
	uint64_t dummy;
	char *dev_name = luaL_checkstring(L, 1);
	SceIoDevInfo info;
	sceClibMemset(&info, 0, sizeof(SceIoDevInfo));
	int res = sceIoDevctl(dev_name, 0x3001, NULL, 0, &info, sizeof(SceIoDevInfo));
	if (res >= 0) total_storage = info.max_size;
	else sceAppMgrGetDevInfo(dev_name, &total_storage, &dummy);
	lua_pushnumber(L, total_storage);
	return 1;
}

static int lua_firmware(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char fw_str[8];
	SceKernelFwInfo info;
	info.size = sizeof(SceKernelFwInfo);
	_vshSblGetSystemSwVersion(&info);
	firmware_string(fw_str, info.version);
	lua_pushstring(L, fw_str);
	return 1;
}

static int lua_firmware2(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	char fw_str[8];
	SceKernelFwInfo info;
	info.size = sizeof(SceKernelFwInfo);
	sceKernelGetSystemSwVersion(&info);
	firmware_string(fw_str, info.version);
	lua_pushstring(L, fw_str);
	return 1;
}

static int lua_firmware3(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char fw_str[8];
	int ver;
	_vshSblAimgrGetSMI(&ver);
	firmware_string(fw_str, ver);
	lua_pushstring(L, fw_str);
	return 1;
}

static int lua_unmount(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	int idx = luaL_checkinteger(L, 1);
	vshIoUmount(idx, 1, 0, 0);
	return 0;
}

static int lua_mount(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	int idx = luaL_checkinteger(L, 1);
	int perm = luaL_checkinteger(L, 2);
	if (!work_buf) {
		work_buf = malloc(0x100);
		sceClibMemset(work_buf, 0, 0x100);
	}
	_vshIoMount(idx, 0, perm, work_buf);
	return 0;
}

static int lua_promote(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char *dir = luaL_checkstring(L, 1);
	
	loadPromoter();
	makeHeadBin(dir);
	scePromoterUtilityPromotePkg(dir, 0);
	
	int state = 0;
	do {
		int ret = scePromoterUtilityGetState(&state);
		if (ret < 0)
			break;
		sceKernelDelayThread(150 * 1000);
	} while (state);
	unloadPromoter();

	return 0;
}

static int lua_demote(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char *titleid = luaL_checkstring(L, 1);
	
	loadPromoter();
	scePromoterUtilityDeletePkg(titleid);
	
	int state = 0;
	do {
		int ret = scePromoterUtilityGetState(&state);
		if (ret < 0)
			break;
		sceKernelDelayThread(150 * 1000);
	} while (state);
	unloadPromoter();
	
	return 0;
}

static int lua_isappin(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char *titleid = luaL_checkstring(L, 1);
	
	loadPromoter();
	int res;
	lua_pushboolean(L, !scePromoterUtilityCheckExist(titleid, &res));
	unloadPromoter();
	return 1;
}

static int lua_bootparams(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char bootparams[1024];
	bootparams[0] = 0;
	sceAppMgrGetAppParam(bootparams);
	lua_pushstring(L, bootparams);
	return 1;
}

static int lua_loadplugin(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char *fname = luaL_checkstring(L, 1);
	SceUID modid = sceKernelLoadStartModule(fname, 0, NULL, 0, NULL, NULL);
	lua_pushinteger(L, modid);
	return 1;
}

static int lua_loadkplugin(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char *fname = luaL_checkstring(L, 1);
	SceUID modid = taiLoadKernelModule(fname, 0, NULL);
    if (modid >= 0) {
		int res = taiStartKernelModule(modid, 0, NULL, 0, NULL, NULL);
		if (res < 0)
			taiStopUnloadKernelModule(modid, 0, NULL, 0, NULL, NULL);
	}
	lua_pushinteger(L, modid);
	return 1;
}

static int lua_unloadplugin(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	SceUID modid = luaL_checkinteger(L, 1);
	sceKernelStopUnloadModule(modid, 0, NULL, 0, NULL, NULL);
	return 0;
}

static int lua_unloadkplugin(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	SceUID modid = luaL_checkinteger(L, 1);
	taiStopUnloadKernelModule(modid, 0, NULL, 0, NULL, NULL);
	return 0;
}

static int lua_unmountvirtual(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!unsafe_mode) return luaL_error(L, "this function requires unsafe mode");
#endif
	char *fname = luaL_checkstring(L, 1);
	sceAppMgrUmount(fname);
	return 0;
}

//Register our System Functions
static const luaL_Reg System_functions[] = {
  {"openFile",                  lua_openfile},
  {"readFile",                  lua_readfile},
  {"writeFile",                 lua_writefile},
  {"closeFile",                 lua_closefile},  
  {"seekFile",                  lua_seekfile},  
  {"sizeFile",                  lua_sizefile},
  {"statFile",                  lua_statfile},
  {"statOpenedFile",            lua_statfilehandle},
  {"doesFileExist",             lua_checkexist},
  {"doesDirExist",              lua_checkexist2},
  {"exit",                      lua_exit},
  {"rename",                    lua_rename},
  {"copyFile",                  lua_copy},
  {"deleteFile",                lua_removef},
  {"deleteDirectory",           lua_removef2},
  {"createDirectory",           lua_newdir},
  {"listDirectory",             lua_dir},
  {"wait",                      lua_wait},
  {"isBatteryCharging",         lua_charging},
  {"getBatteryPercentage",      lua_percent},
  {"getBatteryLife",            lua_lifetime},
  {"getBatteryCapacity",        lua_remainbatt},
  {"getBatteryFullCapacity",    lua_fullbatt},
  {"getBatteryTemp",            lua_tempbatt},
  {"getBatteryVolt",            lua_voltbatt},
  {"getBatteryHealth",          lua_healthbatt},
  {"getBatteryCycles",          lua_cyclebatt},
  {"disableTimer",              lua_offpower},
  {"enableTimer",               lua_onpower},
  {"resetTimer",                lua_tickpower},
  {"setCpuSpeed",               lua_setcpu},
  {"getCpuSpeed",               lua_getcpu},
  {"setBusSpeed",               lua_setbus},
  {"getBusSpeed",               lua_getbus},
  {"setGpuSpeed",               lua_setgpu},
  {"getGpuSpeed",               lua_getgpu},
  {"setGpuXbarSpeed",           lua_setgpu2},
  {"getGpuXbarSpeed",           lua_getgpu2},
  {"launchEboot",               lua_launch},
  {"launchApp",                 lua_launch2},
  {"getTime",                   lua_gettime},
  {"getDate",                   lua_getdate},
  {"getUsername",               lua_nickname},
  {"getLanguage",               lua_lang},
  {"getModel",                  lua_model},
  {"getTitle",                  lua_title},
  {"getTitleID",                lua_titleid},
  {"extractSfo",                lua_extractsfo},
  {"extractZip",                lua_ZipExtract},
  {"compressZip",               lua_zip},
  {"addToZip",                  lua_addzip},
  {"extractZipAsync",           lua_ZipExtractAsync},
  {"extractFromZip",            lua_getfilefromzip},
  {"extractFromZipAsync",       lua_getfilefromzipasync},
  {"takeScreenshot",            lua_screenshot},
  {"executeUri",                lua_executeuri},
  {"reboot",                    lua_reboot},
  {"shutdown",                  lua_shutdown},
  {"standby",                   lua_standby},
  {"isSafeMode",                lua_issafe},
  {"setMessage",                lua_setmsg},
  {"getMessageState",           lua_getmsg},
  {"setMessageProgress",        lua_setprogbar},
  {"setMessageProgMsg",         lua_setprogmsg},
  {"closeMessage",              lua_closemsg},
  {"getAsyncState",             lua_getasyncstate},
  {"getAsyncResult",            lua_getasyncres},
  {"getPsId",                   lua_getpsid},
  {"getFreeSpace",              lua_freespace},
  {"getTotalSpace",             lua_totalspace},
  {"getFirmware",               lua_firmware},
  {"getSpoofedFirmware",        lua_firmware2},
  {"getFactoryFirmware",        lua_firmware3},
  {"unmountPartition",          lua_unmount},
  {"mountPartition",            lua_mount},
  {"installApp",                lua_promote},
  {"uninstallApp",              lua_demote},
  {"doesAppExist",              lua_isappin},
  {"getBootParams",             lua_bootparams},
  {"loadUserPlugin",            lua_loadplugin},
  {"loadKernelPlugin",          lua_loadkplugin},
  {"unloadUserPlugin",          lua_unloadplugin},
  {"unloadKernelPlugin",        lua_unloadkplugin},
  {"unmountMountpoint",         lua_unmountvirtual},
  {0, 0}
};

void luaSystem_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, System_functions, 0);
	lua_setglobal(L, "System");
	int BUTTON_OK = 0;
	int BUTTON_YES_NO = 1;
	int BUTTON_NONE = 2;
	int BUTTON_OK_CANCEL = 3;
	int BUTTON_CANCEL = 4;
	int READ_ONLY = 1;
	int READ_WRITE = 2;
	VariableRegister(L,BUTTON_OK);
	VariableRegister(L,BUTTON_YES_NO);
	VariableRegister(L,BUTTON_NONE);
	VariableRegister(L,BUTTON_OK_CANCEL);
	VariableRegister(L,BUTTON_CANCEL);
	VariableRegister(L,AUTO_SUSPEND_TIMER);
	VariableRegister(L,SCREEN_OFF_TIMER);
	VariableRegister(L,SCREEN_DIMMING_TIMER);
	VariableRegister(L,FREAD);
	VariableRegister(L,FWRITE);
	VariableRegister(L,FCREATE);
	VariableRegister(L,FRDWR);
	VariableRegister(L,SET);
	VariableRegister(L,END);
	VariableRegister(L,CUR);
	VariableRegister(L,READ_ONLY);
	VariableRegister(L,READ_WRITE);
	VariableRegister(L,FORMAT_BMP);
	VariableRegister(L,FORMAT_PNG);
	VariableRegister(L,FORMAT_JPG);
}
