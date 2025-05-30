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
#include "./utils/logging.h"
#include "./utils/return.h"
#include "./utils/types.h"
#include "./debug/pprint.h"

#define JOIN_WORDS(LOW, HIGH) \
((u64)(((u64)(LOW) & 0xFFFFFFFFULL) | ((((u64)(HIGH)) & 0xFFFFFFFFULL) << 32ULL)))

#define SWAP_LOW_HIGH(VALUE) \
((u64)(((((u64)(VALUE)) & 0xFFFFFFFFULL) << 32ULL) | ((((u64)(VALUE)) >> 32ULL) & 0xFFFFFFFFULL)))

RECOMP_EXPORT const u32 recomp_api_version = 1;

static u32 counter = 0UL;

RECOMP_EXPORT void LuaLoader_Init(u8 *rdram, recomp_context *ctx) {
	char *str = NULL;
	if (try_get_array_argument(rdram, ctx, malloc, 0, &str, sizeof(char))) {
		LOG("str = \"%s\"", str);
		free(str);
	}

	lua_State *L = luaL_newstate();

	if (L != NULL) {
		luaL_openlibs(L);
	} else {
		LOG("Call to `luaL_newstate()` returned NULL!");
		return;
	}

	char bits[72];
	format_bits_u64((u64)L, bits, sizeof(bits));
	LOG("0x%016"PRIx64" || 0b%s || %20"PRIu64, (u64)L, bits, (u64)L);

	RETURN_INT(ctx, L);
}

RECOMP_EXPORT void LuaLoader_Deinit(u8 *rdram, recomp_context *ctx) {
	lua_State *L = (lua_State *)JOIN_WORDS(ctx->r4, ctx->r5);
	//LOG("lua_State *L = %p", L);

	if (L != NULL) {
		lua_close(L);
	} else {
		LOG("Expected `L` to be a pointer to `lua_State`, but got NULL instead!");
	}
}

typedef struct {
	u32 L_low;
	u32 L_high;
	u32 script_code;
	u32 script_code_size;
} LuaLoader_InvokeScriptCode_Args;

RECOMP_EXPORT void LuaLoader_InvokeScriptCode(u8 *rdram, recomp_context *ctx) {
	LuaLoader_InvokeScriptCode_Args *args_ptr =
		(LuaLoader_InvokeScriptCode_Args *)(rdram + (ctx->r4 & 0x7FFFFFFFULL));

	LOG("ctx->r4               = 0x%016"PRIx64, (u64)(ctx->r4));
	LOG("ctx->r4 (with offset) = 0x%08" PRIx32, (u32)(ctx->r4 & 0x7FFFFFFFULL));
	LOG("rdram                 = 0x%016"PRIx64, (u64)(rdram));
	LOG("args_ptr              = 0x%016"PRIx64, (u64)(args_ptr));

	LuaLoader_InvokeScriptCode_Args args = *args_ptr;

	LOG("&args                 = 0x%016"PRIx64, (u64)(&args));
	//LOG("args.L                = 0x%016"PRIx64, SWAP_LOW_HIGH(args.L));
	LOG("args.L_low / .L_high  = 0x%016"PRIx64, JOIN_WORDS(args.L_low, args.L_high));
	LOG("args.script_code      = 0x%08" PRIx32, args.script_code);
	LOG("args.script_code_size = %10"PRIu32, args.script_code_size);
	//LOG("args.script_code      = \"%s\"" , MEM_B(args.script_code, 0));

	//lua_State *L = (lua_State *)args.L;
	lua_State *L = (lua_State *)JOIN_WORDS(args.L_low, args.L_high);
	char *script_code = NULL;
	size_t script_code_size = (size_t)args.script_code_size;

	script_code = (char *)malloc(script_code_size + 1ULL);
	for (size_t i = 0ULL; i < script_code_size; i++) {
		const char *address = ((const char *)rdram) + (
			((i + (u64)args.script_code & 0x7FFFFFFFULL) ^ 3ULL)
		);
		char c = *address;
		script_code[i] = c;
	}
	script_code[script_code_size] = '\0';

	LOG("script_code = \"%s\"", script_code);
	LOG("script_code_size = %zu", script_code_size);

	/* if (script_code == NULL) return;
	if (script_code_size < 1) return;

	if (luaL_loadbufferx(L, script_code, script_code_size, "embedded_chunk", "t") != LUA_OK) {
		const char *error_message = lua_tostring(L, -1);
		fprintf(stderr, ">>> Lua load error: %s\n", (error_message != NULL) ? error_message : "Unknown error");
		//lua_close(L);
		//abort();
		return;
	}

	if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
		const char *error_message = lua_tostring(L, -1);
		fprintf(stderr, ">>> Lua runtime error: %s\n", (error_message != NULL) ? error_message : "Unknown error");
		//lua_close(L);
		//abort();
		return;
	} */

	free(script_code); script_code = NULL;
}
