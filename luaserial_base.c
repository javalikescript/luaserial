#include "luamod.h"

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
