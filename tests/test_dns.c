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

void run_dns_tests(void)
{
    RUN_TEST(test_dns_resolve);
    RUN_TEST(test_mdns_responder);
}
