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
#include <vitaGL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <libtoloader.h>
extern "C"{
	#include <utils.h> // utils.h file from vita2d
}
#include "include/luaplayer.h"

// Global Up and Front vectors
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 front = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 7.0f); // Camera position
glm::vec3 camera_front; // View direction
glm::vec3 camera_up; // Up vector for the camera
glm::vec3 camera_orientation = glm::vec3(-90.0f, 0.0f, 0.0f); // Yaw, Pitch, Roll
glm::vec3 camera_right; // Right vector for the camera
glm::mat4 view;

static bool has_texturing = false;
static bool camera_dirty = true;

typedef struct {
	uint32_t magic;
	to_model model;
	glm::mat4 model_matrix;
} lpp_model;

static void update_camera() {
	// Based on https://learnopengl.com/#!Getting-started/Camera
	camera_front = glm::normalize(glm::vec3(cos(glm::radians(camera_orientation.x)) * cos(glm::radians(camera_orientation.y)),
		sin(glm::radians(camera_orientation.y)), sin(glm::radians(camera_orientation.x)) * cos(glm::radians(camera_orientation.y))));
	front = glm::vec3(camera_front.x, 0.0f, camera_front.z);
	camera_right = glm::normalize(glm::cross(camera_front, up));
	camera_up = glm::normalize(glm::cross(camera_right, camera_front));
	view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
}

static int lua_Init(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
#endif
	if (camera_dirty) {
		update_camera();
		camera_dirty = false;
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0f, 960.f/544.0f, 0.01f, 10000.0f);
	glEnable(GL_DEPTH_TEST);
	
	if (has_texturing) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
	} else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}
	return 0;
}

static int lua_Term(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
#endif
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	return 0;
}

static int lua_loadObject(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif
	
	char *file = (char*)luaL_checkstring(L, 1);
	
#ifndef SKIP_ERROR_HANDLING
	FILE* f = fopen(file, "rb");
	if (f == NULL) return luaL_error(L, "file doesn't exist.");
	else fclose(f);
#endif

	to_model mdl;
	if (to_loadObj(file, &mdl))
		return luaL_error(L, "cannot load the model.");
	
	lpp_model *r = (lpp_model *)malloc(sizeof(lpp_model));
	r->magic = 0xFAFFA0BB;
	r->model = mdl;
	r->model_matrix = glm::mat4(1.0f);
	lua_pushinteger(L, (uint32_t)r);
	return 1;
}

static int lua_drawModel(lua_State *L) {
int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif
	lpp_model *mdl = (lpp_model *)luaL_checkinteger(L, 1);
	if (camera_dirty) {
		update_camera();
		camera_dirty = false;
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(view * mdl->model_matrix));
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, mdl->model.pos);
	if (has_texturing)
		glTexCoordPointer(2, GL_FLOAT, 0, mdl->model.texcoord);
	glDrawArrays(GL_TRIANGLES, 0, mdl->model.num_vertices);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	return 0;
}

static int lua_moveModel(lua_State *L) {
int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 4) return luaL_error(L, "wrong number of arguments.");
#endif
	lpp_model *mdl = (lpp_model *)luaL_checkinteger(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	mdl->model_matrix = glm::translate(mdl->model_matrix, glm::vec3(x, y, z));
	return 0;
}

static int lua_scaleModel(lua_State *L) {
int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 4) return luaL_error(L, "wrong number of arguments.");
#endif
	lpp_model *mdl = (lpp_model *)luaL_checkinteger(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	mdl->model_matrix = glm::scale(mdl->model_matrix, glm::vec3(x, y, z));
	return 0;
}

static int lua_rotateModel(lua_State *L) {
int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 4) return luaL_error(L, "wrong number of arguments.");
#endif
	lpp_model *mdl = (lpp_model *)luaL_checkinteger(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	if (x != 0.0f)
		mdl->model_matrix = glm::rotate(mdl->model_matrix, glm::radians(x), glm::vec3(1.0f, 0.0f, 0.0f));
	if (y != 0.0f)
		mdl->model_matrix = glm::rotate(mdl->model_matrix, glm::radians(y), glm::vec3(0.0f, 1.0f, 0.0f));
	if (z != 0.0f)
		mdl->model_matrix = glm::rotate(mdl->model_matrix, glm::radians(z), glm::vec3(0.0f, 0.0f, 1.0f));
	return 0;
}

static int lua_useTexture(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif

	has_texturing = lua_toboolean(L, 1);
	if (has_texturing) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
	} else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}
	return 0;
}

static int lua_setTexture(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments.");
#endif

	lpp_texture *t = (lpp_texture *)luaL_checkinteger(L, 1);
	glBindTexture(GL_TEXTURE_2D, t->text->tex_id);
	return 0;
}

static int lua_moveCamera(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments.");
#endif
	float front = luaL_checknumber(L, 1);
	float right = luaL_checknumber(L, 2);
	camera_pos += camera_right * right;
	camera_pos += camera_front * front;
	camera_dirty = true;
	return 0;
}

static int lua_rotateCamera(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments.");
#endif
	float x_ori = luaL_checknumber(L, 1);
	float y_ori = luaL_checknumber(L, 2);
	camera_orientation.x += x_ori;
	camera_orientation.y += y_ori;
	camera_dirty = true;
	return 0;
}

static int lua_setFog(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments.");
#endif
	bool has_fog = lua_toboolean(L, 1);
	float fog_density = luaL_checknumber(L, 2);
	if (has_fog) {
		glEnable(GL_FOG);
		glFogf(GL_FOG_DENSITY, fog_density);
	} else {
		glDisable(GL_FOG);
	}
	return 0;
}

//Register our Render Functions
static const luaL_Reg Render_functions[] = {
	{"loadObject", lua_loadObject},
	{"drawModel", lua_drawModel},
	{"moveModel", lua_moveModel},
	{"scaleModel", lua_scaleModel},
	{"rotateModel", lua_rotateModel},
	{"setTexture", lua_setTexture},
	{"useTexture", lua_useTexture},
	{"initBlend", lua_Init},
	{"termBlend", lua_Term},
	{"moveCamera", lua_moveCamera},
	{"rotateCamera", lua_rotateCamera},
	{"setFog", lua_setFog},
	{0, 0}
};

void luaRender_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Render_functions, 0);
	lua_setglobal(L, "Render");
}