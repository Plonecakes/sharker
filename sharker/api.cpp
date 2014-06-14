#include "api.h"
#include "logging.h"

const luaL_Reg sharker_api[] = {
	{ "Log", API_LogMessage },
	{ NULL, NULL }
};

LUALIB_API int luaopen_sharker(lua_State *L) {
	lua_pushglobaltable(L);
	luaL_setfuncs(L, sharker_api, 0);
	return 1;
}

/* Log(message)
 Writes a message to the log file.
*/
static int API_LogMessage(lua_State *L) {
	const char *message = luaL_checkstring(L, 1);
	LogMessage(PIDVD "Lua: %s", GetCurrentProcessId(), message);
	return 0;
}
