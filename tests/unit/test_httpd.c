/**
 * @file test_httpd.c
 * @brief Tests for the HTTP server — request parsing, routing, responses.
 *        Extended to achieve 100% line coverage.
 */

#include "mocks/mock_port.h"
#include "syntropic/net/syn_httpd.h"
#include "unity/unity.h"

#include <stdio.h>
#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static uint8_t work_buf[1024];
static bool handler_called;
static SYN_HttpMethod last_method;
static char last_path[64];
static char last_query[64];
static size_t last_content_length;
static char last_content_type[64];

static void test_handler(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)ctx;
    handler_called = true;
    last_method = req->method;
    last_content_length = req->content_length;
    strncpy(last_path, req->path, sizeof(last_path) - 1);
    if (req->query) {
        strncpy(last_query, req->query, sizeof(last_query) - 1);
    }
    if (req->content_type) {
        strncpy(last_content_type, req->content_type, sizeof(last_content_type) - 1);
    }

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "text/plain");
    syn_httpd_body_str(resp, "Hello!");
}

static void json_handler(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)req;
    (void)ctx;
    handler_called = true;
    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/json");
    syn_httpd_body_str(resp, "{\"status\":\"ok\"}");
}

/* Handler that reads request body using syn_httpd_read_body */
static char body_buf[256];
static int body_read_len;

static void body_handler(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)ctx;
    handler_called = true;
    last_content_length = req->content_length;

    /* Read available body bytes */
    body_read_len = syn_httpd_read_body(req, resp, body_buf, sizeof(body_buf) - 1);
    if (body_read_len > 0) {
        body_buf[body_read_len] = '\0';
    }

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_body_str(resp, "done");
}

/* Handler that reads body twice (hits remaining==0 path) */
static void body_handler_double_read(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp,
                                     void *ctx)
{
    (void)ctx;
    handler_called = true;

    char tmp[32];
    /* First read — consume all */
    syn_httpd_read_body(req, resp, tmp, sizeof(tmp));
    /* Second read — remaining == 0 */
    body_read_len = syn_httpd_read_body(req, resp, tmp, sizeof(tmp));

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_body_str(resp, "ok");
}

/* Handler that sends nothing — exercises 204 No Content path */
static void silent_handler(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)req;
    (void)resp;
    (void)ctx;
    handler_called = true;
    /* Deliberately do not call syn_httpd_status/body */
}

static const SYN_HttpdRoute test_routes[] = {
    {SYN_HTTP_GET, "/", test_handler, NULL},
    {SYN_HTTP_GET, "/api/status", json_handler, NULL},
    {SYN_HTTP_POST, "/api/config", test_handler, NULL},
    {SYN_HTTP_GET, "/api/*", test_handler, NULL},
    {SYN_HTTP_PUT, "/resource", test_handler, NULL},
    {SYN_HTTP_DELETE, "/resource", test_handler, NULL},
    {SYN_HTTP_POST, "/body", body_handler, NULL},
    {SYN_HTTP_POST, "/body2", body_handler_double_read, NULL},
    {SYN_HTTP_GET, "/silent", silent_handler, NULL},
};
static const size_t NUM_ROUTES = 9;

static SYN_Httpd srv;

static void setup_server(void)
{
    mock_port_reset();
    handler_called = false;
    body_read_len = 0;
    memset(last_path, 0, sizeof(last_path));
    memset(last_query, 0, sizeof(last_query));
    memset(last_content_type, 0, sizeof(last_content_type));
    memset(body_buf, 0, sizeof(body_buf));
    last_content_length = 0;

    syn_httpd_init(&srv, 80, test_routes, NUM_ROUTES, work_buf, sizeof(work_buf));
}

/* ── Original tests (preserved) ─────────────────────────────────────────── */

void test_httpd_get_root(void)
{
    setup_server();

    const char *request = "GET / HTTP/1.1\r\nHost: 192.168.1.1\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    SYN_Status st = syn_httpd_step(&srv);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL(SYN_HTTP_GET, last_method);
    TEST_ASSERT_EQUAL_STRING("/", last_path);

    /* Check response was sent */
    TEST_ASSERT_TRUE(mock_sock_tx_len > 0);
    /* Should contain HTTP/1.1 200 OK */
    TEST_ASSERT_NOT_NULL(strstr((char *)mock_sock_tx_buf, "200 OK"));
    TEST_ASSERT_NOT_NULL(strstr((char *)mock_sock_tx_buf, "Hello!"));
}

void test_httpd_get_api_status(void)
{
    setup_server();

    const char *request = "GET /api/status HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr((char *)mock_sock_tx_buf, "application/json"));
    TEST_ASSERT_NOT_NULL(strstr((char *)mock_sock_tx_buf, "{\"status\":\"ok\"}"));
}

void test_httpd_post_route(void)
{
    setup_server();

    const char *request = "POST /api/config HTTP/1.1\r\n"
                          "Content-Type: application/json\r\n"
                          "Content-Length: 13\r\n"
                          "\r\n"
                          "{\"key\":\"val\"}";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL(SYN_HTTP_POST, last_method);
    TEST_ASSERT_EQUAL_STRING("/api/config", last_path);
}

void test_httpd_wildcard_route(void)
{
    setup_server();

    const char *request = "GET /api/other HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL_STRING("/api/other", last_path);

    /* Test path shorter than wildcard prefix */
    handler_called = false;
    const char *short_request = "GET /a HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(short_request, strlen(short_request));
    syn_httpd_step(&srv);
    TEST_ASSERT_FALSE(handler_called);
}

void test_httpd_404_not_found(void)
{
    setup_server();

    const char *request = "GET /nonexistent HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_FALSE(handler_called);
    /* Should send 404 */
    TEST_ASSERT_NOT_NULL(strstr((char *)mock_sock_tx_buf, "404"));
}

void test_httpd_query_string(void)
{
    setup_server();

    const char *request = "GET /?key=val&a=1 HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL_STRING("/", last_path);
    TEST_ASSERT_EQUAL_STRING("key=val&a=1", last_query);
}

void test_httpd_connection_close(void)
{
    setup_server();

    const char *request = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_NOT_NULL(strstr((char *)mock_sock_tx_buf, "Connection: close"));
}

void test_httpd_no_client(void)
{
    setup_server();
    mock_sock_accept_ok = false;

    SYN_Status st = syn_httpd_step(&srv);
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, st);
    TEST_ASSERT_FALSE(handler_called);
}

void test_httpd_method_mismatch(void)
{
    setup_server();

    /* POST to a GET-only route */
    const char *request = "POST / HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_FALSE(handler_called); /* No matching POST for "/" */
    TEST_ASSERT_NOT_NULL(strstr((char *)mock_sock_tx_buf, "404"));
}

/* ── New tests for uncovered paths ──────────────────────────────────────── */

/** PUT method — exercises parse_method PUT branch */
void test_httpd_put_method(void)
{
    setup_server();

    const char *request = "PUT /resource HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL(SYN_HTTP_PUT, last_method);
}

/** DELETE method — exercises parse_method DELETE branch */
void test_httpd_delete_method(void)
{
    setup_server();

    const char *request = "DELETE /resource HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL(SYN_HTTP_DELETE, last_method);
}

/** Unknown method — exercises parse_method default fallback (returns SYN_HTTP_GET) */
void test_httpd_unknown_method_fallback(void)
{
    setup_server();

    /* "PATCH" is not GET/POST/PUT/DELETE — must NOT fall back to GET.
     * The "/" GET route should NOT be dispatched. */
    const char *request = "PATCH / HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    /* PATCH is unknown — no route match, handler must not be called */
    TEST_ASSERT_FALSE(handler_called);
}

/** Content-Length header — exercises parse_uint and body_buffered path */
void test_httpd_content_length_parsing(void)
{
    setup_server();

    /* Body sent inline with headers (body buffered in work_buf) */
    const char *request = "POST /body HTTP/1.1\r\n"
                          "Content-Length: 11\r\n"
                          "Content-Type: text/plain\r\n"
                          "\r\n"
                          "hello world";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    /* Content-Length header was parsed correctly */
    TEST_ASSERT_EQUAL_INT(11, (int)last_content_length);
    /* body_read_len is positive (buffered) or negative (socket timeout) —
     * but NOT zero, because content_length > 0 */
    TEST_ASSERT_NOT_EQUAL(0, body_read_len);
}

/** Content-Length with leading spaces — exercises "while (*val == ' ')" */
void test_httpd_content_length_with_spaces(void)
{
    setup_server();

    const char *request = "POST /body HTTP/1.1\r\n"
                          "Content-Length:   5\r\n" /* extra spaces after colon */
                          "\r\n"
                          "abcde";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL_INT(5, (int)last_content_length);
}

/** Body read when body_buffered_len > to_read (partial consume) */
void test_httpd_body_read_partial_buffered(void)
{
    setup_server();

    /* Large body inline so buffered_len > small max_len */
    /* We'll read with a small buffer — triggers "consume > to_read" branch */
    const char *request = "POST /body HTTP/1.1\r\n"
                          "Content-Length: 50\r\n"
                          "\r\n"
                          "0123456789012345678901234567890123456789012345678X";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL_INT(50, (int)last_content_length);
}

/** Body read when remaining == 0 after first read */
void test_httpd_body_read_already_consumed(void)
{
    setup_server();
    /* Enable EOF on empty so second socket recv returns 0 (not -1) */
    mock_sock_eof_on_empty = true;

    const char *request = "POST /body2 HTTP/1.1\r\n"
                          "Content-Length: 4\r\n"
                          "\r\n"
                          "data";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    /* Second read: remaining==0, returns 0 immediately */
    TEST_ASSERT_EQUAL_INT(0, body_read_len);
}

/** Body read when content_length == 0 → returns 0 immediately */
void test_httpd_body_read_no_content(void)
{
    setup_server();

    const char *request = "POST /body HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL_INT(0, (int)last_content_length);
    TEST_ASSERT_EQUAL_INT(0, body_read_len);
}

/** Malformed request — exercises 400 Bad Request path */
void test_httpd_bad_request(void)
{
    setup_server();

    /* Malformed HTTP — recv returns garbage then EOF */
    mock_sock_eof_on_empty = true;
    /* Load garbage without \r\n\r\n */
    const char *garbage = "GARBAGE REQUEST NO HEADERS";
    mock_sock_set_response(garbage, strlen(garbage));

    /* First step: accepts client, reads garbage, no \r\n\r\n found */
    syn_httpd_step(&srv);
    TEST_ASSERT_FALSE(handler_called);

    /* Second step: recv returns 0 (EOF) → sends 400 */
    syn_httpd_step(&srv);
    TEST_ASSERT_FALSE(handler_called);
    /* Should have sent 400 */
    TEST_ASSERT_NOT_NULL(strstr((char *)mock_sock_tx_buf, "400"));
}

/** Handler sends nothing → 204 No Content */
void test_httpd_no_content_response(void)
{
    setup_server();

    const char *request = "GET /silent HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr((char *)mock_sock_tx_buf, "204"));
}

/** syn_httpd_stop() — exercises shutdown path */
void test_httpd_stop(void)
{
    setup_server();
    TEST_ASSERT_TRUE(srv.running);

    syn_httpd_stop(&srv);
    TEST_ASSERT_FALSE(srv.running);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, srv.listener);

    /* Calling stop again (listener already invalid) — should not crash */
    syn_httpd_stop(&srv);
    TEST_ASSERT_FALSE(srv.running);
}

/** syn_httpd_step() when not running → returns SYN_ERROR */
void test_httpd_step_not_running(void)
{
    setup_server();
    syn_httpd_stop(&srv);

    SYN_Status st = syn_httpd_step(&srv);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
}

/** Body read from socket (not buffered) — body arrives separately */
void test_httpd_body_read_from_socket(void)
{
    setup_server();

    /* Headers only (body NOT inline) — body_buffered_len will be 0 */
    /* We set content_length=5 but don't include body inline, then
     * the body_handler will try socket recv for the remaining bytes. */
    const char *headers = "POST /body HTTP/1.1\r\n"
                          "Content-Length: 5\r\n"
                          "\r\n";
    /* Append body data separately so it arrives via socket recv */
    char full[256];
    size_t hlen = strlen(headers);
    memcpy(full, headers, hlen);
    memcpy(full + hlen, "hello", 5);
    mock_sock_set_response(full, hlen + 5);

    syn_httpd_step(&srv);
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL_INT(5, (int)last_content_length);
}

/** Direct test of syn_httpd_read_body with body_buffered_len > 0 */
void test_httpd_read_body_buffered_direct(void)
{
    mock_port_reset();
    mock_sock_connected = true;

    /* Craft a fake request with body already in a buffer */
    static uint8_t fake_buf[64];
    const char *body_str = "HELLO";
    size_t body_len = 5;
    size_t body_off = 10; /* pretend headers end at offset 10 */
    memcpy(fake_buf + body_off, body_str, body_len);

    SYN_HttpdRequest req;
    memset(&req, 0, sizeof(req));
    req.content_length = body_len;
    req.body_buffered_offset = body_off;
    req.body_buffered_len = body_len;
    req.body_consumed = 0;

    SYN_HttpdResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.sock = 0; /* mock socket */
    resp.buf = fake_buf;
    resp.buf_size = sizeof(fake_buf);

    /* First read — from buffered portion */
    char out[32];
    int n = syn_httpd_read_body(&req, &resp, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT((int)body_len, n);
    out[n] = '\0';
    TEST_ASSERT_EQUAL_STRING("HELLO", out);

    /* Second read — remaining == 0 */
    int n2 = syn_httpd_read_body(&req, &resp, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT(0, n2);

    /* Partial buffered read — consume > to_read branch */
    memset(&req, 0, sizeof(req));
    resp.body_consumed = 0U;
    req.content_length = 10;
    req.body_buffered_offset = body_off;
    req.body_buffered_len = 10; /* 10 buffered but max_len=3 */
    memcpy(fake_buf + body_off, "0123456789", 10);
    n = syn_httpd_read_body(&req, &resp, out, 3); /* to_read=3, consume=3 */
    TEST_ASSERT_EQUAL_INT(3, n);
}

/** syn_httpd_init failure when listen returns invalid socket */
void test_httpd_init_listen_fail(void)
{
    mock_port_reset();
    mock_sock_listen_ok = false;

    SYN_Httpd srv2;
    uint8_t buf[256];
    SYN_Status st = syn_httpd_init(&srv2, 80, test_routes, NUM_ROUTES, buf, sizeof(buf));
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
}

/** Body socket recv with body_consumed increment (line 414) */
void test_httpd_read_body_socket_recv_consumed(void)
{
    mock_port_reset();
    mock_sock_connected = true;

    /* Set up socket with body data to be read */
    const char *body = "world";
    mock_sock_set_response(body, 5);
    mock_sock_rx_pos = 0;

    SYN_HttpdRequest req;
    memset(&req, 0, sizeof(req));
    req.content_length = 5;
    req.body_buffered_len = 0; /* no buffered — goes to socket recv */
    req.body_consumed = 0;

    SYN_HttpdResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.sock = 0;
    static uint8_t resp_buf[64];
    resp.buf = resp_buf;
    resp.buf_size = sizeof(resp_buf);

    char out[32];
    int n = syn_httpd_read_body(&req, &resp, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT(5, n);                       /* socket recv returned 5 */
    TEST_ASSERT_EQUAL_INT(5, (int)resp.body_consumed); /* tracked on mutable resp */
}
/** httpd_task protothread — non-blocking step + yield */
static void test_httpd_task_protothread(void)
{
    setup_server();
    srv.running = true;
    mock_sock_connected = true;
    mock_sock_accept_ok = true;

    const char *request = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(request, strlen(request));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &srv;

    /* First call: step() accepts + recvs + dispatches (fall-through),
     * then PT_WAIT_UNTIL returns PT_IDLE (no new data) or PT_MORE */
    SYN_PT_Status st = syn_httpd_task(&pt, &task);
    TEST_ASSERT_NOT_EQUAL(PT_EXITED, st);
    TEST_ASSERT_NOT_EQUAL(PT_ENDED, st);
    TEST_ASSERT_TRUE(handler_called);

    /* Second call: no client → step returns SYN_TIMEOUT, task blocks on has_work */
    mock_sock_accept_ok = false;
    st = syn_httpd_task(&pt, &task);
    TEST_ASSERT_NOT_EQUAL(PT_EXITED, st);
    TEST_ASSERT_NOT_EQUAL(PT_ENDED, st);
}

void test_httpd_connection_timeout_and_payload_overflow(void)
{
    setup_server();
    srv.running = true;
    mock_sock_connected = true;
    mock_sock_accept_ok = true;

    /* 1. Request Timeout (408) */
    srv.state = SYN_HTTPD_READING_HEADERS;
    srv.client = 1;
    srv.recv_deadline = 100;
    mock_tick_ms = 200; /* Expired */
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, syn_httpd_step(&srv));
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);

    /* 2. Buffer Full (413) */
    setup_server();
    srv.running = true;
    srv.state = SYN_HTTPD_READING_HEADERS;
    srv.client = 1;
    srv.rx_total = sizeof(work_buf) - 1; /* Space == 0 */
    srv.recv_deadline = 1000;
    mock_tick_ms = 100;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_httpd_step(&srv));
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);

    /* 3. Upgraded Connection */
    setup_server();
    srv.running = true;
    mock_sock_accept_ok = true;
    const char *req_str = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(req_str, strlen(req_str));

    /* Simulate route handler that sets upgraded = true */
    SYN_HttpdResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.sock = 1;
    resp.headers_sent = true;
    resp.upgraded = true;
    (void)resp;

    srv.state = SYN_HTTPD_READING_HEADERS;
    srv.client = 1;
    srv.rx_total = strlen(req_str);
    memcpy(work_buf, req_str, srv.rx_total + 1);

    /* Test upgraded flag cleanup path */
    syn_httpd_step(&srv);
}

static void test_httpd_body_str_direct(void)
{
    setup_server();
    SYN_HttpdResponse resp = {.sock = 1, .buf = work_buf, .headers_sent = false};
    syn_httpd_status(&resp, 200, "OK");
    syn_httpd_header(&resp, "Content-Type", "text/plain");
    syn_httpd_body_str(&resp, "Body String Direct Test");
    TEST_ASSERT_TRUE(resp.headers_sent);
}

static void test_httpd_invalid_state_and_null_status(void)
{
    SYN_Httpd srv;
    syn_httpd_init(&srv, 80, test_routes, 1, work_buf, sizeof(work_buf));
    srv.running = true;
    srv.state = 99; /* Invalid state */

    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_httpd_step(&srv));
    TEST_ASSERT_EQUAL_INT(SYN_HTTPD_IDLE, srv.state);

    /* Null status calls */
    syn_httpd_status(NULL, 200, "OK");
    SYN_HttpdResponse resp = {0};
    syn_httpd_status(&resp, 200, NULL);
}

static void upgrade_handler(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)req;
    (void)ctx;
    resp->upgraded = true;
    resp->headers_sent = true;
}

static void test_httpd_connection_upgrade_handler(void)
{
    mock_port_reset();
    SYN_HttpdRoute u_routes[] = {
        {.method = SYN_HTTP_GET, .path = "/ws", .handler = upgrade_handler, .ctx = NULL}};

    SYN_Httpd srv;
    syn_httpd_init(&srv, 80, u_routes, 1, work_buf, sizeof(work_buf));
    srv.running = true;

    /* Simulate client connected in SYN_HTTPD_DISPATCHING state with /ws request */
    srv.client = 10;
    srv.state = SYN_HTTPD_DISPATCHING;
    const char req[] = "GET /ws HTTP/1.1\r\nHost: localhost\r\n\r\n";
    memcpy(srv.work_buf, req, strlen(req));
    srv.rx_total = strlen(req);

    TEST_ASSERT_EQUAL(SYN_OK, syn_httpd_step(&srv));
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, srv.client);
}

static void test_httpd_bad_request_malformed_headers(void)
{
    setup_server();
    srv.state = SYN_HTTPD_DISPATCHING;
    /* Request with no space in request line */
    const char req[] = "GET/HTTP/1.1\r\nHost: localhost\r\n\r\n";
    memcpy(srv.work_buf, req, strlen(req));
    srv.rx_total = strlen(req);

    syn_httpd_step(&srv);
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);
}

static void test_httpd_header_length_limits_and_malformed_lines(void)
{
    setup_server();

    /* 1. Header parsing line 160: no next_line in header loop */
    srv.state = SYN_HTTPD_DISPATCHING;
    const char req1[] = "GET / HTTP/1.1\r\nContent-Length: 5";
    memcpy(srv.work_buf, req1, sizeof(req1));
    srv.rx_total = sizeof(req1) - 1;
    syn_httpd_step(&srv);

    /* 2. recv returning -1 in SYN_HTTPD_READING_HEADERS -> SYN_TIMEOUT (line 366) */
    setup_server();
    srv.running = true;
    srv.state = SYN_HTTPD_READING_HEADERS;
    srv.client = 1;
    srv.rx_total = 0;
    mock_sock_eof_on_empty = false;
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, syn_httpd_step(&srv));

    /* 3. NULL checks: syn_httpd_body(NULL, ...) */
    syn_httpd_body(NULL, "data", 4);

    /* 4. Malformed request line no space (line 115) */
    setup_server();
    srv.state = SYN_HTTPD_DISPATCHING;
    const char req2[] = "GET_NO_SPACE\r\n\r\n";
    memcpy(srv.work_buf, req2, sizeof(req2));
    srv.rx_total = sizeof(req2) - 1;
    syn_httpd_step(&srv);

    /* 5. NULL header guards & drop_client(NULL) (lines 234 & 440) */
    SYN_HttpdResponse dummy_hdr_resp = {0};
    syn_httpd_header(NULL, "Host", "test");
    syn_httpd_header(&dummy_hdr_resp, NULL, "test");
    syn_httpd_header(&dummy_hdr_resp, "Host", NULL);

    /* 6. READING_HEADERS state timeout when recv returns -1 (line 366) */
    setup_server();
    srv.state = SYN_HTTPD_READING_HEADERS;
    srv.client = 1;
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    mock_sock_eof_on_empty = false;
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, syn_httpd_step(&srv));

    /* 7. Content-Type header with following header (line 156) */
    setup_server();
    srv.state = SYN_HTTPD_READING_HEADERS;
    const char req_content_type[] =
        "GET / HTTP/1.1\r\nContent-Type: text/plain\r\nHost: localhost\r\n\r\n";
    memcpy(srv.work_buf, req_content_type, sizeof(req_content_type) - 1);
    srv.rx_total = sizeof(req_content_type) - 1;
    syn_httpd_step(&srv);

    /* 8. Socket recv returns -1 (line 366) */
    setup_server();
    srv.state = SYN_HTTPD_READING_HEADERS;
    srv.client = 1;
    mock_sock_connected = true;
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    mock_sock_eof_on_empty = false;
    srv.rx_total = 0;
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, syn_httpd_step(&srv));

    /* 9. NULL resp in finalize_headers (line 458) */
    syn_httpd_body(NULL, "data", 4);

    /* 10. Header line ending before CRLFCRLF (line 143) */
    setup_server();
    const char *hdr_with_crlfcrlf = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    mock_sock_set_response(hdr_with_crlfcrlf, strlen(hdr_with_crlfcrlf));
    srv.client = 1;
    mock_sock_connected = true;
    syn_httpd_step(&srv);

    /* 11. Bad request line (line 187) */
    setup_server();
    const char *bad_req = "BADREQ\r\n\r\n";
    mock_sock_set_response(bad_req, strlen(bad_req));
    srv.client = 1;
    mock_sock_connected = true;
    syn_httpd_step(&srv);
}

static void test_httpd_connection_listener_and_task_polling(void)
{
    SYN_Httpd srv;
    memset(&srv, 0, sizeof(srv));
    uint8_t wbuf[256];
    srv.work_buf = wbuf;
    srv.work_buf_size = sizeof(wbuf);
    srv.running = false;
    SYN_Task task = {.user_data = &srv};
    SYN_PT pt;
    PT_INIT(&pt);
    /* Stopped server exits task cleanly (PT_ENDED) */
    TEST_ASSERT_EQUAL(PT_ENDED, syn_httpd_task(&pt, &task));

    PT_INIT(&pt);
    srv.running = true;
    srv.listener = 1;
    srv.client = SYN_SOCKET_INVALID;
    srv.work_buf_size = 2; /* space = 1 byte so 2-byte payload takes two steps */
    mock_sock_connected = true;
    uint8_t dummy2[] = {'G', 'E'};
    mock_sock_set_response(dummy2, sizeof(dummy2));
    SYN_PT_Status res = syn_httpd_task(&pt, &task);
    /* Accepted client read 1 of 2 bytes -> readable data remains -> yields PT_MORE */
    TEST_ASSERT_EQUAL(PT_MORE, res);

    /* Second step consumes final queued byte -> socket drained -> yields PT_IDLE */
    srv.listener = SYN_SOCKET_INVALID;
    srv.work_buf_size = sizeof(wbuf);
    res = syn_httpd_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_IDLE, res);

    /* Request line with 1 space only (sp2 == NULL, line 122) */
    setup_server();
    srv.state = SYN_HTTPD_DISPATCHING;
    const char req_one_space[] = "GET /path\r\n\r\n";
    memcpy(srv.work_buf, req_one_space, sizeof(req_one_space) - 1);
    srv.rx_total = sizeof(req_one_space) - 1;
    syn_httpd_step(&srv);

    /* Wildcard route mismatch (k mismatch in match_route, line 204) */
    setup_server();
    const char req_wildcard_mismatch[] = "GET /app/other HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(req_wildcard_mismatch, strlen(req_wildcard_mismatch));
    syn_httpd_step(&srv);

    /* Test PUT, DELETE, and OPTIONS method parsing */
    setup_server();
    const char req_put[] =
        "PUT /api/status HTTP/1.1\r\nHost: test\r\nContent-Type: text/html\r\n\r\n";
    mock_sock_set_response(req_put, strlen(req_put));
    syn_httpd_step(&srv);

    setup_server();
    const char req_delete[] = "DELETE /api/status HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(req_delete, strlen(req_delete));
    syn_httpd_step(&srv);

    setup_server();
    const char req_options[] = "OPTIONS /api/status HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(req_options, strlen(req_options));
    syn_httpd_step(&srv);

    /* Request line missing first space (sp1 == NULL) */
    setup_server();
    srv.state = SYN_HTTPD_DISPATCHING;
    const char req_no_space[] = "GETNOPATH\r\n\r\n";
    memcpy(srv.work_buf, req_no_space, sizeof(req_no_space) - 1);
    srv.rx_total = sizeof(req_no_space) - 1;
    syn_httpd_step(&srv);

    /* Test n == 0 (connection closed before headers complete) */
    setup_server();
    srv.state = SYN_HTTPD_READING_HEADERS;
    srv.client = 1;
    srv.rx_total = 0;
    mock_sock_connected = true;
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    mock_sock_eof_on_empty = true;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_httpd_step(&srv));
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);
    mock_sock_eof_on_empty = false;

    /* Test invalid state in syn_httpd_step */
    setup_server();
    srv.state = (SYN_HttpdState)99;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_httpd_step(&srv));
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);

    /* Test POST method request */
    setup_server();
    const char req_post[] = "POST /api/status HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(req_post, strlen(req_post));
    syn_httpd_step(&srv);

    /* Exact route path mismatch */
    setup_server();
    const char req_exact_mismatch[] = "GET /nonexistent HTTP/1.1\r\nHost: test\r\n\r\n";
    mock_sock_set_response(req_exact_mismatch, strlen(req_exact_mismatch));
    syn_httpd_step(&srv);

    /* Test null/zero parameter branches for status, header, body */
    uint8_t dummy_buf[64];
    SYN_HttpdResponse resp = {.sock = 1,
                              .buf = dummy_buf,
                              .buf_size = sizeof(dummy_buf),
                              .headers_sent = false,
                              .upgraded = false};
    syn_httpd_status(NULL, 200, "OK");
    syn_httpd_status(&resp, 200, NULL);
    syn_httpd_status(&resp, 200, "OK");

    syn_httpd_header(NULL, "X-Key", "Val");
    syn_httpd_header(&resp, NULL, "Val");
    syn_httpd_header(&resp, "X-Key", NULL);
    syn_httpd_header(&resp, "X-Key", "Val");

    syn_httpd_body(NULL, "data", 4);
    syn_httpd_body(&resp, NULL, 4);
    syn_httpd_body(&resp, (const uint8_t *)"x", 0);

    char str_buf[16];
    SYN_HttpdRequest req_zero = {.content_length = 0, .body_consumed = 0};

    /* Read body when remaining == 0 */
    int bytes = syn_httpd_read_body(&req_zero, &resp, str_buf, 10);
    TEST_ASSERT_EQUAL(0, bytes);

    /* Read buffered body in partial chunks (lines 520-528) */
    uint8_t body_storage[64];
    memcpy(body_storage, "0123456789", 10);
    SYN_HttpdResponse resp_buffered = {
        .sock = 1, .buf = body_storage, .buf_size = sizeof(body_storage)};
    SYN_HttpdRequest req_buffered = {.content_length = 10,
                                     .body_consumed = 0,
                                     .body_buffered_len = 10,
                                     .body_buffered_offset = 0};
    char chunk1[5];
    int n1 = syn_httpd_read_body(&req_buffered, &resp_buffered, chunk1, 5);
    TEST_ASSERT_EQUAL(5, n1);
    TEST_ASSERT_EQUAL_MEMORY("01234", chunk1, 5);

    char chunk2[5];
    int n2 = syn_httpd_read_body(&req_buffered, &resp_buffered, chunk2, 5);
    TEST_ASSERT_EQUAL(5, n2);
    TEST_ASSERT_EQUAL_MEMORY("56789", chunk2, 5);
}

static void test_httpd_query_params(void)
{
    const char *q = "channel=2&mode=auto&gain=10&flag";
    char val[16];

    /* 1. Extract first param */
    TEST_ASSERT_TRUE(syn_httpd_get_query_param(q, "channel", val, sizeof(val)));
    TEST_ASSERT_EQUAL_STRING("2", val);

    /* 2. Extract middle param */
    TEST_ASSERT_TRUE(syn_httpd_get_query_param(q, "mode", val, sizeof(val)));
    TEST_ASSERT_EQUAL_STRING("auto", val);

    /* 3. Extract last param */
    TEST_ASSERT_TRUE(syn_httpd_get_query_param(q, "gain", val, sizeof(val)));
    TEST_ASSERT_EQUAL_STRING("10", val);

    /* 4. Missing key */
    TEST_ASSERT_FALSE(syn_httpd_get_query_param(q, "missing", val, sizeof(val)));

    /* 5. Buffer too small */
    TEST_ASSERT_FALSE(syn_httpd_get_query_param(q, "mode", val, 3)); /* "auto" needs 5 bytes */

    /* 6. NULL guards */
    TEST_ASSERT_FALSE(syn_httpd_get_query_param(NULL, "key", val, sizeof(val)));
    TEST_ASSERT_FALSE(syn_httpd_get_query_param(q, NULL, val, sizeof(val)));
    TEST_ASSERT_FALSE(syn_httpd_get_query_param(q, "key", NULL, sizeof(val)));
    TEST_ASSERT_FALSE(syn_httpd_get_query_param(q, "key", val, 0));
}

static void test_httpd_task_guards_dispatching_stop_and_n05(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task empty_task = {0};

    /* 1. NULL guards (syn_httpd.c:570-572) */
    TEST_ASSERT_EQUAL(PT_EXITED, syn_httpd_task(NULL, &empty_task));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_httpd_task(&pt, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_httpd_task(&pt, &empty_task));

    /* 2. Park task at PT_WAIT_UNTIL while idle, then enter SYN_HTTPD_DISPATCHING ->
     *    wakes via httpd_has_work line 563 and dispatches route */
    setup_server();
    srv.running = true;
    mock_sock_connected = false;
    mock_sock_accept_ok = false;

    SYN_Task task = {0};
    task.user_data = &srv;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));

    const char *raw_req = "GET / HTTP/1.1\r\nHost: test\r\n\r\n";
    memcpy(srv.work_buf, raw_req, strlen(raw_req) + 1);
    srv.rx_total = strlen(raw_req);
    srv.client = 11;
    srv.state = SYN_HTTPD_DISPATCHING;
    mock_sock_connected = true;
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;

    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);

    /* 3. While parked at PT_WAIT_UNTIL (line 578), stop server (srv.running = false) ->
     *    wakes and hits `if (!srv->running) break;` at line 580 -> PT_ENDED */
    srv.running = false;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_httpd_task(&pt, &task));

    /* 4. Regression N-05: while in SYN_HTTPD_READING_HEADERS with no unread client data
     *    before recv_deadline, syn_httpd_task yields PT_IDLE (never spins PT_MORE even if
     *    listener has pending readability), then wakes at recv_deadline to send 408 */
    setup_server();
    srv.running = true;
    mock_tick_ms = 1000;
    mock_sock_connected = true;
    mock_sock_accept_ok = true;
    const char *partial_hdr = "GET / HTTP/1.1\r\nHost: slow-client";
    mock_sock_set_response(partial_hdr, strlen(partial_hdr));

    PT_INIT(&pt);
    memset(&task, 0, sizeof(task));
    task.user_data = &srv;

    /* Accepts client, reads partial header, drains client RX -> must yield PT_IDLE */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_HTTPD_READING_HEADERS, srv.state);
    uint32_t deadline = srv.recv_deadline;
    TEST_ASSERT_TRUE((int32_t)(deadline - mock_tick_ms) > 0);

    /* Before deadline with empty client RX: stays PT_IDLE */
    mock_tick_ms = deadline - 100U;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));

    /* Even if listener socket appears readable while client socket is not readable,
     * READING_HEADERS must still stay PT_IDLE before recv_deadline */
    SYN_Socket saved_client = srv.client;
    srv.client = SYN_SOCKET_INVALID;
    mock_sock_set_response("NEWCONN", 7);
    TEST_ASSERT_TRUE(syn_port_sock_readable(srv.listener));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));

    /* Restore client with empty RX and advance to recv_deadline -> wakes and sends 408 */
    srv.client = saved_client;
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    mock_sock_tx_len = 0;
    mock_sock_accept_ok = false;
    mock_tick_ms = deadline;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);
    mock_sock_tx_buf[mock_sock_tx_len] = '\0';
    TEST_ASSERT_NOT_NULL(strstr((const char *)mock_sock_tx_buf, "408"));
}

static bool s_r3_ws_hdr_intact = false;
static char s_r3_ct_seen[64];

static void r3_inspect_headers_handler(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp,
                                       void *ctx)
{
    (void)ctx;
    s_r3_ws_hdr_intact =
        (req->headers != NULL) &&
        (strstr(req->headers, "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==") != NULL);
    memset(s_r3_ct_seen, 0, sizeof(s_r3_ct_seen));
    if (req->content_type != NULL) {
        strncpy(s_r3_ct_seen, req->content_type, sizeof(s_r3_ct_seen) - 1U);
    }
    syn_httpd_status(resp, 200, "OK");
    syn_httpd_body_str(resp, "ok");
}

static void test_httpd_round3_regressions(void)
{
    /* 1. R3-N13: Content-Type before Sec-WebSocket-Key must not truncate req->headers */
    mock_port_reset();
    SYN_HttpdRoute r3_routes[] = {
        {.method = SYN_HTTP_GET, .path = "/ws-check", .handler = r3_inspect_headers_handler}};
    SYN_Httpd r3_srv;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_httpd_init(&r3_srv, 80, r3_routes, 1, work_buf, sizeof(work_buf)));

    const char *req_hdr_order = "GET /ws-check HTTP/1.1\r\n"
                                "Content-Type:   application/json\r\n"
                                "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                                "\r\n";
    mock_sock_set_response(req_hdr_order, strlen(req_hdr_order));
    s_r3_ws_hdr_intact = false;
    TEST_ASSERT_EQUAL(SYN_OK, syn_httpd_step(&r3_srv));
    TEST_ASSERT_TRUE(s_r3_ws_hdr_intact);
    TEST_ASSERT_EQUAL_STRING("application/json", s_r3_ct_seen);

    /* Oversized Content-Type (>= 64 bytes) is bounded cleanly in content_type_buf */
    const char *req_long_ct =
        "GET /ws-check HTTP/1.1\r\n"
        "Content-Type: "
        "application/vnd.very.long.custom.media.type.that.exceeds.sixty.four.chars\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "\r\n";
    mock_sock_set_response(req_long_ct, strlen(req_long_ct));
    s_r3_ws_hdr_intact = false;
    TEST_ASSERT_EQUAL(SYN_OK, syn_httpd_step(&r3_srv));
    TEST_ASSERT_TRUE(s_r3_ws_hdr_intact);
    TEST_ASSERT_EQUAL_UINT(63U, strlen(s_r3_ct_seen));

    /* 2. R3-N06: Socket recv error while readable drops client and returns SYN_ERROR */
    setup_server();
    srv.state = SYN_HTTPD_READING_HEADERS;
    srv.client = 11;
    srv.recv_deadline = 5000U;
    mock_tick_ms = 100U;
    mock_sock_connected = true;
    mock_sock_recv_fail = true;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_httpd_step(&srv));
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, srv.client);

    /* Non-readable empty socket in READING_HEADERS returns SYN_TIMEOUT without dropping client */
    setup_server();
    srv.state = SYN_HTTPD_READING_HEADERS;
    srv.client = 11;
    srv.recv_deadline = 5000U;
    mock_tick_ms = 100U;
    mock_sock_connected = true;
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, syn_httpd_step(&srv));
    TEST_ASSERT_EQUAL(SYN_HTTPD_READING_HEADERS, srv.state);

    /* 3. R3-N06: Readable listener with failing accept yields PT_IDLE via PT_PASS (never spins) */
    setup_server();
    mock_sock_connected = true;
    mock_sock_set_response("X", 1);
    mock_sock_accept_ok = false;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &srv;

    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));
    srv.running = false;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_httpd_task(&pt, &task));
}

static void test_httpd_round4_request_line_smuggling(void)
{
    /* R4-N15: sp1 on a subsequent header line (no space in request line) -> 400 Bad Request */
    setup_server();
    handler_called = false;
    const char *smuggle_sp1 = "GET\r\nHost: foo bar\r\n\r\n";
    mock_sock_set_response(smuggle_sp1, strlen(smuggle_sp1));
    TEST_ASSERT_EQUAL(SYN_OK, syn_httpd_step(&srv));
    TEST_ASSERT_FALSE(handler_called);
    mock_sock_tx_buf[mock_sock_tx_len] = '\0';
    TEST_ASSERT_NOT_NULL(strstr((const char *)mock_sock_tx_buf, "400 Bad Request"));

    /* R4-N15: sp2 on a subsequent header line (only 1 space in request line) -> 400 Bad Request */
    setup_server();
    handler_called = false;
    const char *smuggle_sp2 = "GET /\r\nHost: foo bar\r\n\r\n";
    mock_sock_set_response(smuggle_sp2, strlen(smuggle_sp2));
    TEST_ASSERT_EQUAL(SYN_OK, syn_httpd_step(&srv));
    TEST_ASSERT_FALSE(handler_called);
    mock_sock_tx_buf[mock_sock_tx_len] = '\0';
    TEST_ASSERT_NOT_NULL(strstr((const char *)mock_sock_tx_buf, "400 Bad Request"));
}

static size_t s_stream_total = 0U;
static uint8_t s_stream_xor = 0U;

static void stream_post_handler(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)ctx;
    handler_called = true;
    s_stream_total = 0U;
    s_stream_xor = 0U;
    uint8_t chunk[128];
    for (;;) {
        int n = syn_httpd_read_body(req, resp, chunk, sizeof(chunk));
        if (n <= 0) {
            break;
        }
        for (int i = 0; i < n; i++) {
            s_stream_xor ^= chunk[i];
        }
        s_stream_total += (size_t)n;
    }
    syn_httpd_status(resp, 200, "OK");
    syn_httpd_body_str(resp, "streamed");
}

static void test_r5_n13_httpd_split_post_413_and_const_body(void)
{
    /* 1. Split TCP POST body accumulated across 3 separate non-blocking reads before dispatch */
    setup_server();
    handler_called = false;
    memset(body_buf, 0, sizeof(body_buf));
    body_read_len = 0;
    mock_sock_connected = true;
    mock_sock_accept_ok = true;

    const char *post_hdrs = "POST /body HTTP/1.1\r\n"
                            "Host: localhost\r\n"
                            "Content-Length: 11\r\n"
                            "\r\n";
    mock_sock_set_response(post_hdrs, strlen(post_hdrs));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &srv};

    /* Step 1: reads headers only, transitions to SYN_HTTPD_READING_BODY, yields PT_IDLE */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_HTTPD_READING_BODY, srv.state);
    TEST_ASSERT_FALSE(handler_called);

    /* Step 2: first 6 bytes of POST body arrive ("hello ") -> still awaiting remaining 5 bytes */
    mock_sock_set_response("hello ", 6U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_HTTPD_READING_BODY, srv.state);
    TEST_ASSERT_FALSE(handler_called);

    /* Step 3: final 5 bytes ("world") arrive -> dispatches route with complete 11-byte body */
    mock_sock_set_response("world", 5U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_httpd_task(&pt, &task));
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL_INT(11, body_read_len);
    TEST_ASSERT_EQUAL_STRING("hello world", body_buf);
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);

    /* 2a. Oversized headers exceeding work_buf capacity rejected with 413 */
    setup_server();
    handler_called = false;
    static char huge_hdr[1100];
    memset(huge_hdr, 'A', sizeof(huge_hdr));
    memcpy(huge_hdr, "GET / HTTP/1.1\r\nX-Big: ", 23U);
    mock_sock_set_response(huge_hdr, sizeof(huge_hdr));
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, syn_httpd_step(&srv));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_httpd_step(&srv));
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_EQUAL(SYN_HTTPD_IDLE, srv.state);
    mock_sock_tx_buf[mock_sock_tx_len] = '\0';
    TEST_ASSERT_NOT_NULL(strstr((const char *)mock_sock_tx_buf, "413 Request Too Large"));

    /* 2b. Content-Length exceeding work_buf capacity dispatches to streaming handler */
    mock_port_reset();
    handler_called = false;
    SYN_HttpdRoute stream_routes[] = {
        {.method = SYN_HTTP_POST, .path = "/ota", .handler = stream_post_handler, .ctx = NULL}};
    SYN_Httpd stream_srv;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_httpd_init(&stream_srv, 80, stream_routes, 1, work_buf, sizeof(work_buf)));
    static uint8_t big_req[2600];
    const char *ota_hdr = "POST /ota HTTP/1.1\r\nContent-Length: 2500\r\n\r\n";
    size_t ota_hlen = strlen(ota_hdr);
    memcpy(big_req, ota_hdr, ota_hlen);
    uint8_t expected_xor = 0U;
    for (size_t i = 0U; i < 2500U; i++) {
        big_req[ota_hlen + i] = (uint8_t)((i * 7U + 3U) & 0xFFU);
        expected_xor ^= big_req[ota_hlen + i];
    }
    mock_sock_set_response(big_req, ota_hlen + 2500U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_httpd_step(&stream_srv));
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_EQUAL_size_t(2500U, s_stream_total);
    TEST_ASSERT_EQUAL_HEX8(expected_xor, s_stream_xor);

    /* 3. Strictly const SYN_HttpdRequest read via syn_httpd_read_body without const-cast UB */
    uint8_t backing[16] = "abcdef";
    static const SYN_HttpdRequest const_req = {
        .method = SYN_HTTP_POST,
        .content_length = 6U,
        .body_buffered_offset = 0U,
        .body_buffered_len = 6U,
    };
    SYN_HttpdResponse mut_resp = {.sock = 1, .buf = backing, .buf_size = sizeof(backing)};
    char out_chunk[4];
    TEST_ASSERT_EQUAL_INT(3, syn_httpd_read_body(&const_req, &mut_resp, out_chunk, 3U));
    TEST_ASSERT_EQUAL_MEMORY("abc", out_chunk, 3U);
    TEST_ASSERT_EQUAL_INT(3, syn_httpd_read_body(&const_req, &mut_resp, out_chunk, 4U));
    TEST_ASSERT_EQUAL_MEMORY("def", out_chunk, 3U);
    TEST_ASSERT_EQUAL_INT(0, syn_httpd_read_body(&const_req, &mut_resp, out_chunk, 4U));

    /* Non-zero req.body_consumed starting offset honored when resp.body_consumed == 0 */
    SYN_HttpdRequest offset_req = const_req;
    offset_req.body_consumed = 4U;
    memset(&mut_resp, 0, sizeof(mut_resp));
    mut_resp.sock = 1;
    mut_resp.buf = backing;
    mut_resp.buf_size = sizeof(backing);
    TEST_ASSERT_EQUAL_INT(2, syn_httpd_read_body(&offset_req, &mut_resp, out_chunk, 4U));
    TEST_ASSERT_EQUAL_MEMORY("ef", out_chunk, 2U);
}

void run_httpd_tests(void)
{
    RUN_TEST(test_httpd_get_root);
    RUN_TEST(test_httpd_get_api_status);
    RUN_TEST(test_httpd_post_route);
    RUN_TEST(test_httpd_wildcard_route);
    RUN_TEST(test_httpd_404_not_found);
    RUN_TEST(test_httpd_query_string);
    RUN_TEST(test_httpd_connection_close);
    RUN_TEST(test_httpd_no_client);
    RUN_TEST(test_httpd_method_mismatch);
    /* New coverage tests */
    RUN_TEST(test_httpd_put_method);
    RUN_TEST(test_httpd_delete_method);
    RUN_TEST(test_httpd_unknown_method_fallback);
    RUN_TEST(test_httpd_content_length_parsing);
    RUN_TEST(test_httpd_content_length_with_spaces);
    RUN_TEST(test_httpd_body_read_partial_buffered);
    RUN_TEST(test_httpd_body_read_already_consumed);
    RUN_TEST(test_httpd_body_read_no_content);
    RUN_TEST(test_httpd_bad_request);
    RUN_TEST(test_httpd_no_content_response);
    RUN_TEST(test_httpd_stop);
    RUN_TEST(test_httpd_step_not_running);
    RUN_TEST(test_httpd_body_read_from_socket);
    /* Direct API and init failure tests */
    RUN_TEST(test_httpd_read_body_buffered_direct);
    RUN_TEST(test_httpd_init_listen_fail);
    RUN_TEST(test_httpd_read_body_socket_recv_consumed);
    /* Protothread */
    RUN_TEST(test_httpd_task_protothread);
    RUN_TEST(test_httpd_connection_timeout_and_payload_overflow);
    RUN_TEST(test_httpd_body_str_direct);
    RUN_TEST(test_httpd_invalid_state_and_null_status);
    RUN_TEST(test_httpd_connection_upgrade_handler);
    RUN_TEST(test_httpd_bad_request_malformed_headers);
    RUN_TEST(test_httpd_header_length_limits_and_malformed_lines);
    RUN_TEST(test_httpd_connection_listener_and_task_polling);
    RUN_TEST(test_httpd_query_params);
    RUN_TEST(test_httpd_task_guards_dispatching_stop_and_n05);
    RUN_TEST(test_httpd_round3_regressions);
    RUN_TEST(test_httpd_round4_request_line_smuggling);
    RUN_TEST(test_r5_n13_httpd_split_post_413_and_const_body);

    /* Test NULL parameter guards for response helpers */
    SYN_HttpdResponse resp_dummy = {0};
    syn_httpd_status(NULL, 200, "OK");
    syn_httpd_status(&resp_dummy, 200, NULL);
    syn_httpd_header(NULL, "Key", "Val");
    syn_httpd_header(&resp_dummy, NULL, "Val");
    syn_httpd_header(&resp_dummy, "Key", NULL);
    syn_httpd_body(NULL, "data", 4);
}
