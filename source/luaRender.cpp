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
#- cpasjuste for vTinyGL -----------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

#include <string.h>
#include <vitasdk.h>
#include <GL/gl.h>
#include <vita2d.h>
#include "include/luaplayer.h"

struct vertex{
	float x;
	float y;
	float z;
	float t1;
	float t2;
	float n1;
	float n2;
	float n3;
};

struct vertexList{ // We'll use hardcoded GL_TRIANGLES for lpp-3ds compatibility
	vertex v1;
	vertex v2;
	vertex v3;
	vertexList* next;
};

struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
};

Image* BitmapLoad(char *filename) {
	FILE *file;
	unsigned long size;
	unsigned long i;
	unsigned short int planes;
	unsigned short int bpp;
	char temp;
	if ((file = fopen(filename, "rb"))==NULL) return NULL;
    fseek(file, 18, SEEK_CUR);
	Image* image = (Image*)malloc(sizeof(Image));
    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) return NULL;
    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) return NULL;
    size = image->sizeX * image->sizeY * 3;
    if ((fread(&planes, 2, 1, file)) != 1) return NULL;
    if (planes != 1) return 0;
    if ((i = fread(&bpp, 2, 1, file)) != 1) return NULL;
    if (bpp != 24) return NULL;
    fseek(file, 24, SEEK_CUR);
    image->data = (char *) malloc(size);
    if (image->data == NULL) return NULL;
	if ((i = fread(image->data, size, 1, file)) != 1) return NULL;
	for (i=0;i<size;i+=3) {
		temp = image->data[i];
		image->data[i] = image->data[i+2];
		image->data[i+2] = temp;
    }
    return image;
}

struct model{
	uint32_t magic;
	vertexList* v;
	Image* texture;
	GLuint glTexture[1];
};

static int lua_init(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 3) return luaL_error(L, "wrong number of arguments.");
	#endif
	uint32_t w = luaL_checkinteger(L, 1);
	uint32_t h = luaL_checkinteger(L, 2);
	uint32_t clr = luaL_checkinteger(L, 3);
	vglInit();
	float a = (clr >> 24) / 255.0f;
	float b = ((clr << 8) >> 24) / 255.0f;
	float g = ((clr << 16) >> 24) / 255.0f;
	float r = ((clr << 24) >> 24) / 255.0f;
	glClearColor(r, g, b, a);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);
	return 0;
}

static int lua_initblend(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return 0;
}

static int lua_termblend(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	#endif
	vglSwap();
	return 0;
}

static int lua_term(lua_State *L)
{
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
    if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	#endif
	vglClose();
	return 0;
}

static int lua_newVertex(lua_State *L){
	int argc = lua_gettop(L);
    #ifndef SKIP_ERROR_HANDLING
	if (argc != 8) return luaL_error(L, "wrong number of arguments");
	#endif
	vertex* res = (vertex*)malloc(sizeof(vertex));
	res->x = luaL_checknumber(L, 1);
	res->y = luaL_checknumber(L, 2);
	res->z = luaL_checknumber(L, 3);
	res->t1 = luaL_checknumber(L, 4);
	res->t2 = luaL_checknumber(L, 5);
	res->n1 = luaL_checknumber(L, 6);
	res->n2 = luaL_checknumber(L, 7);
	res->n3 = luaL_checknumber(L, 8);
	lua_pushinteger(L,(uint32_t)res);
	return 1;
}

static int lua_loadmodel(lua_State *L){
	int argc = lua_gettop(L);
    #ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	luaL_checktype(L, 1, LUA_TTABLE);
	int len = lua_rawlen(L, 1);
	#ifndef SKIP_ERROR_HANDLING
	if (len % 3 != 0) return luaL_error(L, "invalid vertex list.");
	#endif
	model* mdl = (model*)malloc(sizeof(model));
	mdl->magic = 0xC0C0C0C0;
	mdl->v = (vertexList*)malloc(sizeof(vertexList));
	mdl->v->next = NULL;
	vertexList* mdl_ptr = mdl->v;
	bool first = true;
	char* text = (char*)(luaL_checkstring(L, 2));
	SceUID file = sceIoOpen(text, SCE_O_RDONLY, 0777);
	uint16_t magic;
	sceIoRead(file, &magic, 2);
	sceIoClose(file);
	Image* result;
	if (magic == 0x4D42) result = BitmapLoad(text);
	//else if (magic == 0xD8FF) result = vita2d_load_JPEG_file(text);
	//else if (magic == 0x5089) result = vita2d_load_PNG_file(text);
	else return luaL_error(L, "Error loading image.");
	#ifndef SKIP_ERROR_HANDLING
	if (result == NULL) return luaL_error(L, "Error loading image.");
	#endif
	glGenTextures(1, &mdl->glTexture[0]);
	glBindTexture(GL_TEXTURE_2D, mdl->glTexture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, result->sizeX, result->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, result->data);
	glEnable(GL_TEXTURE_2D);
	
	// Creating vertex list
	for (int i = 0; i < len; i+=3){
		if (!first){
			mdl_ptr->next = (vertexList*)malloc(sizeof(vertexList));
			mdl_ptr = mdl_ptr->next;
			mdl_ptr->next = NULL;
		}else first = false;
		uint8_t* ptr = (uint8_t*)mdl_ptr;
		for (int j = 0; j < 3; j++){
			lua_pushinteger(L, i+j+1);
			lua_gettable(L, -(argc+1));
			vertex* vert = (vertex*)lua_tointeger(L, -1);
			memcpy(&ptr[j*sizeof(vertex)], vert, sizeof(vertex));
			lua_pop(L, 1);
		}
	}
	
	mdl->texture = result;
	lua_pushinteger(L, (uint32_t)mdl);
	return 1;
}

static int lua_unloadmodel(lua_State *L){
	int argc = lua_gettop(L);
    #ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	model* mdl = (model*)luaL_checkinteger(L, 1);
	#ifndef SKIP_ERROR_HANDLING
	if (mdl->magic != 0xC0C0C0C0) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	vertexList* v = mdl->v;
	while (v != NULL){
		vertexList* old = v;
		v = v->next;
		free(old);
	}
	free(mdl->texture->data);
	free(mdl->texture);
	free(mdl);
	return 0;
}

static int lua_delVertex(lua_State *L){
	int argc = lua_gettop(L);
    #ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	#endif
	vertex* v = (vertex*)luaL_checkinteger(L, 1);
	free(v);
	return 0;
}

static int lua_drawmodel(lua_State *L){
	int argc = lua_gettop(L);
    #ifndef SKIP_ERROR_HANDLING
	if (argc != 6) return luaL_error(L, "wrong number of arguments");
	#endif
	model* mdl = (model*)luaL_checkinteger(L, 1);
	#ifndef SKIP_ERROR_HANDLING
	if (mdl->magic != 0xC0C0C0C0) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	float angleX = luaL_checknumber(L, 5);
	float angleY = luaL_checknumber(L, 6);
	glLoadIdentity();
	glTranslatef(x,y,z);
	glRotatef(angleX,1.0f,0.0f,0.0f);
	glRotatef(angleY,0.0f,1.0f,0.0f);
	glBindTexture(GL_TEXTURE_2D, mdl->glTexture[0]);
	glBegin(GL_TRIANGLES);
	vertexList* object = mdl->v;
	while (object != NULL){
		glTexCoord2f(object->v1.t1,object->v1.t2);
		glVertex3f(object->v1.x,object->v1.y,object->v1.z);
		glTexCoord2f(object->v2.t1,object->v2.t2);
		glVertex3f(object->v2.x,object->v2.y,object->v2.z);
		glTexCoord2f(object->v3.t1,object->v3.t2);
		glVertex3f(object->v3.x,object->v3.y,object->v3.z);
		object = object->next;
	}
	glEnd();
}

//Register our Render Functions
static const luaL_Reg Render_functions[] = {
	{"init",							lua_init},
	{"term",							lua_term},
	{"initBlend",						lua_initblend},
	{"termBlend",						lua_termblend},
	{"createVertex",					lua_newVertex},
	{"destroyVertex",					lua_delVertex},
	{"loadModel",						lua_loadmodel},
	{"unloadModel",						lua_unloadmodel},
	{"drawModel",						lua_drawmodel},
	{0, 0}
};

void luaRender_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Render_functions, 0);
	lua_setglobal(L, "Render");
	lua_newtable(L);
}