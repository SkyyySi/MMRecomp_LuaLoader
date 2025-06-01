#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__DEBUG__PPRINT_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__DEBUG__PPRINT_H_ 1

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>

#include "../lua/src/lua.h"
#include "../lua/src/lualib.h"
#include "../lua/src/lauxlib.h"

static void pprint_hexdump(const uint8_t *restrict data, size_t data_size) {
	printf("Hexdump for 0x%016"PRIx64":\n", (uint64_t)data);
	printf("       | 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f | 0 1 2 3 4 5 6 7 8 9 a b c d e f\n");
	printf("-------+-------------------------------------------------+--------------------------------\n");

	if (data_size == 0) {
		return;
	}

	for (size_t i = 0ULL; i < data_size; i++) {
		if (i % 16 == 0) {
			printf("0x%04zx | ", i);
		}

		uint8_t byte = data[i];
		if (byte) {
			printf("%02"PRIx8, data[i]);
		} else {
			printf("\e[2m00\e[22m");
		}

		if (i % 16 == 15) {
			printf(" |");
			for (int j = 15; j >= 0; j--) {
				uint8_t byte = data[i - j];
				if ((32 <= byte) && (byte <= 126)) {
					printf(" %c", byte);
				} else {
					printf(" \e[2m.\e[22m");
				}
			}
			printf("\n");
		} else {
			printf(" ");
		}
	}

	printf("\n");
}

static void pprint_lua_string(lua_State *L, int stack_index) {
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

static void pprint_lua_stack(lua_State *L) {
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
				pprint_lua_string(L, stack_index);
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

#endif
