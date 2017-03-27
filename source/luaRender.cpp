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

struct rawVertexList{
	vertex* vert;
	rawVertexList* next;
};

struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	char* data;
};

Image* ImageLoad(vita2d_texture* tmp){
	if (tmp == NULL) return NULL;
	Image* image = (Image*)malloc(sizeof(Image));
	image->sizeY = vita2d_texture_get_height(tmp);
	image->sizeX = vita2d_texture_get_width(tmp);
	image->data = (char*)malloc(image->sizeY*image->sizeX*3);
	char* rgba_buf = (char*)vita2d_texture_get_datap(tmp);
	for (int y=0; y < image->sizeY; y++){
		for (int x=0; x < image->sizeX; x++){
			int idx = (x + y * image->sizeX);
			*(uint32_t*)(&(image->data[idx*3])) = ((*(uint32_t*)&(rgba_buf[(x + (image->sizeY - y - 1) * image->sizeX)<<2]) & 0x00FFFFFF) | (*(uint32_t*)(&(image->data[idx*3])) & 0xFF000000));
		}
	}
	vita2d_free_texture(tmp);
	return image;
}

Image* PngLoad(const char* filename){
	vita2d_texture* tmp = vita2d_load_PNG_file(filename);
	return ImageLoad(tmp);
}

Image* BitmapLoad(const char* filename){
	vita2d_texture* tmp = vita2d_load_BMP_file(filename);
	return ImageLoad(tmp);
}

Image* JpegLoad(const char* filename){
	vita2d_texture* tmp = vita2d_load_JPEG_file(filename);
	return ImageLoad(tmp);
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
	else if (magic == 0xD8FF) result = JpegLoad(text);
	else if (magic == 0x5089) result = PngLoad(text);
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

static int lua_loadobj(lua_State *L){
	int argc = lua_gettop(L);
    #ifndef SKIP_ERROR_HANDLING
		if (argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *file_tbo = luaL_checkstring(L, 1); //Model filename
	const char* text = luaL_checkstring(L, 2); // Texture filename
	
	// Loading texture
	SceUID file = sceIoOpen(text, SCE_O_RDONLY, 0777);
	uint16_t magic;
	sceIoRead(file, &magic, 2);
	sceIoClose(file);
	Image* result;
	if (magic == 0x4D42) result = BitmapLoad(text);
	else if (magic == 0xD8FF) result = JpegLoad(text);
	else if (magic == 0x5089) result = PngLoad(text);
	else return luaL_error(L, "Error loading image.");
	#ifndef SKIP_ERROR_HANDLING
	if (result == NULL) return luaL_error(L, "Error loading image.");
	#endif
	
	// Opening model file and loading it on RAM
	file = sceIoOpen(file_tbo, SCE_O_RDONLY, 0777);
	uint32_t size = sceIoLseek(file, 0, SEEK_END);
	sceIoLseek(file, 0, SEEK_SET);
	char* content = (char*)malloc(size+1);
	sceIoRead(file, content, size);
	content[size] = 0;
	
	// Closing file
	sceIoClose(file);
	
	// Creating temp vertexList
	rawVertexList* vl = (rawVertexList*)malloc(sizeof(rawVertexList));
	rawVertexList* init = vl;
	
	// Init variables
	char* str = content;
	char* ptr = strstr(str,"v ");
	int idx;
	char float_val[16];
	char* init_val;
	char magics[3][3] = {"v ","vt","vn"};
	int magics_idx = 0;
	vertex* res;
	int v_idx = 0;
	bool skip = false;
	char* end_vert;
	char* end_val;
	float* vert_args;
	rawVertexList* old_vl;
	
	// Vertices extraction
	for(;;){
		
		// Check if a magic change is needed
		while (ptr == NULL){
			if (magics_idx < 2){
				res = init->vert;
				vl = init;
				magics_idx++;
				ptr = strstr(str,magics[magics_idx]);
			}else{
				skip = true;
				break;
			}
		}
		if (skip) break;
		
		// Extract vertex
		if (magics_idx == 0) idx = 0;
		else if (magics_idx == 1) idx = 3;
		else idx = 5;
		if (magics_idx == 0) init_val = ptr + 2;
		else init_val = ptr + 3;
		while (init_val[0] == ' ') init_val++;
		end_vert = strstr(init_val,"\n");
		if (magics_idx == 0) res = (vertex*)malloc(sizeof(vertex));
		end_val = strstr(init_val," ");
		vert_args = (float*)res; // Hacky way to iterate in vertex struct		
		while (init_val < end_vert){
			if (end_val > end_vert) end_val = end_vert;
			strncpy(float_val, init_val, end_val - init_val);
			float_val[end_val - init_val] = 0;
			vert_args[idx] = atof(float_val);
			idx++;
			init_val = end_val + 1;
			while (init_val[0] == ' ') init_val++;
			end_val = strstr(init_val," ");
		}
		
		// Update rawVertexList struct
		if (magics_idx == 0){
			vl->vert = res;
			vl->next = (rawVertexList*)malloc(sizeof(rawVertexList));
		}
		old_vl = vl;
		vl = vl->next;
		if (magics_idx == 0){
			vl->vert = NULL;
			vl->next = NULL;
		}else{
			if (vl == NULL){
				old_vl->next = (rawVertexList*)malloc(sizeof(rawVertexList));
				vl = old_vl->next;
				vl->vert = (vertex*)malloc(sizeof(vertex));
				vl->next = NULL;
			}else if(vl->vert == NULL) vl->vert = (vertex*)malloc(sizeof(vertex));
			res = vl->vert;
		}
		
		// Searching for next vertex
		str = ptr + 1;
		ptr = strstr(str,magics[magics_idx]);
		
	}

	// Creating real RAW vertexList
	ptr = strstr(str, "f ");
	rawVertexList* faces = (rawVertexList*)malloc(sizeof(rawVertexList));
	rawVertexList* initFaces = faces;
	faces->vert = NULL;
	faces->next = NULL;
	int len = 0;
	char val[8];
	int f_idx;
	char* ptr2;
	int t_idx;
	rawVertexList* tmp;
	
	// Faces extraction
	while (ptr != NULL){
		
		// Skipping padding
		ptr+=2;		
		
		// Extracting face info
		f_idx = 0;
		while (f_idx < 3){
		
			// Allocating new vertex
			faces->vert = (vertex*)malloc(sizeof(vertex));
		
			// Extracting x,y,z
			ptr2 = strstr(ptr,"/");
			strncpy(val,ptr,ptr2-ptr);
			val[ptr2-ptr] = 0;
			v_idx = atoi(val);
			t_idx = 1;
			tmp = init;
			while (t_idx < v_idx){
				tmp = tmp->next;
				t_idx++;
			}
			faces->vert->x = tmp->vert->x;
			faces->vert->y = tmp->vert->y;
			faces->vert->z = tmp->vert->z;
			
			// Extracting texture info
			ptr = ptr2+1;
			ptr2 = strstr(ptr,"/");
			if (ptr2 != ptr){
				strncpy(val,ptr,ptr2-ptr);
				val[ptr2-ptr] = 0;
				v_idx = atoi(val);
				t_idx = 1;
				tmp = init;
				while (t_idx < v_idx){
					tmp = tmp->next;
					t_idx++;
				}
				faces->vert->t1 = tmp->vert->t1;
				faces->vert->t2 = tmp->vert->t2;
			}else{
				faces->vert->t1 = 0.0f;
				faces->vert->t2 = 0.0f;
			}
			
			// Extracting normals info
			ptr = ptr2+1;
			if (f_idx < 2) ptr2 = strstr(ptr," ");
			else{
				ptr2 = strstr(ptr,"\n");
				if (ptr2 == NULL) ptr2 = content + size;
			}
			strncpy(val,ptr,ptr2-ptr);
			val[ptr2-ptr] = 0;
			v_idx = atoi(val);
			t_idx = 1;
			tmp = init;
			while (t_idx < v_idx){
				tmp = tmp->next;
				t_idx++;
			}
			faces->vert->n1 = tmp->vert->n1;
			faces->vert->n2 = tmp->vert->n2;
			faces->vert->n3 = tmp->vert->n3;

			// Setting values for next vertex
			ptr = ptr2;
			faces->next = (rawVertexList*)malloc(sizeof(rawVertexList));
			faces = faces->next;
			faces->next = NULL;
			faces->vert = NULL;
			len++;
			f_idx++;
		}
		
		ptr = strstr(ptr,"f ");
		
	}
	
	// Freeing temp vertexList and allocated file
	free(content);
	rawVertexList* tmp_init;
	while (init != NULL){
		tmp_init = init;
		free(init->vert);
		init = init->next;
		free(tmp_init);
	}
	
	// Create the model struct and populating vertex list
	if (len % 3 != 0) return luaL_error(L, "invalid model file.");
	model* res_m = (model*)malloc(sizeof(model));
	res_m->magic = 0xC0C0C0C0;
	vertexList* vlist = (vertexList*)malloc(sizeof(vertexList));
	res_m->v = vlist;
	vlist->next = NULL;
	bool first = true;
	for(int i = 0; i < len; i+=3) {
		if (first) first = false;
		else{
			vlist->next = (vertexList*)malloc(sizeof(vertexList));
			vlist = vlist->next;
			vlist->next = NULL;
		}
		tmp_init = initFaces;
		memcpy(&vlist->v1,initFaces->vert,sizeof(vertex));
		initFaces = initFaces->next;
		free(tmp_init->vert);
		free(tmp_init);
		tmp_init = initFaces;
		memcpy(&vlist->v2,initFaces->vert,sizeof(vertex));
		initFaces = initFaces->next;
		free(tmp_init->vert);
		free(tmp_init);
		tmp_init = initFaces;
		memcpy(&vlist->v3,initFaces->vert,sizeof(vertex));
		initFaces = initFaces->next;
		free(tmp_init->vert);
		free(tmp_init);
	}
	
	// Setting texture
	glGenTextures(1, &res_m->glTexture[0]);
	glBindTexture(GL_TEXTURE_2D, res_m->glTexture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, result->sizeX, result->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, result->data);
	glEnable(GL_TEXTURE_2D);
	res_m->texture = result;
	
	// Create a model object and push it into Lua stack
	lua_pushinteger(L, (uint32_t)res_m);
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
	{"loadObject",						lua_loadobj},
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