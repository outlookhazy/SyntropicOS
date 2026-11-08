#include "mock_port.h"
#include "syntropic/net/syn_http.h"
#include "unity/unity.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char rcv_body[512];
static size_t rcv_len = 0U;

static bool integration_skip_allowed(void)
{
    const char *v = getenv("SYN_ALLOW_SKIP_INTEGRATION");
    return (v != NULL && strcmp(v, "1") == 0);
}

static void fail_or_skip(const char *msg)
{
    if (integration_skip_allowed()) {
        TEST_IGNORE_MESSAGE(msg);
    } else {
        TEST_FAIL_MESSAGE(msg);
    }
}

static bool on_http_body(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    if (rcv_len + len < sizeof(rcv_body)) {
        memcpy(rcv_body + rcv_len, data, len);
        rcv_len += len;
        rcv_body[rcv_len] = '\0';
    }
    return true;
}

static void drive_http_client(SYN_HttpClient *client)
{
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    memset(&task, 0, sizeof(task));
    task.user_data = client;

    int iterations = 0;
    while (client->state != SYN_HTTP_STATE_DONE && iterations < 100) {
        if (client->state == SYN_HTTP_STATE_ERROR) {
            break;
        }
        mock_tick_advance(10U);
        syn_http_client_task(&pt, &task);
        usleep(10000);
        iterations++;
    }
}

void setUp(void)
{
    memset(rcv_body, 0, sizeof(rcv_body));
    rcv_len = 0U;
}

void tearDown(void)
{
}

void test_http_client_e2e(void)
{
    const char *host = getenv("HTTP_HOST");
    if (host == NULL) {
        host = "127.0.0.1";
    }
    uint16_t port = (strcmp(host, "127.0.0.1") == 0) ? 10080U : 80U;

    printf("[Integration Test] Connecting to Nginx HTTP Server at %s:%u...\n", host, port);

    uint8_t work_buf[1024];
    SYN_HttpClient client;

    /* Test 1: GET /api/status against Nginx */
    SYN_Status status;
    status = syn_http_client_init(&client, "GET", host, port, "/api/status", NULL, NULL, 0U, NULL,
                                  0U, on_http_body, NULL, work_buf, sizeof(work_buf));
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);

    drive_http_client(&client);
    if (client.state != SYN_HTTP_STATE_DONE) {
        fail_or_skip("Nginx HTTP server not reachable");
        return;
    }

    TEST_ASSERT_EQUAL_INT(200, client.resp.status_code);
    printf("[Integration Test] Nginx HTTP Response Code: %d, Body: %s\n", client.resp.status_code,
           rcv_body);
    TEST_ASSERT_NOT_NULL(strstr(rcv_body, "nginx"));

    /* Test 2: GET / (Root Endpoint) */
    memset(rcv_body, 0, sizeof(rcv_body));
    rcv_len = 0U;
    status = syn_http_client_init(&client, "GET", host, port, "/", NULL, NULL, 0U, NULL, 0U,
                                  on_http_body, NULL, work_buf, sizeof(work_buf));
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);

    drive_http_client(&client);
    TEST_ASSERT_EQUAL_INT(SYN_HTTP_STATE_DONE, client.state);
    TEST_ASSERT_EQUAL_INT(200, client.resp.status_code);
    TEST_ASSERT_NOT_NULL(strstr(rcv_body, "Ready"));

    printf("[Integration Test] End-to-End Nginx HTTP Integration PASS!\n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_http_client_e2e);
    return UNITY_END();
}
