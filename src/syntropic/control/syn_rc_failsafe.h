/**
 * @file syn_rc_failsafe.h
 * @brief Zero-Heap RC Safety Failsafe Manager & Watchdog.
 *
 * Monitors receiver frame updates and enforces pre-configured failsafe fallback positions
 * or emergency disarming if signal loss exceeds timeout threshold.
 */

#ifndef SYN_RC_FAILSAFE_H
#define SYN_RC_FAILSAFE_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum supported failsafe RC channels. */
#define SYN_RC_FAILSAFE_MAX_CHANNELS 16U

/** Failsafe Channel Mode. */
typedef enum {
    SYN_FAILSAFE_HOLD = 0, /**< Hold last valid channel position. */
    SYN_FAILSAFE_FALLBACK, /**< Go to preset fallback position. */
    SYN_FAILSAFE_DISARM    /**< Force channel to 1000 us (minimum/disarm). */
} SYN_Failsafe_ChannelMode;

/** Failsafe Manager Configuration. */
typedef struct {
    uint32_t timeout_ms; /**< Signal loss timeout threshold in ms (e.g. 500 ms). */
    SYN_Failsafe_ChannelMode
        channel_modes[SYN_RC_FAILSAFE_MAX_CHANNELS];    /**< Per-channel fallback mode. */
    uint16_t fallback_us[SYN_RC_FAILSAFE_MAX_CHANNELS]; /**< Fallback pulse widths in us. */
} SYN_Failsafe_Config;

/** Failsafe Manager State Instance. */
typedef struct {
    SYN_Failsafe_Config config;                      /**< Active configuration settings */
    uint32_t last_frame_ms;                          /**< Timestamp of last valid frame in ms */
    bool in_failsafe;                                /**< True if currently in failsafe mode */
    bool ever_received;                              /**< True once first valid RC frame arrives */
    uint32_t failsafe_events;                        /**< Total failsafe trigger events count */
    uint16_t channels[SYN_RC_FAILSAFE_MAX_CHANNELS]; /**< Output channel pulse widths in us */
} SYN_Failsafe_Manager;

/**
 * @brief Initialize failsafe manager instance.
 *
 * @param mgr    Pointer to failsafe manager struct.
 * @param config Pointer to failsafe configuration struct.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_failsafe_init(SYN_Failsafe_Manager *mgr, const SYN_Failsafe_Config *config);

/**
 * @brief Register valid incoming frame from RC receiver (resets watchdog timer).
 *
 * @param mgr            Pointer to failsafe manager struct.
 * @param in_channels    Pointer to array of channel pulse widths in us.
 * @param num_channels   Total channels provided (max 16).
 * @param timestamp_ms   Current system timestamp in ms.
 * @return SYN_OK on success.
 */
SYN_Status syn_failsafe_feed_frame(SYN_Failsafe_Manager *mgr, const uint16_t *in_channels,
                                   uint8_t num_channels, uint32_t timestamp_ms);

/**
 * @brief Step failsafe manager state machine and retrieve active channels (or fallback channels if
 * timed out).
 *
 * @param mgr          Pointer to failsafe manager struct.
 * @param now_ms       Current system timestamp in ms.
 * @param out_channels Pointer to output channel array (must hold at least 16 uint16_t values).
 * @return true if system is currently in Failsafe state, false if active link.
 */
bool syn_failsafe_step(SYN_Failsafe_Manager *mgr, uint32_t now_ms, uint16_t *out_channels);

#ifdef __cplusplus
}
#endif

#endif /* SYN_RC_FAILSAFE_H */
