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

typedef union HostPointer {
	u64 u64;
	struct {
		u32 u32_le_low;
		u32 u32_le_high;
	};
	struct {
		u32 u32_be_high;
		u32 u32_be_low;
	};
} HostPointer;

#define HOST_PTR(_TYPE) HostPointer

typedef struct {
	HOST_PTR(lua_State) L;
	const char *script_code;
	size_t script_code_size;
} LuaLoader_InvokeScriptCodeArgs;

RECOMP_IMPORT(".", HOST_PTR(lua_State) LuaLoader_Init(void));
RECOMP_IMPORT(".", void LuaLoader_Deinit(HOST_PTR(lua_State) L));
RECOMP_IMPORT(".", void LuaLoader_InvokeScriptCode(LuaLoader_InvokeScriptCodeArgs *args));
RECOMP_IMPORT(".", void LuaLoader_InvokeScriptFile(HOST_PTR(lua_State) L, const char *file_path_str));

#endif
