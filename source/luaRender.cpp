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
extern "C"{
	#include <utils.h> // utils.h file from vita2d
}
#include <vita2d.h>
#include "include/luaplayer.h"

extern "C"{
	#include "include/math_utils.h"
}

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

static matrix4x4 _vita2d_projection_matrix;
extern SceGxmContext* _vita2d_context;
extern SceGxmVertexProgram* _vita2d_colorVertexProgram;
extern SceGxmFragmentProgram* _vita2d_colorFragmentProgram;
extern SceGxmVertexProgram* _vita2d_textureVertexProgram;
extern SceGxmFragmentProgram* _vita2d_textureFragmentProgram;
extern SceGxmFragmentProgram* _vita2d_textureTintFragmentProgram;
extern const SceGxmProgramParameter* _vita2d_colorWvpParam;
extern const SceGxmProgramParameter* _vita2d_textureWvpParam;
extern SceGxmProgramParameter* _vita2d_textureTintColorParam;

// Camera instance
struct camera {
	vector3f pos;
	vector3f rot;
	matrix4x4 matrix;
};
camera cam;

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

struct vertexList{ // We'll use hardcoded triangles primitives for lpp-3ds compatibility
	vertex v1;
	vertex v2;
	vertex v3;
	vertexList* next;
};

struct rawVertexList{
	vertex* vert;
	rawVertexList* next;
};

struct model{
	uint32_t magic;
	vita2d_texture_vertex* vertexList;
	uint16_t* idxList;
	SceUID memblocks[2];
	lpp_texture* texture;
	uint32_t facesCount;
};

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
	res->t2 = 1.0f - luaL_checknumber(L, 5);
	res->n1 = luaL_checknumber(L, 6);
	res->n2 = luaL_checknumber(L, 7);
	res->n3 = luaL_checknumber(L, 8);
	lua_pushinteger(L,(uint32_t)res);
	return 1;
}

static int lua_settext(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	model* mdl = (model*)(luaL_checkinteger(L, 1));
	lpp_texture* txt = (lpp_texture*)(luaL_checkinteger(L, 2));
	#ifndef SKIP_ERROR_HANDLING
	if (mdl->magic != 0xC0C0C0C0) luaL_error(L, "attempt to access wrong memory block type.");
	if (txt->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
	#endif
	mdl->texture = txt;
	return 0;
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
	mdl->facesCount = 0;
	vertexList* mdl_ptr = (vertexList*)malloc(sizeof(vertexList));
	vertexList* vlist = mdl_ptr;
	mdl_ptr->next = NULL;
	bool first = true;
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 2));
	#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
	#endif
	mdl->texture = text;
	
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
		mdl->facesCount++;
	}
	
	// Allocating vertices on VRAM
	uint32_t listSize = ALIGN(mdl->facesCount * 3 * sizeof(vita2d_texture_vertex), sizeof(vita2d_texture_vertex));
	uint32_t idxSize = ALIGN(mdl->facesCount * 3 * sizeof(uint16_t), sizeof(uint16_t));
	SceUID vertexListID;
	SceUID idxListID;
	vita2d_texture_vertex* vertexListPtr = (vita2d_texture_vertex*)gpu_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,listSize,sizeof(void *),SCE_GXM_MEMORY_ATTRIB_READ,&vertexListID);
	uint16_t* idxList = (uint16_t*)gpu_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,idxSize,sizeof(void *),SCE_GXM_MEMORY_ATTRIB_READ,&idxListID);
	vertexList* object = vlist;
	int n = 0;
	while (object != NULL){
		memcpy(&vertexListPtr[n], &object->v1, sizeof(vita2d_texture_vertex));
		memcpy(&vertexListPtr[n+1], &object->v2, sizeof(vita2d_texture_vertex));
		memcpy(&vertexListPtr[n+2], &object->v3, sizeof(vita2d_texture_vertex));
		idxList[n] = n;
		idxList[n+1] = n+1;
		idxList[n+2] = n+2;
		object = object->next;
		n += 3;
	}
	while (vlist != NULL){
		vertexList* old = vlist;
		vlist = vlist->next;
		free(old);
	}
	
	// Properly finishing populating model object
	mdl->vertexList = vertexListPtr;
	mdl->idxList = idxList;
	mdl->memblocks[0] = vertexListID;
	mdl->memblocks[1] = idxListID;
	
	lua_pushinteger(L, (uint32_t)mdl);
	return 1;
}

static int lua_loadobj(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	#endif
	const char *file_tbo = luaL_checkstring(L, 1); //Model filename
	
	// Loading texture
	lpp_texture* text = (lpp_texture*)(luaL_checkinteger(L, 2));
	#ifndef SKIP_ERROR_HANDLING
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
	#endif
	
	// Opening model file and loading it on RAM
	int file = sceIoOpen(file_tbo, SCE_O_RDONLY, 0777);
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
				faces->vert->t2 = 1.0f - tmp->vert->t2;
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
	vertexList* vlist_start = vlist;
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
	res_m->facesCount = len / 3;
	
	// Setting texture
	res_m->texture = text;
	
	// Allocating vertices on VRAM
	vlist = vlist_start;
	uint32_t listSize = ALIGN(res_m->facesCount * 3 * sizeof(vita2d_texture_vertex), sizeof(vita2d_texture_vertex));
	uint32_t idxSize = ALIGN(res_m->facesCount * 3 * sizeof(uint16_t), sizeof(uint16_t));
	SceUID vertexListID;
	SceUID idxListID;
	vita2d_texture_vertex* vertexListPtr = (vita2d_texture_vertex*)gpu_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,listSize,sizeof(void *),SCE_GXM_MEMORY_ATTRIB_READ,&vertexListID);
	uint16_t* idxList = (uint16_t*)gpu_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,idxSize,sizeof(void *),SCE_GXM_MEMORY_ATTRIB_READ,&idxListID);
	vertexList* object = vlist;
	int n = 0;
	while (object != NULL){
		memcpy(&vertexListPtr[n], &object->v1, sizeof(vita2d_texture_vertex));
		memcpy(&vertexListPtr[n+1], &object->v2, sizeof(vita2d_texture_vertex));
		memcpy(&vertexListPtr[n+2], &object->v3, sizeof(vita2d_texture_vertex));
		idxList[n] = n;
		idxList[n+1] = n+1;
		idxList[n+2] = n+2;
		object = object->next;
		n += 3;
	}
	while (vlist != NULL){
		vertexList* old = vlist;
		vlist = vlist->next;
		free(old);
	}
	
	// Properly finishing populating model object
	res_m->vertexList = vertexListPtr;
	res_m->idxList = idxList;
	res_m->memblocks[0] = vertexListID;
	res_m->memblocks[1] = idxListID;
	
	// Push model object into Lua stack
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
	gpu_free(mdl->memblocks[0]);
	gpu_free(mdl->memblocks[1]);
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
	if (argc != 7 && argc != 8) return luaL_error(L, "wrong number of arguments");
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
	float angleZ = luaL_checknumber(L, 7);
	bool skip_camera = (argc == 8) ? lua_toboolean(L, 8) : false;
	
	matrix4x4 model_matrix;
	matrix4x4 modelview_matrix;
	matrix4x4 mvp_matrix;
	matrix4x4 final_mvp_matrix;
	
	sceGxmSetVertexProgram(_vita2d_context, _vita2d_textureVertexProgram);
	sceGxmSetFragmentProgram(_vita2d_context, _vita2d_textureFragmentProgram);
	
	matrix4x4_init_perspective(_vita2d_projection_matrix,45.0f,960.0f/544.0f,0.1f,100.0f);

	matrix4x4_init_translation(model_matrix,x,y,z);
	matrix4x4_rotate_x(model_matrix,DEG_TO_RAD(angleX));
	matrix4x4_rotate_y(model_matrix,DEG_TO_RAD(angleY));
	matrix4x4_rotate_z(model_matrix,DEG_TO_RAD(angleZ));
	if (skip_camera) matrix4x4_multiply(mvp_matrix, _vita2d_projection_matrix, model_matrix);
	else{
		matrix4x4_multiply(modelview_matrix, cam.matrix, model_matrix);
		matrix4x4_multiply(mvp_matrix, _vita2d_projection_matrix, modelview_matrix);
	}
	matrix4x4_transpose(final_mvp_matrix,mvp_matrix);

	void* vertex_wvp_buffer;
	sceGxmReserveVertexDefaultUniformBuffer(_vita2d_context, &vertex_wvp_buffer);
	sceGxmSetUniformDataF(vertex_wvp_buffer, _vita2d_textureWvpParam, 0, 16, (const float*)final_mvp_matrix);
	
	sceGxmSetFragmentTexture(_vita2d_context, 0, &mdl->texture->text->gxm_tex);
	sceGxmSetVertexStream(_vita2d_context, 0, mdl->vertexList);
	sceGxmDraw(_vita2d_context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, mdl->idxList, mdl->facesCount * 3);
	
	return 0;
}

static int lua_cam(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 6) return luaL_error(L, "wrong number of arguments");
	#endif
	cam.pos.x = luaL_checknumber(L, 1);
	cam.pos.y = luaL_checknumber(L, 2);
	cam.pos.z = luaL_checknumber(L, 3);
	cam.rot.x = luaL_checknumber(L, 4);
	cam.rot.y = luaL_checknumber(L, 5);
	cam.rot.z = luaL_checknumber(L, 6);
	
	vector3f opposite;
	matrix4x4 mtmp, morient, mtrans;
	matrix4x4 mx, my, mz;
	
	vector3f_opposite(&opposite, &cam.pos);
	
	matrix4x4_init_translation_vector3f(mtrans, &opposite);
	matrix4x4_init_rotation_x(mx, -cam.rot.x);
	matrix4x4_init_rotation_y(my, -cam.rot.y);
	matrix4x4_init_rotation_z(mz, -cam.rot.z);

	matrix4x4_multiply(mtmp, mx, my);
	matrix4x4_multiply(morient, mtmp, mz);
	matrix4x4_multiply(cam.matrix, morient, mtrans);
	
	return 0;
}

//Register our Render Functions
static const luaL_Reg Render_functions[] = {
	{"createVertex",   lua_newVertex},
	{"destroyVertex",  lua_delVertex},
	{"loadModel",      lua_loadmodel},
	{"loadObject",     lua_loadobj},
	{"unloadModel",    lua_unloadmodel},
	{"drawModel",      lua_drawmodel},
	{"useTexture",     lua_settext},
	{"setCamera",      lua_cam},
	{0, 0}
};

void luaRender_init(lua_State *L) {
	vector3f_init(&cam.pos, 0.0f, 0.0f, 0.0f);
	vector3f_init(&cam.rot, 0.0f, 0.0f, 0.0f);
	matrix4x4_identity(cam.matrix);
	lua_newtable(L);
	luaL_setfuncs(L, Render_functions, 0);
	lua_setglobal(L, "Render");
	lua_newtable(L);
}