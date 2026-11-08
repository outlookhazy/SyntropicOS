/**
 * @file test_smbus.c
 * @brief Unit tests for SMBus protocol engine and PEC CRC-8 calculation.
 */

#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

void test_smbus_pec_calculation(void)
{
    uint8_t data1[] = {0x5AU};
    uint8_t pec1 = syn_smbus_calc_pec(0U, data1, sizeof(data1));
    TEST_ASSERT_NOT_EQUAL(0U, pec1);

    uint8_t data2_part1[] = {0x5AU, 0x01U};
    uint8_t data2_part2[] = {0x02U, 0x03U};
    uint8_t pec_full = syn_smbus_calc_pec(0U, data2_part1, 2U);
    pec_full = syn_smbus_calc_pec(pec_full, data2_part2, 2U);

    uint8_t data2_all[] = {0x5AU, 0x01U, 0x02U, 0x03U};
    uint8_t pec_direct = syn_smbus_calc_pec(0U, data2_all, 4U);
    TEST_ASSERT_EQUAL_HEX8(pec_direct, pec_full);

    TEST_ASSERT_EQUAL_HEX8(0x00U, syn_smbus_calc_pec(0x00U, NULL, 5U));
}

void test_smbus_encode_decode_write_word(void)
{
    SYN_SMBUS_Packet tx_pkt;
    memset(&tx_pkt, 0, sizeof(tx_pkt));
    tx_pkt.slave_addr = 0x4BU;
    tx_pkt.command = 0x21U;
    tx_pkt.proto = SYN_SMBUS_PROTO_WRITE_WORD;
    tx_pkt.pec_enabled = true;
    tx_pkt.length = 2U;
    tx_pkt.data[0] = 0xABU;
    tx_pkt.data[1] = 0xCDU;

    uint8_t buf[64];
    size_t out_len = 0U;
    SYN_Status status = syn_smbus_encode_packet(&tx_pkt, buf, sizeof(buf), &out_len);

    TEST_ASSERT_EQUAL(SYN_OK, status);
    TEST_ASSERT_EQUAL(5U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x96U, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x21U, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0xABU, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0xCDU, buf[3]);

    SYN_SMBUS_Packet rx_pkt;
    memset(&rx_pkt, 0, sizeof(rx_pkt));
    status = syn_smbus_decode_packet(&rx_pkt, buf, out_len, SYN_SMBUS_PROTO_WRITE_WORD, true);
    TEST_ASSERT_EQUAL(SYN_OK, status);
    TEST_ASSERT_TRUE(rx_pkt.pec_valid);
    TEST_ASSERT_EQUAL(2U, rx_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0xABU, rx_pkt.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xCDU, rx_pkt.data[1]);
}

void test_smbus_decode_invalid_pec(void)
{
    uint8_t bad_buf[] = {0x96U, 0x21U, 0xABU, 0xCDU, 0x00U};
    SYN_SMBUS_Packet rx_pkt;
    memset(&rx_pkt, 0, sizeof(rx_pkt));
    SYN_Status status = syn_smbus_decode_packet(&rx_pkt, bad_buf, sizeof(bad_buf),
                                                SYN_SMBUS_PROTO_WRITE_WORD, true);
    TEST_ASSERT_EQUAL(SYN_ERROR, status);
    TEST_ASSERT_FALSE(rx_pkt.pec_valid);
}

void test_smbus_block_process_call(void)
{
    SYN_SMBUS_Packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.slave_addr = 0x16U;
    pkt.command = 0x08U;
    pkt.proto = SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL;
    pkt.pec_enabled = false;
    pkt.length = 4U;
    pkt.data[0] = 1U;
    pkt.data[1] = 2U;
    pkt.data[2] = 3U;
    pkt.data[3] = 4U;

    uint8_t buf[64];
    size_t out_len = 0U;
    SYN_Status status = syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len);

    TEST_ASSERT_EQUAL(SYN_OK, status);
    TEST_ASSERT_EQUAL(7U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x2CU, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x08U, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(4U, buf[2]);

    pkt.proto = SYN_SMBUS_PROTO_PROCESS_CALL;
    pkt.length = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    pkt.length = 1U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
}

void test_smbus_quick_and_byte_protocols(void)
{
    SYN_SMBUS_Packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    uint8_t buf[64];
    size_t out_len = 0U;

    pkt.slave_addr = 0x27U;
    pkt.proto = SYN_SMBUS_PROTO_QUICK_READ;
    pkt.pec_enabled = false;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(1U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x4FU, buf[0]);

    pkt.proto = SYN_SMBUS_PROTO_QUICK_WRITE;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(1U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x4EU, buf[0]);

    pkt.proto = SYN_SMBUS_PROTO_SEND_BYTE;
    pkt.command = 0x55U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(2U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x55U, buf[1]);

    pkt.proto = SYN_SMBUS_PROTO_RECEIVE_BYTE;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(1U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x4FU, buf[0]);

    pkt.proto = SYN_SMBUS_PROTO_WRITE_BYTE;
    pkt.command = 0x10U;
    pkt.length = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(3U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x00U, buf[2]);

    pkt.length = 1U;
    pkt.data[0] = 0x88U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(3U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x88U, buf[2]);

    pkt.proto = SYN_SMBUS_PROTO_READ_BYTE;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(2U, out_len);

    pkt.proto = SYN_SMBUS_PROTO_WRITE_WORD;
    pkt.length = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(4U, out_len);

    pkt.length = 1U;
    pkt.data[0] = 0x11U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(4U, out_len);

    pkt.proto = SYN_SMBUS_PROTO_READ_WORD;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(2U, out_len);

    pkt.proto = SYN_SMBUS_PROTO_PROCESS_CALL;
    pkt.length = 2U;
    pkt.data[0] = 0x12U;
    pkt.data[1] = 0x34U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(4U, out_len);

    pkt.proto = SYN_SMBUS_PROTO_WRITE_BLOCK;
    pkt.length = 50U;
    for (int i = 0; i < 32; i++) {
        pkt.data[i] = (uint8_t)i;
    }
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL_UINT8(0x12U, syn_smbus_calc_pec(0x12U, NULL, 0U));
    TEST_ASSERT_EQUAL(35U, out_len);
    TEST_ASSERT_EQUAL_HEX8(32U, buf[2]);

    pkt.length = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(3U, out_len);

    pkt.proto = SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL;
    pkt.length = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));

    pkt.proto = SYN_SMBUS_PROTO_READ_BLOCK;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(2U, out_len);
}

void test_smbus_decoding_variations(void)
{
    SYN_SMBUS_Packet rx_pkt;
    memset(&rx_pkt, 0, sizeof(rx_pkt));

    uint8_t byte_resp[] = {0x42U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&rx_pkt, byte_resp, 1U,
                                                      SYN_SMBUS_PROTO_RECEIVE_BYTE, false));
    TEST_ASSERT_EQUAL(1U, rx_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0x42U, rx_pkt.data[0]);

    uint8_t byte_full_frame[] = {0x50U, 0x10U, 0x77U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&rx_pkt, byte_full_frame, 3U,
                                                      SYN_SMBUS_PROTO_READ_BYTE, false));
    TEST_ASSERT_EQUAL(1U, rx_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0x77U, rx_pkt.data[0]);

    uint8_t write_byte_raw[] = {0x4EU, 0x10U, 0x99U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&rx_pkt, write_byte_raw, 3U,
                                                      SYN_SMBUS_PROTO_WRITE_BYTE, false));
    TEST_ASSERT_EQUAL(1U, rx_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0x99U, rx_pkt.data[0]);

    uint8_t write_byte_resp[] = {0x88U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&rx_pkt, write_byte_resp, 1U,
                                                      SYN_SMBUS_PROTO_WRITE_BYTE, false));
    TEST_ASSERT_EQUAL(1U, rx_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0x88U, rx_pkt.data[0]);

    uint8_t write_word_resp[] = {0x11U, 0x22U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&rx_pkt, write_word_resp, 2U,
                                                      SYN_SMBUS_PROTO_WRITE_WORD, false));
    TEST_ASSERT_EQUAL(2U, rx_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0x11U, rx_pkt.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x22U, rx_pkt.data[1]);

    uint8_t read_word_resp[] = {0x33U, 0x44U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&rx_pkt, read_word_resp, 2U,
                                                      SYN_SMBUS_PROTO_READ_WORD, false));
    TEST_ASSERT_EQUAL(2U, rx_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0x33U, rx_pkt.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x44U, rx_pkt.data[1]);

    uint8_t block_raw[] = {0x03U, 0xDEU, 0xADU, 0xBEU};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&rx_pkt, block_raw, 4U, SYN_SMBUS_PROTO_READ_BLOCK, false));
    TEST_ASSERT_EQUAL(3U, rx_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0xDEU, rx_pkt.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xADU, rx_pkt.data[1]);
    TEST_ASSERT_EQUAL_HEX8(0xBEU, rx_pkt.data[2]);

    uint8_t block_full_frame[] = {0x50U, 0x20U, 0x02U, 0x55U, 0xAAU};
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&rx_pkt, block_full_frame, 5U,
                                                      SYN_SMBUS_PROTO_WRITE_BLOCK, false));
    TEST_ASSERT_EQUAL(2U, rx_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0x55U, rx_pkt.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xAAU, rx_pkt.data[1]);

    uint8_t custom_raw[300];
    memset(custom_raw, 0xAA, sizeof(custom_raw));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&rx_pkt, custom_raw, 2U, (SYN_SMBUS_Protocol)99, false));
    TEST_ASSERT_EQUAL(2U, rx_pkt.length);
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&rx_pkt, custom_raw, 260U, (SYN_SMBUS_Protocol)99, false));
    TEST_ASSERT_EQUAL(255U, rx_pkt.length);
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&rx_pkt, custom_raw, 0U, (SYN_SMBUS_Protocol)99, false));
    TEST_ASSERT_EQUAL(0U, rx_pkt.length);
}

void test_smbus_invalid_params_and_overflow(void)
{
    SYN_SMBUS_Packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    uint8_t buf[2];
    size_t out_len = 0U;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_smbus_encode_packet(NULL, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_encode_packet(&pkt, NULL, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(NULL, buf, 1U, SYN_SMBUS_PROTO_READ_BYTE, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&pkt, NULL, 1U, SYN_SMBUS_PROTO_READ_BYTE, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&pkt, buf, 0U, SYN_SMBUS_PROTO_READ_BYTE, true));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&pkt, buf, 0U, SYN_SMBUS_PROTO_WRITE_BYTE, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&pkt, buf, 1U, SYN_SMBUS_PROTO_WRITE_WORD, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&pkt, buf, 0U, SYN_SMBUS_PROTO_READ_BYTE, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&pkt, buf, 1U, SYN_SMBUS_PROTO_READ_WORD, false));

    uint8_t invalid_block[] = {0x05U, 0x01U};
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_smbus_decode_packet(&pkt, invalid_block, 2U, SYN_SMBUS_PROTO_READ_BLOCK, false));

    pkt.slave_addr = 0x10U;
    pkt.pec_enabled = true;

    pkt.proto = SYN_SMBUS_PROTO_QUICK_READ;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 0U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_SEND_BYTE;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 2U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_RECEIVE_BYTE;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 0U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_WRITE_BYTE;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 3U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_READ_BYTE;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 1U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_WRITE_WORD;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 2U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_READ_WORD;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 1U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_WRITE_BLOCK;
    pkt.length = 5U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 4U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_READ_BLOCK;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 1U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_PROCESS_CALL;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 3U, &out_len));

    pkt.proto = SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL;
    pkt.length = 5U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_smbus_encode_packet(&pkt, buf, 4U, &out_len));

    pkt.proto = (SYN_SMBUS_Protocol)99;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_smbus_encode_packet(&pkt, buf, sizeof(buf), &out_len));
}

void test_smbus_alert_and_host_notify(void)
{
    SYN_SMBUS_Packet alert_pkt;
    memset(&alert_pkt, 0, sizeof(alert_pkt));
    alert_pkt.slave_addr = SYN_SMBUS_ADDR_ALERT_RESPONSE;
    alert_pkt.proto = SYN_SMBUS_PROTO_RECEIVE_BYTE;
    alert_pkt.pec_enabled = true;

    uint8_t buf[64];
    size_t out_len = 0U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&alert_pkt, buf, sizeof(buf), &out_len));
    TEST_ASSERT_EQUAL(1U, out_len);
    TEST_ASSERT_EQUAL_HEX8(0x19U, buf[0]);

    /* Slave response with device address 0x5A + full transaction PEC over [(0x0C<<1)|1, rx] */
    uint8_t rx_alert[2] = {(uint8_t)(0x5AU << 1U), 0x00U};
    rx_alert[1] = syn_smbus_calc_transaction_pec(&alert_pkt, rx_alert, 1U);

    SYN_SMBUS_Packet decoded;
    memset(&decoded, 0, sizeof(decoded));
    decoded.slave_addr = SYN_SMBUS_ADDR_ALERT_RESPONSE;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&decoded, rx_alert, 2U,
                                                      SYN_SMBUS_PROTO_RECEIVE_BYTE, true));
    TEST_ASSERT_EQUAL_HEX8((uint8_t)(0x5AU << 1U), decoded.data[0]);
    TEST_ASSERT_TRUE(decoded.pec_valid);

    uint8_t word_rx4[4] = {0x11U, 0x22U, 0x33U, 0x44U};
    SYN_SMBUS_Packet word_pkt;
    memset(&word_pkt, 0, sizeof(word_pkt));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&word_pkt, word_rx4, 4U, SYN_SMBUS_PROTO_READ_WORD, false));
    TEST_ASSERT_EQUAL_INT(2, word_pkt.length);
    TEST_ASSERT_EQUAL_HEX8(0x33U, word_pkt.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x44U, word_pkt.data[1]);
}

void test_r5_a02_pec_read_and_process_call_roundtrips(void)
{
    uint8_t tx_buf[64];
    size_t tx_len = 0U;

    /* 1. READ_WORD with PEC: outbound request omits PEC before repeated-start */
    SYN_SMBUS_Packet req;
    memset(&req, 0, sizeof(req));
    req.slave_addr = 0x16U;
    req.command = 0x09U;
    req.proto = SYN_SMBUS_PROTO_READ_WORD;
    req.pec_enabled = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&req, tx_buf, sizeof(tx_buf), &tx_len));
    TEST_ASSERT_EQUAL(2U, tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x2CU, tx_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x09U, tx_buf[1]);

    uint8_t rw_wire[5] = {0x2CU, 0x09U, 0x2DU, 0x34U, 0x12U};
    uint8_t rw_pec = syn_smbus_calc_pec(0U, rw_wire, 5U);
    uint8_t rw_rx[3] = {0x34U, 0x12U, rw_pec};
    TEST_ASSERT_EQUAL_HEX8(rw_pec, syn_smbus_calc_transaction_pec(&req, rw_rx, 2U));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_smbus_decode_packet(&req, rw_rx, 3U, SYN_SMBUS_PROTO_READ_WORD, true));
    TEST_ASSERT_EQUAL(2U, req.length);
    TEST_ASSERT_EQUAL_HEX8(0x34U, req.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x12U, req.data[1]);

    /* Also full 6-byte wire capture for READ_WORD with PEC */
    uint8_t rw_full[6] = {0x2CU, 0x09U, 0x2DU, 0x34U, 0x12U, rw_pec};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_smbus_decode_packet(&req, rw_full, 6U, SYN_SMBUS_PROTO_READ_WORD, true));
    TEST_ASSERT_EQUAL_HEX8(0x34U, req.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x12U, req.data[1]);

    /* 2. READ_BLOCK with PEC + SYN_SMBUS_BLOCK_MAX_LEN (32) enforcement */
    req.command = 0x20U;
    req.proto = SYN_SMBUS_PROTO_READ_BLOCK;
    req.pec_enabled = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&req, tx_buf, sizeof(tx_buf), &tx_len));
    TEST_ASSERT_EQUAL(2U, tx_len);

    uint8_t rb_rx[5] = {0x03U, 0xAAU, 0xBBU, 0xCCU, 0x00U};
    rb_rx[4] = syn_smbus_calc_transaction_pec(&req, rb_rx, 4U);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_smbus_decode_packet(&req, rb_rx, 5U, SYN_SMBUS_PROTO_READ_BLOCK, true));
    TEST_ASSERT_EQUAL(3U, req.length);
    TEST_ASSERT_EQUAL_HEX8(0xAAU, req.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xCCU, req.data[2]);

    /* Full wire capture for READ_BLOCK with PEC */
    uint8_t rb_full[8] = {0x2CU, 0x20U, 0x2DU, 0x03U, 0xAAU, 0xBBU, 0xCCU, 0x00U};
    rb_full[7] = syn_smbus_calc_pec(0U, rb_full, 7U);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_smbus_decode_packet(&req, rb_full, 8U, SYN_SMBUS_PROTO_READ_BLOCK, true));

    /* Reject block count > SYN_SMBUS_BLOCK_MAX_LEN (32) */
    uint8_t overlong_blk[35];
    memset(overlong_blk, 0x11, sizeof(overlong_blk));
    overlong_blk[0] = 33U;
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_smbus_decode_packet(&req, overlong_blk, 34U, SYN_SMBUS_PROTO_READ_BLOCK, false));

    /* 3. READ_BYTE, PROCESS_CALL, BLOCK_PROCESS_CALL with PEC */
    req.proto = SYN_SMBUS_PROTO_READ_BYTE;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&req, tx_buf, sizeof(tx_buf), &tx_len));
    TEST_ASSERT_EQUAL(2U, tx_len);
    uint8_t rbyte_full[5] = {0x2CU, 0x20U, 0x2DU, 0x7EU, 0x00U};
    rbyte_full[4] = syn_smbus_calc_pec(0U, rbyte_full, 4U);
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&req, rbyte_full, 5U, SYN_SMBUS_PROTO_READ_BYTE, true));
    TEST_ASSERT_EQUAL_HEX8(0x7EU, req.data[0]);

    uint8_t rcv_full[3] = {0x2DU, 0x5AU, 0x00U};
    rcv_full[2] = syn_smbus_calc_pec(0U, rcv_full, 2U);
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&req, rcv_full, 3U, SYN_SMBUS_PROTO_RECEIVE_BYTE, true));
    TEST_ASSERT_EQUAL_HEX8(0x5AU, req.data[0]);

    req.proto = SYN_SMBUS_PROTO_PROCESS_CALL;
    req.pec_enabled = true;
    req.length = 2U;
    req.data[0] = 0x11U;
    req.data[1] = 0x22U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&req, tx_buf, sizeof(tx_buf), &tx_len));
    TEST_ASSERT_EQUAL(4U, tx_len);
    uint8_t pc_rx[3] = {0x55U, 0x66U, 0x00U};
    pc_rx[2] = syn_smbus_calc_transaction_pec(&req, pc_rx, 2U);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_smbus_decode_packet(&req, pc_rx, 3U, SYN_SMBUS_PROTO_PROCESS_CALL, true));
    TEST_ASSERT_EQUAL_HEX8(0x55U, req.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x66U, req.data[1]);

    /* Full wire PROCESS_CALL + length 0/1 calc_transaction_pec branches */
    uint8_t pc_full[8] = {0x2CU, 0x20U, 0x11U, 0x22U, 0x2DU, 0x77U, 0x88U, 0x00U};
    pc_full[7] = syn_smbus_calc_pec(0U, pc_full, 7U);
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&req, pc_full, 8U, SYN_SMBUS_PROTO_PROCESS_CALL, true));
    req.length = 0U;
    (void)syn_smbus_calc_transaction_pec(&req, pc_rx, 2U);
    req.length = 1U;
    (void)syn_smbus_calc_transaction_pec(&req, pc_rx, 2U);

    /* BLOCK_PROCESS_CALL with PEC (read-phase & full wire & >32 clamp & 0-len) */
    req.proto = SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL;
    req.length = 2U;
    req.data[0] = 0x01U;
    req.data[1] = 0x02U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&req, tx_buf, sizeof(tx_buf), &tx_len));
    TEST_ASSERT_EQUAL(5U, tx_len);
    uint8_t bpc_rx[4] = {0x02U, 0x0AU, 0x0BU, 0x00U};
    bpc_rx[3] = syn_smbus_calc_transaction_pec(&req, bpc_rx, 3U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&req, bpc_rx, 4U,
                                                      SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL, true));
    TEST_ASSERT_EQUAL(2U, req.length);
    TEST_ASSERT_EQUAL_HEX8(0x0AU, req.data[0]);

    uint8_t bpc_full[10] = {0x2CU, 0x20U, 0x02U, 0x01U, 0x02U, 0x2DU, 0x02U, 0x0AU, 0x0BU, 0x00U};
    bpc_full[9] = syn_smbus_calc_pec(0U, bpc_full, 9U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&req, bpc_full, 10U,
                                                      SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL, true));

    /* Malformed BLOCK_PROCESS_CALL rx_cnt > 32 */
    uint8_t bpc_bad[8] = {0x2CU, 0x20U, 0x01U, 0x01U, 0x2DU, 33U, 0x00U, 0x00U};
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_smbus_decode_packet(&req, bpc_bad, 8U, SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL, false));

    /* Remaining syn_smbus_calc_transaction_pec guards & protocol arms */
    req.length = 40U;
    (void)syn_smbus_calc_transaction_pec(&req, bpc_rx, 3U);
    req.length = 0U;
    (void)syn_smbus_calc_transaction_pec(&req, bpc_rx, 3U);
    req.proto = SYN_SMBUS_PROTO_WRITE_WORD;
    uint8_t ww_pay[3] = {0x11U, 0x22U, 0x00U};
    ww_pay[2] = syn_smbus_calc_transaction_pec(&req, ww_pay, 2U);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_smbus_decode_packet(&req, ww_pay, 3U, SYN_SMBUS_PROTO_WRITE_WORD, true));
    req.proto = SYN_SMBUS_PROTO_QUICK_READ;
    (void)syn_smbus_calc_transaction_pec(&req, bpc_rx, 1U);
    TEST_ASSERT_EQUAL_HEX8(0U, syn_smbus_calc_transaction_pec(NULL, bpc_rx, 1U));
    TEST_ASSERT_EQUAL_HEX8(0U, syn_smbus_calc_transaction_pec(&req, NULL, 1U));

    /* Encode BLOCK_PROCESS_CALL with length > 32 */
    req.proto = SYN_SMBUS_PROTO_BLOCK_PROCESS_CALL;
    req.length = 40U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&req, tx_buf, sizeof(tx_buf), &tx_len));
    TEST_ASSERT_EQUAL(35U, tx_len);

    /* Reject 3-byte READ_BYTE and 4-byte READ_WORD when has_pec == true (missing addr_r) */
    uint8_t bad_rb3[4] = {0x2CU, 0x20U, 0x7EU, 0x00U};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&req, bad_rb3, 4U, SYN_SMBUS_PROTO_READ_BYTE, true));
    uint8_t bad_rw4[5] = {0x2CU, 0x20U, 0x34U, 0x12U, 0x00U};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&req, bad_rw4, 5U, SYN_SMBUS_PROTO_READ_WORD, true));

    /* Smart Battery slave_addr = 0x0B (addr_w = 0x16 == 22) full-wire WRITE_BLOCK (cnt = 20,
     * data_len = 23 == 1 + addr_w) and READ_BLOCK (cnt = 19, data_len = 23 == 1 + addr_w) must
     * match full-wire framing first instead of raw [count, data...] */
    SYN_SMBUS_Packet sb;
    memset(&sb, 0, sizeof(sb));
    sb.slave_addr = 0x0BU;
    sb.command = 0x23U;
    sb.proto = SYN_SMBUS_PROTO_WRITE_BLOCK;
    sb.pec_enabled = true;
    sb.length = 20U;
    for (uint8_t i = 0U; i < 20U; i++) {
        sb.data[i] = (uint8_t)(0x40U + i);
    }
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_encode_packet(&sb, tx_buf, sizeof(tx_buf), &tx_len));
    TEST_ASSERT_EQUAL(24U, tx_len);
    SYN_SMBUS_Packet sb_rx;
    memset(&sb_rx, 0, sizeof(sb_rx));
    sb_rx.slave_addr = 0x0BU;
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&sb_rx, tx_buf, tx_len, SYN_SMBUS_PROTO_WRITE_BLOCK, true));
    TEST_ASSERT_EQUAL_UINT8(20U, sb_rx.length);
    TEST_ASSERT_EQUAL_HEX8(0x40U, sb_rx.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x53U, sb_rx.data[19]);
    TEST_ASSERT_TRUE(sb_rx.pec_valid);

    /* Reject malformed WRITE_BLOCK when length mismatches both full-wire and raw count */
    uint8_t bad_wb[5] = {0x16U, 0x23U, 0x05U, 0xAAU, 0xBBU};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&sb_rx, bad_wb, sizeof(bad_wb),
                                              SYN_SMBUS_PROTO_WRITE_BLOCK, false));

    uint8_t sb_rb_full[24];
    sb_rb_full[0] = 0x16U;
    sb_rb_full[1] = 0x23U;
    sb_rb_full[2] = 0x17U;
    sb_rb_full[3] = 19U;
    for (uint8_t i = 0U; i < 19U; i++) {
        sb_rb_full[4U + i] = (uint8_t)(0x80U + i);
    }
    sb_rb_full[23] = syn_smbus_calc_pec(0U, sb_rb_full, 23U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&sb_rx, sb_rb_full, sizeof(sb_rb_full),
                                                      SYN_SMBUS_PROTO_READ_BLOCK, true));
    TEST_ASSERT_EQUAL_UINT8(19U, sb_rx.length);
    TEST_ASSERT_EQUAL_HEX8(0x80U, sb_rx.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x92U, sb_rx.data[18]);

    /* Raw 22-byte READ_BLOCK response from Smart Battery 0x0B (count = 0x16 == addr_w) falls back
     * cleanly from full-wire check to raw [count, data...] */
    uint8_t sb_rb_raw[23];
    sb_rb_raw[0] = 22U;
    for (uint8_t i = 0U; i < 22U; i++) {
        sb_rb_raw[1U + i] = (uint8_t)(0xA0U + i);
    }
    TEST_ASSERT_EQUAL(SYN_OK, syn_smbus_decode_packet(&sb_rx, sb_rb_raw, sizeof(sb_rb_raw),
                                                      SYN_SMBUS_PROTO_READ_BLOCK, false));
    TEST_ASSERT_EQUAL_UINT8(22U, sb_rx.length);
    TEST_ASSERT_EQUAL_HEX8(0xA0U, sb_rx.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xB5U, sb_rx.data[21]);
}

static void test_tier_a_smbus_exact_u8_u16_and_process_call_len(void)
{
    SYN_SMBUS_Packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.slave_addr = 0x50U;
    pkt.command = 0x10U;

    /* 1. Truncated PROCESS_CALL (4B/5B/6B without PEC) must NOT return master write word! */
    uint8_t pc_trunc4[4] = {0xA0U, 0x10U, 0xADU, 0xDEU};
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_smbus_decode_packet(&pkt, pc_trunc4, 4U, SYN_SMBUS_PROTO_PROCESS_CALL, false));

    uint8_t pc_trunc6[6] = {0xA0U, 0x10U, 0xADU, 0xDEU, 0xA1U, 0xEFU};
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_smbus_decode_packet(&pkt, pc_trunc6, 6U, SYN_SMBUS_PROTO_PROCESS_CALL, false));

    /* Exact 7-byte PROCESS_CALL without PEC succeeds and extracts response word at [5..6] */
    uint8_t pc_valid7[7] = {0xA0U, 0x10U, 0xADU, 0xDEU, 0xA1U, 0xEFU, 0xBEU};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_smbus_decode_packet(&pkt, pc_valid7, 7U, SYN_SMBUS_PROTO_PROCESS_CALL, false));
    TEST_ASSERT_EQUAL_HEX8(0xEFU, pkt.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBEU, pkt.data[1]);

    /* Overlong u8 and u16 buffers must be rejected instead of silently ignoring trailing bytes */
    uint8_t over_u8[6] = {0xA0U, 0x10U, 0xA1U, 0x55U, 0xFFU, 0xFFU};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_smbus_decode_packet(&pkt, over_u8, 6U, SYN_SMBUS_PROTO_READ_BYTE, false));
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_smbus_decode_packet(&pkt, over_u8, 3U, SYN_SMBUS_PROTO_RECEIVE_BYTE, false));

    uint8_t over_u16[9] = {0xA0U, 0x10U, 0xADU, 0xDEU, 0xA1U, 0xEFU, 0xBEU, 0x00U, 0x00U};
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_smbus_decode_packet(&pkt, over_u16, 9U, SYN_SMBUS_PROTO_PROCESS_CALL, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_smbus_decode_packet(&pkt, over_u16, 6U,
                                                                 SYN_SMBUS_PROTO_READ_WORD, false));
}

void run_smbus_tests(void)
{
    RUN_TEST(test_smbus_pec_calculation);
    RUN_TEST(test_smbus_encode_decode_write_word);
    RUN_TEST(test_smbus_decode_invalid_pec);
    RUN_TEST(test_smbus_block_process_call);
    RUN_TEST(test_smbus_quick_and_byte_protocols);
    RUN_TEST(test_smbus_decoding_variations);
    RUN_TEST(test_smbus_invalid_params_and_overflow);
    RUN_TEST(test_smbus_alert_and_host_notify);
    RUN_TEST(test_r5_a02_pec_read_and_process_call_roundtrips);
    RUN_TEST(test_tier_a_smbus_exact_u8_u16_and_process_call_len);
}
