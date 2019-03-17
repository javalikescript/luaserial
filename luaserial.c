#include "luamod.h"

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
	//mode = luaL_checkstring(l, 2);
	mode = luaL_optstring(l, 2, "r");
  luaL_Stream *p = (luaL_Stream *)lua_newuserdata(l, sizeof(luaL_Stream));
  //p->closef = NULL;
  p->closef = &luaFileNoClose;
	p->f = fdopen(fd, mode);
  luaL_setmetatable(l, LUA_FILEHANDLE);
	return 1;
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
  lua_pushliteral(l, "0.1");
  lua_setfield(l, -2, "_VERSION");
  trace("luaopen_serial() done\n");
  return 1;
}
