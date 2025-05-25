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
		return 0; \
	} \
	for (size_t i = 0; i < length; i++) { \
		array[i] = (ITEM_TYPE)MEM_GETTER(arg_n64_ptr, i); \
	} \
	array[length] = 0; \
	*((ITEM_TYPE **)destination) = array; \
	break; \
}

/**
 * @brief Try to clone a byte-array from N64 RAM into host RAM.
 *
 * This function may read out-of-bounds if `length` is `0` and the array pointed
 * to by `array` is not `NULL`-terminated.
 *
 * @param[in] rdram A read-only view into the bytes of the virtual N64's memory.
 * @param[in] ctx A read-only view into the CPU state of the virtual N64.
 * @param[in] alloc_fn A pointer to a function that takes in a size in bytes and
 *                     returns a heap-allocated pointer to a memory block of the
 *                     given size, or `NULL` on error. Defaults to `malloc()`
 *                     from `<stdlib.h>` if `NULL` is passed instead of a
 *                     function pointer.
 * @param[in] array The location of the array in `rdram`, used as a pointer to
 *                  the first element of the target array. The source data will
 *                  not be modified.
 * @param[in] length The length of the target array. If set to `0`, the length
 *                   will instead be determined by counting until the first
 *                   `NULL`-byte has been found in the array. This is
 *                   particularly useful for `NULL`-terminated C-strings.
 * @param[out] destination A pointer to an uninitialized `u8` array. Typically,
 *                         you'll want to first create a local variable like
 *                         `u8 *data = NULL;` and then pass the address of that
 *                         local pointer into this function (so `destination`
 *                         should be `&data` here (not just `data`!)).
 * @return The number of bytes allocated by `alloc_fn`, or `0` on error.
 */
size_t try_get_array_u8(
		const u8 *restrict const rdram,
		const recomp_context *restrict const ctx,
		void *(*alloc_fn)(size_t size),
		gpr const array,
		size_t length,
		u8 **restrict const destination
) {
	if (rdram == NULL) {
		LOG("Invalid value of argument #1 `rdram`! (got NULL)");
		return 0;
	}

	if (ctx == NULL) {
		LOG("Invalid value of argument #2 `ctx`! (got NULL)");
		return 0;
	}

	if (array == 0) {
		LOG("Invalid value of argument #4 `array`! (got 0)");
		return 0;
	}

	if (destination == NULL) {
		LOG("Invalid value of argument #6 `destination`! (got NULL)");
		return 0;
	}

	if (alloc_fn == NULL) {
		alloc_fn = malloc;
	}

	if (length == 0) {
		while (MEM_BU(array, length)) {
			length++;
		}
	}

	size_t allocated_bytes = (length + 1) * sizeof(u8);
	u8 *array_native = (u8 *)alloc_fn(allocated_bytes);
	if (array_native == NULL) {
		LOG("Failed to allocate memory for destination array!");
		return 0;
	}

	for (size_t i = 0; i < length; i++) {
		array_native[i] = (u8)MEM_BU(array, i);
	}

	array_native[length] = 0;

	return allocated_bytes;
}

size_t try_get_array_argument(
		const u8 *restrict const rdram,
		const recomp_context *restrict const ctx,
		void *(*memory_allocator)(size_t size),
		u8 const argument_position,
		void *restrict const destination,
		u8 const item_type_size
) {
	if (rdram == NULL) {
		LOG("Invalid value of argument #1 `rdram`! (got NULL)");
		return 0;
	}

	if (ctx == NULL) {
		LOG("Invalid value of argument #2 `ctx`! (got NULL)");
		return 0;
	}

	gpr arg_n64_ptr = 0;
	switch (argument_position) {
		case 0: { arg_n64_ptr = ctx->r4; break; }
		case 1: { arg_n64_ptr = ctx->r5; break; }
		case 2: { arg_n64_ptr = ctx->r6; break; }
		case 3: { arg_n64_ptr = ctx->r7; break; }
		default: {
			LOG("Invalid value of argument #3 `argument_position`! (expected value in range [0, 3], got: %"PRIu8")", argument_position);
			return 0;
		}
	}

	if (memory_allocator == NULL) {
		memory_allocator = malloc;
	}

	if (destination == NULL) {
		LOG("Invalid value of argument #5 `destination`! (got NULL)");
		return 0;
	}

	size_t length = 0;
	switch (item_type_size) {
		case 1: TRY_GET_ARRAY_ARGUMENT_IMPL_HELPER__(s8,  MEM_B);
		case 2: TRY_GET_ARRAY_ARGUMENT_IMPL_HELPER__(s16, MEM_H);
		case 4: TRY_GET_ARRAY_ARGUMENT_IMPL_HELPER__(s32, MEM_W);
		default: {
			LOG("Invalid value of argument #7 `item_type_size`! (expected one of (1, 2, 4), got: %"PRIu8")", item_type_size);
			LOG("    -> arg_n64_ptr = 0x%016"PRIx64, arg_n64_ptr);
			return 0;
		}
	}

	return (length + 1) * item_type_size;
}

#endif
