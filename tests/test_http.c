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
}
