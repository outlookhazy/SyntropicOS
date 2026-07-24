#include "unity/unity.h"
#include "syntropic/proto/syn_modbus_tcp.h"
#include "mocks/mock_port.h"

#include <string.h>

void test_mbap_encode_decode(void)
{
    SYN_MBAP_Header in_hdr = {
        .transaction_id = 0x1234,
        .protocol_id    = 0x0000,
        .length         = 6,
        .unit_id        = 1
    };

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
    static uint16_t holding_regs[16] = { 0x1111, 0x2222, 0x3333, 0x4444 };
    static uint8_t mb_buf[256];

    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr   = 1,
        .holding_regs = holding_regs,
        .holding_count = 16,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* FC 0x03 Read Holding Registers request: MBAP (trans_id=1, len=6, unit=1) + PDU (FC=3, addr=0, count=2) */
    uint8_t req_adu[12] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x03, 0x00, 0x00, 0x00, 0x02 };
    uint8_t resp_adu[256];
    uint16_t resp_len = 0;

    TEST_ASSERT_TRUE(syn_modbus_tcp_process_slave(&mb, req_adu, sizeof(req_adu), resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(13, resp_len);

    /* MBAP Header of response */
    TEST_ASSERT_EQUAL_HEX8(0x00, resp_adu[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, resp_adu[1]); /* Trans ID 1 */
    TEST_ASSERT_EQUAL_HEX8(0x00, resp_adu[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, resp_adu[3]); /* Protocol ID 0 */
    TEST_ASSERT_EQUAL_HEX8(0x00, resp_adu[4]);
    TEST_ASSERT_EQUAL_HEX8(0x07, resp_adu[5]); /* Length = 7 (unit 1 + fc 1 + byte_cnt 1 + 4 bytes data) */
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
    uint8_t pdu[5] = { 0x03, 0x00, 0x00, 0x00, 0x02 };
    uint8_t req_adu[16];
    uint16_t req_len = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_tcp_build_client_adu(0x00AB, 1, pdu, sizeof(pdu), req_adu, sizeof(req_adu), &req_len));
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
    static uint16_t holding_regs[16] = { 0x1111 };
    static uint8_t mb_buf[256];
    SYN_Modbus mb;
    SYN_Modbus_Config cfg = { .slave_addr = 1, .holding_regs = holding_regs, .holding_count = 16 };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    uint8_t resp_adu[256];
    uint16_t resp_len = 0;

    /* 1. ADU too short */
    uint8_t short_adu[5] = { 0 };
    TEST_ASSERT_FALSE(syn_modbus_tcp_process_slave(&mb, short_adu, 5, resp_adu, sizeof(resp_adu), &resp_len));

    /* 2. Bad protocol ID (0x0001) */
    uint8_t bad_proto[12] = { 0, 1, 0, 1, 0, 6, 1, 3, 0, 0, 0, 2 };
    TEST_ASSERT_FALSE(syn_modbus_tcp_process_slave(&mb, bad_proto, 12, resp_adu, sizeof(resp_adu), &resp_len));

    /* 3. Length mismatch */
    uint8_t bad_len[12] = { 0, 1, 0, 0, 0, 20, 1, 3, 0, 0, 0, 2 };
    TEST_ASSERT_FALSE(syn_modbus_tcp_process_slave(&mb, bad_len, 12, resp_adu, sizeof(resp_adu), &resp_len));

    /* 4. Unit ID mismatch */
    uint8_t bad_unit[12] = { 0, 1, 0, 0, 0, 6, 5, 3, 0, 0, 0, 2 };
    TEST_ASSERT_FALSE(syn_modbus_tcp_process_slave(&mb, bad_unit, 12, resp_adu, sizeof(resp_adu), &resp_len));

    /* 5. FC 0x06 Write Single Register */
    uint8_t write_adu[12] = { 0, 2, 0, 0, 0, 6, 1, 6, 0, 0, 0x12, 0x34 };
    TEST_ASSERT_TRUE(syn_modbus_tcp_process_slave(&mb, write_adu, 12, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(12, resp_len);
    TEST_ASSERT_EQUAL_HEX16(0x1234, holding_regs[0]);

    /* 6. Exception Response (Illegal Address) */
    uint8_t ex_adu[12] = { 0, 3, 0, 0, 0, 6, 1, 3, 0, 0x99, 0, 2 };
    TEST_ASSERT_TRUE(syn_modbus_tcp_process_slave(&mb, ex_adu, 12, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(9, resp_len);
    TEST_ASSERT_EQUAL_HEX8(0x83, resp_adu[7]);
    TEST_ASSERT_EQUAL_HEX8(0x02, resp_adu[8]);

    /* 8. FC 0x07 Read Exception Status */
    uint8_t fc07_adu[12] = { 0, 4, 0, 0, 0, 2, 1, 7 };
    TEST_ASSERT_TRUE(syn_modbus_tcp_process_slave(&mb, fc07_adu, 8, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(9, resp_len);

    /* 9. FC 0x16 Mask Write Register */
    uint8_t fc16_adu[14] = { 0, 5, 0, 0, 0, 8, 1, 0x16, 0, 0, 0x00, 0xFF, 0xFF, 0x00 };
    TEST_ASSERT_TRUE(syn_modbus_tcp_process_slave(&mb, fc16_adu, 14, resp_adu, sizeof(resp_adu), &resp_len));
    TEST_ASSERT_EQUAL(14, resp_len);

    /* 10. Buffer overflow / resp_max too small */
    uint8_t small_resp[8];
    TEST_ASSERT_FALSE(syn_modbus_tcp_process_slave(&mb, write_adu, 12, small_resp, sizeof(small_resp), &resp_len));

    /* 11. PDU length larger than slave buffer size */
    uint8_t huge_adu[260] = { 0, 6, 0, 0, 1, 2, 1, 3 }; /* length = 258 -> pdu_len = 257 */
    TEST_ASSERT_FALSE(syn_modbus_tcp_process_slave(&mb, huge_adu, 260, resp_adu, sizeof(resp_adu), &resp_len));
}

void run_modbus_tcp_tests(void)
{
    RUN_TEST(test_mbap_encode_decode);
    RUN_TEST(test_modbus_tcp_slave);
    RUN_TEST(test_modbus_tcp_client_adu);
    RUN_TEST(test_modbus_tcp_edge_cases);
}
