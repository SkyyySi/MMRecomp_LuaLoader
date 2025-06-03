#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lua/src/lua.h"
#include "../lua/src/lualib.h"
#include "../lua/src/lauxlib.h"

#ifdef FUNC
#error "The macro `FUNC` should not be declared here!"
#endif

#define FUNC_NAME(NAME) \
LuaLoader__RDRAM__##NAME

#define FUNC(NAME) \
static int FUNC_NAME(NAME)(lua_State *L)

// TODO: Rename this to just `get_length` and make the "total lenght" accessable
// through something like `get_capacity` (or just a static `capacity` property)
FUNC(get_occupied_length);
FUNC(get_data_as_string);
FUNC(get_raw_data_as_string);
FUNC(get_data_as_table);
FUNC(get_raw_data_as_table);

#undef FUNC
