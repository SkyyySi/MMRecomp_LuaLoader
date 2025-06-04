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

#define ASSERT(PREDICATE, ...) \
if (!(PREDICATE)) { \
	return luaL_error(L, "Assertion failed: %s", #PREDICATE __VA_OPT__(,) __VA_ARGS__); \
}

#define RDRAM_INDEX(INDEX) ((((u64)(INDEX)) & 0x7FFFFFFFULL) ^ 3ULL)

////////////////////////////////////////////////////////////////////////////////

typedef LuaLoader__RDRAM Self;

////////////////////////////////////////////////////////////////////////////////

static void display_lua_string(lua_State *L, int stack_index) {
	size_t size = 0ULL;
	const char *str = lua_tolstring(L, stack_index, &size);

	fputc('\"', stderr);

	for (size_t i = 0ULL; i < size; i++) {
		u8 c = ((const u8 *)str)[i];

		// https://en.wikipedia.org/wiki/Control_Pictures
		if ((c < 0x20) || (c == 0x7F)) {
			fputc(0xE2, stderr);
			fputc(0x90, stderr);
			fputc(0x80 + c, stderr);
			fprintf(stderr, "\\x%02"PRIX8, c);
			continue;
		}

		if ((c == '\\') || (c == '\"') || (c == '\'')) {
			fputc('\\', stderr);
		}

		fputc(c, stderr);
	}

	fputc('\"', stderr);

	fflush(stderr);
}

static void display_lua_stack(lua_State *L) {
	int top_index = lua_gettop(L);

	fprintf(stderr, "Lua stack at 0x%016"PRIX64", top value first:\n", (u64)L);

	if (top_index < 1) {
		fputs("    The stack is empty.\n", stderr);
		fflush(stderr);
		return;
	}

	for (int stack_index = top_index; stack_index >= 1; stack_index--) {
		int type_id = lua_type(L, stack_index);
		const char *type_name = lua_typename(L, type_id);

		fprintf(stderr, "    [%d | -%d] (%s): ", stack_index, top_index - stack_index + 1, type_name);
		switch (type_id) {
			case LUA_TNIL: {
				fputs("nil", stderr);
				break;
			}
			case LUA_TBOOLEAN: {
				fputs(lua_toboolean(L, stack_index) ? "true" : "false", stderr);
				break;
			}
			case LUA_TNUMBER: {
				if (lua_isinteger(L, stack_index)) {
					fprintf(stderr, "%"PRIi64"", (s64)lua_tointeger(L, stack_index));
				} else {
					fprintf(stderr, "%.15g", lua_tonumber(L, stack_index));
				}
				break;
			}
			case LUA_TSTRING: {
				display_lua_string(L, stack_index);
				break;
			}
			case LUA_TLIGHTUSERDATA:
			case LUA_TTABLE:
			case LUA_TFUNCTION:
			case LUA_TUSERDATA:
			case LUA_TTHREAD:
			default: {
				fprintf(stderr, "0x%016"PRIX64"", (u64)lua_topointer(L, stack_index));
				break;
			}
		}
		fputc('\n', stderr);
		fflush(stderr);
	}
}

////////////////////////////////////////////////////////////////////////////////

__attribute__((__nonnull__))
__attribute__((__optimize__("-Ofast", "-ffast-math")))
static lua_Integer rdram_count_length(const Self *restrict const self) {
	assert(self != NULL);

	const lua_Integer capacity = self->capacity;
	lua_Integer length = capacity;
	//fprintf(stderr, "length = %lld, capacity = %lld\n", length, capacity);

	const u8 *raw_data = self->raw_data;

	/* lua_Integer raw_length = capacity;
	while ((raw_length > 0) && (raw_data[--raw_length] == 0)) {
		continue;
	}

	lua_Integer index = raw_length | 3; */
	lua_Integer index = RDRAM_INDEX(length - 1);
	while ((index >= 0) && (index < capacity) && (raw_data[index] == 0)) {
		length--;
		index = RDRAM_INDEX(length - 1);
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

int LuaLoader__RDRAM__get_data_as_string(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	lua_Integer length = 0LL;
	u8 *rdram_converted = rdram_get_data(self, malloc, &length);
	lua_pushlstring(L, (char *)rdram_converted, length);
	return 1;
}

int LuaLoader__RDRAM__get_raw_data_as_string(lua_State *L) {
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
	switch (lua_type(L, 2)) {
		CASE(LUA_TNUMBER, {
			lua_Integer index = luaL_checkinteger(L, 2);

			size_t real_index = RDRAM_INDEX(index - 1);
			//fprintf(stderr, ">>> real_index = 0x%08zX\n", real_index);

			// The recomp allows N64 code to use up to 512 MiB of RAM. Zero is not a
			// valid index because Lua uses one-based indexing.
			ASSERT(
				(index > 0) && (index <= self->capacity),
				"Index out of range! (expected value in range [1, %d], got: %d)",
				self->capacity,
				index
			);

			lua_pushinteger(L, self->raw_data[RDRAM_INDEX(index - 1)]);

			return 1;
		});
		CASE(LUA_TSTRING, {
			/* size_t key_length = 0ULL;
			const char *key = luaL_checklstring(L, 2, &key_length); */

			ASSERT(lua_getmetatable(L, 1) != 0); // top: table `debug.getmetatable(self)` (metatable of argument #1)
			lua_pushstring(L, "__methods__"); // top: string "__methods__"
			ASSERT(lua_rawget(L, -2) == LUA_TTABLE); // top: table `rawget(debug.getmetatable(self), "__methods__")`
			lua_rotate(L, 2, 2); // top: string `key` (argument #2)
			lua_rawget(L, -2); // top: function or nil `rawget(debug.getmetatable(self), "__methods__")[key]`

			return 1;
		});
	}

	return luaL_typeerror(L, 2, "integer or string");
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
	return LuaLoader__RDRAM__get_length(L);
}

static int next_method_pair(lua_State *L) {
	luaL_Reg *methods = lua_touserdata(L, 1);
	ASSERT(methods != NULL);
	const char *key = luaL_optstring(L, 2, NULL);

	if (key == NULL) {
		if (methods[0].name == NULL) {
			return 0;
		}

		lua_pushstring(L, methods[0].name);
		lua_pushcfunction(L, methods[0].func);

		return 2;
	}

	for (size_t i = 0; methods[i].name != NULL; i++) {
		if (methods[i + 1].name == NULL) {
			break;
		}

		if (strcmp(key, methods[i].name) == 0) {
			lua_pushstring(L, methods[i + 1].name);
			lua_pushcfunction(L, methods[i + 1].func);

			return 2;
		}
	}

	return 0;
}

int LuaLoader__RDRAM__pairs(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);

	lua_pushcfunction(L, next_method_pair);
	lua_pushlightuserdata(L, (void *)LuaLoader__RDRAM_methods);
	lua_pushnil(L);

	return 3;
}

int LuaLoader__RDRAM__ipairs(lua_State *L) {
	Self *self = luaL_checkudata(L, 1, LuaLoader__RDRAM__name);
	TODO;
	return 1;
}

////////////////////////////////////////////////////////////////////////////////

int luaopen_rdram(lua_State *L) {
	if (luaL_newmetatable(L, LuaLoader__RDRAM__name)) {
		luaL_setfuncs(L, LuaLoader__RDRAM_meta_methods, 0);

		lua_pushstring(L, "__methods__");
		lua_createtable(L, 0, sizeof(LuaLoader__RDRAM_methods) / sizeof(luaL_Reg) - 1);
		luaL_setfuncs(L, LuaLoader__RDRAM_methods, 0);
		lua_rawset(L, -3);
	}

	/* lua_pushstring(L, "__index"); */

	/* lua_createtable(L, 0, 1); {
		lua_pushstring(L, "__index");
		lua_pushcfunction(L, LuaLoader__RDRAM__index);
		lua_rawset(L, -3);
	}; lua_setmetatable(L, -2); */

	// Set the table containing normal / non-meta methods as the `__index` field
	// of the `LuaLoader::RDRAM` metatable. If the given key cannot be found in
	// this `__index`-table, `LuaLoader__RDRAM__index()` will be invoked.
	/* lua_rawset(L, -3); */

	//lua_createtable(L, 0, 1);

	size_t rdram_length = 0x20000000ULL;
	u8 *rdram = (u8 *)malloc(rdram_length * sizeof(u8));

	FILE *rdram_dump_file = fopen("./rdram-dump.bin", "rb");
	if (rdram_dump_file != NULL) {
		fread(rdram, sizeof(u8), rdram_length, rdram_dump_file);
		fclose(rdram_dump_file);
	}

	//lua_pushstring(L, "rdram");
	LuaLoader__RDRAM__new(L, rdram, rdram_length);
	//lua_rawset(L, -3);

	return 1;
}
