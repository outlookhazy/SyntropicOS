#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_WEBSOCKET) || SYN_USE_WEBSOCKET

/**
 * @file syn_websocket.c
 * @brief WebSocket protocol implementation.
 */

#include "syn_websocket.h"
#include "../util/syn_assert.h"
#include <string.h>

/* ── SHA-1 ──────────────────────────────────────────────────────────────── */

/**
 * @brief SHA-1 hashing algorithm context.
 */
typedef struct {
    uint32_t state[5];               /**< Internal state registers */
    uint32_t count[2];               /**< Number of bits processed */
    uint8_t  buffer[64];             /**< Input buffer staging area */
} SYN_SHA1_Ctx;

/** @brief SHA-1 rotate-left helper. */
#define SHA1_ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/**
 * @brief Initialize SHA-1 context.
 * @param ctx  SHA-1 context to initialize.
 */
static void sha1_init(SYN_SHA1_Ctx *ctx)
{
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
    ctx->count[0] = ctx->count[1] = 0;
}

/**
 * @brief Process a 64-byte SHA-1 block.
 * @param state   Running hash state.
 * @param buffer  64-byte input block.
 */
static void sha1_transform(uint32_t state[5], const uint8_t buffer[64])
{
    uint32_t w[80];
    for (int i = 0; i < 16; i++) {
        w[i] = ((uint32_t)buffer[i*4] << 24) |
               ((uint32_t)buffer[i*4+1] << 16) |
               ((uint32_t)buffer[i*4+2] << 8) |
               (uint32_t)buffer[i*4+3];
    }
    for (int i = 16; i < 80; i++) {
        w[i] = SHA1_ROL(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
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

/**
 * @brief Feed data into the SHA-1 context.
 * @param ctx   SHA-1 context.
 * @param data  Input data.
 * @param len   Input length.
 */
static void sha1_update(SYN_SHA1_Ctx *ctx, const uint8_t *data, uint32_t len)
{
    uint32_t i, j;
    j = (ctx->count[0] >> 3) & 63;
    if ((ctx->count[0] += len << 3) < (len << 3)) ctx->count[1]++;
    ctx->count[1] += (len >> 29);
    if ((j + len) > 63) {
        memcpy(&ctx->buffer[j], data, (i = 64 - j));
        sha1_transform(ctx->state, ctx->buffer);
        for (; i + 63 < len; i += 64) {
            sha1_transform(ctx->state, &data[i]);
        }
        j = 0;
    } else {
        i = 0;
    }
    memcpy(&ctx->buffer[j], &data[i], len - i);
}

/**
 * @brief Finalize SHA-1 and write the 20-byte digest.
 * @param ctx     SHA-1 context.
 * @param digest  [out] 20-byte digest.
 */
static void sha1_final(SYN_SHA1_Ctx *ctx, uint8_t digest[20])
{
    uint8_t finalcount[8];
    for (int i = 0; i < 8; i++) {
        finalcount[i] = (uint8_t)((ctx->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8) ) & 255);
    }
    uint8_t c = 0200;
    sha1_update(ctx, &c, 1);
    while ((ctx->count[0] >> 3 & 63) != 56) {
        uint8_t z = 0;
        sha1_update(ctx, &z, 1);
    }
    sha1_update(ctx, finalcount, 8);
    for (int i = 0; i < 20; i++) {
        digest[i] = (uint8_t)((ctx->state[i>>2] >> ((3 - (i & 3)) * 8) ) & 255);
    }
}

/* ── Base64 ─────────────────────────────────────────────────────────────── */

/**
 * @brief Base64 encode a byte array.
 * @param src  Source bytes.
 * @param len  Number of bytes.
 * @param dst  [out] Null-terminated base64 string.
 */
static void base64_encode(const uint8_t *src, size_t len, char *dst)
{
    const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    while (len >= 3) {
        dst[0] = table[src[0] >> 2];
        dst[1] = table[((src[0] & 0x03) << 4) | (src[1] >> 4)];
        dst[2] = table[((src[1] & 0x0F) << 2) | (src[2] >> 6)];
        dst[3] = table[src[2] & 0x3F];
        src += 3;
        dst += 4;
        len -= 3;
    }
    if (len == 2) {
        dst[0] = table[src[0] >> 2];
        dst[1] = table[((src[0] & 0x03) << 4) | (src[1] >> 4)];
        dst[2] = table[(src[1] & 0x0F) << 2];
        dst[3] = '=';
        dst += 4;
    } else if (len == 1) {
        dst[0] = table[src[0] >> 2];
        dst[1] = table[(src[0] & 0x03) << 4];
        dst[2] = '=';
        dst[3] = '=';
        dst += 4;
    }
    *dst = '\0';
}

/* ── Websocket Upgrading ────────────────────────────────────────────────── */

/**
 * @brief Case-insensitive prefix match.
 * @param str     String to test.
 * @param prefix  Prefix to match.
 * @return true if match.
 */
static bool prefix_icase(const char *str, const char *prefix)
{
    while (*prefix) {
        char a = *str++;
        char b = *prefix++;
        if (a >= 'A' && a <= 'Z') a += 32;
        if (b >= 'A' && b <= 'Z') b += 32;
        if (a != b) return false;
    }
    return true;
}

SYN_Status syn_websocket_upgrade(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp,
                                 SYN_WebsocketSession *ws,
                                 void (*on_message)(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx),
                                 void *ctx)
{
    SYN_ASSERT(req != NULL);
    SYN_ASSERT(resp != NULL);
    SYN_ASSERT(ws != NULL);

    /* Search for Sec-WebSocket-Key inside request headers */
    const char *headers = req->headers;
    const char *key_hdr = NULL;
    
    /* Safely look through headers */
    const char *cur = headers;
    while (*cur) {
        if (prefix_icase(cur, "sec-websocket-key:")) {
            key_hdr = cur + 18;
            while (*key_hdr == ' ') key_hdr++;
            break;
        }
        cur = strchr(cur, '\n');
        if (!cur) break;
        cur++;
    }

    if (key_hdr == NULL) {
        return SYN_ERROR; /* key header not found */
    }

    /* Extract the key (terminated by \r or \n) */
    char key[64];
    size_t key_len = 0;
    while (key_hdr[key_len] != '\r' && key_hdr[key_len] != '\n' && key_len < sizeof(key) - 1) {
        key[key_len] = key_hdr[key_len];
        key_len++;
    }
    key[key_len] = '\0';

    /* Compute Sec-WebSocket-Accept = Base64(SHA-1(key + UUID)) */
    char accept_buf[128];
    strcpy(accept_buf, key);
    strcat(accept_buf, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    SYN_SHA1_Ctx sha;
    uint8_t digest[20];
    sha1_init(&sha);
    sha1_update(&sha, (const uint8_t *)accept_buf, (uint32_t)strlen(accept_buf));
    sha1_final(&sha, digest);

    char accept_key[32];
    base64_encode(digest, 20, accept_key);

    /* Send response headers */
    syn_port_sock_send_all(resp->sock, "HTTP/1.1 101 Switching Protocols\r\n", 34);
    syn_port_sock_send_all(resp->sock, "Upgrade: websocket\r\n", 20);
    syn_port_sock_send_all(resp->sock, "Connection: Upgrade\r\n", 21);
    syn_port_sock_send_all(resp->sock, "Sec-WebSocket-Accept: ", 22);
    syn_port_sock_send_all(resp->sock, accept_key, strlen(accept_key));
    syn_port_sock_send_all(resp->sock, "\r\n\r\n", 4);

    /* Configure session */
    memset(ws, 0, sizeof(*ws));
    ws->sock = resp->sock;
    ws->state = SYN_WS_STATE_CONNECTED;
    ws->on_message = on_message;
    ws->ctx = ctx;

    /* Flag response as upgraded so httpd doesn't close socket */
    resp->upgraded = true;

    return SYN_OK;
}

SYN_Status syn_websocket_send(SYN_WebsocketSession *ws, uint8_t opcode,
                              const void *data, size_t len)
{
    SYN_ASSERT(ws != NULL);
    if (ws->state != SYN_WS_STATE_CONNECTED) return SYN_ERROR;

    uint8_t header[10];
    header[0] = 0x80 | (opcode & 0x0F); /* FIN = 1 */

    size_t header_len = 2;
    if (len < 126) {
        header[1] = (uint8_t)len; /* Mask = 0 */
    } else if (len <= 0xFFFF) {
        header[1] = 126;
        header[2] = (uint8_t)(len >> 8);
        header[3] = (uint8_t)(len & 0xFF);
        header_len = 4;
    } else {
        /* Large payload (limit/not supported on simple stack) */
        return SYN_ERROR;
    }

    if (syn_port_sock_send_all(ws->sock, header, header_len) != (int)header_len) {
        ws->state = SYN_WS_STATE_CLOSED;
        return SYN_ERROR;
    }

    if (len > 0 && data != NULL) {
        if (syn_port_sock_send_all(ws->sock, data, len) != (int)len) {
            ws->state = SYN_WS_STATE_CLOSED;
            return SYN_ERROR;
        }
    }

    return SYN_OK;
}

SYN_PT_Status syn_websocket_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_WebsocketSession *ws = (SYN_WebsocketSession *)task->user_data;
    SYN_ASSERT(ws != NULL);

    PT_BEGIN(pt);

    for (;;) {
        if (ws->state == SYN_WS_STATE_CONNECTED) {
            /* Try to read multiple bytes (non-blocking) */
            uint8_t buf[64];
            int n = syn_port_sock_recv(ws->sock, buf, sizeof(buf), 0);
            if (n > 0) {
                for (int i = 0; i < n; i++) {
                    uint8_t b = buf[i];
                    /* Process byte using internal state machine */
                    if (ws->rx_state == 0) {
                        /* FIN + Opcode */
                        ws->opcode = b & 0x0F;
                        ws->rx_state = 1;
                    } else if (ws->rx_state == 1) {
                        /* Mask + Length */
                        ws->masked = (b & 0x80) != 0;
                        uint8_t l = b & 0x7F;
                        if (l < 126) {
                            ws->payload_len = l;
                            ws->rx_state = ws->masked ? 2 : 3;
                            ws->bytes_read = 0;
                        } else if (l == 126) {
                            /* 2 byte length */
                            ws->payload_len = 0;
                            ws->rx_state = 4; /* state 4/5: length accumulation */
                        } else {
                            /* Too large, close */
                            syn_port_sock_close(ws->sock);
                            ws->state = SYN_WS_STATE_CLOSED;
                            break;
                        }
                    } else if (ws->rx_state == 4) {
                        ws->payload_len = (uint32_t)b << 8;
                        ws->rx_state = 5;
                    } else if (ws->rx_state == 5) {
                        ws->payload_len |= b;
                        ws->rx_state = ws->masked ? 2 : 3;
                        ws->bytes_read = 0;
                    } else if (ws->rx_state == 2) {
                        /* Read Masking Key */
                        ws->mask_key[ws->bytes_read++] = b;
                        if (ws->bytes_read == 4) {
                            ws->rx_state = 3;
                            ws->bytes_read = 0;
                        }
                    } else if (ws->rx_state == 3) {
                        /* Read Payload */
                        if (ws->bytes_read < sizeof(ws->rx_buf)) {
                            ws->rx_buf[ws->bytes_read] = b;
                            if (ws->masked) {
                                ws->rx_buf[ws->bytes_read] ^= ws->mask_key[ws->bytes_read % 4];
                            }
                        }
                        ws->bytes_read++;
                        if (ws->bytes_read == ws->payload_len) {
                            /* Finished reading frame */
                            if (ws->opcode == 0x08) {
                                /* CLOSE frame */
                                syn_port_sock_close(ws->sock);
                                ws->state = SYN_WS_STATE_CLOSED;
                                break;
                            } else if (ws->opcode == 0x09) {
                                /* PING, reply with PONG */
                                syn_websocket_send(ws, 0x0A, ws->rx_buf, ws->payload_len < sizeof(ws->rx_buf) ? ws->payload_len : sizeof(ws->rx_buf));
                            } else if (ws->opcode == 0x01 || ws->opcode == 0x02) {
                                /* Text/Binary message */
                                if (ws->on_message != NULL) {
                                    size_t act_len = ws->payload_len < sizeof(ws->rx_buf) ? ws->payload_len : sizeof(ws->rx_buf);
                                    ws->on_message(ws->rx_buf, act_len, ws->opcode, ws->ctx);
                                }
                            }
                            ws->rx_state = 0;
                        }
                    }
                }
            } else if (n == 0) {
                /* Connection closed by peer */
                syn_port_sock_close(ws->sock);
                ws->state = SYN_WS_STATE_CLOSED;
            }
        }
        PT_YIELD(pt);
    }

    PT_END(pt);
}

#endif /* SYN_USE_WEBSOCKET */
