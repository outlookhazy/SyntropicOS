

# File syn\_coap.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_coap.c**](syn__coap_8c.md)

[Go to the documentation of this file](syn__coap_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_COAP) || SYN_USE_COAP


#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include "syn_coap.h"

#include <string.h>

size_t syn_coap_encode_block_opt(const SYN_CoapBlock *block, uint8_t buf[3])
{
    if (block == NULL || buf == NULL || block->szx > 6U || block->num > 0xFFFFFU) {
        return 0U;
    }

    uint32_t val = (block->num << 4U) | ((block->more ? 1U : 0U) << 3U) | (block->szx & 0x07U);

    if (val <= 0xFFU) {
        buf[0] = (uint8_t)val;
        return 1U;
    }
    if (val <= 0xFFFFU) {
        buf[0] = (uint8_t)(val >> 8U);
        buf[1] = (uint8_t)(val & 0xFFU);
        return 2U;
    }
    buf[0] = (uint8_t)(val >> 16U);
    buf[1] = (uint8_t)(val >> 8U);
    buf[2] = (uint8_t)(val & 0xFFU);
    return 3U;
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

static void coap_sort_options(SYN_CoapOption *sorted, const SYN_CoapOption *options, size_t count)
{
    if (count > 0U) {
        memcpy(sorted, options, count * sizeof(SYN_CoapOption));
    }
    for (size_t i = 1U; i < count; i++) {
        SYN_CoapOption tmp = sorted[i];
        size_t j = i;
        while (j > 0U && sorted[j - 1U].num > tmp.num) {
            sorted[j] = sorted[j - 1U];
            j--;
        }
        sorted[j] = tmp;
    }
}

static void coap_encode_nibble(size_t val, uint8_t *nibble, uint8_t *ext_len, uint16_t *ext_val)
{
    if (val < 13U) {
        *nibble = (uint8_t)val;
        *ext_len = 0U;
        *ext_val = 0U;
    } else if (val < 269U) {
        *nibble = 13U;
        *ext_len = 1U;
        *ext_val = (uint16_t)(val - 13U);
    } else {
        *nibble = 14U;
        *ext_len = 2U;
        *ext_val = (uint16_t)(val - 269U);
    }
}

static void coap_write_ext(uint8_t *buf, size_t *pos, uint8_t ext_len, uint16_t ext_val)
{
    if (ext_len == 1U) {
        buf[(*pos)++] = (uint8_t)ext_val;
    } else if (ext_len == 2U) {
        buf[(*pos)++] = (uint8_t)(ext_val >> 8U);
        buf[(*pos)++] = (uint8_t)(ext_val & 0xFFU);
    }
}

static bool coap_serialize_args_ok(const SYN_CoapMsg *msg, const SYN_CoapOption *options,
                                   size_t option_count, const uint8_t *buf, size_t max_buf_len)
{
    if (msg == NULL || buf == NULL || option_count > 16U) {
        return false;
    }
    if ((option_count > 0U && options == NULL) || (msg->payload_len > 0U && msg->payload == NULL)) {
        return false;
    }
    return (msg->token_len <= 8U) && (max_buf_len >= (size_t)(4U + msg->token_len));
}

size_t syn_coap_serialize(const SYN_CoapMsg *msg, const SYN_CoapOption *options,
                          size_t option_count, uint8_t *buf, size_t max_buf_len)
{
    if (!coap_serialize_args_ok(msg, options, option_count, buf, max_buf_len)) {
        return 0U;
    }

    SYN_CoapOption sorted[16];
    coap_sort_options(sorted, options, option_count);

    buf[0] =
        (uint8_t)((COAP_VERSION << 6U) | ((msg->type & 0x03U) << 4U) | (msg->token_len & 0x0FU));
    buf[1] = msg->code;
    syn_poke_u16(msg->msg_id, buf, 2U);

    size_t pos = 4U;
    for (size_t i = 0U; i < msg->token_len; i++) {
        buf[pos++] = msg->token[i];
    }

    uint16_t prev_num = 0U;
    for (size_t i = 0U; i < option_count; i++) {
        size_t opt_len = sorted[i].len;
        if (
#if SIZE_MAX > 65804U
            opt_len > 65804U ||
#endif
            (opt_len > 0U && sorted[i].val == NULL)) {
            return 0U;
        }

        uint16_t delta = (uint16_t)(sorted[i].num - prev_num);
        prev_num = sorted[i].num;

        uint8_t delta_val = 0U;
        uint8_t delta_ext_len = 0U;
        uint16_t delta_ext = 0U;
        coap_encode_nibble(delta, &delta_val, &delta_ext_len, &delta_ext);

        uint8_t len_val = 0U;
        uint8_t len_ext_len = 0U;
        uint16_t len_ext = 0U;
        coap_encode_nibble(opt_len, &len_val, &len_ext_len, &len_ext);

        size_t opt_header_len = (size_t)(1U + delta_ext_len + len_ext_len);
        if (opt_header_len > max_buf_len - pos || opt_len > max_buf_len - pos - opt_header_len) {
            return 0U;
        }

        buf[pos++] = (uint8_t)((delta_val << 4U) | len_val);
        coap_write_ext(buf, &pos, delta_ext_len, delta_ext);
        coap_write_ext(buf, &pos, len_ext_len, len_ext);

        if (opt_len > 0U) {
            memcpy(buf + pos, sorted[i].val, opt_len);
            pos += opt_len;
        }
    }

    if (msg->payload_len > 0U) {
        if (msg->payload_len >= max_buf_len - pos) {
            return 0U;
        }
        buf[pos++] = 0xFFU;
        memcpy(buf + pos, msg->payload, msg->payload_len);
        pos += msg->payload_len;
    }

    return pos;
}

static bool coap_decode_nibble(uint8_t nibble, const uint8_t *buf, size_t buf_len, size_t *pos,
                               size_t *out)
{
    if (nibble < 13U) {
        *out = nibble;
        return true;
    }
    if (nibble == 13U) {
        if (buf_len - *pos < 1U) {
            return false;
        }
        *out = 13U + (size_t)buf[(*pos)++];
        return true;
    }
    if (nibble == 14U) {
        if (buf_len - *pos < 2U) {
            return false;
        }
        *out = 269U + (((size_t)buf[*pos] << 8U) | (size_t)buf[*pos + 1U]);
        *pos += 2U;
        return true;
    }
    return false;
}

static bool coap_parse_opt_hdr(const uint8_t *buf, size_t buf_len, size_t *pos, uint16_t *prev_num,
                               size_t *len_out)
{
    uint8_t hdr = buf[(*pos)++];
    size_t delta = 0U;
    size_t len = 0U;
    if (!coap_decode_nibble((hdr >> 4U) & 0x0FU, buf, buf_len, pos, &delta) ||
        delta > (size_t)(UINT16_MAX - *prev_num) ||
        !coap_decode_nibble(hdr & 0x0FU, buf, buf_len, pos, &len) || len > buf_len - *pos) {
        return false;
    }
    *prev_num = (uint16_t)(*prev_num + (uint16_t)delta);
    *len_out = len;
    return true;
}

static bool coap_hdr_valid(const SYN_CoapMsg *msg, uint8_t ver, size_t buf_len)
{
    if ((ver != COAP_VERSION) || (msg->token_len > 8U) ||
        (buf_len < (size_t)(4U + msg->token_len))) {
        return false;
    }
    if ((msg->code == 0U) && ((msg->token_len != 0U) || (buf_len > 4U))) {
        return false;
    }
    uint8_t cls = (msg->code >> 5U) & 0x07U;
    return (cls != 1U) && (cls != 3U) && (cls < 6U);
}

SYN_Status syn_coap_parse(SYN_CoapMsg *msg, SYN_CoapOption *options, size_t max_options,
                          size_t *option_count, const uint8_t *buf, size_t buf_len)
{
    SYN_ASSERT(msg != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(option_count != NULL);

    if (buf_len < 4U) {
        return SYN_ERROR;
    }

    uint8_t ver = (buf[0] >> 6U) & 0x03U;
    msg->type = (buf[0] >> 4U) & 0x03U;
    msg->token_len = buf[0] & 0x0FU;
    msg->code = buf[1];
    msg->msg_id = syn_peek_u16(buf, 2U);

    if (!coap_hdr_valid(msg, ver, buf_len)) {
        return SYN_ERROR;
    }

    memcpy(msg->token, buf + 4U, msg->token_len);

    size_t pos = 4U + msg->token_len;
    size_t opt_idx = 0U;
    uint16_t prev_num = 0U;
    msg->payload = NULL;
    msg->payload_len = 0U;

    while (pos < buf_len) {
        if (buf[pos] == 0xFFU) {
            pos++;
            if (pos >= buf_len) {
                return SYN_ERROR;
            }
            msg->payload = buf + pos;
            msg->payload_len = buf_len - pos;
            break;
        }

        size_t len = 0U;
        if (!coap_parse_opt_hdr(buf, buf_len, &pos, &prev_num, &len)) {
            return SYN_ERROR;
        }

        if ((options != NULL) && (opt_idx < max_options)) {
            options[opt_idx].num = prev_num;
            options[opt_idx].val = buf + pos;
            options[opt_idx].len = len;
            opt_idx++;
        } else if ((prev_num & 1U) != 0U) {
            return SYN_ERROR;
        } else {
            /* Dropped elective option is ignored per RFC 7252 Section 5.4.1 */
        }
        pos += len;
    }
    *option_count = opt_idx;
    return SYN_OK;
}

static void coap_init_backoff(SYN_Backoff *b, const SYN_CoapMsg *msg, uint32_t timeout_ms,
                              uint8_t retries)
{
    uint8_t eff = ((msg != NULL) && (msg->type == COAP_TYPE_NON)) ? 0U : retries;
    uint8_t shift = (eff > 16U) ? 16U : eff;
    uint64_t scaled = (uint64_t)timeout_ms << shift;
    uint32_t max_ms = (scaled > 0x7FFFFFFFULL) ? 0x7FFFFFFFU : (uint32_t)scaled;
    uint8_t max_attempts = (eff == UINT8_MAX) ? UINT8_MAX : (uint8_t)(eff + 1U);
    syn_backoff_init(b, timeout_ms, max_ms, 2U, max_attempts);
}

static bool coap_pt_args_valid(const SYN_PT *pt, const SYN_Task *task)
{
    return (pt != NULL) && (task != NULL) && (task->user_data != NULL);
}

static void coap_close_sock(SYN_CoapRequest *r)
{
    if (r->sock != SYN_SOCKET_INVALID) {
        syn_port_sock_close(r->sock);
        r->sock = SYN_SOCKET_INVALID;
    }
}

static bool coap_response_is_rst(const SYN_CoapMsg *req, const SYN_CoapMsg *resp)
{
    return (resp->type == COAP_TYPE_RST) && (resp->code == 0U) && (resp->msg_id == req->msg_id);
}

static bool coap_response_matches(const SYN_CoapMsg *req, const SYN_CoapMsg *resp)
{
    if (resp->type == COAP_TYPE_RST || resp->code < 64U) {
        return false;
    }
    return (resp->token_len == req->token_len) &&
           (memcmp(resp->token, req->token, resp->token_len) == 0);
}

static bool coap_udp_wait_done(const SYN_CoapRequest *r)
{
    return (r->sock == SYN_SOCKET_INVALID) || syn_port_udp_readable(r->sock) ||
           ((syn_port_get_tick_ms() - r->start_ms) >= r->attempt_delay);
}

static bool coap_tr_wait_done(const SYN_CoapTransportRequest *r)
{
    return (r->transport == NULL) || syn_transport_has_data(r->transport) ||
           ((syn_port_get_tick_ms() - r->start_ms) >= r->attempt_delay);
}

void syn_coap_request_init(SYN_CoapRequest *r, const SYN_SockAddr *server_addr,
                           const SYN_CoapMsg *msg, uint32_t timeout_ms, uint8_t retries)
{
    if (r == NULL) {
        return;
    }
    (void)memset(r, 0, sizeof(*r));
    r->sock = SYN_SOCKET_INVALID;
    if (server_addr == NULL) {
        r->status = SYN_ERROR;
        return;
    }
    r->server_addr = *server_addr;
    r->req_msg = msg;
    r->start_ms = 0U;
    coap_init_backoff(&r->backoff, msg, timeout_ms, retries);
}

static bool coap_udp_consume_rx(SYN_CoapRequest *r, size_t n)
{
    r->resp_len = n;
    SYN_Status st =
        syn_coap_parse(&r->resp_msg, r->resp_options, 8U, &r->resp_option_count, r->resp_buf, n);
    if (st != SYN_OK) {
        return false;
    }
    if (coap_response_is_rst(r->req_msg, &r->resp_msg)) {
        r->status = SYN_ERROR;
        return true;
    }
    if (coap_response_matches(r->req_msg, &r->resp_msg)) {
        r->status = SYN_OK;
        return true;
    }
    return false;
}

SYN_PT_Status syn_coap_request_task(SYN_PT *pt, SYN_Task *task)
{
    if (!coap_pt_args_valid(pt, task)) {
        return PT_EXITED;
    }
    SYN_CoapRequest *r = (SYN_CoapRequest *)task->user_data;

    PT_BEGIN(pt);

    coap_close_sock(r);
    if (r->req_msg == NULL) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    r->status = SYN_TIMEOUT;
    r->sock = syn_port_udp_open(0);
    if (r->sock == SYN_SOCKET_INVALID) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    /* Serialize once into struct-owned buffer (survives across yields) */
    r->tx_len = syn_coap_serialize(r->req_msg, r->req_options, r->req_option_count, r->tx_buf,
                                   sizeof(r->tx_buf));
    if (r->tx_len == 0U) {
        coap_close_sock(r);
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    r->start_ms = syn_port_get_tick_ms();
    syn_backoff_reset(&r->backoff);

    while (!syn_backoff_exhausted(&r->backoff)) {
        if (syn_port_udp_sendto(r->sock, r->tx_buf, r->tx_len, &r->server_addr) != (int)r->tx_len) {
            r->status = SYN_ERROR;
            break;
        }

        r->attempt_delay = syn_backoff_next_ms(&r->backoff);
        r->start_ms = syn_port_get_tick_ms();

        while ((syn_port_get_tick_ms() - r->start_ms) < r->attempt_delay) {
            SYN_SockAddr from;
            int n = syn_port_udp_recvfrom(r->sock, r->resp_buf, sizeof(r->resp_buf), &from, 0);
            if (n > 0) {
                if (coap_udp_consume_rx(r, (size_t)n)) {
                    break;
                }
                PT_CONTINUE_IF_NEEDED(pt, task);
                continue;
            }
            if (syn_port_udp_readable(r->sock)) {
                PT_PASS(pt);
                continue;
            }
            PT_WAIT_UNTIL(pt, coap_udp_wait_done(r));
        }

        if (r->status != SYN_TIMEOUT) {
            break;
        }
    }

    coap_close_sock(r);

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
    r->start_ms = 0U;
    coap_init_backoff(&r->backoff, msg, timeout_ms, retries);
}

static bool coap_tr_poll_rx(SYN_CoapTransportRequest *r, bool *done_out)
{
    *done_out = false;
    size_t rx_len = 0U;
    if (!syn_transport_recv(r->transport, r->resp_buf, sizeof(r->resp_buf), &rx_len) ||
        rx_len == 0U) {
        return false;
    }
    r->resp_len = rx_len;
    SYN_Status st = syn_coap_parse(&r->resp_msg, r->resp_options, 8U, &r->resp_option_count,
                                   r->resp_buf, r->resp_len);
    if (st != SYN_OK) {
        return true;
    }
    if (coap_response_is_rst(r->req_msg, &r->resp_msg)) {
        r->status = SYN_ERROR;
        *done_out = true;
    } else if (coap_response_matches(r->req_msg, &r->resp_msg)) {
        r->status = SYN_OK;
        *done_out = true;
    } else {
        /* Unmatched datagram ignored */
    }
    return true;
}

SYN_PT_Status syn_coap_transport_request_task(SYN_PT *pt, SYN_Task *task)
{
    if (!coap_pt_args_valid(pt, task)) {
        return PT_EXITED;
    }
    SYN_CoapTransportRequest *r = (SYN_CoapTransportRequest *)task->user_data;

    PT_BEGIN(pt);

    r->status = SYN_TIMEOUT;
    if (r->transport == NULL || r->req_msg == NULL) {
        r->status = SYN_ERROR;
        PT_EXIT(pt);
    }

    r->tx_len = syn_coap_serialize(r->req_msg, r->req_options, r->req_option_count, r->tx_buf,
                                   sizeof(r->tx_buf));
    if (r->tx_len == 0U) {
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

        r->attempt_delay = syn_backoff_next_ms(&r->backoff);
        r->start_ms = syn_port_get_tick_ms();

        while ((syn_port_get_tick_ms() - r->start_ms) < r->attempt_delay) {
            bool done = false;
            if (coap_tr_poll_rx(r, &done)) {
                if (done) {
                    break;
                }
                PT_CONTINUE_IF_NEEDED(pt, task);
                continue;
            }
            PT_PASS(pt);
            if (r->transport->has_data != NULL) {
                PT_WAIT_UNTIL(pt, coap_tr_wait_done(r));
            }
        }

        if (r->status != SYN_TIMEOUT) {
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

    if (!coap_response_matches(req, resp)) {
        return SYN_ERROR;
    }

    return SYN_OK;
}

/* ── CoAP over DTLS 1.3 (coaps://) ───────────────────────────────────────── */

#if SYN_DTLS_ENABLED

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
        if (!syn_coaps_client_handshake(client) ||
            client->dtls.state != SYN_DTLS_STATE_ESTABLISHED) {
            return SYN_ERROR;
        }
    }

    return syn_coap_transport_send_request(&client->dtls_transport, req, req_opts, req_opt_cnt,
                                           resp, resp_opts, max_resp_opts, resp_opt_cnt, resp_buf,
                                           resp_buf_sz);
}

#endif /* SYN_DTLS_ENABLED */

#endif /* SYN_USE_COAP */
```


