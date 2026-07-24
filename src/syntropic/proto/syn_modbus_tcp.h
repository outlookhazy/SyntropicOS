/**
 * @file syn_modbus_tcp.h
 * @brief Modbus TCP ADU (MBAP) framing layer.
 * @ingroup syn_protocol
 *
 * Implements Modbus TCP MBAP (Modbus Application Protocol) header framing
 * over TCP transport sockets (syn_transport_tcp).
 *
 * MBAP Header Structure (7 bytes):
 *   - Transaction ID (2 bytes, big-endian)
 *   - Protocol ID    (2 bytes, big-endian, 0x0000 = Modbus)
 *   - Length         (2 bytes, big-endian, length of remaining bytes)
 *   - Unit ID        (1 byte, target slave/server address)
 *
 * Followed by standard Modbus PDU (Function Code + Data). Note that Modbus TCP
 * does NOT include RTU CRC-16.
 */

#ifndef SYN_MODBUS_TCP_H
#define SYN_MODBUS_TCP_H

#include "../common/syn_defs.h"
#include "syn_modbus.h"
#include "syn_modbus_master.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_MBAP_HEADER_LEN   7     /**< MBAP header length in bytes */
#define SYN_MB_TCP_MAX_ADU    260   /**< Max Modbus TCP ADU (7 MBAP + 253 PDU) */

/** @brief Modbus TCP MBAP Header structure */
typedef struct {
    uint16_t transaction_id;  /**< Transaction Identifier */
    uint16_t protocol_id;     /**< Protocol Identifier (0x0000 = Modbus) */
    uint16_t length;          /**< Number of following bytes (Unit ID + PDU) */
    uint8_t  unit_id;         /**< Unit Identifier / Slave address */
} SYN_MBAP_Header;

/**
 * @brief Encode an MBAP header into a byte buffer.
 *
 * @param hdr  Pointer to MBAP header structure.
 * @param buf  Output byte buffer (must be at least 7 bytes).
 */
void syn_mbap_encode_header(const SYN_MBAP_Header *hdr, uint8_t *buf);

/**
 * @brief Decode an MBAP header from a byte buffer.
 *
 * @param buf  Input byte buffer (must be at least 7 bytes).
 * @param hdr  Pointer to destination MBAP header structure.
 * @return true if valid Modbus protocol ID (0x0000).
 */
bool syn_mbap_decode_header(const uint8_t *buf, SYN_MBAP_Header *hdr);

/**
 * @brief Process an incoming Modbus TCP ADU frame on a Modbus Slave instance.
 *
 * Takes a full Modbus TCP ADU (MBAP header + PDU), processes the PDU using
 * the Modbus slave engine, and builds the corresponding Modbus TCP response ADU.
 *
 * @param mb          Modbus RTU/PDU slave instance.
 * @param req_adu     Input Modbus TCP request ADU buffer.
 * @param req_len     Length of request ADU in bytes.
 * @param resp_adu    Output Modbus TCP response ADU buffer.
 * @param resp_max    Capacity of response ADU buffer.
 * @param resp_len    Out: Length of generated response ADU in bytes.
 * @return true if response ADU generated, false if no response (or invalid ADU).
 */
bool syn_modbus_tcp_process_slave(SYN_Modbus *mb,
                                  const uint8_t *req_adu, uint16_t req_len,
                                  uint8_t *resp_adu, uint16_t resp_max,
                                  uint16_t *resp_len);

/**
 * @brief Build a Modbus TCP Master request ADU.
 *
 * @param transaction_id Transaction sequence number.
 * @param unit_id        Target Unit ID (slave address).
 * @param pdu            Raw PDU bytes (Function Code + Data).
 * @param pdu_len        PDU length in bytes.
 * @param req_adu        Output buffer for complete Modbus TCP ADU.
 * @param req_max        Output buffer capacity.
 * @param req_len        Out: Total ADU length (7 + pdu_len).
 * @return SYN_OK on success, SYN_NO_MEMORY if buffer too small.
 */
SYN_Status syn_modbus_tcp_build_client_adu(uint16_t transaction_id, uint8_t unit_id,
                                           const uint8_t *pdu, uint16_t pdu_len,
                                           uint8_t *req_adu, uint16_t req_max,
                                           uint16_t *req_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MODBUS_TCP_H */
