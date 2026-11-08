/**
 * @file syn_blackbox.h
 * @brief Zero-Heap Flight Telemetry Blackbox Binary Recorder.
 *
 * High-speed binary logger for SPI Flash or SD Card logging.
 * Frame Formats:
 * - 'H' (Header): ASCII configuration settings and flight controller parameters.
 * - 'I' (Intra Frame): Full uncompressed 32-bit state vector (Gyro, Accel, Motor outputs,
 * Setpoints).
 * - 'P' (Predictive Delta Frame): Compact variable-length ZigZag delta encoded frame.
 */

#ifndef SYN_BLACKBOX_H
#define SYN_BLACKBOX_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_BLACKBOX_FRAME_INTRA 'I' /**< Intra frame identifier 'I' (full uncompressed frame) */
#define SYN_BLACKBOX_FRAME_DELTA 'P' /**< Delta frame identifier 'P' (predictive delta-encoded) */
#define SYN_BLACKBOX_FRAME_SLOW 'S'  /**< Slow frame identifier 'S' (low-rate status fields) */
#define SYN_BLACKBOX_MAX_FRAME_SIZE 64U /**< Worst-case encoded 'I' or 'P' frame size in bytes */

/** Blackbox Flight State Record. */
typedef struct {
    uint32_t iteration;  /**< Main loop iteration counter */
    uint32_t time_us;    /**< Microsecond timestamp */
    int16_t gyro[3];     /**< Roll, Pitch, Yaw deg/s */
    int16_t accel[3];    /**< X, Y, Z mg             */
    int16_t setpoint[4]; /**< Roll, Pitch, Yaw, Throttle */
    uint16_t motor[4];   /**< Motor 1..4 PWM/DShot us    */
} SYN_Blackbox_Record;

/** Blackbox Recorder Instance. */
typedef struct {
    SYN_Blackbox_Record last_record; /**< Last logged record (used for delta prediction) */
    uint32_t frame_count;            /**< Total frames encoded */
    uint32_t bytes_written;          /**< Total bytes written to stream */
} SYN_Blackbox;

/**
 * @brief Initialize Blackbox recorder.
 *
 * @param bb Pointer to Blackbox instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_blackbox_init(SYN_Blackbox *bb);

/**
 * @brief Encode a full Intra Frame ('I') with explicit output buffer capacity check.
 *
 * @param bb       Pointer to Blackbox instance.
 * @param record   Pointer to flight state record.
 * @param buf_out  Output buffer.
 * @param buf_cap  Capacity of @p buf_out in bytes.
 * @param out_len  Pointer to receive encoded byte length.
 * @return SYN_OK on success, SYN_INVALID_PARAM on NULL argument or insufficient @p buf_cap.
 */
SYN_Status syn_blackbox_encode_intra_ex(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                        uint8_t *buf_out, size_t buf_cap, size_t *out_len);

/**
 * @brief Encode a full Intra Frame ('I') into raw binary log stream.
 *
 * @param bb      Pointer to Blackbox instance.
 * @param record  Pointer to flight state record.
 * @param buf_out Output buffer (must hold at least SYN_BLACKBOX_MAX_FRAME_SIZE bytes).
 * @param out_len Pointer to receive encoded byte length.
 * @return SYN_OK on success.
 */
SYN_Status syn_blackbox_encode_intra(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                     uint8_t *buf_out, size_t *out_len);

/**
 * @brief Encode a compact Predictive Delta Frame ('P') with explicit output buffer capacity check.
 *
 * @param bb       Pointer to Blackbox instance.
 * @param record   Pointer to flight state record.
 * @param buf_out  Output buffer.
 * @param buf_cap  Capacity of @p buf_out in bytes.
 * @param out_len  Pointer to receive encoded byte length.
 * @return SYN_OK on success, SYN_INVALID_PARAM on NULL argument or insufficient @p buf_cap.
 */
SYN_Status syn_blackbox_encode_delta_ex(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                        uint8_t *buf_out, size_t buf_cap, size_t *out_len);

/**
 * @brief Encode a compact Predictive Delta Frame ('P') relative to last frame.
 *
 * @param bb      Pointer to Blackbox instance.
 * @param record  Pointer to flight state record.
 * @param buf_out Output buffer (must hold at least SYN_BLACKBOX_MAX_FRAME_SIZE bytes).
 * @param out_len Pointer to receive encoded byte length.
 * @return SYN_OK on success.
 */
SYN_Status syn_blackbox_encode_delta(SYN_Blackbox *bb, const SYN_Blackbox_Record *record,
                                     uint8_t *buf_out, size_t *out_len);

/**
 * @brief Encode a 32-bit signed integer using ZigZag + LEB128 variable-length format.
 *
 * @param val     Signed integer.
 * @param buf_out Output buffer (must hold at least 5 bytes).
 * @return Number of bytes written to buffer (1..5).
 */
size_t syn_blackbox_encode_varint(int32_t val, uint8_t *buf_out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BLACKBOX_H */
