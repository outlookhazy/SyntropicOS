/**
 * @file syn_j1939.h
 * @brief SAE J1939 Heavy-Duty Vehicle CAN Protocol Stack.
 * @ingroup syn_protocol
 *
 * Implements J1939 29-bit CAN ID parsing/packing, PGN definitions, Transport
 * Protocol (TP.CM BAM & RTS/CTS), Address Claiming (J1939-81), and Active
 * Diagnostic Trouble Codes (DM1 J1939-73).
 */

#ifndef SYN_J1939_H
#define SYN_J1939_H

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup j1939_addresses J1939 Reserved Source/Destination Addresses
 *  @{ */
#define SYN_J1939_ADDR_GLOBAL        0xFFU /**< Global / Broadcast Destination Address */
#define SYN_J1939_ADDR_NULL          0xFEU /**< Null / Unassigned Source Address */
/** @} */

/** @defgroup j1939_pgns Standard J1939 Parameter Group Numbers
 *  @{ */
#define SYN_J1939_PGN_REQUEST        59904U /**< Request PGN (0x00EA00) */
#define SYN_J1939_PGN_TP_DT          60160U /**< Transport Protocol Data Transfer (0x00EB00) */
#define SYN_J1939_PGN_TP_CM          60416U /**< Transport Protocol Connection Management (0x00EC00) */
#define SYN_J1939_PGN_ADDR_CLAIM     60928U /**< Address Claim / Cannot Claim (0x00EE00) */
#define SYN_J1939_PGN_PROPRIETARY_A  61184U /**< Destination-Specific Proprietary A (0x00EF00) */
#define SYN_J1939_PGN_EEC1           61444U /**< Electronic Engine Controller 1 (0x00F004) */
#define SYN_J1939_PGN_DM1            65226U /**< Active Diagnostic Trouble Codes (0x00FECA) */
#define SYN_J1939_PGN_BMS            65251U /**< High Voltage Battery Pack Status (0x00FEE3) */
#define SYN_J1939_PGN_ET1            65262U /**< Engine Temperature 1 (0x00FEEE) */
#define SYN_J1939_PGN_PROPRIETARY_B  65280U /**< Broadcast Proprietary B Start (0x00FF00) */
/** @} */

/** @defgroup j1939_tp_control J1939 TP.CM Control Bytes
 *  @{ */
#define SYN_J1939_TP_CTRL_RTS        16U  /**< Request to Send (P2P) */
#define SYN_J1939_TP_CTRL_CTS        17U  /**< Clear to Send (P2P) */
#define SYN_J1939_TP_CTRL_ACK        19U  /**< End of Message Acknowledgment (P2P) */
#define SYN_J1939_TP_CTRL_BAM        32U  /**< Broadcast Announce Message (BAM) */
#define SYN_J1939_TP_CTRL_ABORT      255U /**< Connection Abort */
/** @} */

/**
 * @brief Parsed J1939 29-bit CAN ID Header.
 */
typedef struct {
    uint8_t  priority; /**< Priority bits (0-7, default 6 or 3) */
    uint8_t  dp;       /**< Data Page bit (0 or 1) */
    uint8_t  pf;       /**< PDU Format (0-255) */
    uint8_t  ps;       /**< PDU Specific (DA for PDU1 < 240, GE for PDU2 >= 240) */
    uint8_t  sa;       /**< Source Address (0-255) */
    uint8_t  da;       /**< Destination Address (DA if PDU1, 0xFF if PDU2) */
    uint32_t pgn;      /**< Computed 18-bit Parameter Group Number */
    bool     is_pdu1;  /**< true if PF < 240 (destination-specific) */
} SYN_J1939_Header;

/**
 * @brief 64-bit J1939 Device NAME Structure (J1939-81).
 */
typedef struct {
    uint32_t identity_number;   /**< 21-bit Unique Serial / Identity Number */
    uint16_t manufacturer_code; /**< 11-bit SAE Manufacturer Code */
    uint8_t  ecu_instance;      /**< 3-bit ECU Instance */
    uint8_t  function_instance; /**< 5-bit Function Instance */
    uint8_t  function;          /**< 8-bit Defined Function */
    uint8_t  vehicle_system;    /**< 7-bit Vehicle System */
    uint8_t  vehicle_system_inst;/**< 4-bit Vehicle System Instance */
    uint8_t  industry_group;    /**< 3-bit Industry Group (e.g. 0=Global, 1=On-Highway, 2=Agricultural) */
    bool     arbitrary_addr_cap;/**< 1-bit Arbitrary Address Capable flag */
} SYN_J1939_Name;

/**
 * @brief J1939 Diagnostic Trouble Code (DTC) Structure (J1939-73).
 */
typedef struct {
    uint32_t spn;              /**< 19-bit Suspect Parameter Number */
    uint8_t  fmi;              /**< 5-bit Failure Mode Identifier */
    uint8_t  occurrence_count; /**< 7-bit Occurrence Count */
    uint8_t  conversion_method;/**< 1-bit SPN Conversion Method */
} SYN_J1939_DTC;

/** @brief J1939 Address Claiming States */
typedef enum {
    SYN_J1939_STATE_UNCLAIMED = 0,
    SYN_J1939_STATE_CLAIMING  = 1,
    SYN_J1939_STATE_CLAIMED   = 2,
    SYN_J1939_STATE_CANNOT_CLAIM = 3
} SYN_J1939_AddressState;

/**
 * @brief J1939 Transport Protocol Rx Re-assembly Buffer.
 */
typedef struct {
    bool     active;           /**< Active reception session flag */
    bool     is_bam;           /**< Broadcast Announce Message session flag */
    uint8_t  sa;               /**< Sender Source Address */
    uint8_t  da;               /**< Destination Address */
    uint32_t pgn;              /**< Reassembled Parameter Group Number */
    uint16_t total_bytes;      /**< Total payload length in bytes */
    uint8_t  total_packets;    /**< Total packet count */
    uint8_t  next_seq;         /**< Next expected sequence number (1..255) */
    uint8_t  received_packets; /**< Currently received packet count */
    uint8_t  data[1785];       /**< Maximum 1785-byte payload buffer */
} SYN_J1939_TpRxSession;

/**
 * @brief J1939 Node Control Instance.
 */
typedef struct {
    uint8_t               sa;        /**< Assigned Source Address */
    SYN_J1939_Name        name;      /**< 64-bit Device NAME */
    SYN_J1939_AddressState state;     /**< Current address claim state */
    SYN_J1939_TpRxSession tp_rx;     /**< Multi-packet Transport Protocol RX session */
} SYN_J1939_Node;

/**
 * @brief Pack J1939 bitfields into a 29-bit CAN Identifier.
 * @param priority Priority (0-7, typically 6).
 * @param pgn Parameter Group Number (18-bit).
 * @param sa Source Address (0-255).
 * @param da Destination Address (used if PGN is PDU1 format).
 * @return 29-bit CAN ID.
 */
uint32_t syn_j1939_id_pack(uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da);

/**
 * @brief Unpack a 29-bit CAN Identifier into a J1939 Header structure.
 * @param can_id 29-bit Extended CAN Identifier.
 * @param header Output structure receiving parsed fields.
 * @return SYN_OK on success, or SYN_INVALID_PARAM if header is NULL.
 */
SYN_Status syn_j1939_id_unpack(uint32_t can_id, SYN_J1939_Header *header);

/**
 * @brief Compute 24-bit Parameter Group Number from Data Page, PF, and PS.
 * @param dp Data Page (0 or 1).
 * @param pf PDU Format (0-255).
 * @param ps PDU Specific (0-255).
 * @return 24-bit PGN value.
 */
uint32_t syn_j1939_calc_pgn(uint8_t dp, uint8_t pf, uint8_t ps);

/**
 * @brief Encode a 64-bit J1939 NAME into an 8-byte buffer.
 * @param name Source NAME structure.
 * @param buf Output 8-byte buffer.
 */
void syn_j1939_name_encode(const SYN_J1939_Name *name, uint8_t buf[8]);

/**
 * @brief Decode an 8-byte buffer into a 64-bit J1939 NAME structure.
 * @param buf Source 8-byte buffer.
 * @param name Output NAME structure.
 */
void syn_j1939_name_decode(const uint8_t buf[8], SYN_J1939_Name *name);

/**
 * @brief Initialize a J1939 Node instance.
 * @param node Pointer to node instance.
 * @param sa Preferred Source Address (0-253).
 * @param name Device 64-bit NAME configuration.
 * @return SYN_OK on success.
 */
SYN_Status syn_j1939_node_init(SYN_J1939_Node *node, uint8_t sa, const SYN_J1939_Name *name);

/**
 * @brief Build an Address Claim CAN frame for the node.
 * @param node Pointer to J1939 node instance.
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_j1939_build_address_claim(const SYN_J1939_Node *node, SYN_CAN_Frame *frame);

/**
 * @brief Build a Request PGN CAN frame (PGN 59904).
 * @param sa Source address sending request.
 * @param da Destination address requested (0xFF for broadcast).
 * @param requested_pgn PGN being requested.
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_j1939_build_request(uint8_t sa, uint8_t da, uint32_t requested_pgn, SYN_CAN_Frame *frame);

/**
 * @brief Build a Transport Protocol Broadcast Announce Message (TP.CM_BAM).
 * @param sa Source address.
 * @param pgn PGN of payload.
 * @param total_bytes Total payload length in bytes (9 to 1785).
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_j1939_build_tp_bam(uint8_t sa, uint32_t pgn, uint16_t total_bytes, SYN_CAN_Frame *frame);

/**
 * @brief Build a Transport Protocol Data Transfer packet (TP.DT).
 * @param sa Source address.
 * @param sequence Packet sequence number (1 to 255).
 * @param payload Pointer to payload chunk (up to 7 bytes).
 * @param payload_len Length of chunk (1 to 7 bytes).
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_j1939_build_tp_dt(uint8_t sa, uint8_t sequence, const uint8_t *payload, size_t payload_len, SYN_CAN_Frame *frame);

/**
 * @brief Encode Active Diagnostic Trouble Codes into a DM1 buffer (PGN 65226).
 * @param buf Output buffer.
 * @param buf_size Buffer capacity.
 * @param dtc_list Array of DTC structures.
 * @param dtc_count Number of DTCs in list.
 * @param mil_lamp_status 2-bit MIL status.
 * @return Total bytes written to buffer.
 */
size_t syn_j1939_encode_dm1(uint8_t *buf, size_t buf_size, const SYN_J1939_DTC *dtc_list, size_t dtc_count, uint8_t mil_lamp_status);

/**
 * @brief Process an incoming CAN frame in the J1939 node state machine.
 * @param node Pointer to node.
 * @param frame Incoming CAN frame.
 * @param out_pgn Output received PGN.
 * @param out_data Pointer to receiving reassembled or single-frame payload.
 * @param out_len Output payload length.
 * @return SYN_OK if a complete J1939 message is ready, or SYN_BUSY if multi-packet assembly is in progress.
 */
SYN_Status syn_j1939_process_frame(SYN_J1939_Node *node,
                                   const SYN_CAN_Frame *frame,
                                   uint32_t *out_pgn,
                                   const uint8_t **out_data,
                                   size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_J1939_H */
