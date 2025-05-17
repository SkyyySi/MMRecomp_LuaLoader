#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>

#include "./lua/src/lua.h"
#include "./lua/src/lualib.h"
#include "./lua/src/lauxlib.h"

#define DLLEXPORT __attribute__((visibility("default")))

DLLEXPORT const uint32_t recomp_api_version = 1;

DLLEXPORT void *LuaLoader_Init(void) {
	fprintf(stderr, ">>> %s()\n", __func__);
	lua_State *L = luaL_newstate();
	if (L == NULL) return NULL;
	luaL_openlibs(L);
	return L;
}

DLLEXPORT void LuaLoader_Deinit(lua_State *L) {
	fprintf(stderr, ">>> %s(L=%p)\n", __func__, L);
	if (L == NULL) return;
	lua_close(L);
}

DLLEXPORT int LuaLoader_InvokeScriptCode(lua_State *L, char **const script_code, size_t const script_code_size) {
	printf(">>> %s -> script_code = %p\n", __func__, script_code);
	fprintf(stderr, ">>> %s(L=%p, *script_code=\"%s\", script_code_size=%zu)\n", __func__, L, *script_code, script_code_size);

	if (script_code == NULL) return 3;
	if (*script_code == NULL) return 3;
	if (script_code_size < 1) return 3;

	// Load the buffer as a Lua chunk
	if (luaL_loadbufferx(L, *script_code, script_code_size, "embedded_chunk", "t") != LUA_OK) {
		const char *error_message = lua_tostring(L, -1);
		fprintf(stderr, ">>> Lua load error: %s\n", (error_message != NULL) ? error_message : "Unknown error");
		//lua_close(L);
		//abort();
		return 1;
	}

	// Execute the chunk
	if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
		const char *error_message = lua_tostring(L, -1);
		fprintf(stderr, ">>> Lua runtime error: %s\n", (error_message != NULL) ? error_message : "Unknown error");
		//lua_close(L);
		//abort();
		return 2;
	}

	return 0;
}
