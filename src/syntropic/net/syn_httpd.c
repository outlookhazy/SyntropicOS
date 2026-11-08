#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_HTTPD) || SYN_USE_HTTPD

/**
 * @file syn_httpd.c
 * @brief Minimal HTTP/1.1 server — fully non-blocking implementation.
 *
 * Designed around the cooperative scheduler: every call to syn_httpd_step()
 * does a bounded amount of work and returns immediately. Socket I/O uses
 * timeout=0 exclusively. Stale connections are timed out via the tick clock.
 */

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "syn_httpd.h"

#include <string.h>

/* ── Constants ─────────────────────────────────────────────────────────── */

/**
 * Maximum time (ms) to wait for a client to send complete headers after
 * connecting. Enforced non-blockingly via the tick clock — not passed to
 * any socket recv call.
 */
#define HTTPD_CLIENT_TIMEOUT_MS 5000

/* ── Internal helpers ──────────────────────────────────────────────────── */

#include "../util/syn_fmt.h"

/**
 * @brief Case-insensitive string prefix check.
 * @param str Input string.
 * @param prefix Prefix to test.
 * @return True if str starts with prefix (case-insensitive).
 */
static inline bool prefix_icase(const char *str, const char *prefix)
{
    return syn_str_prefix_icase(str, prefix);
}

/**
 * @brief Write a null-terminated string to the socket.
 * @param sock  Socket to write to.
 * @param str   String to send.
 * @return true if all bytes were sent.
 */
static bool sock_write(SYN_Socket sock, const char *str)
{
    size_t len = strlen(str);
    return syn_port_sock_send_all(sock, str, len) == (int)len;
}

/**
 * @brief Parse the method string into enum.
 * @param str  Method string (e.g. "GET").
 * @param len  Length of the method string.
 * @return Corresponding SYN_HttpMethod.
 */
static SYN_HttpMethod parse_method(const char *str, size_t len)
{
    if (len == 3 && memcmp(str, "GET", 3) == 0)
        return SYN_HTTP_GET;
    if (len == 4 && memcmp(str, "POST", 4) == 0)
        return SYN_HTTP_POST;
    if (len == 3 && memcmp(str, "PUT", 3) == 0)
        return SYN_HTTP_PUT;
    if (len == 6 && memcmp(str, "DELETE", 6) == 0)
        return SYN_HTTP_DELETE;
    return (SYN_HttpMethod)0xFF; /* Unrecognised — will produce 404/405 */
}

/**
 * @brief Parse unsigned integer from string.
 * @param s Input string.
 * @return Parsed uint32_t value.
 */
static inline uint32_t parse_uint(const char *s)
{
    return syn_fmt_parse_uint(s);
}

/**
 * @brief Advance past leading optional whitespace (SP or HTAB) in a header value.
 * @param s Pointer after header colon.
 * @return Pointer to first non-OWS character.
 */
static const char *httpd_skip_ows(const char *s)
{
    while ((*s == ' ') || (*s == '\t')) {
        s++;
    }
    return s;
}

/**
 * @brief Parse a single HTTP header line into request metadata without mutating headers.
 * @param req       Target parsed request structure.
 * @param hdr_start Start of header line.
 * @param next_line Pointer to terminating CRLF of header line.
 */
static void httpd_parse_header_line(SYN_HttpdRequest *req, const char *hdr_start,
                                    const char *next_line)
{
    if (prefix_icase(hdr_start, "content-length:")) {
        req->content_length = parse_uint(httpd_skip_ows(hdr_start + 15));
    } else if (prefix_icase(hdr_start, "content-type:")) {
        const char *val = httpd_skip_ows(hdr_start + 13);
        size_t ct_len = (size_t)(next_line - val);
        if (ct_len >= sizeof(req->content_type_buf)) {
            ct_len = sizeof(req->content_type_buf) - 1U;
        }
        memcpy(req->content_type_buf, val, ct_len);
        req->content_type_buf[ct_len] = '\0';
        req->content_type = req->content_type_buf;
    }
}

/**
 * @brief Parse request line + headers from already-buffered data.
 *
 * This is a pure parsing function — no socket I/O. The caller guarantees
 * that buf contains a complete header block ending with "\r\n\r\n".
 *
 * @param sock      Client socket (stored in req for body reads).
 * @param req       [out] Parsed request.
 * @param buf       Work buffer containing the raw request.
 * @param total     Number of valid bytes in buf.
 * @return 0 on success, -1 on parse error.
 */
static int parse_headers_from_buf(SYN_Socket sock, SYN_HttpdRequest *req, uint8_t *buf,
                                  size_t total)
{
    memset(req, 0, sizeof(*req));
    req->client_sock = sock;

    buf[total] = '\0';

    char *end_of_headers = strstr((const char *)buf, "\r\n\r\n");
    if (end_of_headers == NULL) {
        return -1;
    }

    /* Parse request line: "GET /path?query HTTP/1.1\r\n" */
    char *line = (char *)buf;
    char *line_end = strstr(line, "\r\n");
    char *sp1 = strchr(line, ' ');
    if (sp1 == NULL || sp1 >= line_end) {
        return -1;
    }

    req->method = parse_method(line, (size_t)(sp1 - line));

    char *path_start = sp1 + 1;
    char *sp2 = strchr(path_start, ' ');
    if (sp2 == NULL || sp2 >= line_end) {
        return -1;
    }
    *sp2 = '\0'; /* null-terminate the path+query */

    /* Split path and query */
    char *qmark = strchr(path_start, '?');
    if (qmark != NULL) {
        *qmark = '\0';
        req->query = qmark + 1;
    }
    req->path = path_start;

    /* Parse headers without mutating header lines in work_buf */
    char *hdr_start = line_end + 2;
    req->headers = hdr_start;

    while (hdr_start < end_of_headers) {
        char *next_line = strstr(hdr_start, "\r\n");
        httpd_parse_header_line(req, hdr_start, next_line);
        hdr_start = next_line + 2;
    }

    /* Calculate buffered body */
    size_t header_len = (size_t)(end_of_headers + 4 - (char *)buf);
    if (total > header_len) {
        req->body_buffered_offset = header_len;
        req->body_buffered_len = total - header_len;
    }

    return 0;
}

/**
 * @brief Match a route against the request.
 * @param srv  Server instance.
 * @param req  Parsed request.
 * @return Matching route, or NULL.
 */
static const SYN_HttpdRoute *match_route(const SYN_Httpd *srv, const SYN_HttpdRequest *req)
{
    for (size_t i = 0; i < srv->route_count; i++) {
        const SYN_HttpdRoute *r = &srv->routes[i];

        if (r->method != req->method) {
            continue;
        }

        size_t plen = strlen(r->path);
        if (plen > 0U && r->path[plen - 1U] == '*') {
            if (strncmp(req->path, r->path, plen - 1U) == 0) {
                return r;
            }
        } else if (strcmp(req->path, r->path) == 0) {
            return r;
        }
    }
    return NULL;
}

/**
 * @brief Send a simple error response.
 * @param sock    Client socket.
 * @param code    HTTP status code.
 * @param reason  Reason phrase (e.g. "Not Found").
 */
static void send_error(SYN_Socket sock, int code, const char *reason)
{
    sock_write(sock, "HTTP/1.1 ");
    /* Simple int-to-string for status code */
    char code_str[4];
    code_str[0] = (char)('0' + (code / 100));
    code_str[1] = (char)('0' + ((code / 10) % 10));
    code_str[2] = (char)('0' + (code % 10));
    code_str[3] = '\0';
    sock_write(sock, code_str);
    sock_write(sock, " ");
    sock_write(sock, reason);
    sock_write(sock, "\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
}

/**
 * @brief Drop the active client and reset to IDLE.
 * @param srv  Server instance.
 */
static void drop_client(SYN_Httpd *srv)
{
    if (srv->client != SYN_SOCKET_INVALID) {
        syn_port_sock_close(srv->client);
        srv->client = SYN_SOCKET_INVALID;
    }
    srv->state = SYN_HTTPD_IDLE;
    srv->rx_total = 0U;
    srv->expected_total = 0U;
}

/* ── Server API ────────────────────────────────────────────────────────── */

SYN_Status syn_httpd_init(SYN_Httpd *srv, uint16_t port, const SYN_HttpdRoute *routes,
                          size_t route_count, uint8_t *work_buf, size_t work_buf_size)
{
    SYN_ASSERT(srv != NULL);
    SYN_ASSERT(routes != NULL || route_count == 0);
    SYN_ASSERT(work_buf != NULL);
    SYN_ASSERT(work_buf_size >= 256);

    memset(srv, 0, sizeof(*srv));
    srv->routes = routes;
    srv->route_count = route_count;
    srv->work_buf = work_buf;
    srv->work_buf_size = work_buf_size;
    srv->port = port;
    srv->client = SYN_SOCKET_INVALID;
    srv->state = SYN_HTTPD_IDLE;

    srv->listener = syn_port_sock_listen(port, 2);
    if (srv->listener == SYN_SOCKET_INVALID) {
        return SYN_ERROR;
    }

    srv->running = true;
    return SYN_OK;
}

/**
 * @brief Handle the DISPATCHING phase — parse, route, respond, close.
 *
 * Called once when headers are fully buffered. Runs synchronously but
 * all data is already in work_buf — no blocking socket reads needed
 * for headers.
 *
 * @param srv  Server instance with complete headers in work_buf.
 */
static void dispatch_request(SYN_Httpd *srv)
{
    /* Parse request from buffered data */
    SYN_HttpdRequest req;
    if (parse_headers_from_buf(srv->client, &req, srv->work_buf, srv->rx_total) != 0) {
        send_error(srv->client, 400, "Bad Request");
        drop_client(srv);
        return;
    }

    /* Match route */
    const SYN_HttpdRoute *route = match_route(srv, &req);
    if (route == NULL) {
        send_error(srv->client, 404, "Not Found");
        drop_client(srv);
        return;
    }

    /* Dispatch to handler */
    SYN_HttpdResponse resp;
    resp.sock = srv->client;
    resp.buf = srv->work_buf;
    resp.buf_size = srv->work_buf_size;
    resp.body_consumed = 0U;
    resp.headers_sent = false;
    resp.upgraded = false;

    route->handler(&req, &resp, route->ctx);

    /* If handler didn't send anything, send 204 */
    if (!resp.headers_sent && !resp.upgraded) {
        send_error(srv->client, 204, "No Content");
    }

    if (!resp.upgraded) {
        drop_client(srv);
    } else {
        /* Upgraded connection (e.g. WebSocket) — don't close, just reset state */
        srv->client = SYN_SOCKET_INVALID;
        srv->state = SYN_HTTPD_IDLE;
        srv->rx_total = 0U;
        srv->expected_total = 0U;
    }
}

/**
 * @brief Inspect buffered headers without mutating work_buf to compute expected_total.
 * @param srv Server instance with NUL-terminated work_buf.
 * @return true if header terminator found, false otherwise.
 */
static bool httpd_check_headers_complete(SYN_Httpd *srv)
{
    const char *buf = (const char *)srv->work_buf;
    const char *eoh = strstr(buf, "\r\n\r\n");
    if (eoh == NULL) {
        return false;
    }

    size_t hdr_len = (size_t)(eoh + 4 - buf);
    uint32_t clen = 0U;
    const char *line_end = strstr(buf, "\r\n");
    const char *hdr = line_end + 2;
    while (hdr < eoh) {
        const char *next = strstr(hdr, "\r\n");
        if (prefix_icase(hdr, "content-length:")) {
            clen = parse_uint(httpd_skip_ows(hdr + 15));
        }
        hdr = next + 2;
    }

    size_t max_body_in_buf = (srv->work_buf_size - 1U) - hdr_len;
    size_t buf_body = ((size_t)clen <= max_body_in_buf) ? (size_t)clen : 0U;
    srv->expected_total = hdr_len + buf_body;
    return true;
}

/**
 * @brief Non-blocking header and Content-Length body accumulation phase for an accepted client.
 * @param srv Server instance in SYN_HTTPD_READING_HEADERS or SYN_HTTPD_READING_BODY state.
 * @return SYN_OK on dispatch, SYN_TIMEOUT if awaiting bytes/expired, SYN_ERROR on failure.
 */
static SYN_Status httpd_step_reading(SYN_Httpd *srv)
{
    /* Check deadline — drop stale connections without blocking */
    if ((int32_t)(syn_port_get_tick_ms() - srv->recv_deadline) >= 0) {
        send_error(srv->client, 408, "Request Timeout");
        drop_client(srv);
        return SYN_TIMEOUT;
    }

    /* Non-blocking recv (timeout=0) */
    size_t space = srv->work_buf_size - 1U - srv->rx_total;
    if (space == 0U) {
        send_error(srv->client, 413, "Request Too Large");
        drop_client(srv);
        return SYN_ERROR;
    }

    bool was_readable = syn_port_sock_readable(srv->client);
    int n = syn_port_sock_recv(srv->client, srv->work_buf + srv->rx_total, space, 0);
    if (n < 0) {
        if (was_readable) {
            drop_client(srv);
            return SYN_ERROR;
        }
        return SYN_TIMEOUT;
    }
    if (n == 0) {
        send_error(srv->client, 400, "Bad Request");
        drop_client(srv);
        return SYN_ERROR;
    }

    srv->rx_total += (size_t)n;
    srv->work_buf[srv->rx_total] = '\0';

    if (srv->state == SYN_HTTPD_READING_HEADERS) {
        if (!httpd_check_headers_complete(srv)) {
            return SYN_TIMEOUT;
        }
        srv->state = SYN_HTTPD_READING_BODY;
    }

    if (srv->rx_total >= srv->expected_total) {
        srv->state = SYN_HTTPD_DISPATCHING;
        dispatch_request(srv);
        return SYN_OK;
    }

    return SYN_TIMEOUT;
}

SYN_Status syn_httpd_step(SYN_Httpd *srv)
{
    SYN_ASSERT(srv != NULL);
    if (!srv->running) {
        return SYN_ERROR;
    }

    switch (srv->state) {
    case SYN_HTTPD_IDLE: {
        SYN_Socket client = syn_port_sock_accept(srv->listener, 0);
        if (client == SYN_SOCKET_INVALID) {
            return SYN_TIMEOUT;
        }
        srv->client = client;
        srv->rx_total = 0U;
        srv->expected_total = 0U;
        srv->recv_deadline = syn_port_get_tick_ms() + HTTPD_CLIENT_TIMEOUT_MS;
        srv->state = SYN_HTTPD_READING_HEADERS;
        return httpd_step_reading(srv);
    }

    case SYN_HTTPD_READING_HEADERS:
    case SYN_HTTPD_READING_BODY:
        return httpd_step_reading(srv);

    case SYN_HTTPD_DISPATCHING:
        dispatch_request(srv);
        return SYN_OK;

    default:
        srv->state = SYN_HTTPD_IDLE;
        return SYN_ERROR;
    }
}

void syn_httpd_stop(SYN_Httpd *srv)
{
    SYN_ASSERT(srv != NULL);
    /* Drop any in-progress client */
    drop_client(srv);
    if (srv->listener != SYN_SOCKET_INVALID) {
        syn_port_sock_close(srv->listener);
        srv->listener = SYN_SOCKET_INVALID;
    }
    srv->running = false;
}

/* ── Response helpers ──────────────────────────────────────────────────── */

void syn_httpd_status(const SYN_HttpdResponse *resp, int code, const char *reason)
{
    if (resp == NULL || reason == NULL) {
        return;
    }
    SYN_ASSERT(resp != NULL);
    SYN_ASSERT(!resp->headers_sent);

    char code_str[4];
    code_str[0] = (char)('0' + (code / 100));
    code_str[1] = (char)('0' + ((code / 10) % 10));
    code_str[2] = (char)('0' + (code % 10));
    code_str[3] = '\0';

    sock_write(resp->sock, "HTTP/1.1 ");
    sock_write(resp->sock, code_str);
    sock_write(resp->sock, " ");
    sock_write(resp->sock, reason);
    sock_write(resp->sock, "\r\n");
    sock_write(resp->sock, "Connection: close\r\n");
}

void syn_httpd_header(const SYN_HttpdResponse *resp, const char *name, const char *value)
{
    if (resp == NULL || name == NULL || value == NULL) {
        return;
    }
    SYN_ASSERT(resp != NULL);
    SYN_ASSERT(!resp->headers_sent);

    sock_write(resp->sock, name);
    sock_write(resp->sock, ": ");
    sock_write(resp->sock, value);
    sock_write(resp->sock, "\r\n");
}

/**
 * @brief Flush the header block — sends the blank line after headers.
 * @param resp  Response context.
 */
static void finalize_headers(SYN_HttpdResponse *resp)
{
    if (!resp->headers_sent) {
        sock_write(resp->sock, "\r\n");
        resp->headers_sent = true;
    }
}

void syn_httpd_body(SYN_HttpdResponse *resp, const void *data, size_t len)
{
    if (resp == NULL) {
        return;
    }
    SYN_ASSERT(resp != NULL);
    finalize_headers(resp);
    if (data != NULL && len > 0) {
        syn_port_sock_send_all(resp->sock, data, len);
    }
}

void syn_httpd_body_str(SYN_HttpdResponse *resp, const char *str)
{
    SYN_ASSERT(resp != NULL);
    SYN_ASSERT(str != NULL);
    syn_httpd_body(resp, str, strlen(str));
}

int syn_httpd_read_body(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *buf,
                        size_t max_len)
{
    SYN_ASSERT(req != NULL);
    SYN_ASSERT(resp != NULL);
    if ((buf == NULL) && (max_len > 0U)) {
        return -1;
    }
    if (max_len == 0U) {
        return 0;
    }

    size_t consumed =
        (resp->body_consumed >= req->body_consumed) ? resp->body_consumed : req->body_consumed;
    if (req->content_length == 0U || consumed >= (size_t)req->content_length) {
        return 0;
    }

    size_t remaining = (size_t)req->content_length - consumed;
    size_t to_read = (max_len < remaining) ? max_len : remaining;

    if (consumed < req->body_buffered_len) {
        size_t buf_avail = req->body_buffered_len - consumed;
        size_t consume = (buf_avail < to_read) ? buf_avail : to_read;
        memcpy(buf, resp->buf + req->body_buffered_offset + consumed, consume);
        resp->body_consumed = consumed + consume;
        return (int)consume;
    }

    /* Non-blocking read — timeout=0, never stalls the scheduler */
    int n = syn_port_sock_recv(resp->sock, buf, to_read, 0);
    if (n > 0) {
        resp->body_consumed = consumed + (size_t)n;
    }
    return n;
}

/**
 * @brief Check if HTTP server task has actionable work.
 * @param srv Pointer to HTTP server instance.
 * @return true if work pending, false otherwise.
 */
static bool httpd_has_work(const SYN_Httpd *srv)
{
    if (srv->state == SYN_HTTPD_IDLE) {
        return srv->listener != SYN_SOCKET_INVALID && syn_port_sock_readable(srv->listener);
    }
    if (srv->state == SYN_HTTPD_READING_HEADERS || srv->state == SYN_HTTPD_READING_BODY) {
        if (srv->client != SYN_SOCKET_INVALID && syn_port_sock_readable(srv->client)) {
            return true;
        }
        return (int32_t)(syn_port_get_tick_ms() - srv->recv_deadline) >= 0;
    }
    return true;
}

/**
 * @brief Run one HTTP server step and report whether state or RX bytes progressed.
 * @param srv Server instance.
 * @return true if progress occurred, false otherwise.
 */
static bool httpd_step_progressed(SYN_Httpd *srv)
{
    SYN_HttpdState prev_state = srv->state;
    size_t prev_rx = srv->rx_total;
    SYN_Status st = syn_httpd_step(srv);
    return (st == SYN_OK) || (srv->state != prev_state) || (srv->rx_total != prev_rx);
}

/* ── Protothread task ──────────────────────────────────────────────────── */

SYN_PT_Status syn_httpd_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }
    SYN_Httpd *srv = (SYN_Httpd *)task->user_data;

    PT_BEGIN(pt);

    while (srv->running) {
        PT_WAIT_UNTIL(pt, !srv->running || httpd_has_work(srv));
        if (!srv->running) {
            break;
        }
        if (!httpd_step_progressed(srv)) {
            PT_PASS(pt);
        } else if (srv->running && httpd_has_work(srv)) {
            PT_CONTINUE(pt);
        }
    }

    PT_END(pt);
}

bool syn_httpd_get_query_param(const char *query, const char *key, char *val_buf, size_t max_len)
{
    if (query == NULL || key == NULL || val_buf == NULL || max_len == 0) {
        return false;
    }

    size_t key_len = strlen(key);
    const char *p = query;

    while (*p != '\0') {
        /* Check if key matches at current position */
        if (strncmp(p, key, key_len) == 0 && p[key_len] == '=') {
            const char *val_start = p + key_len + 1;
            const char *val_end = strchr(val_start, '&');
            size_t val_len = val_end ? (size_t)(val_end - val_start) : strlen(val_start);

            if (val_len >= max_len) {
                return false; /* Buffer capacity exceeded */
            }

            memcpy(val_buf, val_start, val_len);
            val_buf[val_len] = '\0';
            return true;
        }

        /* Move to next param after '&' */
        p = strchr(p, '&');
        if (p == NULL) {
            break;
        }
        p++; /* Skip '&' */
    }

    return false;
}

#endif /* SYN_USE_HTTPD */
