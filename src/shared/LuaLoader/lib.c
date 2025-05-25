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

RECOMP_EXPORT const u32 recomp_api_version = 1;

static u32 counter = 0UL;

RECOMP_EXPORT void LuaLoader_Init(u8 *rdram, recomp_context *ctx) {
	char *str = NULL;
	if (try_get_array_argument(rdram, ctx, malloc, 0, &str, NULL, sizeof(char))) {
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

#define JOIN_WORDS(LOW, HIGH) \
(((u64)(u32)(LOW)) | (((u64)(u32)(HIGH)) << ((u64)32)))

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
	u64 L;
	u32 script_code;
	u32 script_code_size;
} LuaLoader_InvokeScriptCode_Args;

RECOMP_EXPORT void LuaLoader_InvokeScriptCode(u8 *rdram, recomp_context *ctx) {
	LuaLoader_InvokeScriptCode_Args *args_ptr =
		(LuaLoader_InvokeScriptCode_Args *)(rdram + (ctx->r4 - 0xFFFFFFFF80000000ULL));

	LOG("args_ptr              = 0x%016"PRIx64, (u64)(args_ptr));

	LuaLoader_InvokeScriptCode_Args args = *args_ptr;

	LOG("&args                 = 0x%016"PRIx64, (u64)(&args));
	LOG("args.L                = 0x%016"PRIx64, args.L);
	LOG("args.script_code      = 0x%08" PRIx32, args.script_code);
	LOG("args.script_code_size = 0x%08" PRIx32, args.script_code_size);
	//LOG("args.script_code      = \"%s\"" , MEM_B(args.script_code, 0));

	lua_State *L = (lua_State *)args.L;
	char *script_code = NULL;
	size_t script_code_size = (size_t)args.script_code_size;

	script_code = (char *)malloc(script_code_size + 1);
	for (size_t i = 0; i <= script_code_size; i++) {
		//char c = (char)MEM_B(args.script_code, i);
		char c = rdram[(u64)args.script_code - 0xFFFFFFFF80000000ULL + i];
		script_code[i] = c;
		LOG("script_code[%zu] = 0x%02hhx '%c'", i, c, c);
	}
	script_code[script_code_size] = 0;

	LOG("script_code = \"%s\"", script_code);
	LOG("script_code_size = %"PRIu32, script_code_size);

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
}
