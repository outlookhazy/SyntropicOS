#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/net/syn_dns.h"
#include "syntropic/port/syn_port_system.h"
#include <string.h>

void test_dns_resolve(void)
{
    mock_port_reset();

    /* DNS response for "google.com" resolving to 1.2.3.4 */
    /* TransID: 0x1234, Flags: 0x8180 (response, standard), Questions: 1, Answers: 1 */
    uint8_t response[] = {
        0x00, 0x00,             /* ID */
        0x81, 0x80,             /* Flags */
        0x00, 0x01,             /* Questions */
        0x00, 0x01,             /* Answers */
        0x00, 0x00, 0x00, 0x00, /* Authority, Additional */
        /* Question: "google.com" */
        6, 'g', 'o', 'o', 'g', 'l', 'e', 3, 'c', 'o', 'm', 0,
        0x00, 0x01,             /* QTYPE = A */
        0x00, 0x01,             /* QCLASS = IN */
        /* Answer: pointer to google.com (0xC00C), Type: A (1), Class: IN (1), TTL: 300, RDLen: 4, Addr: 1.2.3.4 */
        0xC0, 0x0C,
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x00, 0x01, 0x2C,
        0x00, 0x04,
        1, 2, 3, 4
    };

    SYN_SockAddr from;
    from.ip[0] = 8; from.ip[1] = 8; from.ip[2] = 8; from.ip[3] = 8;
    from.port = 53;
    mock_udp_set_response(response, sizeof(response), &from);

    SYN_SockAddr resolved;
    SYN_DnsResolver r;
    r.dns_server = NULL;
    r.hostname = "google.com";
    r.addr_out = &resolved;
    r.timeout_ms = 1000;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &r;

    while (syn_dns_resolve_task(&pt, &task) == PT_WAITING) {
        syn_port_delay_ms(1);
    }

    TEST_ASSERT_EQUAL(SYN_OK, r.status);
    TEST_ASSERT_EQUAL_UINT8(1, resolved.ip[0]);
    TEST_ASSERT_EQUAL_UINT8(2, resolved.ip[1]);
    TEST_ASSERT_EQUAL_UINT8(3, resolved.ip[2]);
    TEST_ASSERT_EQUAL_UINT8(4, resolved.ip[3]);

    /* Verify query packet sent */
    TEST_ASSERT_TRUE(mock_udp_tx_len > 12);
    TEST_ASSERT_EQUAL_UINT8(8, mock_udp_tx_to.ip[0]);
    TEST_ASSERT_EQUAL_UINT8(8, mock_udp_tx_to.ip[1]);
    TEST_ASSERT_EQUAL_UINT8(8, mock_udp_tx_to.ip[2]);
    TEST_ASSERT_EQUAL_UINT8(8, mock_udp_tx_to.ip[3]);
    TEST_ASSERT_EQUAL_UINT16(53, mock_udp_tx_to.port);
}

void test_mdns_responder(void)
{
    mock_port_reset();

    SYN_Mdns mdns;
    uint8_t ip[] = { 192, 168, 1, 100 };
    SYN_Status init_st = syn_mdns_init(&mdns, "mydevice", ip);
    TEST_ASSERT_EQUAL(SYN_OK, init_st);
    TEST_ASSERT_EQUAL(20, mdns.sock);

    /* Simulate incoming mDNS query for "mydevice.local" on port 5353 */
    /* TransID: 0, Flags: 0, Questions: 1, Answers: 0 */
    uint8_t query[] = {
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x01,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        8, 'm', 'y', 'd', 'e', 'v', 'i', 'c', 'e',
        5, 'l', 'o', 'c', 'a', 'l',
        0,
        0x00, 0x01, /* QTYPE = A */
        0x00, 0x01  /* QCLASS = IN */
    };

    SYN_SockAddr from;
    from.ip[0] = 192; from.ip[1] = 168; from.ip[2] = 1; from.ip[3] = 50;
    from.port = 5353;
    mock_udp_set_response(query, sizeof(query), &from);

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task task;
    task.user_data = &mdns;

    /* Run task */
    syn_mdns_task(&pt, &task);

    /* Verify responder sent a reply to 224.0.0.251:5353 */
    TEST_ASSERT_TRUE(mock_udp_tx_len > 12);
    TEST_ASSERT_EQUAL_UINT8(224, mock_udp_tx_to.ip[0]);
    TEST_ASSERT_EQUAL_UINT8(0, mock_udp_tx_to.ip[1]);
    TEST_ASSERT_EQUAL_UINT8(0, mock_udp_tx_to.ip[2]);
    TEST_ASSERT_EQUAL_UINT8(251, mock_udp_tx_to.ip[3]);
    TEST_ASSERT_EQUAL_UINT16(5353, mock_udp_tx_to.port);

    /* Verify response flags (Authoritative, Response) */
    TEST_ASSERT_EQUAL_UINT8(0x84, mock_udp_tx_buf[2]);
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_udp_tx_buf[3]);
    TEST_ASSERT_EQUAL_UINT8(0x01, mock_udp_tx_buf[7]); /* Answers = 1 */

    /* Verify IP address in answer payload */
    uint8_t *addr_ptr = &mock_udp_tx_buf[mock_udp_tx_len - 4];
    TEST_ASSERT_EQUAL_UINT8(192, addr_ptr[0]);
    TEST_ASSERT_EQUAL_UINT8(168, addr_ptr[1]);
    TEST_ASSERT_EQUAL_UINT8(1,   addr_ptr[2]);
    TEST_ASSERT_EQUAL_UINT8(100, addr_ptr[3]);
}
/** DNS resolve with custom server — exercises line 142 */
static void test_dns_resolve_custom_server(void)
{
    mock_port_reset();

    /* Response matching ID 0 */
    uint8_t response[] = {
        0x00, 0x00, 0x81, 0x80, 0x00, 0x01, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00,
        3, 'f', 'o', 'o', 3, 'c', 'o', 'm', 0,
        0x00, 0x01, 0x00, 0x01,
        0xC0, 0x0C, 0x00, 0x01, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x3C, 0x00, 0x04,
        10, 20, 30, 40
    };
    SYN_SockAddr from = {{ 1, 1, 1, 1 }, 53};
    mock_udp_set_response(response, sizeof(response), &from);

    SYN_SockAddr custom = {{ 1, 1, 1, 1 }, 53};
    SYN_SockAddr resolved;
    SYN_DnsResolver r;
    r.dns_server = &custom;
    r.hostname = "foo.com";
    r.addr_out = &resolved;
    r.timeout_ms = 1000;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &r;
    while (syn_dns_resolve_task(&pt, &task) == PT_WAITING) {
        syn_port_delay_ms(1);
    }
    TEST_ASSERT_EQUAL(SYN_OK, r.status);
    TEST_ASSERT_EQUAL_UINT8(10, resolved.ip[0]);
}

/** DNS resolve: UDP open fail — exercises lines 153-154 */
static void test_dns_resolve_udp_open_fail(void)
{
    mock_port_reset();
    mock_udp_open_ok = false;

    SYN_SockAddr resolved;
    SYN_DnsResolver r;
    r.dns_server = NULL;
    r.hostname = "test.com";
    r.addr_out = &resolved;
    r.timeout_ms = 1000;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &r;
    while (syn_dns_resolve_task(&pt, &task) == PT_WAITING) {
        syn_port_delay_ms(1);
    }
    TEST_ASSERT_EQUAL(SYN_ERROR, r.status);
    mock_udp_open_ok = true;
}

/** DNS resolve: send fails — exercises lines 174-177 */
static void test_dns_resolve_send_fail(void)
{
    mock_port_reset();
    mock_udp_sendto_fail = true;

    SYN_SockAddr resolved;
    SYN_DnsResolver r;
    r.dns_server = NULL;
    r.hostname = "test.com";
    r.addr_out = &resolved;
    r.timeout_ms = 1000;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &r;
    while (syn_dns_resolve_task(&pt, &task) == PT_WAITING) {
        syn_port_delay_ms(1);
    }
    TEST_ASSERT_EQUAL(SYN_ERROR, r.status);
    mock_udp_sendto_fail = false;
}

/** DNS resolve: timeout — exercises lines 192-194 */
static void test_dns_resolve_timeout(void)
{
    mock_port_reset();
    /* No UDP response loaded → recv returns -1 every time → timeout */

    SYN_SockAddr resolved;
    SYN_DnsResolver r;
    r.dns_server = NULL;
    r.hostname = "timeout.com";
    r.addr_out = &resolved;
    r.timeout_ms = 10; /* Very short timeout */

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &r;
    for (int i = 0; i < 200; i++) {
        if (syn_dns_resolve_task(&pt, &task) >= PT_EXITED) break;
        mock_tick_ms += 1; /* advance time */
    }
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, r.status);
}

static void test_dns_resolve_cname(void)
{
    mock_port_reset();
    uint8_t rx[] = {
        0x00, 0x00, 0x81, 0x80, /* ID, flags */
        0x00, 0x01, 0x00, 0x01, /* 1 question, 1 answer */
        0x00, 0x00, 0x00, 0x00, /* 0 auth, 0 add */
        /* Question: "example.com" */
        7, 'e','x','a','m','p','l','e', 3, 'c','o','m', 0,
        0x00, 0x01, 0x00, 0x01, /* QTYPE A, QCLASS IN */
        /* Answer: CNAME (type 5), length 4, "abcd" */
        0xC0, 0x0C, /* Pointer to question */
        0x00, 0x05, /* TYPE CNAME */
        0x00, 0x01, /* CLASS IN */
        0x00, 0x00, 0x00, 0x3C, /* TTL */
        0x00, 0x04, /* RDLEN = 4 */
        'a', 'b', 'c', 'd'
    };
    mock_udp_set_response(rx, sizeof(rx), NULL);

    SYN_SockAddr resolved;
    SYN_DnsResolver r;
    r.dns_server = NULL;
    r.hostname = "example.com";
    r.addr_out = &resolved;
    r.timeout_ms = 1000;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &r;
    for (int i = 0; i < 2000; i++) {
        if (syn_dns_resolve_task(&pt, &task) >= PT_EXITED) break;
        mock_tick_ms += 1;
    }
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, r.status);
}

static void test_mdns_join_fail(void)
{
    mock_port_reset();
    mock_udp_multicast_join_ok = false;
    SYN_Mdns mdns;
    uint8_t ip[4] = {192, 168, 1, 100};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mdns_init(&mdns, "device", ip));
    mock_udp_multicast_join_ok = true;
}

static void test_mdns_malformed_query(void)
{
    mock_port_reset();
    SYN_Mdns mdns;
    uint8_t ip[4] = {192, 168, 1, 100};
    TEST_ASSERT_EQUAL(SYN_OK, syn_mdns_init(&mdns, "device", ip));

    /* Malformed query: valid header, 1 question, but qname exceeds length */
    uint8_t rx[] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        64, 'a' /* Label length 64, but buffer ends here */
    };
    mock_udp_set_response(rx, sizeof(rx), NULL);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &mdns;
    syn_mdns_task(&pt, &task);
}

static void test_mdns_no_match(void)
{
    mock_port_reset();
    SYN_Mdns mdns;
    uint8_t ip[4] = {192, 168, 1, 100};
    TEST_ASSERT_EQUAL(SYN_OK, syn_mdns_init(&mdns, "device", ip));

    /* Query for "other.local", 1 question */
    uint8_t rx[] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        5, 'o', 't', 'h', 'e', 'r',
        5, 'l', 'o', 'c', 'a', 'l', 0,
        0x00, 0x01, 0x00, 0x01
    };
    mock_udp_set_response(rx, sizeof(rx), NULL);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &mdns;
    syn_mdns_task(&pt, &task);
}

void run_dns_tests(void)
{
    RUN_TEST(test_dns_resolve);
    RUN_TEST(test_mdns_responder);
    RUN_TEST(test_dns_resolve_custom_server);
    RUN_TEST(test_dns_resolve_udp_open_fail);
    RUN_TEST(test_dns_resolve_send_fail);
    RUN_TEST(test_dns_resolve_timeout);
    RUN_TEST(test_dns_resolve_cname);
    RUN_TEST(test_mdns_join_fail);
    RUN_TEST(test_mdns_malformed_query);
    RUN_TEST(test_mdns_no_match);
}
