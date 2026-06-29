/**
 * @file test_coap.c
 * @brief Unity tests for CoAP message serialization and parsing.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

static void test_coap_serialization(void)
{
    SYN_CoapMsg req;
    req.type = COAP_TYPE_CON;
    req.code = COAP_CODE_GET;
    req.msg_id = 0x1234;
    req.token_len = 2;
    req.token[0] = 0xAB;
    req.token[1] = 0xCD;
    req.payload = (const uint8_t *)"hello";
    req.payload_len = 5;

    SYN_CoapOption options[2];
    options[0].num = COAP_OPT_URI_PATH;
    options[0].val = (const uint8_t *)"test";
    options[0].len = 4;

    /* Binary content-format option, e.g. text/plain (value 0) */
    uint8_t fmt_val = 0;
    options[1].num = COAP_OPT_CONTENT_FORMAT;
    options[1].val = &fmt_val;
    options[1].len = 1;

    uint8_t buffer[128];
    size_t len = syn_coap_serialize(&req, options, 2, buffer, sizeof(buffer));
    TEST_ASSERT_TRUE(len > 0);

    /* Parse back */
    SYN_CoapMsg resp;
    SYN_CoapOption parsed_options[8];
    size_t parsed_option_count = 0;
    SYN_Status st = syn_coap_parse(&resp, parsed_options, 8, &parsed_option_count, buffer, len);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    TEST_ASSERT_EQUAL_UINT8(COAP_TYPE_CON, resp.type);
    TEST_ASSERT_EQUAL_UINT8(COAP_CODE_GET, resp.code);
    TEST_ASSERT_EQUAL_UINT16(0x1234, resp.msg_id);
    TEST_ASSERT_EQUAL_UINT8(2, resp.token_len);
    TEST_ASSERT_EQUAL_UINT8(0xAB, resp.token[0]);
    TEST_ASSERT_EQUAL_UINT8(0xCD, resp.token[1]);

    TEST_ASSERT_EQUAL_INT(2, parsed_option_count);
    /* Uri-Path (11) and Content-Format (12) must be sorted */
    TEST_ASSERT_EQUAL_UINT16(COAP_OPT_URI_PATH, parsed_options[0].num);
    TEST_ASSERT_EQUAL_INT(4, parsed_options[0].len);
    TEST_ASSERT_EQUAL_UINT16(COAP_OPT_CONTENT_FORMAT, parsed_options[1].num);
    TEST_ASSERT_EQUAL_INT(1, parsed_options[1].len);

    TEST_ASSERT_EQUAL_INT(5, resp.payload_len);
    TEST_ASSERT_EQUAL_MEMORY("hello", resp.payload, 5);
}

static void test_coap_extended_options(void)
{
    SYN_CoapMsg req = {
        .type = COAP_TYPE_CON,
        .code = COAP_CODE_GET,
        .msg_id = 0x1234,
        .token_len = 0,
        .payload_len = 0
    };

    SYN_CoapOption options[1];
    /* Proxy-Uri is option 35, requiring an extended delta because 35 > 12 */
    options[0].num = COAP_OPT_PROXY_URI;
    options[0].val = (const uint8_t *)"http://ext";
    options[0].len = 10;

    uint8_t buffer[128];
    size_t len = syn_coap_serialize(&req, options, 1, buffer, sizeof(buffer));
    TEST_ASSERT_TRUE(len > 0);

    /* Parse back and verify */
    SYN_CoapMsg resp;
    SYN_CoapOption parsed_options[4];
    size_t parsed_count = 0;
    SYN_Status st = syn_coap_parse(&resp, parsed_options, 4, &parsed_count, buffer, len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(1, parsed_count);
    TEST_ASSERT_EQUAL_UINT16(COAP_OPT_PROXY_URI, parsed_options[0].num);
    TEST_ASSERT_EQUAL_INT(10, parsed_options[0].len);
    TEST_ASSERT_EQUAL_MEMORY("http://ext", parsed_options[0].val, 10);
}

static void test_coap_request_task_success(void)
{
    /* Setup mock response packet */
    SYN_CoapMsg resp_msg = {
        .type = COAP_TYPE_ACK,
        .code = COAP_RESP_CONTENT,
        .msg_id = 0x5555,
        .token_len = 2,
        .token = {0x11, 0x22},
        .payload = (const uint8_t *)"payload",
        .payload_len = 7
    };
    uint8_t resp_raw[64];
    size_t resp_raw_len = syn_coap_serialize(&resp_msg, NULL, 0, resp_raw, sizeof(resp_raw));
    TEST_ASSERT_TRUE(resp_raw_len > 0);

    SYN_SockAddr from = { .ip = {127, 0, 0, 1}, .port = 5683 };
    mock_udp_set_response(resp_raw, resp_raw_len, &from);

    /* Setup CoAP request */
    SYN_CoapMsg req_msg = {
        .type = COAP_TYPE_CON,
        .code = COAP_CODE_GET,
        .msg_id = 0x5555,
        .token_len = 2,
        .token = {0x11, 0x22},
        .payload_len = 0
    };

    SYN_CoapRequest req;
    memset(&req, 0, sizeof(req));
    req.server_addr = from;
    req.req_msg = &req_msg;
    req.timeout_ms = 100;
    req.retries = 2;

    SYN_Sched sched;
    SYN_Task task;
    syn_task_create(&task, "coap", syn_coap_request_task, 0, &req);
    syn_sched_init(&sched, &task, 1);

    /* Run scheduler loop */
    bool alive = true;
    uint32_t start = syn_port_get_tick_ms();
    while (alive && (syn_port_get_tick_ms() - start < 1000)) {
        alive = syn_sched_run(&sched);
        mock_tick_advance(10);
    }

    TEST_ASSERT_EQUAL(SYN_OK, req.status);
    TEST_ASSERT_EQUAL_UINT8(COAP_RESP_CONTENT, req.resp_msg.code);
    TEST_ASSERT_EQUAL_INT(7, req.resp_msg.payload_len);
    TEST_ASSERT_EQUAL_MEMORY("payload", req.resp_msg.payload, 7);
}

void run_coap_tests(void)
{
    RUN_TEST(test_coap_serialization);
    RUN_TEST(test_coap_extended_options);
    RUN_TEST(test_coap_request_task_success);
}
