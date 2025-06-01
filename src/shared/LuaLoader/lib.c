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

#define RDRAM_LENGTH 0x20000000ULL
_Static_assert((RDRAM_LENGTH >= 4ULL), "");

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
	const char *name = luaL_checkstring(L, 1);
	lua_Integer vram = luaL_checkinteger(L, 2);
	lua_Integer size = luaL_checkinteger(L, 3);

	LOG("name = \"%s\"", name);
	LOG("vram = 0x%08llx", vram);
	LOG("size = %lld", size);

	lua_getglobal(L, "Recomp");
	lua_pushstring(L, "rdram");
	lua_rawget(L, -2);
	u8 *rdram = lua_touserdata(L, -1);

	//void (*func_ptr)(void) = (void *)(rdram + (u64)(vram & 0x7FFFFFFFULL));
	//LOG("func_ptr = 0x%016"PRIX64, (u64)func_ptr);
	//pprint_hexdump((u8 *)func_ptr, size);
	//LOG("LOOKUP_FUNC(0x%08llx) -> 0x%016"PRIX64, vram, (u64)LOOKUP_FUNC(vram));

	return 0;
}

size_t rdram_get_used_length(const u8 *restrict const rdram) {
	assert(rdram != NULL);

	size_t length = RDRAM_LENGTH;

	size_t index = (length - 1ULL) ^ 3ULL;
	while ((index >= 0) && (index < RDRAM_LENGTH) && (rdram[index] == 0)) {
		length--;
		index = (length - 1ULL) ^ 3ULL;
	}

	// Ensure that no underflow has occured above.
	assert(length <= RDRAM_LENGTH);

	return length /* | 3ULL */;
}

u8 *rdram_get_data_in_host_format(
		const u8 *restrict const rdram,
		void *(*alloc_fn)(size_t size),
		size_t *restrict const out_length
) {
	assert(rdram != NULL);
	if (alloc_fn == NULL) alloc_fn = malloc;
	if (out_length != NULL) *out_length = 0ULL;

	size_t length = rdram_get_used_length(rdram);

	u8 *rdram_converted = (u8 *)alloc_fn(length * sizeof(u8));
	if (rdram_converted == NULL) {
		return NULL;
	}

	// The bitwise XOR operation `z = x ^ y` could cause z to end up larger
	// than the actually allowed maximum index of `RDRAM_LENGTH - 1`. To avoid
	// having to check for this edge case in every loop cycle (which would mean
	// needlessly validating over five million indecies), the loop is split into
	// two separate parts - one where no out-of-bounds checks are needed and one
	// where they are used.
	size_t safe_length = length & ((RDRAM_LENGTH - 1ULL) );

	for (size_t i = 0ULL; i < safe_length; i++) {
		rdram_converted[i] = ((char *)rdram)[i ^ 3ULL];
	}

	for (size_t i = safe_length; i < length; i++) {
		size_t index = i ^ 3ULL;
		if (index >= length) continue;
		rdram_converted[i] = ((char *)rdram)[index];
	}

	if (out_length != NULL) *out_length = length;
	return rdram_converted;
}

int RecompLua_rdram_get_data_as_string(lua_State *L) {
	u8 *rdram = lua_touserdata(L, 1);

	size_t length = 0ULL;
	AUTO_FREE u8 *rdram_converted = rdram_get_data_in_host_format(rdram, malloc, &length);

	//LOG("length = %zu", length);
	//LOG("rdram_converted = 0x%016"PRIX64, (u64)rdram_converted);

	assert((rdram_converted != NULL) && (length != 0ULL));

	lua_pushlstring(L, (char *)rdram_converted, length);

	return 1;
}

int RecompLua_rdram_index(lua_State *L) {
	//LOG("%s(L=0x%016"PRIX64")", __func__, (u64)L);
	u8 *rdram = lua_touserdata(L, 1);

	int arg2_type = lua_type(L, 2);
	switch (arg2_type) {
		case LUA_TNUMBER: {
			lua_Integer index = luaL_checkinteger(L, 2);

			//LOG("rdram.__index(self=0x%016llx, index=%lld)", (unsigned long long int)rdram, (signed long long int)index);

			// The recomp allows N64 code to use up to 512 MiB of RAM. Zero is not a
			// valid index because Lua uses one-based indexing.
			if ((index < 0x00000001LL) || (index > RDRAM_LENGTH)) {
				return luaL_error(L, "Index out of range! (expected value in range [0x00000001, 0x20000000], got: %d)", index);
			}

			lua_pushinteger(L, rdram[((u64)(index - 1) ^ 3ULL) & 0x7FFFFFFFULL]);

			return 1;
		}
		case LUA_TSTRING: {
			luaL_error(L, "Not implemented yet!");
			return 1;
		}
	}

	return luaL_typeerror(L, 2, "integer or string");
}

int RecompLua_rdram_len(lua_State *L) {
	//LOG("%s(L=0x%016"PRIX64")", __func__, (u64)L);
	u8 *_rdram = lua_touserdata(L, 1);

	lua_pushinteger(L, RDRAM_LENGTH);

	return 1;
}

int RecompLua_rdram_next(lua_State *L) {
	//LOG("%s(L=0x%016"PRIX64")", __func__, (u64)L);
	u8 *rdram = lua_touserdata(L, 1);
	lua_Integer index = luaL_optinteger(L, 2, 0);

	lua_Integer next_index = index + 1;

	if (next_index > RDRAM_LENGTH) {
		return 0;
	}

	lua_pushinteger(L, next_index);
	lua_pushinteger(L, rdram[((u64)index ^ 3ULL) & 0x7FFFFFFFULL]);

	return 2;
}

int RecompLua_rdram_ipairs(lua_State *L) {
	//LOG("%s(L=0x%016"PRIX64")", __func__, (u64)L);
	u8 *rdram = lua_touserdata(L, 1);

	lua_pushcfunction(L, RecompLua_rdram_next);
	lua_pushlightuserdata(L, rdram);
	lua_pushinteger(L, 0);

	return 3;
}

int RecompLua_rdram_pairs(lua_State *L) {
	//LOG("%s(L=0x%016"PRIX64")", __func__, (u64)L);
	return RecompLua_rdram_ipairs(L);
}

RECOMP_EXPORT void LuaLoader_Init(u8 *rdram, RecompContext *ctx) {
	lua_State *L = luaL_newstate();
	ASSERT(L != NULL, "Call to `luaL_newstate()` returned NULL!");

	luaL_openlibs(L);

	lua_createtable(L, 0, 3); {
		lua_pushstring(L, "call_game_func");
		lua_pushcfunction(L, RecompLua_call_game_func);
		lua_rawset(L, -3);

		lua_pushstring(L, "rdram_get_data_as_string");
		lua_pushcfunction(L, RecompLua_rdram_get_data_as_string);
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

RECOMP_EXPORT void LuaLoader_Deinit(u8 *rdram, RecompContext *ctx) {
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

RECOMP_EXPORT void LuaLoader_InvokeScriptCode(u8 *rdram, RecompContext *ctx) {
	InvokeScriptCodeArgs args =
		*((InvokeScriptCodeArgs *)(rdram + (ctx->r4 & 0x7FFFFFFFULL)));

	lua_State *L = (lua_State *)join_low_high(args.L.low, args.L.high);
	ASSERT(L != NULL, "Expected `L` to be a pointer to `lua_State`, but got NULL instead!");

	size_t script_code_size = (size_t)args.script_code_size;
	ASSERT(
		(script_code_size & 0xFFFFFFFF00000000ULL) == 0ULL,
		"Argument `script_code_size` larger than should be possible! (got: 0x%016"PRIX64")",
		(u64)script_code_size
	);

	AUTO_FREE char *script_code = NULL;
	ASSERT(get_array(args.script_code, script_code_size, &script_code) > 0, "Failed to get script source code!");
	ASSERT(script_code != NULL, "Expected `script_code` to be a string, but got NULL instead!");

	return InvokeScriptHelper(L, luaL_loadbufferx(L, script_code, script_code_size, "script", "t"));
}

RECOMP_EXPORT void LuaLoader_InvokeScriptFile(u8 *rdram, RecompContext *ctx) {
	lua_State *L = (lua_State *)join_low_high(ctx->r5, ctx->r4);
	ASSERT(L != NULL, "Expected `L` to be a pointer to `lua_State`, but got NULL instead!");

	AUTO_FREE char *file_path_str = NULL;
	ASSERT(get_array(ctx->r6, 0, &file_path_str) > 0, "Failed to get path to script file!");
	ASSERT(file_path_str != NULL, "Expected `file_path_str` to be a string, but got NULL instead!");

	return InvokeScriptHelper(L, luaL_loadfilex(L, file_path_str, "t"));
}
