/**
 * @file syn_bits.h
 * @brief Bit manipulation macros.
 *
 * All macros are pure preprocessor — zero overhead, no function calls.
 * @ingroup syn_core
 */

#ifndef SYN_BITS_H
#define SYN_BITS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Produce a bitmask with bit @p n set (0-indexed). */
#define SYN_BIT(n)                ((uint32_t)1U << (n))

/** Set bit @p bit in register/variable @p reg. */
#define SYN_BIT_SET(reg, bit)     ((reg) |= SYN_BIT(bit))

/** Clear bit @p bit in register/variable @p reg. */
#define SYN_BIT_CLEAR(reg, bit)   ((reg) &= ~SYN_BIT(bit))

/** Toggle bit @p bit in register/variable @p reg. */
#define SYN_BIT_TOGGLE(reg, bit)  ((reg) ^= SYN_BIT(bit))

/** Check if bit @p bit is set in @p reg. Evaluates to non-zero if set. */
#define SYN_BIT_CHECK(reg, bit)   ((reg) & SYN_BIT(bit))

/**
 * Produce a bitmask of @p width bits starting at bit position @p offset.
 * Example: SYN_BITMASK(3, 4) → 0b0000'0000'0111'0000 → 0x70
 */
#define SYN_BITMASK(width, offset) \
    (((1U << (width)) - 1U) << (offset))

/**
 * Extract a bit-field of @p width bits starting at @p offset from @p reg.
 */
#define SYN_BITS_GET(reg, width, offset) \
    (((reg) >> (offset)) & ((1U << (width)) - 1U))

/**
 * Set a bit-field: clear the field in @p reg, then OR in @p value.
 */
#define SYN_BITS_SET(reg, width, offset, value)             \
    do {                                                      \
        (reg) &= ~SYN_BITMASK(width, offset);               \
        (reg) |= (((uint32_t)(value) & ((1U << (width)) - 1U)) << (offset)); \
    } while (0)

/** Number of bytes needed to store @p bits. */
#define SYN_BITS_TO_BYTES(bits)   (((bits) + 7U) / 8U)

#ifdef __cplusplus
}
#endif

#endif /* SYN_BITS_H */
