/**
 * @file test_modbus_master.c
 * @brief Unity tests for Modbus RTU Master protocol engine (loopback against Slave).
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_modbus.h"
#include "syntropic/proto/syn_modbus_master.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

static uint16_t slave_holding[8] = {100, 200, 300, 400, 500, 600, 700, 800};
static uint16_t slave_input[4] = {1000, 2000, 3000, 4000};
static uint8_t slave_buf[256];

static void loopback_transaction(SYN_ModbusMaster *m, SYN_Modbus *slave)
{
    uint32_t req_tick = m->request_tick_ms;
    mock_port_reset();

    /* Feed Master TX buffer into Slave RX */
    memcpy(slave->buf, m->buf, m->tx_len);
    slave->rx_len = m->tx_len;

    /* Process Slave frame */
    TEST_ASSERT_TRUE(syn_modbus_process(slave));

    /* Feed Slave response TX buffer (from mock UART) back into Master RX */
    for (size_t i = 0; i < mock_uart_tx_len; i++) {
        syn_modbus_master_feed(m, mock_uart_tx_buf[i]);
    }

    /* Process Master state machine */
    syn_modbus_master_process(m, req_tick + 100U);
}

static void test_modbus_master_read_holding(void)
{
    mock_port_reset();
    uint16_t holding_init[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    memcpy(slave_holding, holding_init, sizeof(slave_holding));

    SYN_Modbus slave;
    SYN_Modbus_Config slave_cfg = {
        .slave_addr = 1,
        .holding_regs = slave_holding,
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
        .slave_addr = 1,
        .input_regs = slave_input,
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
        .slave_addr = 1,
        .holding_regs = slave_holding,
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
    uint16_t vals[2] = {111, 222};
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
    syn_modbus_master_process(&master, 10); /* tick 10 */
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);
    syn_modbus_master_process(&master, 200); /* tick 200 > 10 + 100 timeout */
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_TIMEOUT, master.state);

    /* 2. Illegal address exception response test */
    SYN_Modbus slave;
    SYN_Modbus_Config slave_cfg = {
        .slave_addr = 1,
        .holding_regs = slave_holding,
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
    uint8_t resp_wrong_addr[7] = {2, 3, 4, 0, 10, 0, 0}; /* slave 2 instead of 1 */
    uint16_t crc = syn_crc16_modbus(resp_wrong_addr, 5);
    resp_wrong_addr[5] = (uint8_t)(crc & 0xFF);
    resp_wrong_addr[6] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 7; i++)
        syn_modbus_master_feed(&master, resp_wrong_addr[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_ERROR, syn_modbus_master_process(&master, 10));

    /* 4. Process with Function Code Mismatch */
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&master, 1, 0, 2));
    uint8_t resp_wrong_fc[7] = {1, 4, 4, 0, 10, 0, 0}; /* FC 4 instead of 3 */
    crc = syn_crc16_modbus(resp_wrong_fc, 5);
    resp_wrong_fc[5] = (uint8_t)(crc & 0xFF);
    resp_wrong_fc[6] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 7; i++)
        syn_modbus_master_feed(&master, resp_wrong_fc[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_ERROR, syn_modbus_master_process(&master, 10));
}

static void test_modbus_master_new_queries(void)
{
    SYN_ModbusMaster master;
    syn_modbus_master_init(&master, 500);

    /* 1. Read Coils (FC 0x01) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_coils(&master, 1, 0, 8));
    uint8_t resp_coils[6] = {1, 1, 1, 0x55, 0, 0};
    uint16_t crc = syn_crc16_modbus(resp_coils, 4);
    resp_coils[4] = (uint8_t)(crc & 0xFF);
    resp_coils[5] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 6; i++)
        syn_modbus_master_feed(&master, resp_coils[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, syn_modbus_master_process(&master, 10));
    TEST_ASSERT_EQUAL(1, master.read_count);
    TEST_ASSERT_EQUAL_HEX16(0x55, master.read_data[0]);

    /* 2. Read Discrete Inputs (FC 0x02) */
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_discrete_inputs(&master, 1, 0, 8));
    uint8_t resp_disc[6] = {1, 2, 1, 0xAA, 0, 0};
    crc = syn_crc16_modbus(resp_disc, 4);
    resp_disc[4] = (uint8_t)(crc & 0xFF);
    resp_disc[5] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 6; i++)
        syn_modbus_master_feed(&master, resp_disc[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, syn_modbus_master_process(&master, 10));

    /* 3. Write Single Coil (FC 0x05) */
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_write_single_coil(&master, 1, 0, true));
    uint8_t resp_sc[8] = {1, 5, 0, 0, 0xFF, 0, 0, 0};
    crc = syn_crc16_modbus(resp_sc, 6);
    resp_sc[6] = (uint8_t)(crc & 0xFF);
    resp_sc[7] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 8; i++)
        syn_modbus_master_feed(&master, resp_sc[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, syn_modbus_master_process(&master, 10));

    /* 4. Write Multiple Coils (FC 0x0F) */
    syn_modbus_master_init(&master, 500);
    uint8_t coil_bits[1] = {0xFF};
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_write_multiple_coils(&master, 1, 0, 8, coil_bits));
    uint8_t resp_mc[8] = {1, 0x0F, 0, 0, 0, 8, 0, 0};
    crc = syn_crc16_modbus(resp_mc, 6);
    resp_mc[6] = (uint8_t)(crc & 0xFF);
    resp_mc[7] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 8; i++)
        syn_modbus_master_feed(&master, resp_mc[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, syn_modbus_master_process(&master, 10));

    /* 5. Mask Write Register (FC 0x16) */
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_mask_write_register(&master, 1, 0, 0xFF00, 0x00AB));
    uint8_t resp_mw[10] = {1, 0x16, 0, 0, 0xFF, 0, 0, 0xAB, 0, 0};
    crc = syn_crc16_modbus(resp_mw, 8);
    resp_mw[8] = (uint8_t)(crc & 0xFF);
    resp_mw[9] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 10; i++)
        syn_modbus_master_feed(&master, resp_mw[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, syn_modbus_master_process(&master, 10));

    /* 6. Read FIFO Queue (FC 0x18) */
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_fifo_queue(&master, 1, 0));
    uint8_t resp_fifo[10] = {1, 0x18, 0, 6, 0, 2, 0, 100, 0, 200};
    crc = syn_crc16_modbus(resp_fifo, 10);
    /* Append CRC */
    uint8_t resp_fifo_full[12];
    memcpy(resp_fifo_full, resp_fifo, 10);
    resp_fifo_full[10] = (uint8_t)(crc & 0xFF);
    resp_fifo_full[11] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 12; i++)
        syn_modbus_master_feed(&master, resp_fifo_full[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, syn_modbus_master_process(&master, 10));
    TEST_ASSERT_EQUAL(2, master.read_count);
    TEST_ASSERT_EQUAL_INT(100, master.read_data[0]);
    TEST_ASSERT_EQUAL_INT(200, master.read_data[1]);

    /* Invalid parameters checks */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_read_coils(NULL, 1, 0, 8));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_read_discrete_inputs(NULL, 1, 0, 8));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_write_single_coil(NULL, 1, 0, true));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_modbus_master_write_multiple_coils(NULL, 1, 0, 8, coil_bits));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_mask_write_register(NULL, 1, 0, 0, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_read_fifo_queue(NULL, 1, 0));

    /* SYN_BUSY state checks when transaction active */
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&master, 1, 0, 2));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_modbus_master_read_holding(&master, 1, 0, 2));
    TEST_ASSERT_EQUAL(SYN_BUSY,
                      syn_modbus_master_write_multiple_coils(&master, 1, 0, 8, coil_bits));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_modbus_master_mask_write_register(&master, 1, 0, 0, 0));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_modbus_master_read_fifo_queue(&master, 1, 0));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_modbus_master_report_server_id(&master, 1));

    /* Report Server ID (FC 0x11) */
    syn_modbus_master_init(&master, 500);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_report_server_id(&master, 1));
    uint8_t resp_srv[10] = {1, 0x11, 5, 'T', 'E', 'S', 'T', 0xFF, 0, 0};
    crc = syn_crc16_modbus(resp_srv, 8);
    resp_srv[8] = (uint8_t)(crc & 0xFF);
    resp_srv[9] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 10; i++)
        syn_modbus_master_feed(&master, resp_srv[i]);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, syn_modbus_master_process(&master, 10));
}

static bool s_cb_called = false;
static uint16_t s_cb_val = 0;

static void test_cb(uint8_t slave_addr, uint8_t func_code, const uint16_t *data, uint16_t count,
                    SYN_Status status, void *user_ctx)
{
    (void)slave_addr;
    (void)func_code;
    (void)user_ctx;
    if (status == SYN_OK && count > 0) {
        s_cb_called = true;
        s_cb_val = data[0];
    }
}

static void test_modbus_master_queue(void)
{
    SYN_ModbusMaster master;
    syn_modbus_master_init(&master, 500);

    SYN_ModbusMasterQueue q;
    syn_modbus_master_queue_init(&q, 2);

    SYN_ModbusMasterQuery qry = {
        .slave_addr = 1,
        .func_code = SYN_MB_FC_READ_HOLDING,
        .start_addr = 10,
        .count = 1,
        .callback = test_cb,
        .user_ctx = NULL,
    };

    s_cb_called = false;
    s_cb_val = 0;

    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_queue_push(&q, &qry));
    TEST_ASSERT_EQUAL(1, q.count);

    /* Step queue to dispatch query */
    syn_modbus_master_queue_step(&master, &q, 0);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);

    /* Feed valid Modbus response */
    uint8_t resp[7] = {1, 0x03, 2, 0x12, 0x34, 0, 0};
    uint16_t crc = syn_crc16_modbus(resp, 5);
    resp[5] = (uint8_t)(crc & 0xFF);
    resp[6] = (uint8_t)(crc >> 8);

    for (int i = 0; i < 7; i++) {
        syn_modbus_master_feed(&master, resp[i]);
    }

    /* Step queue to process response and fire callback */
    syn_modbus_master_queue_step(&master, &q, 10);

    TEST_ASSERT_TRUE(s_cb_called);
    TEST_ASSERT_EQUAL_HEX16(0x1234, s_cb_val);
    TEST_ASSERT_EQUAL(0, q.count);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_IDLE, master.state);
}

static void test_modbus_master_queue_fc_variants(void)
{
    SYN_ModbusMaster master;
    syn_modbus_master_init(&master, 500);

    SYN_ModbusMasterQueue q;
    syn_modbus_master_queue_init(&q, 2);

    /* READ_INPUT */
    SYN_ModbusMasterQuery q1 = {
        .slave_addr = 1, .func_code = SYN_MB_FC_READ_INPUT, .start_addr = 0, .count = 1};
    syn_modbus_master_queue_push(&q, &q1);
    syn_modbus_master_queue_step(&master, &q, 0);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);

    uint8_t r1[7] = {1, 4, 2, 0, 99, 0, 0};
    uint16_t crc = syn_crc16_modbus(r1, 5);
    r1[5] = (uint8_t)(crc & 0xFF);
    r1[6] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 7; i++)
        syn_modbus_master_feed(&master, r1[i]);
    syn_modbus_master_queue_step(&master, &q, 10);
    TEST_ASSERT_EQUAL(0, q.count);

    /* WRITE_SINGLE */
    syn_modbus_master_init(&master, 500);
    SYN_ModbusMasterQuery q2 = {
        .slave_addr = 1, .func_code = SYN_MB_FC_WRITE_SINGLE, .start_addr = 0, .write_value = 123};
    syn_modbus_master_queue_push(&q, &q2);
    syn_modbus_master_queue_step(&master, &q, 0);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);

    uint8_t r2[8] = {1, 6, 0, 0, 0, 123, 0, 0};
    crc = syn_crc16_modbus(r2, 6);
    r2[6] = (uint8_t)(crc & 0xFF);
    r2[7] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 8; i++)
        syn_modbus_master_feed(&master, r2[i]);
    syn_modbus_master_queue_step(&master, &q, 10);
    TEST_ASSERT_EQUAL(0, q.count);

    /* READ_COILS */
    syn_modbus_master_init(&master, 500);
    SYN_ModbusMasterQuery q3 = {
        .slave_addr = 1, .func_code = SYN_MB_FC_READ_COILS, .start_addr = 0, .count = 8};
    syn_modbus_master_queue_push(&q, &q3);
    syn_modbus_master_queue_step(&master, &q, 0);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);

    uint8_t r3[6] = {1, 1, 1, 0x01, 0, 0};
    crc = syn_crc16_modbus(r3, 4);
    r3[4] = (uint8_t)(crc & 0xFF);
    r3[5] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 6; i++)
        syn_modbus_master_feed(&master, r3[i]);
    syn_modbus_master_queue_step(&master, &q, 10);
    TEST_ASSERT_EQUAL(0, q.count);

    /* READ_DISCRETE_INPUTS */
    syn_modbus_master_init(&master, 500);
    SYN_ModbusMasterQuery q4 = {
        .slave_addr = 1, .func_code = SYN_MB_FC_READ_DISCRETE_INPUTS, .start_addr = 0, .count = 8};
    syn_modbus_master_queue_push(&q, &q4);
    syn_modbus_master_queue_step(&master, &q, 0);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);

    uint8_t r4[6] = {1, 2, 1, 0x01, 0, 0};
    crc = syn_crc16_modbus(r4, 4);
    r4[4] = (uint8_t)(crc & 0xFF);
    r4[5] = (uint8_t)(crc >> 8);
    for (int i = 0; i < 6; i++)
        syn_modbus_master_feed(&master, r4[i]);
    syn_modbus_master_queue_step(&master, &q, 10);
    TEST_ASSERT_EQUAL(0, q.count);
}

static int static_cb_called = 0;
static void test_modbus_cb(uint8_t slave, uint8_t fc, const uint16_t *data, uint16_t len,
                           SYN_Status status, void *user_ctx)
{
    (void)slave;
    (void)fc;
    (void)data;
    (void)len;
    (void)status;
    (void)user_ctx;
    static_cb_called++;
}

static void test_modbus_master_queue_retries(void)
{
    SYN_ModbusMaster master;
    syn_modbus_master_init(&master, 100); /* 100ms timeout */

    SYN_ModbusMasterQueue q;
    syn_modbus_master_queue_init(&q, 1); /* 1 retry */

    SYN_ModbusMasterQuery qry = {.slave_addr = 1,
                                 .func_code = SYN_MB_FC_READ_HOLDING,
                                 .start_addr = 0,
                                 .count = 1,
                                 .callback = test_modbus_cb,
                                 .user_ctx = NULL};
    syn_modbus_master_queue_push(&q, &qry);

    syn_modbus_master_queue_step(&master, &q, 1000);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);

    /* Timeout at 1150ms -> increments retry_count to 1, sets state to IDLE */
    syn_modbus_master_queue_step(&master, &q, 1150);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_IDLE, master.state);
    TEST_ASSERT_EQUAL(1, q.retry_count);

    /* Step again at 1150ms -> re-transmits query, sets state to WAITING_RESPONSE */
    syn_modbus_master_queue_step(&master, &q, 1150);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, master.state);

    /* Timeout second time at 1300ms -> retry count exhausted, callback called (line 456), query
     * popped */
    static_cb_called = 0;
    syn_modbus_master_queue_step(&master, &q, 1300);
    TEST_ASSERT_EQUAL(0, q.count);
    TEST_ASSERT_EQUAL(1, static_cb_called);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_IDLE, master.state);

    /* Unknown func_code query in queue (line 431 default) */
    SYN_ModbusMasterQuery qry_bad = {
        .slave_addr = 1, .func_code = 0x99, .start_addr = 0, .count = 1};
    syn_modbus_master_queue_push(&q, &qry_bad);
    syn_modbus_master_queue_step(&master, &q, 1400);
}

static void test_modbus_master_parameter_bounds(void)
{
    SYN_ModbusMaster m;
    syn_modbus_master_init(&m, 100);

    /* Null guards and invalid slave address (0) */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_read_holding(NULL, 1, 0, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_read_holding(&m, 0, 0, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_read_holding(&m, 1, 0, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_read_holding(&m, 1, 0, 126));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_read_input(&m, 0, 0, 1));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_read_input(&m, 1, 0, 126));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_write_single(&m, 0, 0, 123));

    uint16_t vals[2] = {1, 2};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_write_multiple(&m, 0, 0, 2, vals));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_write_multiple(&m, 1, 0, 0, vals));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_write_multiple(&m, 1, 0, 124, vals));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_write_multiple(&m, 1, 0, 2, NULL));
}

static void test_modbus_master_queue_null_and_full_checks(void)
{
    syn_modbus_master_queue_init(NULL, 1);

    SYN_ModbusMasterQueue q;
    SYN_ModbusMasterQuery q_item = {
        .slave_addr = 1, .func_code = SYN_MB_FC_READ_HOLDING, .start_addr = 0, .count = 1};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_queue_push(NULL, &q_item));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_queue_push(&q, NULL));

    syn_modbus_master_queue_init(&q, 1);
    for (int i = 0; i < SYN_MODBUS_QUEUE_SIZE; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_queue_push(&q, &q_item));
    }
    /* Overflow push */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_modbus_master_queue_push(&q, &q_item));
}

static void test_modbus_master_busy_state_checks(void)
{
    SYN_ModbusMaster m;
    syn_modbus_master_init(&m, 500);

    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&m, 1, 0, 1));
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, m.state);

    uint8_t coils = 0x01;
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_modbus_master_write_multiple_coils(&m, 1, 0, 1, &coils));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_modbus_master_mask_write_register(&m, 1, 0, 0xFFFF, 0x0000));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_modbus_master_read_fifo_queue(&m, 1, 0));
    TEST_ASSERT_EQUAL(SYN_BUSY, syn_modbus_master_report_server_id(&m, 1));
}

static void dummy_cb(uint8_t slave_addr, uint8_t fc, const uint16_t *data, uint16_t count,
                     SYN_Status status, void *user_ctx)
{
    (void)slave_addr;
    (void)fc;
    (void)data;
    (void)count;
    (void)status;
    if (user_ctx != NULL) {
        *(bool *)user_ctx = true;
    }
}

static void test_modbus_master_exception_response_handling(void)
{
    SYN_ModbusMaster m;
    syn_modbus_master_init(&m, 10);

    /* 1. report_server_id slave_addr == 0 (line 251) */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_report_server_id(&m, 0));

    /* 2. queue_step NULL params (line 404) */
    SYN_ModbusMasterQueue q;
    syn_modbus_master_queue_init(&q, 0);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_modbus_master_queue_step(NULL, &q, 0));

    /* 3. queue_step unsupported func_code (line 432) */
    SYN_ModbusMasterQuery unsupp_query = {
        .slave_addr = 1, .func_code = 0x99, .start_addr = 0, .count = 1};
    syn_modbus_master_queue_push(&q, &unsupp_query);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_queue_step(&m, &q, 0));

    /* 4. max retries timeout callback invocation (line 457) */
    SYN_ModbusMasterQueue q_cb;
    syn_modbus_master_queue_init(&q_cb, 0); /* 0 retries */
    bool cb_called = false;
    SYN_ModbusMasterQuery cb_query = {.slave_addr = 1,
                                      .func_code = SYN_MB_FC_READ_HOLDING,
                                      .start_addr = 0,
                                      .count = 1,
                                      .callback = dummy_cb,
                                      .user_ctx = &cb_called};
    syn_modbus_master_queue_push(&q_cb, &cb_query);
    syn_modbus_master_queue_step(&m, &q_cb, 100);
    /* Advance time to trigger timeout */
    syn_modbus_master_queue_step(&m, &q_cb, 600);
    TEST_ASSERT_TRUE(cb_called);

    /* 5. Large coil read (> 125 bytes, up to 200 bytes) */
    SYN_ModbusMaster m_coils;
    syn_modbus_master_init(&m_coils, 500);
    m_coils.state = SYN_MB_MASTER_STATE_WAITING_RESPONSE;
    m_coils.slave_addr = 1;
    m_coils.func_code = SYN_MB_FC_READ_COILS;
    uint8_t coil_resp[205];
    coil_resp[0] = 1;
    coil_resp[1] = SYN_MB_FC_READ_COILS;
    coil_resp[2] = 200; /* 200 bytes of coils */
    memset(&coil_resp[3], 0xAA, 200);
    syn_poke_u16_le(syn_crc16_modbus(coil_resp, 203), coil_resp, 203);
    for (size_t i = 0; i < 205; i++) {
        syn_modbus_master_feed(&m_coils, coil_resp[i]);
    }
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, syn_modbus_master_process(&m_coils, 100));
    TEST_ASSERT_EQUAL_UINT16(200, m_coils.read_count);
    TEST_ASSERT_EQUAL_HEX16(0xAA, m_coils.read_data[199]);
}

static void test_r5_a08_master_builders_and_tick_stamping(void)
{
    mock_port_reset();
    uint8_t coils[2] = {0x05U, 0x00U};
    uint8_t disc[2] = {0xA2U, 0x01U};
    uint16_t holding[4] = {0x1234U, 0x0000U, 0x0000U, 0x0000U};
    uint16_t fifo[2] = {0x0101U, 0x0202U};

    SYN_Modbus slave;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1U,
        .holding_regs = holding,
        .holding_count = 4U,
        .coils = coils,
        .coils_count = 16U,
        .discrete_inputs = disc,
        .discrete_count = 16U,
        .fifo_queue = fifo,
        .fifo_count = 2U,
    };
    syn_modbus_init(&slave, &cfg, slave_buf, sizeof(slave_buf));

    SYN_ModbusMaster m;
    syn_modbus_master_init(&m, 200U);

    /* 1. FC 0x01 Read Coils builds full 8-byte RTU request and round-trips against Slave */
    mock_tick_ms = 100U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_coils(&m, 1U, 0U, 8U));
    TEST_ASSERT_EQUAL_UINT16(8U, m.tx_len);
    TEST_ASSERT_EQUAL_UINT32(100U, m.request_tick_ms);
    loopback_transaction(&m, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, m.state);
    TEST_ASSERT_EQUAL_HEX16(0x05U, m.read_data[0]);

    /* 2. FC 0x02 Read Discrete Inputs builds full 8-byte RTU request */
    mock_tick_ms = 200U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_discrete_inputs(&m, 1U, 0U, 8U));
    TEST_ASSERT_EQUAL_UINT16(8U, m.tx_len);
    TEST_ASSERT_EQUAL_UINT32(200U, m.request_tick_ms);
    loopback_transaction(&m, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, m.state);
    TEST_ASSERT_EQUAL_HEX16(0xA2U, m.read_data[0]);

    /* 3. FC 0x05 Write Single Coil (ON then OFF) builds full 8-byte RTU request */
    mock_tick_ms = 300U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_write_single_coil(&m, 1U, 1U, true));
    TEST_ASSERT_EQUAL_UINT16(8U, m.tx_len);
    loopback_transaction(&m, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, m.state);
    TEST_ASSERT_EQUAL_HEX8(0x07U, coils[0]);

    mock_tick_ms = 400U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_write_single_coil(&m, 1U, 1U, false));
    TEST_ASSERT_EQUAL_UINT16(8U, m.tx_len);
    loopback_transaction(&m, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, m.state);
    TEST_ASSERT_EQUAL_HEX8(0x05U, coils[0]);

    /* 4. FC 0x0F, 0x16, 0x18, 0x11 refresh request_tick_ms so later calls do NOT instant-timeout */
    mock_tick_ms = 1000U;
    uint8_t wr_coils[1] = {0xF0U};
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_write_multiple_coils(&m, 1U, 0U, 8U, wr_coils));
    TEST_ASSERT_EQUAL_UINT32(1000U, m.request_tick_ms);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 1050U));
    loopback_transaction(&m, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, m.state);

    mock_tick_ms = 2000U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_mask_write_register(&m, 1U, 0U, 0xFF00U, 0x00CDU));
    TEST_ASSERT_EQUAL_UINT32(2000U, m.request_tick_ms);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 2050U));
    loopback_transaction(&m, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, m.state);
    TEST_ASSERT_EQUAL_HEX16(0x12CDU, holding[0]);

    mock_tick_ms = 3000U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_fifo_queue(&m, 1U, 0U));
    TEST_ASSERT_EQUAL_UINT32(3000U, m.request_tick_ms);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 3050U));
    loopback_transaction(&m, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, m.state);
    TEST_ASSERT_EQUAL_UINT16(2U, m.read_count);

    mock_tick_ms = 4000U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_report_server_id(&m, 1U));
    TEST_ASSERT_EQUAL_UINT32(4000U, m.request_tick_ms);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 4050U));
    loopback_transaction(&m, &slave);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_COMPLETE, m.state);

    mock_port_reset();
}

typedef struct {
    uint32_t calls;
    SYN_Status last_status;
    uint8_t last_fc;
} Pro10QueueCbCtx;

static void pro10_queue_cb(uint8_t slave_addr, uint8_t fc, const uint16_t *data, uint16_t count,
                           SYN_Status status, void *user_ctx)
{
    (void)slave_addr;
    (void)data;
    (void)count;
    Pro10QueueCbCtx *ctx = (Pro10QueueCbCtx *)user_ctx;
    ctx->calls++;
    ctx->last_status = status;
    ctx->last_fc = fc;
}

static void feed_master_crc_frame(SYN_ModbusMaster *m, uint8_t *frame, uint16_t payload_len)
{
    uint16_t crc = syn_crc16_modbus(frame, payload_len);
    frame[payload_len] = (uint8_t)(crc & 0xFFU);
    frame[payload_len + 1U] = (uint8_t)((crc >> 8U) & 0xFFU);
    for (uint16_t i = 0U; i < (uint16_t)(payload_len + 2U); i++) {
        syn_modbus_master_feed(m, frame[i]);
    }
}

static void test_pro10_master_expected_len_and_queue_invalid_pop(void)
{
    mock_port_reset();
    SYN_ModbusMaster m;

    /* 1. Truncated valid-CRC FC 0x03 response (claims byte_count=4 -> 9B, only 7B fed) */
    syn_modbus_master_init(&m, 200U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_holding(&m, 1U, 0U, 2U));
    uint8_t trunc_fc03[7] = {1U, 0x03U, 4U, 0x00U, 0x0AU, 0U, 0U};
    feed_master_crc_frame(&m, trunc_fc03, 5U);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 50U));
    TEST_ASSERT_EQUAL_UINT16(0U, m.read_count);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_TIMEOUT, syn_modbus_master_process(&m, 300U));

    /* 2. Truncated valid-CRC FC 0x06 (6B < 8B) and FC 0x16 (8B < 10B) */
    syn_modbus_master_init(&m, 200U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_write_single(&m, 1U, 0U, 0x1234U));
    uint8_t trunc_fc06[6] = {1U, 0x06U, 0U, 0U, 0U, 0U};
    feed_master_crc_frame(&m, trunc_fc06, 4U);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 50U));

    syn_modbus_master_init(&m, 200U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_mask_write_register(&m, 1U, 0U, 0xFFU, 0x01U));
    uint8_t trunc_fc16[8] = {1U, 0x16U, 0U, 0U, 0U, 0xFFU, 0U, 0U};
    feed_master_crc_frame(&m, trunc_fc16, 6U);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 50U));

    /* 3. Truncated valid-CRC FC 0x18: 6B (< 8B) and 8B claiming fifo_count=2 (< 12B) */
    syn_modbus_master_init(&m, 200U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_fifo_queue(&m, 1U, 0U));
    uint8_t trunc_fc18_6[6] = {1U, 0x18U, 0U, 6U, 0U, 0U};
    feed_master_crc_frame(&m, trunc_fc18_6, 4U);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 50U));

    syn_modbus_master_init(&m, 200U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_fifo_queue(&m, 1U, 0U));
    uint8_t trunc_fc18_8[8] = {1U, 0x18U, 0U, 6U, 0U, 2U, 0U, 0U};
    feed_master_crc_frame(&m, trunc_fc18_8, 6U);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 50U));

    /* Overflowing fifo_count = 0x8000 must not wrap uint16_t to 8B */
    syn_modbus_master_init(&m, 200U);
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_read_fifo_queue(&m, 1U, 0U));
    uint8_t ovf_fc18_8[8] = {1U, 0x18U, 0xFFU, 0xFFU, 0x80U, 0x00U, 0U, 0U};
    feed_master_crc_frame(&m, ovf_fc18_8, 6U);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 50U));

    /* Default 5B length fallback for FC 0x07 */
    syn_modbus_master_init(&m, 200U);
    m.slave_addr = 1U;
    m.func_code = SYN_MB_FC_READ_EXCEPTION_STATUS;
    m.state = SYN_MB_MASTER_STATE_WAITING_RESPONSE;
    uint8_t fc07_resp[5] = {1U, 0x07U, 0x5AU, 0U, 0U};
    feed_master_crc_frame(&m, fc07_resp, 3U);
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, syn_modbus_master_process(&m, 50U));

    /* 4. Queue step pops invalid query (SYN_INVALID_PARAM) and notifies callback without wedging */
    syn_modbus_master_init(&m, 200U);
    SYN_ModbusMasterQueue q;
    syn_modbus_master_queue_init(&q, 1U);
    Pro10QueueCbCtx cb_ctx = {0};

    SYN_ModbusMasterQuery bad_qry = {
        .slave_addr = 0U, /* invalid slave_addr -> SYN_INVALID_PARAM */
        .func_code = SYN_MB_FC_READ_HOLDING,
        .start_addr = 0U,
        .count = 1U,
        .callback = pro10_queue_cb,
        .user_ctx = &cb_ctx,
    };
    SYN_ModbusMasterQuery good_qry = {
        .slave_addr = 1U,
        .func_code = SYN_MB_FC_READ_HOLDING,
        .start_addr = 0U,
        .count = 1U,
        .callback = pro10_queue_cb,
        .user_ctx = &cb_ctx,
    };
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_queue_push(&q, &bad_qry));
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_queue_push(&q, &good_qry));
    TEST_ASSERT_EQUAL_UINT8(2U, q.count);

    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_queue_step(&m, &q, 100U));
    TEST_ASSERT_EQUAL_UINT32(1U, cb_ctx.calls);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, cb_ctx.last_status);
    TEST_ASSERT_EQUAL_UINT8(1U, q.count);

    /* Next step dispatches the good query behind it */
    TEST_ASSERT_EQUAL(SYN_OK, syn_modbus_master_queue_step(&m, &q, 110U));
    TEST_ASSERT_EQUAL(SYN_MB_MASTER_STATE_WAITING_RESPONSE, m.state);
}

void run_modbus_master_tests(void)
{
    RUN_TEST(test_modbus_master_read_holding);
    RUN_TEST(test_modbus_master_read_input);
    RUN_TEST(test_modbus_master_write_single_and_multiple);
    RUN_TEST(test_modbus_master_timeout_and_exceptions);
    RUN_TEST(test_modbus_master_process_edge_cases);
    RUN_TEST(test_modbus_master_new_queries);
    RUN_TEST(test_modbus_master_queue);
    RUN_TEST(test_modbus_master_queue_fc_variants);
    RUN_TEST(test_modbus_master_queue_retries);
    RUN_TEST(test_modbus_master_parameter_bounds);
    RUN_TEST(test_modbus_master_queue_null_and_full_checks);
    RUN_TEST(test_modbus_master_busy_state_checks);
    RUN_TEST(test_modbus_master_exception_response_handling);
    RUN_TEST(test_r5_a08_master_builders_and_tick_stamping);
    RUN_TEST(test_pro10_master_expected_len_and_queue_invalid_pop);
}
