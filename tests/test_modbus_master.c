/**
 * @file test_modbus_master.c
 * @brief Unity tests for Modbus RTU Master protocol engine (loopback against Slave).
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/proto/syn_modbus.h"
#include "syntropic/proto/syn_modbus_master.h"
#include <string.h>

static uint16_t slave_holding[8] = { 100, 200, 300, 400, 500, 600, 700, 800 };
static uint16_t slave_input[4]   = { 1000, 2000, 3000, 4000 };
static uint8_t  slave_buf[256];

static void loopback_transaction(SYN_ModbusMaster *m, SYN_Modbus *slave)
{
    mock_port_reset();

    /* Feed Master TX buffer into Slave RX */
    memcpy(slave->buf, m->buf, m->tx_len);
    slave->rx_len = m->tx_len;

    /* Process Slave frame */
    TEST_ASSERT_TRUE(syn_modbus_process(slave));

    /* Feed Slave response TX buffer (from mock UART) back into Master RX */
    for (uint16_t i = 0; i < mock_uart_tx_len; i++) {
        syn_modbus_master_feed(m, mock_uart_tx_buf[i]);
    }

    /* Process Master state machine */
    syn_modbus_master_process(m, 100);
}

static void test_modbus_master_read_holding(void)
{
    mock_port_reset();
    uint16_t holding_init[8] = { 100, 200, 300, 400, 500, 600, 700, 800 };
    memcpy(slave_holding, holding_init, sizeof(slave_holding));

    SYN_Modbus slave;
    SYN_Modbus_Config slave_cfg = {
        .slave_addr    = 1,
        .holding_regs  = slave_holding,
        .holding_count = 8,
    };
    syn_modbus_init(&slave, &slave_cfg, slave_buf, sizeof(slave_buf));

    SYN_ModbusMaster master;
    syn_modbus_master_init(&master, 500);

    /* Issue FC 0x03 request: read 2 registers starting at address 1 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&master, 1, 1, 2));
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);

    loopback_transaction(&master, &slave);

    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, master.state);
    TEST_ASSERT_EQUAL_INT(2, master.read_count);
    TEST_ASSERT_EQUAL_INT(200, master.read_data[0]);
    TEST_ASSERT_EQUAL_INT(300, master.read_data[1]);
}

static void test_modbus_master_read_input(void)
{
    mock_port_reset();

    SYN_Modbus slave;
    SYN_Modbus_Config slave_cfg = {
        .slave_addr  = 1,
        .input_regs  = slave_input,
        .input_count = 4,
    };
    syn_modbus_init(&slave, &slave_cfg, slave_buf, sizeof(slave_buf));

    SYN_ModbusMaster master;
    syn_modbus_master_init(&master, 500);

    /* Issue FC 0x04 request: read 2 input registers starting at address 0 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_input(&master, 1, 0, 2));
    loopback_transaction(&master, &slave);

    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, master.state);
    TEST_ASSERT_EQUAL_INT(2, master.read_count);
    TEST_ASSERT_EQUAL_INT(1000, master.read_data[0]);
    TEST_ASSERT_EQUAL_INT(2000, master.read_data[1]);
}

static void test_modbus_master_write_single_and_multiple(void)
{
    mock_port_reset();

    SYN_Modbus slave;
    SYN_Modbus_Config slave_cfg = {
        .slave_addr    = 1,
        .holding_regs  = slave_holding,
        .holding_count = 8,
    };
    syn_modbus_init(&slave, &slave_cfg, slave_buf, sizeof(slave_buf));

    SYN_ModbusMaster master;
    syn_modbus_master_init(&master, 500);

    /* 1. Write Single (FC 0x06): write 999 to reg 0 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_write_single(&master, 1, 0, 999));
    loopback_transaction(&master, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, master.state);
    TEST_ASSERT_EQUAL_INT(999, slave_holding[0]);

    /* 2. Write Multiple (FC 0x10): write 111, 222 to regs 2..3 */
    uint16_t vals[2] = { 111, 222 };
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_write_multiple(&master, 1, 2, 2, vals));
    loopback_transaction(&master, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, master.state);
    TEST_ASSERT_EQUAL_INT(111, slave_holding[2]);
    TEST_ASSERT_EQUAL_INT(222, slave_holding[3]);
}

static void test_modbus_master_timeout_and_exceptions(void)
{
    mock_port_reset();

    SYN_ModbusMaster master;
    syn_modbus_master_init(&master, 100);

    /* 1. Timeout test: issue request and advance tick past timeout without feeding response */
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&master, 1, 0, 2));
    syn_modbus_master_process(&master, 10);  /* tick 10 */
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);
    syn_modbus_master_process(&master, 200); /* tick 200 > 10 + 100 timeout */
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_TIMEOUT, master.state);

    /* 2. Illegal address exception response test */
    SYN_Modbus slave;
    SYN_Modbus_Config slave_cfg = {
        .slave_addr    = 1,
        .holding_regs  = slave_holding,
        .holding_count = 8,
    };
    syn_modbus_init(&slave, &slave_cfg, slave_buf, sizeof(slave_buf));

    syn_modbus_master_init(&master, 500);
    /* Read out-of-bounds register (start at 7, count 5 -> 7+5 = 12 > 8) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&master, 1, 7, 5));
    loopback_transaction(&master, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_ERROR, master.state);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, master.exception_code);
}

static void test_modbus_master_process_edge_cases(void)
{
    mock_port_reset();
    SYN_ModbusMaster master;
    syn_modbus_master_init(&master, 500);

    /* 1. Process when state is IDLE -> returns IDLE */
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_IDLE, syn_modbus_master_process(&master, 0));

    /* 2. Process with CRC error */
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&master, 1, 0, 2));
    /* Feed bad frame (valid len >= 5, but bad CRC) */
    syn_modbus_master_feed(&master, 1);
    syn_modbus_master_feed(&master, 3);
    syn_modbus_master_feed(&master, 4);
    syn_modbus_master_feed(&master, 0xFF);
    syn_modbus_master_feed(&master, 0xFF);
    /* Should remain in WAITING_RESPONSE waiting for valid frame or timeout */
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&master, 10));

    /* 3. Process with Slave Address Mismatch */
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&master, 1, 0, 2));
    uint8_t resp_wrong_addr[7] = { 2, 3, 4, 0, 10, 0, 0 }; /* slave 2 instead of 1 */
    uint16_t crc = syn_crc16_modbus(resp_wrong_addr, 5);
    resp_wrong_addr[5] = (uint8_t)(crc & 0xFF);
    resp_wrong_addr[6] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 7; i++) syn_modbus_master_feed(&master, resp_wrong_addr[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_ERROR, syn_modbus_master_process(&master, 10));

    /* 4. Process with Function Code Mismatch */
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&master, 1, 0, 2));
    uint8_t resp_wrong_fc[7] = { 1, 4, 4, 0, 10, 0, 0 }; /* FC 4 instead of 3 */
    crc = syn_crc16_modbus(resp_wrong_fc, 5);
    resp_wrong_fc[5] = (uint8_t)(crc & 0xFF);
    resp_wrong_fc[6] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 7; i++) syn_modbus_master_feed(&master, resp_wrong_fc[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_ERROR, syn_modbus_master_process(&master, 10));
}

void run_modbus_master_tests(void)
{
    RUN_TEST(test_modbus_master_read_holding);
    RUN_TEST(test_modbus_master_read_input);
    RUN_TEST(test_modbus_master_write_single_and_multiple);
    RUN_TEST(test_modbus_master_timeout_and_exceptions);
    RUN_TEST(test_modbus_master_process_edge_cases);
}
