

# File syn\_doip.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_doip.c**](syn__doip_8c.md)

[Go to the documentation of this file](syn__doip_8c.md)


```C++

#include "syntropic/proto/syn_doip.h"

#include <string.h>

static inline void poke_u16_be(uint16_t val, uint8_t *buf)
{
    buf[0] = (uint8_t)(val >> 8U);
    buf[1] = (uint8_t)(val & 0xFFU);
}

static inline uint16_t peek_u16_be(const uint8_t *buf)
{
    return (uint16_t)(((uint16_t)buf[0] << 8U) | (uint16_t)buf[1]);
}

static inline void poke_u32_be(uint32_t val, uint8_t *buf)
{
    buf[0] = (uint8_t)(val >> 24U);
    buf[1] = (uint8_t)(val >> 16U);
    buf[2] = (uint8_t)(val >> 8U);
    buf[3] = (uint8_t)(val & 0xFFU);
}

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

    if (rx_len < SYN_DOIP_HEADER_SIZE + hdr.payload_length) {
        return build_generic_nack(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx_buf, max_tx_len, tx_len);
    }

    const uint8_t *payload = &rx_buf[SYN_DOIP_HEADER_SIZE];

    switch (hdr.payload_type) {
    case SYN_DOIP_TYPE_VEHICLE_ID_REQ:
    case SYN_DOIP_TYPE_VEHICLE_ID_REQ_EID:
    case SYN_DOIP_TYPE_VEHICLE_ID_REQ_VIN: {
        /* Vehicle Announcement Response (33 bytes payload) */
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
        memcpy(p, server->vin, 17);                   /* VIN 17 bytes */
        poke_u16_be(server->logical_address, &p[17]); /* Logical Address 2 bytes */
        memcpy(&p[19], server->eid, 6);               /* EID 6 bytes */
        memcpy(&p[25], server->gid, 6);               /* GID 6 bytes */
        p[31] = 0x00U;                                /* Further action required: none */
        p[32] = 0x00U;                                /* VIN/GID sync status: synced */

        *tx_len = (uint16_t)(SYN_DOIP_HEADER_SIZE + payload_len);
        return true;
    }

    case SYN_DOIP_TYPE_ROUTING_ACTIVATION_REQ: {
        if (hdr.payload_length < 7U) {
            return build_generic_nack(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx_buf, max_tx_len,
                                      tx_len);
        }
        uint16_t tester_addr = peek_u16_be(&payload[0]);
        uint8_t act_type = payload[2];
        (void)act_type;

        server->active_tester_address = tester_addr;
        server->routing_activated = true;

        /* Routing Activation Response (9 bytes payload) */
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
        poke_u16_be(tester_addr, &p[0]);             /* Tester logical address (2B) */
        poke_u16_be(server->logical_address, &p[2]); /* Server logical address (2B) */
        p[4] = SYN_DOIP_ROUTING_SUCCESS;             /* Response code (0x00) */
        poke_u32_be(0x00000000U, &p[5]);             /* Reserved (4B) */

        *tx_len = (uint16_t)(SYN_DOIP_HEADER_SIZE + payload_len);
        return true;
    }

    case SYN_DOIP_TYPE_DIAGNOSTIC_MSG: {
        if (hdr.payload_length < 4U) {
            return build_generic_nack(SYN_DOIP_NACK_INVALID_PAYLOAD_LENGTH, tx_buf, max_tx_len,
                                      tx_len);
        }
        uint16_t sa = peek_u16_be(&payload[0]);
        uint16_t ta = peek_u16_be(&payload[2]);
        (void)sa;

        if (ta != server->logical_address && ta != 0xFFFFU) {
            /* Diagnostic Message NACK (5 bytes payload) */
            uint32_t payload_len = 5U;
            if (max_tx_len < SYN_DOIP_HEADER_SIZE + payload_len) {
                return false;
            }
            SYN_DoIP_Header resp_hdr = {.protocol_version = SYN_DOIP_PROTOCOL_VERSION,
                                        .inverse_protocol_version =
                                            SYN_DOIP_INVERSE_PROTOCOL_VERSION,
                                        .payload_type = SYN_DOIP_TYPE_DIAGNOSTIC_MSG_NACK,
                                        .payload_length = payload_len};
            syn_doip_encode_header(&resp_hdr, tx_buf, max_tx_len);
            uint8_t *p = &tx_buf[SYN_DOIP_HEADER_SIZE];
            poke_u16_be(server->logical_address, &p[0]);
            poke_u16_be(sa, &p[2]);
            p[4] = 0x02U; /* NACK code: Unknown target address */
            *tx_len = (uint16_t)(SYN_DOIP_HEADER_SIZE + payload_len);
            return true;
        }

        const uint8_t *uds_req = &payload[4];
        uint16_t uds_req_len = (uint16_t)(hdr.payload_length - 4U);

        /* Diagnostic Message Positive ACK + UDS Response payload */
        uint8_t uds_resp[512];
        uint16_t uds_resp_len = 0U;

        if (uds != NULL && uds_req_len > 0U) {
            syn_uds_process_request(uds, uds_req, uds_req_len, uds_resp, sizeof(uds_resp),
                                    &uds_resp_len, SYN_UDS_ADDR_PHYSICAL);
        }

        uint32_t payload_len = 4U + uds_resp_len;
        if (max_tx_len < SYN_DOIP_HEADER_SIZE + payload_len) {
            return false;
        }

        SYN_DoIP_Header resp_hdr = {.protocol_version = SYN_DOIP_PROTOCOL_VERSION,
                                    .inverse_protocol_version = SYN_DOIP_INVERSE_PROTOCOL_VERSION,
                                    .payload_type = SYN_DOIP_TYPE_DIAGNOSTIC_MSG,
                                    .payload_length = payload_len};
        syn_doip_encode_header(&resp_hdr, tx_buf, max_tx_len);

        uint8_t *p = &tx_buf[SYN_DOIP_HEADER_SIZE];
        poke_u16_be(server->logical_address, &p[0]); /* Source address = ECU (2B) */
        poke_u16_be(sa, &p[2]);                      /* Target address = Tester (2B) */
        if (uds_resp_len > 0U) {
            memcpy(&p[4], uds_resp, uds_resp_len);
        }

        *tx_len = (uint16_t)(SYN_DOIP_HEADER_SIZE + payload_len);
        return true;
    }

    default:
        return build_generic_nack(SYN_DOIP_NACK_UNKNOWN_PAYLOAD_TYPE, tx_buf, max_tx_len, tx_len);
    }
}
```


