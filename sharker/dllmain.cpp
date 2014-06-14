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
		// Logging
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
			LogMessage(PIDVD "Lua error: %s", PID, luaL_checkstring(lua_state, 1));
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

