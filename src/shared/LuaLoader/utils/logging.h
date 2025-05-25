#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__LOGGING_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__LOGGING_H_ 1

#include <stdio.h>

#include "./types.h"

#define LOG(...) { \
	fprintf(stderr, "\e[1;31m>>>\e[22;39m "); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\e[0m \e[1;35m<<<\e[22;39m \e[36m[\e[32m%s\e[39m() \e[35m::\e[39m \e[33m%s\e[35m:\e[34m%u\e[36m]\e[0m\n", (__func__), (__FILE__), (__LINE__)); \
}

#define LOG_TYPE_SIZE(TYPE) \
LOG("sizeof("#TYPE") = %zu", sizeof(TYPE));

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

#endif
