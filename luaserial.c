#if LUA_VERSION_NUM < 503
#include "lua-compat/compat.h"
#endif

#include "lua-compat/luamod.h"
#include "lua-compat/file.h"

#if LUA_VERSION_NUM > 501

// from liolib.c
static int luaFileNoClose(lua_State *l) {
  luaL_Stream *p = (luaL_Stream *)luaL_checkudata(l, 1, LUA_FILEHANDLE);
  p->closef = &luaFileNoClose;
  lua_pushnil(l);
  lua_pushliteral(l, "cannot close fd file");
  return 2;
}
#define INIT_UDATA_FILE(_UDATA) (((luaL_Stream *)_UDATA)->closef = &luaFileNoClose)

#else

#define INIT_UDATA_FILE(_UDATA)

#endif

static int toLuaFile(lua_State *l) {
  int fd = luaL_checkinteger(l, 1);
  const char* mode = luaL_optstring(l, 2, "r");
  FILE_UDATA_TYPE* pFileUData = (FILE_UDATA_TYPE*)lua_newuserdata(l, sizeof(FILE_UDATA_TYPE));
  INIT_UDATA_FILE(pFileUData);
  REF_UDATA_FILE(pFileUData) = fdopen(fd, mode);
  luaL_getmetatable(l, FILE_UDATA_NAME);
  lua_setmetatable(l, -2);
  return 1;
}

static int getFileDesc(lua_State *l, int arg) {
  void *pFileUData = luaL_testudata(l, arg, FILE_UDATA_NAME);
  if (pFileUData != NULL) {
    return fileno(REF_UDATA_FILE(pFileUData));
  }
  return luaL_checkinteger(l, arg);
}

static int toFileNo(lua_State *l) {
  int fd = getFileDesc(l, 1);
  lua_pushinteger(l, fd);
  return 1;
}

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
    { "toFileNo", toFileNo },
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
