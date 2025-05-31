#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__ARRAY_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__ARRAY_H_ 1

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../mod_recomp.h"
#include "./types.h"

static size_t get_array_char(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	size_t length,
	char **restrict const destination
) {
	if (!(rdram || ctx || array || destination)) return 0ULL;
	if (alloc_fn != NULL) alloc_fn = malloc;

	const gpr array_corrected = array & 0x7FFFFFFFULL;

	if (length == 0ULL) {
		while (((u8 *)rdram)[(array_corrected + length) ^ 3ULL]) {
			length++;
		}
	}

	size_t allocated_bytes = (length + 1ULL) * sizeof(char);
	char *result = (char *)alloc_fn(allocated_bytes);
	assert(result != NULL);
	*destination = result;
	result[length] = 0;

	for (size_t i = 0ULL; i < length; i++) {
		size_t index = (array_corrected + i) ^ 3ULL;
		assert((index & 0xFFFFFFFF80000000ULL) == 0ULL);
		result[i] = ((const char *)rdram)[index];
	}

	return allocated_bytes;
}

static size_t get_array_s8(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	const size_t length,
	s8 **restrict const destination
) {
	if (!(rdram || ctx || array || length || destination)) return 0ULL;
	if (alloc_fn != NULL) alloc_fn = malloc;

	size_t allocated_bytes = (length + 1ULL) * sizeof(s8);
	s8 *result = (s8 *)alloc_fn(allocated_bytes);
	assert(result != NULL);
	*destination = result;
	result[length] = 0;

	const gpr array_corrected = array & 0x7FFFFFFFULL;
	for (size_t i = 0ULL; i < length; i++) {
		size_t index = (array_corrected + i) ^ 3ULL;
		assert((index & 0xFFFFFFFF80000000ULL) == 0ULL);
		result[i] = ((const s8 *)rdram)[index];
	}

	return allocated_bytes;
}

static size_t get_array_u8(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	const size_t length,
	u8 **restrict const destination
) {
	if (!(rdram || ctx || array || length || destination)) return 0ULL;
	if (alloc_fn != NULL) alloc_fn = malloc;

	size_t allocated_bytes = (length + 1ULL) * sizeof(u8);
	u8 *result = (u8 *)alloc_fn(allocated_bytes);
	assert(result != NULL);
	*destination = result;
	result[length] = 0;

	const gpr array_corrected = array & 0x7FFFFFFFULL;
	for (size_t i = 0ULL; i < length; i++) {
		size_t index = (array_corrected + i) ^ 3ULL;
		assert((index & 0xFFFFFFFF80000000ULL) == 0ULL);
		result[i] = ((const u8 *)rdram)[index];
	}

	return allocated_bytes;
}

static size_t get_array_s16(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	const size_t length,
	s16 **restrict const destination
) {
	if (!(rdram || ctx || array || length || destination)) return 0ULL;
	if (alloc_fn != NULL) alloc_fn = malloc;

	size_t allocated_bytes = (length + 1ULL) * sizeof(s16);
	s16 *result = (s16 *)alloc_fn(allocated_bytes);
	assert(result != NULL);
	*destination = result;
	result[length] = 0;

	const gpr array_corrected = array & 0x7FFFFFFFULL;
	for (size_t i = 0ULL; i < length; i++) {
		size_t index = (array_corrected + i) ^ 2ULL;
		assert((index & 0xFFFFFFFF80000000ULL) == 0ULL);
		result[i] = ((const s16 *)rdram)[index];
	}

	return allocated_bytes;
}

static size_t get_array_u16(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	const size_t length,
	u16 **restrict const destination
) {
	if (!(rdram || ctx || array || length || destination)) return 0ULL;
	if (alloc_fn != NULL) alloc_fn = malloc;

	size_t allocated_bytes = (length + 1ULL) * sizeof(u16);
	u16 *result = (u16 *)alloc_fn(allocated_bytes);
	assert(result != NULL);
	*destination = result;
	result[length] = 0;

	const gpr array_corrected = array & 0x7FFFFFFFULL;
	for (size_t i = 0ULL; i < length; i++) {
		size_t index = (array_corrected + i) ^ 2ULL;
		assert((index & 0xFFFFFFFF80000000ULL) == 0ULL);
		result[i] = ((const u16 *)rdram)[index];
	}

	return allocated_bytes;
}

static size_t get_array_s32(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	const size_t length,
	s32 **restrict const destination
) {
	if (!(rdram || ctx || array || length || destination)) return 0ULL;
	if (alloc_fn != NULL) alloc_fn = malloc;

	size_t allocated_bytes = (length + 1ULL) * sizeof(s32);
	s32 *result = (s32 *)alloc_fn(allocated_bytes);
	assert(result != NULL);
	*destination = result;
	result[length] = 0;

	const gpr array_corrected = array & 0x7FFFFFFFULL;
	for (size_t i = 0ULL; i < length; i++) {
		size_t index = array_corrected + i;
		assert((index & 0xFFFFFFFF80000000ULL) == 0ULL);
		result[i] = ((const s32 *)rdram)[index];
	}

	return allocated_bytes;
}

static size_t get_array_u32(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	const size_t length,
	u32 **restrict const destination
) {
	if (!(rdram || ctx || array || length || destination)) return 0ULL;
	if (alloc_fn != NULL) alloc_fn = malloc;

	size_t allocated_bytes = (length + 1ULL) * sizeof(u32);
	u32 *result = (u32 *)alloc_fn(allocated_bytes);
	assert(result != NULL);
	*destination = result;
	result[length] = 0;

	const gpr array_corrected = array & 0x7FFFFFFFULL;
	for (size_t i = 0ULL; i < length; i++) {
		size_t index = array_corrected + i;
		assert((index & 0xFFFFFFFF80000000ULL) == 0ULL);
		result[i] = ((const u32 *)rdram)[index];
	}

	return allocated_bytes;
}

static size_t get_array_s64(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	const size_t length,
	s64 **restrict const destination
) {
	if (!(rdram || ctx || array || length || destination)) return 0ULL;
	if (alloc_fn != NULL) alloc_fn = malloc;

	size_t allocated_bytes = (length + 1ULL) * sizeof(s64);
	u64 *result = (u64 *)alloc_fn(allocated_bytes);
	assert(result != NULL);
	*destination = (s64 *)result;
	result[length] = 0;

	const gpr array_corrected = array & 0x7FFFFFFFULL;
	const u32 *rdram_u32 = (const u32 *)rdram;
	for (size_t i = 0ULL; i < length; i++) {
		size_t high_index = array_corrected + i;

		u64 high = (u64)(rdram_u32[high_index]);
		u64 low  = (u64)(rdram_u32[high_index + 4ULL]);

		result[i] = (low << 0ULL) | (high << 32ULL);
	}

	return allocated_bytes;
}

static size_t get_array_u64(
	const void *restrict const rdram,
	const recomp_context *restrict const ctx,
	void *(*alloc_fn)(size_t size),
	const gpr array,
	const size_t length,
	u64 **restrict const destination
) {
	if (!(rdram || ctx || array || length || destination)) return 0ULL;
	if (alloc_fn != NULL) alloc_fn = malloc;

	size_t allocated_bytes = (length + 1ULL) * sizeof(u64);
	u64 *result = (u64 *)alloc_fn(allocated_bytes);
	assert(result != NULL);
	*destination = result;
	result[length] = 0;

	const gpr array_corrected = array & 0x7FFFFFFFULL;
	const u32 *rdram_u32 = (const u32 *)rdram;
	for (size_t i = 0ULL; i < length; i++) {
		size_t high_index = array_corrected + i;

		u64 high = (u64)(rdram_u32[high_index]);
		u64 low  = (u64)(rdram_u32[high_index + 4ULL]);

		result[i] = (low << 0ULL) | (high << 32ULL);
	}

	return allocated_bytes;
}

#define get_array(array, length, destination) \
(_Generic((destination), \
	char **: get_array_char, \
	s8 **:   get_array_s8, \
	u8 **:   get_array_u8, \
	s16 **:  get_array_s16, \
	u16 **:  get_array_u16, \
	s32 **:  get_array_s32, \
	u32 **:  get_array_u32, \
	s64 **:  get_array_s64, \
	u64 **:  get_array_u64, \
	default: get_array_u8 \
)(rdram, ctx, malloc, (array), (length), (destination)))

#endif
