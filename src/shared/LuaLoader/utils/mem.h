#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__MEM_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__MEM_H_ 1

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>

#include "../mod_recomp.h"
#include "./types.h"

static void mem_get_array_s8(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	const size_t length,
	const s8 **restrict const destination
) {
	assert(rdram != NULL);
	assert(ctx != NULL);
	if (alloc_fn == NULL) alloc_fn = malloc;
	assert(array != NULL);
	assert(length > 0ULL);
	assert(destination != NULL);

	//assert(*destination == NULL);
	s8 *result = (s8 *)alloc_fn(length + 1ULL);
	assert(result != NULL);
	*destination = result;

	const uintptr_t array_corrected = (uintptr_t)(array & 0x7FFFFFFFULL);
	const s8 *rdram_casted = (const s8 *)rdram;
	for (size_t i = 0ULL; i < length; i++) {
		//rdram_casted[((array + i) ^ 3) & 0x7FFFFFFFULL];
		size_t index = (array_corrected + i) ^ 3;
		assert((index & 0xFFFFFFFF80000000ULL) == 0);
		rdram_casted[index];
	}
}

#endif
