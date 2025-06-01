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
 * Purpose Register" (GPR) on real hardware. A value with a type of `RecompGPR`
 * can be thought of as an integer value with an unknown size and signedness.
 *
 * In general, this cannot hold 64-bit values (despite being implemented as a
 * `u64`), since the N64 is typically not used in 64-bit-register mode. When a
 * 64-bit value is stored, it will instead be split up into a low and high part,
 * where the high part gets written into the specified register (let's call it
 * `X`), while the low part is placed into the GPR `X + 1`. For example, passing
 * a 64-bit value as the first argument to an N64 function will place the upper
 * 32 bits into `gpr4`, while `gpr5` holds the lower / less significant 32 bits.
 *
 * And yes, to be clear, it really is this way around. It may sound like `X + 1`
 * should hold the higher bits and `X` the lower bits, but that's just what
 * happens when you try to move data across the boundry of two completely
 * different architecutures, especially when one is little endian (x86_64 and
 * ARM64) while the other is big endian (MIPS3, at least on the N64).
 */
typedef uint64_t RecompGPR;

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
} RecompFPR;

/**
 * @brief A type which represents the entire state of the N64 processor.
 */
typedef struct {
    /**
     * Constant zero register. Always reads as 0. Writes are ignored.
     */
    RecompGPR r0;

    /**
     * Assembler temporary register. Not preserved across calls.
     */
    RecompGPR r1;

    /**
     * Function return value or expression result. Also used as argument for
	 * leaf functions.
     */
    RecompGPR r2;

    /**
     * Secondary return value register.
     */
    RecompGPR r3;

    /**
     * First argument register, used to pass arguments to functions.
     */
    RecompGPR r4;

    /**
     * Second argument register.
     */
    RecompGPR r5;

    /**
     * Third argument register.
     */
    RecompGPR r6;

    /**
     * Fourth argument register.
     */
    RecompGPR r7;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r8;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r9;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r10;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r11;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r12;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r13;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r14;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r15;

    /**
     * Saved register. Preserved across function calls.
     */
    RecompGPR r16;

    /**
     * Saved register. Preserved across function calls.
     */
    RecompGPR r17;

    /**
     * Saved register. Preserved across function calls.
     */
    RecompGPR r18;

    /**
     * Saved register. Preserved across function calls.
     */
    RecompGPR r19;

    /**
     * Saved register. Preserved across function calls.
     */
    RecompGPR r20;

    /**
     * Saved register. Preserved across function calls.
     */
    RecompGPR r21;

    /**
     * Saved register. Preserved across function calls.
     */
    RecompGPR r22;

    /**
     * Saved register. Preserved across function calls.
     */
    RecompGPR r23;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r24;

    /**
     * Temporary register. Not preserved across function calls.
     */
    RecompGPR r25;

    /**
     * Reserved for OS kernel. Do not use.
     */
    RecompGPR r26;

    /**
     * Reserved for OS kernel. Do not use.
     */
    RecompGPR r27;

    /**
     * Global pointer (gp) used for addressing static data.
     */
    RecompGPR r28;

    /**
     * Stack pointer (sp) used to manage the call stack.
     */
    RecompGPR r29;

    /**
     * Frame pointer (fp) or another saved register depending on calling
	 * convention.
     */
    RecompGPR r30;

    /**
     * Return address (ra) used for storing the function return address.
     */
    RecompGPR r31;

    /**
     * Floating-point register f0, often used for temporary values.
     */
    RecompFPR f0;

    /**
     * Floating-point register f1, used for temporary computations.
     */
    RecompFPR f1;

    /**
     * Floating-point register f2, used for intermediate floating-point results.
     */
    RecompFPR f2;

    /**
     * Floating-point register f3, used for intermediate floating-point results.
     */
    RecompFPR f3;

    /**
     * Floating-point register f4, used for intermediate floating-point results.
     */
    RecompFPR f4;

    /**
     * Floating-point register f5, general-purpose FP temporary.
     */
    RecompFPR f5;

    /**
     * Floating-point register f6, general-purpose FP temporary.
     */
    RecompFPR f6;

    /**
     * Floating-point register f7, general-purpose FP temporary.
     */
    RecompFPR f7;

    /**
     * Floating-point register f8, general-purpose FP temporary.
     */
    RecompFPR f8;

    /**
     * Floating-point register f9, general-purpose FP temporary.
     */
    RecompFPR f9;

    /**
     * Floating-point register f10, general-purpose FP temporary.
     */
    RecompFPR f10;

    /**
     * Floating-point register f11, general-purpose FP temporary.
     */
    RecompFPR f11;

    /**
     * Floating-point register f12, used for function argument or return value.
     */
    RecompFPR f12;

    /**
     * Floating-point register f13, used for function argument or return value.
     */
    RecompFPR f13;

    /**
     * Floating-point register f14, general-purpose FP value.
     */
    RecompFPR f14;

    /**
     * Floating-point register f15, general-purpose FP value.
     */
    RecompFPR f15;

    /**
     * Floating-point register f16, general-purpose FP value.
     */
    RecompFPR f16;

    /**
     * Floating-point register f17, general-purpose FP value.
     */
    RecompFPR f17;

    /**
     * Floating-point register f18, general-purpose FP value.
     */
    RecompFPR f18;

    /**
     * Floating-point register f19, general-purpose FP value.
     */
    RecompFPR f19;

    /**
     * Floating-point register f20, general-purpose FP value.
     */
    RecompFPR f20;

    /**
     * Floating-point register f21, general-purpose FP value.
     */
    RecompFPR f21;

    /**
     * Floating-point register f22, general-purpose FP value.
     */
    RecompFPR f22;

    /**
     * Floating-point register f23, general-purpose FP value.
     */
    RecompFPR f23;

    /**
     * Floating-point register f24, general-purpose FP value.
     */
    RecompFPR f24;

    /**
     * Floating-point register f25, general-purpose FP value.
     */
    RecompFPR f25;

    /**
     * Floating-point register f26, general-purpose FP value.
     */
    RecompFPR f26;

    /**
     * Floating-point register f27, general-purpose FP value.
     */
    RecompFPR f27;

    /**
     * Floating-point register f28, general-purpose FP value.
     */
    RecompFPR f28;

    /**
     * Floating-point register f29, general-purpose FP value.
     */
    RecompFPR f29;

    /**
     * Floating-point register f30, general-purpose FP value.
     */
    RecompFPR f30;

    /**
     * Floating-point register f31, often used as a return value register or
     * temporary.
     */
    RecompFPR f31;

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
} RecompContext;

/**
 * @brief A type which indicates that a byte array is in the "reversed" order of
 *        the N64, meaning that it has to be accessed through the utility macros
 *        and functions defined in `mod_recomp.h` to read its data in the
 *        correct order. It is also a promise that the array has a size of (at
 *        least) 512 mebibytes / `0x20000000` bytes.
 */
typedef uint8_t RecompRDRAM;

/**
 * @brief A type representing the possible ways to round a floating-point value,
 *        converting it to an integer.
 */
typedef enum RecompRoundingMode {
    /**
     * @brief Performs mathemathically correct rounding to the nearest integer.
     */
    RecompRoundingMode_Nearest = 0U,
    /**
     * @brief Round towards zero. Rounds down for positive values and rounds up
     *        for negative values.
     *
     * This is usually the fastest rounding method, as it can directly cast the
     * given floating-point value into an integer without calling any math
     * functions from the C standard library first.
     */
    RecompRoundingMode_Truncate = 1U,
    /**
     * @brief Round up towards positive infinity.
     */
    RecompRoundingMode_Ceiling = 2U,
    /**
     * @brief Round down towards negative infinity.
     */
    RecompRoundingMode_Floor = 3U,
} RecompRoundingMode;

/**
 * @brief Cast a number into a signed 64-bit integer.
 * @param[in] VAL The value to convert.
 * @return `s64` The converted value.
 */
#define SIGNED(VAL) \
    ((int64_t)(VAL))

/**
 * @brief Perform an addition of two numbers and store the result as a signed
 *        32-bit integer into a general purpose register.
 * @param[in] A The first addend value.
 * @param[in] B The second addend value.
 * @return `RecompGPR` The sum of `A` and `B` (the result of the computation).
 */
#define ADD32(A, B) \
    ((RecompGPR)(int32_t)((A) + (B)))

/**
 * @brief Perform a subtraction of two numbers and store the result as a signed
 *        32-bit integer into a general purpose register.
 * @param[in] A The minuend value (the value that `B` is subtracted from).
 * @param[in] B The subtrahend value (the value to subtract from `a`).
 * @return `RecompGPR` The difference between `A` and `B` (the result of the
 *               computation).
 */
#define SUB32(A, B) \
    ((RecompGPR)(int32_t)((A) - (B)))

/**
 * @brief Load a signed 32-bit integer (a "word") from N64 memory.
 * @param[in] OFFSET `RecompGPR` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] REG `RecompGPR` The number of the target register to read from, acting
 *                      as an index into the array at `OFFSET`.
 * @return `s32` The value stored in the N64 register `REG`.
 */
#define MEM_W(OFFSET, REG) \
    (*(int32_t *)(rdram + ((((REG) + (OFFSET))) & 0x7FFFFFFFULL)))

/**
 * @brief Load a signed 16-bit integer (a "halfword") from N64 memory.
 * @param[in] OFFSET `RecompGPR` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] REG `RecompGPR` The number of the target register to read from, acting
 *                      as an index into the array at `OFFSET`.
 * @return `s16` The value stored in the N64 register `REG`.
 */
#define MEM_H(OFFSET, REG) \
    (*(int16_t *)(rdram + ((((REG) + (OFFSET)) ^ 2) & 0x7FFFFFFFULL)))

/**
 * @brief Load a signed 8-bit integer (a "byte") from N64 memory.
 * @param[in] OFFSET `RecompGPR` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] REG `RecompGPR` The number of the target register to read from, acting
 *                      as an index into the array at `OFFSET`.
 * @return `s8` The value stored in the N64 register `REG`.
 */
#define MEM_B(OFFSET, REG) \
    (*(int8_t *)(rdram + ((((REG) + (OFFSET)) ^ 3) & 0x7FFFFFFFULL)))

/**
 * @brief Load an unsigned 16-bit integer (a "halfword") from N64 memory.
 * @param[in] OFFSET `RecompGPR` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] REG `RecompGPR` The number of the target register to read from, acting
 *                      as an index into the array at `OFFSET`.
 * @return `u16` The value stored in the N64 register `REG`.
 */
#define MEM_HU(OFFSET, REG) \
    (*(uint16_t *)(rdram + ((((REG) + (OFFSET)) ^ 2) & 0x7FFFFFFFULL)))

/**
 * @brief Load an unsigned 8-bit integer (a "byte") from N64 memory.
 * @param[in] OFFSET `RecompGPR` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] REG `RecompGPR` The number of the target register to read from, acting
 *                      as an index into the array at `OFFSET`.
 * @return `u8` The value stored in the N64 register `REG`.
 */
#define MEM_BU(OFFSET, REG) \
    (*(uint8_t *)(rdram + ((((REG) + (OFFSET)) ^ 3) & 0x7FFFFFFFULL)))

static inline void store_doubleword(RecompRDRAM *rdram, RecompGPR val, RecompGPR offset, RecompGPR reg) {
    *(uint32_t *)(rdram + ((reg + offset + 4) & 0x7FFFFFFFULL)) = (uint32_t)(val >> 0);
    *(uint32_t *)(rdram + ((reg + offset + 0) & 0x7FFFFFFFULL)) = (uint32_t)(val >> 32);
}

/**
 * @brief Stores a 64-bit integer (a "doubleword") value into an N64 register.
 * @param[in] VAL The value to store in `REG`. Since this value is always casted
 *                into `RecompGPR`, it can be of any numeric type.
 * @param[out] OFFSET `RecompGPR` A pointer to the array where the recomp stores its
 *                                data for N64 registers in memory.
 * @param[out] REG `RecompGPR` The number of the target register to write to, acting
 *                             as an index into the array at `OFFSET`.
 * @return `void` Nothing.
 * @note This macro can only be used as a statement. Attempting to use it as an
 *       expression will result in a syntax error.
 */
#define SD(VAL, OFFSET, REG) store_doubleword(rdram, (VAL), (OFFSET), (REG))

/**
 * @brief Implementation helper for `LD`. Please use that macro instead.
 * @private
 * @deprecated Use `LD` instead.
 */
static inline uint64_t load_doubleword(RecompRDRAM *rdram, RecompGPR reg, RecompGPR offset) {
    uint64_t ret = 0;
    uint64_t lo = (uint64_t)(uint32_t)MEM_W(reg, offset + 4);
    uint64_t hi = (uint64_t)(uint32_t)MEM_W(reg, offset + 0);
    ret = (lo << 0) | (hi << 32);
    return ret;
}

/**
 * @brief Loads a 64-bit integer (a "doubleword") value from an N64 register.
 * @param[in] OFFSET `RecompGPR` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] REG `RecompGPR` The number of the target register to read from, acting
 *                      as an index into the array at `OFFSET`.
 * @return `u64` The loaded value.
 */
#define LD(OFFSET, REG) \
    load_doubleword(rdram, OFFSET, REG)

/**
 * @todo Test which part of the value is loaded by this function.
 * @brief Partially load a value from an N64 register.
 * @param[in] rdram Currently unused.
 * @param[in] initial_value TODO
 * @param[in] offset `RecompGPR` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `RecompGPR` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `s32` The loaded part of the value.
 */
static inline RecompGPR do_lwl(RecompRDRAM *rdram, RecompGPR initial_value, RecompGPR offset, RecompGPR reg) {
    // Calculate the overall address
    RecompGPR address = (offset + reg);

    // Load the aligned word
    RecompGPR word_address = address & ~0x3;
    uint32_t loaded_value = MEM_W(0, word_address);

    // Mask the existing value and shift the loaded value appropriately
    RecompGPR misalignment = address & 0x3;
    RecompGPR masked_value = initial_value & ~(0xFFFFFFFFu << (misalignment * 8));
    loaded_value <<= (misalignment * 8);

    // Cast to int32_t to sign extend first
    return (RecompGPR)(int32_t)(masked_value | loaded_value);
}

/**
 * @todo Test which part of the value is loaded by this function.
 * @brief Partially load a value from an N64 register.
 * @param[in] rdram Currently unused.
 * @param[in] initial_value TODO
 * @param[in] offset `RecompGPR` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `RecompGPR` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `s32` The loaded part of the value.
 */
static inline RecompGPR do_lwr(RecompRDRAM *rdram, RecompGPR initial_value, RecompGPR offset, RecompGPR reg) {
    // Calculate the overall address
    RecompGPR address = (offset + reg);
    
    // Load the aligned word
    RecompGPR word_address = address & ~0x3;
    uint32_t loaded_value = MEM_W(0, word_address);

    // Mask the existing value and shift the loaded value appropriately
    RecompGPR misalignment = address & 0x3;
    RecompGPR masked_value = initial_value & ~(0xFFFFFFFFu >> (24 - misalignment * 8));
    loaded_value >>= (24 - misalignment * 8);

    // Cast to int32_t to sign extend first
    return (RecompGPR)(int32_t)(masked_value | loaded_value);
}

/**
 * @todo Test which part of the value is stored by this function.
 * @brief Partially store a value into an N64 register.
 * @param[in] rdram Currently unused.
 * @param[out] offset `RecompGPR` A pointer to the array where the recomp stores its
 *                          data for N64 registers in memory.
 * @param[out] reg `RecompGPR` The number of the target register to write to, acting
 *                       as an index into the array at `offset`.
 * @param[in] val The value you want to store.
 * @return `void` Nothing.
 */
static inline void do_swl(RecompRDRAM *rdram, RecompGPR offset, RecompGPR reg, RecompGPR val) {
    // Calculate the overall address
    RecompGPR address = (offset + reg);

    // Get the initial value of the aligned word
    RecompGPR word_address = address & ~0x3;
    uint32_t initial_value = MEM_W(0, word_address);

    // Mask the initial value and shift the input value appropriately
    RecompGPR misalignment = address & 0x3;
    uint32_t masked_initial_value = initial_value & ~(0xFFFFFFFFu >> (misalignment * 8));
    uint32_t shifted_input_value = ((uint32_t)val) >> (misalignment * 8);
    MEM_W(0, word_address) = masked_initial_value | shifted_input_value;
}

/**
 * @todo Test which part of the value is stored by this function.
 * @brief Partially store a value into an N64 register.
 * @param[in] rdram Currently unused.
 * @param[out] offset `RecompGPR` A pointer to the array where the recomp stores its
 *                          data for N64 registers in memory.
 * @param[out] reg `RecompGPR` The number of the target register to write to, acting
 *                       as an index into the array at `offset`.
 * @param[in] val The value you want to store.
 * @return `void` Nothing.
 */
static inline void do_swr(RecompRDRAM *rdram, RecompGPR offset, RecompGPR reg, RecompGPR val) {
    // Calculate the overall address
    RecompGPR address = (offset + reg);

    // Get the initial value of the aligned word
    RecompGPR word_address = address & ~0x3;
    uint32_t initial_value = MEM_W(0, word_address);

    // Mask the initial value and shift the input value appropriately
    RecompGPR misalignment = address & 0x3;
    uint32_t masked_initial_value = initial_value & ~(0xFFFFFFFFu << (24 - misalignment * 8));
    uint32_t shifted_input_value = ((uint32_t)val) << (24 - misalignment * 8);
    MEM_W(0, word_address) = masked_initial_value | shifted_input_value;
}

/**
 * @brief Cast a number into a signed 32-bit integer.
 * @param[in] VAL The value to convert.
 * @return `s32` The converted value.
 */
#define S32(VAL) \
    ((int32_t)(VAL))

/**
 * @brief Cast a number into an unsigned 32-bit integer.
 * @param[in] VAL The value to convert.
 * @return `u32` The converted value.
 */
#define U32(VAL) \
    ((uint32_t)(VAL))

/**
 * @brief Cast a number into a signed 64-bit integer.
 * @param[in] VAL The value to convert.
 * @return `s64` The converted value.
 */
#define S64(VAL) \
    ((int64_t)(VAL))

/**
 * @brief Cast a number into an unsigned 64-bit integer.
 * @param[in] VAL The value to convert.
 * @return `u64` The converted value.
 */
#define U64(VAL) \
    ((uint64_t)(VAL))

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
 * @param[in] VAL `s32` The value to convert.
 * @return `float` The converted value.
 */
#define CVT_S_W(VAL) \
    ((float)((int32_t)(VAL)))

/**
 * @brief Convert a signed 32-bit integer (a "word") into a 64-bit ("double-
 *        precision") floating-point value.
 * @param[in] VAL `s32` The value to convert.
 * @return `double` The converted value.
 */
#define CVT_D_W(VAL) \
    ((double)((int32_t)(VAL)))

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        64-bit ("double-precision") floating-point value.
 * @param[in] VAL `float` The value to convert.
 * @return `double` The converted value.
 */
#define CVT_D_S(VAL) \
    ((double)(VAL))

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        32-bit ("single-precision") floating-point value.
 * @param[in] VAL `double` The value to convert.
 * @return `float` The converted value.
 */
#define CVT_S_D(VAL) \
    ((float)(VAL))

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        32-bit integer (a "word") by truncating, thus cutting off any decimal
 *        places, rounding towards zero and losing some precision (especially if
 *        the value is very small or extremely large).
 * @param[in] VAL `float` The value to convert.
 * @return `s32` The converted value.
 */
#define TRUNC_W_S(VAL) \
    ((int32_t)(VAL))

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        32-bit integer (a "word") by truncating, thus cutting off any decimal
 *        places, rounding towards zero and losing some precision (especially if
 *        the value is very small or extremely large).
 * @param[in] VAL `double` The value to convert.
 * @return `s32` The converted value.
 */
#define TRUNC_W_D(VAL) \
    ((int32_t)(VAL))

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        64-bit integer (a "long") by truncating, thus cutting off any decimal
 *        places, rounding towards zero and losing some precision (especially if
 *        the value is very small or extremely large).
 * @param[in] VAL `float` The value to convert.
 * @return `s64` The converted value.
 */
#define TRUNC_L_S(VAL) \
    ((int64_t)(VAL))

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        64-bit integer (a "long") by truncating, thus cutting off any decimal
 *        places, rounding towards zero and losing some precision (especially if
 *        the value is very small or extremely large).
 * @param[in] VAL `double` The value to convert.
 * @return `s64` The converted value.
 */
#define TRUNC_L_D(VAL) \
    ((int64_t)(VAL))

/**
 * @brief A constant that you may use as a default when calling a function that
 *        requires you to explicitly pass a rounding mode flag when you don't
 *        care about the specifics.
 */
#define DEFAULT_ROUNDING_MODE RecompRoundingMode_Nearest

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        32-bit signed integer (a "word") using a manually specified rounding
 *        mode.
 *
 * You probably want to use the wrapper macro `CVT_W_S` instead of calling this
 * function directly.
 *
 * @param[in] val `float` The value to round.
 * @param[in] rounding_mode `RecompRoundingMode` The rounding mode to use.
 * @return `s32` The result of rounding `val` with the given `rounding_mode`.
 */
static inline int32_t do_cvt_w_s(float val, RecompRoundingMode rounding_mode) {
    switch (rounding_mode) {
        case RecompRoundingMode_Nearest:  return (int32_t)lroundf(val);
        case RecompRoundingMode_Truncate: return (int32_t)val;
        case RecompRoundingMode_Ceiling:  return (int32_t)ceilf(val);
        case RecompRoundingMode_Floor:    return (int32_t)floorf(val);
    }
    fprintf(stderr, "Invalid rounding mode! (RecompRoundingMode expected, got: %d)", (int)rounding_mode);
    assert(0);
    return 0;
}

/**
 * @brief Convert a 32-bit ("single-precision") floating-point value into a
 *        32-bit signed integer (a "word") using the default rounding mode.
 *
 * @param[in] VAL `float` The value to round.
 * @return `s32` The result of rounding `VAL`.
 */
#define CVT_W_S(VAL) \
    do_cvt_w_s(VAL, rounding_mode)

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        32-bit signed integer (a "word") using a manually specified rounding
 *        mode.
 *
 * You probably want to use the wrapper macro `CVT_W_D` instead of calling this
 * function directly.
 *
 * @param[in] val `double` The value to round.
 * @param[in] rounding_mode `RecompRoundingMode` The rounding mode to use.
 * @return `s32` The result of rounding `val` with the given `rounding_mode`.
 */
static inline int32_t do_cvt_w_d(double val, RecompRoundingMode rounding_mode) {
    switch (rounding_mode) {
        case RecompRoundingMode_Nearest:  return (int32_t)lround(val);
        case RecompRoundingMode_Truncate: return (int32_t)val;
        case RecompRoundingMode_Ceiling:  return (int32_t)ceil(val);
        case RecompRoundingMode_Floor:    return (int32_t)floor(val);
    }
    fprintf(stderr, "Invalid rounding mode! (RecompRoundingMode expected, got: %d)", (int)rounding_mode);
    assert(0);
    return 0;
}

/**
 * @brief Convert a 64-bit ("double-precision") floating-point value into a
 *        32-bit signed integer (a "word") using the default rounding mode.
 *
 * @param[in] VAL `double` The value to round.
 * @return `s32` The result of rounding `VAL`.
 */
#define CVT_W_D(VAL) \
    do_cvt_w_d(VAL, rounding_mode)

/**
 * @brief Check if a floating-point value is NaN (not a number), force stopping
 *        the game immediately and logging a message to the terminal if it is.
 * @param[in] VAL `float|double` The value to check.
 * @return `void` Nothing.
 */
#define NAN_CHECK(VAL) \
    assert((VAL) == (VAL))

/**
 * @todo Explain *why* this is here / why someone would actually need this.
 * @brief Check if the target is an even float register or that the MIPS3 float
 *        mode is enabled, force stopping the game immediately and logging a
 *        message to the terminal otherwise.
 * @param[in] CTX `RecompContext*` The current execution context of the recomp.
 * @param[in] IDX `size_t` The index number for the register to check.
 * @return `void` Nothing.
 */
#define CHECK_FR(CTX, IDX) \
    assert(((IDX) & 1) == 0 || (CTX)->mips3_float_mode)

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
 * @param[in] ctx `RecompContext*` The current execution context of the recomp.
 * @return `void` Nothing.
 */
typedef void (recomp_func_t)(RecompRDRAM *rdram, RecompContext* ctx);

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
 * @param[in] ctx `RecompContext*` The current execution context of the recomp.
 * @param[in] value `RecompGPR` The value to write into the `COP0` register.
 * @return `void` Nothing.
 */
extern RECOMP_EXPORT void (*cop0_status_write)(RecompContext* ctx, RecompGPR value);

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
 * @param[in] ctx `RecompContext*` The current execution context of the recomp.
 * @return `RecompGPR` The current value stored in the `COP0` register.
 */
extern RECOMP_EXPORT RecompGPR (*cop0_status_read)(RecompContext* ctx);

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

#define LOOKUP_FUNC(VAL) \
    get_function((int32_t)(VAL))

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
 * @param[in] section_index `RecompGPR` Index into `section_addresses` array.
 * @param[in] OFFSET `RecompGPR` Offset within the section.
 * @return `RecompGPR` Adjusted upper 16 bits of the relocated address.
 */
#define RELOC_HI16(section_index, OFFSET) \
    HI16(section_addresses[section_index] + (OFFSET))

/**
 * @brief Extracts the lower 16 bits of a relocated address for a given section.
 *
 * Used to generate the immediate value for an `ori` instruction in recompiled MIPS code.
 *
 * @param[in] section_index `RecompGPR` Index into `section_addresses` array.
 * @param[in] OFFSET `RecompGPR` Offset within the section.
 * @return `RecompGPR` Lower 16 bits of the relocated address.
 */
#define RELOC_LO16(section_index, OFFSET) \
    LO16(section_addresses[section_index] + (OFFSET))

/**
 * @brief Computes the adjusted upper 16 bits of a reference (original) address
 *        for a section.
 *
 * Useful for validation or symbol resolution against the original binary layout.
 *
 * @param[in] section_index `RecompGPR` Index into `reference_section_addresses` array.
 * @param[in] OFFSET `RecompGPR` Offset within the section.
 * @return `RecompGPR` Adjusted upper 16 bits of the reference address.
 */
#define REF_RELOC_HI16(section_index, OFFSET) \
    HI16(reference_section_addresses[section_index] + (OFFSET))

/**
 * @brief Extracts the lower 16 bits of a reference (original) address for a
 *        section.
 *
 * Useful for validation or symbol resolution against the original binary
 * layout.
 *
 * @param[in] section_index `RecompGPR` Index into `reference_section_addresses` array.
 * @param[in] OFFSET `RecompGPR` Offset within the section.
 * @return `RecompGPR` Lower 16 bits of the reference address.
 */
#define REF_RELOC_LO16(section_index, OFFSET) \
    LO16(reference_section_addresses[section_index] + (OFFSET))

/**
 * @brief I'm guessing this function is called back when an attempt is made to
 *        invoke a system call from N64/mod code, but I have no idea if that's
 *        actually what it does.
 * @param[inout] rdram `uint8_t*` 
 * @param[in] ctx `RecompContext*` The current execution context of the recomp.
 * @param[in] instruction_vram `s32` 
 * @return `void` Nothing.
 */
void recomp_syscall_handler(RecompRDRAM *rdram, RecompContext* ctx, int32_t instruction_vram);

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
