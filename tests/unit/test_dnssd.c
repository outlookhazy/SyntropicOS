/**
 * @file test_dnssd.c
 * @brief Unit tests for DNS-SD (RFC 6763 / RFC 6762) service discovery engine.
 */

#include "mocks/mock_port.h"
#include "syntropic/net/syn_dnssd.h"
#include "syntropic/sched/syn_sched.h"
#include "unity/unity.h"

#include <string.h>

void test_dnssd_init_and_registration(void)
{
    mock_port_reset();

    SYN_DnsSd sd;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_init(&sd));

    /* Null checks on init */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_init(NULL));

    /* Mock socket open failure */
    mock_udp_open_ok = false;
    SYN_DnsSd fail_sd;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_init(&fail_sd));
    mock_udp_open_ok = true;

    /* Mock multicast join failure */
    mock_udp_multicast_join_ok = false;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_init(&fail_sd));
    mock_udp_multicast_join_ok = true;

    SYN_DnsSd_Service svc1 = {.instance_name = "Kitchen Light",
                              .service_type = "_http._tcp",
                              .hostname = "kitchen-node",
                              .port = 80,
                              .ip = {192, 168, 1, 100},
                              .txt_records = {"version=1.0", "vendor=Syntropic"},
                              .txt_count = 2};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&sd, &svc1));

    /* Null parameter checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_register(NULL, &svc1));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_register(&sd, NULL));

    SYN_DnsSd_Service bad_svc = svc1;
    bad_svc.instance_name = NULL;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_register(&sd, &bad_svc));

    bad_svc = svc1;
    bad_svc.service_type = NULL;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_register(&sd, &bad_svc));

    bad_svc = svc1;
    bad_svc.hostname = NULL;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_register(&sd, &bad_svc));

    bad_svc = svc1;
    bad_svc.port = 0;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_register(&sd, &bad_svc));

    /* Single label service type without dots */
    SYN_DnsSd_Service svc2 = svc1;
    svc2.service_type = "myservice";
    svc2.port = 5683;
    svc2.txt_count = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&sd, &svc2));

    SYN_DnsSd_Service svc3 = svc1;
    svc3.service_type = "_mqtt._tcp";
    svc3.port = 1883;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&sd, &svc3));

    SYN_DnsSd_Service svc4 = svc1;
    svc4.service_type = "_ws._tcp";
    svc4.port = 8080;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&sd, &svc4));

    /* 5th registration must fail (table max = 4) */
    SYN_DnsSd_Service svc5 = svc1;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_register(&sd, &svc5));
}

void test_dnssd_query_and_response_generation(void)
{
    mock_port_reset();

    SYN_DnsSd sd;
    syn_dnssd_init(&sd);

    SYN_DnsSd_Service http_svc = {.instance_name = "Syntropic Sensor",
                                  .service_type = "_http._tcp",
                                  .hostname = "sensor-node",
                                  .port = 8080,
                                  .ip = {10, 0, 0, 42},
                                  .txt_records = {"id=123", "model=PRO"},
                                  .txt_count = 2};
    syn_dnssd_register(&sd, &http_svc);

    SYN_DnsSd_Service empty_txt_svc = {.instance_name = "Empty TXT",
                                       .service_type = "_coap._udp",
                                       .hostname = "coap-node",
                                       .port = 5683,
                                       .ip = {10, 0, 0, 43},
                                       .txt_count = 0};
    syn_dnssd_register(&sd, &empty_txt_svc);

    /* Construct query for _http._tcp.local */
    uint8_t query[64] = {
        0x00, 0x00, /* ID 0 */
        0x00, 0x00, /* Flags: Standard Query */
        0x00, 0x01, /* QDCOUNT = 1 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 5,   '_', 'h', 't', 't', 'p',
        4,    '_',  't',  'c',  'p',  5,    'l', 'o', 'c', 'a', 'l', 0, /* Root */
        0x00, 0x0C,                                                     /* QTYPE = PTR (12) */
        0x00, 0x01                                                      /* QCLASS = IN (1) */
    };
    size_t qlen = 12 + 6 + 5 + 6 + 1 + 4;

    uint8_t resp[1024];
    size_t resp_len = 0;

    /* Query matching registered service */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_dnssd_process_query(&sd, query, qlen, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_TRUE(resp_len > 0);

    /* Verify response header */
    uint16_t resp_flags = ((uint16_t)resp[2] << 8) | resp[3];
    TEST_ASSERT_EQUAL_HEX16(0x8400, resp_flags); /* Authoritative Response */
    uint16_t ancount = ((uint16_t)resp[6] << 8) | resp[7];
    TEST_ASSERT_EQUAL_UINT16(1, ancount); /* 1 PTR Answer */
    uint16_t arcount = ((uint16_t)resp[10] << 8) | resp[11];
    TEST_ASSERT_EQUAL_UINT16(3, arcount); /* 3 Additional Records (SRV, TXT, A) */

    /* Query for empty TXT service */
    uint8_t coap_query[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 5,    '_',  'c',  'o',  'a',  'p',
                              4,    '_',  'u',  'd',  'p',  5,    'l',  'o',  'c',
                              'a',  'l',  0,    0x00, 0x0C, 0x00, 0x01};
    size_t coap_qlen = 12 + 6 + 5 + 6 + 1 + 4;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_dnssd_process_query(&sd, coap_query, coap_qlen, resp, sizeof(resp), &resp_len));

    /* Register and query single-label service (no dot) */
    SYN_DnsSd_Service single_svc = {.instance_name = "Single",
                                    .service_type = "single",
                                    .hostname = "single-node",
                                    .port = 80,
                                    .ip = {10, 0, 0, 50},
                                    .txt_count = 0};
    syn_dnssd_register(&sd, &single_svc);
    uint8_t single_query[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 6,    's',  'i',  'n',  'g',  'l',  'e',  5,
                                'l',  'o',  'c',  'a',  'l',  0,    0x00, 0x0C, 0x00, 0x01};
    size_t single_qlen = 12 + 7 + 6 + 1 + 4;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_process_query(&sd, single_query, single_qlen, resp,
                                                          sizeof(resp), &resp_len));

    /* Query for unknown service */
    uint8_t unk_query[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 4,    '_',  'f',  'o',  'o',  4,    '_',  't',  'c',  'p',
                             5,    'l',  'o',  'c',  'a',  'l',  0,    0x00, 0x0C, 0x00, 0x01};
    size_t unk_qlen = 12 + 5 + 5 + 6 + 1 + 4;
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND, syn_dnssd_process_query(&sd, unk_query, unk_qlen, resp,
                                                                 sizeof(resp), &resp_len));

    /* Malformed label query */
    uint8_t bad_label_query[64] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 70,   '_',  'h',  't',  't',  'p' /* Label > 63 */
    };
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND, syn_dnssd_process_query(&sd, bad_label_query, 18, resp,
                                                                 sizeof(resp), &resp_len));

    /* Null and error bounds */
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_dnssd_process_query(NULL, query, qlen, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dnssd_process_query(&sd, NULL, qlen, resp, sizeof(resp), &resp_len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_process_query(&sd, query, 5, resp, sizeof(resp),
                                                             &resp_len)); /* Truncated */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_process_query(&sd, query, qlen, resp, 64,
                                                             &resp_len)); /* Small buffer */

    /* Non-query flag reject */
    uint8_t resp_flag_pkt[64];
    memcpy(resp_flag_pkt, query, qlen);
    resp_flag_pkt[2] = 0x80; /* Set QR bit to 1 */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_process_query(&sd, resp_flag_pkt, qlen, resp,
                                                             sizeof(resp), &resp_len));

    /* qdcount = 0 reject */
    uint8_t zero_qd_pkt[64];
    memcpy(zero_qd_pkt, query, qlen);
    zero_qd_pkt[4] = 0x00;
    zero_qd_pkt[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_dnssd_process_query(&sd, zero_qd_pkt, qlen, resp, sizeof(resp), &resp_len));
}

void test_dnssd_task_execution(void)
{
    mock_port_reset();

    SYN_DnsSd sd;
    syn_dnssd_init(&sd);

    SYN_DnsSd_Service svc = {.instance_name = "Syntropic CoAP",
                             .service_type = "_coap._udp",
                             .hostname = "coap-node",
                             .port = 5683,
                             .ip = {192, 168, 1, 50},
                             .txt_count = 0};
    syn_dnssd_register(&sd, &svc);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &sd};

    /* Run one step with no packets */
    SYN_PT_Status st = syn_dnssd_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_IDLE, st);

    /* Inject packet to UDP socket and run task */
    uint8_t query[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         5,    '_',  'c',  'o',  'a',  'p',  4,    '_',  'u',  'd',  'p',  5,
                         'l',  'o',  'c',  'a',  'l',  0,    0x00, 0x0C, 0x00, 0x01};
    size_t qlen = 12 + 6 + 5 + 6 + 1 + 4;
    SYN_SockAddr from_addr = {.ip = {192, 168, 1, 1}, .port = 5353};
    mock_udp_inject_packet(query, qlen, &from_addr);

    mock_udp_tx_len = 0;
    st = syn_dnssd_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_IDLE, st);
    TEST_ASSERT_GREATER_THAN_size_t(12U, mock_udp_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x84U, mock_udp_tx_buf[2]);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(1U, mock_udp_tx_buf[7]);
}

void test_dnssd_announce(void)
{
    mock_port_reset();

    SYN_DnsSd sd;
    syn_dnssd_init(&sd);

    SYN_DnsSd_Service svc = {.instance_name = "Syntropic Announce",
                             .service_type = "_http._tcp",
                             .hostname = "http-node",
                             .port = 8080,
                             .ip = {192, 168, 1, 100},
                             .txt_count = 0};
    syn_dnssd_register(&sd, &svc);

    uint8_t buf[1024];
    size_t len = 0;

    /* Valid announcement */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_announce(&sd, 0, buf, sizeof(buf), &len));
    TEST_ASSERT_TRUE(len > 0);

    /* Null and bounds checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_announce(NULL, 0, buf, sizeof(buf), &len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dnssd_announce(&sd, 1, buf, sizeof(buf), &len)); /* Out of range */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_announce(&sd, 0, NULL, sizeof(buf), &len));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_announce(&sd, 0, buf, sizeof(buf), NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_announce(&sd, 0, buf, 64, &len)); /* Small buffer */
}

void test_dnssd_query_and_response_parsing(void)
{
    uint8_t qbuf[256];
    size_t qlen = 0;

    /* Build query */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_build_query("_http._tcp", qbuf, sizeof(qbuf), &qlen));
    TEST_ASSERT_TRUE(qlen > 0);

    /* Null and bounds checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_build_query(NULL, qbuf, sizeof(qbuf), &qlen));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dnssd_build_query("_http._tcp", NULL, sizeof(qbuf), &qlen));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_build_query("_http._tcp", qbuf, sizeof(qbuf), NULL));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_build_query("_http._tcp", qbuf, 32, &qlen));

    /* Build response using announce and then parse it back */
    SYN_DnsSd sd;
    syn_dnssd_init(&sd);
    SYN_DnsSd_Service svc = {.instance_name = "Discovered Node",
                             .service_type = "_coap._udp",
                             .hostname = "coap-host",
                             .port = 5683,
                             .ip = {10, 0, 0, 42},
                             .txt_records = {"path=/sensors", "ver=2"},
                             .txt_count = 2};
    syn_dnssd_register(&sd, &svc);

    uint8_t resp[1024];
    size_t rlen = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_announce(&sd, 0, resp, sizeof(resp), &rlen));

    SYN_DnsSd_Discovered disc;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_parse_response(resp, rlen, &disc));
    TEST_ASSERT_EQUAL_STRING("Discovered Node", disc.instance_name);
    TEST_ASSERT_EQUAL_STRING("_coap._udp", disc.service_type);
    TEST_ASSERT_EQUAL_STRING("coap-host", disc.hostname);
    TEST_ASSERT_EQUAL_UINT16(5683, disc.port);
    TEST_ASSERT_EQUAL_UINT8(10, disc.ip[0]);
    TEST_ASSERT_EQUAL_UINT8(42, disc.ip[3]);
    TEST_ASSERT_EQUAL_STRING("path=/sensors", disc.txt);

    /* Discover transmit */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_discover(&sd, "_coap._udp"));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_discover(NULL, "_coap._udp"));
    SYN_DnsSd bad_sd = sd;
    bad_sd.sock = SYN_SOCKET_INVALID;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_discover(&bad_sd, "_coap._udp"));
    uint8_t small_qbuf[16];
    size_t small_qlen = 99U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_build_query("_coap._udp", small_qbuf,
                                                           sizeof(small_qbuf), &small_qlen));
    TEST_ASSERT_EQUAL_size_t(0U, small_qlen);

    /* Error parsing checks */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_parse_response(NULL, rlen, &disc));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_parse_response(resp, 5, &disc));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_parse_response(resp, rlen, NULL));
    resp[2] = 0x00; /* Force QR=0 (query, not response) */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_parse_response(resp, rlen, &disc));

    /* Response with total_rr == 0 */
    uint8_t zero_rr_resp[12] = {0, 0, 0x84, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dnssd_parse_response(zero_rr_resp, sizeof(zero_rr_resp), &disc));

    /* Response with pointer compression and truncated labels */
    uint8_t ptr_resp[32] = {
        0,    0,    0x84, 0,    0,    0,    0,    1,    0,    0,    0, 0, /* Header: 1 answer */
        0xC0, 0x0C,                                                       /* Pointer to offset 12 */
        0x00, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x04,       /* PTR record, rdlen 4 */
        0x03, 'f',  'o',  'o'};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_parse_response(ptr_resp, sizeof(ptr_resp), &disc));

    /* Truncated RR header and truncated RDATA */
    uint8_t trunc_rr_resp[16] = {
        0, 0, 0x84, 0, 0,    0,   0,   1,
        0, 0, 0,    0, 0x03, 'f', 'o', 'o' /* No room for 10-byte RR header */
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_dnssd_parse_response(trunc_rr_resp, sizeof(trunc_rr_resp), &disc));

    uint8_t trunc_rdata_resp[24] = {
        0,    0,    0x84, 0,    0,    0,    0,    1,    0,    0,   0, 0, 0x00, /* Null name */
        0x00, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x20 /* rdlen 32 > remaining */
    };
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_dnssd_parse_response(trunc_rdata_resp, sizeof(trunc_rdata_resp), &disc));

    /* Truncated label length */
    uint8_t trunc_label_resp[20] = {
        0, 0, 0x84, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0x20, 'a', 'b' /* label length 32 > buffer */
    };
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_dnssd_parse_response(trunc_label_resp, sizeof(trunc_label_resp), &disc));
}

static void on_discovered_service(const SYN_DnsSd_Discovered *svc, void *user_data)
{
    int *count = (int *)user_data;
    if (svc != NULL && count != NULL) {
        (*count)++;
    }
}

void test_dnssd_browser_task_coroutine(void)
{
    mock_port_reset();

    SYN_DnsSd sd;
    syn_dnssd_init(&sd);

    SYN_DnsSd_Browser browser;
    int discovered = 0;

    /* Init browser */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_browser_init(&browser, &sd, "_coap._udp",
                                                         on_discovered_service, &discovered, 1000));
    TEST_ASSERT_TRUE(browser.active);

    /* Null and bounds checks */
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR,
        syn_dnssd_browser_init(NULL, &sd, "_coap._udp", on_discovered_service, &discovered, 1000));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dnssd_browser_init(&browser, NULL, "_coap._udp",
                                                 on_discovered_service, &discovered, 1000));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR,
        syn_dnssd_browser_init(&browser, &sd, NULL, on_discovered_service, &discovered, 1000));
    TEST_ASSERT_EQUAL_INT(
        SYN_ERROR, syn_dnssd_browser_init(&browser, &sd, "_coap._udp", NULL, &discovered, 1000));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &browser};

    /* Run coroutine step - no packets, should wait */
    SYN_PT_Status st = syn_dnssd_browse_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_IDLE, st);
    TEST_ASSERT_EQUAL_INT(0, discovered);

    /* Create response packet and inject */
    SYN_DnsSd_Service svc = {.instance_name = "Sensors Node",
                             .service_type = "_coap._udp",
                             .hostname = "sensors-1",
                             .port = 5683,
                             .ip = {192, 168, 1, 50},
                             .txt_count = 0};
    SYN_DnsSd remote_sd;
    syn_dnssd_init(&remote_sd);
    syn_dnssd_register(&remote_sd, &svc);

    uint8_t resp[1024];
    size_t rlen = 0;
    syn_dnssd_announce(&remote_sd, 0, resp, sizeof(resp), &rlen);

    SYN_SockAddr from_addr = {.ip = {192, 168, 1, 50}, .port = 5353};
    mock_udp_inject_packet(resp, rlen, &from_addr);

    /* Run coroutine step - should receive and invoke callback */
    st = syn_dnssd_browse_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_IDLE, st);
    TEST_ASSERT_EQUAL_INT(1, discovered);
    TEST_ASSERT_EQUAL_UINT32(1, browser.discovered_count);

    /* Expire timeout and run coroutine */
    mock_tick_advance(2000);
    st = syn_dnssd_browse_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, st);
    TEST_ASSERT_FALSE(browser.active);
}

void test_dnssd_tasks_guards_and_multi_packet_yielding(void)
{
    mock_port_reset();

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task empty_task = {0};

    /* 1. syn_dnssd_browse_task NULL & browser->sd == NULL guards (syn_dnssd.c:565-571) */
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dnssd_browse_task(NULL, &empty_task));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dnssd_browse_task(&pt, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dnssd_browse_task(&pt, &empty_task));

    SYN_DnsSd_Browser browser = {0};
    browser.sd = NULL;
    browser.active = true;
    SYN_Task task = {0};
    task.user_data = &browser;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dnssd_browse_task(&pt, &task));

    /* 2. syn_dnssd_browse_task with sd->sock == SYN_SOCKET_INVALID (syn_dnssd.c:576-579) */
    SYN_DnsSd sd = {0};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_init(&sd));
    int discovered = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_browser_init(&browser, &sd, "_coap._udp",
                                                         on_discovered_service, &discovered, 1000));
    sd.sock = SYN_SOCKET_INVALID;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dnssd_browse_task(&pt, &task));
    TEST_ASSERT_FALSE(browser.active);

    /* 3. syn_dnssd_browse_task multi-packet queue hits PT_CONTINUE_IF_NEEDED (syn_dnssd.c:605) */
    mock_port_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_init(&sd));
    discovered = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_browser_init(&browser, &sd, "_coap._udp",
                                                         on_discovered_service, &discovered, 1000));

    SYN_DnsSd remote_sd;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_init(&remote_sd));
    SYN_DnsSd_Service svc = {.instance_name = "NodeA",
                             .service_type = "_coap._udp",
                             .hostname = "node-a",
                             .port = 5683,
                             .ip = {192, 168, 1, 60},
                             .txt_count = 0};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&remote_sd, &svc));
    uint8_t resp[512];
    size_t rlen = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_announce(&remote_sd, 0, resp, sizeof(resp), &rlen));

    SYN_SockAddr from_addr = {.ip = {192, 168, 1, 60}, .port = 5353};
    mock_udp_rx_count = 0;
    mock_udp_rx_pos = 0;
    mock_udp_inject_packet(resp, rlen, &from_addr);
    mock_udp_inject_packet(resp, rlen, &from_addr);

    SYN_Sched sched = {0};
    memset(&task, 0, sizeof(task));
    task.user_data = &browser;
    task.sched = &sched;
    sched.running = &task;
    sched.share_contended = true;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_MORE, syn_dnssd_browse_task(&pt, &task));
    TEST_ASSERT_EQUAL_INT(1, discovered);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dnssd_browse_task(&pt, &task));
    TEST_ASSERT_EQUAL_INT(2, discovered);

    /* 4. syn_dnssd_task NULL guards, invalid sock, and multi-packet PT_CONTINUE_IF_NEEDED
     *    (syn_dnssd.c:621-631, 650-652) */
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dnssd_task(NULL, &empty_task));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dnssd_task(&pt, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dnssd_task(&pt, &empty_task));

    SYN_DnsSd bad_sd = {0};
    bad_sd.sock = SYN_SOCKET_INVALID;
    memset(&task, 0, sizeof(task));
    task.user_data = &bad_sd;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_dnssd_task(&pt, &task));

    /* Queue 2 discovery queries into remote_sd and step syn_dnssd_task with contended sched */
    mock_port_reset();
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_init(&remote_sd));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&remote_sd, &svc));
    mock_udp_tx_len = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_discover(&sd, "_coap._udp"));
    uint8_t query_pkt[256];
    size_t query_len = mock_udp_tx_len;
    TEST_ASSERT_TRUE(query_len > 0 && query_len <= sizeof(query_pkt));
    memcpy(query_pkt, mock_udp_tx_buf, query_len);

    mock_udp_tx_len = 0;
    mock_udp_rx_count = 0;
    mock_udp_rx_pos = 0;
    mock_udp_inject_packet(query_pkt, query_len, &from_addr);
    mock_udp_inject_packet(query_pkt, query_len, &from_addr);

    memset(&sched, 0, sizeof(sched));
    memset(&task, 0, sizeof(task));
    task.user_data = &remote_sd;
    task.sched = &sched;
    sched.running = &task;
    sched.share_contended = true;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_MORE, syn_dnssd_task(&pt, &task));
    size_t tx_after_first = mock_udp_tx_len;
    TEST_ASSERT_TRUE(tx_after_first > 0);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dnssd_task(&pt, &task));
    TEST_ASSERT_TRUE(mock_udp_tx_len > tx_after_first);
}

void test_dnssd_round3_regressions(void)
{
    mock_port_reset();

    SYN_DnsSd sd;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_init(&sd));

    /* 1. R3-N05: syn_dnssd_register rejects txt_count > SYN_DNSSD_MAX_TXT_RECORDS and NULL TXT */
    SYN_DnsSd_Service bad_txt = {.instance_name = "Node",
                                 .service_type = "_http._tcp",
                                 .hostname = "node",
                                 .port = 80,
                                 .ip = {10, 0, 0, 1},
                                 .txt_records = {NULL},
                                 .txt_count = 1U};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_register(&sd, &bad_txt));

    bad_txt.txt_records[0] = "k=v";
    bad_txt.txt_count = SYN_DNSSD_MAX_TXT_RECORDS + 1U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_register(&sd, &bad_txt));

    /* 2. R3-N05: 70-char TXT clamping (>63) + exact per-record truncation bounds in
     * pack_service_response */
    const char *long_txt =
        "attr=0123456789012345678901234567890123456789012345678901234567890123456789";
    SYN_DnsSd_Service big_svc = {.instance_name = "BigNode",
                                 .service_type = "_http._tcp",
                                 .hostname = "bignode",
                                 .port = 8080,
                                 .ip = {10, 0, 0, 2},
                                 .txt_records = {long_txt, long_txt, long_txt, long_txt, long_txt,
                                                 long_txt, long_txt, long_txt},
                                 .txt_count = 8U};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&sd, &big_svc));

    uint8_t full_buf[1500];
    size_t full_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_dnssd_announce(&sd, 0U, full_buf, sizeof(full_buf), &full_len));
    TEST_ASSERT_GREATER_THAN_size_t(512U, full_len);

    /* Exhaustively test every truncation boundary 0 .. full_len - 1 without OOB writes */
    uint8_t guard_buf[1502];
    for (size_t cap = 0U; cap < full_len; cap++) {
        guard_buf[cap] = 0xA5U;
        size_t out_len = 99U;
        TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_announce(&sd, 0U, guard_buf, cap, &out_len));
        TEST_ASSERT_EQUAL_size_t(0U, out_len);
        TEST_ASSERT_EQUAL_HEX8(0xA5U, guard_buf[cap]);
    }

    /* Empty-TXT truncation across every byte boundary 0 .. empty_full - 1 */
    SYN_DnsSd sd_empty;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_init(&sd_empty));
    SYN_DnsSd_Service empty_svc = {.instance_name = "E",
                                   .service_type = "_coap._udp",
                                   .hostname = "e",
                                   .port = 5683,
                                   .ip = {10, 0, 0, 3},
                                   .txt_count = 0U};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&sd_empty, &empty_svc));
    size_t empty_full = 0U;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_dnssd_announce(&sd_empty, 0U, full_buf, sizeof(full_buf), &empty_full));
    for (size_t cap = 0U; cap < empty_full; cap++) {
        size_t trunc_txt_len = 0U;
        TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                              syn_dnssd_announce(&sd_empty, 0U, guard_buf, cap, &trunc_txt_len));
    }

    /* 3. R3-N05: Multi-question mDNS query with 0xC0 pointer on Q1 still matches Q2 */
    uint8_t comp_query[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            /* Question 1: _other + pointer 0xC0 0x0C + QTYPE PTR + QCLASS IN */
                            6, '_', 'o', 't', 'h', 'e', 'r', 0xC0, 0x0C, 0x00, 0x0C, 0x00, 0x01,
                            /* Question 2: _http._tcp.local + QTYPE PTR + QCLASS IN */
                            5, '_', 'h', 't', 't', 'p', 4, '_', 't', 'c', 'p', 5, 'l', 'o', 'c',
                            'a', 'l', 0, 0x00, 0x0C, 0x00, 0x01};
    size_t comp_resp_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_dnssd_process_query(&sd, comp_query, sizeof(comp_query), full_buf,
                                                  sizeof(full_buf), &comp_resp_len));
    TEST_ASSERT_EQUAL_size_t(full_len, comp_resp_len);

    /* Truncated 1-byte 0xC0 pointer and truncated QTYPE/QCLASS rejected safely */
    uint8_t trunc_ptr_q[] = {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0xC0};
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND,
                          syn_dnssd_process_query(&sd, trunc_ptr_q, sizeof(trunc_ptr_q), full_buf,
                                                  sizeof(full_buf), &comp_resp_len));
    uint8_t trunc_qtype[] = {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 3, 'f', 'o', 'o', 0, 0x00, 0x0C};
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND,
                          syn_dnssd_process_query(&sd, trunc_qtype, sizeof(trunc_qtype), full_buf,
                                                  sizeof(full_buf), &comp_resp_len));
    uint8_t unterminated_q[] = {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 3, 'f', 'o', 'o'};
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND,
                          syn_dnssd_process_query(&sd, unterminated_q, sizeof(unterminated_q),
                                                  full_buf, sizeof(full_buf), &comp_resp_len));

    /* 4. R3-N06: UDP recvfrom failure while readable yields PT_IDLE in both tasks + PT_END
     * fallthrough */
    SYN_DnsSd_Browser browser;
    int disc_cnt = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_browser_init(&browser, &sd, "_http._tcp",
                                                         on_discovered_service, &disc_cnt, 1000U));
    mock_udp_recv_fail = true;

    SYN_PT bpt;
    PT_INIT(&bpt);
    SYN_Task btask = {.user_data = &browser};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dnssd_browse_task(&bpt, &btask));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dnssd_browse_task(&bpt, &btask));

    SYN_PT dpt;
    PT_INIT(&dpt);
    SYN_Task dtask = {.user_data = &sd};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dnssd_task(&dpt, &dtask));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dnssd_task(&dpt, &dtask));
    mock_udp_recv_fail = false;

    dpt.lc = 9999;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_dnssd_task(&dpt, &dtask));

    /* 5. Corrupted service slot guard, malformed browse response, and inactive browse wake */
    uint8_t bad_pkt[16] = {0};
    SYN_SockAddr bad_from = {.ip = {192, 168, 1, 99}, .port = 5353};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_browser_init(&browser, &sd, "_http._tcp",
                                                         on_discovered_service, &disc_cnt, 1000U));
    mock_udp_set_response(bad_pkt, sizeof(bad_pkt), &bad_from);
    PT_INIT(&bpt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_dnssd_browse_task(&bpt, &btask));
    TEST_ASSERT_EQUAL(0, disc_cnt);

    browser.active = false;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_dnssd_browse_task(&bpt, &btask));

    sd.services[0].txt_count = 99U;
    size_t bad_svc_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dnssd_announce(&sd, 0U, full_buf, sizeof(full_buf), &bad_svc_len));
}

void test_dnssd_round4_compression_qdcount_and_rdata_bounds(void)
{
    mock_port_reset();

    SYN_DnsSd sd;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_init(&sd));
    SYN_DnsSd_Service svc = {.instance_name = "NodeComp",
                             .service_type = "_http._tcp",
                             .hostname = "compnode",
                             .port = 8080,
                             .ip = {192, 168, 1, 77},
                             .txt_records = {"k=v"},
                             .txt_count = 1U};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&sd, &svc));

    /* 1. Compressed QNAME in Q2 referencing "_tcp.local" at offset 19 in Q1 */
    uint8_t comp_qname_pkt[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* offset 12: 6 "_other" + offset 19: 4 "_tcp" 5 "local" 0 + QTYPE/QCLASS */
        6, '_', 'o', 't', 'h', 'e', 'r', 4, '_', 't', 'c', 'p', 5, 'l', 'o', 'c', 'a', 'l', 0, 0x00,
        0x0C, 0x00, 0x01,
        /* Q2: 5 "_http" + pointer 0xC0 0x13 (offset 19) + QTYPE PTR + QCLASS IN */
        5, '_', 'h', 't', 't', 'p', 0xC0, 0x13, 0x00, 0x0C, 0x00, 0x01};
    uint8_t resp_out[512];
    size_t resp_out_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_dnssd_process_query(&sd, comp_qname_pkt, sizeof(comp_qname_pkt),
                                                  resp_out, sizeof(resp_out), &resp_out_len));
    TEST_ASSERT_GREATER_THAN_size_t(12U, resp_out_len);

    /* Invalid 0x80 label byte in query rejected */
    uint8_t bad_label_bits[] = {0, 0, 0,    0,   0,    1,    0,    0,    0,   0,
                                0, 0, 0x80, 'a', 0x00, 0x00, 0x0C, 0x00, 0x01};
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND,
                          syn_dnssd_process_query(&sd, bad_label_bits, sizeof(bad_label_bits),
                                                  resp_out, sizeof(resp_out), &resp_out_len));

    /* 2. Compressed mDNS response with QDCOUNT=1, ANCOUNT=1, NSCOUNT=1, ARCOUNT=3 */
    uint8_t comp_resp[] = {
        0x00, 0x00, 0x84, 0x00, 0x00, 0x01, /* QDCOUNT = 1 */
        0x00, 0x01,                         /* ANCOUNT = 1 */
        0x00, 0x01,                         /* NSCOUNT = 1 */
        0x00, 0x03,                         /* ARCOUNT = 3 */
        /* offset 12: Question _http._tcp.local (offset 12="_http._tcp.local", offset 23="local") */
        5, '_', 'h', 't', 't', 'p', 4, '_', 't', 'c', 'p', 5, 'l', 'o', 'c', 'a', 'l', 0, 0x00,
        0x0C, 0x00, 0x01,
        /* offset 34: Answer PTR: owner 0xC0 0x0C, rdlen=11: 8 "NodeComp" + 0xC0 0x0C */
        0xC0, 0x0C, 0x00, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x0B,
        /* offset 46: PTR RDATA */
        8, 'N', 'o', 'd', 'e', 'C', 'o', 'm', 'p', 0xC0, 0x0C,
        /* offset 57: Authority NSEC (type 47): owner 0xC0 0x2E, rdlen=2, skipped cleanly */
        0xC0, 0x2E, 0x00, 0x2F, 0x00, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x02, 0x00, 0x00,
        /* offset 71: Additional SRV: owner 0xC0 0x2E, rdlen=17: port=8080 + "compnode" + 0xC0 0x17
         */
        0xC0, 0x2E, 0x00, 0x21, 0x80, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x11, 0x00, 0x00, 0x00,
        0x00, 0x1F, 0x90, 8, 'c', 'o', 'm', 'p', 'n', 'o', 'd', 'e', 0xC0, 0x17,
        /* offset 100: Additional TXT: owner 0xC0 0x2E, rdlen=4: 3 "k=v" */
        0xC0, 0x2E, 0x00, 0x10, 0x80, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x04, 3, 'k', '=', 'v',
        /* offset 116: Additional A: owner 8 "compnode" + 0xC0 0x17, rdlen=4 */
        8, 'c', 'o', 'm', 'p', 'n', 'o', 'd', 'e', 0xC0, 0x17, 0x00, 0x01, 0x80, 0x01, 0x00, 0x00,
        0x00, 0x78, 0x00, 0x04, 192, 168, 1, 77};

    SYN_DnsSd_Discovered disc;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_parse_response(comp_resp, sizeof(comp_resp), &disc));
    TEST_ASSERT_EQUAL_STRING("NodeComp", disc.instance_name);
    TEST_ASSERT_EQUAL_STRING("_http._tcp", disc.service_type);
    TEST_ASSERT_EQUAL_STRING("compnode", disc.hostname);
    TEST_ASSERT_EQUAL_UINT16(8080U, disc.port);
    TEST_ASSERT_EQUAL_STRING("k=v", disc.txt);
    TEST_ASSERT_EQUAL_UINT8(192U, disc.ip[0]);
    TEST_ASSERT_EQUAL_UINT8(77U, disc.ip[3]);

    /* 3. Malformed question with QDCOUNT=1 fails syn_dnssd_parse_response */
    uint8_t bad_qd_resp[] = {0, 0, 0x84, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0xC0};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR,
                          syn_dnssd_parse_response(bad_qd_resp, sizeof(bad_qd_resp), &disc));

    /* 4. RDATA overread guards, pointer bounds/hops, single-label PTR/SRV, and short RR header */
    uint8_t rdata_bounds_resp[] = {
        0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
        /* RR1: PTR with rdlength=2 but label claims 10 bytes -> rejected without overreading RR2 */
        0x00, 0x00, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x02, 10, 'X',
        /* RR2: PTR with single-label "nodot" (covers first_dot == NULL) */
        0x00, 0x00, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x07, 5, 'n', 'o', 'd', 'o',
        't', 0,
        /* RR3: SRV with rdlength=6 (zero target bytes -> rejected without setting port) */
        0x00, 0x00, 0x21, 0x80, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
        0x22, 0xB8,
        /* RR4: SRV with single-label target "plainhost" (covers first_dot == NULL false branch) */
        0x00, 0x00, 0x21, 0x80, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00,
        0x13, 0x88, 9, 'p', 'l', 'a', 'i', 'n', 'h', 'o', 's', 't', 0,
        /* RR5: PTR with out-of-bounds compression pointer 0xC0 0xFF in RDATA */
        0x00, 0x00, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x02, 0xC0, 0xFF,
        /* RR6: Valid root owner 0x00 followed by < 10 bytes (covers resp_len - pos < 10U) */
        0x00, 0x00, 0x01};
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_dnssd_parse_response(rdata_bounds_resp, sizeof(rdata_bounds_resp), &disc));
    TEST_ASSERT_EQUAL_STRING("", disc.instance_name);
    TEST_ASSERT_EQUAL_UINT16(5000U, disc.port);
    TEST_ASSERT_EQUAL_STRING("plainhost", disc.hostname);

    /* Truncated 1-byte 0xC0 inside RDATA */
    uint8_t trunc_c0_rdata[] = {0,    0,    0x84, 0,    0,    0,    0,    1,
                                0,    0,    0,    0,    0x00, 0x00, 0x0C, 0x00,
                                0x01, 0x00, 0x00, 0x00, 0x78, 0x00, 0x01, 0xC0};
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_dnssd_parse_response(trunc_c0_rdata, sizeof(trunc_c0_rdata), &disc));
}

void test_r5_n13_dnssd_build_query_truncation_bounds(void)
{
    uint8_t qbuf[128];
    size_t full_qlen = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_dnssd_build_query("_http._tcp", qbuf, sizeof(qbuf), &full_qlen));
    TEST_ASSERT_EQUAL_size_t(34U, full_qlen);

    /* Sweep every truncation boundary 0 .. full_qlen - 1 (covers header, QNAME, QTYPE, QCLASS) */
    uint8_t guard[64];
    for (size_t cap = 0U; cap < full_qlen; cap++) {
        guard[cap] = 0x5AU;
        size_t out_len = 77U;
        TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_build_query("_http._tcp", guard, cap, &out_len));
        TEST_ASSERT_EQUAL_size_t(0U, out_len);
        TEST_ASSERT_EQUAL_HEX8(0x5AU, guard[cap]);
    }

    size_t exact_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_dnssd_build_query("_http._tcp", guard, full_qlen, &exact_len));
    TEST_ASSERT_EQUAL_size_t(full_qlen, exact_len);
}

static void test_tier_a_dnssd_qtype_case_dot_and_errors(void)
{
    mock_port_reset();

    SYN_DnsSd sd;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_init(&sd));
    SYN_DnsSd_Service svc = {
        .instance_name = "Syntropic.v1.2",
        .service_type = "_http._tcp",
        .hostname = "dot-node",
        .port = 8080U,
        .ip = {10U, 20U, 30U, 40U},
        .txt_records = {"ver=1.2"},
        .txt_count = 1U,
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_register(&sd, &svc));

    /* 1. Case-insensitive service matching + QTYPE/QCLASS validation */
    uint8_t q_pkt[34] = {0U};
    q_pkt[5] = 1U;
    q_pkt[12] = 5U;
    memcpy(&q_pkt[13], "_HTTP", 5U);
    q_pkt[18] = 4U;
    memcpy(&q_pkt[19], "_TcP", 4U);
    q_pkt[23] = 5U;
    memcpy(&q_pkt[24], "LOCAL", 5U);
    q_pkt[31] = 12U;
    q_pkt[32] = 0x80U;
    q_pkt[33] = 1U;
    uint8_t resp[512];
    size_t rlen = 0U;

    /* Valid QTYPEs: PTR (12) + QU bit, SRV (33), TXT (16), ANY (255) + QCLASS ANY (255) */
    SYN_Status st = syn_dnssd_process_query(&sd, q_pkt, sizeof(q_pkt), resp, sizeof(resp), &rlen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    q_pkt[31] = 33U;
    q_pkt[32] = 0U;
    q_pkt[33] = 1U;
    st = syn_dnssd_process_query(&sd, q_pkt, sizeof(q_pkt), resp, sizeof(resp), &rlen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    q_pkt[31] = 16U;
    st = syn_dnssd_process_query(&sd, q_pkt, sizeof(q_pkt), resp, sizeof(resp), &rlen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    q_pkt[31] = 255U;
    q_pkt[33] = 255U;
    st = syn_dnssd_process_query(&sd, q_pkt, sizeof(q_pkt), resp, sizeof(resp), &rlen);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);

    /* Rejected QTYPE = AAAA (28) and rejected QCLASS = CH (3) */
    q_pkt[31] = 28U;
    q_pkt[33] = 1U;
    st = syn_dnssd_process_query(&sd, q_pkt, sizeof(q_pkt), resp, sizeof(resp), &rlen);
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND, st);
    q_pkt[31] = 12U;
    q_pkt[33] = 3U;
    st = syn_dnssd_process_query(&sd, q_pkt, sizeof(q_pkt), resp, sizeof(resp), &rlen);
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND, st);

    uint8_t bad_qptr[18] = {0U};
    bad_qptr[5] = 1U;
    bad_qptr[12] = 0xC0U;
    bad_qptr[13] = 0xFFU;
    st = syn_dnssd_process_query(&sd, bad_qptr, sizeof(bad_qptr), resp, sizeof(resp), &rlen);
    TEST_ASSERT_EQUAL_INT(SYN_NOT_FOUND, st);

    /* 2. Announce + parse preserves literal '.' in <Instance> label */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_announce(&sd, 0U, resp, sizeof(resp), &rlen));
    SYN_DnsSd_Discovered disc;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_parse_response(resp, rlen, &disc));
    TEST_ASSERT_EQUAL_STRING("Syntropic.v1.2", disc.instance_name);
    TEST_ASSERT_EQUAL_STRING("_http._tcp", disc.service_type);

    /* 3. PTR RDATA edge cases: 0-length RDATA, pointer loop (>8 hops), OOB ptr, and compressed */
    uint8_t ptr_edges[49] = {0U};
    ptr_edges[2] = 0x84U;
    ptr_edges[7] = 3U;
    /* RR1 (offset 12): PTR rdlen=0 (p >= limit) */
    const uint8_t rr_ptr0[11] = {0U, 0U, 0x0CU, 0U, 1U, 0U, 0U, 0U, 60U, 0U, 0U};
    /* RR2 (offset 23): PTR rdlen=2 at RDATA offset 34 (0x22) self-loop 0xC0 0x22 */
    const uint8_t rr_loop[13] = {0U, 0U, 0x0CU, 0U, 1U, 0U, 0U, 0U, 60U, 0U, 2U, 0xC0U, 0x22U};
    /* RR3 (offset 36): PTR rdlen=2 out-of-bounds target 0xC0 0xFF (>= msg_len) */
    const uint8_t rr_oob[13] = {0U, 0U, 0x0CU, 0U, 1U, 0U, 0U, 0U, 60U, 0U, 2U, 0xC0U, 0xFFU};
    memcpy(&ptr_edges[12], rr_ptr0, sizeof(rr_ptr0));
    memcpy(&ptr_edges[23], rr_loop, sizeof(rr_loop));
    memcpy(&ptr_edges[36], rr_oob, sizeof(rr_oob));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_dnssd_parse_response(ptr_edges, sizeof(ptr_edges), &disc));
    TEST_ASSERT_EQUAL_STRING("", disc.instance_name);

    uint8_t ptr_comp_pkt[67] = {0U};
    ptr_comp_pkt[2] = 0x84U;
    ptr_comp_pkt[7] = 2U;
    ptr_comp_pkt[12] = 8U;
    memcpy(&ptr_comp_pkt[13], "Ptr.Inst", 8U);
    ptr_comp_pkt[21] = 5U;
    memcpy(&ptr_comp_pkt[22], "_http", 5U);
    ptr_comp_pkt[27] = 4U;
    memcpy(&ptr_comp_pkt[28], "_tcp", 4U);
    ptr_comp_pkt[32] = 5U;
    memcpy(&ptr_comp_pkt[33], "local", 5U);
    const uint8_t rr_txt[14] = {0U, 0x10U, 0x80U, 1U, 0U, 0U, 0U, 60U, 0U, 4U, 3U, 'a', '=', '1'};
    const uint8_t rr_cptr[14] = {0xC0U, 21U, 0U, 12U, 0U, 1U, 0U, 0U, 0U, 60U, 0U, 2U, 0xC0U, 12U};
    memcpy(&ptr_comp_pkt[39], rr_txt, sizeof(rr_txt));
    memcpy(&ptr_comp_pkt[53], rr_cptr, sizeof(rr_cptr));
    st = syn_dnssd_parse_response(ptr_comp_pkt, sizeof(ptr_comp_pkt), &disc);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);
    TEST_ASSERT_EQUAL_STRING("Ptr.Inst", disc.instance_name);
    TEST_ASSERT_EQUAL_STRING("_http._tcp", disc.service_type);

    /* 4. Error propagation in syn_dnssd_discover, syn_dnssd_browser_init, and syn_dnssd_announce */
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_discover(&sd, ""));
    SYN_DnsSd_Browser browser;
    int cnt = 0;
    st = syn_dnssd_browser_init(&browser, &sd, "", on_discovered_service, &cnt, 500U);
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, st);
    TEST_ASSERT_FALSE(browser.active);

    mock_udp_sendto_fail = true;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_discover(&sd, "_http._tcp"));
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_dnssd_announce(&sd, 0U, resp, sizeof(resp), &rlen));
    mock_udp_sendto_fail = false;
}

void run_dnssd_tests(void)
{
    RUN_TEST(test_dnssd_init_and_registration);
    RUN_TEST(test_dnssd_query_and_response_generation);
    RUN_TEST(test_dnssd_task_execution);
    RUN_TEST(test_dnssd_announce);
    RUN_TEST(test_dnssd_query_and_response_parsing);
    RUN_TEST(test_dnssd_browser_task_coroutine);
    RUN_TEST(test_dnssd_tasks_guards_and_multi_packet_yielding);
    RUN_TEST(test_dnssd_round3_regressions);
    RUN_TEST(test_dnssd_round4_compression_qdcount_and_rdata_bounds);
    RUN_TEST(test_r5_n13_dnssd_build_query_truncation_bounds);
    RUN_TEST(test_tier_a_dnssd_qtype_case_dot_and_errors);
}
