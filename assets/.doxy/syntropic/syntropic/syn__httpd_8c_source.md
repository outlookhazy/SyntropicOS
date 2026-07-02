

# File syn\_httpd.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_httpd.c**](syn__httpd_8c.md)

[Go to the documentation of this file](syn__httpd_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_HTTPD) || SYN_USE_HTTPD

#include "syn_httpd.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Constants ─────────────────────────────────────────────────────────── */

#define HTTPD_ACCEPT_TIMEOUT_MS  100  
#define HTTPD_RECV_TIMEOUT_MS    5000  
/* ── Internal helpers ──────────────────────────────────────────────────── */

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

static uint32_t parse_uint(const char *s)
{
    uint32_t val = 0;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (uint32_t)(*s - '0');
        s++;
    }
    return val;
}

static int parse_request(SYN_Socket sock, SYN_HttpdRequest *req,
                          uint8_t *buf, size_t buf_size)
{
    size_t total = 0;
    bool headers_done = false;
    char *end_of_headers = NULL;

    memset(req, 0, sizeof(*req));
    req->client_sock = sock;

    /* Read until \r\n\r\n */
    while (!headers_done && total < buf_size - 1) {
        int n = syn_port_sock_recv(sock, buf + total,
                                    buf_size - 1 - total,
                                    HTTPD_RECV_TIMEOUT_MS);
        if (n <= 0) return -1;
        total += (size_t)n;
        buf[total] = '\0';

        end_of_headers = strstr((const char *)buf, "\r\n\r\n");
        if (end_of_headers != NULL) {
            headers_done = true;
        }
    }

    if (!headers_done) return -1;

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

    while (hdr_start && *hdr_start != '\r') {
        if (prefix_icase(hdr_start, "content-length:")) {
            const char *val = hdr_start + 15;
            while (*val == ' ') val++;
            req->content_length = parse_uint(val);
        } else if (prefix_icase(hdr_start, "content-type:")) {
            char *val = hdr_start + 13;
            while (*val == ' ') val++;
            req->content_type = val;
            /* Null-terminate at \r */
            char *cr = strchr(val, '\r');
            if (cr) *cr = '\0';
        }

        hdr_start = strstr(hdr_start, "\r\n");
        if (hdr_start) hdr_start += 2;
    }

    /* Calculate buffered body — use the pointer captured before any
     * null-termination (path/header parsing inserts '\0' into buf,
     * which would break a strstr re-scan). */
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

    srv->listener = syn_port_sock_listen(port, 2);
    if (srv->listener == SYN_SOCKET_INVALID) {
        return SYN_ERROR;
    }

    srv->running = true;
    return SYN_OK;
}


static void handle_client(SYN_Httpd *srv, SYN_Socket client)
{
    /* Parse request */
    SYN_HttpdRequest req;
    if (parse_request(client, &req, srv->work_buf, srv->work_buf_size) != 0) {
        send_error(client, 400, "Bad Request");
        syn_port_sock_close(client);
        return;
    }

    /* Match route */
    const SYN_HttpdRoute *route = match_route(srv, &req);
    if (route == NULL) {
        send_error(client, 404, "Not Found");
        syn_port_sock_close(client);
        return;
    }

    /* Dispatch to handler */
    SYN_HttpdResponse resp;
    resp.sock = client;
    resp.buf = srv->work_buf;
    resp.buf_size = srv->work_buf_size;
    resp.headers_sent = false;
    resp.upgraded = false;

    route->handler(&req, &resp, route->ctx);

    /* If handler didn't send anything, send 204 */
    if (!resp.headers_sent && !resp.upgraded) {
        send_error(client, 204, "No Content");
    }

    if (!resp.upgraded) {
        syn_port_sock_close(client);
    }
}

SYN_Status syn_httpd_step(SYN_Httpd *srv)
{
    SYN_ASSERT(srv != NULL);
    if (!srv->running) return SYN_ERROR;

    SYN_Socket client = syn_port_sock_accept(srv->listener,
                                              HTTPD_ACCEPT_TIMEOUT_MS);
    if (client == SYN_SOCKET_INVALID) {
        return SYN_TIMEOUT;
    }

    handle_client(srv, client);
    return SYN_OK;
}

void syn_httpd_stop(SYN_Httpd *srv)
{
    SYN_ASSERT(srv != NULL);
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

    int n = syn_port_sock_recv(resp->sock, buf, to_read,
                                HTTPD_RECV_TIMEOUT_MS);
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
        /* Yield until a client connects (non-blocking accept) */
        PT_WAIT_UNTIL(pt,
            srv->running &&
            (syn_port_sock_accept(srv->listener, 0) != SYN_SOCKET_INVALID));

        /* A client connected — handle the request synchronously.
         * This is fine because request handling is fast (parse + dispatch).
         * For long responses, handlers could be split into sub-protothreads. */
        syn_httpd_step(srv);

        /* Yield after handling to let other tasks run */
        PT_YIELD(pt);
    }

    PT_END(pt);
}


#endif /* SYN_USE_HTTPD */
```


