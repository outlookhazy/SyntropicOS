

# File syn\_modbus\_tcp.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus\_tcp.c**](syn__modbus__tcp_8c.md)

[Go to the documentation of this file](syn__modbus__tcp_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_MODBUS) || SYN_USE_MODBUS

#include "syn_modbus_tcp.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"

#include <string.h>

void syn_mbap_encode_header(const SYN_MBAP_Header *hdr, uint8_t *buf)
{
    SYN_ASSERT(hdr != NULL);
    SYN_ASSERT(buf != NULL);

    size_t pos = 0;
    syn_pack_u16(buf, &pos, hdr->transaction_id);
    syn_pack_u16(buf, &pos, hdr->protocol_id);
    syn_pack_u16(buf, &pos, hdr->length);
    syn_pack_u8(buf, &pos, hdr->unit_id);
}

bool syn_mbap_decode_header(const uint8_t *buf, SYN_MBAP_Header *hdr)
{
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(hdr != NULL);

    size_t pos = 0;
    hdr->transaction_id = syn_unpack_u16(buf, &pos);
    hdr->protocol_id    = syn_unpack_u16(buf, &pos);
    hdr->length         = syn_unpack_u16(buf, &pos);
    hdr->unit_id        = syn_unpack_u8(buf, &pos);

    return (hdr->protocol_id == 0x0000);
}

static uint16_t get_response_pdu_len(const uint8_t *buf)
{
    uint8_t fc = buf[1];
    if (fc & 0x80) {
        return 2; /* Function Code + Exception Code */
    }

    switch (fc) {
    case SYN_MB_FC_READ_COILS:
    case SYN_MB_FC_READ_DISCRETE_INPUTS:
    case SYN_MB_FC_READ_HOLDING:
    case SYN_MB_FC_READ_INPUT:
    case SYN_MB_FC_REPORT_SERVER_ID:
        return (uint16_t)(2 + buf[2]);

    case SYN_MB_FC_WRITE_SINGLE_COIL:
    case SYN_MB_FC_WRITE_SINGLE:
    case SYN_MB_FC_WRITE_MULTIPLE_COILS:
    case SYN_MB_FC_WRITE_MULTIPLE:
        return 5;

    case SYN_MB_FC_READ_EXCEPTION_STATUS:
        return 2;

    case SYN_MB_FC_MASK_WRITE_REGISTER:
        return 7;

    default:
        return (uint16_t)(2 + buf[2]);
    }
}

bool syn_modbus_tcp_process_slave(SYN_Modbus *mb,
                                  const uint8_t *req_adu, uint16_t req_len,
                                  uint8_t *resp_adu, uint16_t resp_max,
                                  uint16_t *resp_len)
{
    SYN_ASSERT(mb != NULL);
    SYN_ASSERT(req_adu != NULL);
    SYN_ASSERT(resp_adu != NULL);
    SYN_ASSERT(resp_len != NULL);

    *resp_len = 0;

    if (req_len < SYN_MBAP_HEADER_LEN + 1) {
        return false; /* ADU too short */
    }

    SYN_MBAP_Header req_hdr;
    if (!syn_mbap_decode_header(req_adu, &req_hdr)) {
        return false; /* Not Modbus TCP protocol */
    }

    uint16_t expected_payload_len = req_hdr.length;
    if (expected_payload_len < 2 || req_len < SYN_MBAP_HEADER_LEN - 1 + expected_payload_len) {
        return false; /* Length mismatch */
    }

    uint16_t pdu_len = expected_payload_len - 1; /* Unit ID is 1 byte */
    if (pdu_len > mb->buf_size - 3) {
        return false;
    }

    /* Unit ID check: Accept if matching slave address or unit_id == 0 or 255 */
    if (req_hdr.unit_id != mb->cfg.slave_addr && req_hdr.unit_id != 0 && req_hdr.unit_id != 0xFF) {
        return false;
    }

    /* Copy Unit ID and PDU into slave buffer, then append CRC for syn_modbus_process */
    mb->buf[0] = req_hdr.unit_id;
    memcpy(&mb->buf[1], &req_adu[SYN_MBAP_HEADER_LEN], pdu_len);

    uint16_t crc = syn_crc16_modbus(mb->buf, 1 + pdu_len);
    mb->buf[1 + pdu_len] = (uint8_t)(crc & 0xFF);
    mb->buf[2 + pdu_len] = (uint8_t)((crc >> 8) & 0xFF);
    mb->rx_len = 3 + pdu_len;

    /* Process PDU using slave dispatcher */
    bool has_response = syn_modbus_process(mb);

    /* Broadcast or exception without response */
    if (!has_response || mb->buf[0] == 0) {
        return false;
    }

    uint16_t resp_pdu_len = get_response_pdu_len(mb->buf);
    uint16_t total_resp_adu_len = SYN_MBAP_HEADER_LEN + resp_pdu_len;
    if (total_resp_adu_len > resp_max) {
        return false;
    }

    SYN_MBAP_Header resp_hdr;
    resp_hdr.transaction_id = req_hdr.transaction_id;
    resp_hdr.protocol_id    = 0x0000;
    resp_hdr.length         = (uint16_t)(1 + resp_pdu_len); /* Unit ID (1) + PDU */
    resp_hdr.unit_id        = req_hdr.unit_id;

    syn_mbap_encode_header(&resp_hdr, resp_adu);
    memcpy(&resp_adu[SYN_MBAP_HEADER_LEN], &mb->buf[1], resp_pdu_len);

    *resp_len = total_resp_adu_len;
    return true;
}

SYN_Status syn_modbus_tcp_build_client_adu(uint16_t transaction_id, uint8_t unit_id,
                                           const uint8_t *pdu, uint16_t pdu_len,
                                           uint8_t *req_adu, uint16_t req_max,
                                           uint16_t *req_len)
{
    SYN_ASSERT(pdu != NULL);
    SYN_ASSERT(req_adu != NULL);
    SYN_ASSERT(req_len != NULL);

    if (pdu_len == 0 || pdu_len > 253) return SYN_INVALID_PARAM;

    uint16_t total_len = SYN_MBAP_HEADER_LEN + pdu_len;
    if (total_len > req_max) return SYN_INVALID_PARAM;

    SYN_MBAP_Header hdr;
    hdr.transaction_id = transaction_id;
    hdr.protocol_id    = 0x0000;
    hdr.length         = (uint16_t)(1 + pdu_len);
    hdr.unit_id        = unit_id;

    syn_mbap_encode_header(&hdr, req_adu);
    memcpy(&req_adu[SYN_MBAP_HEADER_LEN], pdu, pdu_len);

    *req_len = total_len;
    return SYN_OK;
}

#endif /* SYN_USE_MODBUS */
```


