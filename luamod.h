#ifndef JLS_LUA_MOD_H
#define JLS_LUA_MOD_H

#include <lua.h>
#include <lauxlib.h>

//#define JLS_LUA_MOD_TRACE 1

#ifdef JLS_LUA_MOD_TRACE
#include <stdio.h>
#define trace(...) printf(__VA_ARGS__)
#else
#define trace(...) ((void)0)
#endif

/*
 * All the API functions return nil in case of error following by an optional error message string.
 * It implies that functions must return at least one value and this value cannot be false or nil.
 */

/*
lua_pushboolean(l, 1); // true
lua_pushboolean(l, 0); // false
// to be used with assert
lua_pushnil(l);
lua_pushstring(l, "some error");
or
lua_pushfstring (l, "%s", strerror(errno));
*/

#define RETURN_SUCCESS(LS) lua_pushboolean(l, 1); \
	return 1;

#define RETURN_ERROR(LS, MSG) lua_pushnil(LS); \
	lua_pushstring(LS, MSG); \
	return 2;

#define SET_TABLE_KEY_STRING(LS, KEY, VALUE) \
	lua_pushstring(LS, KEY); \
	lua_pushstring(LS, VALUE); \
	lua_rawset(LS, -3);

#define SET_TABLE_KEY_INTEGER(LS, KEY, VALUE) \
	lua_pushstring(LS, KEY); \
	lua_pushinteger(LS, (lua_Integer) (VALUE)); \
	lua_rawset(LS, -3);

#define SET_TABLE_INDEX_STRING(LS, INDEX, VALUE) \
	lua_pushstring(LS, VALUE); \
	lua_rawseti(LS, -2, (lua_Integer) (INDEX));

#define b2s(b) ((b) ? "true" : "false")

#endif /* JLS_LUA_MOD_H */
