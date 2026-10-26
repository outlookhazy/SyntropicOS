

# File syn\_smbus.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_smbus.c**](syn__smbus_8c.md)

[Go to the documentation of this file](syn__smbus_8c.md)


```C++

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
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ 0x07);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

SYN_Status syn_smbus_encode_packet(const SYN_SMBUS_Packet *pkt, uint8_t *tx_buf, size_t buf_size,
                                   size_t *out_len)
{
    if (pkt == NULL || tx_buf == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    size_t idx = 0;
    uint8_t addr_rw_write = (uint8_t)((pkt->slave_addr << 1) | 0x00);
    uint8_t addr_rw_read = (uint8_t)((pkt->slave_addr << 1) | 0x01);

    switch (pkt->proto) {
    case SYN_SMBUS_PROTO_QUICK_READ:
    case SYN_SMBUS_PROTO_QUICK_WRITE:
        if (buf_size < 1)
            return SYN_ERROR;
        tx_buf[idx++] = (pkt->proto == SYN_SMBUS_PROTO_QUICK_READ) ? addr_rw_read : addr_rw_write;
        break;

    case SYN_SMBUS_PROTO_SEND_BYTE:
        if (buf_size < 2 + (pkt->pec_enabled ? 1 : 0))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_write;
        tx_buf[idx++] = pkt->command;
        break;

    case SYN_SMBUS_PROTO_RECEIVE_BYTE:
        if (buf_size < 1 + (pkt->pec_enabled ? 1 : 0))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_read;
        break;

    case SYN_SMBUS_PROTO_WRITE_BYTE:
        if (buf_size < 3 + (pkt->pec_enabled ? 1 : 0))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_write;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = (pkt->length > 0) ? pkt->data[0] : 0;
        break;

    case SYN_SMBUS_PROTO_READ_BYTE:
        if (buf_size < 2 + (pkt->pec_enabled ? 1 : 0))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_write;
        tx_buf[idx++] = pkt->command;
        break;

    case SYN_SMBUS_PROTO_WRITE_WORD:
        if (buf_size < 4 + (pkt->pec_enabled ? 1 : 0))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_write;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = (pkt->length >= 1) ? pkt->data[0] : 0;
        tx_buf[idx++] = (pkt->length >= 2) ? pkt->data[1] : 0;
        break;

    case SYN_SMBUS_PROTO_READ_WORD:
        if (buf_size < 2 + (pkt->pec_enabled ? 1 : 0))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_write;
        tx_buf[idx++] = pkt->command;
        break;

    case SYN_SMBUS_PROTO_WRITE_BLOCK: {
        uint8_t payload_len =
            (pkt->length > SYN_SMBUS_BLOCK_MAX_LEN) ? SYN_SMBUS_BLOCK_MAX_LEN : pkt->length;
        if (buf_size < (size_t)(3 + payload_len + (pkt->pec_enabled ? 1 : 0)))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_write;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = payload_len;
        if (payload_len > 0) {
            memcpy(&tx_buf[idx], pkt->data, payload_len);
            idx += payload_len;
        }
        break;
    }

    case SYN_SMBUS_PROTO_READ_BLOCK:
        if (buf_size < 2 + (pkt->pec_enabled ? 1 : 0))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_write;
        tx_buf[idx++] = pkt->command;
        break;

    case SYN_SMBUS_PROTO_PROCESS_CALL:
        if (buf_size < 4 + (pkt->pec_enabled ? 1 : 0))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_write;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = (pkt->length >= 1) ? pkt->data[0] : 0;
        tx_buf[idx++] = (pkt->length >= 2) ? pkt->data[1] : 0;
        break;

    case SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL: {
        uint8_t payload_len =
            (pkt->length > SYN_SMBUS_BLOCK_MAX_LEN) ? SYN_SMBUS_BLOCK_MAX_LEN : pkt->length;
        if (buf_size < (size_t)(3 + payload_len + (pkt->pec_enabled ? 1 : 0)))
            return SYN_ERROR;
        tx_buf[idx++] = addr_rw_write;
        tx_buf[idx++] = pkt->command;
        tx_buf[idx++] = payload_len;
        if (payload_len > 0) {
            memcpy(&tx_buf[idx], pkt->data, payload_len);
            idx += payload_len;
        }
        break;
    }

    default:
        return SYN_INVALID_PARAM;
    }

    if (pkt->pec_enabled) {
        tx_buf[idx] = syn_smbus_calc_pec(0, tx_buf, idx);
        idx++;
    }

    *out_len = idx;
    return SYN_OK;
}

SYN_Status syn_smbus_decode_packet(SYN_SMBUS_Packet *pkt, const uint8_t *rx_buf, size_t rx_len,
                                   SYN_SMBUS_Protocol proto, bool has_pec)
{
    if (pkt == NULL || rx_buf == NULL) {
        return SYN_INVALID_PARAM;
    }

    size_t min_len = has_pec ? 1 : 0;
    if (rx_len < min_len) {
        return SYN_INVALID_PARAM;
    }

    pkt->proto = proto;
    pkt->pec_enabled = has_pec;

    size_t data_len = has_pec ? (rx_len - 1) : rx_len;

    if (has_pec) {
        uint8_t calc_pec = syn_smbus_calc_pec(0, rx_buf, data_len);
        pkt->pec = rx_buf[rx_len - 1];
        pkt->pec_valid = (pkt->pec == calc_pec);
        if (!pkt->pec_valid) {
            return SYN_ERROR;
        }
    } else {
        pkt->pec = 0;
        pkt->pec_valid = true;
    }

    switch (proto) {
    case SYN_SMBUS_PROTO_WRITE_BYTE:
        if (data_len >= 3) {
            pkt->length = 1;
            pkt->data[0] = rx_buf[2];
        } else if (data_len >= 1) {
            pkt->length = 1;
            pkt->data[0] = rx_buf[0];
        } else {
            return SYN_INVALID_PARAM;
        }
        break;

    case SYN_SMBUS_PROTO_WRITE_WORD:
        if (data_len >= 4) {
            pkt->length = 2;
            pkt->data[0] = rx_buf[2];
            pkt->data[1] = rx_buf[3];
        } else if (data_len >= 2) {
            pkt->length = 2;
            pkt->data[0] = rx_buf[0];
            pkt->data[1] = rx_buf[1];
        } else {
            return SYN_INVALID_PARAM;
        }
        break;

    case SYN_SMBUS_PROTO_RECEIVE_BYTE:
    case SYN_SMBUS_PROTO_READ_BYTE:
        if (data_len >= 3) {
            pkt->length = 1;
            pkt->data[0] = rx_buf[2];
        } else if (data_len >= 1) {
            pkt->length = 1;
            pkt->data[0] = rx_buf[0];
        } else {
            return SYN_INVALID_PARAM;
        }
        break;

    case SYN_SMBUS_PROTO_READ_WORD:
    case SYN_SMBUS_PROTO_PROCESS_CALL:
        if (data_len >= 4) {
            pkt->length = 2;
            pkt->data[0] = rx_buf[2];
            pkt->data[1] = rx_buf[3];
        } else if (data_len >= 2) {
            pkt->length = 2;
            pkt->data[0] = rx_buf[0];
            pkt->data[1] = rx_buf[1];
        } else {
            return SYN_INVALID_PARAM;
        }
        break;

    case SYN_SMBUS_PROTO_WRITE_BLOCK:
    case SYN_SMBUS_PROTO_READ_BLOCK:
    case SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL: {
        size_t header_len = 0;
        if (data_len >= 3 && (size_t)(3 + rx_buf[2]) <= data_len) {
            header_len = 2;
        } else if (data_len >= 1 && (size_t)(1 + rx_buf[0]) <= data_len) {
            header_len = 0;
        } else {
            return SYN_INVALID_PARAM;
        }
        uint8_t count = rx_buf[header_len];
        pkt->length = count;
        if (count > 0) {
            size_t copy_len = (size_t)count;
            memcpy(pkt->data, &rx_buf[header_len + 1], copy_len);
        }
        break;
    }

    default:
        pkt->length =
            (uint8_t)(data_len > SYN_SMBUS_BUF_MAX_LEN ? SYN_SMBUS_BUF_MAX_LEN : data_len);
        if (pkt->length > 0) {
            memcpy(pkt->data, rx_buf, pkt->length);
        }
        break;
    }

    return SYN_OK;
}

#endif /* SYN_USE_SMBUS */
```


