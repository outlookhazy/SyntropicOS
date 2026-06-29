/**
 * @file test_httpd.c
 * @brief Tests for the HTTP server — request parsing, routing, responses.
 */

#include "unity/unity.h"
#include "syntropic/net/syn_httpd.h"
#include "mocks/mock_port.h"

#include <string.h>
#include <stdio.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static uint8_t work_buf[1024];
static bool handler_called;
static SYN_HttpMethod last_method;
static char last_path[64];
static char last_query[64];

static void test_handler(const SYN_HttpdRequest *req,
                           SYN_HttpdResponse *resp, void *ctx)
{
    (void)ctx;
    handler_called = true;
    last_method = req->method;
    strncpy(last_path, req->path, sizeof(last_path) - 1);
    if (req->query) {
        strncpy(last_query, req->query, sizeof(last_query) - 1);
    }

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "text/plain");
    syn_httpd_body_str(resp, "Hello!");
}

static void json_handler(const SYN_HttpdRequest *req,
                           SYN_HttpdResponse *resp, void *ctx)
{
    (void)req; (void)ctx;
    handler_called = true;
    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/json");
    syn_httpd_body_str(resp, "{\"status\":\"ok\"}");
}

static const SYN_HttpdRoute test_routes[] = {
    { SYN_HTTP_GET,  "/",            test_handler, NULL },
    { SYN_HTTP_GET,  "/api/status",  json_handler, NULL },
    { SYN_HTTP_POST, "/api/config",  test_handler, NULL },
    { SYN_HTTP_GET,  "/api/*",       test_handler, NULL },
};

static SYN_Httpd srv;

static void setup_server(void)
{
    mock_port_reset();
    handler_called = false;
    memset(last_path, 0, sizeof(last_path));
    memset(last_query, 0, sizeof(last_query));

    syn_httpd_init(&srv, 80, test_routes, 4, work_buf, sizeof(work_buf));
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

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

    const char *request =
        "POST /api/config HTTP/1.1\r\n"
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

/* ── Test group ────────────────────────────────────────────────────────── */

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
}
