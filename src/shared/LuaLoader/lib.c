#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>

#include "./lua/src/lua.h"
#include "./lua/src/lualib.h"
#include "./lua/src/lauxlib.h"

#include "./mod_recomp.h"

#define LOG(...) { \
	fprintf(stderr, "\e[1;31m>>>\e[22;39m "); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\e[0m \e[1;35m<<<\e[22;39m \e[36m[\e[32m%s\e[39m() \e[35m::\e[39m \e[33m%s\e[35m:\e[34m%u\e[36m]\e[0m\n", (__func__), (__FILE__), (__LINE__)); \
}

#define LOG_TYPE_SIZE(TYPE) \
LOG("sizeof("#TYPE") = %zu", sizeof(TYPE));

/*
sizeof(char) = 1
sizeof(int) = 4
sizeof(long int) = 8
sizeof(size_t) = 8
sizeof(void*) = 8
sizeof(int8_t) = 1
sizeof(int16_t) = 2
sizeof(int32_t) = 4
sizeof(int64_t) = 8
sizeof(uint8_t) = 1
sizeof(uint16_t) = 2
sizeof(uint32_t) = 4
sizeof(uint64_t) = 8
*/

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
	u64 L;
	u64 script_code;
	u32 script_code_size;
} LuaLoader_InvokeScriptCode_Args;

RECOMP_EXPORT const u32 recomp_api_version = 1;

static void format_bits_u64(u64 x, char *out_buffer, unsigned long buffer_size) {
	if ((out_buffer == NULL) || (buffer_size < 72)) {
		return;
	}

	int offset = 0;
	for (int i = 0; i < 64; i++) {
		out_buffer[i + offset] = (x & (1ULL << (63 - i))) ? '1' : '0';

		if ((i > 0) && ((i % 8) == 7)) {
			offset++;
			out_buffer[i + offset] = '.';
		}
	}

	out_buffer[buffer_size - 1] = '\0';
}

static u32 counter = 0UL;

RECOMP_EXPORT u32 LuaLoader_Init(void) {
	//LOG("called %s()", __func__);

	/* lua_State *L = luaL_newstate();

	if (L != NULL) {
		luaL_openlibs(L);
	} else {
		LOG("Call to `luaL_newstate()` returned NULL!");
	} */

	char bits[72];

	u32 L = counter++;
	format_bits_u64(L, bits, sizeof(bits));
	LOG("0x%016"PRIX32" || 0b%s || %20"PRIu32, L, bits, L);

	//LOG("%s() returned (u32)%"PRIu32, __func__, L);

	return L;
}

RECOMP_EXPORT void LuaLoader_Deinit(u64 L_as_u64) {
	LOG("called %s(L_as_u64=%"PRIu64")", __func__, L_as_u64);
	lua_State *L = (lua_State *)L_as_u64;
	LOG("lua_State *L = %p", L);

	if (L != NULL) {
		//lua_close(L);
	} else {
		LOG("Expected `L` to be a pointer to `lua_State`, but got NULL instead!");
	}

	LOG("%s() returned (void)", __func__);
}

/* lua_State *L, char **const script_code, size_t const script_code_size */
RECOMP_EXPORT s32 LuaLoader_InvokeScriptCode(u32 args_ptr_low, u32 args_ptr_high) {
	LOG("called %s(args_ptr_low=%"PRIu32", args_ptr_high=%"PRIu32")", __func__, args_ptr_low, args_ptr_high);

	u64 args_ptr_full = ((u64)(args_ptr_low)) | (((u64)(args_ptr_high)) << 32ULL);
	LOG("args_ptr_full = %"PRIu64, args_ptr_full);

	LuaLoader_InvokeScriptCode_Args *args_ptr = (LuaLoader_InvokeScriptCode_Args *)args_ptr_full;
	LOG("args_ptr = %p", args_ptr);

	/* lua_State *L;
	u8 **const script_code;
	u32 const script_code_size;

	LOG("script_code = %p", script_code);
	//printf(">>> %s -> script_code = %p\n", __func__, script_code);
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
	} */

	return 0;
}
