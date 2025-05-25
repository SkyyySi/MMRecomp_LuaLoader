#pragma once

#ifndef HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__TYPES_H_
#define HEADER_GUARD__SRC__SHARED__LUA_LOADER__UTILS__TYPES_H_ 1

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

/*
sizeof(char) = 1
sizeof(int) = 4
sizeof(long int) = 8
sizeof(size_t) = 8
sizeof(void*) = 8
sizeof(int8_t) = 1
sizeof(int16_t) = 2
sizeof(int32_t) = 4
sizeof(int64_t) = 8
sizeof(uint8_t) = 1
sizeof(uint16_t) = 2
sizeof(uint32_t) = 4
sizeof(uint64_t) = 8
*/

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

#endif
