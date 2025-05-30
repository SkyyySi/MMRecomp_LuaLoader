#pragma once

#ifndef __MOD_RECOMP_H__
#define __MOD_RECOMP_H__ 1

/**
 * @brief Core header for libraries that use native code instead of compiling
 * into N64 MIPS instructions, opening up access to calling arbitrary system
 * and C standard library functions (or any other shared library code).
 * 
 * It is required to use this header in any shared libraries that are compiled
 * into native host instructions ahead of time (instead of into MIPS like normal
 * mod code). If you don't use it, you'll only be able to call functions without
 * any arguments and without any return values, as those will be completely
 * incorrect. It acts as a bridge / translation layer between N64 mod code and
 * native / shared library code.
 *
 * This header can be thought of as "pseudo-assembly", see
 * https://n64brew.dev/wiki/MIPS_III_instructions for more details on how the
 * N64's instruction set works.
 *
 * @note In the code below, the suffixes "B", "H", "W" and "D" generally refer
 *       to integers with a size of 8, 16, 32 and 64 bits and are abbreviations
 *       for "byte", "halfword", "word" and "doubleword", respectively.
 *       Sometimes, "L" is used to refer to a 64-integer instead, referring to
 *       the more commonly used "long" in the C programming language.
 *       Additionally, "S" and "D" are also used to refer to 32-bit ("single-
 *       precision") and 64-bit ("double-precision") floating-point values.
 *
 * @note For the sake of simplicity, when the documentation below refers to
 *       something like an "N64 register" or the "N64 memory", it's refering to
 *       the recomp's "emulation" of these parts.
 */

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#if defined(_WIN32)
/**
 * @brief Mark a function as a member of your public API, allowing it to be
 *        called by mod code.
 *
 * A function marked with this macro is intended to be accessed from mod code by
 * importing it with `RECOMP_IMPORT` over there. Don't forget that you also need
 * to add its name to the `funcs` list of your shared library in `mod.toml`.
 */
#define RECOMP_EXPORT __declspec(dllexport)
#else
/**
 * @brief Mark a function as a member of your public API, allowing it to be
 *        called by mod code.
 *
 * A function marked with this macro is intended to be accessed from mod code by
 * importing it with `RECOMP_IMPORT` over there. Don't forget that you also need
 * to add its name to the `funcs` list of your shared library in `mod.toml`.
 */
#define RECOMP_EXPORT __attribute__((visibility("default")))
#endif

// Compiler definition to disable inter-procedural optimization, allowing multiple functions to be in a single file without breaking interposition.
#if defined(_MSC_VER) && !defined(__clang__)
    // MSVC's __declspec(noinline) seems to disable inter-procedural optimization entirely, so it's all that's needed.
    #define RECOMP_FUNC RECOMP_EXPORT __declspec(noinline)
#elif defined(__clang__)
    // Clang has no dedicated IPO attribute, so we use a combination of other attributes to give the desired behavior.
    // The inline keyword allows multiple definitions during linking, and extern forces clang to emit an externally visible definition.
    // Weak forces Clang to not perform any IPO as the symbol can be interposed, which prevents actual inlining due to the inline keyword.
    // Add noinline on for good measure, which doesn't conflict with the inline keyword as they have different meanings.
    #define RECOMP_FUNC RECOMP_EXPORT extern inline __attribute__((weak,noinline))
#elif defined(__GNUC__)
    // Use GCC's attribute for disabling inter-procedural optimizations.
    #define RECOMP_FUNC RECOMP_EXPORT __attribute__((noipa))
#else
    #error "No RECOMP_FUNC definition for this compiler"
#endif

/**
 * This type is used whereever an N64 instruction would try to use a "General
 * Purpose Register" (GPR) on real hardware. A value with a type of `gpr` can
 * be thought of as an integer value with an unknown size and signedness.
 * It may be a `u64`, but it may also be an `s32`, a `u8` or a pointer (which
 * could also be either 32 or 64 bits in size, since the N64 had both a 32-bit
 * and a 64-bit addressing mode).
 * @todo This should probably be renamed to `gpr_t`.
 */
typedef uint64_t gpr;

/**
 * @brief Cast a number into a signed 64-bit integer.
 * @param[in] val The value to convert.
 * @return `s64` The converted value.
 */
#define SIGNED(val) \
    ((int64_t)(val))

/**
 * @brief Perform an addition of two numbers and store the result as a signed
 *        32-bit integer into a general purpose register.
 * @param[in] a The first addend value.
 * @param[in] b The second addend value.
 * @return `gpr` The sum of `a` and `b` (the result of the computation).
 */
#define ADD32(a, b) \
    ((gpr)(int32_t)((a) + (b)))

/**
 * @brief Perform a subtraction of two numbers and store the result as a signed
 *        32-bit integer into a general purpose register.
 * @param[in] a The minuend value (the value that `b` is subtracted from).
 * @param[in] b The subtrahend value (the value to subtract from `a`).
 * @return `gpr` The difference between `a` and `b` (the result of the
 *               computation).
 */
#define SUB32(a, b) \
    ((gpr)(int32_t)((a) - (b)))

/**
 * @brief Load a signed 32-bit integer (a "word") from N64 memory.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `s32` The value stored in the N64 register `reg`.
 */
#define MEM_W(offset, reg) \
    (*(int32_t*)(rdram + ((((reg) + (offset))) & 0x7FFFFFFF)))

/**
 * @brief Load a signed 16-bit integer (a "halfword") from N64 memory.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `s16` The value stored in the N64 register `reg`.
 */
#define MEM_H(offset, reg) \
    (*(int16_t*)(rdram + ((((reg) + (offset)) ^ 2) & 0x7FFFFFFF)))

/**
 * @brief Load a signed 8-bit integer (a "byte") from N64 memory.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `s8` The value stored in the N64 register `reg`.
 */
#define MEM_B(offset, reg) \
    (*(int8_t*)(rdram + ((((reg) + (offset)) ^ 3) & 0x7FFFFFFF)))

/**
 * @brief Load an unsigned 16-bit integer (a "halfword") from N64 memory.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `u16` The value stored in the N64 register `reg`.
 */
#define MEM_HU(offset, reg) \
    (*(uint16_t*)(rdram + ((((reg) + (offset)) ^ 2) & 0x7FFFFFFF)))

/**
 * @brief Load an unsigned 8-bit integer (a "byte") from N64 memory.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `u8` The value stored in the N64 register `reg`.
 */
#define MEM_BU(offset, reg) \
    (*(uint8_t*)(rdram + ((((reg) + (offset)) ^ 3) & 0x7FFFFFFF)))

/**
 * @brief Stores a 64-bit integer (a "doubleword") value into an N64 register.
 * @param[in] val The value to store in `reg`. Since this value is always casted
 *                into `gpr`, it can be of any numeric type.
 * @param[out] offset `gpr` A pointer to the array where the recomp stores its
 *                          data for N64 registers in memory.
 * @param[out] reg `gpr` The number of the target register to write to, acting
 *                       as an index into the array at `offset`.
 * @return `void` Nothing.
 * @note This macro can only be used as a statement. Attempting to use it as an
 *       expression will result in a syntax error.
 */
#define SD(val, offset, reg) { \
    *(uint32_t*)(rdram + ((((reg) + (offset) + 4)) & 0x7FFFFFFF)) = (uint32_t)((gpr)(val) >> 0); \
    *(uint32_t*)(rdram + ((((reg) + (offset) + 0)) & 0x7FFFFFFF)) = (uint32_t)((gpr)(val) >> 32); \
}

/**
 * @brief Implementation helper for `LD`. Please use that macro instead.
 * @private
 * @deprecated Use `LD` instead.
 */
static inline uint64_t load_doubleword(uint8_t* rdram, gpr reg, gpr offset) {
    uint64_t ret = 0;
    uint64_t lo = (uint64_t)(uint32_t)MEM_W(reg, offset + 4);
    uint64_t hi = (uint64_t)(uint32_t)MEM_W(reg, offset + 0);
    ret = (lo << 0) | (hi << 32);
    return ret;
}

/**
 * @brief Loads a 64-bit integer (a "doubleword") value from an N64 register.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `u64` The loaded value.
 */
#define LD(offset, reg) \
    load_doubleword(rdram, offset, reg)

/**
 * @todo Test which part of the value is loaded by this function.
 * @brief Partially load a value from an N64 register.
 * @param[in] rdram Currently unused.
 * @param[in] initial_value TODO
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `s32` The loaded part of the value.
 */
static inline gpr do_lwl(uint8_t* rdram, gpr initial_value, gpr offset, gpr reg) {
    // Calculate the overall address
    gpr address = (offset + reg);

    // Load the aligned word
    gpr word_address = address & ~0x3;
    uint32_t loaded_value = MEM_W(0, word_address);

    // Mask the existing value and shift the loaded value appropriately
    gpr misalignment = address & 0x3;
    gpr masked_value = initial_value & ~(0xFFFFFFFFu << (misalignment * 8));
    loaded_value <<= (misalignment * 8);

    // Cast to int32_t to sign extend first
    return (gpr)(int32_t)(masked_value | loaded_value);
}

/**
 * @todo Test which part of the value is loaded by this function.
 * @brief Partially load a value from an N64 register.
 * @param[in] rdram Currently unused.
 * @param[in] initial_value TODO
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `s32` The loaded part of the value.
 */
static inline gpr do_lwr(uint8_t* rdram, gpr initial_value, gpr offset, gpr reg) {
    // Calculate the overall address
    gpr address = (offset + reg);
    
    // Load the aligned word
    gpr word_address = address & ~0x3;
    uint32_t loaded_value = MEM_W(0, word_address);

    // Mask the existing value and shift the loaded value appropriately
    gpr misalignment = address & 0x3;
    gpr masked_value = initial_value & ~(0xFFFFFFFFu >> (24 - misalignment * 8));
    loaded_value >>= (24 - misalignment * 8);

    // Cast to int32_t to sign extend first
    return (gpr)(int32_t)(masked_value | loaded_value);
}

/**
 * @todo Test which part of the value is stored by this function.
 * @brief Partially store a value into an N64 register.
 * @param[in] rdram Currently unused.
 * @param[out] offset `gpr` A pointer to the array where the recomp stores its
 *                          data for N64 registers in memory.
 * @param[out] reg `gpr` The number of the target register to write to, acting
 *                       as an index into the array at `offset`.
 * @param[in] val The value you want to store.
 * @return `void` Nothing.
 */
static inline void do_swl(uint8_t* rdram, gpr offset, gpr reg, gpr val) {
    // Calculate the overall address
    gpr address = (offset + reg);

    // Get the initial value of the aligned word
    gpr word_address = address & ~0x3;
    uint32_t initial_value = MEM_W(0, word_address);

    // Mask the initial value and shift the input value appropriately
    gpr misalignment = address & 0x3;
    uint32_t masked_initial_value = initial_value & ~(0xFFFFFFFFu >> (misalignment * 8));
    uint32_t shifted_input_value = ((uint32_t)val) >> (misalignment * 8);
    MEM_W(0, word_address) = masked_initial_value | shifted_input_value;
}

/**
 * @todo Test which part of the value is stored by this function.
 * @brief Partially store a value into an N64 register.
 * @param[in] rdram Currently unused.
 * @param[out] offset `gpr` A pointer to the array where the recomp stores its
 *                          data for N64 registers in memory.
 * @param[out] reg `gpr` The number of the target register to write to, acting
 *                       as an index into the array at `offset`.
 * @param[in] val The value you want to store.
 * @return `void` Nothing.
 */
static inline void do_swr(uint8_t* rdram, gpr offset, gpr reg, gpr val) {
    // Calculate the overall address
    gpr address = (offset + reg);

    // Get the initial value of the aligned word
    gpr word_address = address & ~0x3;
    uint32_t initial_value = MEM_W(0, word_address);

    // Mask the initial value and shift the input value appropriately
    gpr misalignment = address & 0x3;
    uint32_t masked_initial_value = initial_value & ~(0xFFFFFFFFu << (24 - misalignment * 8));
    uint32_t shifted_input_value = ((uint32_t)val) << (24 - misalignment * 8);
    MEM_W(0, word_address) = masked_initial_value | shifted_input_value;
}

/**
 * @brief Cast a number into a signed 32-bit integer.
 * @param[in] val The value to convert.
 * @return `s32` The converted value.
 */
#define S32(val) \
    ((int32_t)(val))

/**
 * @brief Cast a number into an unsigned 32-bit integer.
 * @param[in] val The value to convert.
 * @return `u32` The converted value.
 */
#define U32(val) \
    ((uint32_t)(val))

/**
 * @brief Cast a number into a signed 64-bit integer.
 * @param[in] val The value to convert.
 * @return `s64` The converted value.
 */
#define S64(val) \
    ((int64_t)(val))

/**
 * @brief Cast a number into an unsigned 64-bit integer.
 * @param[in] val The value to convert.
 * @return `u64` The converted value.
 */
#define U64(val) \
    ((uint64_t)(val))

/**
 * @brief Multiply two 32-bit ("single-precision") floating-point values.
 * @param[in] val1 `float` The first multiplier value.
 * @param[in] val1 `float` The second multiplier value.
 * @return `float` The product of multiplying `val1` with `val2`.
 */
#define MUL_S(val1, val2) \
    ((val1) * (val2))

/**
 * @brief Multiply two 64-bit ("double-precision") floating-point values.
 * @param[in] val1 `double` The first multiplier value.
 * @param[in] val1 `double` The second multiplier value.
 * @return `double` The product of multiplying `val1` with `val2`.
 */
#define MUL_D(val1, val2) \
    ((val1) * (val2))

/**
 * @brief Divide a 32-bit ("single-precision") floating-point value by another.
 * @param[in] val1 `float` The dividend value.
 * @param[in] val1 `float` The divisor value.
 * @return `float` The quotient value of dividing `val1` by `val2`.
 */
#define DIV_S(val1, val2) \
    ((val1) / (val2))

/**
 * @brief Divide a 64-bit ("double-precision") floating-point value by another.
 * @param[in] val1 `double` The dividend value.
 * @param[in] val1 `double` The divisor value.
 * @return `double` The quotient value of dividing `val1` by `val2`.
 */
#define DIV_D(val1, val2) \
    ((val1) / (val2))

/**
 * @brief Convert a signed 32-bit integer (a "word") into a 32-bit ("single-
 *        precision") floating-point value.
 * @param[in] val `s32` The value to convert.
 * @return `float` The converted value.
 */
#define CVT_S_W(val) \
    ((float)((int32_t)(val)))

/**
 * @brief Convert a signed 32-bit integer (a "word") into a 64-bit ("double-
 *        precision") floating-point value.
 * @param[in] val `s32` The value to convert.
 * @return `double` The converted value.
 */
#define CVT_D_W(val) \
    ((double)((int32_t)(val)))

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        64-bit ("double-precision") floating-point value.
 * @param[in] val `float` The value to convert.
 * @return `double` The converted value.
 */
#define CVT_D_S(val) \
    ((double)(val))

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        32-bit ("single-precision") floating-point value.
 * @param[in] val `double` The value to convert.
 * @return `float` The converted value.
 */
#define CVT_S_D(val) \
    ((float)(val))

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        32-bit integer (a "word") by truncating, thus cutting off any decimal
 *        places, rounding towards zero and losing some precision (especially if
 *        the value is very small or extremely large).
 * @param[in] val `float` The value to convert.
 * @return `s32` The converted value.
 */
#define TRUNC_W_S(val) \
    ((int32_t)(val))

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        32-bit integer (a "word") by truncating, thus cutting off any decimal
 *        places, rounding towards zero and losing some precision (especially if
 *        the value is very small or extremely large).
 * @param[in] val `double` The value to convert.
 * @return `s32` The converted value.
 */
#define TRUNC_W_D(val) \
    ((int32_t)(val))

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        64-bit integer (a "long") by truncating, thus cutting off any decimal
 *        places, rounding towards zero and losing some precision (especially if
 *        the value is very small or extremely large).
 * @param[in] val `float` The value to convert.
 * @return `s64` The converted value.
 */
#define TRUNC_L_S(val) \
    ((int64_t)(val))

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        64-bit integer (a "long") by truncating, thus cutting off any decimal
 *        places, rounding towards zero and losing some precision (especially if
 *        the value is very small or extremely large).
 * @param[in] val `double` The value to convert.
 * @return `s64` The converted value.
 */
#define TRUNC_L_D(val) \
    ((int64_t)(val))

/**
 * @brief A type representing the possible ways to round a floating-point value,
 *        converting it to an integer.
 */
typedef enum ModRecompRoundingMode {
    /**
     * @brief Performs mathemathically correct rounding to the nearest integer.
     */
    ModRecompRoundingMode_Nearest = 0U,
    /**
     * @brief Round towards zero. Rounds down for positive values and rounds up
     *        for negative values.
     * 
     * This is usually the fastest rounding method, as it can directly cast the
     * given floating-point value into an integer without calling any math
     * functions from the C standard library first.
     */
    ModRecompRoundingMode_Truncate = 1U,
    /**
     * @brief Round up towards positive infinity.
     */
    ModRecompRoundingMode_Ceiling = 2U,
    /**
     * @brief Round down towards negative infinity.
     */
    ModRecompRoundingMode_Floor = 3U,
} ModRecompRoundingMode;

/**
 * @brief A constant that you may use as a default when calling a function that
 *        requires you to explicitly pass a rounding mode flag when you don't
 *        care about the specifics.
 */
#define DEFAULT_ROUNDING_MODE ModRecompRoundingMode_Nearest

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        32-bit signed integer (a "word") using a manually specified rounding
 *        mode.
 *
 * You probably want to use the wrapper macro `CVT_W_S` instead of calling this
 * function directly.
 *
 * @param[in] val `float` The value to round.
 * @param[in] rounding_mode `ModRecompRoundingMode` The rounding mode to use.
 * @return `s32` The result of rounding `val` with the given `rounding_mode`.
 */
static inline int32_t do_cvt_w_s(float val, ModRecompRoundingMode rounding_mode) {
    switch (rounding_mode) {
        case ModRecompRoundingMode_Nearest:  return (int32_t)lroundf(val);
        case ModRecompRoundingMode_Truncate: return (int32_t)val;
        case ModRecompRoundingMode_Ceiling:  return (int32_t)ceilf(val);
        case ModRecompRoundingMode_Floor:    return (int32_t)floorf(val);
    }
    fprintf(stderr, "Invalid rounding mode! (ModRecompRoundingMode expected, got: %d)", (int)rounding_mode);
    assert(0);
    return 0;
}

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        32-bit signed integer (a "word") using the default rounding mode.
 *
 * @param[in] val `float` The value to round.
 * @return `s32` The result of rounding `val`.
 */
#define CVT_W_S(val) \
    do_cvt_w_s(val, rounding_mode)

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        32-bit signed integer (a "word") using a manually specified rounding
 *        mode.
 *
 * You probably want to use the wrapper macro `CVT_W_D` instead of calling this
 * function directly.
 *
 * @param[in] val `double` The value to round.
 * @param[in] rounding_mode `ModRecompRoundingMode` The rounding mode to use.
 * @return `s32` The result of rounding `val` with the given `rounding_mode`.
 */
static inline int32_t do_cvt_w_d(double val, ModRecompRoundingMode rounding_mode) {
    switch (rounding_mode) {
        case ModRecompRoundingMode_Nearest:  return (int32_t)lround(val);
        case ModRecompRoundingMode_Truncate: return (int32_t)val;
        case ModRecompRoundingMode_Ceiling:  return (int32_t)ceil(val);
        case ModRecompRoundingMode_Floor:    return (int32_t)floor(val);
    }
    fprintf(stderr, "Invalid rounding mode! (ModRecompRoundingMode expected, got: %d)", (int)rounding_mode);
    assert(0);
    return 0;
}

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        32-bit signed integer (a "word") using the default rounding mode.
 *
 * @param[in] val `double` The value to round.
 * @return `s32` The result of rounding `val`.
 */
#define CVT_W_D(val) \
    do_cvt_w_d(val, rounding_mode)

/**
 * @brief Check if a floating-point value is NaN (not a number), force stopping
 *        the game immediately and logging a message to the terminal if it is.
 * @param[in] val `float|double` The value to check.
 * @return `void` Nothing.
 */
#define NAN_CHECK(val) \
    assert(val == val)

/**
 * @brief A type which represents the data stored in an N64 floating-point
 *        register, allowing to access it as various different numeric types.
 * @todo This should probably be renamed to `fpr_t`.
 */
typedef union {
    double d;
    struct {
        float fl;
        float fh;
    };
    struct {
        uint32_t u32l;
        uint32_t u32h;
    };
    uint64_t u64;
} fpr;

/**
 * @brief A type which represents the entire state of the N64 processor.
 * @todo This should probably be renamed to `recomp_context_t`.
 */
typedef struct {
    /**
     * Constant zero register. Always reads as 0. Writes are ignored.
     */
    gpr r0;

    /**
     * Assembler temporary register. Not preserved across calls.
     */
    gpr r1;

    /**
     * Function return value or expression result. Also used as argument for
	 * leaf functions.
     */
    gpr r2;

    /**
     * Secondary return value register.
     */
    gpr r3;

    /**
     * First argument register, used to pass arguments to functions.
     */
    gpr r4;

    /**
     * Second argument register.
     */
    gpr r5;

    /**
     * Third argument register.
     */
    gpr r6;

    /**
     * Fourth argument register.
     */
    gpr r7;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r8;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r9;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r10;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r11;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r12;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r13;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r14;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r15;

    /**
     * Saved register. Preserved across function calls.
     */
    gpr r16;

    /**
     * Saved register. Preserved across function calls.
     */
    gpr r17;

    /**
     * Saved register. Preserved across function calls.
     */
    gpr r18;

    /**
     * Saved register. Preserved across function calls.
     */
    gpr r19;

    /**
     * Saved register. Preserved across function calls.
     */
    gpr r20;

    /**
     * Saved register. Preserved across function calls.
     */
    gpr r21;

    /**
     * Saved register. Preserved across function calls.
     */
    gpr r22;

    /**
     * Saved register. Preserved across function calls.
     */
    gpr r23;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r24;

    /**
     * Temporary register. Not preserved across function calls.
     */
    gpr r25;

    /**
     * Reserved for OS kernel. Do not use.
     */
    gpr r26;

    /**
     * Reserved for OS kernel. Do not use.
     */
    gpr r27;

    /**
     * Global pointer (gp) used for addressing static data.
     */
    gpr r28;

    /**
     * Stack pointer (sp) used to manage the call stack.
     */
    gpr r29;

    /**
     * Frame pointer (fp) or another saved register depending on calling
	 * convention.
     */
    gpr r30;

    /**
     * Return address (ra) used for storing the function return address.
     */
    gpr r31;

    /**
     * Floating-point register f0, often used for temporary values.
     */
    fpr f0;

    /**
     * Floating-point register f1, used for temporary computations.
     */
    fpr f1;

    /**
     * Floating-point register f2, used for intermediate floating-point results.
     */
    fpr f2;

    /**
     * Floating-point register f3, used for intermediate floating-point results.
     */
    fpr f3;

    /**
     * Floating-point register f4, used for intermediate floating-point results.
     */
    fpr f4;

    /**
     * Floating-point register f5, general-purpose FP temporary.
     */
    fpr f5;

    /**
     * Floating-point register f6, general-purpose FP temporary.
     */
    fpr f6;

    /**
     * Floating-point register f7, general-purpose FP temporary.
     */
    fpr f7;

    /**
     * Floating-point register f8, general-purpose FP temporary.
     */
    fpr f8;

    /**
     * Floating-point register f9, general-purpose FP temporary.
     */
    fpr f9;

    /**
     * Floating-point register f10, general-purpose FP temporary.
     */
    fpr f10;

    /**
     * Floating-point register f11, general-purpose FP temporary.
     */
    fpr f11;

    /**
     * Floating-point register f12, used for function argument or return value.
     */
    fpr f12;

    /**
     * Floating-point register f13, used for function argument or return value.
     */
    fpr f13;

    /**
     * Floating-point register f14, general-purpose FP value.
     */
    fpr f14;

    /**
     * Floating-point register f15, general-purpose FP value.
     */
    fpr f15;

    /**
     * Floating-point register f16, general-purpose FP value.
     */
    fpr f16;

    /**
     * Floating-point register f17, general-purpose FP value.
     */
    fpr f17;

    /**
     * Floating-point register f18, general-purpose FP value.
     */
    fpr f18;

    /**
     * Floating-point register f19, general-purpose FP value.
     */
    fpr f19;

    /**
     * Floating-point register f20, general-purpose FP value.
     */
    fpr f20;

    /**
     * Floating-point register f21, general-purpose FP value.
     */
    fpr f21;

    /**
     * Floating-point register f22, general-purpose FP value.
     */
    fpr f22;

    /**
     * Floating-point register f23, general-purpose FP value.
     */
    fpr f23;

    /**
     * Floating-point register f24, general-purpose FP value.
     */
    fpr f24;

    /**
     * Floating-point register f25, general-purpose FP value.
     */
    fpr f25;

    /**
     * Floating-point register f26, general-purpose FP value.
     */
    fpr f26;

    /**
     * Floating-point register f27, general-purpose FP value.
     */
    fpr f27;

    /**
     * Floating-point register f28, general-purpose FP value.
     */
    fpr f28;

    /**
     * Floating-point register f29, general-purpose FP value.
     */
    fpr f29;

    /**
     * Floating-point register f30, general-purpose FP value.
     */
    fpr f30;

    /**
     * Floating-point register f31, often used as a return value register or
     * temporary.
     */
    fpr f31;

    /**
     * The high and low registers are used for certain arithmetic operations
     * like integer multiplication, where the output may be way to large to fit
     * into a single 64-bit register. These are essentially used as if they were
     * a single, 128-bit-wide register.
    */
    uint64_t hi, lo;

    /**
     * Pointer to the odd-numbered halves of paired single-precision FPRs. Used
     * for operations in 32-bit mode.
     */
    uint32_t* f_odd;

    /**
     * @todo This should be made into an `enum`.
     * Status register of the CPU (coprocessor 0 status register). Controls
     * system-level state like interrupt masking, exception mode, etc.
     */
    uint32_t status_reg;

    /**
     * @todo This should be made into an `enum`.
     * Indicates whether the FPU is in 64-bit or 32-bit mode.
     * This affects how FP registers and operations are interpreted.
     */
    uint8_t mips3_float_mode;
} recomp_context;

/**
 * @todo Explain *why* this is here / why someone would actually need this.
 * @brief Check if the target is an even float register or that the MIPS3 float
 *        mode is enabled, force stopping the game immediately and logging a
 *        message to the terminal otherwise.
 * @param[in] ctx `recomp_context*` The current execution context of the recomp.
 * @param[in] idx `size_t` The index number for the register to check.
 * @return `void` Nothing.
 */
#define CHECK_FR(ctx, idx) \
    assert(((idx) & 1) == 0 || (ctx)->mips3_float_mode)

// Shouldn't this entire header be wrapped in an `extern "C"`-block for C++?
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @todo Document whether `rdram` is an `[in]` param or an `[inout]` param.
 * @todo Document whether `ctx` is an `[in]` param or an `[inout]` param.
 * @brief The type of a an N64 function.
 *
 * This type is used for function pointers that are passed from mod code to
 * native code. It's a pointer to the function from the native host's point
 * of view, rather than from the N64 code's / mod code's perspective (like what
 * you'd get by just taking a functions address inside an N64 code segment).
 *
 * Do note that `recomp_func_t` itself it not a pointer type.
 *
 * @param[in] rdram `u8*` A view into the current N64 RAM.
 * @param[in] ctx `recomp_context*` The current execution context of the recomp.
 * @return `void` Nothing.
 */
typedef void (recomp_func_t)(uint8_t* rdram, recomp_context* ctx);

/**
 * @todo Test if the returned pointer is nullable or not.
 * @todo Test what happens when `vram` is not set to a valid function address.
 * @todo Change the type of `vram` from `s32` to `u32` once it's been confirmed
 *       to be bug / typo (https://github.com/Zelda64Recomp/MMRecompModTemplate/issues/10).
 * @brief Get a pointer to a function in the virtual N64 memory.
 *
 * You probably want to use the wrapper macro `LOOKUP_FUNC` instead of calling this
 * function directly.
 *
 * @param[inout] vram `s32` An index into N64 memory where the desired function
 *                          is stored. This is the value you would get when
 *                          taking the function's address on the N64 code side.
 * @return `recomp_func_t*` A pointer to the target function that's usable by
 *                          native host code.
 */
extern RECOMP_EXPORT recomp_func_t* (*get_function)(int32_t vram);

/**
 * @todo Document whether `ctx` is an `[in]` param or an `[inout]` param.
 * @brief Write a value into the `SP_STATUS` register of the N64's RSP.
 *
 * This is a low-level mechanism for setting the current status of the N64's
 * "Reality Signal Processor" (RSP) (the part of the N64's architecture which
 * executes "microcode", if you ever head that term thrown around).
 *
 * See https://n64brew.dev/wiki/Reality_Signal_Processor/Interface#SP_STATUS
 * as well as https://n64brew.dev/wiki/Reality_Signal_Processor for more
 * information on how this works and what you can pass in for `value`.
 *
 * @param[in] ctx `recomp_context*` The current execution context of the recomp.
 * @param[in] value `gpr` The value to write into the `COP0` register.
 * @return `void` Nothing.
 */
extern RECOMP_EXPORT void (*cop0_status_write)(recomp_context* ctx, gpr value);

/**
 * @todo Document whether `ctx` is an `[in]` param or an `[inout]` param.
 * @brief Read a value from the `SP_STATUS` register of the N64's RSP.
 *
 * This is a low-level mechanism for getting the current status of the N64's
 * "Reality Signal Processor" (RSP) (the part of the N64's architecture which
 * executes "microcode", if you ever head that term thrown around).
 *
 * See https://n64brew.dev/wiki/Reality_Signal_Processor/Interface#SP_STATUS
 * as well as https://n64brew.dev/wiki/Reality_Signal_Processor for more
 * information on how this works and what you can pass in for `value`.
 *
 * @param[in] ctx `recomp_context*` The current execution context of the recomp.
 * @return `gpr` The current value stored in the `COP0` register.
 */
extern RECOMP_EXPORT gpr (*cop0_status_read)(recomp_context* ctx);

/**
 * @brief ???
 *
 * This is probably meant to be an error handler for when a `switch`-statement
 * failed to match any of its specified `case`s and no `default` was provided,
 * but I don't actually know. Ask @Mr-Wiseguy about it.
 *
 * @param[in] func `const char*` I'm guessing this supposed to be the name of
 *                               the calling function where the error occured.
 * @param[inout] vram `u32` An index into N64 memory. What does it point to? 🤷
 * @param[in] jtbl `u32` I'm guessing this is short for "jump table", so it's a
 *                       pointer to where `switch`-statement's jump table was
 *                       placed in memory by the compiler.
 * @return `void` Nothing.
 */
extern RECOMP_EXPORT void (*switch_error)(const char* func, uint32_t vram, uint32_t jtbl);

/**
 * @brief ???
 *
 * I'm guessing this is supposed to inject a breakpoint into the game or
 * something, but to be honest, I have no idea. Ask @Mr-Wiseguy about it.
 *
 * @param[inout] vram `u32` An index into N64 memory. What does it point to? 🤷
 * @return `void` Nothing.
 */
extern RECOMP_EXPORT void (*do_break)(uint32_t vram);

#define LOOKUP_FUNC(val) \
    get_function((int32_t)(val))

// Some of the comments below have been written by ChatGPT.

/**
 * @brief Reference base addresses for each section in the original / reference
 *        binary.
 *
 * This array points to the original section base addresses, used for comparison
 * or reference during relocation processing or recompiled code generation.
 */
extern RECOMP_EXPORT int32_t* reference_section_addresses;

/**
 * @brief Base addresses of recompiled sections.
 *
 * This array holds the relocated base addresses for each code or data section
 * in the recompiled binary.
 */
extern RECOMP_EXPORT int32_t section_addresses[];

/**
 * @brief Get the 16 least significant bits of a number.
 *
 * Used to obtain the immediate value for MIPS instructions like `ori` when
 * reconstructing a full 32-bit address in two parts.
 *
 * @param[in] x A numeric value, preferably an unsigned integer with a size of
 *              at least 32 bits to avoid things from getting "funky".
 * @return The lower 16 bits of `x`, stored as the same type as `x`.
 */
#define LO16(x) \
    ((x) & 0xFFFF)

/**
 * @brief Zero out the 15 least significant bits of a number (yes, 15, not 16).
 *
 * MIPS uses a trick when splitting 32-bit addresses to handle carry-over from
 * the low 16 bits. This macro computes the correct upper 16 bits for `lui` instructions.
 *
 * @param[in] x A numeric value, preferably an unsigned integer with a size of
 *              at least 16 bits to avoid things from getting "funky".
 * @return The upper bits of `x`, stored as the same type as `x`.
 * @note If `x` is an expression that cannot be statically evaluated at compile
 *       time (e.g. when it's a function call), you should assign it to a
 *       temporary variable before passing it to this macro, as it will need to
 *       be evaluated twice.
 */
#define HI16(x) \
    (((x) >> 16) + (((x) >> 15) & 1))

/**
 * @brief Computes the adjusted upper 16 bits of a relocated address for a given
 *        section.
 *
 * Used to generate the immediate value for a `lui` instruction in recompiled
 * MIPS code.
 *
 * @param[in] section_index `gpr` Index into `section_addresses` array.
 * @param[in] offset `gpr` Offset within the section.
 * @return `gpr` Adjusted upper 16 bits of the relocated address.
 */
#define RELOC_HI16(section_index, offset) \
    HI16(section_addresses[section_index] + (offset))

/**
 * @brief Extracts the lower 16 bits of a relocated address for a given section.
 *
 * Used to generate the immediate value for an `ori` instruction in recompiled MIPS code.
 *
 * @param[in] section_index `gpr` Index into `section_addresses` array.
 * @param[in] offset `gpr` Offset within the section.
 * @return `gpr` Lower 16 bits of the relocated address.
 */
#define RELOC_LO16(section_index, offset) \
    LO16(section_addresses[section_index] + (offset))

/**
 * @brief Computes the adjusted upper 16 bits of a reference (original) address
 *        for a section.
 *
 * Useful for validation or symbol resolution against the original binary layout.
 *
 * @param[in] section_index `gpr` Index into `reference_section_addresses` array.
 * @param[in] offset `gpr` Offset within the section.
 * @return `gpr` Adjusted upper 16 bits of the reference address.
 */
#define REF_RELOC_HI16(section_index, offset) \
    HI16(reference_section_addresses[section_index] + (offset))

/**
 * @brief Extracts the lower 16 bits of a reference (original) address for a
 *        section.
 *
 * Useful for validation or symbol resolution against the original binary
 * layout.
 *
 * @param[in] section_index `gpr` Index into `reference_section_addresses` array.
 * @param[in] offset `gpr` Offset within the section.
 * @return `gpr` Lower 16 bits of the reference address.
 */
#define REF_RELOC_LO16(section_index, offset) \
    LO16(reference_section_addresses[section_index] + (offset))

/**
 * @brief I'm guessing this function is called back when an attempt is made to
 *        invoke a system call from N64/mod code, but I have no idea if that's
 *        actually what it does.
 * @param[inout] rdram `uint8_t*` 
 * @param[in] ctx `recomp_context*` The current execution context of the recomp.
 * @param[in] instruction_vram `s32` 
 * @return `void` Nothing.
 */
void recomp_syscall_handler(uint8_t* rdram, recomp_context* ctx, int32_t instruction_vram);

/**
 * @brief I'm guessing this opens the recomp's pause menu (the dark-purple one)?
 * @param[inout] rdram `uint8_t*` 
 * @return `void` Nothing.
 */
void pause_self(uint8_t *rdram);

#ifdef __cplusplus
}
#endif

#endif
