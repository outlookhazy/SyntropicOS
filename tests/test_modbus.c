/**
 * @file test_modbus.c
 * @brief Unity tests for syn_modbus.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/proto/syn_modbus.h"

static void test_modbus(void)
{

    static uint16_t holding[8] = { 100, 200, 300, 400, 500, 600, 700, 800 };
    static uint16_t input[4]   = { 1000, 2000, 3000, 4000 };
    static uint8_t mb_buf[256];

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr    = 1,
        .uart          = 0,
        .holding_regs  = holding,
        .holding_count = 8,
        .input_regs    = input,
        .input_count   = 4,
        .on_write      = NULL,
        .on_write_ctx  = NULL,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* Build a Read Holding Registers request: addr=1, FC=03, start=0, count=2 */
    uint8_t req[8];
    req[0] = 1;     /* slave addr */
    req[1] = 0x03;  /* FC */
    req[2] = 0x00;  /* start addr high */
    req[3] = 0x00;  /* start addr low */
    req[4] = 0x00;  /* count high */
    req[5] = 0x02;  /* count low */
    uint16_t crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);

    /* Feed bytes */
    size_t i;
    for (i = 0; i < 8; i++) {
        mb.buf[i] = req[i];
    }
    mb.rx_len = 8;

    bool processed = syn_modbus_process(&mb);
    TEST_ASSERT_TRUE(processed);
    TEST_ASSERT_EQUAL_INT(1, mb.frames_rx);

    /* Build Write Single Register: addr=1, FC=06, reg=0, value=999 */
    req[0] = 1;
    req[1] = 0x06;
    req[2] = 0x00;
    req[3] = 0x00;  /* register 0 */
    req[4] = 0x03;
    req[5] = 0xE7;  /* value = 999 */
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);

    for (i = 0; i < 8; i++) {
        mb.buf[i] = req[i];
    }
    mb.rx_len = 8;

    processed = syn_modbus_process(&mb);
    TEST_ASSERT_TRUE(processed);
    TEST_ASSERT_EQUAL_INT(999, holding[0]);

    /* Wrong address — should be ignored */
    req[0] = 2; /* different slave */
    req[1] = 0x03;
    req[2] = 0; req[3] = 0;
    req[4] = 0; req[5] = 1;
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);

    for (i = 0; i < 8; i++) mb.buf[i] = req[i];
    mb.rx_len = 8;

    processed = syn_modbus_process(&mb);
    TEST_ASSERT_FALSE(processed);

    /* Bad CRC */
    req[0] = 1;
    req[6] = 0xFF; req[7] = 0xFF; /* corrupt CRC */
    for (i = 0; i < 8; i++) mb.buf[i] = req[i];
    mb.rx_len = 8;

    processed = syn_modbus_process(&mb);
    TEST_ASSERT_FALSE(processed);
}

void run_modbus_tests(void)
{
    RUN_TEST(test_modbus);
}
