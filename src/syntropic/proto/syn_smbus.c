/**
 * @file syn_smbus.c
 * @brief SMBus Protocol Engine implementation.
 * @ingroup syn_protocol
 */

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#include "syn_smbus.h"

#include <string.h>

#if !defined(SYN_USE_SMBUS) || SYN_USE_SMBUS

uint8_t syn_smbus_calc_pec(uint8_t init_crc, const uint8_t *data, size_t len)
{
    if (data == NULL) {
        return init_crc;
    }
    uint8_t crc = init_crc;
    for (size_t i = 0U; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((crc << 1U) ^ 0x07U);
            } else {
                crc = (uint8_t)(crc << 1U);
            }
        }
    }
    return crc;
}

/**
 * @brief Check whether an SMBus transaction appends PEC directly to the master request phase.
 * @param proto SMBus transaction protocol type.
 * @return true for write-only protocols with request PEC, false otherwise.
 */
static bool smbus_has_request_pec(SYN_SMBUS_Protocol proto)
{
    return (proto == SYN_SMBUS_PROTO_SEND_BYTE || proto == SYN_SMBUS_PROTO_WRITE_BYTE ||
            proto == SYN_SMBUS_PROTO_WRITE_WORD || proto == SYN_SMBUS_PROTO_WRITE_BLOCK);
}

/**
 * @brief Encode SMBus write-direction packet bytes prior to optional PEC byte.
 * @param pkt SMBus packet descriptor.
 * @param addr_w 8-bit shifted slave write address byte.
 * @param tx_buf Output transmit buffer.
 * @param buf_size Capacity of tx_buf in bytes.
 * @param pec_extra Additional byte count reserved for PEC (0 or 1).
 * @param out_idx Output encoded byte length before PEC.
 * @return SYN_OK on success, SYN_ERROR if tx_buf is too small.
 */
static SYN_Status smbus_encode_write(const SYN_SMBUS_Packet *pkt, uint8_t addr_w, uint8_t *tx_buf,
                                     size_t buf_size, size_t pec_extra, size_t *out_idx)
{
    size_t idx = 0U;

    switch (pkt->proto) {
    case SYN_SMBUS_PROTO_SEND_BYTE:
        if (buf_size < (2U + pec_extra)) {
            return SYN_ERROR;
        }
        tx_buf[idx++] = addr_w;
        tx_buf[idx++] = pkt->command;
        break;

    case SYN_SMBUS_PROTO_WRITE_BYTE:
        if (buf_size < (3U + pec_extra)) {
            return SYN_ERROR;
        }
        tx_buf[idx++] = addr_w;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = (pkt->length > 0U) ? pkt->data[0] : 0U;
        break;

    case SYN_SMBUS_PROTO_WRITE_WORD:
        if (buf_size < (4U + pec_extra)) {
            return SYN_ERROR;
        }
        tx_buf[idx++] = addr_w;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = (pkt->length >= 1U) ? pkt->data[0] : 0U;
        tx_buf[idx++] = (pkt->length >= 2U) ? pkt->data[1] : 0U;
        break;

    default: {
        uint8_t payload_len =
            (pkt->length > SYN_SMBUS_BLOCK_MAX_LEN) ? SYN_SMBUS_BLOCK_MAX_LEN : pkt->length;
        if (buf_size < (3U + (size_t)payload_len + pec_extra)) {
            return SYN_ERROR;
        }
        tx_buf[idx++] = addr_w;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = payload_len;
        if (payload_len > 0U) {
            memcpy(&tx_buf[idx], pkt->data, payload_len);
            idx += payload_len;
        }
        break;
    }
    }

    *out_idx = idx;
    return SYN_OK;
}

/**
 * @brief Encode master write phase for SMBus Process Call or Block Process Call.
 * @param pkt SMBus packet descriptor.
 * @param addr_w 8-bit shifted slave write address byte.
 * @param tx_buf Output transmit buffer.
 * @param buf_size Capacity of tx_buf in bytes.
 * @param out_idx Output encoded byte length.
 * @return SYN_OK on success, SYN_ERROR if tx_buf is too small.
 */
static SYN_Status smbus_encode_proc_call(const SYN_SMBUS_Packet *pkt, uint8_t addr_w,
                                         uint8_t *tx_buf, size_t buf_size, size_t *out_idx)
{
    size_t idx = 0U;

    if (pkt->proto == SYN_SMBUS_PROTO_PROCESS_CALL) {
        if (buf_size < 4U) {
            return SYN_ERROR;
        }
        tx_buf[idx++] = addr_w;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = (pkt->length >= 1U) ? pkt->data[0] : 0U;
        tx_buf[idx++] = (pkt->length >= 2U) ? pkt->data[1] : 0U;
    } else {
        uint8_t payload_len =
            (pkt->length > SYN_SMBUS_BLOCK_MAX_LEN) ? SYN_SMBUS_BLOCK_MAX_LEN : pkt->length;
        if (buf_size < (3U + (size_t)payload_len)) {
            return SYN_ERROR;
        }
        tx_buf[idx++] = addr_w;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = payload_len;
        if (payload_len > 0U) {
            memcpy(&tx_buf[idx], pkt->data, payload_len);
            idx += payload_len;
        }
    }

    *out_idx = idx;
    return SYN_OK;
}

/**
 * @brief Encode SMBus Quick, Receive Byte, Read, or Process Call request phase.
 * @param pkt SMBus packet descriptor.
 * @param addr_w 8-bit shifted slave write address byte.
 * @param addr_r 8-bit shifted slave read address byte.
 * @param tx_buf Output transmit buffer.
 * @param buf_size Capacity of tx_buf in bytes.
 * @param out_idx Output encoded byte length.
 * @return SYN_OK on success, SYN_ERROR on buffer overflow, SYN_INVALID_PARAM on unknown protocol.
 */
static SYN_Status smbus_encode_read_or_call(const SYN_SMBUS_Packet *pkt, uint8_t addr_w,
                                            uint8_t addr_r, uint8_t *tx_buf, size_t buf_size,
                                            size_t *out_idx)
{
    switch (pkt->proto) {
    case SYN_SMBUS_PROTO_QUICK_READ:
    case SYN_SMBUS_PROTO_QUICK_WRITE:
    case SYN_SMBUS_PROTO_RECEIVE_BYTE:
        if (buf_size < 1U) {
            return SYN_ERROR;
        }
        tx_buf[0] = (pkt->proto == SYN_SMBUS_PROTO_QUICK_WRITE) ? addr_w : addr_r;
        *out_idx = 1U;
        return SYN_OK;

    case SYN_SMBUS_PROTO_READ_BYTE:
    case SYN_SMBUS_PROTO_READ_WORD:
    case SYN_SMBUS_PROTO_READ_BLOCK:
        if (buf_size < 2U) {
            return SYN_ERROR;
        }
        tx_buf[0] = addr_w;
        tx_buf[1] = pkt->command;
        *out_idx = 2U;
        return SYN_OK;

    case SYN_SMBUS_PROTO_PROCESS_CALL:
    case SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL:
        return smbus_encode_proc_call(pkt, addr_w, tx_buf, buf_size, out_idx);

    default:
        return SYN_INVALID_PARAM;
    }
}

SYN_Status syn_smbus_encode_packet(const SYN_SMBUS_Packet *pkt, uint8_t *tx_buf, size_t buf_size,
                                   size_t *out_len)
{
    if (pkt == NULL || tx_buf == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t addr_w = (uint8_t)(pkt->slave_addr << 1U);
    uint8_t addr_r = (uint8_t)((pkt->slave_addr << 1U) | 0x01U);
    bool append_pec = pkt->pec_enabled && smbus_has_request_pec(pkt->proto);
    size_t idx = 0U;
    SYN_Status st;

    if (smbus_has_request_pec(pkt->proto)) {
        st = smbus_encode_write(pkt, addr_w, tx_buf, buf_size, append_pec ? 1U : 0U, &idx);
    } else {
        st = smbus_encode_read_or_call(pkt, addr_w, addr_r, tx_buf, buf_size, &idx);
    }

    if (st != SYN_OK) {
        return st;
    }

    if (append_pec) {
        tx_buf[idx] = syn_smbus_calc_pec(0U, tx_buf, idx);
        idx++;
    }

    *out_len = idx;
    return SYN_OK;
}

/**
 * @brief Compute running PEC seed over request header/payload for Process Call transactions.
 * @param req_pkt Request SMBus packet descriptor.
 * @param addr_w 8-bit shifted slave write address byte.
 * @param addr_r 8-bit shifted slave read address byte.
 * @return Running CRC-8 PEC value prior to slave read data bytes.
 */
static uint8_t smbus_calc_call_pec(const SYN_SMBUS_Packet *req_pkt, uint8_t addr_w, uint8_t addr_r)
{
    if (req_pkt->proto == SYN_SMBUS_PROTO_PROCESS_CALL) {
        uint8_t hdr[5] = {addr_w, req_pkt->command, (req_pkt->length >= 1U) ? req_pkt->data[0] : 0U,
                          (req_pkt->length >= 2U) ? req_pkt->data[1] : 0U, addr_r};
        return syn_smbus_calc_pec(0U, hdr, sizeof(hdr));
    }

    uint8_t wlen =
        (req_pkt->length > SYN_SMBUS_BLOCK_MAX_LEN) ? SYN_SMBUS_BLOCK_MAX_LEN : req_pkt->length;
    uint8_t hdr[3] = {addr_w, req_pkt->command, wlen};
    uint8_t crc = syn_smbus_calc_pec(0U, hdr, sizeof(hdr));
    if (wlen > 0U) {
        crc = syn_smbus_calc_pec(crc, req_pkt->data, wlen);
    }
    return syn_smbus_calc_pec(crc, &addr_r, 1U);
}

uint8_t syn_smbus_calc_transaction_pec(const SYN_SMBUS_Packet *req_pkt, const uint8_t *rx_data,
                                       size_t rx_data_len)
{
    if (req_pkt == NULL || (rx_data == NULL && rx_data_len > 0U)) {
        return 0U;
    }

    uint8_t addr_w = (uint8_t)(req_pkt->slave_addr << 1U);
    uint8_t addr_r = (uint8_t)((req_pkt->slave_addr << 1U) | 0x01U);
    uint8_t crc = 0U;

    switch (req_pkt->proto) {
    case SYN_SMBUS_PROTO_RECEIVE_BYTE:
        crc = syn_smbus_calc_pec(0U, &addr_r, 1U);
        break;

    case SYN_SMBUS_PROTO_READ_BYTE:
    case SYN_SMBUS_PROTO_READ_WORD:
    case SYN_SMBUS_PROTO_READ_BLOCK: {
        uint8_t hdr[3] = {addr_w, req_pkt->command, addr_r};
        crc = syn_smbus_calc_pec(0U, hdr, sizeof(hdr));
        break;
    }

    case SYN_SMBUS_PROTO_PROCESS_CALL:
    case SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL:
        crc = smbus_calc_call_pec(req_pkt, addr_w, addr_r);
        break;

    case SYN_SMBUS_PROTO_SEND_BYTE:
    case SYN_SMBUS_PROTO_WRITE_BYTE:
    case SYN_SMBUS_PROTO_WRITE_WORD:
    case SYN_SMBUS_PROTO_WRITE_BLOCK: {
        uint8_t hdr[2] = {addr_w, req_pkt->command};
        crc = syn_smbus_calc_pec(0U, hdr, sizeof(hdr));
        break;
    }

    default:
        break;
    }

    return syn_smbus_calc_pec(crc, rx_data, rx_data_len);
}

/**
 * @brief Locate 1-byte SMBus payload in either raw-response or full-wire buffer layout.
 * @param proto SMBus transaction protocol type.
 * @param data_len Buffer length excluding trailing PEC byte.
 * @param has_pec Whether PEC byte is present in rx_buf.
 * @param payload_off Output byte offset of 1-byte payload.
 * @param payload_len Output payload byte length (1).
 * @param is_full Set true if rx_buf contains full wire framing, false if payload-only.
 * @return SYN_OK on valid framing, SYN_INVALID_PARAM otherwise.
 */
static SYN_Status smbus_parse_u8(SYN_SMBUS_Protocol proto, size_t data_len, bool has_pec,
                                 size_t *payload_off, uint8_t *payload_len, bool *is_full)
{
    *payload_len = 1U;
    if (data_len == 1U) {
        *payload_off = 0U;
        return SYN_OK;
    }
    if ((proto == SYN_SMBUS_PROTO_RECEIVE_BYTE) && (data_len == 2U)) {
        *payload_off = 1U;
        *is_full = true;
        return SYN_OK;
    }
    if ((proto == SYN_SMBUS_PROTO_READ_BYTE) && (data_len == 4U)) {
        *payload_off = 3U;
        *is_full = true;
        return SYN_OK;
    }
    if (((proto == SYN_SMBUS_PROTO_WRITE_BYTE) ||
         ((proto == SYN_SMBUS_PROTO_READ_BYTE) && !has_pec)) &&
        (data_len == 3U)) {
        *payload_off = 2U;
        *is_full = true;
        return SYN_OK;
    }
    return SYN_INVALID_PARAM;
}

/**
 * @brief Locate 2-byte SMBus word payload in either raw-response or full-wire buffer layout.
 * @param proto SMBus transaction protocol type.
 * @param data_len Buffer length excluding trailing PEC byte.
 * @param has_pec Whether PEC byte is present in rx_buf.
 * @param payload_off Output byte offset of 2-byte word payload.
 * @param payload_len Output payload byte length (2).
 * @param is_full Set true if rx_buf contains full wire framing, false if payload-only.
 * @return SYN_OK on valid framing, SYN_INVALID_PARAM otherwise.
 */
static SYN_Status smbus_parse_u16(SYN_SMBUS_Protocol proto, size_t data_len, bool has_pec,
                                  size_t *payload_off, uint8_t *payload_len, bool *is_full)
{
    *payload_len = 2U;
    if (data_len == 2U) {
        *payload_off = 0U;
        return SYN_OK;
    }
    if ((proto == SYN_SMBUS_PROTO_PROCESS_CALL) && (data_len == 7U)) {
        *payload_off = 5U;
        *is_full = true;
        return SYN_OK;
    }
    if ((proto == SYN_SMBUS_PROTO_READ_WORD) && (data_len == 5U)) {
        *payload_off = 3U;
        *is_full = true;
        return SYN_OK;
    }
    if (((proto == SYN_SMBUS_PROTO_WRITE_WORD) ||
         ((proto == SYN_SMBUS_PROTO_READ_WORD) && !has_pec)) &&
        (data_len == 4U)) {
        *payload_off = 2U;
        *is_full = true;
        return SYN_OK;
    }
    return SYN_INVALID_PARAM;
}

/**
 * @brief Check whether wire address byte matches expected 7-bit slave write address.
 * @param slave_addr Expected 7-bit slave address (0 if wildcard/unspecified).
 * @param wire_addr_w First byte on SMBus wire.
 * @return true if wire_addr_w is a valid matching write address byte, false otherwise.
 */
static bool smbus_match_addr_w(uint8_t slave_addr, uint8_t wire_addr_w)
{
    if (slave_addr != 0U) {
        return wire_addr_w == (uint8_t)(slave_addr << 1U);
    }
    return (wire_addr_w & 1U) == 0U;
}

/**
 * @brief Parse full-wire SMBus Block Process Call buffer and locate read-phase block payload.
 * @param slave_addr Expected 7-bit slave address (0 if wildcard/unspecified).
 * @param rx_buf Received wire buffer.
 * @param data_len Buffer length excluding trailing PEC byte.
 * @param payload_off Output byte offset of response block data.
 * @param payload_len Output response block byte count.
 * @return true if buffer matches Block Process Call framing, false otherwise.
 */
static bool smbus_parse_bpc_block(uint8_t slave_addr, const uint8_t *rx_buf, size_t data_len,
                                  size_t *payload_off, uint8_t *payload_len)
{
    if (data_len >= 5U && smbus_match_addr_w(slave_addr, rx_buf[0]) &&
        rx_buf[2] <= SYN_SMBUS_BLOCK_MAX_LEN && (4U + (size_t)rx_buf[2]) < data_len) {
        size_t addr_r_idx = 3U + (size_t)rx_buf[2];
        size_t cnt_idx = addr_r_idx + 1U;
        uint8_t rx_cnt = rx_buf[cnt_idx];
        if (rx_buf[addr_r_idx] == (uint8_t)(rx_buf[0] | 1U) && rx_cnt <= SYN_SMBUS_BLOCK_MAX_LEN &&
            (cnt_idx + 1U + (size_t)rx_cnt) == data_len) {
            *payload_off = cnt_idx + 1U;
            *payload_len = rx_cnt;
            return true;
        }
    }
    return false;
}

/**
 * @brief Match full-wire SMBus block framing before raw count+payload fallback.
 * @param pkt SMBus packet context providing protocol and expected slave address.
 * @param rx_buf Received wire buffer.
 * @param data_len Buffer length excluding trailing PEC byte.
 * @param payload_off Output byte offset of block payload bytes.
 * @param payload_len Output block byte count.
 * @return true if rx_buf matches full-wire block framing for pkt->proto, false otherwise.
 */
static bool smbus_parse_full_block(const SYN_SMBUS_Packet *pkt, const uint8_t *rx_buf,
                                   size_t data_len, size_t *payload_off, uint8_t *payload_len)
{
    if (data_len < 3U || !smbus_match_addr_w(pkt->slave_addr, rx_buf[0])) {
        return false;
    }

    if (pkt->proto == SYN_SMBUS_PROTO_READ_BLOCK) {
        if (data_len >= 4U && rx_buf[2] == (uint8_t)(rx_buf[0] | 1U) &&
            rx_buf[3] <= SYN_SMBUS_BLOCK_MAX_LEN && (4U + (size_t)rx_buf[3]) == data_len) {
            *payload_off = 4U;
            *payload_len = rx_buf[3];
            return true;
        }
        return false;
    }

    if (pkt->proto == SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL) {
        return smbus_parse_bpc_block(pkt->slave_addr, rx_buf, data_len, payload_off, payload_len);
    }

    if (rx_buf[2] <= SYN_SMBUS_BLOCK_MAX_LEN && (3U + (size_t)rx_buf[2]) == data_len) {
        *payload_off = 3U;
        *payload_len = rx_buf[2];
        return true;
    }
    return false;
}

/**
 * @brief Locate variable-length SMBus block payload in full-wire or raw-response layout.
 * @param pkt SMBus packet context providing protocol and expected slave address.
 * @param rx_buf Received wire or response buffer.
 * @param data_len Buffer length excluding trailing PEC byte.
 * @param payload_off Output byte offset of block payload bytes.
 * @param payload_len Output block byte count.
 * @param is_full Set true if rx_buf contains full wire framing, false if count+payload only.
 * @return SYN_OK on valid block framing, SYN_INVALID_PARAM otherwise.
 */
static SYN_Status smbus_parse_block(const SYN_SMBUS_Packet *pkt, const uint8_t *rx_buf,
                                    size_t data_len, size_t *payload_off, uint8_t *payload_len,
                                    bool *is_full)
{
    if (smbus_parse_full_block(pkt, rx_buf, data_len, payload_off, payload_len)) {
        *is_full = true;
        return SYN_OK;
    }

    if (data_len >= 1U && rx_buf[0] <= SYN_SMBUS_BLOCK_MAX_LEN &&
        (1U + (size_t)rx_buf[0]) == data_len) {
        *payload_off = 1U;
        *payload_len = rx_buf[0];
        *is_full = false;
        return SYN_OK;
    }

    return SYN_INVALID_PARAM;
}

/**
 * @brief Verify trailing SMBus PEC byte on decoded packet when PEC checking is enabled.
 * @param pkt Decoded SMBus packet structure (pec and pec_valid updated).
 * @param rx_buf Received buffer including trailing PEC byte when enabled.
 * @param rx_len Total length of rx_buf in bytes.
 * @param data_len Length of rx_buf excluding PEC byte.
 * @param has_pec Whether PEC verification is enabled.
 * @param is_full Whether rx_buf includes full wire address/command framing.
 * @return SYN_OK if PEC is disabled or valid, SYN_ERROR on PEC mismatch.
 */
static SYN_Status smbus_verify_rx_pec(SYN_SMBUS_Packet *pkt, const uint8_t *rx_buf, size_t rx_len,
                                      size_t data_len, bool has_pec, bool is_full)
{
    if (!has_pec) {
        pkt->pec = 0U;
        pkt->pec_valid = true;
        return SYN_OK;
    }

    uint8_t calc_pec = is_full ? syn_smbus_calc_pec(0U, rx_buf, data_len)
                               : syn_smbus_calc_transaction_pec(pkt, rx_buf, data_len);
    pkt->pec = rx_buf[rx_len - 1U];
    pkt->pec_valid = (pkt->pec == calc_pec);
    return pkt->pec_valid ? SYN_OK : SYN_ERROR;
}

/**
 * @brief Dispatch SMBus payload framing parser by transaction protocol family.
 * @param pkt SMBus packet context providing protocol and expected slave address.
 * @param rx_buf Received wire or response buffer.
 * @param data_len Buffer length excluding trailing PEC byte.
 * @param has_pec Whether PEC byte is present in rx_buf.
 * @param payload_off Output byte offset of payload within rx_buf.
 * @param payload_len Output payload byte count.
 * @param is_full Set true if rx_buf contains full wire framing, false if payload-only.
 * @return SYN_OK on valid framing, SYN_INVALID_PARAM otherwise.
 */
static SYN_Status smbus_parse_rx_payload(const SYN_SMBUS_Packet *pkt, const uint8_t *rx_buf,
                                         size_t data_len, bool has_pec, size_t *payload_off,
                                         uint8_t *payload_len, bool *is_full)
{
    SYN_SMBUS_Protocol proto = pkt->proto;
    switch (proto) {
    case SYN_SMBUS_PROTO_WRITE_BYTE:
    case SYN_SMBUS_PROTO_RECEIVE_BYTE:
    case SYN_SMBUS_PROTO_READ_BYTE:
        return smbus_parse_u8(proto, data_len, has_pec, payload_off, payload_len, is_full);

    case SYN_SMBUS_PROTO_WRITE_WORD:
    case SYN_SMBUS_PROTO_READ_WORD:
    case SYN_SMBUS_PROTO_PROCESS_CALL:
        return smbus_parse_u16(proto, data_len, has_pec, payload_off, payload_len, is_full);

    case SYN_SMBUS_PROTO_WRITE_BLOCK:
    case SYN_SMBUS_PROTO_READ_BLOCK:
    case SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL:
        return smbus_parse_block(pkt, rx_buf, data_len, payload_off, payload_len, is_full);

    default:
        *is_full = true;
        *payload_len = (uint8_t)((data_len > 255U) ? 255U : data_len);
        return SYN_OK;
    }
}

SYN_Status syn_smbus_decode_packet(SYN_SMBUS_Packet *pkt, const uint8_t *rx_buf, size_t rx_len,
                                   SYN_SMBUS_Protocol proto, bool has_pec)
{
    if (pkt == NULL || rx_buf == NULL || (has_pec && rx_len < 1U)) {
        return SYN_INVALID_PARAM;
    }

    pkt->proto = proto;
    pkt->pec_enabled = has_pec;
    size_t data_len = has_pec ? (rx_len - 1U) : rx_len;
    size_t payload_off = 0U;
    uint8_t payload_len = 0U;
    bool is_full = false;

    SYN_Status st = smbus_parse_rx_payload(pkt, rx_buf, data_len, has_pec, &payload_off,
                                           &payload_len, &is_full);
    if (st != SYN_OK) {
        return st;
    }

    st = smbus_verify_rx_pec(pkt, rx_buf, rx_len, data_len, has_pec, is_full);
    if (st != SYN_OK) {
        return st;
    }

    pkt->length = payload_len;
    if (payload_len > 0U) {
        memcpy(pkt->data, &rx_buf[payload_off], payload_len);
    }

    return SYN_OK;
}

#endif /* SYN_USE_SMBUS */
