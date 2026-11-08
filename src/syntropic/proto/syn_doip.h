/**
 * @file syn_doip.h
 * @brief SyntropicOS ISO 13400-2 Diagnostic over IP (DoIP) Protocol Stack.
 *
 * Implements lightweight, zero-malloc ISO 13400-2 DoIP header serialization,
 * vehicle identification, routing activation, and UDS (ISO 14229-1) message routing over TCP/UDP.
 */

#ifndef SYNTROPIC_DOIP_H
#define SYNTROPIC_DOIP_H

#include "syntropic/common/syn_defs.h"

#if (!defined(SYN_USE_DOIP) || SYN_USE_DOIP) && (!defined(SYN_USE_UDS) || SYN_USE_UDS)
/** @brief Non-zero when DoIP and UDS dependencies are enabled. */
#define SYN_DOIP_ENABLED 1
#else
/** @brief Non-zero when DoIP and UDS dependencies are enabled. */
#define SYN_DOIP_ENABLED 0
#endif

#if SYN_DOIP_ENABLED

#include "syntropic/proto/syn_uds.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ISO 13400-2 Protocol Constants */
#define SYN_DOIP_PROTOCOL_VERSION 0x02U         /**< ISO 13400-2 protocol version */
#define SYN_DOIP_INVERSE_PROTOCOL_VERSION 0xFDU /**< Bitwise inverse of protocol version */
#define SYN_DOIP_HEADER_SIZE 8U                 /**< Fixed 8-byte DoIP header size */
#define SYN_DOIP_DEFAULT_PORT 13400U            /**< Standard DoIP TCP/UDP port */

/** ISO 13400-2 Payload Types */
#define SYN_DOIP_TYPE_GENERIC_NACK 0x0000U            /**< Generic NACK payload type */
#define SYN_DOIP_TYPE_VEHICLE_ID_REQ 0x0001U          /**< Vehicle Identification Request */
#define SYN_DOIP_TYPE_VEHICLE_ID_REQ_EID 0x0002U      /**< Vehicle ID Request with EID */
#define SYN_DOIP_TYPE_VEHICLE_ID_REQ_VIN 0x0003U      /**< Vehicle ID Request with VIN */
#define SYN_DOIP_TYPE_VEHICLE_ANNOUNCEMENT 0x0004U    /**< Vehicle Announcement Message */
#define SYN_DOIP_TYPE_ROUTING_ACTIVATION_REQ 0x0005U  /**< Routing Activation Request */
#define SYN_DOIP_TYPE_ROUTING_ACTIVATION_RESP 0x0006U /**< Routing Activation Response */
#define SYN_DOIP_TYPE_ALIVE_CHECK_REQ 0x0007U         /**< Alive Check Request */
#define SYN_DOIP_TYPE_ALIVE_CHECK_RESP 0x0008U        /**< Alive Check Response */
#define SYN_DOIP_TYPE_DIAGNOSTIC_MSG 0x8001U          /**< UDS Diagnostic Message */
#define SYN_DOIP_TYPE_DIAGNOSTIC_MSG_ACK 0x8002U      /**< Diagnostic Message Positive Ack */
#define SYN_DOIP_TYPE_DIAGNOSTIC_MSG_NACK 0x8003U     /**< Diagnostic Message Negative Ack */

/** ISO 13400-2 Generic NACK Codes */
#define SYN_DOIP_NACK_INCORRECT_PATTERN 0x00U      /**< Incorrect header pattern */
#define SYN_DOIP_NACK_UNKNOWN_PAYLOAD_TYPE 0x01U   /**< Unknown payload type */
#define SYN_DOIP_NACK_MESSAGE_TOO_LARGE 0x02U      /**< Message payload too large */
#define SYN_DOIP_NACK_OUT_OF_MEMORY 0x03U          /**< Out of memory */
#define SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH 0x04U /**< Invalid payload length */

/** ISO 13400-2 Routing Activation Response Codes */
#define SYN_DOIP_ROUTING_SUCCESS 0x10U               /**< Routing activation successful */
#define SYN_DOIP_ROUTING_DENIED_UNKNOWN_TESTER 0x02U /**< Denied: Unknown tester address */
#define SYN_DOIP_ROUTING_DENIED_DIFFERENT_PORT 0x04U /**< Denied: Invalid socket port */

/** ISO 13400-2 Diagnostic Message ACK / NACK Codes */
#define SYN_DOIP_DIAG_ACK_OK 0x00U          /**< Diagnostic message positive ACK */
#define SYN_DOIP_DIAG_NACK_INVALID_SA 0x02U /**< Invalid source address or unactivated routing */
#define SYN_DOIP_DIAG_NACK_UNKNOWN_TA 0x03U /**< Unknown target address */

/**
 * @brief ISO 13400-2 DoIP Header Structure (8 bytes)
 */
typedef struct {
    uint8_t protocol_version;         /**< Must be 0x02 */
    uint8_t inverse_protocol_version; /**< Must be ~protocol_version (0xFD) */
    uint16_t payload_type;            /**< Big-endian 2-byte payload type */
    uint32_t payload_length;          /**< Big-endian 4-byte payload length */
} SYN_DoIP_Header;

/**
 * @brief DoIP Server State Structure
 */
typedef struct {
    uint16_t logical_address;       /**< Server / ECU logical address (e.g. 0x1001) */
    uint16_t active_tester_address; /**< Logical address of activated diagnostic tester */
    bool routing_activated;         /**< Routing activation flag */
    uint8_t vin[17];                /**< Vehicle Identification Number */
    uint8_t eid[6];                 /**< Entity Identifier (MAC Address) */
    uint8_t gid[6];                 /**< Group Identifier */
} SYN_DoIP_Server;

/**
 * @brief Initialize DoIP Server instance.
 * @param server Pointer to DoIP server instance.
 * @param logical_address ECU logical address (e.g. 0x1001).
 * @return true on success, false if server is NULL.
 */
bool syn_doip_init(SYN_DoIP_Server *server, uint16_t logical_address);

/**
 * @brief Set VIN, EID, and GID parameters for vehicle identification responses.
 * @param server Pointer to DoIP server.
 * @param vin 17-byte VIN buffer.
 * @param eid 6-byte EID (MAC) buffer.
 * @param gid 6-byte GID buffer.
 * @return true on success, false on NULL pointers.
 */
bool syn_doip_set_identifiers(SYN_DoIP_Server *server, const uint8_t vin[17], const uint8_t eid[6],
                              const uint8_t gid[6]);

/**
 * @brief Parse 8-byte DoIP Header from raw buffer.
 * @param buf Input buffer.
 * @param len Input buffer length.
 * @param hdr Output header structure.
 * @return true if header is valid, false otherwise.
 */
bool syn_doip_parse_header(const uint8_t *buf, uint16_t len, SYN_DoIP_Header *hdr);

/**
 * @brief Encode 8-byte DoIP Header into target buffer.
 * @param hdr Input header structure.
 * @param buf Output buffer.
 * @param max_len Output buffer capacity (must be >= 8).
 * @return Length written (8), or 0 on error.
 */
uint16_t syn_doip_encode_header(const SYN_DoIP_Header *hdr, uint8_t *buf, uint16_t max_len);

/**
 * @brief Process incoming DoIP message (UDP or TCP) and route UDS requests to syn_uds.
 * @param server Pointer to DoIP server instance.
 * @param uds Pointer to UDS server instance.
 * @param rx_buf Incoming raw DoIP packet.
 * @param rx_len Incoming packet length.
 * @param tx_buf Output buffer for DoIP response frame.
 * @param max_tx_len Maximum output buffer capacity.
 * @param tx_len Output pointer for written response length.
 * @return true if message was processed successfully, false on invalid request/null.
 */
bool syn_doip_process_msg(SYN_DoIP_Server *server, SYN_UDS_Server *uds, const uint8_t *rx_buf,
                          uint16_t rx_len, uint8_t *tx_buf, uint16_t max_tx_len, uint16_t *tx_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DOIP_ENABLED */

#endif /* SYNTROPIC_DOIP_H */
