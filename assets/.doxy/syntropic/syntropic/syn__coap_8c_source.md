

# File syn\_coap.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_coap.c**](syn__coap_8c.md)

[Go to the documentation of this file](syn__coap_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_COAP) || SYN_USE_COAP

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include "syn_coap.h"

#include <string.h>

size_t syn_coap_encode_block_opt(const SYN_CoapBlock *block, uint8_t buf[3])
{
    if (block == NULL || buf == NULL || block->szx > 6) {
        return 0;
    }

    uint32_t val = (block->num << 4U) | ((block->more ? 1U : 0U) << 3U) | (block->szx & 0x07U);

    if (val <= 0xFFU) {
        buf[0] = (uint8_t)val;
        return 1;
    } else if (val <= 0xFFFFU) {
        buf[0] = (uint8_t)(val >> 8U);
        buf[1] = (uint8_t)(val & 0xFFU);
        return 2;
    } else if (val <= 0xFFFFFFU) {
        buf[0] = (uint8_t)(val >> 16U);
        buf[1] = (uint8_t)(val >> 8U);
        buf[2] = (uint8_t)(val & 0xFFU);
        return 3;
    }

    return 0;
}

bool syn_coap_decode_block_opt(const uint8_t *opt_val, size_t opt_len, SYN_CoapBlock *block)
{
    if (block == NULL) {
        return false;
    }
    if (opt_len > 3) {
        return false;
    }

    uint32_t val = 0;
    if (opt_len > 0 && opt_val == NULL) {
        return false;
    }

    if (opt_len == 1) {
        val = opt_val[0];
    } else if (opt_len == 2) {
        val = ((uint32_t)opt_val[0] << 8U) | opt_val[1];
    } else if (opt_len == 3) {
        val = ((uint32_t)opt_val[0] << 16U) | ((uint32_t)opt_val[1] << 8U) | opt_val[2];
    }

    block->szx = (uint8_t)(val & 0x07U);
    if (block->szx > 6) {
        return false;
    }
    block->more = ((val & 0x08U) != 0);
    block->num = val >> 4U;
    return true;
}

size_t syn_coap_serialize(const SYN_CoapMsg *msg, const SYN_CoapOption *options,
                          size_t option_count, uint8_t *buf, size_t max_buf_len)
{
    SYN_ASSERT(msg != NULL);
    SYN_ASSERT(buf != NULL);

    if (msg->token_len > 8 || max_buf_len < (size_t)(4 + msg->token_len)) {
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
            if (pos >= buf_len)
                return SYN_ERROR;
            delta = 13 + buf[pos++];
        } else if (delta_val == 14) {
            if (pos + 1 >= buf_len)
                return SYN_ERROR;
            delta = (uint16_t)(269 + (((uint16_t)buf[pos] << 8) | buf[pos + 1]));
            pos += 2;
        } else {
            return SYN_ERROR;
        }

        size_t len = 0;
        if (len_val < 13) {
            len = len_val;
        } else if (len_val == 13) {
            if (pos >= buf_len)
                return SYN_ERROR;
            len = 13 + buf[pos++];
        } else if (len_val == 14) {
            if (pos + 1 >= buf_len)
                return SYN_ERROR;
            len = (size_t)(269 + (((uint16_t)buf[pos] << 8) | buf[pos + 1]));
            pos += 2;
        } else {
            return SYN_ERROR;
        }

        if (pos + len > buf_len) {
            return SYN_ERROR;
        }

        uint16_t num = prev_num + delta;
        prev_num = num;

        if (options != NULL && opt_idx < max_options) {
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
    (void)memset(r, 0, sizeof(*r));
    r->server_addr = *server_addr;
    r->req_msg = msg;
    r->start_ms = 0;
    /* Initialize backoff: factor 2 for binary exponential.
     * max_attempts = retries + 1 (original + retransmissions) */
    syn_backoff_init(&r->backoff, timeout_ms, timeout_ms << retries, 2, retries + 1);
}

SYN_PT_Status syn_coap_request_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_CoapRequest *r = (SYN_CoapRequest *)task->user_data;
    SYN_ASSERT(r != NULL);
    uint32_t attempt_delay = 0;

    PT_BEGIN(pt);

    r->status = SYN_TIMEOUT;
    r->sock = syn_port_udp_open(0);
    if (r->sock == SYN_SOCKET_INVALID) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    /* Serialize once into struct-owned buffer (survives across yields) */
    r->tx_len = syn_coap_serialize(r->req_msg, r->req_options, r->req_option_count, r->tx_buf,
                                   sizeof(r->tx_buf));
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
        attempt_delay = syn_backoff_next_ms(&r->backoff);
        r->start_ms = syn_port_get_tick_ms();

        /* Wait for response */
        while ((syn_port_get_tick_ms() - r->start_ms) < attempt_delay) {
            SYN_SockAddr from;
            int n = syn_port_udp_recvfrom(r->sock, r->resp_buf, sizeof(r->resp_buf), &from, 0);
            if (n > 0) {
                r->resp_len = (size_t)n;
                SYN_Status st = syn_coap_parse(&r->resp_msg, r->resp_options, 8,
                                               &r->resp_option_count, r->resp_buf, r->resp_len);
                if (st == SYN_OK && r->resp_msg.token_len == r->req_msg->token_len &&
                    memcmp(r->resp_msg.token, r->req_msg->token, r->resp_msg.token_len) == 0) {
                    r->status = SYN_OK;
                    break;
                }
            }
            PT_WAIT_UNTIL(pt, r->sock == SYN_SOCKET_INVALID || syn_port_udp_readable(r->sock) ||
                                  (syn_port_get_tick_ms() - r->start_ms) >= attempt_delay);
        }

        if (r->status == SYN_OK)
            break;
    }

    /* LCOV_EXCL_START: Defensive COAP timeout status fallback */
    if (r->status == SYN_BUSY)
        r->status = SYN_TIMEOUT;
    /* LCOV_EXCL_STOP */

    syn_port_sock_close(r->sock);
    r->sock = SYN_SOCKET_INVALID;

    PT_END(pt);
}

/* ── CoAP over Generic Transport (SYN_Transport) ────────────────────────── */

void syn_coap_transport_request_init(SYN_CoapTransportRequest *r, SYN_Transport *transport,
                                     const SYN_CoapMsg *msg, uint32_t timeout_ms, uint8_t retries)
{
    if (r == NULL) {
        return;
    }
    (void)memset(r, 0, sizeof(*r));
    r->transport = transport;
    r->req_msg = msg;
    r->start_ms = 0;
    syn_backoff_init(&r->backoff, timeout_ms, timeout_ms << retries, 2, retries + 1);
}

SYN_PT_Status syn_coap_transport_request_task(SYN_PT *pt, SYN_Task *task)
{
    if (task == NULL || pt == NULL) {
        return PT_EXITED;
    }
    SYN_CoapTransportRequest *r = (SYN_CoapTransportRequest *)task->user_data;
    if (r == NULL) {
        return PT_EXITED;
    }

    uint32_t attempt_delay = 0;

    PT_BEGIN(pt);

    r->status = SYN_TIMEOUT;
    if (r->transport == NULL || r->req_msg == NULL) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    r->tx_len = syn_coap_serialize(r->req_msg, r->req_options, r->req_option_count, r->tx_buf,
                                   sizeof(r->tx_buf));
    if (r->tx_len == 0) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    r->start_ms = syn_port_get_tick_ms();
    syn_backoff_reset(&r->backoff);

    while (!syn_backoff_exhausted(&r->backoff)) {
        if (!syn_transport_send(r->transport, r->tx_buf, r->tx_len)) {
            r->status = SYN_ERROR;
            break;
        }

        attempt_delay = syn_backoff_next_ms(&r->backoff);
        r->start_ms = syn_port_get_tick_ms();

        while ((syn_port_get_tick_ms() - r->start_ms) < attempt_delay) {
            size_t rx_len = 0;
            if (syn_transport_recv(r->transport, r->resp_buf, sizeof(r->resp_buf), &rx_len) &&
                rx_len > 0) {
                r->resp_len = rx_len;
                SYN_Status st = syn_coap_parse(&r->resp_msg, r->resp_options, 8,
                                               &r->resp_option_count, r->resp_buf, r->resp_len);
                if (st == SYN_OK && r->resp_msg.token_len == r->req_msg->token_len &&
                    memcmp(r->resp_msg.token, r->req_msg->token, r->resp_msg.token_len) == 0) {
                    r->status = SYN_OK;
                    break;
                }
            }
            PT_YIELD(pt);
        }

        if (r->status == SYN_OK) {
            break;
        }
    }

    PT_END(pt);
}

SYN_Status syn_coap_transport_send_request(SYN_Transport *transport, const SYN_CoapMsg *req,
                                           const SYN_CoapOption *req_opts, size_t req_opt_cnt,
                                           SYN_CoapMsg *resp, SYN_CoapOption *resp_opts,
                                           size_t max_resp_opts, size_t *resp_opt_cnt,
                                           uint8_t *resp_buf, size_t resp_buf_sz)
{
    if (transport == NULL || req == NULL || resp == NULL || resp_buf == NULL || resp_buf_sz == 0) {
        return SYN_ERROR;
    }

    uint8_t tx_buf[256];
    size_t tx_len = syn_coap_serialize(req, req_opts, req_opt_cnt, tx_buf, sizeof(tx_buf));
    if (tx_len == 0) {
        return SYN_ERROR;
    }

    if (!syn_transport_send(transport, tx_buf, tx_len)) {
        return SYN_ERROR;
    }

    size_t rx_len = 0;
    if (!syn_transport_recv(transport, resp_buf, resp_buf_sz, &rx_len) || rx_len == 0) {
        return SYN_TIMEOUT;
    }

    size_t dummy_cnt = 0;
    size_t *opt_cnt_ptr = (resp_opt_cnt != NULL) ? resp_opt_cnt : &dummy_cnt;
    SYN_Status st = syn_coap_parse(resp, resp_opts, max_resp_opts, opt_cnt_ptr, resp_buf, rx_len);
    if (st != SYN_OK) {
        return st;
    }

    if (resp->token_len != req->token_len || memcmp(resp->token, req->token, req->token_len) != 0) {
        return SYN_ERROR;
    }

    return SYN_OK;
}

/* ── CoAP over DTLS 1.3 (coaps://) ───────────────────────────────────────── */

bool syn_coaps_client_init(SYN_CoapsClient *client, const SYN_DTLS_Config *config,
                           SYN_Transport *underlying_transport, uint8_t *rx_buf, size_t rx_buf_size,
                           uint8_t *tx_buf, size_t tx_buf_size)
{
    if (client == NULL || config == NULL || underlying_transport == NULL) {
        return false;
    }

    if (!syn_dtls_init(&client->dtls, config, underlying_transport, rx_buf, rx_buf_size, tx_buf,
                       tx_buf_size)) {
        return false;
    }

    syn_dtls_bind_transport(&client->dtls, &client->dtls_transport);
    return true;
}

bool syn_coaps_client_handshake(SYN_CoapsClient *client)
{
    if (client == NULL) {
        return false;
    }
    return syn_dtls_handshake(&client->dtls);
}

SYN_Status syn_coaps_client_send_request(SYN_CoapsClient *client, const SYN_CoapMsg *req,
                                         const SYN_CoapOption *req_opts, size_t req_opt_cnt,
                                         SYN_CoapMsg *resp, SYN_CoapOption *resp_opts,
                                         size_t max_resp_opts, size_t *resp_opt_cnt,
                                         uint8_t *resp_buf, size_t resp_buf_sz)
{
    if (client == NULL) {
        return SYN_ERROR;
    }

    if (client->dtls.state != SYN_DTLS_STATE_ESTABLISHED) {
        if (!syn_coaps_client_handshake(client)) {
            return SYN_ERROR;
        }
    }

    return syn_coap_transport_send_request(&client->dtls_transport, req, req_opts, req_opt_cnt,
                                           resp, resp_opts, max_resp_opts, resp_opt_cnt, resp_buf,
                                           resp_buf_sz);
}

#endif /* SYN_USE_COAP */
```


