#pragma once

#include "stdafx.h"
#include "lua.h"

LUALIB_API int luaopen_sharker(lua_State *L);
static int API_LogMessage(lua_State *L);