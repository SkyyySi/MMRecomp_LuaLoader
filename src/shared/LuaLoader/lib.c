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

RECOMP_EXPORT void LuaLoader_Init(u8 *rdram, recomp_context *ctx) {
	lua_State *L = luaL_newstate();
	ASSERT(L != NULL, "Call to `luaL_newstate()` returned NULL!");

	luaL_openlibs(L);

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
