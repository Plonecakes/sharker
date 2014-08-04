// dllmain.cpp : Defines the entry point for the DLL application.
#include "sharker.h"
#include "logging.h"
#include "api.h"
//#include "symbols.h"
#include "lua.h"

lua_State *lua_state = NULL;

int APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	DWORD PID = GetCurrentProcessId();
	int s;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// Logging.
		LogMessage("DLL loaded into process %i.", PID);

		// Proxy DLL?
		// Load Lua system.
		lua_state = luaL_newstate();
		luaL_openlibs(lua_state);
		luaopen_sharker(lua_state);
		LogMessage(PIDVD "Lua system initiated.", PID);

		// Load scripts.
		s = luaL_loadfile(lua_state, "test.lua");
		if (s == 0) {
			s = lua_pcall(lua_state, 0, LUA_MULTRET, 0);
		}
		if (s != 0) {
			char err[50];
			switch (s) {
			case LUA_ERRSYNTAX: strcpy_s(err, "Syntax error"); break;
			case LUA_ERRRUN: strcpy_s(err, "Runtime error"); break;
			case LUA_ERRMEM: strcpy_s(err, "Memory allocation error"); break;
			case LUA_ERRERR: strcpy_s(err, "Error while running the message handler"); break;
			case LUA_ERRGCMM: strcpy_s(err, "Error while running a __gc metamethod"); break;
			}
			LogMessage(PIDVD "Lua error: %s", PID, err);
			if(lua_gettop(lua_state) > 0) LogMessage(PIDVD "Error message: %s", PID, lua_tostring(lua_state, -1));
		}
		break;

	case DLL_PROCESS_DETACH:
		LogMessage(PIDVD "Detaching and cleaning up...", PID);
		// Cleanup.
		lua_close(lua_state);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return 0;
}

