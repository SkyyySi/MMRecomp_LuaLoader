#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__RETURN_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__RETURN_H_ 1

#include "../mod_recomp.h"
#include "./logging.h"
#include "./types.h"

#define BIT_CAST(FROM_TYPE, INTO_TYPE, VALUE) \
(((union { FROM_TYPE in; INTO_TYPE out; }){ .in = (VALUE) }).out)

// The general purpose register R2 contains the upper 32 bits of a function
// return value, while R3 contains the lower 32 bits.

static inline void return_u64(recomp_context *ctx, u64 value) {
	ctx->r2 = (gpr)((value >> 32) & 0xFFFFFFFF);
	ctx->r3 = (gpr)((value >>  0) & 0xFFFFFFFF);
}

static inline void return_u32(recomp_context *ctx, u32 value) {
	ctx->r2 = (gpr)value;
	ctx->r3 = (gpr)0;
}

static inline void return_u16(recomp_context *ctx, u16 value) {
	ctx->r2 = (gpr)value;
	ctx->r3 = (gpr)0;
}

static inline void return_u8(recomp_context *ctx, u8 value) {
	ctx->r2 = (gpr)value;
	ctx->r3 = (gpr)0;
}

static inline void return_s64(recomp_context *ctx, s64 value) {
	return_u64(ctx, BIT_CAST(s64, u64, value));
}

static inline void return_s32(recomp_context *ctx, s32 value) {
	return_u32(ctx, BIT_CAST(s32, u32, value));
}

static inline void return_s16(recomp_context *ctx, s16 value) {
	return_u16(ctx, BIT_CAST(s16, u16, value));
}

static inline void return_s8(recomp_context *ctx, s8 value) {
	return_u8(ctx, BIT_CAST(s8, u8, value));
}

static inline void return_f64(recomp_context *ctx, f64 value) {
	ctx->f0.d = value;
}

static inline void return_f32(recomp_context *ctx, f32 value) {
	ctx->f0.fl = value;
}

/**
 * @brief Return an integer value to the recomp.
 *
 * This can be used for any integer value
 */
#define RETURN_INT(RECOMP_CONTEXT, VALUE) { \
	u64 RETURN_INT__bit_casted_value__ = 0; \
	memcpy(&RETURN_INT__bit_casted_value__, &(VALUE), sizeof(VALUE)); \
	return_u64(RECOMP_CONTEXT, RETURN_INT__bit_casted_value__); \
	return; \
}

#endif
