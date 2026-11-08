/**
 * @file syn_dshot_telemetry.h
 * @brief Zero-Heap Bidirectional DShot (BDShot) Telemetry Decoder.
 *
 * BDShot Specifications:
 * - ESC returns a 20-bit GCR (Group Code Recording 5b/4b) encoded packet on the DShot line.
 * - 20 GCR bits -> 16 decoded data bits:
 *   - Bits [15:4]: 12-bit eRPM period value (mantissa & exponent format).
 *   - Bits [3:0]:  4-bit CRC (computed via `(payload ^ (payload >> 4) ^ (payload >> 8)) & 0x0F`).
 * - eRPM Calculation:
 *   - `period_us = (mantissa << exponent)`
 *   - `eRPM = 60,000,000 / period_us`
 *   - `RPM = eRPM / (pole_pairs / 2)`
 */

#ifndef SYN_DSHOT_TELEMETRY_H
#define SYN_DSHOT_TELEMETRY_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Decoded BDShot Telemetry Packet. */
typedef struct {
    uint32_t period_us; /**< Motor commutation period in microseconds. */
    uint32_t erpm;      /**< Electrical RPM (eRPM). */
    uint32_t rpm;       /**< Mechanical RPM (for specified pole pairs). */
    bool valid;         /**< True if GCR decoding and CRC check succeeded. */
} SYN_DShot_Telemetry;

/**
 * @brief Decode 20-bit raw GCR bitstream into 16-bit BDShot telemetry payload.
 *
 * @param gcr_20bit 20-bit raw GCR frame.
 * @param payload_out Pointer to receive 16-bit decoded payload (`[15:4] period`, `[3:0] crc`).
 * @return SYN_OK on valid GCR decode, SYN_ERROR if invalid GCR symbol found.
 */
SYN_Status syn_dshot_decode_gcr_20bit(uint32_t gcr_20bit, uint16_t *payload_out);

/**
 * @brief Decode complete 20-bit BDShot GCR packet into telemetry measurements.
 *
 * @param gcr_20bit 20-bit raw GCR frame.
 * @param pole_pairs Motor pole pairs (e.g., 14 poles = 7 pole pairs). Default 7 if 0.
 * @param telemetry Pointer to output telemetry structure.
 * @return SYN_OK on valid decode and CRC, SYN_ERROR on bad GCR/CRC.
 */
SYN_Status syn_dshot_parse_telemetry(uint32_t gcr_20bit, uint8_t pole_pairs,
                                     SYN_DShot_Telemetry *telemetry);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DSHOT_TELEMETRY_H */
