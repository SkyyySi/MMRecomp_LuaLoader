#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__LUA_EXPORTS__RDRAM_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__LUA_EXPORTS__RDRAM_H_ 1

////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lua/src/lua.h"
#include "../lua/src/lualib.h"
#include "../lua/src/lauxlib.h"

#include "../mod_recomp.h"
#include "../utils/types.h"

////////////////////////////////////////////////////////////////////////////////

typedef struct LuaLoader__RDRAM {
	u8 *raw_data;
	lua_Integer capacity;
} LuaLoader__RDRAM;

#define LuaLoader__RDRAM__name "LuaLoader::RDRAM"

int LuaLoader__RDRAM__new(lua_State *L, u8 *rdram, lua_Integer capacity) __attribute__((__nonnull__));

int LuaLoader__RDRAM__get_length(lua_State *L) __attribute__((__nonnull__)); // formerly `get_occupied_length`
int LuaLoader__RDRAM__get_capacity(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__get_data_as_string(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__get_raw_data_as_string(lua_State *L) __attribute__((__nonnull__));
//int LuaLoader__RDRAM__get_data_as_table(lua_State *L) __attribute__((__nonnull__));
//int LuaLoader__RDRAM__get_raw_data_as_table(lua_State *L) __attribute__((__nonnull__));

int LuaLoader__RDRAM__read_value_s8(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__read_value_s16(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__read_value_s32(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__read_value_s64(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__read_value_u8(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__read_value_u16(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__read_value_u32(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__read_value_u64(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__read_value_f32(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__read_value_f64(lua_State *L) __attribute__((__nonnull__));

int LuaLoader__RDRAM__next_pair_s8(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__next_pair_s16(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__next_pair_s32(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__next_pair_s64(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__next_pair_u8(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__next_pair_u16(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__next_pair_u32(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__next_pair_u64(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__next_pair_f32(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__next_pair_f64(lua_State *L) __attribute__((__nonnull__));

int LuaLoader__RDRAM__index(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__newindex(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__tostring(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__len(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__pairs(lua_State *L) __attribute__((__nonnull__));
int LuaLoader__RDRAM__ipairs(lua_State *L) __attribute__((__nonnull__));

////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg LuaLoader__RDRAM_methods[] = {
	{ "get_length",             LuaLoader__RDRAM__get_length             },
	{ "get_capacity",           LuaLoader__RDRAM__get_capacity           },
	{ "get_data_as_string",     LuaLoader__RDRAM__get_data_as_string     },
	{ "get_raw_data_as_string", LuaLoader__RDRAM__get_raw_data_as_string },
	//{ "get_data_as_table",      LuaLoader__RDRAM__get_data_as_table     },
	//{ "get_raw_data_as_table",  LuaLoader__RDRAM__get_raw_data_as_table },
	{ "read_value_s8",          LuaLoader__RDRAM__read_value_s8          },
	{ "read_value_s16",         LuaLoader__RDRAM__read_value_s16         },
	{ "read_value_s32",         LuaLoader__RDRAM__read_value_s32         },
	{ "read_value_s64",         LuaLoader__RDRAM__read_value_s64         },
	{ "read_value_u8",          LuaLoader__RDRAM__read_value_u8          },
	{ "read_value_u16",         LuaLoader__RDRAM__read_value_u16         },
	{ "read_value_u32",         LuaLoader__RDRAM__read_value_u32         },
	{ "read_value_u64",         LuaLoader__RDRAM__read_value_u64         },
	{ "read_value_f32",         LuaLoader__RDRAM__read_value_f32         },
	{ "read_value_f64",         LuaLoader__RDRAM__read_value_f64         },
	{ "next_pair_s8",           LuaLoader__RDRAM__next_pair_s8           },
	{ "next_pair_s16",          LuaLoader__RDRAM__next_pair_s16          },
	{ "next_pair_s32",          LuaLoader__RDRAM__next_pair_s32          },
	{ "next_pair_s64",          LuaLoader__RDRAM__next_pair_s64          },
	{ "next_pair_u8",           LuaLoader__RDRAM__next_pair_u8           },
	{ "next_pair_u16",          LuaLoader__RDRAM__next_pair_u16          },
	{ "next_pair_u32",          LuaLoader__RDRAM__next_pair_u32          },
	{ "next_pair_u64",          LuaLoader__RDRAM__next_pair_u64          },
	{ "next_pair_f32",          LuaLoader__RDRAM__next_pair_f32          },
	{ "next_pair_f64",          LuaLoader__RDRAM__next_pair_f64          },
	{ NULL,                     NULL                                     },
};

static const luaL_Reg LuaLoader__RDRAM_meta_methods[] = {
	{ "__index",    LuaLoader__RDRAM__index    },
	{ "__newindex", LuaLoader__RDRAM__newindex },
	{ "__tostring", LuaLoader__RDRAM__tostring },
	{ "__len",      LuaLoader__RDRAM__len      },
	{ "__pairs",    LuaLoader__RDRAM__pairs    },
	{ "__ipairs",   LuaLoader__RDRAM__ipairs   },
	{ NULL,         NULL                       },
};

////////////////////////////////////////////////////////////////////////////////

#endif
