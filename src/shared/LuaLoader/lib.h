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

typedef struct {
	u64 L;
	const char *script_code;
	size_t script_code_size;
} LuaLoader_InvokeScriptCodeArgs;

RECOMP_IMPORT(".", u64 LuaLoader_Init(void));
RECOMP_IMPORT(".", void LuaLoader_Deinit(u64 L));
RECOMP_IMPORT(".", void LuaLoader_InvokeScriptCode(LuaLoader_InvokeScriptCodeArgs *args));
RECOMP_IMPORT(".", void LuaLoader_InvokeScriptFile(u64 L, const char *file_path_str));
RECOMP_IMPORT(".", void LuaLoader_DumpRDRAM(const char *file_path_str, bool include_tail_nulls));

#endif
