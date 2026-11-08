/**
 * @file syn_smbus.h
 * @brief SMBus (System Management Bus 1.1 / 2.0 / 3.0) Protocol Engine.
 *
 * Provides non-blocking SMBus packet encoding, decoding, Packet Error Checking (PEC)
 * CRC-8 calculations, transaction protocol builders, and ARP/alert definitions.
 * @ingroup syn_protocol
 */

#ifndef SYN_SMBUS_H
#define SYN_SMBUS_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_SMBUS) || SYN_USE_SMBUS

#ifdef __cplusplus
extern "C" {
#endif

/* ── Standard SMBus Slave Addresses ─────────────────────────────────────── */

#define SYN_SMBUS_ADDR_ALERT_RESPONSE 0x0C        /**< Alert Response Address (ARA) */
#define SYN_SMBUS_ADDR_SMART_BATTERY_CHARGER 0x12 /**< Smart Battery Charger */
#define SYN_SMBUS_ADDR_SMART_BATTERY_SELECT 0x13  /**< Smart Battery Selector */
#define SYN_SMBUS_ADDR_SMART_BATTERY 0x16         /**< Smart Battery Data */
#define SYN_SMBUS_ADDR_DEFAULT 0x61               /**< SMBus Default Device Address (ARP) */

/* ── SMBus Limits ───────────────────────────────────────────────────────── */

#define SYN_SMBUS_BLOCK_MAX_LEN 32 /**< SMBus 2.0 default block payload max length */
#define SYN_SMBUS_BUF_MAX_LEN 260  /**< Buffer payload max length for extended frames */

/* ── SMBus Transaction Protocol Types ────────────────────────────────────── */

/** SMBus Transaction Protocol Types Enum. */
typedef enum {
    SYN_SMBUS_PROTO_QUICK_READ = 0,    /**< Quick Command (Read bit) */
    SYN_SMBUS_PROTO_QUICK_WRITE,       /**< Quick Command (Write bit) */
    SYN_SMBUS_PROTO_SEND_BYTE,         /**< Send Byte */
    SYN_SMBUS_PROTO_RECEIVE_BYTE,      /**< Receive Byte */
    SYN_SMBUS_PROTO_WRITE_BYTE,        /**< Write Byte */
    SYN_SMBUS_PROTO_READ_BYTE,         /**< Read Byte */
    SYN_SMBUS_PROTO_WRITE_WORD,        /**< Write Word */
    SYN_SMBUS_PROTO_READ_WORD,         /**< Read Word */
    SYN_SMBUS_PROTO_WRITE_BLOCK,       /**< Write Block */
    SYN_SMBUS_PROTO_READ_BLOCK,        /**< Read Block */
    SYN_SMBUS_PROTO_PROCESS_CALL,      /**< Process Call */
    SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL /**< Block Write - Block Read Process Call */
} SYN_SMBUS_Protocol;

/* ── SMBus Packet Structure ──────────────────────────────────────────────── */

/** @brief SMBus transaction packet structure. */
typedef struct {
    uint8_t slave_addr;                  /**< 7-bit slave address */
    uint8_t command;                     /**< Command code byte */
    SYN_SMBUS_Protocol proto;            /**< Protocol type */
    bool pec_enabled;                    /**< True if Packet Error Checking is active */
    uint8_t length;                      /**< Payload length in data buffer */
    uint8_t data[SYN_SMBUS_BUF_MAX_LEN]; /**< Payload data */
    uint8_t pec;                         /**< Calculated or received PEC byte */
    bool pec_valid;                      /**< True if received PEC matched calculated PEC */
} SYN_SMBUS_Packet;

/* ── API Function Declarations ───────────────────────────────────────────── */

/**
 * @brief Calculate SMBus Packet Error Checking (PEC) CRC-8 byte.
 * Polynomial: x^8 + x^2 + x + 1 (0x07), Initial value: 0x00.
 *
 * @param init_crc Running CRC value (0x00 for fresh calculation).
 * @param data     Pointer to data buffer.
 * @param len      Data length in bytes.
 * @return Computed PEC byte.
 */
uint8_t syn_smbus_calc_pec(uint8_t init_crc, const uint8_t *data, size_t len);

/**
 * @brief Calculate full SMBus 2.0 transaction PEC CRC-8 across write header, repeated-start, and
 * response payload.
 *
 * @param req_pkt     Pointer to request packet holding slave_addr, command, proto, and write data.
 * @param rx_data     Pointer to slave read-phase response bytes (excluding trailing PEC byte).
 * @param rx_data_len Number of response payload bytes in rx_data.
 * @return Computed transaction PEC byte.
 */
uint8_t syn_smbus_calc_transaction_pec(const SYN_SMBUS_Packet *req_pkt, const uint8_t *rx_data,
                                       size_t rx_data_len);

/**
 * @brief Encode an SMBus packet into a byte stream for transmission.
 *
 * @param pkt      Pointer to packet structure.
 * @param tx_buf   Output buffer.
 * @param buf_size Size of output buffer.
 * @param out_len  Pointer to receive encoded byte count.
 * @return SYN_OK on success, or error status.
 */
SYN_Status syn_smbus_encode_packet(const SYN_SMBUS_Packet *pkt, uint8_t *tx_buf, size_t buf_size,
                                   size_t *out_len);

/**
 * @brief Decode an SMBus byte stream into an SMBus packet structure.
 *
 * @param pkt     Pointer to destination packet structure.
 * @param rx_buf  Input buffer containing received bytes.
 * @param rx_len  Length of received byte stream.
 * @param proto   Protocol format expected.
 * @param has_pec True if PEC byte is appended at end of rx_buf.
 * @return SYN_OK on success, SYN_ERROR if PEC mismatch, or SYN_INVALID_PARAM.
 */
SYN_Status syn_smbus_decode_packet(SYN_SMBUS_Packet *pkt, const uint8_t *rx_buf, size_t rx_len,
                                   SYN_SMBUS_Protocol proto, bool has_pec);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_SMBUS */
#endif /* SYN_SMBUS_H */
