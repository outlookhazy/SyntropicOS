/**
 * @file syn_datalog.h
 * @brief High-speed binary telemetry logger.
 *
 * Captures arbitrary structs or data frames into a static ringbuffer.
 * Useful for high-speed PID tuning, state recording, or telemetry streams.
 *
 * Uses a zero-allocation, lock-free ringbuffer backend. Data is prefixed
 * with a 16-bit ID and 16-bit length to allow demultiplexing on the host.
 * @ingroup syn_storage
 */

#ifndef SYN_DATALOG_H
#define SYN_DATALOG_H

#include "../common/syn_defs.h"
#include "../util/syn_ringbuf.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Telemetry frame header.
 */
typedef struct {
    uint16_t id;    /**< User-defined stream ID */
    uint16_t len;   /**< Payload length in bytes */
} SYN_DataLogHeader;

/** @brief Data logger control block — ring buffer + drop counter. */
typedef struct {
    SYN_RingBuf rb;              /**< Ring buffer backend                */
    uint32_t dropped_frames;     /**< Counter for frames dropped due to full buffer */
} SYN_DataLog;

/**
 * @brief Initialize a datalogger.
 * @param log   Pointer to logger instance.
 * @param buf   Caller-provided backing array.
 * @param size  Size of backing array in bytes.
 */
void syn_datalog_init(SYN_DataLog *log, uint8_t *buf, size_t size);

/**
 * @brief Write a telemetry frame to the logger.
 * @param log   Pointer to logger instance.
 * @param id    Stream ID.
 * @param data  Payload to write.
 * @param len   Payload size in bytes.
 * @return true if written, false if buffer is full (frame dropped).
 */
bool syn_datalog_write(SYN_DataLog *log, uint16_t id, const void *data, uint16_t len);

/**
 * @brief Read the next telemetry frame from the logger.
 * @param log      Pointer to logger instance.
 * @param out_id   [out] Stream ID of read frame.
 * @param out_data [out] Buffer to store payload.
 * @param max_len  Maximum size of out_data.
 * @return Number of payload bytes read, or 0 if empty or buffer too small.
 */
size_t syn_datalog_read(SYN_DataLog *log, uint16_t *out_id, void *out_data, size_t max_len);

/**
 * @brief Get number of dropped frames.
 * @param log  Logger instance.
 * @return Drop count.
 */
static inline uint32_t syn_datalog_get_dropped(const SYN_DataLog *log) {
    return log->dropped_frames;
}

/**
 * @brief Reset the datalog, discarding all frames.
 * @param log  Logger instance.
 */
static inline void syn_datalog_reset(SYN_DataLog *log) {
    syn_ringbuf_reset(&log->rb);
    log->dropped_frames = 0;
}

/**
 * @brief Get number of bytes available for reading.
 * @param log  Logger instance.
 * @return Bytes available.
 */
static inline size_t syn_datalog_available(const SYN_DataLog *log) {
    return syn_ringbuf_count(&log->rb);
}

#ifdef __cplusplus
}
#endif
#endif // SYN_DATALOG_H
