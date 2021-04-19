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
#include <vitasdk.h>
}
#include <imgui_vita2d/imgui_vita.h>
#include "include/luaplayer.h"
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

enum {
	DARK_THEME,
	LIGHT_THEME,
	CLASSIC_THEME
};

enum {
	SET_ONCE,
	SET_ALWAYS
};

enum {
	FLAG_NONE                 = 0,
	FLAG_NO_TITLEBAR          = 1 << 0,
	FLAG_NO_RESIZE            = 1 << 1,
	FLAG_NO_MOVE              = 1 << 2,
	FLAG_NO_SCROLLBAR         = 1 << 3,
	FLAG_NO_COLLAPSE          = 1 << 5,
	FLAG_HORIZONTAL_SCROLLBAR = 1 << 11
};

static int lua_init(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplVita2D_Init();
	ImGui::StyleColorsDark();
	ImGui_ImplVita2D_TouchUsage(true);
	ImGui_ImplVita2D_UseIndirectFrontTouch(true);
	ImGui_ImplVita2D_UseRearTouch(false);
	ImGui_ImplVita2D_GamepadUsage(true);
	
	return 0;
}

static int lua_config(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 4) return luaL_error(L, "wrong number of arguments");
#endif
	bool use_touch = lua_toboolean(L, 1);
	bool use_rearpad = lua_toboolean(L, 2);
	bool use_buttons = lua_toboolean(L, 3);
	bool use_indirect_touch = lua_toboolean(L, 4);
	ImGui_ImplVita2D_TouchUsage(use_touch);
	ImGui_ImplVita2D_UseIndirectFrontTouch(use_indirect_touch);
	ImGui_ImplVita2D_UseRearTouch(use_rearpad);
	ImGui_ImplVita2D_GamepadUsage(use_buttons);
	return 0;
}

static int lua_term(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui_ImplVita2D_Shutdown();
	ImGui::DestroyContext();

	return 0;
}

static int lua_initblend(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui_ImplVita2D_NewFrame();
	
	return 0;
}

static int lua_termblend(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui::Render();
	ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());
	
	return 0;
}

static int lua_settheme(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	int idx = luaL_checkinteger(L, 1);
	switch (idx) {
	case DARK_THEME:
		ImGui::StyleColorsDark();
		break;
	case LIGHT_THEME:
		ImGui::StyleColorsLight();
		break;
	default:
		ImGui::StyleColorsClassic();
		break;
	}
	
	return 0;
}

static int lua_smmenubar(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	lua_pushboolean(L, ImGui::BeginMainMenuBar());
	
	return 1;
}

static int lua_emmenubar(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui::EndMainMenuBar();
	
	return 0;
}

static int lua_smenu(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	bool enabled = true;
	if (argc == 2) {
		enabled = lua_toboolean(L, 2);
	}
	lua_pushboolean(L, ImGui::BeginMenu(label, enabled));
	
	return 1;
}

static int lua_emenu(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui::EndMenu();
	
	return 0;
}

static int lua_text(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	if (argc == 1) {
		ImGui::Text(label);
	} else {
		int color = luaL_checkinteger(L, 2);
		ImGui::TextColored(ImVec4(
			(float)(color & 0xFF) / 255.0f,
			(float)((color >> 8) & 0xFF) / 255.0f,
			(float)((color >> 16) & 0xFF) / 255.0f,
			(float)((color >> 24) & 0xFF) / 255.0f),
			label);
	}
	
	return 0;
}

static int lua_distext(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	ImGui::TextDisabled(label);
	
	return 0;
}

static int lua_wraptext(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	ImGui::TextWrapped(label);
	
	return 0;
}

static int lua_button(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	float sizex = 0;
	float sizey = 0;
	if (argc == 3) {
		sizex = luaL_checknumber(L, 2);
		sizey = luaL_checknumber(L, 3);
	}
	lua_pushboolean(L, ImGui::Button(label, ImVec2(sizex, sizey)));
	
	return 1;
}

static int lua_sbutton(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	lua_pushboolean(L, ImGui::SmallButton(label));
	
	return 1;
}

static int lua_checkbox(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	bool status = lua_toboolean(L, 2);
	ImGui::Checkbox(label, &status);
	lua_pushboolean(L, status);
	
	return 1;
}

static int lua_radiobutton(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	bool status = lua_toboolean(L, 2);
	lua_pushboolean(L, ImGui::RadioButton(label, status));
	
	return 1;
}

static int lua_sameline(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui::SameLine();
	
	return 0;
}

static int lua_swindow(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	
	ImGuiWindowFlags flags = FLAG_NONE;
	if (argc == 2) {
		flags = luaL_checkinteger(L, 2);
	}
	
	ImGui::Begin(label, nullptr, flags);
	
	return 0;
}

static int lua_ewindow(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui::End();
	
	return 0;
}

static int lua_winpos(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	int mode  = luaL_checkinteger(L, 3);
	ImGuiCond flags;
	switch (mode) {
	case SET_ONCE:
		flags = ImGuiCond_Once;
		break;
	default:
		flags = ImGuiCond_Always;
		break;
	}
	ImGui::SetNextWindowPos(ImVec2(x, y), flags);
	
	return 0;
}

static int lua_winsize(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	int mode  = luaL_checkinteger(L, 3);
	ImGuiCond flags;
	switch (mode) {
	case SET_ONCE:
		flags = ImGuiCond_Once;
		break;
	default:
		flags = ImGuiCond_Always;
		break;
	}
	ImGui::SetNextWindowSize(ImVec2(x, y), flags);
	
	return 0;
}

static int lua_separator(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui::Separator();
	
	return 0;
}

static int lua_slider(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc < 4 && argc > 7) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	float v_min = luaL_checknumber(L, 2);
	float v_max = luaL_checknumber(L, 3);
	
	float vals[4];
	vals[0] = luaL_checknumber(L, 4);
	
	switch (argc) {
	case 4:
		ImGui::SliderFloat(label, vals, v_min, v_max);
		lua_pushnumber(L, vals[0]);
		break;
	case 5:
		vals[1] = luaL_checknumber(L, 5);
		ImGui::SliderFloat2(label, vals, v_min, v_max);
		lua_pushnumber(L, vals[0]);
		lua_pushnumber(L, vals[1]);
		break;
	case 6:
		vals[1] = luaL_checknumber(L, 5);
		vals[2] = luaL_checknumber(L, 6);
		ImGui::SliderFloat3(label, vals, v_min, v_max);
		lua_pushnumber(L, vals[0]);
		lua_pushnumber(L, vals[1]);
		lua_pushnumber(L, vals[2]);
		break;
	
	default:
		vals[1] = luaL_checknumber(L, 5);
		vals[2] = luaL_checknumber(L, 6);
		vals[3] = luaL_checknumber(L, 7);
		ImGui::SliderFloat4(label, vals, v_min, v_max);
		lua_pushnumber(L, vals[0]);
		lua_pushnumber(L, vals[1]);
		lua_pushnumber(L, vals[2]);
		lua_pushnumber(L, vals[3]);
		break;
	}
	
	return argc - 3;
}

static int lua_islider(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc < 4 && argc > 7) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	int v_min = luaL_checkinteger(L, 2);
	int v_max = luaL_checkinteger(L, 3);
	
	int vals[4];
	vals[0] = luaL_checkinteger(L, 4);
	
	switch (argc) {
	case 4:
		ImGui::SliderInt(label, vals, v_min, v_max);
		lua_pushinteger(L, vals[0]);
		break;
	case 5:
		vals[1] = luaL_checknumber(L, 5);
		ImGui::SliderInt2(label, vals, v_min, v_max);
		lua_pushinteger(L, vals[0]);
		lua_pushinteger(L, vals[1]);
		break;
	case 6:
		vals[1] = luaL_checknumber(L, 5);
		vals[2] = luaL_checknumber(L, 6);
		ImGui::SliderInt3(label, vals, v_min, v_max);
		lua_pushinteger(L, vals[0]);
		lua_pushinteger(L, vals[1]);
		lua_pushinteger(L, vals[2]);
		break;
	
	default:
		vals[1] = luaL_checkinteger(L, 5);
		vals[2] = luaL_checkinteger(L, 6);
		vals[3] = luaL_checkinteger(L, 7);
		ImGui::SliderInt4(label, vals, v_min, v_max);
		lua_pushinteger(L, vals[0]);
		lua_pushinteger(L, vals[1]);
		lua_pushinteger(L, vals[2]);
		lua_pushinteger(L, vals[3]);
		break;
	}
	
	return argc - 3;
}

static int lua_mitem(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 2 && argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	bool selected = false;
	bool enabled = true;
	if (argc > 1) {
		selected = lua_toboolean(L, 2);
		if (argc == 3) enabled = lua_toboolean(L, 3);
	}
	lua_pushboolean(L, ImGui::MenuItem(label, nullptr, selected, enabled));
	
	return 1;
}

static int lua_tooltip(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(label);
	
	return 0;
}

static int lua_combobox(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	int idx = luaL_checkinteger(L, 2);
	lua_rawgeti(L, 3, idx);
	if (ImGui::BeginCombo(label, lua_tostring(L, -1))) {
		uint32_t len = lua_objlen(L, 3);
		for (int i = 1; i <= len; i++) {
			bool is_selected = i == idx;
			lua_rawgeti(L, 3, i);
			if (ImGui::Selectable(lua_tostring(L, -1), is_selected))
				idx = i;
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	lua_pushinteger(L, idx);
	return 1;
}

static int lua_cursorpos(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
#endif
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	ImGui::SetCursorPos(ImVec2(x, y));
	return 0;
}

static int lua_textsize(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	ImVec2 size = ImGui::CalcTextSize(label);
	lua_pushnumber(L, size.x);
	lua_pushnumber(L, size.y);
	return 2;
}

static int lua_progressbar(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	float status = luaL_checknumber(L, 1);
	ImVec2 size;
	if (argc == 1) {
		size = ImVec2(-1, 0);
	} else {
		float w = luaL_checknumber(L, 2);
		float h = luaL_checknumber(L, 3);
		size = ImVec2(w, h);
	}
	ImGui::ProgressBar(status, size);
	return 0;
}

static int lua_colorpicker(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 2 && argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	int color = luaL_checkinteger(L, 2);
	bool alpha = false;
	
	float clr[4];
	clr[0] = (float)(color & 0xFF) / 255.0f;
	clr[1] = (float)((color >> 8) & 0xFF) / 255.0f;
	clr[2] = (float)((color >> 16) & 0xFF) / 255.0f;
	if (argc == 2) {
		alpha = lua_toboolean(L, 3);
	}
	
	if (alpha) {
		clr[3] = (float)((color >> 24) & 0xFF) / 255.0f;
		ImGui::ColorPicker4(label, clr);
	} else {
		ImGui::ColorPicker3(label, clr);
	}
	
	lua_pushinteger(L, (int)(clr[0] * 255.0f) | ((int)(clr[1] * 255.0f) << 8) | ((int)(clr[2] * 255.0f) << 16) | ((int)(clr[3] * 255.0f) << 24));
	return 1;
}

static int lua_widgetwidth(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
#endif
	float w = luaL_checknumber(L, 1);
	ImGui::PushItemWidth(w);
	return 0;
}

static int lua_widgetwidthr(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
#endif
	ImGui::PopItemWidth();
	return 0;
}

static int lua_listbox(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 3) return luaL_error(L, "wrong number of arguments");
#endif
	char *label = luaL_checkstring(L, 1);
	int idx = luaL_checkinteger(L, 2);
	uint32_t len = lua_objlen(L, 3);
	ImGui::ListBoxHeader(label, len);
	for (int i = 1; i <= len; i++) {
		bool is_selected = i == idx;
		lua_rawgeti(L, 3, i);
		if (ImGui::Selectable(lua_tostring(L, -1), is_selected))
			idx = i;
		if (is_selected)
			ImGui::SetItemDefaultFocus();
	}
	ImGui::ListBoxFooter();
	lua_pushinteger(L, idx);
	return 1;
}

static int lua_gimg(lua_State *L) {
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc < 1) return luaL_error(L, "wrong number of arguments");
#endif
	lpp_texture *text = (lpp_texture*)(luaL_checkinteger(L, 1));
#ifndef SKIP_ERROR_HANDLING	
	if (text->magic != 0xABADBEEF) luaL_error(L, "attempt to access wrong memory block type.");
#endif
	
	float width, height, tex_x, tex_y, tex_w, tex_h;
	uint32_t color;
	
	if (argc > 1) {
		width = luaL_checknumber(L, 2);
		height = luaL_checknumber(L, 3);
		if (argc > 3) {
			tex_x = luaL_checknumber(L, 4);
			tex_y = luaL_checknumber(L, 5);
			tex_w = luaL_checknumber(L, 6);
			tex_h = luaL_checknumber(L, 7);
			if (argc == 8) color = luaL_checkinteger(L, 8);
		}
	}
	
	switch (argc) {
	case 1:
		ImGui::Image(text->text, ImVec2(vita2d_texture_get_width(text->text), vita2d_texture_get_height(text->text)));
		break;
	case 3:
		ImGui::Image(text->text, ImVec2(width, height));
		break;
	case 7:
		ImGui::Image(text->text, ImVec2(width, height),
			ImVec2(tex_x / (float)vita2d_texture_get_width(text->text), tex_y / (float)vita2d_texture_get_height(text->text)),
			ImVec2((tex_x + tex_w) / (float)vita2d_texture_get_width(text->text), (tex_y + tex_h) / (float)vita2d_texture_get_height(text->text)));
		break;
	case 8:
		ImGui::Image(text->text, ImVec2(width, height),
			ImVec2(tex_x / (float)vita2d_texture_get_width(text->text), tex_y / (float)vita2d_texture_get_height(text->text)),
			ImVec2((tex_x + tex_w) / (float)vita2d_texture_get_width(text->text), (tex_y + tex_h) / (float)vita2d_texture_get_height(text->text)),
			ImVec4((float)(color & 0xFF) / 255.0f, (float)((color >> 8) & 0xFF) / 255.0f, (float)((color >> 16) & 0xFF) / 255.0f, (float)((color >> 24) & 0xFF) / 255.0f));
		break;
	default:
		return luaL_error(L, "wrong number of arguments");
	}
	
	return 0;
}

//Register our Gui Functions
static const luaL_Reg Gui_functions[] = {
  {"init",                lua_init},
  {"initBlend",           lua_initblend},
  {"termBlend",           lua_termblend},
  {"setTheme",            lua_settheme},
  {"initMainMenubar",     lua_smmenubar},
  {"termMainMenubar",     lua_emmenubar},
  {"initMenu",            lua_smenu},
  {"termMenu",            lua_emenu},
  {"drawText",            lua_text},
  {"drawDisabledText",    lua_distext},
  {"drawWrappedText",     lua_wraptext},
  {"drawButton",          lua_button},
  {"drawSmallButton",     lua_sbutton},
  {"drawCheckbox",        lua_checkbox},
  {"drawRadioButton",     lua_radiobutton},
  {"resetLine",           lua_sameline},
  {"initWindow",          lua_swindow},
  {"termWindow",          lua_ewindow},
  {"setWindowPos",        lua_winpos},
  {"setWindowSize",       lua_winsize},
  {"drawSeparator",       lua_separator},
  {"drawSlider",          lua_slider},
  {"drawIntSlider",       lua_islider},
  {"drawMenuItem",        lua_mitem},
  {"drawTooltip",         lua_tooltip},
  {"setInputMode",        lua_config},
  {"drawComboBox",        lua_combobox},
  {"setWidgetPos",        lua_cursorpos},
  {"getTextSize",         lua_textsize},
  {"drawProgressbar",     lua_progressbar},
  {"drawColorPicker",     lua_colorpicker},
  {"setWidgetWidth",      lua_widgetwidth},
  {"resetWidgetWidth",    lua_widgetwidthr},
  {"drawListBox",         lua_listbox},
  {"drawImage",           lua_gimg},
  {0, 0}
};

void luaGui_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Gui_functions, 0);
	lua_setglobal(L, "Gui");
	VariableRegister(L,DARK_THEME);
	VariableRegister(L,LIGHT_THEME);
	VariableRegister(L,CLASSIC_THEME);
	VariableRegister(L,SET_ONCE);
	VariableRegister(L,SET_ALWAYS);
	VariableRegister(L,FLAG_NONE);
	VariableRegister(L,FLAG_NO_COLLAPSE);
	VariableRegister(L,FLAG_NO_MOVE);
	VariableRegister(L,FLAG_NO_RESIZE);
	VariableRegister(L,FLAG_NO_SCROLLBAR);
	VariableRegister(L,FLAG_NO_TITLEBAR);
	VariableRegister(L,FLAG_HORIZONTAL_SCROLLBAR);
}
