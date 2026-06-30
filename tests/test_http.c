/**
 * @file test_http.c
 * @brief Tests for the HTTP client — header parsing, body streaming.
 */

#include "unity/unity.h"
#include "syntropic/net/syn_http.h"
#include "mocks/mock_port.h"

#include <string.h>
#include <stdio.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static uint8_t work_buf[512];

/* Body accumulator */
static uint8_t  body_accum[2048];
static size_t   body_accum_len;

static bool body_accumulate(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    if (body_accum_len + len > sizeof(body_accum)) return false;
    memcpy(body_accum + body_accum_len, data, len);
    body_accum_len += len;
    return true;
}

static void reset_accum(void)
{
    memset(body_accum, 0, sizeof(body_accum));
    body_accum_len = 0;
}

static SYN_Status run_client_task(SYN_HttpClient *client)
{
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = client;
    
    SYN_PT_Status status;
    while ((status = syn_http_client_task(&pt, &task)) == PT_WAITING || status == PT_YIELDED) {
        mock_tick_advance(10);
    }
    return client->status;
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

void test_http_get_200(void)
{
    mock_port_reset();
    reset_accum();

    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 13\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Hello, World!";

    mock_sock_set_response(response, strlen(response));

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    SYN_HttpResponse resp = client.resp;

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(200, resp.status_code);
    TEST_ASSERT_EQUAL(13, resp.content_length);
    TEST_ASSERT_TRUE(resp.connection_close);
    TEST_ASSERT_FALSE(resp.chunked);
    TEST_ASSERT_EQUAL(13, body_accum_len);
    TEST_ASSERT_EQUAL_STRING_LEN("Hello, World!", body_accum, 13);
}

void test_http_get_404(void)
{
    mock_port_reset();
    reset_accum();

    const char *response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 9\r\n"
        "\r\n"
        "Not Found";

    mock_sock_set_response(response, strlen(response));

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/missing",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    SYN_HttpResponse resp = client.resp;

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(404, resp.status_code);
    TEST_ASSERT_EQUAL(9, body_accum_len);
}

void test_http_get_large_body(void)
{
    mock_port_reset();
    reset_accum();

    /* Build a response with a 1024-byte body */
    char response[2048];
    int hdr_len = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 1024\r\n"
        "\r\n");

    /* Fill body with pattern */
    for (int i = 0; i < 1024; i++) {
        response[hdr_len + i] = (char)('A' + (i % 26));
    }

    mock_sock_set_response(response, (size_t)(hdr_len + 1024));

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/big",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    SYN_HttpResponse resp = client.resp;

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(200, resp.status_code);
    TEST_ASSERT_EQUAL(1024, body_accum_len);

    /* Verify pattern */
    for (int i = 0; i < 1024; i++) {
        TEST_ASSERT_EQUAL('A' + (i % 26), body_accum[i]);
    }
}

void test_http_get_no_content_length(void)
{
    mock_port_reset();
    mock_sock_eof_on_empty = true;
    reset_accum();

    /* Connection: close without Content-Length — read until EOF */
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "\r\n"
        "streamed data";

    mock_sock_set_response(response, strlen(response));

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/stream",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    SYN_HttpResponse resp = client.resp;

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(200, resp.status_code);
    TEST_ASSERT_EQUAL(13, body_accum_len);
    TEST_ASSERT_EQUAL_STRING_LEN("streamed data", body_accum, 13);
}

void test_http_get_sends_correct_request(void)
{
    mock_port_reset();

    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    mock_sock_set_response(response, strlen(response));

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "myhost.local", 8080, "/api/v1/status",
                          NULL, NULL, 0, NULL, 0,
                          NULL, NULL,
                          work_buf, sizeof(work_buf));
    run_client_task(&client);

    /* Verify the sent request contains the right pieces */
    mock_sock_tx_buf[mock_sock_tx_len] = '\0';
    const char *tx = (const char *)mock_sock_tx_buf;

    TEST_ASSERT_NOT_NULL(strstr(tx, "GET /api/v1/status HTTP/1.1\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(tx, "Host: myhost.local\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(tx, "Connection: close\r\n"));
}

void test_http_post_basic(void)
{
    mock_port_reset();
    reset_accum();

    const char *response =
        "HTTP/1.1 201 Created\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "OK";

    mock_sock_set_response(response, strlen(response));

    const char *body = "{\"key\":\"value\"}";

    SYN_HttpClient client;
    syn_http_client_init(&client, "POST", "api.example.com", 80, "/data",
                          "application/json",
                          (const uint8_t *)body, strlen(body),
                          NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    SYN_HttpResponse resp = client.resp;

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(201, resp.status_code);
    TEST_ASSERT_EQUAL(2, body_accum_len);

    /* Verify request included content-type and body */
    mock_sock_tx_buf[mock_sock_tx_len] = '\0';
    const char *tx = (const char *)mock_sock_tx_buf;
    TEST_ASSERT_NOT_NULL(strstr(tx, "POST /data HTTP/1.1\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(tx, "Content-Type: application/json\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(tx, "{\"key\":\"value\"}"));
}

void test_http_get_chunked(void)
{
    mock_port_reset();
    reset_accum();

    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Connection: close\r\n"
        "\r\n"
        "4\r\n"
        "Wiki\r\n"
        "5\r\n"
        "pedia\r\n"
        "E\r\n"
        " in\r\n\r\nchunks.\r\n"
        "0\r\n"
        "\r\n";

    mock_sock_set_response(response, strlen(response));

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/chunked",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    SYN_HttpResponse resp = client.resp;

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(200, resp.status_code);
    TEST_ASSERT_TRUE(resp.chunked);
    TEST_ASSERT_EQUAL(23, body_accum_len);
    TEST_ASSERT_EQUAL_STRING_LEN("Wikipedia in\r\n\r\nchunks.", body_accum, 23);
}

static int s_redirect_count = 0;
static void on_redirect_connect(const char *host, uint16_t port)
{
    (void)port;
    if (s_redirect_count == 0) {
        /* First request goes to original page, redirecting to destination */
        TEST_ASSERT_EQUAL_STRING("example.com", host);
        const char *redirect =
            "HTTP/1.1 302 Found\r\n"
            "Location: http://dest.com/target\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        mock_sock_set_response(redirect, strlen(redirect));
    } else if (s_redirect_count == 1) {
        /* Second request goes to target destination */
        TEST_ASSERT_EQUAL_STRING("dest.com", host);
        const char *ok =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 7\r\n"
            "\r\n"
            "Success";
        mock_sock_set_response(ok, strlen(ok));
    }
    s_redirect_count++;
}

void test_http_get_redirect(void)
{
    mock_port_reset();
    reset_accum();
    s_redirect_count = 0;
    mock_sock_connect_cb = on_redirect_connect;

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/redirect-me",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    SYN_HttpResponse resp = client.resp;

    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(200, resp.status_code);
    TEST_ASSERT_EQUAL(2, s_redirect_count);
    TEST_ASSERT_EQUAL(7, body_accum_len);
    TEST_ASSERT_EQUAL_STRING_LEN("Success", body_accum, 7);
}

static void on_loop_redirect_connect(const char *host, uint16_t port)
{
    (void)host; (void)port;
    const char *redirect =
        "HTTP/1.1 302 Found\r\n"
        "Location: /loop\r\n"
        "Content-Length: 0\r\n"
        "\r\n";
    mock_sock_set_response(redirect, strlen(redirect));
    s_redirect_count++;
}

void test_http_get_redirect_limit(void)
{
    mock_port_reset();
    reset_accum();
    s_redirect_count = 0;
    mock_sock_connect_cb = on_loop_redirect_connect;

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/loop",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);

    /* Should return error and stop after 5 hops */
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_EQUAL(5, s_redirect_count);
}

/* ── New VFS / HTTP Client Edge Cases ────────────────────────────────────── */

static void on_port_redirect_connect(const char *host, uint16_t port)
{
    if (s_redirect_count == 0) {
        TEST_ASSERT_EQUAL_STRING("example.com", host);
        const char *redirect =
            "HTTP/1.1 302 Found\r\n"
            "Location: http://dest.com:8080/target\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        mock_sock_set_response(redirect, strlen(redirect));
    } else if (s_redirect_count == 1) {
        TEST_ASSERT_EQUAL_STRING("dest.com", host);
        TEST_ASSERT_EQUAL_UINT16(8080, port);
        const char *ok =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 7\r\n"
            "\r\n"
            "Success";
        mock_sock_set_response(ok, strlen(ok));
    }
    s_redirect_count++;
}

static void on_noslash_redirect_connect(const char *host, uint16_t port)
{
    if (s_redirect_count == 0) {
        const char *redirect =
            "HTTP/1.1 302 Found\r\n"
            "Location: http://dest.com\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        mock_sock_set_response(redirect, strlen(redirect));
    } else if (s_redirect_count == 1) {
        TEST_ASSERT_EQUAL_STRING("dest.com", host);
        TEST_ASSERT_EQUAL_UINT16(80, port);
        const char *ok =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 7\r\n"
            "\r\n"
            "Success";
        mock_sock_set_response(ok, strlen(ok));
    }
    s_redirect_count++;
}

static void on_relative_redirect_connect(const char *host, uint16_t port)
{
    (void)port;
    if (s_redirect_count == 0) {
        const char *redirect =
            "HTTP/1.1 302 Found\r\n"
            "Location: target\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        mock_sock_set_response(redirect, strlen(redirect));
    } else if (s_redirect_count == 1) {
        TEST_ASSERT_EQUAL_STRING("example.com", host);
        const char *ok =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 7\r\n"
            "\r\n"
            "Success";
        mock_sock_set_response(ok, strlen(ok));
    }
    s_redirect_count++;
}

static void on_body_send_fail_connect(const char *host, uint16_t port)
{
    (void)host; (void)port;
    mock_sock_send_fail_after_bytes = 95;
}

static bool s_body_cb_fail = false;
static bool body_cb_rejectable(const uint8_t *data, size_t len, void *ctx)
{
    if (s_body_cb_fail) return false;
    return body_accumulate(data, len, ctx);
}

void test_http_connect_fail(void)
{
    mock_port_reset();
    mock_sock_connect_fail = true;

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_EQUAL(SYN_HTTP_STATE_ERROR, client.state);
}

void test_http_send_fail(void)
{
    mock_port_reset();
    mock_sock_send_fail = true;

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_EQUAL(SYN_HTTP_STATE_ERROR, client.state);
}

void test_http_body_send_fail(void)
{
    mock_port_reset();
    reset_accum();
    mock_sock_connect_cb = on_body_send_fail_connect;

    SYN_HttpClient client;
    syn_http_client_init(&client, "POST", "h", 80, "/",
                          "text/plain",
                          (const uint8_t *)"hellohello", 10,
                          NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_EQUAL(SYN_HTTP_STATE_ERROR, client.state);
}

void test_http_redirect_formats(void)
{
    /* A. Port redirect */
    mock_port_reset();
    reset_accum();
    s_redirect_count = 0;
    mock_sock_connect_cb = on_port_redirect_connect;

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(2, s_redirect_count);

    /* B. Noslash redirect */
    mock_port_reset();
    reset_accum();
    s_redirect_count = 0;
    mock_sock_connect_cb = on_noslash_redirect_connect;

    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(2, s_redirect_count);
    TEST_ASSERT_EQUAL_STRING("/", client.cur_path);

    /* C. Relative URL redirect without slash */
    mock_port_reset();
    reset_accum();
    s_redirect_count = 0;
    mock_sock_connect_cb = on_relative_redirect_connect;

    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(2, s_redirect_count);
    TEST_ASSERT_EQUAL_STRING("/target", client.cur_path);
}

void test_http_header_edge_cases(void)
{
    /* 1. Header tag formatting, case insensitivity and spacing */
    mock_port_reset();
    reset_accum();
    const char *resp1 =
        "HTTP/1.1 200 OK\r\n"
        "CONTENT-LENGTH:   5\r\n"
        "CONNECTION:  close\r\n"
        "LOCATION:   http://dest\r\n"
        "\r\n"
        "hello";
    mock_sock_set_response(resp1, strlen(resp1));

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(5, client.resp.content_length);
    TEST_ASSERT_TRUE(client.resp.connection_close);
    TEST_ASSERT_EQUAL_STRING("hello", (char *)body_accum);

    /* 2. Invalid status line (no space) */
    mock_port_reset();
    const char *resp2 = "HTTP/1.1_200_OK\r\n\r\n";
    mock_sock_set_response(resp2, strlen(resp2));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* 3. Connection closed before header finishes */
    mock_port_reset();
    mock_sock_eof_on_empty = true;
    const char *resp3 = "HTTP/1.1 200 OK\r\nContent-Length: 5";
    mock_sock_set_response(resp3, strlen(resp3));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* 4. Header receive timeout */
    mock_port_reset();
    const char *resp4 = "HTTP/1.1 200 OK\r\n";
    mock_sock_set_response(resp4, strlen(resp4));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &client;
    
    SYN_PT_Status pst;
    while ((pst = syn_http_client_task(&pt, &task)) == PT_WAITING || pst == PT_YIELDED) {
        mock_tick_advance(11000); // Exceed HTTP_RECV_TIMEOUT_MS (10000ms)
    }
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, client.status);

    /* 5. Header buffer overflow */
    mock_port_reset();
    char resp5[600];
    memset(resp5, 'A', sizeof(resp5));
    resp5[599] = '\0';
    mock_sock_set_response(resp5, strlen(resp5));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
}

void test_http_chunked_errors(void)
{
    /* 1. Invalid hex characters in chunk size */
    mock_port_reset();
    const char *resp1 =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "3g\r\ndata\r\n";
    mock_sock_set_response(resp1, strlen(resp1));

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* 2. Chunk size line overflow */
    mock_port_reset();
    const char *resp2 =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "1234567890123456789012345678901234567890\r\n";
    mock_sock_set_response(resp2, strlen(resp2));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, st);

    /* 3. Body callback rejection */
    mock_port_reset();
    reset_accum();
    s_body_cb_fail = true;
    const char *resp3 =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "5\r\nhello\r\n0\r\n\r\n";
    mock_sock_set_response(resp3, strlen(resp3));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_cb_rejectable, NULL,
                          work_buf, sizeof(work_buf));
    st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    s_body_cb_fail = false;

    /* 4. Premature EOF during chunk */
    mock_port_reset();
    mock_sock_eof_on_empty = true;
    const char *resp4 =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "a\r\nhello"; // EOF before 10 bytes read
    mock_sock_set_response(resp4, strlen(resp4));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* 5. Timeout during chunk read */
    mock_port_reset();
    const char *resp5 =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "a\r\nhello";
    mock_sock_set_response(resp5, strlen(resp5));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &client;
    SYN_PT_Status pst;
    while ((pst = syn_http_client_task(&pt, &task)) == PT_WAITING || pst == PT_YIELDED) {
        mock_tick_advance(11000);
    }
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, client.status);
}

void test_http_streaming_errors(void)
{
    /* 1. Body callback rejection for cached initial read bytes */
    mock_port_reset();
    reset_accum();
    s_body_cb_fail = true;
    const char *resp1 =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello";
    mock_sock_set_response(resp1, strlen(resp1));
    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_cb_rejectable, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    s_body_cb_fail = false;

    /* 2. Body callback rejection for newly read bytes */
    mock_port_reset();
    reset_accum();
    const char *resp2 =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 10\r\n"
        "\r\n"
        "hello";
    mock_sock_set_response(resp2, strlen(resp2));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_cb_rejectable, NULL,
                          work_buf, sizeof(work_buf));
    
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &client;
    
    /* Deliver "hello" first (state changes to reading body) */
    TEST_ASSERT_EQUAL(PT_YIELDED, syn_http_client_task(&pt, &task));
    
    /* Set next response in socket and configure callback to reject */
    s_body_cb_fail = true;
    mock_sock_set_response("world", 5);
    while (syn_http_client_task(&pt, &task) == PT_YIELDED) {
        mock_tick_advance(10);
    }
    TEST_ASSERT_EQUAL(SYN_ERROR, client.status);
    s_body_cb_fail = false;

    /* 3. Premature EOF before content-length satisfied */
    mock_port_reset();
    reset_accum();
    mock_sock_eof_on_empty = true;
    const char *resp3 =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 10\r\n"
        "\r\n"
        "hello";
    mock_sock_set_response(resp3, strlen(resp3));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* 4. Timeout during body streaming */
    mock_port_reset();
    reset_accum();
    const char *resp4 =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 10\r\n"
        "\r\n"
        "hello";
    mock_sock_set_response(resp4, strlen(resp4));
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    PT_INIT(&pt);
    task.user_data = &client;
    TEST_ASSERT_EQUAL(PT_YIELDED, syn_http_client_task(&pt, &task));
    while (syn_http_client_task(&pt, &task) == PT_YIELDED) {
        mock_tick_advance(11000);
    }
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, client.status);
}

void test_http_extra_data_in_buffer(void)
{
    mock_port_reset();
    reset_accum();

    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 5\r\n"
        "Connection: close\r\n"
        "\r\n"
        "HelloExtraGarbage";

    mock_sock_set_response(response, strlen(response));

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(5, body_accum_len);
    TEST_ASSERT_EQUAL_STRING_LEN("Hello", body_accum, 5);
}

void test_http_custom_headers(void)
{
    mock_port_reset();
    reset_accum();

    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 2\r\n"
        "Connection: close\r\n"
        "\r\n"
        "OK";
    mock_sock_set_response(response, strlen(response));

    SYN_HttpHeader custom_headers[3] = {
        {"X-Custom-1", "Value1"},
        {NULL, NULL}, // Should be skipped
        {"X-Custom-2", "Value2"}
    };

    SYN_HttpClient client;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0,
                          custom_headers, 3,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    SYN_Status st = run_client_task(&client);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    // Verify custom headers in mock_sock_tx_buf
    mock_sock_tx_buf[mock_sock_tx_len] = '\0';
    TEST_ASSERT_NOT_NULL(strstr((const char *)mock_sock_tx_buf, "X-Custom-1: Value1\r\n"));
    TEST_ASSERT_NOT_NULL(strstr((const char *)mock_sock_tx_buf, "X-Custom-2: Value2\r\n"));
}

void test_http_chunked_boundary_cases(void)
{
    const char *headers =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Connection: close\r\n"
        "\r\n";

    SYN_HttpClient client;
    SYN_PT pt;
    SYN_Task task;
    int status;

    /* A. Successful boundary parse */
    mock_port_reset();
    reset_accum();
    mock_sock_connected = true;

    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));

    PT_INIT(&pt);
    task.user_data = &client;

    // First call connects and writes request headers, yielding on response headers read
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_YIELDED, status);

    // 1. Feed headers
    mock_sock_set_response(headers, strlen(headers));
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_YIELDED, status);

    // 2. Feed chunk size part 1 (chunk_state == 0 socket read)
    mock_sock_set_response("3", 1);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_YIELDED, status);

    // 3. Feed chunk size part 2
    mock_sock_set_response("\r\n", 2);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_YIELDED, status);

    // 4. Feed partial chunk data (chunk_state == 1 socket read when chunk_remaining > 0)
    mock_sock_set_response("ab", 2);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_YIELDED, status);
    TEST_ASSERT_EQUAL(2, body_accum_len);

    // 5. Feed remaining chunk data
    mock_sock_set_response("c", 1);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_YIELDED, status);
    TEST_ASSERT_EQUAL(3, body_accum_len);

    // 6. Feed chunk separator (chunk_state == 1 socket read when chunk_remaining == 0)
    mock_sock_set_response("\r\n", 2);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_YIELDED, status);

    // 7. Feed next chunk size
    mock_sock_set_response("0\r\n", 3);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_YIELDED, status);

    // 8. Feed final chunked trailer separator (chunk_state == 2 socket read)
    mock_sock_set_response("\r\n", 2);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_EXITED, status);
    TEST_ASSERT_EQUAL(SYN_OK, client.status);

    /* B. State 0: Socket close */
    mock_port_reset();
    reset_accum();
    mock_sock_connected = true;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    PT_INIT(&pt);
    task.user_data = &client;

    syn_http_client_task(&pt, &task);
    mock_sock_set_response(headers, strlen(headers));
    syn_http_client_task(&pt, &task);

    mock_sock_set_response("3", 1);
    syn_http_client_task(&pt, &task);

    mock_sock_eof_on_empty = true;
    mock_sock_set_response("", 0);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL(SYN_ERROR, client.status);

    /* C. State 0: Socket timeout */
    mock_port_reset();
    reset_accum();
    mock_sock_connected = true;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    PT_INIT(&pt);
    task.user_data = &client;

    syn_http_client_task(&pt, &task);
    mock_sock_set_response(headers, strlen(headers));
    syn_http_client_task(&pt, &task);

    mock_sock_set_response("3", 1);
    syn_http_client_task(&pt, &task);

    mock_tick_advance(11000);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, client.status);

    /* D. State 1: Socket close */
    mock_port_reset();
    reset_accum();
    mock_sock_connected = true;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    PT_INIT(&pt);
    task.user_data = &client;

    syn_http_client_task(&pt, &task);
    mock_sock_set_response(headers, strlen(headers));
    syn_http_client_task(&pt, &task);
    mock_sock_set_response("3\r\n", 3);
    syn_http_client_task(&pt, &task);

    mock_sock_eof_on_empty = true;
    mock_sock_set_response("", 0);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL(SYN_ERROR, client.status);

    /* E. State 1: Socket timeout */
    mock_port_reset();
    reset_accum();
    mock_sock_connected = true;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    PT_INIT(&pt);
    task.user_data = &client;

    syn_http_client_task(&pt, &task);
    mock_sock_set_response(headers, strlen(headers));
    syn_http_client_task(&pt, &task);
    mock_sock_set_response("3\r\n", 3);
    syn_http_client_task(&pt, &task);

    mock_tick_advance(11000);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, client.status);

    /* F. State 1 separator: Socket close */
    mock_port_reset();
    reset_accum();
    mock_sock_connected = true;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    PT_INIT(&pt);
    task.user_data = &client;

    syn_http_client_task(&pt, &task);
    mock_sock_set_response(headers, strlen(headers));
    syn_http_client_task(&pt, &task);
    mock_sock_set_response("3\r\nabc", 6);
    syn_http_client_task(&pt, &task);

    mock_sock_eof_on_empty = true;
    mock_sock_set_response("", 0);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL(SYN_ERROR, client.status);

    /* G. State 1 separator: Socket timeout */
    mock_port_reset();
    reset_accum();
    mock_sock_connected = true;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    PT_INIT(&pt);
    task.user_data = &client;

    syn_http_client_task(&pt, &task);
    mock_sock_set_response(headers, strlen(headers));
    syn_http_client_task(&pt, &task);
    mock_sock_set_response("3\r\nabc", 6);
    syn_http_client_task(&pt, &task);

    mock_tick_advance(11000);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, client.status);

    /* H. State 2: Socket close (which breaks and succeeds) */
    mock_port_reset();
    reset_accum();
    mock_sock_connected = true;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    PT_INIT(&pt);
    task.user_data = &client;

    syn_http_client_task(&pt, &task);
    mock_sock_set_response(headers, strlen(headers));
    syn_http_client_task(&pt, &task);
    mock_sock_set_response("3\r\nabc\r\n0\r\n", 11);
    syn_http_client_task(&pt, &task);

    mock_sock_eof_on_empty = true;
    mock_sock_set_response("", 0);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_EXITED, status);
    TEST_ASSERT_EQUAL(SYN_OK, client.status);

    /* I. State 2: Socket timeout */
    mock_port_reset();
    reset_accum();
    mock_sock_connected = true;
    syn_http_client_init(&client, "GET", "example.com", 80, "/",
                          NULL, NULL, 0, NULL, 0,
                          body_accumulate, NULL,
                          work_buf, sizeof(work_buf));
    PT_INIT(&pt);
    task.user_data = &client;

    syn_http_client_task(&pt, &task);
    mock_sock_set_response(headers, strlen(headers));
    syn_http_client_task(&pt, &task);
    mock_sock_set_response("3\r\nabc\r\n0\r\n", 11);
    syn_http_client_task(&pt, &task);

    mock_tick_advance(11000);
    status = syn_http_client_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, client.status);
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

void run_http_tests(void)
{
    RUN_TEST(test_http_get_200);
    RUN_TEST(test_http_get_404);
    RUN_TEST(test_http_get_large_body);
    RUN_TEST(test_http_get_no_content_length);
    RUN_TEST(test_http_get_sends_correct_request);
    RUN_TEST(test_http_post_basic);
    RUN_TEST(test_http_get_chunked);
    RUN_TEST(test_http_get_redirect);
    RUN_TEST(test_http_get_redirect_limit);
    RUN_TEST(test_http_connect_fail);
    RUN_TEST(test_http_send_fail);
    RUN_TEST(test_http_body_send_fail);
    RUN_TEST(test_http_redirect_formats);
    RUN_TEST(test_http_header_edge_cases);
    RUN_TEST(test_http_chunked_errors);
    RUN_TEST(test_http_streaming_errors);
    RUN_TEST(test_http_extra_data_in_buffer);
    RUN_TEST(test_http_custom_headers);
    RUN_TEST(test_http_chunked_boundary_cases);
}
