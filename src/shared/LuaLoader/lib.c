#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "./lua/src/lua.h"
#include "./lua/src/lualib.h"
#include "./lua/src/lauxlib.h"

#include "./mod_recomp.h"

#include "./utils/arguments.h"
#include "./utils/array.h"
#include "./utils/logging.h"
#include "./utils/mem.h"
#include "./utils/return.h"
#include "./utils/types.h"
#include "./debug/pprint.h"

/* #define SWAP_LOW_HIGH(VALUE) \
((u64)(((((u64)(VALUE)) & 0xFFFFFFFFULL) << 32ULL) | ((((u64)(VALUE)) >> 32ULL) & 0xFFFFFFFFULL))) */

#define ASSERT(PREDICATE, ...) if (!(PREDICATE)) { \
	LOG("Assertion failed: %s", (#PREDICATE)); \
	LOG(__VA_ARGS__); \
	return; \
}

u64 join_low_high(u64 low, u64 high) {
	return (low & 0xFFFFFFFFULL) | ((high & 0xFFFFFFFFULL) << 32ULL);
}

RECOMP_EXPORT const u32 recomp_api_version = 1;

int RecompLua_call_game_func(lua_State *L) {
	return 0;
}

int RecompLua_rdram_index(lua_State *L) {
	//LOG("%s(L=0x%016"PRIx64")", __func__, (u64)L);
	u8 *rdram = lua_touserdata(L, 1);
	lua_Integer index = luaL_checkinteger(L, 2);

	//LOG("rdram.__index(self=0x%016llx, index=%lld)", (unsigned long long int)rdram, (signed long long int)index);

	// The recomp allows N64 code to use up to 512 MiB of RAM. Zero is not a
	// valid index because Lua uses one-based indexing.
	if ((index < 0x00000001) || (index > 0x20000000)) {
		return luaL_error(L, "Index out of range! (expected value in range [0x00000001, 0x20000000], got: %d)", index);
	}

	lua_pushinteger(L, rdram[((u64)(index - 1) ^ 3ULL) & 0x7FFFFFFFULL]);

	return 1;
}

int RecompLua_rdram_len(lua_State *L) {
	//LOG("%s(L=0x%016"PRIx64")", __func__, (u64)L);
	u8 *_rdram = lua_touserdata(L, 1);

	lua_pushinteger(L, 0x20000000);

	return 1;
}

int RecompLua_rdram_next(lua_State *L) {
	//LOG("%s(L=0x%016"PRIx64")", __func__, (u64)L);
	u8 *rdram = lua_touserdata(L, 1);
	lua_Integer index = luaL_optinteger(L, 2, 0);

	lua_Integer next_index = index + 1;

	if (next_index > 0x20000000) {
		return 0;
	}

	lua_pushinteger(L, next_index);
	lua_pushinteger(L, rdram[((u64)index ^ 3ULL) & 0x7FFFFFFFULL]);

	return 2;
}

int RecompLua_rdram_ipairs(lua_State *L) {
	//LOG("%s(L=0x%016"PRIx64")", __func__, (u64)L);
	u8 *rdram = lua_touserdata(L, 1);

	lua_pushcfunction(L, RecompLua_rdram_next);
	lua_pushlightuserdata(L, rdram);
	lua_pushinteger(L, 0);

	return 3;
}

int RecompLua_rdram_pairs(lua_State *L) {
	//LOG("%s(L=0x%016"PRIx64")", __func__, (u64)L);
	return RecompLua_rdram_ipairs(L);
}

RECOMP_EXPORT void LuaLoader_Init(u8 *rdram, recomp_context *ctx) {
	lua_State *L = luaL_newstate();
	ASSERT(L != NULL, "Call to `luaL_newstate()` returned NULL!");

	luaL_openlibs(L);

	lua_createtable(L, 0, 3); {
		lua_pushstring(L, "call_game_func");
		lua_pushcfunction(L, RecompLua_call_game_func);
		lua_rawset(L, -3);

		lua_pushstring(L, "rdram");
		lua_pushlightuserdata(L, rdram);
		lua_createtable(L, 0, 6); {
			lua_pushstring(L, "__name");
			lua_pushstring(L, "RecompLuaLoader::rdram");
			lua_rawset(L, -3);

			lua_pushstring(L, "__index");
			lua_pushcfunction(L, RecompLua_rdram_index);
			lua_rawset(L, -3);

			lua_pushstring(L, "__len");
			lua_pushcfunction(L, RecompLua_rdram_len);
			lua_rawset(L, -3);

			// Lua's `next` function doesn't actually check for a `__next`
			// meta-method (unlike how e.g. Python's `next` checks for the
			// presence of a `__next__` class attribute), but whatever.
			lua_pushstring(L, "__next");
			lua_pushcfunction(L, RecompLua_rdram_next);
			lua_rawset(L, -3);

			lua_pushstring(L, "__ipairs");
			lua_pushcfunction(L, RecompLua_rdram_ipairs);
			lua_rawset(L, -3);

			lua_pushstring(L, "__pairs");
			lua_pushcfunction(L, RecompLua_rdram_pairs);
			lua_rawset(L, -3);
		};
		lua_setmetatable(L, -2);
		lua_rawset(L, -3);

		/* lua_pushstring(L, "ctx");
		lua_pushlightuserdata(L, ctx);
		lua_createtable(L, 0, 0); {
			lua_pushstring(L, "__name");
			lua_pushstring(L, "RecompLuaLoader::ctx");
			lua_rawset(L, -3);

			//lua_pushstring(L, "__index");
			//lua_pushcfunction(L, RecompLua_ctx_index);
			//lua_rawset(L, -3);
		};
		lua_setmetatable(L, -2);
		lua_rawset(L, -3); */
	}; lua_setglobal(L, "Recomp");

	RETURN_INT(ctx, L);
}

RECOMP_EXPORT void LuaLoader_Deinit(u8 *rdram, recomp_context *ctx) {
	lua_State *L = (lua_State *)join_low_high(ctx->r5, ctx->r4);
	ASSERT(L != NULL, "Expected `L` to be a pointer to `lua_State`, but got NULL instead!");

	lua_close(L);
}

typedef struct {
	struct {
		u32 high;
		u32 low;
	} L;
	u32 script_code;
	u32 script_code_size;
} InvokeScriptCodeArgs;

void InvokeScriptHelper(lua_State *L, int run_status) {
	ASSERT(L != NULL, "Expected `L` to be a pointer to `lua_State`, but got NULL instead!");

	if (run_status != LUA_OK) {
		const char *error_message = lua_tostring(L, -1);
		if (error_message == NULL) error_message = "<unknown error>";
		LOG("Lua loading error:\n    %s", error_message);
		return;
	}

	if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
		const char *error_message = lua_tostring(L, -1);
		if (error_message == NULL) error_message = "<unknown error>";
		LOG("Lua runtime error:\n    %s", error_message);
		return;
	}
}

RECOMP_EXPORT void LuaLoader_InvokeScriptCode(u8 *rdram, recomp_context *ctx) {
	InvokeScriptCodeArgs args =
		*((InvokeScriptCodeArgs *)(rdram + (ctx->r4 & 0x7FFFFFFFULL)));

	lua_State *L = (lua_State *)join_low_high(args.L.low, args.L.high);
	ASSERT(L != NULL, "Expected `L` to be a pointer to `lua_State`, but got NULL instead!");

	size_t script_code_size = (size_t)args.script_code_size;
	ASSERT(
		(script_code_size & 0xFFFFFFFF00000000ULL) == 0ULL,
		"Argument `script_code_size` larger than should be possible! (got: 0x%016"PRIx64")",
		(u64)script_code_size
	);

	AUTO_FREE char *script_code = NULL;
	ASSERT(get_array(args.script_code, script_code_size, &script_code) > 0, "Failed to get script source code!");
	ASSERT(script_code != NULL, "Expected `script_code` to be a string, but got NULL instead!");

	return InvokeScriptHelper(L, luaL_loadbufferx(L, script_code, script_code_size, "script", "t"));
}

RECOMP_EXPORT void LuaLoader_InvokeScriptFile(u8 *rdram, recomp_context *ctx) {
	lua_State *L = (lua_State *)join_low_high(ctx->r5, ctx->r4);
	ASSERT(L != NULL, "Expected `L` to be a pointer to `lua_State`, but got NULL instead!");

	AUTO_FREE char *file_path_str = NULL;
	ASSERT(get_array(ctx->r6, 0, &file_path_str) > 0, "Failed to get path to script file!");
	ASSERT(file_path_str != NULL, "Expected `file_path_str` to be a string, but got NULL instead!");

	return InvokeScriptHelper(L, luaL_loadfilex(L, file_path_str, "t"));
}
