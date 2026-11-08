/**
 * @file syn_dshot.h
 * @brief Zero-Heap DShot (DShot150/300/600) Digital ESC Driver.
 *
 * DShot Frame Format:
 * - 16 bits total:
 *   - Bits [15:5]: 11-bit Throttle value (0 = Disarmed/Stop, 1..47 = ESC commands, 48..2047 =
 * Throttle command).
 *   - Bit [4]:     Telemetry Request flag (1 = Request telemetry byte via bidirectional DShot).
 *   - Bits [3:0]:  4-bit CRC (computed via nibble XOR sum: `(payload ^ (payload >> 4) ^ (payload >>
 * 8)) & 0x0F`).
 */

#ifndef SYN_DSHOT_H
#define SYN_DSHOT_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** DShot Speed Variant. */
typedef enum {
    SYN_DSHOT150 = 150, /**< 150 kbit/s (6.67 us per bit). */
    SYN_DSHOT300 = 300, /**< 300 kbit/s (3.33 us per bit). */
    SYN_DSHOT600 = 600  /**< 600 kbit/s (1.67 us per bit). */
} SYN_DShot_Speed;

/** DShot Encoded Packet Structure. */
typedef struct {
    uint16_t throttle;  /**< 11-bit throttle value (0..2047). */
    bool telemetry;     /**< Telemetry request flag. */
    uint8_t crc;        /**< 4-bit calculated CRC. */
    uint16_t raw_frame; /**< 16-bit packed raw frame ready for DMA transmission. */
} SYN_DShot_Packet;

/**
 * @brief Calculate 4-bit DShot CRC for an 12-bit payload (`throttle[11:1]` + `telemetry[0]`).
 *
 * @param payload_12bit 12-bit value `(throttle << 1) | (telemetry ? 1 : 0)`.
 * @return 4-bit CRC value.
 */
uint8_t syn_dshot_calc_crc(uint16_t payload_12bit);

/**
 * @brief Encode throttle value and telemetry flag into a 16-bit DShot frame.
 *
 * @param throttle 11-bit throttle value (clamped 0..2047).
 * @param telemetry Telemetry request flag.
 * @param packet   Pointer to output packet struct.
 * @return SYN_OK on success, SYN_INVALID_PARAM if packet is NULL.
 */
SYN_Status syn_dshot_encode(uint16_t throttle, bool telemetry, SYN_DShot_Packet *packet);

/**
 * @brief Convert standard 1000..2000 us RC pulse width to DShot 48..2047 throttle value.
 *
 * @param us Pulse width in microseconds (1000..2000 us).
 * @return DShot throttle value (0 if us < 1048, 48..2047).
 */
uint16_t syn_dshot_us_to_throttle(uint16_t us);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DSHOT_H */
