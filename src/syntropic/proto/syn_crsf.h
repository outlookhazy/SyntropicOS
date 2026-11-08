/**
 * @file syn_crsf.h
 * @brief Zero-Heap CRSF (TBS Crossfire / ExpressLRS) Protocol Parser.
 *
 * CRSF Specifications:
 * - Baud rate: 420,000 / 921,600 / 1,875,000 bps, 8N1.
 * - Frame Format: [Device Addr] [Payload Length] [Frame Type] [Payload...] [CRC8]
 * - Address: 0xC8 (Flight Controller), 0xEA (Radio Transmitter).
 * - Types:
 *   - 0x16: RC Channels Frame (16 11-bit channels packed in 22 bytes, raw 170..1908 -> 988..2012
 * us).
 *   - 0x14: Link Statistics (RSSI 1 & 2, Link Quality, SNR).
 *   - 0x08: Battery Telemetry (Voltage, Current, Capacity, Remaining %).
 *   - 0x02: GPS Telemetry (Lat, Lon, Groundspeed, Heading, Altitude, Satellites).
 * - CRC: CRC8 DVB-S2 (poly 0xD5) over Frame Type + Payload.
 */

#ifndef SYN_CRSF_H
#define SYN_CRSF_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_CRSF_MAX_PACKET_LEN 64U /**< Maximum CRSF frame byte length */
#define SYN_CRSF_NUM_CHANNELS 16U   /**< Number of RC channels in CRSF frame (16) */
#define SYN_CRSF_ADDR_FC 0xC8U      /**< Flight Controller sync address (0xC8) */

/** CRSF Frame Types. */
typedef enum {
    SYN_CRSF_TYPE_GPS = 0x02U,
    SYN_CRSF_TYPE_BATTERY = 0x08U,
    SYN_CRSF_TYPE_LINK_STATISTICS = 0x14U,
    SYN_CRSF_TYPE_RC_CHANNELS = 0x16U
} SYN_CRSF_FrameType;

/** Parsed CRSF RC Channels Frame. */
typedef struct {
    uint16_t channels[SYN_CRSF_NUM_CHANNELS]; /**< 16 11-bit channels (170..1908 raw). */
} SYN_CRSF_ChannelsFrame;

/** Parsed CRSF Link Statistics Frame. */
typedef struct {
    uint8_t uplink_rssi1;     /**< Uplink RSSI Antenna 1 (-dBm). */
    uint8_t uplink_rssi2;     /**< Uplink RSSI Antenna 2 (-dBm). */
    uint8_t uplink_quality;   /**< Link Quality (0..100%). */
    int8_t uplink_snr;        /**< Signal-to-Noise Ratio (dB). */
    uint8_t active_antenna;   /**< Active antenna index (0 or 1). */
    uint8_t rf_mode;          /**< RF mode (e.g. 50Hz, 150Hz, 500Hz). */
    uint8_t tx_power;         /**< Transmitter output power. */
    uint8_t downlink_rssi;    /**< Downlink RSSI (-dBm). */
    uint8_t downlink_quality; /**< Downlink Quality (0..100%). */
    int8_t downlink_snr;      /**< Downlink SNR (dB). */
} SYN_CRSF_LinkStats;

/** CRSF Parser Instance. */
typedef struct {
    uint8_t buf[SYN_CRSF_MAX_PACKET_LEN]; /**< Assembly buffer */
    uint8_t idx;                          /**< Current write index */
    uint8_t payload_len;                  /**< Expected payload length */
    uint32_t packets_received;            /**< Total valid packets received */
    uint32_t crc_errors;                  /**< Count of CRC errors */
    SYN_CRSF_ChannelsFrame last_channels; /**< Last decoded RC channels */
    SYN_CRSF_LinkStats last_link_stats;   /**< Last decoded link statistics */
} SYN_CRSF_Parser;

/**
 * @brief Calculate CRC8 DVB-S2 (poly 0xD5) for CRSF buffer.
 *
 * @param buf Pointer to data.
 * @param len Length in bytes.
 * @return 8-bit CRC value.
 */
uint8_t syn_crsf_calc_crc(const uint8_t *buf, size_t len);

/**
 * @brief Initialize CRSF parser.
 *
 * @param parser Pointer to parser struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_crsf_init(SYN_CRSF_Parser *parser);

/**
 * @brief Process single byte from serial RX stream.
 *
 * @param parser Pointer to parser struct.
 * @param byte   Incoming byte.
 * @param type_out Optional pointer to receive frame type when a complete valid packet is parsed.
 * @return SYN_OK on complete packet, SYN_BUSY if incomplete, SYN_ERROR on CRC failure.
 */
SYN_Status syn_crsf_parse_byte(SYN_CRSF_Parser *parser, uint8_t byte, SYN_CRSF_FrameType *type_out);

/**
 * @brief Convert raw 11-bit CRSF channel value to pulse width in microseconds (us).
 *
 * Standard scaling:
 * - 170 raw  -> 988 us
 * - 992 raw  -> 1500 us
 * - 1908 raw -> 2012 us
 *
 * @param raw_val 11-bit raw value (170..1908).
 * @return Pulse width in microseconds (clamped 988..2012 us).
 */
uint16_t syn_crsf_raw_to_us(uint16_t raw_val);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CRSF_H */
