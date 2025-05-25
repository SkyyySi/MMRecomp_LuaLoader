#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__LIB_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__LIB_H_ 1

/**
 * Do NOT include this header in `lib.c` or any other native code! It is meant
 * for use in mod code ONLY!
 */

#include <stdint.h>

#include "modding.h"
#include "global.h"

/**
 * @brief A host pointer to a `lua_State` instance.
 */
typedef u64 Lua;

typedef struct {
	//union {
	//	Lua L;
		u32 L_low, L_high;
	//};
	const char *script_code;
	size_t script_code_size;
} LuaLoader_InvokeScriptCode_Args;

/* RECOMP_IMPORT(".", void LuaLoader_Init(unsigned char *rdram, void *ctx));
RECOMP_IMPORT(".", void LuaLoader_Deinit(unsigned char *rdram, void *ctx));
RECOMP_IMPORT(".", void LuaLoader_InvokeScriptCode(unsigned char *rdram, void *ctx)); */

RECOMP_IMPORT(".", Lua LuaLoader_Init(void));
//RECOMP_IMPORT(".", void LuaLoader_Deinit(Lua L));
RECOMP_IMPORT(".", void LuaLoader_Deinit(u32 L_low, u32 L_high));
//RECOMP_IMPORT(".", void LuaLoader_InvokeScriptCode(Lua L, const char *restrict script_code, size_t script_code_size));
RECOMP_IMPORT(".", void LuaLoader_InvokeScriptCode(LuaLoader_InvokeScriptCode_Args *args));

#endif
