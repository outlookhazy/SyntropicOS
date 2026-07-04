/**
 * @file syn_random.h
 * @brief Hardware-agnostic random number generation.
 * @ingroup syn_util
 */

#ifndef SYN_RANDOM_H
#define SYN_RANDOM_H

#include "../common/syn_defs.h"
#include "../common/syn_compiler.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fill a buffer with cryptographically secure random bytes.
 *
 * This function relies on a platform-specific hardware RNG or secure
 * OS source (via syn_port_random_fill).
 *
 * @param buf Output buffer.
 * @param len Number of bytes to generate.
 * @return SYN_OK on success, SYN_ERROR if the RNG is unavailable.
 */
SYN_Status syn_random_fill(void *buf, size_t len);

/**
 * @brief Internal XOR-shift fallback for random data.
 * @note NOT SECURE. Only for testing or desperate fallbacks.
 */
SYN_Status syn_random_fallback_fill(void *buf, size_t len);

/**
 * @brief Generate a random 32-bit integer.
 * @return Random value, or 0 on failure (RNG check is recommended for sensitive use).
 */
uint32_t syn_random_u32(void);

/**
 * @brief Generate a random integer within a range [min, max].
 * @param min Minimum value (inclusive).
 * @param max Maximum value (inclusive).
 * @return Random value.
 */
uint32_t syn_random_range(uint32_t min, uint32_t max);

/* ── Porting Hook ───────────────────────────────────────────────────────── */

/**
 * @brief Porting hook: must be implemented by the platform to provide entropy.
 *
 * @param buf Output buffer.
 * @param len Number of bytes.
 * @return SYN_OK on success.
 */
SYN_WEAK SYN_Status syn_port_random_fill(void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RANDOM_H */
