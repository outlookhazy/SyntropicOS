#include "mocks/mock_port.h"
#include "syntropic/proto/syn_modbus_tcp.h"
#include "unity/unity.h"

#include <string.h>

void test_mbap_encode_decode(void)
{
    SYN_MBAP_Header in_hdr = {
        .transaction_id = 0x1234, .protocol_id = 0x0000, .length = 6, .unit_id = 1};

    uint8_t buf[7];
    syn_mbap_encode_header(&in_hdr, buf);

    TEST_ASSERT_EQUAL_HEX8(0x12, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x34, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[4]);
    TEST_ASSERT_EQUAL_HEX8(0x06, buf[5]);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[6]);

    SYN_MBAP_Header out_hdr;
    TEST_ASSERT_TRUE(syn_mbap_decode_header(buf, &out_hdr));
    TEST_ASSERT_EQUAL_HEX16(0x1234, out_hdr.transaction_id);
    TEST_ASSERT_EQUAL_HEX16(0x0000, out_hdr.protocol_id);
    TEST_ASSERT_EQUAL_HEX16(6, out_hdr.length);
    TEST_ASSERT_EQUAL_HEX8(1, out_hdr.unit_id);
}

void test_modbus_tcp_slave(void)
{
    static uint16_t holding_regs[16] = {0x1111, 0x2222, 0x3333, 0x4444};
    static uint8_t mb_buf[256];

    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding_regs,
        .holding_count = 16,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* FC 0x03 Read Holding Registers request: MBAP (trans_id=1, len=6, unit=1) + PDU (FC=3, addr=0,
     * count=2) */
    uint8_t req_adu[12] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
    uint8_t resp_adu[256];
    uint16_t resp_len = 0;

    TEST_ASSERT_TRUE(syn_modbus_tcp_process_slave(&mb, req_adu, sizeof(req_adu), resp_adu,
                                                  sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(13, resp_len);

    /* MBAP Header of response */
    TEST_ASSERT_EQUAL_HEX8(0x00, resp_adu[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp_adu[1]); /* Trans ID 1 */
    TEST_ASSERT_EQUAL_HEX8(0x00, resp_adu[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, resp_adu[3]); /* Protocol ID 0 */
    TEST_ASSERT_EQUAL_HEX8(0x00, resp_adu[4]);
    TEST_ASSERT_EQUAL_HEX8(
        0x07, resp_adu[5]); /* Length = 7 (unit 1 + fc 1 + byte_cnt 1 + 4 bytes data) */
    TEST_ASSERT_EQUAL_HEX8(0x01, resp_adu[6]); /* Unit ID 1 */

    /* PDU of response */
    TEST_ASSERT_EQUAL_HEX8(0x03, resp_adu[7]); /* FC 3 */
    TEST_ASSERT_EQUAL_HEX8(0x04, resp_adu[8]); /* 4 bytes */
    TEST_ASSERT_EQUAL_HEX8(0x11, resp_adu[9]);
    TEST_ASSERT_EQUAL_HEX8(0x11, resp_adu[10]);
    TEST_ASSERT_EQUAL_HEX8(0x22, resp_adu[11]);
}

void test_modbus_tcp_client_adu(void)
{
    uint8_t pdu[5] = {0x03, 0x00, 0x00, 0x00, 0x02};
    uint8_t req_adu[16];
    uint16_t req_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_tcp_build_client_adu(0x00AB, 1, pdu, sizeof(pdu), req_adu,
                                                              sizeof(req_adu), &req_len));
    TEST_ASSERT_EQUAL(12, req_len);

    TEST_ASSERT_EQUAL_HEX8(0x00, req_adu[0]);
    TEST_ASSERT_EQUAL_HEX8(0xAB, req_adu[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, req_adu[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, req_adu[3]);
    TEST_ASSERT_EQUAL_HEX8(0x00, req_adu[4]);
    TEST_ASSERT_EQUAL_HEX8(0x06, req_adu[5]);
    TEST_ASSERT_EQUAL_HEX8(0x01, req_adu[6]);
    TEST_ASSERT_EQUAL_MEMORY(pdu, &req_adu[7], 5);
}

void test_modbus_tcp_edge_cases(void)
{
    static uint16_t holding_regs[16] = {0x1111};
    static uint8_t mb_buf[256];
    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {.slave_addr = 1, .holding_regs = holding_regs, .holding_count = 16};
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    uint8_t resp_adu[256];
    uint16_t resp_len = 0;

    /* 1. ADU too short */
    uint8_t short_adu[5] = {0};
    TEST_ASSERT_FALSE(
        syn_modbus_tcp_process_slave(&mb, short_adu, 5, resp_adu, sizeof(resp_adu), &resp_len));

    /* 2. Bad protocol ID (0x0001) */
    uint8_t bad_proto[12] = {0, 1, 0, 1, 0, 6, 1, 3, 0, 0, 0, 2};
    TEST_ASSERT_FALSE(
        syn_modbus_tcp_process_slave(&mb, bad_proto, 12, resp_adu, sizeof(resp_adu), &resp_len));

    /* 3. Length mismatch */
    uint8_t bad_len[12] = {0, 1, 0, 0, 0, 20, 1, 3, 0, 0, 0, 2};
    TEST_ASSERT_FALSE(
        syn_modbus_tcp_process_slave(&mb, bad_len, 12, resp_adu, sizeof(resp_adu), &resp_len));

    /* 4. Unit ID mismatch */
    uint8_t bad_unit[12] = {0, 1, 0, 0, 0, 6, 5, 3, 0, 0, 0, 2};
    TEST_ASSERT_FALSE(
        syn_modbus_tcp_process_slave(&mb, bad_unit, 12, resp_adu, sizeof(resp_adu), &resp_len));

    /* 5. FC 0x06 Write Single Register */
    uint8_t write_adu[12] = {0, 2, 0, 0, 0, 6, 1, 6, 0, 0, 0x12, 0x34};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, write_adu, 12, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(12, resp_len);
    TEST_ASSERT_EQUAL_HEX16(0x1234, holding_regs[0]);

    /* 6. Exception Response (Illegal Address) */
    uint8_t ex_adu[12] = {0, 3, 0, 0, 0, 6, 1, 3, 0, 0x99, 0, 2};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, ex_adu, 12, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(9, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x83, resp_adu[7]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp_adu[8]);

    /* 8. FC 0x07 Read Exception Status */
    uint8_t fc07_adu[12] = {0, 4, 0, 0, 0, 2, 1, 7};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc07_adu, 8, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(9, resp_len);

    /* 9. FC 0x16 Mask Write Register */
    uint8_t fc16_adu[14] = {0, 5, 0, 0, 0, 8, 1, 0x16, 0, 0, 0x00, 0xFF, 0xFF, 0x00};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc16_adu, 14, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(14, resp_len);

    /* 10. Buffer overflow / resp_max too small */
    uint8_t small_resp[8];
    TEST_ASSERT_FALSE(syn_modbus_tcp_process_slave(&mb, write_adu, 12, small_resp,
                                                   sizeof(small_resp), &resp_len));

    /* 11. PDU length larger than slave buffer size */
    uint8_t huge_adu[260] = {0, 6, 0, 0, 1, 2, 1, 3}; /* length = 258 -> pdu_len = 257 */
    TEST_ASSERT_FALSE(
        syn_modbus_tcp_process_slave(&mb, huge_adu, 260, resp_adu, sizeof(resp_adu), &resp_len));
}

void test_modbus_tcp_header_length_mismatch(void)
{
    static uint16_t holding_regs[16] = {0x1111};
    static uint8_t mb_buf[256];
    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {.slave_addr = 1, .holding_regs = holding_regs, .holding_count = 16};
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    uint8_t resp_adu[256];
    uint16_t resp_len = 0;

    /* 1. Broadcast unit ID (0) -> Processed, but no response generated */
    uint8_t bcast_adu[12] = {0, 1, 0, 0, 0, 6, 0, 6, 0, 0, 0x12, 0x34};
    TEST_ASSERT_FALSE(
        syn_modbus_tcp_process_slave(&mb, bcast_adu, 12, resp_adu, sizeof(resp_adu), &resp_len));

    /* 2. Custom/unsupported FC (0x17 / 23) falling through default case in get_response_pdu_len */
    uint8_t custom_fc_adu[12] = {0, 2, 0, 0, 0, 6, 1, 0x17, 0x02, 0xAA, 0xBB, 0xCC};
    TEST_ASSERT_TRUE(syn_modbus_tcp_process_slave(&mb, custom_fc_adu, 12, resp_adu,
                                                  sizeof(resp_adu), &resp_len));

    /* 3. syn_modbus_tcp_build_client_adu error bounds */
    uint8_t pdu[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t req_adu[16];
    uint16_t req_len = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_tcp_build_client_adu(
                                             1, 1, pdu, 0, req_adu, sizeof(req_adu), &req_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_modbus_tcp_build_client_adu(1, 1, pdu, 4, req_adu, 5, &req_len));

    /* FC 0x11 (Report Server ID) PDU length test */
    uint8_t fc11_adu[12] = {0, 4, 0, 0, 0, 2, 1, 0x11};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc11_adu, 8, resp_adu, sizeof(resp_adu), &resp_len));

    /* Unit ID mismatch (unit_id = 99 when slave = 1) */
    uint8_t mismatch_unit_adu[12] = {0, 5, 0, 0, 0, 6, 99, 6, 0, 0, 0, 1};
    TEST_ASSERT_FALSE(syn_modbus_tcp_process_slave(&mb, mismatch_unit_adu, 12, resp_adu,
                                                   sizeof(resp_adu), &resp_len));

    /* FC 0x16 (Mask Write Register) test */
    uint8_t fc16_adu[14] = {0, 6, 0, 0, 0, 8, 1, 0x16, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x12};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc16_adu, 14, resp_adu, sizeof(resp_adu), &resp_len));

    /* FC 0x07 (Read Exception Status) test */
    uint8_t fc07_adu[10] = {0, 7, 0, 0, 0, 2, 1, 0x07};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc07_adu, 8, resp_adu, sizeof(resp_adu), &resp_len));

    /* FC 0x18 (Read FIFO Queue) test */
    static uint16_t fifo_data[4] = {0x1111, 0x2222, 0x3333, 0x4444};
    cfg.fifo_queue = fifo_data;
    cfg.fifo_count = 4;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    uint8_t fc18_adu[12] = {0, 8, 0, 0, 0, 4, 1, 0x18, 0x00, 0x01};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc18_adu, 10, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(20,
                      resp_len); /* 7 MBAP + 1 FC + 2 byte_count + 2 fifo_count + 8 bytes data */

    /* FC 0x2B (Read Device Info) test */
    uint8_t fc2b_adu[12] = {0, 9, 0, 0, 0, 5, 1, 0x2B, 0x0E, 0x01, 0x00};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc2b_adu, 11, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_TRUE(resp_len > 14);
}

void test_r5_a08_tcp_fc08_fc0b_fc0c_fc17_no_uart_tx(void)
{
    static uint16_t holding_regs[8] = {0x1111U, 0x2222U, 0x3333U, 0x4444U};
    static uint8_t mb_buf[256];
    uint8_t resp_adu[256];
    uint16_t resp_len = 0U;

    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1U,
        .holding_regs = holding_regs,
        .holding_count = 8U,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. FC 0x08 Diagnostics (Return Query Data 0x0000, data 0xA55A) -> 7 MBAP + 5 PDU = 12B */
    uint8_t fc08_adu[12] = {0U, 1U, 0U, 0U, 0U, 6U, 0xFFU, 0x08U, 0x00U, 0x00U, 0xA5U, 0x5AU};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc08_adu, 12U, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL_UINT16(12U, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0xFFU, resp_adu[6]);
    TEST_ASSERT_EQUAL_HEX8(0x08U, resp_adu[7]);
    TEST_ASSERT_EQUAL_HEX8(0xA5U, resp_adu[10]);
    TEST_ASSERT_EQUAL_HEX8(0x5AU, resp_adu[11]);
    TEST_ASSERT_EQUAL_size_t(0U, mock_uart_tx_len);

    /* Short FC 0x08 PDU (< 5B) returns Modbus exception 0x88 / 0x03 without UART TX */
    uint8_t fc08_short[10] = {0U, 2U, 0U, 0U, 0U, 4U, 1U, 0x08U, 0x00U, 0x00U};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc08_short, 10U, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL_UINT16(9U, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x88U, resp_adu[7]);
    TEST_ASSERT_EQUAL_size_t(0U, mock_uart_tx_len);

    /* 2. FC 0x0B Get Comm Event Counter -> 7 MBAP + 5 PDU = 12B */
    uint8_t fc0b_adu[8] = {0U, 3U, 0U, 0U, 0U, 2U, 1U, 0x0BU};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc0b_adu, 8U, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL_UINT16(12U, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x0BU, resp_adu[7]);
    TEST_ASSERT_EQUAL_HEX8(0x01U, resp_adu[11]); /* 1 successful event from FC 0x08 */
    TEST_ASSERT_EQUAL_size_t(0U, mock_uart_tx_len);

    /* 3. FC 0x0C Get Comm Event Log -> 7 MBAP + 10 PDU = 17B */
    uint8_t fc0c_adu[8] = {0U, 4U, 0U, 0U, 0U, 2U, 1U, 0x0CU};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc0c_adu, 8U, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL_UINT16(17U, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x0CU, resp_adu[7]);
    TEST_ASSERT_EQUAL_HEX8(0x08U, resp_adu[8]);
    TEST_ASSERT_EQUAL_size_t(0U, mock_uart_tx_len);

    /* 4. FC 0x17 Read/Write Multiple Registers -> 7 MBAP + (2 + 4) PDU = 13B */
    uint8_t fc17_adu[19] = {0U,    5U,    0U,    0U,    0U,    13U,   1U,    0x17U, 0x00U, 0x00U,
                            0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x01U, 0x02U, 0xBEU, 0xEFU};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc17_adu, 19U, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL_UINT16(13U, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x17U, resp_adu[7]);
    TEST_ASSERT_EQUAL_HEX8(0x04U, resp_adu[8]);
    TEST_ASSERT_EQUAL_HEX8(0xBEU, resp_adu[9]);
    TEST_ASSERT_EQUAL_HEX8(0xEFU, resp_adu[10]);
    TEST_ASSERT_EQUAL_size_t(0U, mock_uart_tx_len);

    /* 5. Unsupported FC (0x20) over TCP returns 0xA0 / 0x01 exception ADU without UART TX */
    uint8_t fc20_adu[8] = {0U, 6U, 0U, 0U, 0U, 2U, 1U, 0x20U};
    TEST_ASSERT_TRUE(
        syn_modbus_tcp_process_slave(&mb, fc20_adu, 8U, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL_UINT16(9U, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0xA0U, resp_adu[7]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_FUNC, resp_adu[8]);
    TEST_ASSERT_EQUAL_size_t(0U, mock_uart_tx_len);
}

static void test_pro09_tcp_tiny_rtu_buf_size_underflow_guard(void)
{
    uint16_t holding[4] = {0};
    uint8_t rtu_buf[8];
    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1U,
        .holding_regs = holding,
        .holding_count = 4U,
    };
    syn_modbus_init(&mb, &cfg, rtu_buf, 6U);

    uint8_t req_adu[12] = {0U, 1U, 0U, 0U, 0U, 6U, 1U, 0x03U, 0U, 0U, 0U, 1U};
    uint8_t resp_adu[32];
    uint16_t resp_len = 99U;

    /* pdu_len (5) > mb.buf_size (6) - 3 (3) -> false */
    TEST_ASSERT_FALSE(
        syn_modbus_tcp_process_slave(&mb, req_adu, 12U, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL_UINT16(0U, resp_len);

    /* mb.buf_size < 3U -> guarded against unsigned underflow */
    mb.buf_size = 2U;
    TEST_ASSERT_FALSE(
        syn_modbus_tcp_process_slave(&mb, req_adu, 12U, resp_adu, sizeof(resp_adu), &resp_len));
}

void run_modbus_tcp_tests(void)
{
    RUN_TEST(test_mbap_encode_decode);
    RUN_TEST(test_modbus_tcp_slave);
    RUN_TEST(test_modbus_tcp_client_adu);
    RUN_TEST(test_modbus_tcp_edge_cases);
    RUN_TEST(test_modbus_tcp_header_length_mismatch);
    RUN_TEST(test_r5_a08_tcp_fc08_fc0b_fc0c_fc17_no_uart_tx);
    RUN_TEST(test_pro09_tcp_tiny_rtu_buf_size_underflow_guard);
}
