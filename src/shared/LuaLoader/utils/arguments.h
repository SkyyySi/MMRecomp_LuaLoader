#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__ARGUMENTS_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__ARGUMENTS_H_ 1

#include "../mod_recomp.h"
#include "./logging.h"
#include "./types.h"

#define TRY_GET_ARRAY_ARGUMENT_IMPL_HELPER__(ITEM_TYPE, MEM_GETTER) { \
	while (MEM_GETTER(arg_n64_ptr, length)) length++; \
	ITEM_TYPE *array = (ITEM_TYPE *)memory_allocator((length + 1) * sizeof(ITEM_TYPE)); \
	if (array == NULL) { \
		LOG("Failed to allocate memory for destination array!"); \
		return false; \
	} \
	for (size_t i = 0; i < length; i++) { \
		array[i] = (ITEM_TYPE)MEM_GETTER(arg_n64_ptr, i); \
	} \
	array[length] = 0; \
	*((ITEM_TYPE **)destination) = array; \
	break; \
}

bool try_get_array_argument(
		const u8 *restrict const rdram,
		const recomp_context *restrict const ctx,
		void *(*memory_allocator)(size_t size),
		u8 const argument_position,
		void *restrict const destination,
		size_t *restrict const allocated_bytes,
		u8 const item_type_size
) {
	if (allocated_bytes) *allocated_bytes = 0;

	if (rdram == NULL) {
		LOG("Invalid value of argument #1 `rdram`! (got NULL)");
		return false;
	}

	if (ctx == NULL) {
		LOG("Invalid value of argument #2 `ctx`! (got NULL)");
		return false;
	}

	gpr arg_n64_ptr = 0;
	switch (argument_position) {
		case 0: { arg_n64_ptr = ctx->r4; break; }
		case 1: { arg_n64_ptr = ctx->r5; break; }
		case 2: { arg_n64_ptr = ctx->r6; break; }
		case 3: { arg_n64_ptr = ctx->r7; break; }
		default: {
			LOG("Invalid value of argument #3 `argument_position`! (expected value in range [0, 3], got: %"PRIu8")", argument_position);
			return false;
		}
	}

	if (memory_allocator == NULL) {
		memory_allocator = malloc;
	}

	if (destination == NULL) {
		LOG("Invalid value of argument #5 `destination`! (got NULL)");
		return false;
	}

	size_t length = 0;
	switch (item_type_size) {
		case 1: TRY_GET_ARRAY_ARGUMENT_IMPL_HELPER__(s8,  MEM_B);
		case 2: TRY_GET_ARRAY_ARGUMENT_IMPL_HELPER__(s16, MEM_H);
		case 4: TRY_GET_ARRAY_ARGUMENT_IMPL_HELPER__(s32, MEM_W);
		case 8: {
			LOG("arg_n64_ptr = 0x%016"PRIx64, arg_n64_ptr);
			return false;
		}
		default: {
			LOG("Invalid value of argument #7 `item_type_size`! (expected one of (1, 2, 4), got: %"PRIu8")", item_type_size);
			return false;
		}
	}

	if (allocated_bytes) *allocated_bytes = length + 1;
	return true;
}

#endif
