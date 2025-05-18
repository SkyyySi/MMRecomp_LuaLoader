#ifndef __MOD_RECOMP_H__
#define __MOD_RECOMP_H__ 1

/*
It is required to use this header in any shared libraries that are compiled into
native host instructions ahead of time (instead of into MIPS like regular mods).
If you don't use it, you'll only be able to call functions without any arguments
and without any return values, as those will be completely incorrect. It acts as
a bridge between N64 code and native code.

This header can be thought of as "pseudo-assembly", see
https://n64brew.dev/wiki/MIPS_III_instructions for more details on how the N64's
instruction set works.
*/

/**
 * @note In the code below, the suffixes "B", "H", "W" and "D" generally refer
 *       to integers with a size of 8, 16, 32 and 64 bits and are abbreviations
 *       for "byte", "halfword", "word" and "doubleword", respectively.
 */

/**
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
 * A function marked with this macro is intended to be accessed by mod code by
 * importing it with `RECOMP_IMPORT` over there. Don't forget that you also need
 * to add its name to the `funcs` list of your shared library in `mod.toml`.
 */
#define RECOMP_EXPORT __declspec(dllexport)
#else
/**
 * @brief Mark a function as a member of your public API, allowing it to be
 *        called by mod code.
 *
 * A function marked with this macro is intended to be accessed by mod code by
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
 * This type is used whereever an N64 instruction would try to use the "General
 * Purpose Register" (GPR) on real hardware. A value with a type of `gpr` can
 * be thought of as an integer value with an unknown size and no specified type.
 * It may be a `u64`, but it may also be an `s32`, a `u8` or a pointer (which
 * could also be either 32 or 64 bits in size, since the N64 had both a 32-bit
 * and a 64-bit register mode).
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
    (*(int32_t*)(rdram + ((((reg) + (offset))) - 0xFFFFFFFF80000000)))

/**
 * @brief Load a signed 16-bit integer (a "halfword") from N64 memory.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `s16` The value stored in the N64 register `reg`.
 */
#define MEM_H(offset, reg) \
    (*(int16_t*)(rdram + ((((reg) + (offset)) ^ 2) - 0xFFFFFFFF80000000)))

/**
 * @brief Load a signed 8-bit integer (a "byte") from N64 memory.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `s8` The value stored in the N64 register `reg`.
 */
#define MEM_B(offset, reg) \
    (*(int8_t*)(rdram + ((((reg) + (offset)) ^ 3) - 0xFFFFFFFF80000000)))

/**
 * @brief Load an unsigned 16-bit integer (a "halfword") from N64 memory.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `u16` The value stored in the N64 register `reg`.
 */
#define MEM_HU(offset, reg) \
    (*(uint16_t*)(rdram + ((((reg) + (offset)) ^ 2) - 0xFFFFFFFF80000000)))

/**
 * @brief Load an unsigned 8-bit integer (a "byte") from N64 memory.
 * @param[in] offset `gpr` A pointer to the array where the recomp stores its
 *                         data for N64 registers in memory.
 * @param[in] reg `gpr` The number of the target register to read from, acting
 *                      as an index into the array at `offset`.
 * @return `u8` The value stored in the N64 register `reg`.
 */
#define MEM_BU(offset, reg) \
    (*(uint8_t*)(rdram + ((((reg) + (offset)) ^ 3) - 0xFFFFFFFF80000000)))

/**
 * @brief Stores a 64-bit integer (a "doubleword") value into an N64 register.
 * @param[in] val The value to store in `reg`. Since this value is always casted
 *                into `gpr`, it can be of any numeric type.
 * @param[out] offset `gpr` A pointer to the array where the recomp stores its
 *                          data for N64 registers in memory.
 * @param[out] reg `gpr` The number of the target register to write to, acting
 *                       as an index into the array at `offset`.
 * @return Nothing.
 * @note This macro can only be used as a statement. Attempting to use it as an
 *       expression will result in a syntax error.
 */
#define SD(val, offset, reg) { \
    *(uint32_t*)(rdram + ((((reg) + (offset) + 4)) - 0xFFFFFFFF80000000)) = (uint32_t)((gpr)(val) >> 0); \
    *(uint32_t*)(rdram + ((((reg) + (offset) + 0)) - 0xFFFFFFFF80000000)) = (uint32_t)((gpr)(val) >> 32); \
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
 * @return Nothing.
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
 * @return Nothing.
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

#define S32(val) \
    ((int32_t)(val))
    
#define U32(val) \
    ((uint32_t)(val))

#define S64(val) \
    ((int64_t)(val))

#define U64(val) \
    ((uint64_t)(val))

#define MUL_S(val1, val2) \
    ((val1) * (val2))

#define MUL_D(val1, val2) \
    ((val1) * (val2))

#define DIV_S(val1, val2) \
    ((val1) / (val2))

#define DIV_D(val1, val2) \
    ((val1) / (val2))

#define CVT_S_W(val) \
    ((float)((int32_t)(val)))

#define CVT_D_W(val) \
    ((double)((int32_t)(val)))

#define CVT_D_S(val) \
    ((double)(val))

#define CVT_S_D(val) \
    ((float)(val))

#define TRUNC_W_S(val) \
    ((int32_t)(val))

#define TRUNC_W_D(val) \
    ((int32_t)(val))

#define TRUNC_L_S(val) \
    ((int64_t)(val))

#define TRUNC_L_D(val) \
    ((int64_t)(val))

#define DEFAULT_ROUNDING_MODE 0

static inline int32_t do_cvt_w_s(float val, unsigned int rounding_mode) {
    switch (rounding_mode) {
        case 0: // round to nearest value
            return (int32_t)lroundf(val);
        case 1: // round to zero (truncate)
            return (int32_t)val;
        case 2: // round to positive infinity (ceil)
            return (int32_t)ceilf(val);
        case 3: // round to negative infinity (floor)
            return (int32_t)floorf(val);
    }
    assert(0);
    return 0;
}

#define CVT_W_S(val) \
    do_cvt_w_s(val, rounding_mode)

static inline int32_t do_cvt_w_d(double val, unsigned int rounding_mode) {
    switch (rounding_mode) {
        case 0: // round to nearest value
            return (int32_t)lround(val);
        case 1: // round to zero (truncate)
            return (int32_t)val;
        case 2: // round to positive infinity (ceil)
            return (int32_t)ceil(val);
        case 3: // round to negative infinity (floor)
            return (int32_t)floor(val);
    }
    assert(0);
    return 0;
}

#define CVT_W_D(val) \
    do_cvt_w_d(val, rounding_mode)

#define NAN_CHECK(val) \
    assert(val == val)

//#define NAN_CHECK(val)

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

typedef struct {
    gpr r0,  r1,  r2,  r3,  r4,  r5,  r6,  r7,
        r8,  r9,  r10, r11, r12, r13, r14, r15,
        r16, r17, r18, r19, r20, r21, r22, r23,
        r24, r25, r26, r27, r28, r29, r30, r31;
    fpr f0,  f1,  f2,  f3,  f4,  f5,  f6,  f7,
        f8,  f9,  f10, f11, f12, f13, f14, f15,
        f16, f17, f18, f19, f20, f21, f22, f23,
        f24, f25, f26, f27, f28, f29, f30, f31;
    uint64_t hi, lo;
    uint32_t* f_odd;
    uint32_t status_reg;
    uint8_t mips3_float_mode;
} recomp_context;

// Checks if the target is an even float register or that mips3 float mode is enabled
#define CHECK_FR(ctx, idx) \
    assert(((idx) & 1) == 0 || (ctx)->mips3_float_mode)

#ifdef __cplusplus
extern "C" {
#endif

typedef void (recomp_func_t)(uint8_t* rdram, recomp_context* ctx);

extern RECOMP_EXPORT recomp_func_t* (*get_function)(int32_t vram);
extern RECOMP_EXPORT void (*cop0_status_write)(recomp_context* ctx, gpr value);
extern RECOMP_EXPORT gpr (*cop0_status_read)(recomp_context* ctx);
extern RECOMP_EXPORT void (*switch_error)(const char* func, uint32_t vram, uint32_t jtbl);
extern RECOMP_EXPORT void (*do_break)(uint32_t vram);

#define LOOKUP_FUNC(val) \
    get_function((int32_t)(val))

extern RECOMP_EXPORT int32_t* reference_section_addresses;
extern RECOMP_EXPORT int32_t section_addresses[];

#define LO16(x) \
    ((x) & 0xFFFF)

#define HI16(x) \
    (((x) >> 16) + (((x) >> 15) & 1))

#define RELOC_HI16(section_index, offset) \
    HI16(section_addresses[section_index] + (offset))

#define RELOC_LO16(section_index, offset) \
    LO16(section_addresses[section_index] + (offset))

#define REF_RELOC_HI16(section_index, offset) \
    HI16(reference_section_addresses[section_index] + (offset))

#define REF_RELOC_LO16(section_index, offset) \
    LO16(reference_section_addresses[section_index] + (offset))

void recomp_syscall_handler(uint8_t* rdram, recomp_context* ctx, int32_t instruction_vram);

void pause_self(uint8_t *rdram);

#ifdef __cplusplus
}
#endif

#endif
