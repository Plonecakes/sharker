#include "api.h"
#include "logging.h"
#include "CDataFile.h"
#include <algorithm>
#include "ctype.h"

/* Log(message)
 Writes a message to the log file.
*/
static int API_LogMessage(lua_State *L) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "Log takes 1 argument: message");
	}
	const char *message = luaL_checkstring(L, 1);
	LogMessage(PIDVD "Lua: %s", GetCurrentProcessId(), message);
	return 0;
}

/*// OPTIONS FUNCTIONS //*/
/* Option value class
 Returns a simple accessor.
 If the name exists as a key in Options:
  This acts as its value in terms of operators:
   + - / * ^ % < <= > >= -
    Treats the value as a number, errors if it cannot.
   .. #
    Treats the value as a string.
   == ~=
    If compared with a number, it acts as a number if it can, otherwise it returns false.
    If compared with a string or another accessor, it treats both as strings.
   not
    Treats as inverse of .enabled(), see below.
    
  This also has a method .enabled() that will return its truthiness. Case insensitive.
    true:  true,  on,  1+, enabled
    false: false, off, 0,  disabled
 If the name does not exist as a key, value operations will fail.
 If the name does not exist:
  It must still return an accessor, which is assumed to be a new section.
 __index will search a section of this name for keys.
 __newindex will add or modify a key in this section.
*/

CDataFile* GetOptions(lua_State *L, int idx = 1) {
	/*0*/ luaL_checktype(L, idx, LUA_TTABLE);
	/*+*/ lua_getfield(L, idx, "__self");
	/*0*/ luaL_checktype(L, -1, LUA_TUSERDATA);
	/*0*/ CDataFile **opts = (CDataFile**)luaL_checkudata(L, -1, "Sharker.Options");
	/*-*/ lua_pop(L, 1);
	if (opts == NULL) {
		luaL_error(L, "Options:Get error retrieving information, may not be an Options instance");
		return NULL;
	}
	return *opts;
}

int CreateOptionValue(lua_State *L, int idx, const char *section, const char *key) {
	/*0*/ CDataFile *options = GetOptions(L, idx);

	// Create the returned table.
	/*+*/ lua_newtable(L);

	// Associate Options table.
	/*+*/ lua_pushvalue(L, idx < 0 ? idx - 1 : idx);
	/*-*/ lua_setfield(L, -2, "__options");

	if (section) {
		/*+*/ lua_pushstring(L, section);
		/*-*/ lua_setfield(L, -2, "__section");
		if (key) {
			/*+*/ lua_pushstring(L, key);
			/*-*/ lua_setfield(L, -2, "__key");

			// Set the value.
			/*0*/ const char *value;
			/*0*/ if (section) value = options->GetValue(key, section).c_str();
			/*0*/ else value = options->GetValue(key, "Options").c_str();
			/*+*/ lua_pushstring(L, value);
			/*-*/ lua_setfield(L, -2, "__value");
		}
		else {
			/*0*/ const char *value;
			/*0*/ value = options->GetValue(section, "Options").c_str();
			/*+*/ lua_pushstring(L, value);
			/*-*/ lua_setfield(L, -2, "__value");
		}
	}
	/*+*/ luaL_getmetatable(L, "Sharker.OptionValue");
	/*-*/ lua_setmetatable(L, -2);
	return 1;
}

t_Str BubbledOption(CDataFile *options, t_Str section, t_Str key) {
	t_Str value;
	value = options->GetValue(key, section);
	if (value != "") return value;
	if (section != "Options") {
		value = options->GetValue(key, "Options");
		if (value != "") return value;
	}
	if (options->parent == NULL) return "";
	else return BubbledOption(options->parent, section, key);
}

bool iszero(const char *str) {
	const char *c;
	for (c = str; *c != 0; ++c) {
		if (*c != '0' && *c != '.') return false;
	}
	// Returns false if the string is empty.
	return c != str;
}

int GetOptionValue(lua_State *L, bool number = false, int idx = 1) {
	const char *section, *key;
	/*+*/ lua_getfield(L, idx, "__options");
	/*0*/ CDataFile *options = GetOptions(L, -1);
	/*+*/ lua_getfield(L, idx, "__section");
	/*0*/ section = lua_tostring(L, -1);
	/*+*/ lua_getfield(L, idx, "__key");
	/*0*/ if (lua_isnil(L, -1)) key = NULL;
	/*0*/ else key = lua_tostring(L, -1);
	/*-
	  -
	  -*/ lua_pop(L, 3);
	t_Str value = "";
	if(key) value = BubbledOption(options, section, key);
	else value = BubbledOption(options, "Options", section);
	if (value == "") {
		/*+*/ lua_pushnil(L);
		return 1;
	}
	else {
		if (number) {
			double num_version = atof(value.c_str());
			if (num_version == 0 && !iszero(value.c_str())) {
				return luaL_error(L, "Attempted to use a non-numerical Option value as a number.");
			}
			/*+*/ lua_pushnumber(L, num_version);
		}
		else {
			/*+*/ lua_pushstring(L, value.c_str());
		}
		return 1;
	}
}

#define MetaError(a, b) if (lua_gettop(L) != b) { return luaL_error(L, "It's easier if you don't call __"a" directly!"); }
#define GetNumber(a) double a;  \
if (lua_istable(L, 1)) {        \
    GetOptionValue(L, true, 1); \
    a = lua_tonumber(L, -1);    \
}                               \
else a = lua_tonumber(L, 1);
#define GetNumbers(a, b) double a, b; \
if (lua_istable(L, 1)) {              \
    GetOptionValue(L, true, 1);       \
    a = lua_tonumber(L, -1);          \
}                                     \
else a = lua_tonumber(L, 1);          \
if (lua_istable(L, 2)) {              \
    GetOptionValue(L, true, 2);       \
    b = lua_tonumber(L, -1);          \
}                                     \
else b = lua_tonumber(L, 2);

#define GetStrings(a, b)         \
std::string a, b;                \
if (lua_istable(L, 1)) {         \
    GetOptionValue(L, false, 1); \
	a = lua_tostring(L, -1);     \
}                                \
else a = lua_tostring(L, 1);     \
if (lua_istable(L, 2)) {         \
    GetOptionValue(L, false, 2); \
	b = lua_tostring(L, -1);     \
}                                \
else b = lua_tostring(L, 2);

int API_Option_Value__index(lua_State *L) {
	MetaError("index", 2);

	const char *real_key = lua_tostring(L, 2);
	// Sometimes, these may be unset.
	if (strcmp(real_key, "__key") == 0 || strcmp(real_key, "__value") == 0) {
		lua_pushnil(L);
		return 1;
	}

	const char *section, *key;
	/*+*/ lua_getfield(L, 1, "__section");
	/*0*/ if (lua_isnil(L, -1)) section = NULL;
	/*0*/ else section = lua_tostring(L, -1);
	/*+*/ lua_getfield(L, 1, "__key");
	/*0*/ if (lua_isnil(L, -1)) key = NULL;
	/*0*/ else key = "fail";
	/*-
	  -
	  -*/ lua_pop(L, 3);

	// If this is already a key, indexing should fail.
	if (key) {
		return luaL_error(L, "Options error: Cannot further index a key.");
	}

	// Otherwise, this is a section, and we must check the key.
	/*+*/ lua_getfield(L, 1, "__options");
	return CreateOptionValue(L, -1, section, real_key);
}

int API_Option_Value__newindex(lua_State *L) {
	MetaError("newindex", 3);

	const char *section, *key, *newkey, *value;
	/*+*/ lua_getfield(L, 1, "__options");
	/*0*/ CDataFile *options = GetOptions(L, -1);
	/*+*/ lua_getfield(L, 1, "__section");
	/*0*/ section = lua_tostring(L, -1);
	/*+*/ lua_getfield(L, 1, "__key");
	/*0*/ if (lua_isnil(L, -1)) key = NULL;
	/*0*/ else key = "fail";
	/*0*/ newkey = lua_tostring(L, 2);
	/*0*/ value = lua_tostring(L, 3);
	/*-
	  -
	  -*/ lua_pop(L, 3);

	// If this is already a key, indexing should fail.
	if (key) {
		return luaL_error(L, "Options error: Cannot further index a key.");
	}

	// Otherwise, we can set this.
	options->SetValue(newkey, value, "", section);
	return 0;
}

int API_Option_Value__add(lua_State *L) {
	MetaError("add", 2);
	GetNumbers(num1, num2);

	lua_pushnumber(L, num1 + num2);
	return 1;
}

int API_Option_Value__sub(lua_State *L) {
	MetaError("sub", 2);
	GetNumbers(num1, num2);

	lua_pushnumber(L, num1 - num2);
	return 1;
}

int API_Option_Value__mul(lua_State *L) {
	MetaError("mul", 2);
	GetNumbers(num1, num2);

	lua_pushnumber(L, num1 * num2);
	return 1;
}

int API_Option_Value__div(lua_State *L) {
	MetaError("div", 2);
	GetNumbers(num1, num2);

	lua_pushnumber(L, num1 / num2);
	return 1;
}

int API_Option_Value__mod(lua_State *L) {
	MetaError("mod", 2);
	GetNumbers(num1, num2);

	lua_pushnumber(L, fmod(num1, num2));
	return 1;
}

int API_Option_Value__pow(lua_State *L) {
	MetaError("mod", 2);
	GetNumbers(num1, num2);

	lua_pushnumber(L, pow(num1, num2));
	return 1;
}

int API_Option_Value__unm(lua_State *L) {
	MetaError("unm", 2);
	GetNumber(num1);

	lua_pushnumber(L, -num1);
	return 1;
}

int API_Option_Value__lt(lua_State *L) {
	MetaError("lt", 2);
	GetNumbers(num1, num2);

	lua_pushboolean(L, num1 < num2);
	return 1;
}

int API_Option_Value__le(lua_State *L) {
	MetaError("mod", 2);
	GetNumbers(num1, num2);

	lua_pushboolean(L, num1 <= num2);
	return 1;
}

int API_Option_Value__concat(lua_State *L) {
	MetaError("concat", 2);
	GetStrings(str1, str2);

	lua_pushstring(L, (str1 + str2).c_str());
	return 1;
}

int API_Option_Value__len(lua_State *L) {
	MetaError("len", 2);
	std::string str1;
	if (lua_istable(L, 1)) {
		GetOptionValue(L, false, 1);
		str1 = lua_tostring(L, -1);
	}
	else str1 = lua_tostring(L, 1);

	lua_pushinteger(L, str1.length());
	return 1;
}

int API_Option_Value__eq(lua_State *L) {
	MetaError("eq", 2);
	bool first = false;
	if ((first = lua_isnumber(L, 1)) || lua_isnumber(L, 2)) {
		// If one of the compared values is a number, we must try to compare as numbers.
		const char *str;
		double value, number;
		number = lua_tonumber(L, first ? 1 : 2);
		GetOptionValue(L, false, first ? 2 : 1);
		str = lua_tostring(L, -1);
		lua_pop(L, 1);
		value = atof(str);
		if (value == 0 && !iszero(str)) {
			// If it can't compare as a number, it can't compare as a string either!
			lua_pushboolean(L, false);
		}
		else {
			lua_pushboolean(L, value == number);
		}
	}
	else {
		// If neither are numbers, we can compare as strings.
		// I don't think that, if both are OptionValues, that they should ever be compared as numbers.
		GetStrings(str1, str2);
		lua_pushboolean(L, str1 == str2);
	}
	return 1;
}

int API_Option_Value_enabled(lua_State *L) {
	if (lua_gettop(L) != 1 || !lua_istable(L,1)) {
		return luaL_error(L, "OptionValue:enabled() takes 0 arguments.");
	}
	lua_getfield(L, 1, "__value");
	std::string str = lua_tostring(L, -1);
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	if (str == "1" || str == "true" || str == "on" || str == "enabled") {
		lua_pushboolean(L, true);
	}
	else if (str == "0" || str == "false" || str == "off" || str == "disabled") {
		lua_pushboolean(L, false);
	}
	else {
		// Perhaps this should error!
		lua_pushnil(L);
	}
	return 1;
}
/* Options(filename)
 Creates an Options object from the INI file.
*/
int CreateOptions(lua_State *L, CDataFile *parent = NULL) {
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "Options takes 1 argument: filename");
	}

	/*0*/ const char *filename = luaL_checkstring(L, 1);
	// Create the returned table.
	/*+*/ lua_newtable(L);

	// Create instance in Lua userspace and load the file.
	/*+*/ CDataFile** opts = (CDataFile**)lua_newuserdata(L, sizeof(CDataFile*));
	/*0*/ *opts = new CDataFile(filename);
	/*0*/ if (parent != NULL) (*opts)->parent = parent;

	// Associate the metatable to the userdata and the table.
	/*+*/ luaL_getmetatable(L, "Sharker.Options");
	/*+*/ lua_pushvalue(L, -1);
	/*-*/ lua_setmetatable(L, -3);
	/*-*/ lua_setmetatable(L, -3);

	// Associate the user data with the table.
	/*-*/ lua_setfield(L, -2, "__self");

	// Return the table.
	return 1;
}

static int API_CreateOptions(lua_State *L) {
	return CreateOptions(L, NULL);
}

/* Options:Foster(Options)
   Options:Foster(filename)
 Returns a new Options instance created from the argument, with this instance as its parent.
*/
static int API_Options_Foster(lua_State *L) {
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "Options:Foster takes 1 argument: filename or Options");
	}

	if (lua_istable(L, 2)) {
		// Copy this table and then set its parent.
		// TODO: Must implement copy in CDataFile, and do Lua's copy, and stuff...
		return luaL_error(L, "I lied, sorry! It doesn't take Options yet!");
	}

	// Retrieve the instance to pass as the parent and create its child.
	CDataFile *parent = GetOptions(L);
	lua_remove(L, 1);
	return CreateOptions(L, parent);
}

/* Options:Get([section = "Options", ]key)
 Returns the value associated with this key in the given section, then Options, then parents.
*/
static int API_Options_Get(lua_State *L) {
	int argc = lua_gettop(L);
	const char *key, *section;
	if (argc > 3 || argc < 2) {
		return luaL_error(L, "Options:Get can take at most 2 arguments: [section], key");
	}
	else if (argc == 3) {
		section = luaL_checkstring(L, 2);
		key = luaL_checkstring(L, 3);
	}
	else {
		section = "Options";
		key = luaL_checkstring(L, 2);
	}

	// Retrieve the value and push it.
	return CreateOptionValue(L, 1, section, key);
}

/* Options:Set([section = "Options", ]key, value)
 Sets a value to this key in this section. It does not bubble up!
*/
static int API_Options_Set(lua_State *L) {
	int argc = lua_gettop(L);
	const char *key, *section, *value;
	if (argc > 4 || argc < 3) {
		return luaL_error(L, "Options:Set takes 2 or 3 arguments: [section], key, value");
	}
	else if (argc == 4) {
		section = luaL_checkstring(L, 2);
		key = luaL_checkstring(L, 3);
		value = luaL_checkstring(L, 4);
	}
	else {
		section = "Options";
		key = luaL_checkstring(L, 2);
		value = luaL_checkstring(L, 3);
	}

	// Retrieve the C instance reference.
	CDataFile *options = GetOptions(L);

	// Set the value.
	options->SetValue(key, value, "", section);

	return 0;
}

/* Options.key
   Options.section.key
 Return or set a key nicely.
*/
static int API_Options__index(lua_State *L) {
	std::string str;
	str = luaL_checkstring(L, 2);
	if (luaL_getmetafield(L, 1, str.c_str())) return 1;
	return CreateOptionValue(L, 1, str.c_str(), NULL);
}

/*// MENU FUNCTIONS //*/
//static int API_CreateMenu(lua_State *L) {

//}

/*// REGISTRATION //*/
const luaL_Reg sharker_api[] = {
	{ "Log", API_LogMessage },
	{ "Options", API_CreateOptions },
	{ NULL, NULL }
};

const luaL_Reg sharker_options[] = {
	{ "Foster", API_Options_Foster },
	{ "Get", API_Options_Get },
	{ "Set", API_Options_Set },
	{ "__index", API_Options__index },
	{ NULL, NULL }
};

const luaL_Reg sharker_option_value[] = {
	// Accessing.
	{ "__index", API_Option_Value__index },
	{ "__newindex", API_Option_Value__newindex },

	// Numerical.
	{ "__add", API_Option_Value__add },
	{ "__sub", API_Option_Value__sub },
	{ "__mul", API_Option_Value__mul },
	{ "__div", API_Option_Value__div },
	{ "__mod", API_Option_Value__mod },
	{ "__pow", API_Option_Value__pow },
	{ "__unm", API_Option_Value__unm },
	{ "__lt", API_Option_Value__lt },
	{ "__le", API_Option_Value__le },

	// String.
	{ "__concat", API_Option_Value__concat },
	{ "__len", API_Option_Value__len },

	// Equivalence.
	{ "__eq", API_Option_Value__eq },
	{ NULL, NULL }
};

LUALIB_API int luaopen_sharker(lua_State *L) {
	// Create Options metatables.
	luaL_newmetatable(L, "Sharker.Options");
	luaL_setfuncs(L, sharker_options, 0);
	luaL_newmetatable(L, "Sharker.OptionValue");
	luaL_setfuncs(L, sharker_option_value, 0);

	// Register base API.
	lua_pushglobaltable(L);
	luaL_setfuncs(L, sharker_api, 0);
	return 1;
}