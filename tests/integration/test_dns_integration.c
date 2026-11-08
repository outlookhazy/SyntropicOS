#include "mock_port.h"
#include "syntropic/net/syn_dns.h"
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

void test_dns_resolver_e2e(void)
{
    const char *host = getenv("DNS_HOST");
    if (host == NULL) {
        host = "127.0.0.1";
    }
    uint16_t port = (strcmp(host, "127.0.0.1") == 0) ? 10053U : 53U;

    printf("[Integration Test] Querying CoreDNS UDP Server at %s:%u...\n", host, port);

    SYN_SockAddr server = {0};
    server.port = port;

    struct hostent *he = gethostbyname(host);
    if (he != NULL && he->h_addr_list[0] != NULL) {
        memcpy(server.ip, he->h_addr_list[0], 4U);
    } else {
        fail_or_skip("Failed to resolve DNS_HOST");
        return;
    }

    SYN_SockAddr resolved_addr = {0};
    SYN_DnsResolver dns;
    syn_dns_init(&dns);
    dns.dns_server = &server;
    dns.hostname = "syntropic.local";
    dns.addr_out = &resolved_addr;
    dns.timeout_ms = 3000U;
    dns.status = SYN_TIMEOUT;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    memset(&task, 0, sizeof(task));
    task.user_data = &dns;

    int iterations = 0;
    while (dns.status != SYN_OK && iterations < 100) {
        mock_tick_advance(10U);
        syn_dns_resolve_task(&pt, &task);
        usleep(10000);
        iterations++;
    }

    if (dns.status != SYN_OK) {
        fail_or_skip("CoreDNS server not reachable or resolution failed");
        return;
    }

    printf("[Integration Test] Resolved 'syntropic.local' -> %u.%u.%u.%u\n", resolved_addr.ip[0],
           resolved_addr.ip[1], resolved_addr.ip[2], resolved_addr.ip[3]);

    TEST_ASSERT_EQUAL_UINT8(1U, resolved_addr.ip[0]);
    TEST_ASSERT_EQUAL_UINT8(2U, resolved_addr.ip[1]);
    TEST_ASSERT_EQUAL_UINT8(3U, resolved_addr.ip[2]);
    TEST_ASSERT_EQUAL_UINT8(4U, resolved_addr.ip[3]);

    printf("[Integration Test] End-to-End CoreDNS Integration PASS!\n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_dns_resolver_e2e);
    return UNITY_END();
}
