#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__LIB_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__LIB_H_ 1

#include "modding.h"

typedef struct {
	u64 L;
	u64 script_code;
	u32 script_code_size;
} LuaLoader_InvokeScriptCode_Args;

RECOMP_IMPORT(".", void *LuaLoader_Init(void));
RECOMP_IMPORT(".", void LuaLoader_Deinit(void *L));
RECOMP_IMPORT(".", int LuaLoader_InvokeScriptCode(u32 args_ptr_low, u32 args_ptr_high));

#endif
