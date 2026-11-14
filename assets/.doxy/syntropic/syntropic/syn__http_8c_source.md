

# File syn\_http.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_http.c**](syn__http_8c.md)

[Go to the documentation of this file](syn__http_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_HTTP) || SYN_USE_HTTP


#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "../util/syn_assert.h"
#include "../util/syn_fmt.h"
#include "syn_http.h"

#include <string.h>

#define HTTP_RECV_TIMEOUT_MS 10000 

/* ── Helpers ────────────────────────────────────────────────────────────── */

static bool prefix_icase(const char *str, const char *prefix)
{
    return syn_str_prefix_icase(str, prefix);
}

static uint32_t parse_uint(const char *s)
{
    return syn_fmt_parse_uint(s);
}

static bool sock_write_str(SYN_Socket sock, const char *str)
{
    size_t len = strlen(str);
    int sent = syn_port_sock_send_all(sock, str, len);
    return (sent == (int)len);
}

static bool send_content_headers(SYN_Socket sock, const char *content_type, size_t content_length)
{
    if (content_type != NULL) {
        if (!sock_write_str(sock, "Content-Type: ")) {
            return false;
        }
        if (!sock_write_str(sock, content_type)) {
            return false;
        }
        if (!sock_write_str(sock, "\r\n")) {
            return false;
        }
    }

    if (content_length > 0U) {
        char len_str[16];
        int pos = 0;
        uint32_t cl = (uint32_t)content_length;
        char tmp[16];
        int tpos = 0;
        while (cl > 0U) {
            tmp[tpos++] = (char)('0' + (cl % 10U));
            cl /= 10U;
        }
        for (int i = tpos - 1; i >= 0; i--) {
            len_str[pos++] = tmp[i];
        }
        len_str[pos] = '\0';

        if (!sock_write_str(sock, "Content-Length: ")) {
            return false;
        }
        if (!sock_write_str(sock, len_str)) {
            return false;
        }
        if (!sock_write_str(sock, "\r\n")) {
            return false;
        }
    }
    return true;
}

static bool send_custom_headers(SYN_Socket sock, const SYN_HttpHeader *headers,
                                uint8_t header_count)
{
    for (uint8_t i = 0; i < header_count; i++) {
        if (headers[i].name == NULL || headers[i].value == NULL) {
            continue;
        }
        if (!sock_write_str(sock, headers[i].name)) {
            return false;
        }
        if (!sock_write_str(sock, ": ")) {
            return false;
        }
        if (!sock_write_str(sock, headers[i].value)) {
            return false;
        }
        if (!sock_write_str(sock, "\r\n")) {
            return false;
        }
    }
    return true;
}

static bool send_request(SYN_Socket sock, const char *method, const char *host, const char *path,
                         const SYN_HttpHeader *headers, uint8_t header_count,
                         const char *content_type, size_t content_length)
{
    if (!sock_write_str(sock, method)) {
        return false;
    }
    if (!sock_write_str(sock, " ")) {
        return false;
    }
    if (!sock_write_str(sock, path)) {
        return false;
    }
    if (!sock_write_str(sock, " HTTP/1.1\r\n")) {
        return false;
    }
    if (!sock_write_str(sock, "Host: ")) {
        return false;
    }
    if (!sock_write_str(sock, host)) {
        return false;
    }
    if (!sock_write_str(sock, "\r\n")) {
        return false;
    }
    if (!send_content_headers(sock, content_type, content_length)) {
        return false;
    }
    if (!sock_write_str(sock, "Connection: close\r\n")) {
        return false;
    }
    if (!send_custom_headers(sock, headers, header_count)) {
        return false;
    }
    if (!sock_write_str(sock, "\r\n")) {
        return false;
    }
    return true;
}

static bool http_parse_port(const char *p, uint16_t *port_out)
{
    uint32_t port = 0U;
    bool has_digit = false;
    while ((*p != '\0') && (*p != '/')) {
        if ((*p < '0') || (*p > '9')) {
            return false;
        }
        has_digit = true;
        port = (port * 10U) + (uint32_t)(*p - '0');
        if (port > 65535U) {
            return false;
        }
        p++;
    }
    if ((!has_digit) || (port == 0U)) {
        return false;
    }
    *port_out = (uint16_t)port;
    return true;
}

static bool parse_redirect_url(const char *url, const char *orig_host, uint16_t orig_port,
                               char *host_out, size_t host_sz, char *path_out, size_t path_sz,
                               uint16_t *port_out)
{
    if (prefix_icase(url, "http://")) {
        url += 7;
        const char *slash = strchr(url, '/');
        const char *colon = strchr(url, ':');
        if ((colon != NULL) && ((slash == NULL) || (colon < slash))) {
            if (!http_parse_port(colon + 1, port_out)) {
                return false;
            }
            size_t host_len = (size_t)(colon - url);
            if (host_len >= host_sz) {
                host_len = host_sz - 1U;
            }
            memcpy(host_out, url, host_len);
            host_out[host_len] = '\0';
        } else {
            size_t host_len = (slash != NULL) ? (size_t)(slash - url) : strlen(url);
            if (host_len >= host_sz) {
                host_len = host_sz - 1U;
            }
            memcpy(host_out, url, host_len);
            host_out[host_len] = '\0';
            *port_out = 80;
        }
        strncpy(path_out, (slash != NULL) ? slash : "/", path_sz - 1U);
        path_out[path_sz - 1U] = '\0';
        return true;
    }

    if (strstr(url, "://") != NULL) {
        return false;
    }

    if (url[0] == '/') {
        strncpy(host_out, orig_host, host_sz - 1U);
        host_out[host_sz - 1U] = '\0';
        *port_out = orig_port;
        strncpy(path_out, url, path_sz - 1U);
        path_out[path_sz - 1U] = '\0';
    } else {
        strncpy(host_out, orig_host, host_sz - 1U);
        host_out[host_sz - 1U] = '\0';
        *port_out = orig_port;
        path_out[0] = '/';
        strncpy(path_out + 1, url, path_sz - 2U);
        path_out[path_sz - 1U] = '\0';
    }
    return true;
}

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_http_client_init(SYN_HttpClient *client, const char *method, const char *host,
                                uint16_t port, const char *path, const char *content_type,
                                const uint8_t *body, size_t body_len, const SYN_HttpHeader *headers,
                                uint8_t header_count, SYN_HttpBodyCallback body_cb, void *cb_ctx,
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

static void http_close_sock(SYN_HttpClient *c)
{
    if (c->sock != SYN_SOCKET_INVALID) {
        syn_port_sock_close(c->sock);
        c->sock = SYN_SOCKET_INVALID;
    }
}

static int http_fail(SYN_HttpClient *c, SYN_Status status)
{
    http_close_sock(c);
    c->state = SYN_HTTP_STATE_ERROR;
    c->status = status;
    return -1;
}

static bool http_task_args_valid(const SYN_PT *pt, const SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return false;
    }
    const SYN_HttpClient *c = (const SYN_HttpClient *)task->user_data;
    return c->work_buf != NULL && c->work_buf_size >= 2U;
}

static bool http_sock_readable(const SYN_HttpClient *c)
{
    return c->sock != SYN_SOCKET_INVALID && syn_port_sock_readable(c->sock);
}

static bool http_header_wake_ready(const SYN_HttpClient *c)
{
    if (c->sock == SYN_SOCKET_INVALID || syn_port_sock_readable(c->sock)) {
        return true;
    }
    return (syn_port_get_tick_ms() - c->header_timeout_ms) >= HTTP_RECV_TIMEOUT_MS;
}

static bool http_body_wake_ready(const SYN_HttpClient *c)
{
    if (c->sock == SYN_SOCKET_INVALID || syn_port_sock_readable(c->sock)) {
        return true;
    }
    return (syn_port_get_tick_ms() - c->body_timeout_ms) >= HTTP_RECV_TIMEOUT_MS;
}

static bool http_connect_and_send(SYN_HttpClient *c)
{
    c->state = SYN_HTTP_STATE_CONNECTING;
    c->sock = syn_port_sock_connect_host(c->cur_host, c->cur_port);
    if (c->sock == SYN_SOCKET_INVALID) {
        (void)http_fail(c, SYN_ERROR);
        return false;
    }

    c->state = SYN_HTTP_STATE_SENDING_REQUEST;
    if (!send_request(c->sock, c->method, c->cur_host, c->cur_path, c->headers, c->header_count,
                      c->content_type, c->body_len)) {
        (void)http_fail(c, SYN_ERROR);
        return false;
    }

    if (c->body != NULL && c->body_len > 0U) {
        int sent = syn_port_sock_send_all(c->sock, c->body, c->body_len);
        if (sent != (int)c->body_len) {
            (void)http_fail(c, SYN_ERROR);
            return false;
        }
    }

    c->state = SYN_HTTP_STATE_READING_HEADERS;
    c->buf_used = 0U;
    memset(&c->resp, 0, sizeof(c->resp));
    c->header_timeout_ms = syn_port_get_tick_ms();
    return true;
}

static int http_step_headers(SYN_HttpClient *c)
{
    size_t space = (c->work_buf_size - 1U) - c->buf_used;
    bool was_readable = syn_port_sock_readable(c->sock);
    int n = syn_port_sock_recv(c->sock, c->work_buf + c->buf_used, space, 0);
    if (n > 0) {
        c->buf_used += (size_t)n;
        c->work_buf[c->buf_used] = '\0';
        c->header_timeout_ms = syn_port_get_tick_ms();

        const char *end = strstr((const char *)c->work_buf, "\r\n\r\n");
        if (end != NULL) {
            c->body_start = (size_t)(end - (const char *)c->work_buf) + 4U;
            return 1;
        }
        if (c->buf_used >= c->work_buf_size - 1U) {
            return http_fail(c, SYN_ERROR);
        }
        return http_sock_readable(c) ? 2 : 0;
    }
    if (n == 0 || was_readable) {
        return http_fail(c, SYN_ERROR);
    }
    if ((syn_port_get_tick_ms() - c->header_timeout_ms) >= HTTP_RECV_TIMEOUT_MS) {
        return http_fail(c, SYN_TIMEOUT);
    }
    return 0;
}

static const char *http_skip_spaces(const char *s)
{
    while ((*s == ' ') || (*s == '\t')) {
        s++;
    }
    return s;
}

static void http_parse_header_line(SYN_HttpResponse *resp, const char *cur)
{
    if (prefix_icase(cur, "content-length:")) {
        resp->content_length = parse_uint(http_skip_spaces(cur + 15));
        resp->has_content_length = true;
    } else if (prefix_icase(cur, "transfer-encoding:")) {
        if (prefix_icase(http_skip_spaces(cur + 18), "chunked")) {
            resp->chunked = true;
        }
    } else if (prefix_icase(cur, "connection:")) {
        if (prefix_icase(http_skip_spaces(cur + 11), "close")) {
            resp->connection_close = true;
        }
    } else if (prefix_icase(cur, "location:")) {
        const char *val = http_skip_spaces(cur + 9);
        size_t len = 0U;
        while (val[len] != '\r' && val[len] != '\n' && len < sizeof(resp->location) - 1U) {
            resp->location[len] = val[len];
            len++;
        }
        resp->location[len] = '\0';
    }
}

static bool http_status_is_bodyless(int status_code)
{
    return status_code == 204 || status_code == 304 || (status_code >= 100 && status_code < 200);
}

static bool http_apply_redirect(SYN_HttpClient *c)
{
    http_close_sock(c);
    if (++c->hops >= 5) {
        return false;
    }

    char next_host[64];
    char next_path[128];
    uint16_t next_port = c->cur_port;
    if (!parse_redirect_url(c->resp.location, c->cur_host, c->cur_port, next_host,
                            sizeof(next_host), next_path, sizeof(next_path), &next_port)) {
        return false;
    }

    if (strcmp(next_host, c->cur_host) != 0 || next_port != c->cur_port) {
        c->headers = NULL;
        c->header_count = 0U;
    }

    if (c->resp.status_code == 303 || ((c->resp.status_code == 301 || c->resp.status_code == 302) &&
                                       strcmp(c->method, "POST") == 0)) {
        c->method = "GET";
        c->body = NULL;
        c->body_len = 0U;
        c->content_type = NULL;
    }

    strncpy(c->cur_host, next_host, sizeof(c->cur_host) - 1U);
    c->cur_host[sizeof(c->cur_host) - 1U] = '\0';
    strncpy(c->cur_path, next_path, sizeof(c->cur_path) - 1U);
    c->cur_path[sizeof(c->cur_path) - 1U] = '\0';
    c->cur_port = next_port;
    return true;
}

static bool http_response_is_bodyless(const char *method, int status_code)
{
    if (prefix_icase(method, "head") && (method[4] == '\0')) {
        return true;
    }
    return http_status_is_bodyless(status_code);
}

static int http_prepare_response(SYN_HttpClient *c)
{
    const char *line = (const char *)c->work_buf;
    const char *hdr_end = (const char *)c->work_buf + c->body_start - 2U;
    const char *sp = strchr(line, ' ');
    if (sp == NULL || sp > hdr_end) {
        return http_fail(c, SYN_ERROR);
    }
    c->resp.status_code = (int)parse_uint(sp + 1);

    const char *cur = strchr(line, '\n') + 1;
    while (cur < hdr_end) {
        http_parse_header_line(&c->resp, cur);
        cur = strchr(cur, '\n') + 1;
    }

    if (c->resp.status_code >= 300 && c->resp.status_code < 400 && c->resp.location[0] != '\0') {
        if (!http_apply_redirect(c)) {
            return http_fail(c, SYN_ERROR);
        }
        return 2;
    }

    if (c->body_cb == NULL) {
        http_close_sock(c);
        c->state = SYN_HTTP_STATE_DONE;
        c->status = SYN_OK;
        return -1;
    }

    bool bodyless = http_response_is_bodyless(c->method, c->resp.status_code);
    if (bodyless) {
        c->resp.chunked = false;
    }
    c->state = SYN_HTTP_STATE_READING_BODY;
    c->buf_pos = c->body_start;
    c->body_timeout_ms = syn_port_get_tick_ms();
    c->chunk_state = 0U;
    c->chunk_line_pos = 0U;
    c->body_remaining = bodyless ? 0U : c->resp.content_length;
    c->known_length = c->resp.has_content_length || bodyless;
    return 1;
}

static int http_hex_nibble(char hc)
{
    if (hc >= '0' && hc <= '9') {
        return (int)(hc - '0');
    }
    if (hc >= 'a' && hc <= 'f') {
        return (int)(hc - 'a') + 10;
    }
    if (hc >= 'A' && hc <= 'F') {
        return (int)(hc - 'A') + 10;
    }
    return -1;
}

static bool http_parse_chunk_hex(SYN_HttpClient *c)
{
    c->chunk_remaining = 0U;
    size_t digits = 0U;
    const char *p = c->chunk_line;
    while (*p != '\0') {
        char hc = *p++;
        int nib = http_hex_nibble(hc);
        if (nib >= 0) {
            if (c->chunk_remaining > (UINT32_MAX >> 4U)) {
                return false;
            }
            c->chunk_remaining = (c->chunk_remaining << 4U) | (uint32_t)nib;
            digits++;
        } else if (hc == ';' || hc == '\r' || hc == '\n') {
            return (digits > 0U);
        } else {
            return false;
        }
    }
    return false;
}

static bool http_feed_chunk_size_byte(SYN_HttpClient *c, uint8_t ch)
{
    if (c->chunk_line_pos < sizeof(c->chunk_line) - 1U) {
        c->chunk_line[c->chunk_line_pos++] = (char)ch;
    }
    if (ch == '\n') {
        c->chunk_line[c->chunk_line_pos] = '\0';
        if (!http_parse_chunk_hex(c)) {
            return false;
        }
        c->chunk_state = (c->chunk_remaining == 0U) ? 2U : 1U;
        c->chunk_line_pos = 0U;
    }
    return true;
}

static bool http_consume_chunk_crlf(SYN_HttpClient *c, uint8_t ch)
{
    if (c->chunk_state == 1U) {
        if (ch != '\r') {
            return false;
        }
        c->chunk_state = 3U;
        return true;
    }
    if (ch != '\n') {
        return false;
    }
    c->chunk_state = 0U;
    return true;
}

static int http_feed_trailer_byte(SYN_HttpClient *c, uint8_t ch)
{
    if (c->chunk_state == 2U) {
        if (ch == '\r') {
            c->chunk_state = 4U;
            return 0;
        }
        if (ch == '\n') {
            return http_fail(c, SYN_ERROR);
        }
        c->chunk_line_pos++;
        return 0;
    }
    if (ch != '\n') {
        return http_fail(c, SYN_ERROR);
    }
    if (c->chunk_line_pos == 0U) {
        return 1;
    }
    c->chunk_line_pos = 0U;
    c->chunk_state = 2U;
    return 0;
}

static int http_consume_chunked_buf(SYN_HttpClient *c)
{
    while (c->buf_pos < c->buf_used) {
        if (c->chunk_state == 0U) {
            if (!http_feed_chunk_size_byte(c, c->work_buf[c->buf_pos++])) {
                return http_fail(c, SYN_ERROR);
            }
        } else if (c->chunk_state == 1U && c->chunk_remaining > 0U) {
            size_t avail = c->buf_used - c->buf_pos;
            size_t to_deliver = (avail > c->chunk_remaining) ? c->chunk_remaining : avail;
            if (!c->body_cb(c->work_buf + c->buf_pos, to_deliver, c->cb_ctx)) {
                return http_fail(c, SYN_ERROR);
            }
            c->buf_pos += to_deliver;
            c->chunk_remaining -= (uint32_t)to_deliver;
        } else if (c->chunk_state == 1U || c->chunk_state == 3U) {
            if (!http_consume_chunk_crlf(c, c->work_buf[c->buf_pos++])) {
                return http_fail(c, SYN_ERROR);
            }
        } else {
            int trc = http_feed_trailer_byte(c, c->work_buf[c->buf_pos++]);
            if (trc != 0) {
                return trc;
            }
        }
    }
    return 0;
}

static int http_handle_body_recv_neg(SYN_HttpClient *c, bool was_readable)
{
    if (was_readable) {
        return http_fail(c, SYN_ERROR);
    }
    if ((syn_port_get_tick_ms() - c->body_timeout_ms) >= HTTP_RECV_TIMEOUT_MS) {
        return http_fail(c, SYN_TIMEOUT);
    }
    return 0;
}

static int http_step_chunked_body(SYN_HttpClient *c)
{
    int crc = http_consume_chunked_buf(c);
    if (crc != 0) {
        return crc;
    }

    bool was_readable = syn_port_sock_readable(c->sock);
    int n = syn_port_sock_recv(c->sock, c->work_buf, c->work_buf_size, 0);
    if (n > 0) {
        c->buf_used = (size_t)n;
        c->buf_pos = 0U;
        c->body_timeout_ms = syn_port_get_tick_ms();
        return http_sock_readable(c) ? 2 : 3;
    }
    if (n == 0) {
        return http_fail(c, SYN_ERROR);
    }
    return http_handle_body_recv_neg(c, was_readable);
}

static bool http_drain_initial_body(SYN_HttpClient *c)
{
    if (c->buf_pos < c->buf_used) {
        size_t to_deliver = c->buf_used - c->buf_pos;
        if (c->known_length && to_deliver > c->body_remaining) {
            to_deliver = c->body_remaining;
        }
        if (to_deliver > 0U && !c->body_cb(c->work_buf + c->buf_pos, to_deliver, c->cb_ctx)) {
            return false;
        }
        if (c->known_length) {
            c->body_remaining -= (uint32_t)to_deliver;
        }
        c->buf_pos = c->buf_used;
    }
    return true;
}

static int http_step_stream_body(SYN_HttpClient *c)
{
    if (!http_drain_initial_body(c)) {
        return http_fail(c, SYN_ERROR);
    }
    if (c->known_length && c->body_remaining == 0U) {
        return 1;
    }

    size_t to_read = (c->known_length && c->work_buf_size > c->body_remaining) ? c->body_remaining
                                                                               : c->work_buf_size;
    bool was_readable = syn_port_sock_readable(c->sock);
    int n = syn_port_sock_recv(c->sock, c->work_buf, to_read, 0);
    if (n > 0) {
        if (!c->body_cb(c->work_buf, (size_t)n, c->cb_ctx)) {
            return http_fail(c, SYN_ERROR);
        }
        c->body_timeout_ms = syn_port_get_tick_ms();
        if (c->known_length) {
            c->body_remaining -= (uint32_t)n;
            if (c->body_remaining == 0U) {
                return 1;
            }
        }
        return http_sock_readable(c) ? 2 : 3;
    }
    if (n == 0) {
        return c->known_length ? http_fail(c, SYN_ERROR) : 1;
    }
    return http_handle_body_recv_neg(c, was_readable);
}

static int http_step_body(SYN_HttpClient *c)
{
    return c->resp.chunked ? http_step_chunked_body(c) : http_step_stream_body(c);
}

SYN_PT_Status syn_http_client_task(SYN_PT *pt, SYN_Task *task)
{
    if (!http_task_args_valid(pt, task)) {
        return PT_EXITED;
    }
    SYN_HttpClient *c = (SYN_HttpClient *)task->user_data;

    PT_BEGIN(pt);

    http_close_sock(c);

    for (;;) {
        if (!http_connect_and_send(c)) {
            PT_EXIT(pt);
        }

        int hrc;
        while ((hrc = http_step_headers(c)) != 1) {
            if (hrc < 0) {
                PT_EXIT(pt);
            }
            if (hrc == 0) {
                PT_WAIT_UNTIL(pt, http_header_wake_ready(c));
            } else {
                PT_CONTINUE_IF_NEEDED(pt, task);
            }
        }

        int rrc = http_prepare_response(c);
        if (rrc < 0) {
            PT_EXIT(pt);
        }
        if (rrc == 2) {
            continue;
        }

        int brc;
        while ((brc = http_step_body(c)) != 1) {
            if (brc < 0) {
                PT_EXIT(pt);
            }
            if (brc == 0) {
                PT_WAIT_UNTIL(pt, http_body_wake_ready(c));
            } else {
                PT_CONTINUE_IF_NEEDED(pt, task);
            }
        }

        http_close_sock(c);
        c->state = SYN_HTTP_STATE_DONE;
        c->status = SYN_OK;
        break;
    }

    PT_END(pt);
}

#endif /* SYN_USE_HTTP */
```


