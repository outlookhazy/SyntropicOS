#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_HTTP) || SYN_USE_HTTP

/**
 * @file syn_http.c
 * @brief Cooperative HTTP/1.1 client implementation.
 */

#include "syn_http.h"
#include "../util/syn_assert.h"
#include "../util/syn_fmt.h"
#include "../port/syn_port_system.h"

#include <string.h>
#include <stdlib.h>

#define HTTP_RECV_TIMEOUT_MS  10000  /**< HTTP receive timeout (ms). */

/* ── Helpers ────────────────────────────────────────────────────────────── */

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

/**
 * @brief Parse a decimal unsigned integer.
 * @param s  Input string.
 * @return Parsed value.
 */
static uint32_t parse_uint(const char *s)
{
    uint32_t val = 0;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (uint32_t)(*s - '0');
        s++;
    }
    return val;
}

/**
 * @brief Write a null-terminated string to the socket.
 * @param sock  Socket.
 * @param str   String to send.
 * @return true if all bytes sent.
 */
static bool sock_write_str(SYN_Socket sock, const char *str)
{
    size_t len = strlen(str);
    int sent = syn_port_sock_send_all(sock, str, len);
    return (sent == (int)len);
}

/**
 * @brief Build and send an HTTP request line + headers.
 * @param sock            Socket.
 * @param method          HTTP method string.
 * @param host            Host header value.
 * @param path            Request path.
 * @param headers         Custom headers array.
 * @param header_count    Number of custom headers.
 * @param content_type    Content-Type (or NULL).
 * @param content_length  Body length (0 for no body).
 * @return true on success.
 */
static bool send_request(SYN_Socket sock, const char *method,
                         const char *host, const char *path,
                         const SYN_HttpHeader *headers, uint8_t header_count,
                         const char *content_type, size_t content_length)
{
    if (!sock_write_str(sock, method))       return false;
    if (!sock_write_str(sock, " "))          return false;
    if (!sock_write_str(sock, path))         return false;
    if (!sock_write_str(sock, " HTTP/1.1\r\n")) return false;

    if (!sock_write_str(sock, "Host: "))     return false;
    if (!sock_write_str(sock, host))         return false;
    if (!sock_write_str(sock, "\r\n"))       return false;

    if (content_type != NULL) {
        if (!sock_write_str(sock, "Content-Type: ")) return false;
        if (!sock_write_str(sock, content_type))     return false;
        if (!sock_write_str(sock, "\r\n"))           return false;
    }

    if (content_length > 0) {
        char len_str[16];
        int pos = 0;
        uint32_t cl = (uint32_t)content_length;
        char tmp[16];
        int tpos = 0;
        while (cl > 0) {
            tmp[tpos++] = (char)('0' + (cl % 10));
            cl /= 10;
        }
        for (int i = tpos - 1; i >= 0; i--) {
            len_str[pos++] = tmp[i];
        }
        len_str[pos] = '\0';

        if (!sock_write_str(sock, "Content-Length: ")) return false;
        if (!sock_write_str(sock, len_str))            return false;
        if (!sock_write_str(sock, "\r\n"))             return false;
    }

    if (!sock_write_str(sock, "Connection: close\r\n")) return false;

    for (uint8_t i = 0; i < header_count; i++) {
        if (headers[i].name == NULL || headers[i].value == NULL) continue;
        if (!sock_write_str(sock, headers[i].name))  return false;
        if (!sock_write_str(sock, ": "))             return false;
        if (!sock_write_str(sock, headers[i].value)) return false;
        if (!sock_write_str(sock, "\r\n"))           return false;
    }

    if (!sock_write_str(sock, "\r\n")) return false;
    return true;
}

/**
 * @brief Parse a redirect Location header URL.
 * @param url         Location URL string.
 * @param orig_host   Original request host.
 * @param orig_port   Original request port.
 * @param host_out    [out] Redirected host.
 * @param host_sz     Host buffer size.
 * @param path_out    [out] Redirected path.
 * @param path_sz     Path buffer size.
 * @param port_out    [out] Redirected port.
 */
static void parse_redirect_url(const char *url, const char *orig_host, uint16_t orig_port,
                               char *host_out, size_t host_sz,
                               char *path_out, size_t path_sz,
                               uint16_t *port_out)
{
    if (strncmp(url, "http://", 7) == 0) {
        url += 7;
        const char *slash = strchr(url, '/');
        const char *colon = strchr(url, ':');
        if (colon && (!slash || colon < slash)) {
            size_t host_len = (size_t)(colon - url);
            if (host_len >= host_sz) host_len = host_sz - 1;
            memcpy(host_out, url, host_len);
            host_out[host_len] = '\0';
            *port_out = (uint16_t)atoi(colon + 1);
        } else {
            size_t host_len = slash ? (size_t)(slash - url) : strlen(url);
            if (host_len >= host_sz) host_len = host_sz - 1;
            memcpy(host_out, url, host_len);
            host_out[host_len] = '\0';
            *port_out = 80;
        }
        if (slash) {
            strncpy(path_out, slash, path_sz - 1);
        } else {
            strcpy(path_out, "/");
        }
        path_out[path_sz - 1] = '\0';
    } else if (url[0] == '/') {
        strncpy(host_out, orig_host, host_sz - 1);
        host_out[host_sz - 1] = '\0';
        *port_out = orig_port;
        strncpy(path_out, url, path_sz - 1);
        path_out[path_sz - 1] = '\0';
    } else {
        strncpy(host_out, orig_host, host_sz - 1);
        host_out[host_sz - 1] = '\0';
        *port_out = orig_port;
        path_out[0] = '/';
        strncpy(path_out + 1, url, path_sz - 2);
        path_out[path_sz - 1] = '\0';
    }
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_http_client_init(SYN_HttpClient *client,
                                const char *method,
                                const char *host, uint16_t port,
                                const char *path,
                                const char *content_type,
                                const uint8_t *body, size_t body_len,
                                const SYN_HttpHeader *headers, uint8_t header_count,
                                SYN_HttpBodyCallback body_cb, void *cb_ctx,
                                uint8_t *work_buf, size_t work_buf_size)
{
    SYN_ASSERT(client != NULL);
    SYN_ASSERT(method != NULL);
    SYN_ASSERT(host != NULL);
    SYN_ASSERT(path != NULL);
    SYN_ASSERT(work_buf != NULL);
    SYN_ASSERT(work_buf_size >= 128);

    memset(client, 0, sizeof(*client));
    client->state = SYN_HTTP_STATE_IDLE;
    client->sock = SYN_SOCKET_INVALID;
    client->method = method;
    client->host = host;
    client->port = port;
    client->path = path;
    client->content_type = content_type;
    client->body = body;
    client->body_len = body_len;
    client->headers = headers;
    client->header_count = header_count;
    client->body_cb = body_cb;
    client->cb_ctx = cb_ctx;
    client->work_buf = work_buf;
    client->work_buf_size = work_buf_size;
    client->status = SYN_OK;

    strncpy(client->cur_host, host, sizeof(client->cur_host) - 1);
    client->cur_host[sizeof(client->cur_host) - 1] = '\0';
    strncpy(client->cur_path, path, sizeof(client->cur_path) - 1);
    client->cur_path[sizeof(client->cur_path) - 1] = '\0';
    client->cur_port = port;

    return SYN_OK;
}

SYN_PT_Status syn_http_client_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_HttpClient *c = (SYN_HttpClient *)task->user_data;
    SYN_ASSERT(c != NULL);

    PT_BEGIN(pt);

    c->hops = 0;

    for (;;) {
        c->state = SYN_HTTP_STATE_CONNECTING;
        c->sock = syn_port_sock_connect_host(c->cur_host, c->cur_port);
        if (c->sock == SYN_SOCKET_INVALID) {
            c->state = SYN_HTTP_STATE_ERROR;
            c->status = SYN_ERROR;
            PT_EXIT(pt);
        }

        c->state = SYN_HTTP_STATE_SENDING_REQUEST;
        if (!send_request(c->sock, c->method, c->cur_host, c->cur_path,
                          c->headers, c->header_count,
                          c->content_type, c->body_len)) {
            syn_port_sock_close(c->sock);
            c->sock = SYN_SOCKET_INVALID;
            c->state = SYN_HTTP_STATE_ERROR;
            c->status = SYN_ERROR;
            PT_EXIT(pt);
        }

        if (c->body != NULL && c->body_len > 0) {
            int sent = syn_port_sock_send_all(c->sock, c->body, c->body_len);
            if (sent != (int)c->body_len) {
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_HTTP_STATE_ERROR;
                c->status = SYN_ERROR;
                PT_EXIT(pt);
            }
        }

        c->state = SYN_HTTP_STATE_READING_HEADERS;
        c->buf_used = 0;
        memset(&c->resp, 0, sizeof(c->resp));
        c->header_timeout_ms = syn_port_get_tick_ms();

        for (;;) {
            int n = syn_port_sock_recv(c->sock, c->work_buf + c->buf_used,
                                       c->work_buf_size - 1 - c->buf_used, 0);
            if (n > 0) {
                c->buf_used += (size_t)n;
                c->work_buf[c->buf_used] = '\0';
                c->header_timeout_ms = syn_port_get_tick_ms();
            } else if (n == 0) {
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_HTTP_STATE_ERROR;
                c->status = SYN_ERROR;
                PT_EXIT(pt);
            } else {
                if ((syn_port_get_tick_ms() - c->header_timeout_ms) >= HTTP_RECV_TIMEOUT_MS) {
                    syn_port_sock_close(c->sock);
                    c->sock = SYN_SOCKET_INVALID;
                    c->state = SYN_HTTP_STATE_ERROR;
                    c->status = SYN_TIMEOUT;
                    PT_EXIT(pt);
                }
                PT_YIELD(pt);
                continue;
            }

            const char *end = strstr((const char *)c->work_buf, "\r\n\r\n");
            if (end != NULL) {
                c->body_start = (size_t)(end - (const char *)c->work_buf) + 4;
                break;
            }

            if (c->buf_used >= c->work_buf_size - 1) {
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_HTTP_STATE_ERROR;
                c->status = SYN_ERROR;
                PT_EXIT(pt);
            }
        }

        {
            const char *line = (const char *)c->work_buf;
            const char *sp = strchr(line, ' ');
            if (sp == NULL) {
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_HTTP_STATE_ERROR;
                c->status = SYN_ERROR;
                PT_EXIT(pt);
            }
            c->resp.status_code = (int)parse_uint(sp + 1);

            const char *hdr_end = (const char *)c->work_buf + c->body_start - 2;
            const char *cur = strchr(line, '\n');
            if (cur) cur++;

            while (cur && cur < hdr_end) {
                if (prefix_icase(cur, "content-length:")) {
                    const char *val = cur + 15;
                    while (*val == ' ') val++;
                    c->resp.content_length = parse_uint(val);
                } else if (prefix_icase(cur, "transfer-encoding:")) {
                    const char *val = cur + 18;
                    while (*val == ' ') val++;
                    if (prefix_icase(val, "chunked")) {
                        c->resp.chunked = true;
                    }
                } else if (prefix_icase(cur, "connection:")) {
                    const char *val = cur + 11;
                    while (*val == ' ') val++;
                    if (prefix_icase(val, "close")) {
                        c->resp.connection_close = true;
                    }
                } else if (prefix_icase(cur, "location:")) {
                    const char *val = cur + 9;
                    while (*val == ' ') val++;
                    size_t len = 0;
                    while (val[len] != '\r' && val[len] != '\n' && len < sizeof(c->resp.location) - 1) {
                        c->resp.location[len] = val[len];
                        len++;
                    }
                    c->resp.location[len] = '\0';
                }
                cur = strchr(cur, '\n');
                if (cur) cur++;
            }
        }

        if (c->resp.status_code >= 300 && c->resp.status_code < 400 && c->resp.location[0] != '\0') {
            syn_port_sock_close(c->sock);
            c->sock = SYN_SOCKET_INVALID;
            if (++c->hops >= 5) {
                c->state = SYN_HTTP_STATE_ERROR;
                c->status = SYN_ERROR;
                PT_EXIT(pt);
            }
            char next_host[64];
            char next_path[128];
            uint16_t next_port;
            parse_redirect_url(c->resp.location, c->cur_host, c->cur_port,
                               next_host, sizeof(next_host),
                               next_path, sizeof(next_path),
                               &next_port);
            strcpy(c->cur_host, next_host);
            strcpy(c->cur_path, next_path);
            c->cur_port = next_port;
            continue;
        }

        if (c->body_cb == NULL) {
            syn_port_sock_close(c->sock);
            c->sock = SYN_SOCKET_INVALID;
            c->state = SYN_HTTP_STATE_DONE;
            c->status = SYN_OK;
            PT_EXIT(pt);
        }

        c->state = SYN_HTTP_STATE_READING_BODY;
        c->buf_pos = c->body_start;
        c->body_timeout_ms = syn_port_get_tick_ms();

        if (c->resp.chunked) {
            c->chunk_state = 0;
            c->chunk_line_pos = 0;
            for (;;) {
                if (c->chunk_state == 0) {
                    if (c->buf_pos < c->buf_used) {
                        uint8_t ch = c->work_buf[c->buf_pos++];
                        if (c->chunk_line_pos < sizeof(c->chunk_line) - 1) {
                            c->chunk_line[c->chunk_line_pos++] = (char)ch;
                        }
                        if (ch == '\n') {
                            c->chunk_line[c->chunk_line_pos] = '\0';
                            c->chunk_remaining = 0;
                            const char *p = c->chunk_line;
                            while (*p) {
                                char hc = *p++;
                                if (hc >= '0' && hc <= '9') c->chunk_remaining = (c->chunk_remaining << 4) | (uint32_t)(hc - '0');
                                else if (hc >= 'a' && hc <= 'f') c->chunk_remaining = (c->chunk_remaining << 4) | (uint32_t)(hc - 'a' + 10);
                                else if (hc >= 'A' && hc <= 'F') c->chunk_remaining = (c->chunk_remaining << 4) | (uint32_t)(hc - 'A' + 10);
                                else if (hc == ';' || hc == '\r' || hc == '\n') break;
                                else {
                                    syn_port_sock_close(c->sock);
                                    c->sock = SYN_SOCKET_INVALID;
                                    c->state = SYN_HTTP_STATE_ERROR;
                                    c->status = SYN_ERROR;
                                    PT_EXIT(pt);
                                }
                            }

                            if (c->chunk_remaining == 0) {
                                c->chunk_state = 2;
                            } else {
                                c->chunk_state = 1;
                            }
                            c->chunk_line_pos = 0;
                        }
                    } else {
                        int n = syn_port_sock_recv(c->sock, c->work_buf, c->work_buf_size, 0);
                        if (n > 0) {
                            c->buf_used = (size_t)n;
                            c->buf_pos = 0;
                            c->body_timeout_ms = syn_port_get_tick_ms();
                        } else if (n == 0) {
                            syn_port_sock_close(c->sock);
                            c->sock = SYN_SOCKET_INVALID;
                            c->state = SYN_HTTP_STATE_ERROR;
                            c->status = SYN_ERROR;
                            PT_EXIT(pt);
                        } else {
                            if ((syn_port_get_tick_ms() - c->body_timeout_ms) >= HTTP_RECV_TIMEOUT_MS) {
                                syn_port_sock_close(c->sock);
                                c->sock = SYN_SOCKET_INVALID;
                                c->state = SYN_HTTP_STATE_ERROR;
                                c->status = SYN_TIMEOUT;
                                PT_EXIT(pt);
                            }
                            PT_YIELD(pt);
                        }
                    }
                } else if (c->chunk_state == 1) {
                    if (c->chunk_remaining > 0) {
                        size_t avail = c->buf_used - c->buf_pos;
                        if (avail > 0) {
                            size_t to_deliver = (avail > c->chunk_remaining) ? c->chunk_remaining : avail;
                            if (!c->body_cb(c->work_buf + c->buf_pos, to_deliver, c->cb_ctx)) {
                                syn_port_sock_close(c->sock);
                                c->sock = SYN_SOCKET_INVALID;
                                c->state = SYN_HTTP_STATE_ERROR;
                                c->status = SYN_ERROR;
                                PT_EXIT(pt);
                            }
                            c->buf_pos += to_deliver;
                            c->chunk_remaining -= (uint32_t)to_deliver;
                        } else {
                            int n = syn_port_sock_recv(c->sock, c->work_buf, c->work_buf_size, 0);
                            if (n > 0) {
                                c->buf_used = (size_t)n;
                                c->buf_pos = 0;
                                c->body_timeout_ms = syn_port_get_tick_ms();
                            } else if (n == 0) {
                                syn_port_sock_close(c->sock);
                                c->sock = SYN_SOCKET_INVALID;
                                c->state = SYN_HTTP_STATE_ERROR;
                                c->status = SYN_ERROR;
                                PT_EXIT(pt);
                            } else {
                                if ((syn_port_get_tick_ms() - c->body_timeout_ms) >= HTTP_RECV_TIMEOUT_MS) {
                                    syn_port_sock_close(c->sock);
                                    c->sock = SYN_SOCKET_INVALID;
                                    c->state = SYN_HTTP_STATE_ERROR;
                                    c->status = SYN_TIMEOUT;
                                    PT_EXIT(pt);
                                }
                                PT_YIELD(pt);
                            }
                        }
                    } else {
                        if (c->buf_pos < c->buf_used) {
                            uint8_t ch = c->work_buf[c->buf_pos++];
                            if (ch == '\n') {
                                c->chunk_state = 0;
                            }
                        } else {
                            int n = syn_port_sock_recv(c->sock, c->work_buf, c->work_buf_size, 0);
                            if (n > 0) {
                                c->buf_used = (size_t)n;
                                c->buf_pos = 0;
                                c->body_timeout_ms = syn_port_get_tick_ms();
                            } else if (n == 0) {
                                syn_port_sock_close(c->sock);
                                c->sock = SYN_SOCKET_INVALID;
                                c->state = SYN_HTTP_STATE_ERROR;
                                c->status = SYN_ERROR;
                                PT_EXIT(pt);
                            } else {
                                if ((syn_port_get_tick_ms() - c->body_timeout_ms) >= HTTP_RECV_TIMEOUT_MS) {
                                    syn_port_sock_close(c->sock);
                                    c->sock = SYN_SOCKET_INVALID;
                                    c->state = SYN_HTTP_STATE_ERROR;
                                    c->status = SYN_TIMEOUT;
                                    PT_EXIT(pt);
                                }
                                PT_YIELD(pt);
                            }
                        }
                    }
                } else if (c->chunk_state == 2) {
                    if (c->buf_pos < c->buf_used) {
                        uint8_t ch = c->work_buf[c->buf_pos++];
                        if (ch == '\n') {
                            break;
                        }
                    } else {
                        int n = syn_port_sock_recv(c->sock, c->work_buf, c->work_buf_size, 0);
                        if (n > 0) {
                            c->buf_used = (size_t)n;
                            c->buf_pos = 0;
                            c->body_timeout_ms = syn_port_get_tick_ms();
                        } else if (n == 0) {
                            break;
                        } else {
                            if ((syn_port_get_tick_ms() - c->body_timeout_ms) >= HTTP_RECV_TIMEOUT_MS) {
                                syn_port_sock_close(c->sock);
                                c->sock = SYN_SOCKET_INVALID;
                                c->state = SYN_HTTP_STATE_ERROR;
                                c->status = SYN_TIMEOUT;
                                PT_EXIT(pt);
                            }
                            PT_YIELD(pt);
                        }
                    }
                }
            }
        } else {
            c->body_remaining = c->resp.content_length;
            c->known_length = (c->resp.content_length > 0);

            size_t avail = c->buf_used - c->buf_pos;
            if (avail > 0) {
                size_t to_deliver = avail;
                if (c->known_length && to_deliver > c->body_remaining) {
                    to_deliver = c->body_remaining;
                }
                if (!c->body_cb(c->work_buf + c->buf_pos, to_deliver, c->cb_ctx)) {
                    syn_port_sock_close(c->sock);
                    c->sock = SYN_SOCKET_INVALID;
                    c->state = SYN_HTTP_STATE_ERROR;
                    c->status = SYN_ERROR;
                    PT_EXIT(pt);
                }
                if (c->known_length) {
                    c->body_remaining -= (uint32_t)to_deliver;
                }
                c->buf_pos += to_deliver;
            }

            while (!c->known_length || c->body_remaining > 0) {
                size_t to_read = c->work_buf_size;
                if (c->known_length && to_read > c->body_remaining) {
                    to_read = c->body_remaining;
                }
                int n = syn_port_sock_recv(c->sock, c->work_buf, to_read, 0);
                if (n > 0) {
                    if (!c->body_cb(c->work_buf, (size_t)n, c->cb_ctx)) {
                        syn_port_sock_close(c->sock);
                        c->sock = SYN_SOCKET_INVALID;
                        c->state = SYN_HTTP_STATE_ERROR;
                        c->status = SYN_ERROR;
                        PT_EXIT(pt);
                    }
                    if (c->known_length) {
                        c->body_remaining -= (uint32_t)n;
                    }
                    c->body_timeout_ms = syn_port_get_tick_ms();
                } else if (n == 0) {
                    if (c->known_length && c->body_remaining > 0) {
                        syn_port_sock_close(c->sock);
                        c->sock = SYN_SOCKET_INVALID;
                        c->state = SYN_HTTP_STATE_ERROR;
                        c->status = SYN_ERROR;
                        PT_EXIT(pt);
                    }
                    break;
                } else {
                    if ((syn_port_get_tick_ms() - c->body_timeout_ms) >= HTTP_RECV_TIMEOUT_MS) {
                        syn_port_sock_close(c->sock);
                        c->sock = SYN_SOCKET_INVALID;
                        c->state = SYN_HTTP_STATE_ERROR;
                        c->status = SYN_TIMEOUT;
                        PT_EXIT(pt);
                    }
                    PT_YIELD(pt);
                }
            }
        }

        syn_port_sock_close(c->sock);
        c->sock = SYN_SOCKET_INVALID;
        c->state = SYN_HTTP_STATE_DONE;
        c->status = SYN_OK;
        break;
    }

    PT_END(pt);
}

#endif /* SYN_USE_HTTP */
