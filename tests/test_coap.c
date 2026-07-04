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
    /* Binary content-format option, e.g. text/plain (value 0) */
    uint8_t fmt_val = 0;
    options[0].num = COAP_OPT_CONTENT_FORMAT;
    options[0].val = &fmt_val;
    options[0].len = 1;

    options[1].num = COAP_OPT_URI_PATH;
    options[1].val = (const uint8_t *)"test";
    options[1].len = 4;

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
    syn_coap_request_init(&req, &from, &req_msg, 100, 2);

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

static void test_coap_serialization_boundaries(void)
{
    SYN_CoapMsg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = COAP_TYPE_CON;
    msg.code = COAP_CODE_GET;
    msg.msg_id = 0x1234;

    /* 1. Delta >= 269, len_ext_len = 1 and len_ext_len = 2 */
    SYN_CoapOption options[2];
    /* Option 1: num = 300 (delta = 300) */
    uint8_t val_large[300];
    memset(val_large, 'A', sizeof(val_large));
    options[0].num = 300;
    options[0].val = val_large;
    options[0].len = 300; // exercises len_val = 14, len_ext_len = 2

    /* Option 2: num = 350 (delta = 50) */
    uint8_t val_medium[50];
    memset(val_medium, 'B', sizeof(val_medium));
    options[1].num = 350;
    options[1].val = val_medium;
    options[1].len = 50; // exercises len_val = 13, len_ext_len = 1

    uint8_t buf[1024];
    size_t len = syn_coap_serialize(&msg, options, 2, buf, sizeof(buf));
    TEST_ASSERT_TRUE(len > 0);

    /* Parse back */
    SYN_CoapMsg resp;
    SYN_CoapOption parsed_options[20];
    size_t parsed_option_count = 0;
    SYN_Status st = syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, buf, len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(2, parsed_option_count);
    TEST_ASSERT_EQUAL_UINT16(300, parsed_options[0].num);
    TEST_ASSERT_EQUAL_INT(300, parsed_options[0].len);
    TEST_ASSERT_EQUAL_UINT16(350, parsed_options[1].num);
    TEST_ASSERT_EQUAL_INT(50, parsed_options[1].len);

    /* 2. Buffer overflows */
    /* A. max_buf_len < 4 */
    TEST_ASSERT_EQUAL_INT(0, syn_coap_serialize(&msg, NULL, 0, buf, 3));
    /* B. max_buf_len too small for options */
    TEST_ASSERT_EQUAL_INT(0, syn_coap_serialize(&msg, options, 1, buf, 10));
    /* C. max_buf_len too small for payload */
    msg.payload = (const uint8_t *)"hello";
    msg.payload_len = 5;
    TEST_ASSERT_EQUAL_INT(0, syn_coap_serialize(&msg, NULL, 0, buf, 9)); // header 4 + payload 5 + marker 1 = 10 needed

    /* 3. Option count > 16 */
    SYN_CoapOption excess_options[18];
    for (int i = 0; i < 18; i++) {
        excess_options[i].num = (uint16_t)(i + 1);
        excess_options[i].val = (const uint8_t *)"x";
        excess_options[i].len = 1;
    }
    msg.payload_len = 0;
    len = syn_coap_serialize(&msg, excess_options, 18, buf, sizeof(buf));
    TEST_ASSERT_TRUE(len > 0);
    st = syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, buf, len);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    /* Only 16 options should be encoded */
    TEST_ASSERT_EQUAL_INT(16, parsed_option_count);

    /* 4. Parse failures */
    /* A. buf_len < 4 */
    st = syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, buf, 3);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* B. Version mismatch */
    buf[0] = 0x80; // ver = 2
    st = syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, buf, len);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    buf[0] = 0x40; // reset to ver = 1

    /* C. Token len > 8 */
    buf[0] = 0x49; // ver = 1, type = 0, token_len = 9
    st = syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, buf, len);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    buf[0] = 0x40; // reset

    /* D. buf_len < 4 + token_len */
    buf[0] = 0x42; // token_len = 2
    st = syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, buf, 5); // only 5 bytes, need 6
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    buf[0] = 0x40; // reset

    /* E. Delta extension truncation/error */
    /* Delta val = 13 but EOF */
    uint8_t bad_delta1[] = { 0x40, 0x01, 0x00, 0x00, 0xD0 };
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, bad_delta1, sizeof(bad_delta1)));

    /* Delta val = 14 but EOF */
    uint8_t bad_delta2[] = { 0x40, 0x01, 0x00, 0x00, 0xE0, 0x00 };
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, bad_delta2, sizeof(bad_delta2)));

    /* Delta val = 15 (invalid) */
    uint8_t bad_delta3[] = { 0x40, 0x01, 0x00, 0x00, 0xF0 };
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, bad_delta3, sizeof(bad_delta3)));

    /* F. Length extension truncation/error */
    /* Len val = 13 but EOF */
    uint8_t bad_len1[] = { 0x40, 0x01, 0x00, 0x00, 0x0D };
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, bad_len1, sizeof(bad_len1)));

    /* Len val = 14 but EOF */
    uint8_t bad_len2[] = { 0x40, 0x01, 0x00, 0x00, 0x0E, 0x00 };
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, bad_len2, sizeof(bad_len2)));

    /* Len val = 15 (invalid) */
    uint8_t bad_len3[] = { 0x40, 0x01, 0x00, 0x00, 0x0F };
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, bad_len3, sizeof(bad_len3)));

    /* G. Length exceeds buffer */
    uint8_t bad_len_exceed[] = { 0x40, 0x01, 0x00, 0x00, 0x05, 0x01, 0x02 }; // len = 5, only 2 bytes available
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_coap_parse(&resp, parsed_options, 20, &parsed_option_count, bad_len_exceed, sizeof(bad_len_exceed)));

    /* H. Option count exceeds max_options */
    uint8_t options_multi[] = { 0x40, 0x01, 0x00, 0x00, 0x11, 0x0A, 0x11, 0x0B }; // 2 options (num=1, len=1, num=2, len=1)
    st = syn_coap_parse(&resp, parsed_options, 1, &parsed_option_count, options_multi, sizeof(options_multi));
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(1, parsed_option_count); // parsed 2 options, but only 1 stored in array
}

static void test_coap_request_task_failures(void)
{
    SYN_CoapMsg req_msg = {
        .type = COAP_TYPE_CON,
        .code = COAP_CODE_GET,
        .msg_id = 0x5555,
        .token_len = 2,
        .token = {0x11, 0x22},
        .payload_len = 0
    };

    SYN_SockAddr server_addr = { .ip = {127, 0, 0, 1}, .port = 5683 };

    SYN_CoapRequest req;
    
    /* 1. UDP open fails */
    mock_port_reset();
    mock_udp_open_ok = false;
    syn_coap_request_init(&req, &server_addr, &req_msg, 100, 1);

    SYN_Sched sched;
    SYN_Task task;
    syn_task_create(&task, "coap", syn_coap_request_task, 0, &req);
    syn_sched_init(&sched, &task, 1);

    while (syn_sched_run(&sched)) {
        mock_tick_advance(10);
    }
    TEST_ASSERT_EQUAL(SYN_ERROR, req.status);

    /* 2. Serialization fails */
    mock_port_reset();
    /* Invalid token length to fail serialization */
    req_msg.token_len = 255;
    syn_coap_request_init(&req, &server_addr, &req_msg, 100, 2);
    
    /* Reset token_len */
    req_msg.token_len = 2;

    /* 3. UDP send fails */
    mock_port_reset();
    syn_coap_request_init(&req, &server_addr, &req_msg, 100, 1);
    mock_udp_tx_len = MOCK_UDP_BUF_SIZE; // make sendto fail

    syn_task_create(&task, "coap", syn_coap_request_task, 0, &req);
    syn_sched_init(&sched, &task, 1);

    while (syn_sched_run(&sched)) {
        mock_tick_advance(10);
    }
    TEST_ASSERT_EQUAL(SYN_ERROR, req.status);

    /* 4. Request timeout and retries */
    mock_port_reset();
    syn_coap_request_init(&req, &server_addr, &req_msg, 100, 2);

    syn_task_create(&task, "coap", syn_coap_request_task, 0, &req);
    syn_sched_init(&sched, &task, 1);

    bool alive = true;
    uint32_t start = syn_port_get_tick_ms();
    while (alive && (syn_port_get_tick_ms() - start < 2000)) {
        alive = syn_sched_run(&sched);
        mock_tick_advance(10);
    }
    TEST_ASSERT_FALSE(alive);
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, req.status);
    TEST_ASSERT_EQUAL_INT(3, req.backoff.attempts); // attempts went up to 3 (which is > retries)

    /* 5. Request mismatching token */
    mock_port_reset();
    syn_coap_request_init(&req, &server_addr, &req_msg, 100, 1);

    /* Mock response with WRONG token */
    SYN_CoapMsg resp_msg_bad = {
        .type = COAP_TYPE_ACK,
        .code = COAP_RESP_CONTENT,
        .msg_id = 0x5555,
        .token_len = 2,
        .token = {0x99, 0x99}, // wrong token!
        .payload_len = 0
    };
    uint8_t resp_raw[64];
    size_t resp_raw_len = syn_coap_serialize(&resp_msg_bad, NULL, 0, resp_raw, sizeof(resp_raw));
    TEST_ASSERT_TRUE(resp_raw_len > 0);
    mock_udp_set_response(resp_raw, resp_raw_len, &server_addr);

    syn_task_create(&task, "coap", syn_coap_request_task, 0, &req);
    syn_sched_init(&sched, &task, 1);

    /* Run loop, should consume the mismatching packet but keep waiting until timeout */
    alive = true;
    start = syn_port_get_tick_ms();
    while (alive && (syn_port_get_tick_ms() - start < 1000)) {
        alive = syn_sched_run(&sched);
        mock_tick_advance(10);
    }
    TEST_ASSERT_FALSE(alive);
    TEST_ASSERT_EQUAL(SYN_TIMEOUT, req.status);
}

void run_coap_tests(void)
{
    RUN_TEST(test_coap_serialization);
    RUN_TEST(test_coap_extended_options);
    RUN_TEST(test_coap_request_task_success);
    RUN_TEST(test_coap_serialization_boundaries);
    RUN_TEST(test_coap_request_task_failures);
}
