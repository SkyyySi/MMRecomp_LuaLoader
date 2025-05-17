#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__LIB_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__LIB_H_ 1

#include "modding.h"

RECOMP_IMPORT(".", void *LuaLoader_Init(void));
RECOMP_IMPORT(".", void LuaLoader_Deinit(void *L));
RECOMP_IMPORT(".", int LuaLoader_InvokeScriptCode(void *L, char **const script_code, size_t const script_code_size));

#endif
