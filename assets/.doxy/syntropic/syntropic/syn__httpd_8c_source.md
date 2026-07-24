

# File syn\_httpd.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_httpd.c**](syn__httpd_8c.md)

[Go to the documentation of this file](syn__httpd_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_HTTPD) || SYN_USE_HTTPD

#include "syn_httpd.h"
#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Constants ─────────────────────────────────────────────────────────── */

#define HTTPD_CLIENT_TIMEOUT_MS  5000

/* ── Internal helpers ──────────────────────────────────────────────────── */

#include "../util/syn_fmt.h"

static inline bool prefix_icase(const char *str, const char *prefix)
{
    return syn_str_prefix_icase(str, prefix);
}

static bool sock_write(SYN_Socket sock, const char *str)
{
    size_t len = strlen(str);
    return syn_port_sock_send_all(sock, str, len) == (int)len;
}

static SYN_HttpMethod parse_method(const char *str, size_t len)
{
    if (len == 3 && memcmp(str, "GET", 3) == 0) return SYN_HTTP_GET;
    if (len == 4 && memcmp(str, "POST", 4) == 0) return SYN_HTTP_POST;
    if (len == 3 && memcmp(str, "PUT", 3) == 0) return SYN_HTTP_PUT;
    if (len == 6 && memcmp(str, "DELETE", 6) == 0) return SYN_HTTP_DELETE;
    return SYN_HTTP_GET; /* default fallback */
}

static inline uint32_t parse_uint(const char *s)
{
    return syn_fmt_parse_uint(s);
}

static int parse_headers_from_buf(SYN_Socket sock, SYN_HttpdRequest *req,
                                   uint8_t *buf, size_t total)
{
    memset(req, 0, sizeof(*req));
    req->client_sock = sock;

    buf[total] = '\0';

    char *end_of_headers = strstr((const char *)buf, "\r\n\r\n");
    if (end_of_headers == NULL) return -1;

    /* Parse request line: "GET /path?query HTTP/1.1\r\n" */
    char *line = (char *)buf;
    char *sp1 = strchr(line, ' ');
    if (sp1 == NULL) return -1;

    req->method = parse_method(line, (size_t)(sp1 - line));

    char *path_start = sp1 + 1;
    char *sp2 = strchr(path_start, ' ');
    if (sp2 == NULL) return -1;
    *sp2 = '\0'; /* null-terminate the path+query */

    /* Split path and query */
    char *qmark = strchr(path_start, '?');
    if (qmark != NULL) {
        *qmark = '\0';
        req->query = qmark + 1;
    }
    req->path = path_start;

    /* Parse headers */
    char *hdr_start = strstr(sp2 + 1, "\r\n");
    if (hdr_start) {
        hdr_start += 2;
        req->headers = hdr_start;
    }

    while (hdr_start && *hdr_start != '\r' && *hdr_start != '\n') {
        char *next_line = strstr(hdr_start, "\r\n");

        if (prefix_icase(hdr_start, "content-length:")) {
            const char *val = hdr_start + 15;
            while (*val == ' ') val++;
            req->content_length = parse_uint(val);
        } else if (prefix_icase(hdr_start, "content-type:")) {
            /* cppcheck-suppress constVariablePointer */
            char *val = hdr_start + 13;
            while (*val == ' ') val++;
            req->content_type = val;
            /* Null-terminate Content-Type value at end of line */
            if (next_line) *next_line = '\0';
        }

        if (!next_line) break;
        hdr_start = next_line + 2;
    }

    /* Calculate buffered body */
    if (end_of_headers) {
        size_t header_len = (size_t)(end_of_headers + 4 - (char *)buf);
        if (total > header_len) {
            req->body_buffered_offset = header_len;
            req->body_buffered_len = total - header_len;
        }
    }

    return 0;
}

static const SYN_HttpdRoute *match_route(const SYN_Httpd *srv,
                                           const SYN_HttpdRequest *req)
{
    for (size_t i = 0; i < srv->route_count; i++) {
        const SYN_HttpdRoute *r = &srv->routes[i];

        if (r->method != req->method) continue;

        size_t plen = strlen(r->path);
        if (plen > 0 && r->path[plen - 1] == '*') {
            /* Prefix match: "/api/" with wildcard */
            if (strncmp(req->path, r->path, plen - 1) == 0) {
                return r;
            }
        } else {
            /* Exact match */
            if (strcmp(req->path, r->path) == 0) {
                return r;
            }
        }
    }
    return NULL;
}

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

static void drop_client(SYN_Httpd *srv)
{
    if (srv->client != SYN_SOCKET_INVALID) {
        syn_port_sock_close(srv->client);
        srv->client = SYN_SOCKET_INVALID;
    }
    srv->state    = SYN_HTTPD_IDLE;
    srv->rx_total = 0;
}

/* ── Server API ────────────────────────────────────────────────────────── */

SYN_Status syn_httpd_init(SYN_Httpd *srv, uint16_t port,
                           const SYN_HttpdRoute *routes, size_t route_count,
                           uint8_t *work_buf, size_t work_buf_size)
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
    srv->state  = SYN_HTTPD_IDLE;

    srv->listener = syn_port_sock_listen(port, 2);
    if (srv->listener == SYN_SOCKET_INVALID) {
        return SYN_ERROR;
    }

    srv->running = true;
    return SYN_OK;
}

static void dispatch_request(SYN_Httpd *srv)
{
    /* Parse request from buffered data */
    SYN_HttpdRequest req;
    if (parse_headers_from_buf(srv->client, &req,
                                srv->work_buf, srv->rx_total) != 0) {
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
        srv->client   = SYN_SOCKET_INVALID;
        srv->state    = SYN_HTTPD_IDLE;
        srv->rx_total = 0;
    }
}

SYN_Status syn_httpd_step(SYN_Httpd *srv)
{
    SYN_ASSERT(srv != NULL);
    if (!srv->running) return SYN_ERROR;

    switch (srv->state) {

    /* ── IDLE: poll for new client (non-blocking) ──────────────────── */
    case SYN_HTTPD_IDLE: {
        SYN_Socket client = syn_port_sock_accept(srv->listener, 0);
        if (client == SYN_SOCKET_INVALID) {
            return SYN_TIMEOUT; /* No client — caller should yield */
        }
        /* Client connected — start reading headers */
        srv->client        = client;
        srv->rx_total      = 0;
        srv->recv_deadline = syn_port_get_tick_ms() + HTTPD_CLIENT_TIMEOUT_MS;
        srv->state         = SYN_HTTPD_READING_HEADERS;
        /* Fall through — attempt first recv in the same tick */
    }
    /* fallthrough */

    /* ── READING_HEADERS: non-blocking recv, accumulate in work_buf ─ */
    case SYN_HTTPD_READING_HEADERS: {
        /* Check deadline — drop stale connections without blocking */
        if ((int32_t)(syn_port_get_tick_ms() - srv->recv_deadline) >= 0) {
            send_error(srv->client, 408, "Request Timeout");
            drop_client(srv);
            return SYN_TIMEOUT;
        }

        /* Non-blocking recv (timeout=0) */
        size_t space = srv->work_buf_size - 1 - srv->rx_total;
        if (space == 0) {
            /* Buffer full without finding \r\n\r\n — reject */
            send_error(srv->client, 413, "Request Too Large");
            drop_client(srv);
            return SYN_ERROR;
        }

        int n = syn_port_sock_recv(srv->client,
                                    srv->work_buf + srv->rx_total,
                                    space, 0);
        if (n < 0) {
            /* No data available this tick — yield and retry next tick */
            return SYN_TIMEOUT;
        }
        if (n == 0) {
            /* Connection closed before headers were complete */
            send_error(srv->client, 400, "Bad Request");
            drop_client(srv);
            return SYN_ERROR;
        }


        srv->rx_total += (size_t)n;
        srv->work_buf[srv->rx_total] = '\0';

        /* Check for complete headers */
        if (strstr((const char *)srv->work_buf, "\r\n\r\n") != NULL) {
            srv->state = SYN_HTTPD_DISPATCHING;
            /* Fall through to dispatch immediately */
            dispatch_request(srv);
            return SYN_OK;
        }

        /* Headers incomplete — yield, try again next tick */
        return SYN_TIMEOUT;
    }

    /* ── DISPATCHING: parse + route + respond ──────────────────────── */
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

void syn_httpd_header(const SYN_HttpdResponse *resp,
                       const char *name, const char *value)
{
    SYN_ASSERT(resp != NULL);
    SYN_ASSERT(!resp->headers_sent);

    sock_write(resp->sock, name);
    sock_write(resp->sock, ": ");
    sock_write(resp->sock, value);
    sock_write(resp->sock, "\r\n");
}

static void finalize_headers(SYN_HttpdResponse *resp)
{
    if (!resp) return;
    if (!resp->headers_sent) {
        sock_write(resp->sock, "\r\n");
        resp->headers_sent = true;
    }
}

void syn_httpd_body(SYN_HttpdResponse *resp, const void *data, size_t len)
{
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

int syn_httpd_read_body(const SYN_HttpdRequest *req,
                        const SYN_HttpdResponse *resp,
                        void *buf, size_t max_len)
{
    SYN_ASSERT(req != NULL);
    SYN_ASSERT(resp != NULL);

    if (req->content_length == 0) return 0;

    SYN_HttpdRequest *rw = (SYN_HttpdRequest *)(uintptr_t)req;

    size_t remaining = req->content_length - rw->body_consumed;
    if (remaining == 0) return 0;

    size_t to_read = max_len;
    if (to_read > remaining) to_read = remaining;

    if (rw->body_buffered_len > 0) {
        size_t consume = rw->body_buffered_len;
        if (consume > to_read) consume = to_read;
        memcpy(buf, resp->buf + rw->body_buffered_offset, consume);
        rw->body_buffered_offset += consume;
        rw->body_buffered_len -= consume;
        rw->body_consumed += consume;
        return (int)consume;
    }

    /* Non-blocking read — timeout=0, never stalls the scheduler */
    int n = syn_port_sock_recv(resp->sock, buf, to_read, 0);
    if (n > 0) {
        rw->body_consumed += (size_t)n;
    }
    return n;
}

/* ── Protothread task ──────────────────────────────────────────────────── */

SYN_PT_Status syn_httpd_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_Httpd *srv = (SYN_Httpd *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        /* Non-blocking step — always returns immediately */
        if (srv->running) {
            syn_httpd_step(srv);
        }
        PT_YIELD(pt);
    }

    PT_END(pt);
}

#endif /* SYN_USE_HTTPD */
```


