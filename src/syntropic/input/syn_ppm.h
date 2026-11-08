/**
 * @file syn_ppm.h
 * @brief Zero-Heap PPM (Pulse-Position Modulation) Multi-Channel RC Receiver Decoder.
 *
 * PPM Protocol Specifications:
 * - Single-wire pulse train consisting of 4..12 channel pulse widths (1000..2000 us).
 * - Sync gap: Pulse width > 2700 us demarcates frame boundary.
 * - Pulse separator: Short low pulse (e.g. 300..500 us).
 */

#ifndef SYN_PPM_H
#define SYN_PPM_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_PPM_MAX_CHANNELS 12   /**< Maximum supported PPM channels (12) */
#define SYN_PPM_SYNC_MIN_US 2700U /**< Minimum sync pulse gap in microseconds (2700 us) */

/** PPM Decoder Instance. */
typedef struct {
    uint16_t channels[SYN_PPM_MAX_CHANNELS]; /**< Channel pulse widths in microseconds. */
    uint8_t channel_count;                   /**< Total channels detected in frame. */
    uint8_t current_channel;                 /**< Current decoding channel index. */
    bool in_frame;                           /**< True if decoder is synced to frame boundary. */
    uint32_t frames_received;                /**< Total valid PPM frames received. */
} SYN_PPM_Decoder;

/**
 * @brief Initialize PPM decoder instance.
 *
 * @param ppm Pointer to decoder struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_ppm_init(SYN_PPM_Decoder *ppm);

/**
 * @brief Process high-resolution measured pulse width (us) from Timer Input Capture interrupt.
 *
 * @param ppm      Pointer to decoder struct.
 * @param pulse_us Measured pulse width in microseconds (us).
 * @return SYN_OK when a complete multi-channel PPM frame is finished, SYN_BUSY during channel
 * reception.
 */
SYN_Status syn_ppm_process_pulse(SYN_PPM_Decoder *ppm, uint16_t pulse_us);

/**
 * @brief Get pulse width (us) for specified 0-indexed channel.
 *
 * @param ppm        Pointer to decoder struct.
 * @param channel_idx 0-indexed channel index (0..11).
 * @return Pulse width in microseconds (clamped 1000..2000 us, or 1500 us default if unreceived).
 */
uint16_t syn_ppm_get_channel(const SYN_PPM_Decoder *ppm, uint8_t channel_idx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PPM_H */
