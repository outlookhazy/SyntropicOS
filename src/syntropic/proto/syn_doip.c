/**
 * @file syn_doip.c
 * @brief SyntropicOS ISO 13400-2 Diagnostic over IP (DoIP) Implementation.
 */

#include "syntropic/proto/syn_doip.h"

#if SYN_DOIP_ENABLED

#include <string.h>

/**
 * @brief Pack 16-bit unsigned integer into big-endian byte array.
 * @param val 16-bit integer value.
 * @param buf Target byte buffer.
 */
static inline void poke_u16_be(uint16_t val, uint8_t *buf)
{
    buf[0] = (uint8_t)(val >> 8U);
    buf[1] = (uint8_t)(val & 0xFFU);
}

/**
 * @brief Unpack 16-bit unsigned integer from big-endian byte array.
 * @param buf Input byte buffer.
 * @return 16-bit integer value.
 */
static inline uint16_t peek_u16_be(const uint8_t *buf)
{
    return (uint16_t)(((uint16_t)buf[0] << 8U) | (uint16_t)buf[1]);
}

/**
 * @brief Pack 32-bit unsigned integer into big-endian byte array.
 * @param val 32-bit integer value.
 * @param buf Target byte buffer.
 */
static inline void poke_u32_be(uint32_t val, uint8_t *buf)
{
    buf[0] = (uint8_t)(val >> 24U);
    buf[1] = (uint8_t)(val >> 16U);
    buf[2] = (uint8_t)(val >> 8U);
    buf[3] = (uint8_t)(val & 0xFFU);
}

/**
 * @brief Unpack 32-bit unsigned integer from big-endian byte array.
 * @param buf Input byte buffer.
 * @return 32-bit integer value.
 */
static inline uint32_t peek_u32_be(const uint8_t *buf)
{
    return ((uint32_t)buf[0] << 24U) | ((uint32_t)buf[1] << 16U) | ((uint32_t)buf[2] << 8U) |
           (uint32_t)buf[3];
}

bool syn_doip_init(SYN_DoIP_Server *server, uint16_t logical_address)
{
    if (server == NULL) {
        return false;
    }
    memset(server, 0, sizeof(SYN_DoIP_Server));
    server->logical_address = logical_address;
    server->routing_activated = false;
    return true;
}

bool syn_doip_set_identifiers(SYN_DoIP_Server *server, const uint8_t vin[17], const uint8_t eid[6],
                              const uint8_t gid[6])
{
    if (server == NULL) {
        return false;
    }
    if (vin != NULL) {
        memcpy(server->vin, vin, 17);
    }
    if (eid != NULL) {
        memcpy(server->eid, eid, 6);
    }
    if (gid != NULL) {
        memcpy(server->gid, gid, 6);
    }
    return true;
}

bool syn_doip_parse_header(const uint8_t *buf, uint16_t len, SYN_DoIP_Header *hdr)
{
    if (buf == NULL || len < SYN_DOIP_HEADER_SIZE || hdr == NULL) {
        return false;
    }
    hdr->protocol_version = buf[0];
    hdr->inverse_protocol_version = buf[1];
    hdr->payload_type = peek_u16_be(&buf[2]);
    hdr->payload_length = peek_u32_be(&buf[4]);

    if (hdr->protocol_version != SYN_DOIP_PROTOCOL_VERSION ||
        hdr->inverse_protocol_version != SYN_DOIP_INVERSE_PROTOCOL_VERSION) {
        return false;
    }
    return true;
}

uint16_t syn_doip_encode_header(const SYN_DoIP_Header *hdr, uint8_t *buf, uint16_t max_len)
{
    if (hdr == NULL || buf == NULL || max_len < SYN_DOIP_HEADER_SIZE) {
        return 0U;
    }
    buf[0] = hdr->protocol_version;
    buf[1] = hdr->inverse_protocol_version;
    poke_u16_be(hdr->payload_type, &buf[2]);
    poke_u32_be(hdr->payload_length, &buf[4]);
    return SYN_DOIP_HEADER_SIZE;
}

/**
 * @brief Build DoIP Generic NACK response.
 * @param nack_code NACK error code.
 * @param tx_buf Target output buffer.
 * @param max_tx Capacity of tx_buf.
 * @param tx_len Output length pointer.
 * @return true on success, false if buffer is too small.
 */
static bool build_generic_nack(uint8_t nack_code, uint8_t *tx_buf, uint16_t max_tx,
                               uint16_t *tx_len)
{
    if (max_tx < SYN_DOIP_HEADER_SIZE + 1U) {
        return false;
    }
    SYN_DoIP_Header resp_hdr = {.protocol_version = SYN_DOIP_PROTOCOL_VERSION,
                                .inverse_protocol_version = SYN_DOIP_INVERSE_PROTOCOL_VERSION,
                                .payload_type = SYN_DOIP_TYPE_GENERIC_NACK,
                                .payload_length = 1U};
    syn_doip_encode_header(&resp_hdr, tx_buf, max_tx);
    tx_buf[8] = nack_code;
    *tx_len = SYN_DOIP_HEADER_SIZE + 1U;
    return true;
}

/**
 * @brief Build DoIP Diagnostic Message NACK response.
 * @param server DoIP server instance.
 * @param sa Tester source logical address.
 * @param nack_code Diagnostic NACK code.
 * @param tx_buf Target output buffer.
 * @param max_tx_len Capacity of tx_buf in bytes.
 * @param tx_len Output frame length in bytes.
 * @return true on success, false if tx_buf is too small.
 */
static bool build_diag_nack(const SYN_DoIP_Server *server, uint16_t sa, uint8_t nack_code,
                            uint8_t *tx_buf, uint16_t max_tx_len, uint16_t *tx_len)
{
    uint32_t payload_len = 5U;
    if (max_tx_len < SYN_DOIP_HEADER_SIZE + payload_len) {
        return false;
    }
    SYN_DoIP_Header resp_hdr = {.protocol_version = SYN_DOIP_PROTOCOL_VERSION,
                                .inverse_protocol_version = SYN_DOIP_INVERSE_PROTOCOL_VERSION,
                                .payload_type = SYN_DOIP_TYPE_DIAGNOSTIC_MSG_NACK,
                                .payload_length = payload_len};
    syn_doip_encode_header(&resp_hdr, tx_buf, max_tx_len);
    uint8_t *p = &tx_buf[SYN_DOIP_HEADER_SIZE];
    poke_u16_be(server->logical_address, &p[0]);
    poke_u16_be(sa, &p[2]);
    p[4] = nack_code;
    *tx_len = (uint16_t)(SYN_DOIP_HEADER_SIZE + payload_len);
    return true;
}

/**
 * @brief Handle DoIP Vehicle Identification Request (generic, EID, or VIN).
 * @param server DoIP server instance.
 * @param hdr Parsed DoIP request header.
 * @param payload Request payload bytes after DoIP header.
 * @param tx_buf Target output buffer.
 * @param max_tx_len Capacity of tx_buf in bytes.
 * @param tx_len Output frame length in bytes.
 * @return true on success, false if tx_buf is too small.
 */
static bool handle_vehicle_id_req(const SYN_DoIP_Server *server, const SYN_DoIP_Header *hdr,
                                  const uint8_t *payload, uint8_t *tx_buf, uint16_t max_tx_len,
                                  uint16_t *tx_len)
{
    if (hdr->payload_type == SYN_DOIP_TYPE_VEHICLE_ID_REQ) {
        if (hdr->payload_length != 0U) {
            return build_generic_nack(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx_buf, max_tx_len,
                                      tx_len);
        }
    } else if (hdr->payload_type == SYN_DOIP_TYPE_VEHICLE_ID_REQ_EID) {
        if (hdr->payload_length != 6U) {
            return build_generic_nack(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx_buf, max_tx_len,
                                      tx_len);
        }
        if (memcmp(payload, server->eid, 6U) != 0) {
            *tx_len = 0U;
            return true;
        }
    } else {
        if (hdr->payload_length != 17U) {
            return build_generic_nack(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx_buf, max_tx_len,
                                      tx_len);
        }
        if (memcmp(payload, server->vin, 17U) != 0) {
            *tx_len = 0U;
            return true;
        }
    }

    uint32_t payload_len = 33U;
    if (max_tx_len < SYN_DOIP_HEADER_SIZE + payload_len) {
        return false;
    }
    SYN_DoIP_Header resp_hdr = {.protocol_version = SYN_DOIP_PROTOCOL_VERSION,
                                .inverse_protocol_version = SYN_DOIP_INVERSE_PROTOCOL_VERSION,
                                .payload_type = SYN_DOIP_TYPE_VEHICLE_ANNOUNCEMENT,
                                .payload_length = payload_len};
    syn_doip_encode_header(&resp_hdr, tx_buf, max_tx_len);

    uint8_t *p = &tx_buf[SYN_DOIP_HEADER_SIZE];
    memcpy(p, server->vin, 17U);
    poke_u16_be(server->logical_address, &p[17]);
    memcpy(&p[19], server->eid, 6U);
    memcpy(&p[25], server->gid, 6U);
    p[31] = 0x00U;
    p[32] = 0x00U;

    *tx_len = (uint16_t)(SYN_DOIP_HEADER_SIZE + payload_len);
    return true;
}

/**
 * @brief Handle DoIP Routing Activation Request and encode response.
 * @param server DoIP server instance.
 * @param hdr Parsed DoIP request header.
 * @param payload Request payload bytes after DoIP header.
 * @param tx_buf Target output buffer.
 * @param max_tx_len Capacity of tx_buf in bytes.
 * @param tx_len Output frame length in bytes.
 * @return true on success, false if tx_buf is too small.
 */
static bool handle_routing_activation(SYN_DoIP_Server *server, const SYN_DoIP_Header *hdr,
                                      const uint8_t *payload, uint8_t *tx_buf, uint16_t max_tx_len,
                                      uint16_t *tx_len)
{
    if (hdr->payload_length < 7U) {
        return build_generic_nack(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx_buf, max_tx_len, tx_len);
    }
    uint16_t tester_addr = peek_u16_be(&payload[0]);
    server->active_tester_address = tester_addr;
    server->routing_activated = true;

    uint32_t payload_len = 9U;
    if (max_tx_len < SYN_DOIP_HEADER_SIZE + payload_len) {
        return false;
    }
    SYN_DoIP_Header resp_hdr = {.protocol_version = SYN_DOIP_PROTOCOL_VERSION,
                                .inverse_protocol_version = SYN_DOIP_INVERSE_PROTOCOL_VERSION,
                                .payload_type = SYN_DOIP_TYPE_ROUTING_ACTIVATION_RESP,
                                .payload_length = payload_len};
    syn_doip_encode_header(&resp_hdr, tx_buf, max_tx_len);

    uint8_t *p = &tx_buf[SYN_DOIP_HEADER_SIZE];
    poke_u16_be(tester_addr, &p[0]);
    poke_u16_be(server->logical_address, &p[2]);
    p[4] = SYN_DOIP_ROUTING_SUCCESS;
    poke_u32_be(0x00000000U, &p[5]);

    *tx_len = (uint16_t)(SYN_DOIP_HEADER_SIZE + payload_len);
    return true;
}

/**
 * @brief Handle DoIP Diagnostic Message, dispatch UDS payload, and encode ACK plus response.
 * @param server DoIP server instance.
 * @param uds Optional UDS server instance.
 * @param hdr Parsed DoIP request header.
 * @param payload Request payload bytes after DoIP header.
 * @param tx_buf Target output buffer.
 * @param max_tx_len Capacity of tx_buf in bytes.
 * @param tx_len Output frame length in bytes.
 * @return true on success, false if tx_buf is too small.
 */
static bool handle_diagnostic_msg(SYN_DoIP_Server *server, SYN_UDS_Server *uds,
                                  const SYN_DoIP_Header *hdr, const uint8_t *payload,
                                  uint8_t *tx_buf, uint16_t max_tx_len, uint16_t *tx_len)
{
    if (hdr->payload_length < 4U) {
        return build_generic_nack(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx_buf, max_tx_len, tx_len);
    }
    uint16_t sa = peek_u16_be(&payload[0]);
    uint16_t ta = peek_u16_be(&payload[2]);

    if (!server->routing_activated || sa != server->active_tester_address) {
        return build_diag_nack(server, sa, SYN_DOIP_DIAG_NACK_INVALID_SA, tx_buf, max_tx_len,
                               tx_len);
    }
    if (ta != server->logical_address && ta != 0xFFFFU) {
        return build_diag_nack(server, sa, SYN_DOIP_DIAG_NACK_UNKNOWN_TA, tx_buf, max_tx_len,
                               tx_len);
    }

    const uint8_t *uds_req = &payload[4];
    uint16_t uds_req_len = (uint16_t)(hdr->payload_length - 4U);
    uint8_t uds_resp[512];
    uint16_t uds_resp_len = 0U;

    if (uds != NULL && uds_req_len > 0U) {
        SYN_UDS_AddrMode addr_type =
            (ta == 0xFFFFU) ? SYN_UDS_ADDR_FUNCTIONAL : SYN_UDS_ADDR_PHYSICAL;
        syn_uds_process_request(uds, uds_req, uds_req_len, uds_resp, (uint16_t)sizeof(uds_resp),
                                &uds_resp_len, addr_type);
    }

    uint32_t ack_frame_len = SYN_DOIP_HEADER_SIZE + 5U;
    uint32_t diag_frame_len =
        (uds_resp_len > 0U) ? (uint32_t)(SYN_DOIP_HEADER_SIZE + 4U + uds_resp_len) : 0U;
    if ((uint32_t)max_tx_len < ack_frame_len + diag_frame_len) {
        return false;
    }

    SYN_DoIP_Header ack_hdr = {.protocol_version = SYN_DOIP_PROTOCOL_VERSION,
                               .inverse_protocol_version = SYN_DOIP_INVERSE_PROTOCOL_VERSION,
                               .payload_type = SYN_DOIP_TYPE_DIAGNOSTIC_MSG_ACK,
                               .payload_length = 5U};
    syn_doip_encode_header(&ack_hdr, tx_buf, max_tx_len);
    poke_u16_be(server->logical_address, &tx_buf[8]);
    poke_u16_be(sa, &tx_buf[10]);
    tx_buf[12] = SYN_DOIP_DIAG_ACK_OK;

    if (uds_resp_len > 0U) {
        uint8_t *diag_out = &tx_buf[ack_frame_len];
        SYN_DoIP_Header resp_hdr = {.protocol_version = SYN_DOIP_PROTOCOL_VERSION,
                                    .inverse_protocol_version = SYN_DOIP_INVERSE_PROTOCOL_VERSION,
                                    .payload_type = SYN_DOIP_TYPE_DIAGNOSTIC_MSG,
                                    .payload_length = 4U + (uint32_t)uds_resp_len};
        syn_doip_encode_header(&resp_hdr, diag_out, (uint16_t)(max_tx_len - ack_frame_len));
        poke_u16_be(server->logical_address, &diag_out[8]);
        poke_u16_be(sa, &diag_out[10]);
        memcpy(&diag_out[12], uds_resp, uds_resp_len);
    }

    *tx_len = (uint16_t)(ack_frame_len + diag_frame_len);
    return true;
}

bool syn_doip_process_msg(SYN_DoIP_Server *server, SYN_UDS_Server *uds, const uint8_t *rx_buf,
                          uint16_t rx_len, uint8_t *tx_buf, uint16_t max_tx_len, uint16_t *tx_len)
{
    if (server == NULL || rx_buf == NULL || tx_buf == NULL || tx_len == NULL) {
        return false;
    }
    *tx_len = 0U;

    SYN_DoIP_Header hdr;
    if (!syn_doip_parse_header(rx_buf, rx_len, &hdr)) {
        return build_generic_nack(SYN_DOIP_NACK_INCORRECT_PATTERN, tx_buf, max_tx_len, tx_len);
    }

    if ((size_t)hdr.payload_length > (size_t)(rx_len - SYN_DOIP_HEADER_SIZE)) {
        return build_generic_nack(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx_buf, max_tx_len, tx_len);
    }

    const uint8_t *payload = &rx_buf[SYN_DOIP_HEADER_SIZE];

    switch (hdr.payload_type) {
    case SYN_DOIP_TYPE_VEHICLE_ID_REQ:
    case SYN_DOIP_TYPE_VEHICLE_ID_REQ_EID:
    case SYN_DOIP_TYPE_VEHICLE_ID_REQ_VIN:
        return handle_vehicle_id_req(server, &hdr, payload, tx_buf, max_tx_len, tx_len);
    case SYN_DOIP_TYPE_ROUTING_ACTIVATION_REQ:
        return handle_routing_activation(server, &hdr, payload, tx_buf, max_tx_len, tx_len);
    case SYN_DOIP_TYPE_DIAGNOSTIC_MSG:
        return handle_diagnostic_msg(server, uds, &hdr, payload, tx_buf, max_tx_len, tx_len);
    default:
        return build_generic_nack(SYN_DOIP_NACK_UNKNOWN_PAYLOAD_TYPE, tx_buf, max_tx_len, tx_len);
    }
}

#endif /* SYN_DOIP_ENABLED */
