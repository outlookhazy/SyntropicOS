

# File syn\_websocket.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_websocket.c**](syn__websocket_8c.md)

[Go to the documentation of this file](syn__websocket_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_WEBSOCKET) || SYN_USE_WEBSOCKET


#include "../sched/syn_sched.h"
#include "../util/syn_assert.h"
#include "../util/syn_pack.h"
#include "syn_websocket.h"

#include <stdio.h>
#include <string.h>

/* ── SHA-1 ──────────────────────────────────────────────────────────────── */

typedef struct {
    uint32_t state[5];  
    uint32_t count[2];  
    uint8_t buffer[64]; 
} SYN_SHA1_Ctx;

#define SHA1_ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

static void sha1_init(SYN_SHA1_Ctx *ctx)
{
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
    ctx->count[0] = ctx->count[1] = 0;
}

static void sha1_transform(uint32_t state[5], const uint8_t buffer[64])
{
    uint32_t w[80];
    for (int i = 0; i < 16; i++) {
        w[i] = syn_peek_u32(buffer, i * 4);
    }
    for (int i = 16; i < 80; i++) {
        w[i] = SHA1_ROL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];

    for (int i = 0; i < 80; i++) {
        uint32_t f, k;
        if (i < 20) {
            f = (b & c) | (~b & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }
        uint32_t temp = SHA1_ROL(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = SHA1_ROL(b, 30);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

static void sha1_update(SYN_SHA1_Ctx *ctx, const uint8_t *data, uint32_t len)
{
    uint32_t j = (ctx->count[0] >> 3U) & 63U;
    uint64_t total_bits =
        (((uint64_t)ctx->count[1] << 32U) | (uint64_t)ctx->count[0]) + ((uint64_t)len << 3U);
    ctx->count[0] = (uint32_t)total_bits;
    ctx->count[1] = (uint32_t)(total_bits >> 32U);

    for (uint32_t i = 0U; i < len; i++) {
        ctx->buffer[j++] = data[i];
        if (j == 64U) {
            sha1_transform(ctx->state, ctx->buffer);
            j = 0U;
        }
    }
}

static void sha1_final(SYN_SHA1_Ctx *ctx, uint8_t digest[20])
{
    uint8_t finalcount[8];
    for (int i = 0; i < 8; i++) {
        finalcount[i] = (uint8_t)((ctx->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
    }
    uint8_t c = 0200;
    sha1_update(ctx, &c, 1);
    while ((ctx->count[0] >> 3 & 63) != 56) {
        uint8_t z = 0;
        sha1_update(ctx, &z, 1);
    }
    sha1_update(ctx, finalcount, 8);
    for (int i = 0; i < 20; i++) {
        digest[i] = (uint8_t)((ctx->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
}

/* ── Websocket Upgrading ────────────────────────────────────────────────── */

#include "../util/syn_base64.h"
#include "../util/syn_fmt.h"

static inline bool prefix_icase(const char *str, const char *prefix)
{
    return syn_str_prefix_icase(str, prefix);
}

static const char *ws_find_key_header(const char *headers)
{
    const char *cur = headers;
    while (cur != NULL && *cur != '\0' && *cur != '\r' && *cur != '\n') {
        if (prefix_icase(cur, "sec-websocket-key:")) {
            const char *key_hdr = cur + 18;
            while ((*key_hdr == ' ') || (*key_hdr == '\t')) {
                key_hdr++;
            }
            return key_hdr;
        }
        cur = strchr(cur, '\n');
        if (cur == NULL) {
            break;
        }
        cur++;
    }
    return NULL;
}

static bool ws_copy_trimmed_key(const char *key_hdr, char *key_out, size_t key_sz)
{
    size_t raw_len = 0U;
    while ((key_hdr[raw_len] != '\0') && (key_hdr[raw_len] != '\r') && (key_hdr[raw_len] != '\n')) {
        raw_len++;
    }
    while ((raw_len > 0U) && ((key_hdr[raw_len - 1U] == ' ') || (key_hdr[raw_len - 1U] == '\t'))) {
        raw_len--;
    }
    if (raw_len == 0U) {
        return false;
    }
    if (raw_len >= key_sz) {
        raw_len = key_sz - 1U;
    }
    memcpy(key_out, key_hdr, raw_len);
    key_out[raw_len] = '\0';
    return true;
}

SYN_Status syn_websocket_upgrade(
    const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, SYN_WebsocketSession *ws,
    void (*on_message)(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx), void *ctx)
{
    SYN_ASSERT(req != NULL);
    SYN_ASSERT(resp != NULL);
    SYN_ASSERT(ws != NULL);

    char key[64];
    const char *key_hdr = ws_find_key_header(req->headers);
    if ((key_hdr == NULL) || !ws_copy_trimmed_key(key_hdr, key, sizeof(key))) {
        return SYN_ERROR;
    }
    if (req->body_buffered_len > sizeof(ws->pending_rx) ||
        (req->body_buffered_len > 0U &&
         (resp->buf == NULL ||
          req->body_buffered_offset + req->body_buffered_len > resp->buf_size))) {
        return SYN_ERROR;
    }

    char accept_buf[128];
    (void)snprintf(accept_buf, sizeof(accept_buf), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", key);

    SYN_SHA1_Ctx sha;
    uint8_t digest[20];
    sha1_init(&sha);
    sha1_update(&sha, (const uint8_t *)accept_buf, (uint32_t)strlen(accept_buf));
    sha1_final(&sha, digest);

    char accept_key[32];
    (void)syn_base64_encode(digest, 20U, accept_key, sizeof(accept_key), NULL);

    char resp_buf[160];
    int rlen = snprintf(resp_buf, sizeof(resp_buf),
                        "HTTP/1.1 101 Switching Protocols\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Accept: %s\r\n\r\n",
                        accept_key);
    if (syn_port_sock_send_all(resp->sock, resp_buf, (size_t)rlen) != rlen) {
        return SYN_ERROR;
    }

    memset(ws, 0, sizeof(*ws));
    ws->sock = resp->sock;
    ws->state = SYN_WS_STATE_CONNECTED;
    ws->is_server = true;
    ws->on_message = on_message;
    ws->ctx = ctx;
    if (req->body_buffered_len > 0U) {
        memcpy(ws->pending_rx, resp->buf + req->body_buffered_offset, req->body_buffered_len);
        ws->pending_rx_len = (uint16_t)req->body_buffered_len;
    }

    resp->upgraded = true;
    return SYN_OK;
}

static void ws_close_session(SYN_WebsocketSession *ws)
{
    if (ws->sock != SYN_SOCKET_INVALID) {
        syn_port_sock_close(ws->sock);
        ws->sock = SYN_SOCKET_INVALID;
    }
    ws->state = SYN_WS_STATE_CLOSED;
    ws->rx_state = 0U;
    ws->payload_len = 0U;
    ws->bytes_read = 0U;
    ws->pending_rx_len = 0U;
    ws->pending_rx_pos = 0U;
}

void syn_websocket_close(SYN_WebsocketSession *ws)
{
    if (ws != NULL) {
        ws_close_session(ws);
    }
}

static void ws_fail_close(SYN_WebsocketSession *ws, uint16_t status_code)
{
    uint8_t code_be[2];
    syn_poke_u16(status_code, code_be, 0U);
    (void)syn_websocket_send(ws, 0x08U, code_be, sizeof(code_be));
    ws_close_session(ws);
}

SYN_Status syn_websocket_send(SYN_WebsocketSession *ws, uint8_t opcode, const void *data,
                              size_t len)
{
    SYN_ASSERT(ws != NULL);
    if (ws->state != SYN_WS_STATE_CONNECTED || ws->sock == SYN_SOCKET_INVALID) {
        return SYN_ERROR;
    }

    uint8_t header[10];
    header[0] = (uint8_t)(0x80U | (opcode & 0x0FU));

    size_t header_len = 2U;
    if (len < 126U) {
        header[1] = (uint8_t)len;
    } else if (len <= 0xFFFFU) {
        header[1] = 126U;
        syn_poke_u16((uint16_t)len, header, 2U);
        header_len = 4U;
    } else {
        return SYN_ERROR;
    }

    if ((data == NULL) && (len > 0U)) {
        return SYN_ERROR;
    }

    if (syn_port_sock_send_all(ws->sock, header, header_len) != (int)header_len) {
        ws_close_session(ws);
        return SYN_ERROR;
    }

    if (len > 0U) {
        if (syn_port_sock_send_all(ws->sock, data, len) != (int)len) {
            ws_close_session(ws);
            return SYN_ERROR;
        }
    }

    return SYN_OK;
}

static bool ws_has_work(const SYN_WebsocketSession *ws)
{
    if (ws->state != SYN_WS_STATE_CONNECTED) {
        return true;
    }
    if (ws->pending_rx_pos < ws->pending_rx_len) {
        return true;
    }
    return (ws->sock != SYN_SOCKET_INVALID) && syn_port_sock_readable(ws->sock);
}

static void dispatch_ws_frame(SYN_WebsocketSession *ws)
{
    size_t act_len = (size_t)ws->payload_len;
    if (ws->opcode == 0x08U) {
        size_t echo_len = (act_len >= 2U) ? 2U : 0U;
        (void)syn_websocket_send(ws, 0x08U, ws->rx_buf, echo_len);
        ws_close_session(ws);
    } else if (ws->opcode == 0x09U) {
        (void)syn_websocket_send(ws, 0x0AU, ws->rx_buf, act_len);
    } else if (((ws->opcode == 0x01U) || (ws->opcode == 0x02U)) && (ws->on_message != NULL)) {
        ws->on_message(ws->rx_buf, act_len, ws->opcode, ws->ctx);
    }
}

static bool ws_header0_valid(uint8_t b0)
{
    if ((b0 & 0xF0U) != 0x80U) {
        return false;
    }
    uint8_t op = b0 & 0x0FU;
    return (op == 0x01U) || (op == 0x02U) || ((op >= 0x08U) && (op <= 0x0AU));
}

static void ws_begin_payload_phase(SYN_WebsocketSession *ws)
{
    ws->bytes_read = 0U;
    if (ws->masked) {
        ws->rx_state = 2U;
    } else if (ws->payload_len == 0U) {
        dispatch_ws_frame(ws);
        ws->rx_state = 0U;
    } else {
        ws->rx_state = 3U;
    }
}

static void ws_feed_len_byte(SYN_WebsocketSession *ws, uint8_t b)
{
    ws->masked = (b & 0x80U) != 0U;
    uint8_t l = b & 0x7FU;
    if (ws->is_server && !ws->masked) {
        ws_fail_close(ws, 1002U);
    } else if (((ws->opcode & 0x08U) != 0U) && (l >= 126U)) {
        ws_fail_close(ws, 1002U);
    } else if ((ws->opcode == 0x08U) && (l == 1U)) {
        ws_fail_close(ws, 1002U);
    } else if (l < 126U) {
        ws->payload_len = l;
        ws_begin_payload_phase(ws);
    } else if (l == 126U) {
        ws->payload_len = 0U;
        ws->rx_state = 4U;
    } else {
        ws_fail_close(ws, 1009U);
    }
}

static void ws_feed_mask_byte(SYN_WebsocketSession *ws, uint8_t b)
{
    ws->mask_key[ws->bytes_read++] = b;
    if (ws->bytes_read == 4U) {
        ws->bytes_read = 0U;
        if (ws->payload_len == 0U) {
            dispatch_ws_frame(ws);
            ws->rx_state = 0U;
        } else {
            ws->rx_state = 3U;
        }
    }
}

static void ws_feed_payload_byte(SYN_WebsocketSession *ws, uint8_t b)
{
    uint8_t out = ws->masked ? (uint8_t)(b ^ ws->mask_key[ws->bytes_read % 4U]) : b;
    ws->rx_buf[ws->bytes_read++] = out;
    if (ws->bytes_read == ws->payload_len) {
        dispatch_ws_frame(ws);
        ws->rx_state = 0U;
    }
}

static bool ws_feed_byte(SYN_WebsocketSession *ws, uint8_t b)
{
    if (ws->rx_state == 0U) {
        ws->opcode = b & 0x0FU;
        if (!ws_header0_valid(b)) {
            ws_fail_close(ws, 1002U);
        } else {
            ws->rx_state = 1U;
        }
    } else if (ws->rx_state == 1U) {
        ws_feed_len_byte(ws, b);
    } else if (ws->rx_state == 4U) {
        ws->payload_len = (uint32_t)b << 8U;
        ws->rx_state = 5U;
    } else if (ws->rx_state == 5U) {
        ws->payload_len |= b;
        if (ws->payload_len < 126U) {
            ws_fail_close(ws, 1002U);
        } else if (ws->payload_len > sizeof(ws->rx_buf)) {
            ws_fail_close(ws, 1009U);
        } else {
            ws_begin_payload_phase(ws);
        }
    } else if (ws->rx_state == 2U) {
        ws_feed_mask_byte(ws, b);
    } else {
        ws_feed_payload_byte(ws, b);
    }
    return (ws->state == SYN_WS_STATE_CONNECTED);
}

static bool ws_poll_chunk(SYN_WebsocketSession *ws)
{
    if (ws->pending_rx_pos < ws->pending_rx_len) {
        while (ws->pending_rx_pos < ws->pending_rx_len) {
            uint8_t b = ws->pending_rx[ws->pending_rx_pos++];
            if (!ws_feed_byte(ws, b)) {
                break;
            }
        }
        ws->pending_rx_pos = 0U;
        ws->pending_rx_len = 0U;
        return (ws->state == SYN_WS_STATE_CONNECTED);
    }

    uint8_t buf[64];
    bool was_readable = syn_port_sock_readable(ws->sock);
    int n = syn_port_sock_recv(ws->sock, buf, sizeof(buf), 0);
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            if (!ws_feed_byte(ws, buf[i])) {
                break;
            }
        }
        return (ws->state == SYN_WS_STATE_CONNECTED);
    }
    if (n == 0 || (n < 0 && was_readable)) {
        ws_close_session(ws);
    }
    return false;
}

SYN_PT_Status syn_websocket_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }
    SYN_WebsocketSession *ws = (SYN_WebsocketSession *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        PT_WAIT_UNTIL(pt, ws->state == SYN_WS_STATE_CONNECTED);
        if (ws->sock == SYN_SOCKET_INVALID) {
            ws_close_session(ws);
            continue;
        }

        bool progressed = ws_poll_chunk(ws);
        if (ws->state != SYN_WS_STATE_CONNECTED) {
            continue;
        }
        if (progressed && ws->sock != SYN_SOCKET_INVALID && syn_port_sock_readable(ws->sock)) {
            PT_CONTINUE_IF_NEEDED(pt, task);
            continue;
        }
        PT_WAIT_UNTIL(pt, ws_has_work(ws));
    }

    PT_END(pt);
}

#endif /* SYN_USE_WEBSOCKET */
```


