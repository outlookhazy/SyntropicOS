

# File syn\_coap.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_coap.c**](syn__coap_8c.md)

[Go to the documentation of this file](syn__coap_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_COAP) || SYN_USE_COAP

#include "syn_coap.h"
#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include <string.h>

size_t syn_coap_serialize(const SYN_CoapMsg *msg, const SYN_CoapOption *options, size_t option_count,
                          uint8_t *buf, size_t max_buf_len)
{
    SYN_ASSERT(msg != NULL);
    SYN_ASSERT(buf != NULL);

    if (max_buf_len < (size_t)(4 + msg->token_len)) {
        return 0;
    }

    /* Copy and sort options in ascending order of option numbers (CoAP spec requirement) */
    SYN_CoapOption sorted[16];
    size_t count = option_count > 16 ? 16 : option_count;
    if (count > 0) {
        memcpy(sorted, options, count * sizeof(SYN_CoapOption));
    }

    for (size_t i = 0; i < count; i++) {
        for (size_t j = i + 1; j < count; j++) {
            if (sorted[i].num > sorted[j].num) {
                SYN_CoapOption tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }

    /* Encode 4-byte header */
    buf[0] = (uint8_t)((COAP_VERSION << 6) | ((msg->type & 0x03) << 4) | (msg->token_len & 0x0F));
    buf[1] = msg->code;
    syn_poke_u16(msg->msg_id, buf, 2);

    size_t pos = 4;
    for (size_t i = 0; i < msg->token_len; i++) {
        buf[pos++] = msg->token[i];
    }

    /* Encode options */
    uint16_t prev_num = 0;
    for (size_t i = 0; i < count; i++) {
        uint16_t delta = sorted[i].num - prev_num;
        prev_num = sorted[i].num;

        uint8_t delta_val = 0;
        uint8_t delta_ext_len = 0;
        uint16_t delta_ext = 0;
        if (delta < 13) {
            delta_val = (uint8_t)delta;
        } else if (delta < 269) {
            delta_val = 13;
            delta_ext_len = 1;
            delta_ext = delta - 13;
        } else {
            delta_val = 14;
            delta_ext_len = 2;
            delta_ext = delta - 269;
        }

        uint8_t len_val = 0;
        uint8_t len_ext_len = 0;
        uint16_t len_ext = 0;
        size_t opt_len = sorted[i].len;
        if (opt_len < 13) {
            len_val = (uint8_t)opt_len;
        } else if (opt_len < 269) {
            len_val = 13;
            len_ext_len = 1;
            len_ext = (uint16_t)(opt_len - 13);
        } else {
            len_val = 14;
            len_ext_len = 2;
            len_ext = (uint16_t)(opt_len - 269);
        }

        size_t opt_header_len = (size_t)(1 + delta_ext_len + len_ext_len);
        if (pos + opt_header_len + opt_len > max_buf_len) {
            return 0;
        }

        buf[pos++] = (uint8_t)((delta_val << 4) | len_val);

        if (delta_ext_len == 1) {
            buf[pos++] = (uint8_t)delta_ext;
        } else if (delta_ext_len == 2) {
            buf[pos++] = (uint8_t)(delta_ext >> 8);
            buf[pos++] = (uint8_t)(delta_ext & 0xFF);
        }

        if (len_ext_len == 1) {
            buf[pos++] = (uint8_t)len_ext;
        } else if (len_ext_len == 2) {
            buf[pos++] = (uint8_t)(len_ext >> 8);
            buf[pos++] = (uint8_t)(len_ext & 0xFF);
        }

        memcpy(buf + pos, sorted[i].val, opt_len);
        pos += opt_len;
    }

    /* Encode payload */
    if (msg->payload_len > 0) {
        if (pos + 1 + msg->payload_len > max_buf_len) {
            return 0;
        }
        buf[pos++] = 0xFF; /* Payload marker */
        memcpy(buf + pos, msg->payload, msg->payload_len);
        pos += msg->payload_len;
    }

    return pos;
}

SYN_Status syn_coap_parse(SYN_CoapMsg *msg, SYN_CoapOption *options, size_t max_options,
                          size_t *option_count, const uint8_t *buf, size_t buf_len)
{
    SYN_ASSERT(msg != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(option_count != NULL);

    if (buf_len < 4) {
        return SYN_ERROR;
    }

    uint8_t ver = (buf[0] >> 6) & 0x03;
    if (ver != COAP_VERSION) {
        return SYN_ERROR;
    }
    msg->type = (buf[0] >> 4) & 0x03;
    msg->token_len = buf[0] & 0x0F;
    msg->code = buf[1];
    msg->msg_id = syn_peek_u16(buf, 2);

    if (msg->token_len > 8) {
        return SYN_ERROR;
    }
    if (buf_len < (size_t)(4 + msg->token_len)) {
        return SYN_ERROR;
    }

    memcpy(msg->token, buf + 4, msg->token_len);

    size_t pos = 4 + msg->token_len;
    size_t opt_idx = 0;
    uint16_t prev_num = 0;
    msg->payload = NULL;
    msg->payload_len = 0;

    while (pos < buf_len) {
        if (buf[pos] == 0xFF) {
            pos++;
            msg->payload = buf + pos;
            msg->payload_len = buf_len - pos;
            break;
        }

        uint8_t delta_val = (buf[pos] >> 4) & 0x0F;
        uint8_t len_val = buf[pos] & 0x0F;
        pos++;

        uint16_t delta = 0;
        if (delta_val < 13) {
            delta = delta_val;
        } else if (delta_val == 13) {
            if (pos >= buf_len) return SYN_ERROR;
            delta = 13 + buf[pos++];
        } else if (delta_val == 14) {
            if (pos + 1 >= buf_len) return SYN_ERROR;
            delta = (uint16_t)(269 + (((uint16_t)buf[pos] << 8) | buf[pos+1]));
            pos += 2;
        } else {
            return SYN_ERROR;
        }

        size_t len = 0;
        if (len_val < 13) {
            len = len_val;
        } else if (len_val == 13) {
            if (pos >= buf_len) return SYN_ERROR;
            len = 13 + buf[pos++];
        } else if (len_val == 14) {
            if (pos + 1 >= buf_len) return SYN_ERROR;
            len = (size_t)(269 + (((uint16_t)buf[pos] << 8) | buf[pos+1]));
            pos += 2;
        } else {
            return SYN_ERROR;
        }

        if (pos + len > buf_len) {
            return SYN_ERROR;
        }

        uint16_t num = prev_num + delta;
        prev_num = num;

        if (opt_idx < max_options) {
            options[opt_idx].num = num;
            options[opt_idx].val = buf + pos;
            options[opt_idx].len = len;
            opt_idx++;
        }
        pos += len;
    }
    *option_count = opt_idx;
    return SYN_OK;
}

void syn_coap_request_init(SYN_CoapRequest *r, const SYN_SockAddr *server_addr,
                           const SYN_CoapMsg *msg, uint32_t timeout_ms, uint8_t retries)
{
    SYN_ASSERT(r != NULL);
    memset(r, 0, sizeof(*r));
    r->server_addr = *server_addr;
    r->req_msg     = msg;
    /* Initialize backoff: factor 2 for binary exponential.
     * max_attempts = retries + 1 (original + retransmissions) */
    syn_backoff_init(&r->backoff, timeout_ms, timeout_ms << retries, 2, retries + 1);
}

SYN_PT_Status syn_coap_request_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_CoapRequest *r = (SYN_CoapRequest *)task->user_data;
    SYN_ASSERT(r != NULL);

    PT_BEGIN(pt);

    r->status = SYN_TIMEOUT;
    r->sock = syn_port_udp_open(0);
    if (r->sock == SYN_SOCKET_INVALID) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    /* Serialize once into struct-owned buffer (survives across yields) */
    r->tx_len = syn_coap_serialize(r->req_msg, r->req_options, r->req_option_count,
                                   r->tx_buf, sizeof(r->tx_buf));
    if (r->tx_len == 0) {
        syn_port_sock_close(r->sock);
        r->sock = SYN_SOCKET_INVALID;
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    r->start_ms = syn_port_get_tick_ms();

    syn_backoff_reset(&r->backoff);

    while (!syn_backoff_exhausted(&r->backoff)) {
        /* Send request */
        if (syn_port_udp_sendto(r->sock, r->tx_buf, r->tx_len, &r->server_addr) != (int)r->tx_len) {
            r->status = SYN_ERROR;
            break;
        }

        /* Calculate delay for THIS attempt (including jitter) */
        uint32_t delay = syn_backoff_next_ms(&r->backoff);
        r->start_ms = syn_port_get_tick_ms();

        /* Wait for response */
        while ((syn_port_get_tick_ms() - r->start_ms) < delay) {
            SYN_SockAddr from;
            int n = syn_port_udp_recvfrom(r->sock, r->resp_buf, sizeof(r->resp_buf), &from, 0);
            if (n > 0) {
                r->resp_len = (size_t)n;
                SYN_Status st = syn_coap_parse(&r->resp_msg, r->resp_options, 8,
                                               &r->resp_option_count, r->resp_buf, r->resp_len);
                if (st == SYN_OK &&
                    r->resp_msg.token_len == r->req_msg->token_len &&
                    memcmp(r->resp_msg.token, r->req_msg->token, r->resp_msg.token_len) == 0) {
                    r->status = SYN_OK;
                    break;
                }
            }
            PT_DEFER(pt, task);
        }

        if (r->status == SYN_OK) break;
    }

    if (r->status == SYN_BUSY) r->status = SYN_TIMEOUT;

    syn_port_sock_close(r->sock);
    r->sock = SYN_SOCKET_INVALID;

    PT_END(pt);
}


#endif /* SYN_USE_COAP */
```


