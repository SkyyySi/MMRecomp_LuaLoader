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

#include "../mod_recomp.h"
#include "../utils/mem.h"
#include "../utils/types.h"

#include "rdram.h"

////////////////////////////////////////////////////////////////////////////////

#define TODO { \
	const char *func_name = __func__; \
	return luaL_error( \
		L, \
		"Function '%s' is not yet implemented!", \
		(func_name + sizeof("LuaLoader__RDRAM__") - 1) \
	); \
}

#define ASSERT(PREDICATE) \
if (!(PREDICATE)) { \
	return luaL_error(L, "Assertion failed: %s", #PREDICATE); \
}

#define RDRAM_INDEX(INDEX) ((((u64)(INDEX)) & 0x7FFFFFFFULL) ^ 3ULL)

////////////////////////////////////////////////////////////////////////////////

typedef LuaLoader__RDRAM Self;

////////////////////////////////////////////////////////////////////////////////

static lua_Integer rdram_count_length(const Self *restrict const self) {
	assert(self != NULL);

	const lua_Integer capacity = self->capacity;
	lua_Integer length = capacity;

	lua_Integer index = RDRAM_INDEX(length - 1ULL);
	while ((index >= 0) && (index < capacity) && (self->raw_data[index] == 0)) {
		length--;
		index = RDRAM_INDEX(length - 1ULL);
	}

	// Ensure that no underflow has occured above.
	assert(length <= capacity);

	return length;
}

static u8 *rdram_get_data(
		const Self *restrict const self,
		void *(*alloc_fn)(size_t size),
		lua_Integer *restrict const out_length
) {
	assert(self != NULL);
	if (alloc_fn == NULL) {
		alloc_fn = malloc;
	}
	if (out_length != NULL) {
		*out_length = 0LL;
	}

	u8 *rdram = self->raw_data;

	lua_Integer length = rdram_count_length(self);

	u8 *rdram_converted = (u8 *)alloc_fn(length * sizeof(u8));
	if (rdram_converted == NULL) {
		return NULL;
	}

	// The bitwise XOR operation `z = x ^ y` could cause z to end up larger
	// than the actually allowed maximum index of `self->capacity - 1`. To avoid
	// having to check for this edge case in every loop cycle (which would mean
	// needlessly validating over five million indecies), the loop is split into
	// two separate parts - one where no out-of-bounds checks are needed and one
	// where they are used.
	lua_Integer safe_length = length & ((self->capacity - 1LL) );

	for (lua_Integer i = 0ULL; i < safe_length; i++) {
		rdram_converted[i] = ((char *)rdram)[i ^ 3LL];
	}

	for (lua_Integer i = safe_length; i < length; i++) {
		lua_Integer index = i ^ 3LL;
		if (index >= length) continue;
		rdram_converted[i] = ((char *)rdram)[index];
	}

	if (out_length != NULL) {
		*out_length = length;
	}

	return rdram_converted;
}

////////////////////////////////////////////////////////////////////////////////

int LuaLoader__RDRAM__new(lua_State *L, u8 *rdram, lua_Integer capacity) {
	ASSERT(rdram != NULL);

	Self *self = (Self *)lua_newuserdata(L, sizeof(Self));
	luaL_setmetatable(L, LuaLoader__RDRAM__name);

	self->raw_data = rdram;
	self->capacity = capacity;

	return 1;
}


int LuaLoader__RDRAM__get_length(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	lua_pushinteger(L, rdram_count_length(self));
	return 1;
}

int LuaLoader__RDRAM__get_capacity(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	lua_pushinteger(L, self->capacity);
	return 1;
}

int LuaLoader__RDRAM__get_bytes_as_string(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	lua_Integer length = 0LL;
	u8 *rdram_converted = rdram_get_data(self, malloc, &length);
	lua_pushlstring(L, (char *)rdram_converted, length);
	return 1;
}

int LuaLoader__RDRAM__get_raw_bytes_as_string(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	lua_pushlstring(L, (char *)(self->raw_data), self->capacity);
	return 1;
}



#define CASE(VALUE, BLOCK) case (VALUE): { { BLOCK; }; break; }

/**
 * @param[in] L A pointer to the current Lua interpreter stack.
 * @param[in] self A pointer to the instance of `LuaLoader__RDRAM` to read from.
 * @param[in] index A 1-based index into `self->raw_data`. Must be in range
 *                  `1 <= index <= self->capacity`, `0` is a not valid index!
 * @param[in] type_size The size in bytes of the type `T` to interpret the data.
 *                      This should simply be `sizeof(T)`.
 */
static u64 read_value_helper(
		lua_State *L,
		const Self *restrict const self,
		const lua_Integer index,
		const int_fast8_t type_size
) {
	assert(L != NULL);
	ASSERT(self != NULL);
	ASSERT(index >= 1);
	ASSERT(index <= self->capacity);

	switch (type_size) {
		CASE(0, { return 0; });
		CASE(1, {
			TODO;
		});
		CASE(2, {
			TODO;
		});
		CASE(4, {
			TODO;
		});
		CASE(8, {
			/* uint64_t ret = 0;
			uint64_t lo = (*(int32_t *)(rdram + ((((offset + 4) + (reg))) & 0x7FFFFFFFULL)));
			uint64_t hi = (*(int32_t *)(rdram + ((((offset + 0) + (reg))) & 0x7FFFFFFFULL)));
			ret = (lo << 0) | (hi << 32);
			return ret; */

			// TODO: Test if the `& 0x7FFFFFFFULL` bitmask is redundant here
			// and, if it is, remove it.
			u32 low  = *(u32 *)(self->raw_data + ((index + 4) & 0x7FFFFFFFULL));
			u32 high = *(u32 *)(self->raw_data + ((index + 0) & 0x7FFFFFFFULL));

			return ((u64)low) | (((u64)high) << 32);
		});
	}

	return 0;
}

#define IMPL_READ_VALUE_METHOD(TYPENAME) \
int LuaLoader__RDRAM__read_value_##TYPENAME(lua_State *L) { \
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name); \
	TODO; \
	return 1; \
}

const size_t x = sizeof(u32);

IMPL_READ_VALUE_METHOD(s8)
IMPL_READ_VALUE_METHOD(s16)
IMPL_READ_VALUE_METHOD(s32)
IMPL_READ_VALUE_METHOD(s64)
IMPL_READ_VALUE_METHOD(u8)
IMPL_READ_VALUE_METHOD(u16)
IMPL_READ_VALUE_METHOD(u32)
IMPL_READ_VALUE_METHOD(u64)
IMPL_READ_VALUE_METHOD(f32)
IMPL_READ_VALUE_METHOD(f64)



#define IMPL_NEXT_PAIR_METHOD(TYPENAME) \
int LuaLoader__RDRAM__next_pair_##TYPENAME(lua_State *L) { \
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name); \
	TODO; \
	return 1; \
}

IMPL_NEXT_PAIR_METHOD(s8)
IMPL_NEXT_PAIR_METHOD(s16)
IMPL_NEXT_PAIR_METHOD(s32)
IMPL_NEXT_PAIR_METHOD(s64)
IMPL_NEXT_PAIR_METHOD(u8)
IMPL_NEXT_PAIR_METHOD(u16)
IMPL_NEXT_PAIR_METHOD(u32)
IMPL_NEXT_PAIR_METHOD(u64)
IMPL_NEXT_PAIR_METHOD(f32)
IMPL_NEXT_PAIR_METHOD(f64)



int LuaLoader__RDRAM__index(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	TODO;
	return 1;
}

int LuaLoader__RDRAM__newindex(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	TODO;
	return 1;
}

int LuaLoader__RDRAM__tostring(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);

	AUTO_FREE char *str = NULL;
	int str_length = asprintf(
		&str,
		"<userdata %s at 0x%016"PRIX64" { raw_data: 0x%016"PRIX64", capacity: 0x%08"PRIX64" }>",
		LuaLoader__RDRAM__name,
		(u64)self,
		(u64)(self->raw_data),
		(u64)(self->capacity)
	);

	if (str_length < 0) {
		return luaL_error(L, "Call to `asprintf()` failed! (return code: %d)", str_length);
	}

	lua_pushlstring(L, str, str_length);

	return 1;
}

int LuaLoader__RDRAM__len(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	TODO;
	return 1;
}

int LuaLoader__RDRAM__pairs(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	TODO;
	return 1;
}

int LuaLoader__RDRAM__ipairs(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	TODO;
	return 1;
}


int luaopen_rdram(lua_State *L) {
	if (luaL_newmetatable(L, LuaLoader__RDRAM__name)) {
		luaL_setfuncs(L, LuaLoader__RDRAM_meta_methods, 0);
	}

	lua_pushstring(L, "__index");
	lua_createtable(L, 0, sizeof(LuaLoader__RDRAM_methods) / sizeof(luaL_Reg) - 1);
	luaL_setfuncs(L, LuaLoader__RDRAM_methods, 0);

	/* lua_createtable(L, 0, 1);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, LuaLoader__RDRAM__index);
	lua_setmetatable(L, -2); */

	// Set the table containing normal / non-meta methods as the `__index` field
	// of the `LuaLoader::RDRAM` metatable. If the given key cannot be found in
	// this `__index`-table, `LuaLoader__RDRAM__index()` will be invoked.
	lua_rawset(L, -3);

	//lua_createtable(L, 0, 1);

	//lua_pushstring(L, "rdram");
	LuaLoader__RDRAM__new(L, malloc(0x20000000L), 0x20000000L);
	//lua_rawset(L, -3);

	return 1;
}
