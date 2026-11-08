#include "syntropic/net/syn_sntp.h"
#include "unity/unity.h"

#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

void setUp(void)
{
}

void tearDown(void)
{
}

void test_sntp_chrony_e2e(void)
{
    const char *host = getenv("SNTP_HOST");
    if (host == NULL) {
        host = "127.0.0.1";
    }
    uint16_t port = (strcmp(host, "127.0.0.1") == 0) ? 10123U : 123U;

    SYN_SockAddr server = {0};
    server.port = port;

    struct hostent *he = gethostbyname(host);
    if (he != NULL && he->h_addr_list[0] != NULL) {
        memcpy(server.ip, he->h_addr_list[0], 4U);
    } else {
        fail_or_skip("Failed to resolve SNTP_HOST");
        return;
    }

    SYN_SNTP sntp;
    syn_sntp_init(&sntp, &server, 3600U);

    printf("[Integration Test] Querying Chrony SNTP Server at %s:%u...\n", host, port);
    SYN_Status status = SYN_ERROR;
    for (int attempt = 0; attempt < 5; attempt++) {
        status = syn_sntp_query(&sntp);
        if (status == SYN_OK) {
            break;
        }
        usleep(200000);
    }

    if (status != SYN_OK) {
        fail_or_skip("Chrony SNTP server query failed or unreachable");
        return;
    }

    TEST_ASSERT_EQUAL_INT(SYN_OK, status);
    TEST_ASSERT_TRUE(syn_sntp_is_synced(&sntp));

    uint32_t epoch = syn_sntp_get_epoch_s(&sntp);
    printf("[Integration Test] Synced Epoch Time: %u UTC seconds\n", epoch);
    TEST_ASSERT_GREATER_THAN_UINT32(1600000000UL, epoch);

    printf("[Integration Test] End-to-End Chrony SNTP Integration PASS!\n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_sntp_chrony_e2e);
    return UNITY_END();
}
