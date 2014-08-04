#pragma once

#include "stdafx.h"
#include "lua.h"

LUALIB_API int luaopen_sharker(lua_State *L);
static int API_LogMessage(lua_State *L);

static int API_CreateOptions(lua_State *L);
static int API_Options_Foster(lua_State *L);
static int API_Options_Get(lua_State *L);
static int API_Options_Set(lua_State *L);
//static int API_Options__index(lua_State *L);

/*static int API_CreateMenu(lua_State *L);
static int API_CreateCheckMenu(lua_State *L);
static int API_CreateRadioMenu(lua_State *L);
static int API_Menu_Append(lua_State *L);
static int API_Menu_Replace(lua_State *L);
static int API_Menu_AddItem(lua_State *L);
static int API_Menu_AddSep(lua_State *L);
static int API_Menu_Remove(lua_State *L);
static int API_Menu_EnableOption(lua_State *L);
static int API_Menu_DisableOption(lua_State *L);
static int API_Menu_EnableAll(lua_State *L);
static int API_Menu_DisableAll(lua_State *L);*/