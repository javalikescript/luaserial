#include "lua-compat/luamod.h"

#if LUA_VERSION_NUM < 503
#include "lua-compat/compat.h"
#endif

#if LUA_VERSION_NUM > 501

static int luaFileNoClose(lua_State *l) {
	luaL_Stream *p = (luaL_Stream *)luaL_checkudata(l, 1, LUA_FILEHANDLE);
  p->closef = &luaFileNoClose;
  lua_pushnil(l);
  lua_pushliteral(l, "cannot close standard file");
  return 2;
}

static int toLuaFile(lua_State *l) {
	int fd;
	const char* mode;
	fd = luaL_checkinteger(l, 1);
	mode = luaL_optstring(l, 2, "r");
  luaL_Stream *p = (luaL_Stream *)lua_newuserdata(l, sizeof(luaL_Stream));
  p->closef = &luaFileNoClose;
	p->f = fdopen(fd, mode);
  luaL_setmetatable(l, LUA_FILEHANDLE);
	return 1;
}

static int getFileDesc(lua_State *l, int arg) {
	int fd;
	luaL_Stream *pLuaStream;
  trace("getFileDesc()\n");
	pLuaStream = (luaL_Stream *)luaL_testudata(l, arg, LUA_FILEHANDLE);
	if (pLuaStream != NULL) {
		fd = fileno(pLuaStream->f);
	} else {
		fd = luaL_checkinteger(l, arg);
	}
	return fd;
}

#else

static int toLuaFile(lua_State *l) {
	int fd;
	const char* mode;
	fd = luaL_checkinteger(l, 1);
	mode = luaL_optstring(l, 2, "r");
  FILE **pf = (FILE **)lua_newuserdata(l, sizeof(FILE *));
  *pf = fdopen(fd, mode);
  luaL_getmetatable(l, "FILE*");
  lua_setmetatable(l, -2);
	return 1;
}

static int getFileDesc(lua_State *l, int arg) {
	int fd;
	FILE *f;
  trace("getFileDesc()\n");
  if (lua_isinteger(l, arg)) {
		fd = lua_tointeger(l, arg);
  } else {
    f = *(FILE **)luaL_checkudata(l, arg, "FILE*");
		fd = fileno(f);
  }
	return fd;
}

#endif

static int getIntegerField(lua_State *l, int i, const char *k, int def) {
  int v;
  lua_getfield(l, i, k);
  if (lua_isinteger(l, -1)) {
    v = (int) lua_tointeger(l, -1);
  } else {
    v = def;
  }
  lua_pop(l, 1);
  return v;
}

static int getBoolean(lua_State *l, int i, int def) {
  if (lua_isboolean(l, i)) {
    return lua_toboolean(l, i);
  }
  return def;
}

static int getBooleanField(lua_State *l, int i, const char *k, int def) {
  int v;
  lua_getfield(l, i, k);
  v = getBoolean(l, -1, def);
  lua_pop(l, 1);
  return v;
}

#ifdef WIN32
#include "luaserial_windows.c"
#else
#include "luaserial_linux.c"
#endif

// set_baud_rate set_data_bits set_dtr set_flow_control set_parity set_rts set_stop_bits

LUALIB_API int luaopen_serial(lua_State *l) {
  trace("luaopen_serial()\n");
  luaL_Reg reg[] = {
    { "getSerial", getSerial },
    { "setSerial", setSerial },
    { "setTimeout", setTimeout },
    //{ "cancelIo", cancelIo },
    { "flush", flush },
    { "waitDataAvailable", waitDataAvailable },
    { "available", available },
    { "toLuaFile", toLuaFile },
    { NULL, NULL }
  };
  lua_newtable(l);
  luaL_setfuncs(l, reg, 0);
  lua_pushliteral(l, "Lua serial");
  lua_setfield(l, -2, "_NAME");
  lua_pushliteral(l, "0.2");
  lua_setfield(l, -2, "_VERSION");
  trace("luaopen_serial() done\n");
  return 1;
}
