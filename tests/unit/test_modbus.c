/**
 * @file test_modbus.c
 * @brief Unity tests for syn_modbus.
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_modbus.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static bool on_write_allow = true;

static bool test_on_write(SYN_Modbus *mb, uint16_t addr, uint16_t count, void *ctx)
{
    (void)mb;
    (void)count;
    (void)ctx;
    return on_write_allow && (addr != 5);
}

static void test_modbus_rtu_frame_crc_and_runt_buffer(void)
{
    static uint16_t holding[8] = {0};
    static uint8_t mb_buf[256];
    mock_port_reset();

    static char long_str[300];
    memset(long_str, 'A', 260);
    long_str[260] = '\0';
    SYN_Modbus_DeviceInfo dev_info = {.vendor_name = long_str};

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 8,
        .device_info = &dev_info,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* Short FC 0x06 write single frame (< 8 bytes) */
    uint8_t req[8] = {1, 0x06, 0x00, 0x00, 0x00, 0x00};
    uint16_t crc = syn_crc16_modbus(req, 4);
    req[4] = (uint8_t)(crc & 0xFF);
    req[5] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req, 6);
    mb.rx_len = 6;
    syn_modbus_process(&mb);

    /* MEI read code 0x04 & long string truncation & small buffer overflow */
    uint8_t mei_req[8] = {1, 0x2B, 0x0E, 0x04, 0x00, 0x00, 0x00, 0x00};
    crc = syn_crc16_modbus(mei_req, 5);
    mei_req[5] = (uint8_t)(crc & 0xFF);
    mei_req[6] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, mei_req, 7);
    mb.rx_len = 7;
    syn_modbus_process(&mb);

    /* Small buffer overflow check */
    static uint8_t small_buf[12];
    SYN_Modbus small_mb;
    syn_modbus_init(&small_mb, &cfg, small_buf, sizeof(small_buf));
    TEST_ASSERT_FALSE(syn_modbus_process(&small_mb));
    memcpy(small_mb.buf, mei_req, 7);
    small_mb.rx_len = 7;
    syn_modbus_process(&small_mb);
}

static void test_modbus_basic(void)
{
    static uint16_t holding[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    static uint16_t input[4] = {1000, 2000, 3000, 4000};
    static uint8_t mb_buf[256];

    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .uart = 0,
        .holding_regs = holding,
        .holding_count = 8,
        .input_regs = input,
        .input_count = 4,
        .on_write = test_on_write,
        .on_write_ctx = NULL,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. Build a Read Holding Registers request: addr=1, FC=03, start=0, count=2 */
    uint8_t req[20];
    req[0] = 1;    /* slave addr */
    req[1] = 0x03; /* FC */
    req[2] = 0x00; /* start addr high */
    req[3] = 0x00; /* start addr low */
    req[4] = 0x00; /* count high */
    req[5] = 0x02; /* count low */
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
    TEST_ASSERT_TRUE(mock_uart_tx_len > 0);

    /* 2. Build Write Single Register: addr=1, FC=06, reg=0, value=999 */
    mock_port_reset();
    req[0] = 1;
    req[1] = 0x06;
    req[2] = 0x00;
    req[3] = 0x00; /* register 0 */
    req[4] = 0x03;
    req[5] = 0xE7; /* value = 999 */
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

    /* 3. Wrong address — should be ignored */
    mock_port_reset();
    req[0] = 2; /* different slave */
    req[1] = 0x03;
    req[2] = 0;
    req[3] = 0;
    req[4] = 0;
    req[5] = 1;
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);

    for (i = 0; i < 8; i++)
        mb.buf[i] = req[i];
    mb.rx_len = 8;

    processed = syn_modbus_process(&mb);
    TEST_ASSERT_FALSE(processed);
    TEST_ASSERT_EQUAL_INT(0, mock_uart_tx_len);

    /* 4. Bad CRC */
    mock_port_reset();
    req[0] = 1;
    req[6] = 0xFF;
    req[7] = 0xFF; /* corrupt CRC */
    for (i = 0; i < 8; i++)
        mb.buf[i] = req[i];
    mb.rx_len = 8;

    processed = syn_modbus_process(&mb);
    TEST_ASSERT_FALSE(processed);
    TEST_ASSERT_EQUAL_INT(0, mock_uart_tx_len);
}

static void test_modbus_extended(void)
{
    static uint16_t holding[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    static uint16_t input[4] = {1000, 2000, 3000, 4000};
    static uint8_t mb_buf[256];

    mock_port_reset();
    on_write_allow = true;

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .uart = 0,
        .holding_regs = holding,
        .holding_count = 8,
        .input_regs = input,
        .input_count = 4,
        .on_write = test_on_write,
        .on_write_ctx = NULL,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. Read Input Registers (FC 0x04) */
    uint8_t req[30];
    req[0] = 1;    /* addr */
    req[1] = 0x04; /* FC 04 */
    req[2] = 0x00; /* start high */
    req[3] = 0x01; /* start low (register 1) */
    req[4] = 0x00; /* count high */
    req[5] = 0x02; /* count low (2 registers) */
    uint16_t crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);

    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    /* Expect response of length 3 (addr, FC, byte_count) + 4 (data) + 2 (CRC) = 9 */
    TEST_ASSERT_EQUAL_INT(9, mock_uart_tx_len);
    TEST_ASSERT_EQUAL_INT(0x04, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_INT(4, mock_uart_tx_buf[2]); /* byte count = 4 */
    /* Reg 1 value is 2000 (0x07D0) */
    TEST_ASSERT_EQUAL_INT(0x07, mock_uart_tx_buf[3]);
    TEST_ASSERT_EQUAL_INT(0xD0, mock_uart_tx_buf[4]);

    /* 2. Write Multiple Registers (FC 0x10) */
    mock_port_reset();
    req[0] = 1;
    req[1] = 0x10; /* FC 16 */
    req[2] = 0x00; /* start high */
    req[3] = 0x02; /* start low (register 2) */
    req[4] = 0x00; /* quantity high */
    req[5] = 0x02; /* quantity low (2 registers) */
    req[6] = 0x04; /* byte count */
    /* reg 2 = 1111 (0x0457) */
    req[7] = 0x04;
    req[8] = 0x57;
    /* reg 3 = 2222 (0x08AE) */
    req[9] = 0x08;
    req[10] = 0xAE;
    crc = syn_crc16_modbus(req, 11);
    req[11] = (uint8_t)(crc & 0xFF);
    req[12] = (uint8_t)((crc >> 8) & 0xFF);

    memcpy(mb.buf, req, 13);
    mb.rx_len = 13;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(1111, holding[2]);
    TEST_ASSERT_EQUAL_INT(2222, holding[3]);
    /* Expect response of length 8 */
    TEST_ASSERT_EQUAL_INT(8, mock_uart_tx_len);

    /* 3. Short Frame Check */
    mb.rx_len = 3;
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));
}

static void test_modbus_exceptions(void)
{
    static uint16_t holding[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    static uint16_t input[4] = {1000, 2000, 3000, 4000};
    static uint8_t mb_buf[256];

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 8,
        .input_regs = input,
        .input_count = 4,
        .on_write = test_on_write,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    uint8_t req[30] = {0};
    uint16_t crc;

    /* A. Illegal Function (FC 0x20) */
    mock_port_reset();
    req[0] = 1;
    req[1] = 0x20; /* Unsupported */
    req[2] = 0;
    req[3] = 0;
    req[4] = 0;
    req[5] = 0;
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(5, mock_uart_tx_len);       /* Exception response len = 5 */
    TEST_ASSERT_EQUAL_INT(0xA0, mock_uart_tx_buf[1]); /* FC | 0x80 */
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_FUNC, mock_uart_tx_buf[2]);

    /* B. Illegal Address (Read past end of holding register range) */
    mock_port_reset();
    req[1] = 0x03;
    req[3] = 7; /* Start at 7 */
    req[5] = 2; /* Quantity = 2 (7 + 2 = 9 > 8) */
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(0x83, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* C. Illegal Address (Write single past end) */
    mock_port_reset();
    req[1] = 0x06;
    req[3] = 8; /* Out of bounds */
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(0x86, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* D. Illegal Address (Write multiple past end) */
    mock_port_reset();
    req[1] = 0x10;
    req[3] = 7;
    req[5] = 2;
    req[6] = 4;
    crc = syn_crc16_modbus(req, 11);
    req[11] = (uint8_t)(crc & 0xFF);
    req[12] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 13);
    mb.rx_len = 13;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(0x90, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* E. Illegal Value (Read holding quantity = 0) */
    mock_port_reset();
    req[1] = 0x03;
    req[3] = 0;
    req[5] = 0; /* Quantity = 0 */
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* F. Illegal Value (Read holding quantity = 126) */
    mock_port_reset();
    req[5] = 126; /* Max is 125 */
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* G. Illegal Value (Write multiple quantity = 0) */
    mock_port_reset();
    req[1] = 0x10;
    req[3] = 0;
    req[5] = 0;
    req[6] = 0;
    crc = syn_crc16_modbus(req, 7);
    req[7] = (uint8_t)(crc & 0xFF);
    req[8] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 9);
    mb.rx_len = 9;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* H. Illegal Value (Write multiple byte count mismatch) */
    mock_port_reset();
    req[5] = 2;
    req[6] = 5; /* Should be 4 */
    crc = syn_crc16_modbus(req, 11);
    req[11] = (uint8_t)(crc & 0xFF);
    req[12] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 13);
    mb.rx_len = 13;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* I. Write Callback Rejection (Single register callback rejects) */
    mock_port_reset();
    on_write_allow = false; /* callback will deny */
    req[1] = 0x06;
    req[3] = 0;
    req[5] = 100;
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* J. Write Callback Rejection (Multiple registers callback rejects) */
    mock_port_reset();
    req[1] = 0x10;
    req[3] = 0;
    req[5] = 2;
    req[6] = 4;
    crc = syn_crc16_modbus(req, 11);
    req[11] = (uint8_t)(crc & 0xFF);
    req[12] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 13);
    mb.rx_len = 13;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* K. Specific Rejected Address via callback */
    mock_port_reset();
    on_write_allow = true;
    req[1] = 0x06;
    req[3] = 5; /* Address 5 is blocked in callback */
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);
}

static void test_modbus_broadcast(void)
{
    static uint16_t holding[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    static uint8_t mb_buf[256];

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 8,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    uint8_t req[30];
    uint16_t crc;

    /* A. Write Single Broadcast (address 0, should process but not send response) */
    mock_port_reset();
    req[0] = 0; /* Broadcast address */
    req[1] = 0x06;
    req[2] = 0;
    req[3] = 1; /* register 1 */
    req[4] = 0x03;
    req[5] = 0xE7; /* 999 */
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_FALSE(syn_modbus_process(&mb)); /* Returns false when no reply to send */
    TEST_ASSERT_EQUAL_INT(999, holding[1]);
    TEST_ASSERT_EQUAL_INT(0, mock_uart_tx_len);

    /* B. Write Multiple Broadcast */
    mock_port_reset();
    req[0] = 0;
    req[1] = 0x10;
    req[2] = 0;
    req[3] = 2;
    req[4] = 0;
    req[5] = 2;
    req[6] = 4;
    req[7] = 0x0F;
    req[8] = 0xA0; /* 4000 */
    req[9] = 0x13;
    req[10] = 0x88; /* 5000 */
    crc = syn_crc16_modbus(req, 11);
    req[11] = (uint8_t)(crc & 0xFF);
    req[12] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 13);
    mb.rx_len = 13;
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(4000, holding[2]);
    TEST_ASSERT_EQUAL_INT(5000, holding[3]);
    TEST_ASSERT_EQUAL_INT(0, mock_uart_tx_len);

    /* C. Read Holding Broadcast (should ignore/break out) */
    mock_port_reset();
    req[0] = 0;
    req[1] = 0x03;
    req[2] = 0;
    req[3] = 0;
    req[4] = 0;
    req[5] = 2;
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(0, mock_uart_tx_len);

    /* D. Read Input Broadcast */
    mock_port_reset();
    req[0] = 0;
    req[1] = 0x04;
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(0, mock_uart_tx_len);

    /* E. Unsupported Function Broadcast */
    mock_port_reset();
    req[0] = 0;
    req[1] = 0x99;
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(0, mock_uart_tx_len);

    /* F. Broadcast Write Exception (should not respond, but increment errors) */
    mock_port_reset();
    req[0] = 0;
    req[1] = 0x06;
    req[2] = 0;
    req[3] = 10; /* Illegal register 10 */
    req[4] = 0x03;
    req[5] = 0xE7;
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;

    uint32_t prev_errors = mb.errors;
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(0, mock_uart_tx_len);
    TEST_ASSERT_EQUAL_UINT32(prev_errors + 1, mb.errors);
}

static void test_modbus_feed_timeout(void)
{
    static uint8_t mb_buf[10];

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {.slave_addr = 1};
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    mock_tick_ms = 0;
    syn_modbus_feed(&mb, 1);
    syn_modbus_feed(&mb, 2);
    TEST_ASSERT_EQUAL_INT(2, mb.rx_len);

    /* Advance time past MB_SILENCE_MS gap to start new frame */
    mock_tick_advance(10);
    syn_modbus_feed(&mb, 3);
    /* rx_len should be reset to 0 internally and then not accumulate */
    TEST_ASSERT_EQUAL_INT(0, mb.rx_len);

    /* Feed frame that exceeds buffer size */
    mock_port_reset();
    syn_modbus_reset(&mb);
    for (int i = 0; i < 15; i++) {
        syn_modbus_feed(&mb, (uint8_t)i);
    }
    TEST_ASSERT_EQUAL_INT(10, mb.rx_len); /* capped at buffer size */
}

static void test_modbus_polling_reset(void)
{
    static uint16_t holding[2] = {0};
    static uint8_t mb_buf[64];

    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .uart = 0,
        .holding_regs = holding,
        .holding_count = 2,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* A. syn_modbus_reset */
    mb.rx_len = 10;
    mb.frame_ready = true;
    syn_modbus_reset(&mb);
    TEST_ASSERT_EQUAL_INT(0, mb.rx_len);
    TEST_ASSERT_FALSE(mb.frame_ready);

    /* B. syn_modbus_poll no data */
    mock_tick_ms = 100;
    TEST_ASSERT_FALSE(syn_modbus_poll(&mb));

    /* C. syn_modbus_poll with frame */
    uint8_t req[8];
    req[0] = 1;    /* addr */
    req[1] = 0x03; /* FC */
    req[2] = 0;
    req[3] = 0;
    req[4] = 0;
    req[5] = 1;
    uint16_t crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)((crc >> 8) & 0xFF);

    memcpy(mock_uart_rx_buf, req, 8);
    mock_uart_rx_len = 8;
    mock_uart_rx_pos = 0;

    /* Feed it in poll */
    TEST_ASSERT_FALSE(syn_modbus_poll(&mb)); /* bytes read, but no silence gap yet */
    TEST_ASSERT_EQUAL_INT(8, mb.rx_len);

    /* Wait for silence gap and poll again to trigger process */
    mock_tick_advance(10);
    TEST_ASSERT_TRUE(syn_modbus_poll(&mb));
    TEST_ASSERT_EQUAL_INT(0, mb.rx_len); /* processed and cleared */
}

/**
 * Validates configurable inter-frame silence gap via silence_ms.
 *
 * With silence_ms = 20, the default 5ms gap should NOT trigger
 * a frame timeout, but 20ms should.
 */
static void test_modbus_custom_silence(void)
{
    static uint8_t mb_buf[64];

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1, .silence_ms = 20, /* Custom: 20ms instead of default 5ms */
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* Feed two bytes at t=0 */
    mock_tick_ms = 0;
    syn_modbus_feed(&mb, 0xAA);
    syn_modbus_feed(&mb, 0xBB);
    TEST_ASSERT_EQUAL_INT(2, mb.rx_len);

    /* Advance 10 ms — past the default MB_SILENCE_MS (5ms) but below
     * our custom 20ms.  Feed should accumulate, not reset. */
    mock_tick_advance(10);
    syn_modbus_feed(&mb, 0xCC);
    TEST_ASSERT_EQUAL_INT(3, mb.rx_len); /* still accumulating */

    /* Advance to 25 ms total — now past our custom silence gap.
     * Next feed should detect the gap and start a new frame. */
    mock_tick_advance(25);
    syn_modbus_feed(&mb, 0xDD);
    /* The 3-byte runt was discarded (< MB_MIN_FRAME_LEN), rx_len reset */
    TEST_ASSERT_EQUAL_INT(0, mb.rx_len);

    /* B. Zero silence_ms (default fallback) should use MB_SILENCE_MS */
    SYN_Modbus mb2;
    SYN_Modbus_Config cfg2 = {
        .slave_addr = 1, .silence_ms = 0, /* Should fall back to default 5ms */
    };
    syn_modbus_init(&mb2, &cfg2, mb_buf, sizeof(mb_buf));

    mock_tick_ms = 0;
    syn_modbus_feed(&mb2, 0x01);
    syn_modbus_feed(&mb2, 0x02);
    TEST_ASSERT_EQUAL_INT(2, mb2.rx_len);

    /* 6ms gap — exceeds default 5ms, should trigger reset */
    mock_tick_advance(6);
    syn_modbus_feed(&mb2, 0x03);
    TEST_ASSERT_EQUAL_INT(0, mb2.rx_len); /* runt discarded */
}

static bool mock_read_file(SYN_Modbus *mb, uint16_t file_num, uint16_t record_num,
                           uint16_t record_len, uint16_t *record_data, void *ctx)
{
    (void)mb;
    (void)ctx;
    if (file_num != 1 || record_num > 10)
        return false;
    for (uint16_t i = 0; i < record_len; i++) {
        record_data[i] = (uint16_t)(file_num * 1000 + record_num * 10 + i);
    }
    return true;
}

static uint16_t mock_file_store[10];
static bool mock_write_file(SYN_Modbus *mb, uint16_t file_num, uint16_t record_num,
                            uint16_t record_len, const uint16_t *record_data, void *ctx)
{
    (void)mb;
    (void)ctx;
    if (file_num != 1 || record_num >= 10 || record_len > 10)
        return false;
    for (uint16_t i = 0; i < record_len; i++) {
        mock_file_store[record_num + i] = record_data[i];
    }
    return true;
}

static void test_modbus_read_exception_status(void)
{
    static uint8_t mb_buf[64];
    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .exception_status = 0x5A,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* Build FC 0x07 request */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_EXCEPTION_STATUS;
    uint16_t crc = syn_crc16_modbus(mb.buf, 2);
    mb.buf[2] = (uint8_t)(crc & 0xFF);
    mb.buf[3] = (uint8_t)(crc >> 8);
    mb.rx_len = 4;

    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(1, mb.frames_rx);
    TEST_ASSERT_EQUAL_INT(5, mock_uart_tx_len);
    TEST_ASSERT_EQUAL_HEX8(1, mock_uart_tx_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x07, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x5A, mock_uart_tx_buf[2]);
}

static void test_modbus_read_write_multiple(void)
{
    static uint16_t holding[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    static uint8_t mb_buf[256];
    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 8,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* FC 0x17: Read 2 regs from addr 0; Write 2 regs to addr 2 (val 888, 999) */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_WRITE_MULTIPLE;
    mb.buf[2] = 0x00;
    mb.buf[3] = 0x00; /* Read start = 0 */
    mb.buf[4] = 0x00;
    mb.buf[5] = 0x02; /* Read count = 2 */
    mb.buf[6] = 0x00;
    mb.buf[7] = 0x02; /* Write start = 2 */
    mb.buf[8] = 0x00;
    mb.buf[9] = 0x02;  /* Write count = 2 */
    mb.buf[10] = 0x04; /* Write byte count = 4 */
    mb.buf[11] = 0x03;
    mb.buf[12] = 0x78; /* 888 */
    mb.buf[13] = 0x03;
    mb.buf[14] = 0xE7; /* 999 */
    uint16_t crc = syn_crc16_modbus(mb.buf, 15);
    mb.buf[15] = (uint8_t)(crc & 0xFF);
    mb.buf[16] = (uint8_t)(crc >> 8);
    mb.rx_len = 17;

    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_INT(888, holding[2]);
    TEST_ASSERT_EQUAL_INT(999, holding[3]);
    TEST_ASSERT_EQUAL_HEX8(0x17, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_INT(4, mock_uart_tx_buf[2]); /* byte count */
}

static void test_modbus_read_device_identification(void)
{
    static uint8_t mb_buf[256];
    mock_port_reset();

    SYN_Modbus_DeviceInfo dev_info = {
        .vendor_name = "Acme Corp",
        .product_code = "ACME-100",
        .revision = "v2.1.0",
    };

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .device_info = &dev_info,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* FC 0x2B / MEI 0x0E / Read Code 0x01 / Obj 0x00 */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
    mb.buf[2] = SYN_MB_MEI_TYPE_READ_DEVICE_ID;
    mb.buf[3] = 0x01; /* Read basic */
    mb.buf[4] = 0x00; /* Start object */
    uint16_t crc = syn_crc16_modbus(mb.buf, 5);
    mb.buf[5] = (uint8_t)(crc & 0xFF);
    mb.buf[6] = (uint8_t)(crc >> 8);
    mb.rx_len = 7;

    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x2B, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x0E, mock_uart_tx_buf[2]);
    TEST_ASSERT_EQUAL_INT(3, mock_uart_tx_buf[7]); /* 3 objects */
}

static void test_modbus_file_records(void)
{
    static uint8_t mb_buf[256];
    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .on_read_file = mock_read_file,
        .on_write_file = mock_write_file,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. Read file record (FC 0x14) */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    mb.buf[2] = 0x07; /* byte count */
    mb.buf[3] = 0x06; /* ref type */
    mb.buf[4] = 0x00;
    mb.buf[5] = 0x01; /* file 1 */
    mb.buf[6] = 0x00;
    mb.buf[7] = 0x02; /* record 2 */
    mb.buf[8] = 0x00;
    mb.buf[9] = 0x02; /* len 2 */
    uint16_t crc = syn_crc16_modbus(mb.buf, 10);
    mb.buf[10] = (uint8_t)(crc & 0xFF);
    mb.buf[11] = (uint8_t)(crc >> 8);
    mb.rx_len = 12;

    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x14, mock_uart_tx_buf[1]);

    /* 2. Write file record (FC 0x15) */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x0B; /* data len = 7 + 4 = 11 */
    mb.buf[3] = 0x06;
    mb.buf[4] = 0x00;
    mb.buf[5] = 0x01; /* file 1 */
    mb.buf[6] = 0x00;
    mb.buf[7] = 0x00; /* record 0 */
    mb.buf[8] = 0x00;
    mb.buf[9] = 0x02; /* len 2 */
    mb.buf[10] = 0x12;
    mb.buf[11] = 0x34; /* val 0x1234 */
    mb.buf[12] = 0x56;
    mb.buf[13] = 0x78; /* val 0x5678 */
    crc = syn_crc16_modbus(mb.buf, 14);
    mb.buf[14] = (uint8_t)(crc & 0xFF);
    mb.buf[15] = (uint8_t)(crc >> 8);
    mb.rx_len = 16;

    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX16(0x1234, mock_file_store[0]);
    TEST_ASSERT_EQUAL_HEX16(0x5678, mock_file_store[1]);
}

static bool mock_file_cb(SYN_Modbus *m, uint16_t f, uint16_t r, uint16_t l, uint16_t *d, void *c)
{
    (void)m;
    (void)f;
    (void)r;
    (void)l;
    (void)d;
    (void)c;
    return false;
}

static bool mock_file_write_cb(SYN_Modbus *m, uint16_t f, uint16_t r, uint16_t l, const uint16_t *d,
                               void *c)
{
    (void)m;
    (void)f;
    (void)r;
    (void)l;
    (void)d;
    (void)c;
    return false;
}

static void test_modbus_ext_error_paths(void)
{
    static uint16_t holding[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    static uint8_t mb_buf[256];
    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 8,
        .on_read_file = NULL, /* No callbacks configured */
        .on_write_file = NULL,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. FC 0x14 without callback -> EXCEPTION ILLEGAL_VALUE */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    mb.buf[2] = 0x07;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 2;
    mb.buf[8] = 0;
    mb.buf[9] = 2;
    uint16_t crc = syn_crc16_modbus(mb.buf, 10);
    mb.buf[10] = (uint8_t)(crc & 0xFF);
    mb.buf[11] = (uint8_t)(crc >> 8);
    mb.rx_len = 12;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x94, mock_uart_tx_buf[1]); /* Exception FC 0x80 | 0x14 */

    /* 2. FC 0x15 without callback -> EXCEPTION ILLEGAL_VALUE */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x0B;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 2;
    mb.buf[10] = 0x12;
    mb.buf[11] = 0x34;
    mb.buf[12] = 0x56;
    mb.buf[13] = 0x78;
    crc = syn_crc16_modbus(mb.buf, 14);
    mb.buf[14] = (uint8_t)(crc & 0xFF);
    mb.buf[15] = (uint8_t)(crc >> 8);
    mb.rx_len = 16;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);

    /* 3. FC 0x17 out of range -> EXCEPTION ILLEGAL_ADDR */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_WRITE_MULTIPLE;
    mb.buf[2] = 0x00;
    mb.buf[3] = 0x00;
    mb.buf[4] = 0x00;
    mb.buf[5] = 0x02;
    mb.buf[6] = 0x00;
    mb.buf[7] = 0x07;
    mb.buf[8] = 0x00;
    mb.buf[9] = 0x03; /* 7+3=10 > 8 */
    mb.buf[10] = 0x06;
    crc = syn_crc16_modbus(mb.buf, 11);
    mb.buf[11] = (uint8_t)(crc & 0xFF);
    mb.buf[12] = (uint8_t)(crc >> 8);
    mb.rx_len = 13;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x97, mock_uart_tx_buf[1]);

    /* 4. FC 0x2B invalid MEI type -> EXCEPTION ILLEGAL_VALUE */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
    mb.buf[2] = 0x0F; /* invalid */
    crc = syn_crc16_modbus(mb.buf, 3);
    mb.buf[3] = (uint8_t)(crc & 0xFF);
    mb.buf[4] = (uint8_t)(crc >> 8);
    mb.rx_len = 5;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0xAB, mock_uart_tx_buf[1]);

    /* 5. Read Coils count 0 & out of range */
    mock_port_reset();
    static uint8_t coils[2] = {0x55, 0xAA};
    cfg.coils = coils;
    cfg.coils_count = 16;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_COILS;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 0;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)(crc & 0xFF);
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x81, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_COILS;
    mb.buf[2] = 0;
    mb.buf[3] = 10;
    mb.buf[4] = 0;
    mb.buf[5] = 10;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)(crc & 0xFF);
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x81, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* 6. Write Single Coil out of range */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_SINGLE_COIL;
    mb.buf[2] = 0;
    mb.buf[3] = 20;
    mb.buf[4] = 0xFF;
    mb.buf[5] = 0;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)(crc & 0xFF);
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x85, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* 7. Write Multiple Coils invalid count & out of range */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_MULTIPLE_COILS;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 0;
    mb.buf[6] = 0;
    crc = syn_crc16_modbus(mb.buf, 7);
    mb.buf[7] = (uint8_t)(crc & 0xFF);
    mb.buf[8] = (uint8_t)(crc >> 8);
    mb.rx_len = 9;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x8F, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_MULTIPLE_COILS;
    mb.buf[2] = 0;
    mb.buf[3] = 10;
    mb.buf[4] = 0;
    mb.buf[5] = 10;
    mb.buf[6] = 2;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    crc = syn_crc16_modbus(mb.buf, 9);
    mb.buf[9] = (uint8_t)(crc & 0xFF);
    mb.buf[10] = (uint8_t)(crc >> 8);
    mb.rx_len = 11;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x8F, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* 8. Diagnostics unsupported subfunction */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_SINGLE;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    crc = syn_crc16_modbus(mb.buf, 4);
    mb.buf[4] = (uint8_t)(crc & 0xFF);
    mb.buf[5] = (uint8_t)(crc >> 8);
    mb.rx_len = 6; /* frame_len = 6 < 8 */
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x86, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_DIAGNOSTICS;
    mb.buf[2] = 0;
    mb.buf[3] = 1;
    mb.buf[4] = 0;
    mb.buf[5] = 0;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)(crc & 0xFF);
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x88, mock_uart_tx_buf[1]);

    /* 9. Mask Write Register out of range & on_write rejection */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_MASK_WRITE_REGISTER;
    mb.buf[2] = 0;
    mb.buf[3] = 20;
    mb.buf[4] = 0;
    mb.buf[5] = 0;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    crc = syn_crc16_modbus(mb.buf, 8);
    mb.buf[8] = (uint8_t)(crc & 0xFF);
    mb.buf[9] = (uint8_t)(crc >> 8);
    mb.rx_len = 10;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x96, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    mock_port_reset();
    cfg.on_write = test_on_write;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_MASK_WRITE_REGISTER;
    mb.buf[2] = 0;
    mb.buf[3] = 5; /* rejected addr 5 */
    mb.buf[4] = 0;
    mb.buf[5] = 0;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    crc = syn_crc16_modbus(mb.buf, 8);
    mb.buf[8] = (uint8_t)(crc & 0xFF);
    mb.buf[9] = (uint8_t)(crc >> 8);
    mb.rx_len = 10;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x96, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 10. Read FIFO Queue fifo_count > 31 */
    mock_port_reset();
    static uint16_t large_fifo[35];
    cfg.fifo_queue = large_fifo;
    cfg.fifo_count = 35;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FIFO_QUEUE;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    crc = syn_crc16_modbus(mb.buf, 4);
    mb.buf[4] = (uint8_t)(crc & 0xFF);
    mb.buf[5] = (uint8_t)(crc >> 8);
    mb.rx_len = 6;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x98, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_DEVICE_FAILURE, mock_uart_tx_buf[2]);

    /* 11. Read Write Multiple invalid write_count & on_write rejection */
    mock_port_reset();
    cfg.on_write = test_on_write;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_WRITE_MULTIPLE;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 1; /* read 0:1 */
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 0; /* write_count = 0 */
    mb.buf[10] = 0;
    crc = syn_crc16_modbus(mb.buf, 11);
    mb.buf[11] = (uint8_t)(crc & 0xFF);
    mb.buf[12] = (uint8_t)(crc >> 8);
    mb.rx_len = 13;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x97, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_WRITE_MULTIPLE;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 5;
    mb.buf[8] = 0;
    mb.buf[9] = 1; /* write to 5 (rejected) */
    mb.buf[10] = 2;
    mb.buf[11] = 0x12;
    mb.buf[12] = 0x34;
    crc = syn_crc16_modbus(mb.buf, 13);
    mb.buf[13] = (uint8_t)(crc & 0xFF);
    mb.buf[14] = (uint8_t)(crc >> 8);
    mb.rx_len = 15;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x97, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 12. Device Info invalid read_code & custom fields & default fields */
    mock_port_reset();
    static SYN_Modbus_DeviceInfo dev_info = {.vendor_name = "V",
                                             .product_code = "P",
                                             .revision = "R",
                                             .vendor_url = "http://url",
                                             .product_name = "PN",
                                             .model_name = "MN",
                                             .user_app_name = "UA"};
    cfg.device_info = &dev_info;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
    mb.buf[2] = 0x0E;
    mb.buf[3] = 0x05; /* read code 5 invalid */
    mb.buf[4] = 0x00;
    crc = syn_crc16_modbus(mb.buf, 5);
    mb.buf[5] = (uint8_t)(crc & 0xFF);
    mb.buf[6] = (uint8_t)(crc >> 8);
    mb.rx_len = 7;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0xAB, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* Read custom device info objects 0x03..0x06 */
    for (uint8_t obj_id = 3; obj_id <= 6; obj_id++) {
        mock_port_reset();
        mb.buf[0] = 1;
        mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
        mb.buf[2] = 0x0E;
        mb.buf[3] = 0x01;
        mb.buf[4] = obj_id;
        crc = syn_crc16_modbus(mb.buf, 5);
        mb.buf[5] = (uint8_t)(crc & 0xFF);
        mb.buf[6] = (uint8_t)(crc >> 8);
        mb.rx_len = 7;
        TEST_ASSERT_TRUE(syn_modbus_process(&mb));
        TEST_ASSERT_EQUAL_HEX8(0x2B, mock_uart_tx_buf[1]);
    }

    /* Read default device info (device_info = NULL) for obj_ids 0, 1, 2, and unknown 9 */
    mock_port_reset();
    cfg.device_info = NULL;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    for (uint8_t obj_id = 0; obj_id <= 2; obj_id++) {
        mock_port_reset();
        mb.buf[0] = 1;
        mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
        mb.buf[2] = 0x0E;
        mb.buf[3] = 0x01;
        mb.buf[4] = obj_id;
        crc = syn_crc16_modbus(mb.buf, 5);
        mb.buf[5] = (uint8_t)(crc & 0xFF);
        mb.buf[6] = (uint8_t)(crc >> 8);
        mb.rx_len = 7;
        TEST_ASSERT_TRUE(syn_modbus_process(&mb));
        TEST_ASSERT_EQUAL_HEX8(0x2B, mock_uart_tx_buf[1]);
    }

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
    mb.buf[2] = 0x0E;
    mb.buf[3] = 0x01;
    mb.buf[4] = 9; /* unknown id */
    crc = syn_crc16_modbus(mb.buf, 5);
    mb.buf[5] = (uint8_t)(crc & 0xFF);
    mb.buf[6] = (uint8_t)(crc >> 8);
    mb.rx_len = 7;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x2B, mock_uart_tx_buf[1]);

    /* 13. File Record error cases */
    mock_port_reset();
    cfg.on_read_file = mock_file_cb;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* FC 0x14 bad ref_type */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    mb.buf[2] = 0x07;
    mb.buf[3] = 0x05; /* ref_type != 0x06 */
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 2;
    mb.buf[8] = 0;
    mb.buf[9] = 2;
    crc = syn_crc16_modbus(mb.buf, 10);
    mb.buf[10] = (uint8_t)(crc & 0xFF);
    mb.buf[11] = (uint8_t)(crc >> 8);
    mb.rx_len = 12;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x94, mock_uart_tx_buf[1]);

    /* FC 0x14 out of range */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    mb.buf[2] = 0x07;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0x27;
    mb.buf[7] = 0x0E;
    mb.buf[8] = 0;
    mb.buf[9] = 10; /* 9998 + 10 = 10008 > 10000 */
    crc = syn_crc16_modbus(mb.buf, 10);
    mb.buf[10] = (uint8_t)(crc & 0xFF);
    mb.buf[11] = (uint8_t)(crc >> 8);
    mb.rx_len = 12;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x94, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* FC 0x14 callback returns false */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    mb.buf[2] = 0x07;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 2;
    crc = syn_crc16_modbus(mb.buf, 10);
    mb.buf[10] = (uint8_t)(crc & 0xFF);
    mb.buf[11] = (uint8_t)(crc >> 8);
    mb.rx_len = 12;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x94, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* FC 0x15 file write error cases */
    cfg.on_write_file = mock_file_write_cb;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* FC 0x15 byte_count < 7 */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x05; /* byte_count < 7 */
    crc = syn_crc16_modbus(mb.buf, 3);
    mb.buf[3] = (uint8_t)(crc & 0xFF);
    mb.buf[4] = (uint8_t)(crc >> 8);
    mb.rx_len = 5;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);

    /* FC 0x15 record_len mismatch */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x09;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 2; /* len = 2, but only 2 bytes data (1 word) provided */
    mb.buf[10] = 0x12;
    mb.buf[11] = 0x34;
    crc = syn_crc16_modbus(mb.buf, 12);
    mb.buf[12] = (uint8_t)(crc & 0xFF);
    mb.buf[13] = (uint8_t)(crc >> 8);
    mb.rx_len = 14;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);

    /* FC 0x15 out of range */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x0B;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0x27;
    mb.buf[7] = 0x0E;
    mb.buf[8] = 0;
    mb.buf[9] = 10;
    mb.buf[10] = 0;
    mb.buf[11] = 0;
    mb.buf[12] = 0;
    mb.buf[13] = 0;
    crc = syn_crc16_modbus(mb.buf, 14);
    mb.buf[14] = (uint8_t)(crc & 0xFF);
    mb.buf[15] = (uint8_t)(crc >> 8);
    mb.rx_len = 16;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* FC 0x15 callback returns false */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x0B;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 2;
    mb.buf[10] = 0x12;
    mb.buf[11] = 0x34;
    mb.buf[12] = 0x56;
    mb.buf[13] = 0x78;
    crc = syn_crc16_modbus(mb.buf, 14);
    mb.buf[14] = (uint8_t)(crc & 0xFF);
    mb.buf[15] = (uint8_t)(crc >> 8);
    mb.rx_len = 16;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* 11. Read Write Multiple invalid write_count & on_write rejection */
    mock_port_reset();
    cfg.on_write = test_on_write;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_WRITE_MULTIPLE;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 1; /* read 0:1 */
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 0; /* write_count = 0 */
    mb.buf[10] = 0;
    crc = syn_crc16_modbus(mb.buf, 11);
    mb.buf[11] = (uint8_t)(crc & 0xFF);
    mb.buf[12] = (uint8_t)(crc >> 8);
    mb.rx_len = 13;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x97, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_WRITE_MULTIPLE;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 5;
    mb.buf[8] = 0;
    mb.buf[9] = 1; /* write to 5 (rejected) */
    mb.buf[10] = 2;
    mb.buf[11] = 0x12;
    mb.buf[12] = 0x34;
    crc = syn_crc16_modbus(mb.buf, 13);
    mb.buf[13] = (uint8_t)(crc & 0xFF);
    mb.buf[14] = (uint8_t)(crc >> 8);
    mb.rx_len = 15;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x97, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 12. Device Info invalid read_code & custom fields & default fields */
    mock_port_reset();
    dev_info.vendor_name = "V";
    dev_info.product_code = "P";
    dev_info.revision = "R";
    dev_info.vendor_url = "http://url";
    dev_info.product_name = "PN";
    dev_info.model_name = "MN";
    dev_info.user_app_name = "UA";
    cfg.device_info = &dev_info;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
    mb.buf[2] = 0x0E;
    mb.buf[3] = 0x05; /* read code 5 invalid */
    mb.buf[4] = 0x00;
    crc = syn_crc16_modbus(mb.buf, 5);
    mb.buf[5] = (uint8_t)(crc & 0xFF);
    mb.buf[6] = (uint8_t)(crc >> 8);
    mb.rx_len = 7;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0xAB, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* Read custom device info objects 0x03..0x06 */
    for (uint8_t obj_id = 3; obj_id <= 6; obj_id++) {
        mock_port_reset();
        mb.buf[0] = 1;
        mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
        mb.buf[2] = 0x0E;
        mb.buf[3] = 0x01;
        mb.buf[4] = obj_id;
        crc = syn_crc16_modbus(mb.buf, 5);
        mb.buf[5] = (uint8_t)(crc & 0xFF);
        mb.buf[6] = (uint8_t)(crc >> 8);
        mb.rx_len = 7;
        TEST_ASSERT_TRUE(syn_modbus_process(&mb));
        TEST_ASSERT_EQUAL_HEX8(0x2B, mock_uart_tx_buf[1]);
    }

    /* Read default device info (device_info = NULL) for obj_ids 0, 1, 2, and unknown 9 */
    mock_port_reset();
    cfg.device_info = NULL;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    for (uint8_t obj_id = 0; obj_id <= 2; obj_id++) {
        mock_port_reset();
        mb.buf[0] = 1;
        mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
        mb.buf[2] = 0x0E;
        mb.buf[3] = 0x01;
        mb.buf[4] = obj_id;
        crc = syn_crc16_modbus(mb.buf, 5);
        mb.buf[5] = (uint8_t)(crc & 0xFF);
        mb.buf[6] = (uint8_t)(crc >> 8);
        mb.rx_len = 7;
        TEST_ASSERT_TRUE(syn_modbus_process(&mb));
        TEST_ASSERT_EQUAL_HEX8(0x2B, mock_uart_tx_buf[1]);
    }

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
    mb.buf[2] = 0x0E;
    mb.buf[3] = 0x01;
    mb.buf[4] = 9; /* unknown id */
    crc = syn_crc16_modbus(mb.buf, 5);
    mb.buf[5] = (uint8_t)(crc & 0xFF);
    mb.buf[6] = (uint8_t)(crc >> 8);
    mb.rx_len = 7;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x2B, mock_uart_tx_buf[1]);

    /* 13. File Record error cases */
    mock_port_reset();
    cfg.on_read_file = mock_file_cb;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* FC 0x14 bad ref_type */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    mb.buf[2] = 0x07;
    mb.buf[3] = 0x05; /* ref_type != 0x06 */
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 2;
    mb.buf[8] = 0;
    mb.buf[9] = 2;
    crc = syn_crc16_modbus(mb.buf, 10);
    mb.buf[10] = (uint8_t)(crc & 0xFF);
    mb.buf[11] = (uint8_t)(crc >> 8);
    mb.rx_len = 12;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x94, mock_uart_tx_buf[1]);

    /* FC 0x14 out of range */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    mb.buf[2] = 0x07;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0x27;
    mb.buf[7] = 0x0E;
    mb.buf[8] = 0;
    mb.buf[9] = 10; /* 9998 + 10 = 10008 > 10000 */
    crc = syn_crc16_modbus(mb.buf, 10);
    mb.buf[10] = (uint8_t)(crc & 0xFF);
    mb.buf[11] = (uint8_t)(crc >> 8);
    mb.rx_len = 12;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x94, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* FC 0x14 callback returns false */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    mb.buf[2] = 0x07;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 2;
    crc = syn_crc16_modbus(mb.buf, 10);
    mb.buf[10] = (uint8_t)(crc & 0xFF);
    mb.buf[11] = (uint8_t)(crc >> 8);
    mb.rx_len = 12;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x94, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* FC 0x15 file write error cases */
    cfg.on_write_file = mock_file_write_cb;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* FC 0x15 byte_count < 7 */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x05; /* byte_count < 7 */
    crc = syn_crc16_modbus(mb.buf, 3);
    mb.buf[3] = (uint8_t)(crc & 0xFF);
    mb.buf[4] = (uint8_t)(crc >> 8);
    mb.rx_len = 5;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);

    /* FC 0x15 record_len mismatch */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x09;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 2; /* len = 2, but only 2 bytes data (1 word) provided */
    mb.buf[10] = 0x12;
    mb.buf[11] = 0x34;
    crc = syn_crc16_modbus(mb.buf, 12);
    mb.buf[12] = (uint8_t)(crc & 0xFF);
    mb.buf[13] = (uint8_t)(crc >> 8);
    mb.rx_len = 14;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);

    /* FC 0x15 out of range */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x0B;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0x27;
    mb.buf[7] = 0x0E;
    mb.buf[8] = 0;
    mb.buf[9] = 10;
    mb.buf[10] = 0;
    mb.buf[11] = 0;
    mb.buf[12] = 0;
    mb.buf[13] = 0;
    crc = syn_crc16_modbus(mb.buf, 14);
    mb.buf[14] = (uint8_t)(crc & 0xFF);
    mb.buf[15] = (uint8_t)(crc >> 8);
    mb.rx_len = 16;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* FC 0x15 callback returns false */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 0x0B;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 2;
    mb.buf[10] = 0x12;
    mb.buf[11] = 0x34;
    mb.buf[12] = 0x56;
    mb.buf[13] = 0x78;
    crc = syn_crc16_modbus(mb.buf, 14);
    mb.buf[14] = (uint8_t)(crc & 0xFF);
    mb.buf[15] = (uint8_t)(crc >> 8);
    mb.rx_len = 16;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);
}

static void test_modbus_new_function_codes(void)
{
    static uint8_t coils[2] = {0x55, 0xAA}; /* 16 bits */
    static uint8_t discrete[2] = {0xF0, 0x0F};
    static uint16_t holding[4] = {0x1234, 0x5678, 0x9ABC, 0xDEF0};
    static uint16_t fifo[3] = {111, 222, 333};
    static uint8_t mb_buf[256];

    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .coils = coils,
        .coils_count = 16,
        .discrete_inputs = discrete,
        .discrete_count = 16,
        .holding_regs = holding,
        .holding_count = 4,
        .fifo_queue = fifo,
        .fifo_count = 3,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. FC 0x01 Read Coils */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_COILS;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 8;
    uint16_t crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)crc;
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x01, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x01, mock_uart_tx_buf[2]); /* 1 byte */
    TEST_ASSERT_EQUAL_HEX8(0x55, mock_uart_tx_buf[3]);

    /* 2. FC 0x02 Read Discrete Inputs */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_DISCRETE_INPUTS;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 8;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)crc;
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0xF0, mock_uart_tx_buf[3]);

    /* 3. FC 0x05 Write Single Coil ON / OFF / Invalid */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_SINGLE_COIL;
    mb.buf[2] = 0;
    mb.buf[3] = 1;
    mb.buf[4] = 0xFF;
    mb.buf[5] = 0x00;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)crc;
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x57, coils[0]); /* bit 1 turned ON (0x55 | 0x02 = 0x57) */

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_SINGLE_COIL;
    mb.buf[2] = 0;
    mb.buf[3] = 1;
    mb.buf[4] = 0x00;
    mb.buf[5] = 0x00;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)crc;
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x55, coils[0]); /* bit 1 turned OFF */

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_SINGLE_COIL;
    mb.buf[2] = 0;
    mb.buf[3] = 1;
    mb.buf[4] = 0x12;
    mb.buf[5] = 0x34; /* invalid val */
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)crc;
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x85, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 4. FC 0x0F Write Multiple Coils */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_MULTIPLE_COILS;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 8;
    mb.buf[6] = 1;
    mb.buf[7] = 0xFF;
    crc = syn_crc16_modbus(mb.buf, 8);
    mb.buf[8] = (uint8_t)crc;
    mb.buf[9] = (uint8_t)(crc >> 8);
    mb.rx_len = 10;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0xFF, coils[0]);

    /* 5. FC 0x08 Diagnostics Loopback */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_DIAGNOSTICS;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0x12;
    mb.buf[5] = 0x34;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)crc;
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x08, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x12, mock_uart_tx_buf[4]);

    /* 6. FC 0x0B Get Comm Event Counter */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_GET_COMM_EVENT_CNT;
    crc = syn_crc16_modbus(mb.buf, 2);
    mb.buf[2] = (uint8_t)crc;
    mb.buf[3] = (uint8_t)(crc >> 8);
    mb.rx_len = 4;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x0B, mock_uart_tx_buf[1]);

    /* 7. FC 0x0C Get Comm Event Log */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_GET_COMM_EVENT_LOG;
    crc = syn_crc16_modbus(mb.buf, 2);
    mb.buf[2] = (uint8_t)crc;
    mb.buf[3] = (uint8_t)(crc >> 8);
    mb.rx_len = 4;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x0C, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x08, mock_uart_tx_buf[2]);

    /* 8. FC 0x16 Mask Write Register */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_MASK_WRITE_REGISTER;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0xFF;
    mb.buf[5] = 0x00; /* AND mask: 0xFF00 */
    mb.buf[6] = 0x00;
    mb.buf[7] = 0xAB; /* OR mask: 0x00AB */
    crc = syn_crc16_modbus(mb.buf, 8);
    mb.buf[8] = (uint8_t)crc;
    mb.buf[9] = (uint8_t)(crc >> 8);
    mb.rx_len = 10;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX16(0x12AB, holding[0]);

    /* 9. FC 0x18 Read FIFO Queue */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FIFO_QUEUE;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    crc = syn_crc16_modbus(mb.buf, 4);
    mb.buf[4] = (uint8_t)crc;
    mb.buf[5] = (uint8_t)(crc >> 8);
    mb.rx_len = 6;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x18, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, mock_uart_tx_buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x08, mock_uart_tx_buf[3]); /* 8 bytes payload (2 + 3*2) */
    TEST_ASSERT_EQUAL_HEX8(0x00, mock_uart_tx_buf[4]);
    TEST_ASSERT_EQUAL_HEX8(0x03, mock_uart_tx_buf[5]); /* 3 items */

    /* 10. Error paths for new function codes */
    mock_port_reset();
    mb.cfg.coils = NULL;
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_COILS;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 8;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)crc;
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x81, mock_uart_tx_buf[1]);

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_SINGLE_COIL;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 0;
    crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)crc;
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x85, mock_uart_tx_buf[1]);

    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_MULTIPLE_COILS;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 8;
    mb.buf[6] = 1;
    mb.buf[7] = 0xFF;
    crc = syn_crc16_modbus(mb.buf, 8);
    mb.buf[8] = (uint8_t)crc;
    mb.buf[9] = (uint8_t)(crc >> 8);
    mb.rx_len = 10;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x8F, mock_uart_tx_buf[1]);

    mock_port_reset();
    mb.cfg.holding_regs = NULL;
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_MASK_WRITE_REGISTER;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 0;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    crc = syn_crc16_modbus(mb.buf, 8);
    mb.buf[8] = (uint8_t)crc;
    mb.buf[9] = (uint8_t)(crc >> 8);
    mb.rx_len = 10;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x96, mock_uart_tx_buf[1]);

    mock_port_reset();
    mb.cfg.fifo_queue = NULL;
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FIFO_QUEUE;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    crc = syn_crc16_modbus(mb.buf, 4);
    mb.buf[4] = (uint8_t)crc;
    mb.buf[5] = (uint8_t)(crc >> 8);
    mb.rx_len = 6;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x98, mock_uart_tx_buf[1]);

    /* FC 0x11 Report Server ID */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_REPORT_SERVER_ID;
    crc = syn_crc16_modbus(mb.buf, 2);
    mb.buf[2] = (uint8_t)crc;
    mb.buf[3] = (uint8_t)(crc >> 8);
    mb.rx_len = 4;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x11, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(7, mock_uart_tx_buf[2]); /* 6 bytes "SYN-MB" + 1 byte status */
    TEST_ASSERT_EQUAL_MEMORY("SYN-MB", &mock_uart_tx_buf[3], 6);
    TEST_ASSERT_EQUAL_HEX8(0xFF, mock_uart_tx_buf[9]); /* Run status ON */
}

static bool dummy_read_file_cb(SYN_Modbus *m, uint16_t f, uint16_t r, uint16_t l, uint16_t *d,
                               void *c)
{
    (void)m;
    (void)f;
    (void)r;
    (void)l;
    (void)d;
    (void)c;
    return true;
}

static bool dummy_write_file_cb(SYN_Modbus *m, uint16_t f, uint16_t r, uint16_t l,
                                const uint16_t *d, void *c)
{
    (void)m;
    (void)f;
    (void)r;
    (void)l;
    (void)d;
    (void)c;
    return true;
}

static void test_modbus_exception_code_responses(void)
{
    static uint8_t mb_buf[256];
    mock_port_reset();

    static SYN_Modbus_DeviceInfo dev_info = {.vendor_name = "V",
                                             .product_code = "P",
                                             .revision = "R",
                                             .vendor_url = "URL",
                                             .product_name = "PN",
                                             .model_name = "MN",
                                             .user_app_name = "UA"};

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .device_info = &dev_info,
        .on_read_file = dummy_read_file_cb,
        .on_write_file = dummy_write_file_cb,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. FC 0x2B read_code = 0x02 (Regular stream access, covers objects 0x00..0x06) */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
    mb.buf[2] = 0x0E;
    mb.buf[3] = 0x02;
    mb.buf[4] = 0x00;
    uint16_t crc = syn_crc16_modbus(mb.buf, 5);
    mb.buf[5] = (uint8_t)crc;
    mb.buf[6] = (uint8_t)(crc >> 8);
    mb.rx_len = 7;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x2B, mock_uart_tx_buf[1]);

    /* 2. FC 0x2B read_code = 0x02 with device_info = NULL (triggers else continue for objects 3..6)
     */
    mock_port_reset();
    cfg.device_info = NULL;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_DEVICE_INFO;
    mb.buf[2] = 0x0E;
    mb.buf[3] = 0x02;
    mb.buf[4] = 0x00;
    crc = syn_crc16_modbus(mb.buf, 5);
    mb.buf[5] = (uint8_t)crc;
    mb.buf[6] = (uint8_t)(crc >> 8);
    mb.rx_len = 7;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x2B, mock_uart_tx_buf[1]);

    /* 3. FC 0x14 read_file_record response buffer overflow limit */
    mock_port_reset();
    cfg.on_read_file = dummy_read_file_cb;
    /* Initialize with tiny buffer size 30 */
    syn_modbus_init(&mb, &cfg, mb_buf, 30);
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    mb.buf[2] = 0x07;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 20; /* rec_len 20 = 40 bytes > 30 */
    crc = syn_crc16_modbus(mb.buf, 10);
    mb.buf[10] = (uint8_t)crc;
    mb.buf[11] = (uint8_t)(crc >> 8);
    mb.rx_len = 12;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x94, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 4. FC 0x15 write_file_record trailing byte mismatch & rec_len > 120 */
    mock_port_reset();
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    /* trailing byte mismatch: data_len = 10, subreq 1 has rec_len = 1 (2 data bytes -> pos 12),
     * next pos+7 (19) > end (13) */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 10;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 1;
    mb.buf[10] = 0;
    mb.buf[11] = 0;
    mb.buf[12] = 0;
    crc = syn_crc16_modbus(mb.buf, 13);
    mb.buf[13] = (uint8_t)crc;
    mb.buf[14] = (uint8_t)(crc >> 8);
    mb.rx_len = 15;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);

    /* rec_len > 120 */
    mock_port_reset();
    /* data_len = 249 (7 + 121*2), rec_len = 121 */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_WRITE_FILE_RECORD;
    mb.buf[2] = 249;
    mb.buf[3] = 0x06;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    mb.buf[6] = 0;
    mb.buf[7] = 0;
    mb.buf[8] = 0;
    mb.buf[9] = 121; /* > 120 */
    crc = syn_crc16_modbus(mb.buf, 252);
    mb.buf[252] = (uint8_t)crc;
    mb.buf[253] = (uint8_t)(crc >> 8);
    mb.rx_len = 254;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x95, mock_uart_tx_buf[1]);

    /* 5. FC 0x11 Report Server ID with custom server_id_len > 240 */
    mock_port_reset();
    static uint8_t long_id[250];
    memset(long_id, 'A', sizeof(long_id));
    cfg.server_id = long_id;
    cfg.server_id_len = 245;
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_REPORT_SERVER_ID;
    crc = syn_crc16_modbus(mb.buf, 2);
    mb.buf[2] = (uint8_t)crc;
    mb.buf[3] = (uint8_t)(crc >> 8);
    mb.rx_len = 4;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x11, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(241, mock_uart_tx_buf[2]); /* 240 max + 1 status */
}

static void test_modbus_tight_loop_streaming(void)
{
    static uint16_t holding[4] = {10, 20, 30, 40};
    static uint8_t mb_buf[64];

    mock_port_reset();
    mock_tick_ms = 1000;

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 4,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    uint8_t req[8] = {1, 0x03, 0, 0, 0, 2, 0, 0};
    uint16_t crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)(crc >> 8);

    /* Simulate tight while(1) loop feeding bytes one by one and processing immediately */
    for (size_t idx = 0; idx < 8; idx++) {
        syn_modbus_feed(&mb, req[idx]);
        /* syn_modbus_process must NOT wipe rx_len while bytes are actively arriving */
        bool processed = syn_modbus_process(&mb);
        TEST_ASSERT_FALSE(processed);
    }

    TEST_ASSERT_EQUAL_INT(8, mb.rx_len);

    /* Advance time past inter-frame silence gap (5ms) */
    mock_tick_advance(6);
    bool processed = syn_modbus_process(&mb);

    TEST_ASSERT_TRUE(processed);
    TEST_ASSERT_EQUAL_INT(1, mb.frames_rx);
    TEST_ASSERT_EQUAL_INT(9, mock_uart_tx_len);
}

static void test_modbus_truncated_frame_protection(void)
{
    static uint16_t holding[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    static uint8_t mb_buf[256];
    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 8,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* Truncated FC 0x17 (Read/Write Multiple) < 13 bytes */
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_READ_WRITE_MULTIPLE;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    mb.buf[4] = 0;
    mb.buf[5] = 1;
    uint16_t crc = syn_crc16_modbus(mb.buf, 6);
    mb.buf[6] = (uint8_t)(crc & 0xFF);
    mb.buf[7] = (uint8_t)(crc >> 8);
    mb.rx_len = 8;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x97, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* Truncated FC 0x16 (Mask Write Register) < 10 bytes */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_MASK_WRITE_REGISTER;
    mb.buf[2] = 0;
    mb.buf[3] = 0;
    crc = syn_crc16_modbus(mb.buf, 4);
    mb.buf[4] = (uint8_t)(crc & 0xFF);
    mb.buf[5] = (uint8_t)(crc >> 8);
    mb.rx_len = 6;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x96, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* Truncated FC 0x08 (Diagnostics) < 6 bytes */
    mock_port_reset();
    mb.buf[0] = 1;
    mb.buf[1] = SYN_MB_FC_DIAGNOSTICS;
    crc = syn_crc16_modbus(mb.buf, 2);
    mb.buf[2] = (uint8_t)(crc & 0xFF);
    mb.buf[3] = (uint8_t)(crc >> 8);
    mb.rx_len = 4;
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x88, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);
}

static void test_modbus_broadcast_read_and_diagnostic_rejection(void)
{
    static uint16_t holding[4] = {0};
    static uint8_t mb_buf[256];

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 4,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* Broadcast address = 0 */
    uint8_t req[8];
    req[0] = 0; /* broadcast */
    req[1] = SYN_MB_FC_READ_EXCEPTION_STATUS;
    uint16_t crc = syn_crc16_modbus(req, 2);
    req[2] = (uint8_t)(crc & 0xFF);
    req[3] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req, 4);
    mb.rx_len = 4;

    /* Broadcast read exception status should be silently ignored (returns false) */
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));

    /* Broadcast FC 0x08 Diagnostics */
    req[1] = SYN_MB_FC_DIAGNOSTICS;
    req[2] = 0;
    req[3] = 0;
    req[4] = 0;
    req[5] = 0;
    crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));
}

static void test_modbus_broadcast_unsupported_fc_rejection(void)
{
    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {.slave_addr = 1};
    uint8_t mb_buf[256];
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* Broadcast (addr 0) rejection for read/get function codes */
    uint8_t fcs[] = {SYN_MB_FC_READ_COILS,         SYN_MB_FC_READ_DISCRETE_INPUTS,
                     SYN_MB_FC_GET_COMM_EVENT_CNT, SYN_MB_FC_GET_COMM_EVENT_LOG,
                     SYN_MB_FC_REPORT_SERVER_ID,   SYN_MB_FC_READ_FILE_RECORD,
                     SYN_MB_FC_READ_FIFO_QUEUE,    SYN_MB_FC_READ_DEVICE_INFO};

    for (size_t i = 0; i < sizeof(fcs); i++) {
        uint8_t req[8] = {0, fcs[i], 0, 0, 0, 0, 0, 0};
        uint16_t crc = syn_crc16_modbus(req, 6);
        req[6] = (uint8_t)(crc & 0xFF);
        req[7] = (uint8_t)(crc >> 8);
        memcpy(mb.buf, req, 8);
        mb.rx_len = 8;
        TEST_ASSERT_FALSE(syn_modbus_process(&mb));
    }
}

static void test_modbus_short_frame_exceptions(void)
{
    static uint8_t coils[2] = {0};
    static uint8_t mb_buf[256];
    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .coils = coils,
        .coils_count = 16,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. Read Coils frame_len < 8 */
    uint8_t req_short_rc[7] = {1, SYN_MB_FC_READ_COILS, 0, 0, 0, 1, 0};
    uint16_t crc = syn_crc16_modbus(req_short_rc, 5);
    req_short_rc[5] = (uint8_t)(crc & 0xFF);
    req_short_rc[6] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req_short_rc, 7);
    mb.rx_len = 7;
    syn_modbus_process(&mb);
    TEST_ASSERT_EQUAL_UINT32(1, mb.frames_tx);

    /* 2. Write Single Coil frame_len < 8 */
    uint8_t req_short_wsc[7] = {1, SYN_MB_FC_WRITE_SINGLE_COIL, 0, 0, 0, 0, 0};
    crc = syn_crc16_modbus(req_short_wsc, 5);
    req_short_wsc[5] = (uint8_t)(crc & 0xFF);
    req_short_wsc[6] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req_short_wsc, 7);
    mb.rx_len = 7;
    syn_modbus_process(&mb);

    /* 3. Write Multiple Coils frame_len < 9 */
    uint8_t req_short_wmc[8] = {1, SYN_MB_FC_WRITE_MULTIPLE_COILS, 0, 0, 0, 1, 1, 0};
    crc = syn_crc16_modbus(req_short_wmc, 6);
    req_short_wmc[6] = (uint8_t)(crc & 0xFF);
    req_short_wmc[7] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req_short_wmc, 8);
    mb.rx_len = 8;
    syn_modbus_process(&mb);
}

static void test_modbus_write_multiple_short_frame_exception(void)
{
    static uint8_t mb_buf[256];
    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {.slave_addr = 1};
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* Short FC 16 frame: len = 8 < 9 */
    uint8_t req[8] = {1, SYN_MB_FC_WRITE_MULTIPLE, 0, 0, 0, 1, 0, 0};
    uint16_t crc = syn_crc16_modbus(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req, 8);
    mb.rx_len = 8;

    mock_port_reset();
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0x90, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);
}

static void test_modbus_fc17_on_write_rejection_and_broadcast_exception(void)
{
    static uint16_t holding[8] = {0};
    static uint8_t mb_buf[256];
    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 8,
        .on_write = test_on_write,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. FC 0x17 Read/Write Multiple with on_write rejecting (write_addr = 5) */
    uint8_t fc17_req[15] = {1, 0x17, 0, 0, 0, 1, 0, 5, 0, 1, 2, 0x12, 0x34, 0, 0};
    uint16_t crc = syn_crc16_modbus(fc17_req, 13);
    fc17_req[13] = (uint8_t)(crc & 0xFF);
    fc17_req[14] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, fc17_req, 15);
    mb.rx_len = 15;

    mock_port_reset();
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0x97, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 2. Broadcast write single exception (slave_addr = 0, illegal addr = 99) */
    uint8_t bc_req[8] = {0, 0x06, 0, 99, 0, 1, 0, 0};
    crc = syn_crc16_modbus(bc_req, 6);
    bc_req[6] = (uint8_t)(crc & 0xFF);
    bc_req[7] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, bc_req, 8);
    mb.rx_len = 8;

    mock_port_reset();
    TEST_ASSERT_FALSE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0, mock_uart_tx_len);
    TEST_ASSERT_TRUE(mb.errors > 0);

    /* 3. FC 0x03 short frame len < 8 */
    uint8_t short_read[6] = {1, 0x03, 0, 0, 0, 0};
    crc = syn_crc16_modbus(short_read, 4);
    short_read[4] = (uint8_t)(crc & 0xFF);
    short_read[5] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, short_read, 6);
    mb.rx_len = 6;

    mock_port_reset();
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0x83, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);
}

static void test_modbus_truncated_payload_exceptions(void)
{
    static uint16_t holding[16] = {0};
    static uint8_t mb_buf[256];
    mock_port_reset();

    SYN_Modbus mb;
    SYN_Modbus_Config cfg = {
        .slave_addr = 1,
        .holding_regs = holding,
        .holding_count = 16,
    };
    syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

    /* 1. FC 0x17 Read/Write Multiple with write_count = 2 (write_bytes = 4), but frame only 13
     * bytes instead of 17 */
    uint8_t req_fc17[13] = {1, 0x17, 0, 0, 0, 1, 0, 0, 0, 2, 4, 0, 0};
    uint16_t crc = syn_crc16_modbus(req_fc17, 11);
    req_fc17[11] = (uint8_t)(crc & 0xFF);
    req_fc17[12] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req_fc17, 13);
    mb.rx_len = 13;

    mock_port_reset();
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0x97, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 1b. FC 0x17 with out of bounds address */
    uint8_t req_fc17_addr[15] = {1, 0x17, 0, 20, 0, 1, 0, 0, 0, 1, 2, 0, 0, 0, 0};
    crc = syn_crc16_modbus(req_fc17_addr, 13);
    req_fc17_addr[13] = (uint8_t)(crc & 0xFF);
    req_fc17_addr[14] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req_fc17_addr, 15);
    mb.rx_len = 15;

    mock_port_reset();
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0x97, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* 2. FC 0x18 Read FIFO Queue with short frame < 6 */
    uint8_t req_fc18[5] = {1, 0x18, 0, 0, 0};
    crc = syn_crc16_modbus(req_fc18, 3);
    req_fc18[3] = (uint8_t)(crc & 0xFF);
    req_fc18[4] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req_fc18, 5);
    mb.rx_len = 5;

    mock_port_reset();
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0x98, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 3. FC 0x2B with wrong MEI type */
    uint8_t req_fc2b[7] = {1, 0x2B, 0x01, 0x01, 0x00, 0, 0};
    crc = syn_crc16_modbus(req_fc2b, 5);
    req_fc2b[5] = (uint8_t)(crc & 0xFF);
    req_fc2b[6] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req_fc2b, 7);
    mb.rx_len = 7;

    mock_port_reset();
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0xAB, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 4. FC 0x14 with short frame < 5 */
    uint8_t req_fc14[4] = {1, 0x14, 0x00, 0x00};
    crc = syn_crc16_modbus(req_fc14, 2);
    req_fc14[2] = (uint8_t)(crc & 0xFF);
    req_fc14[3] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req_fc14, 4);
    mb.rx_len = 4;

    mock_port_reset();
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0x94, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 5. FC 0x15 with short frame < 5 */
    uint8_t req_fc15[4] = {1, 0x15, 0x00, 0x00};
    crc = syn_crc16_modbus(req_fc15, 2);
    req_fc15[2] = (uint8_t)(crc & 0xFF);
    req_fc15[3] = (uint8_t)(crc >> 8);
    memcpy(mb.buf, req_fc15, 4);
    mb.rx_len = 4;

    mock_port_reset();
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL(0x95, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);
}

static void feed_rtu_frame_direct(SYN_Modbus *mb, const uint8_t *pdu, uint16_t len_no_crc)
{
    memcpy(mb->buf, pdu, len_no_crc);
    uint16_t crc = syn_crc16_modbus(mb->buf, len_no_crc);
    mb->buf[len_no_crc] = (uint8_t)(crc & 0xFFU);
    mb->buf[len_no_crc + 1U] = (uint8_t)((crc >> 8U) & 0xFFU);
    mb->rx_len = (uint16_t)(len_no_crc + 2U);
    mb->frame_ready = true;
}

static void test_pro01_pro09_tx_cap_null_regs_file_clamp_and_poll_silence(void)
{
    uint16_t holding[8] = {0x1111U, 0x2222U, 0x3333U, 0x4444U, 0U, 0U, 0U, 0U};
    uint8_t coils[8] = {0xAAU};
    uint16_t fifo[4] = {10U, 20U, 30U, 40U};
    SYN_Modbus mb;

    /* 1. Runt 4-byte buffer: FC 0x07 hits mb_check_tx_cap AND send_exception buf_size < 5U */
    uint8_t buf4[8] = {0xEEU, 0xEEU, 0xEEU, 0xEEU, 0xEEU, 0xEEU, 0xEEU, 0xEEU};
    SYN_Modbus_Config cfg_full = {
        .slave_addr = 1U,
        .holding_regs = holding,
        .holding_count = 8U,
        .coils = coils,
        .coils_count = 64U,
        .fifo_queue = fifo,
        .fifo_count = 4U,
        .on_read_file = mock_read_file,
    };
    mock_port_reset();
    syn_modbus_init(&mb, &cfg_full, buf4, 4U);
    uint8_t fc07[2] = {1U, SYN_MB_FC_READ_EXCEPTION_STATUS};
    feed_rtu_frame_direct(&mb, fc07, 2U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_UINT32(1U, mb.errors);
    TEST_ASSERT_EQUAL_size_t(0U, mock_uart_tx_len);
    TEST_ASSERT_EQUAL_HEX8(0xEEU, buf4[4]);

    /* 2. Small TX capacity checks across FC 0x01, 0x03, 0x0B, 0x0C, 0x11, 0x17, 0x18, 0x2B */
    uint8_t small10[10];
    syn_modbus_init(&mb, &cfg_full, small10, 10U);
    uint8_t fc03_4[6] = {1U, 0x03U, 0U, 0U, 0U, 4U}; /* needs 11B > 10B */
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc03_4, 6U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x83U, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_DEVICE_FAILURE, mock_uart_tx_buf[2]);

    uint8_t fc01_64[6] = {1U, 0x01U, 0U, 0U, 0U, 64U}; /* needs 13B > 10B */
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc01_64, 6U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x81U, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_DEVICE_FAILURE, mock_uart_tx_buf[2]);

    uint8_t fc0c[2] = {1U, 0x0CU}; /* needs 13B > 10B */
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc0c, 2U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x8CU, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_DEVICE_FAILURE, mock_uart_tx_buf[2]);

    uint8_t fc11[2] = {1U, 0x11U}; /* needs 12B > 10B */
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc11, 2U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x91U, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_DEVICE_FAILURE, mock_uart_tx_buf[2]);

    uint8_t fc18[4] = {1U, 0x18U, 0U, 0U}; /* needs 16B > 10B */
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc18, 4U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x98U, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_DEVICE_FAILURE, mock_uart_tx_buf[2]);

    uint8_t small6[6];
    syn_modbus_init(&mb, &cfg_full, small6, 6U);
    uint8_t fc0b[2] = {1U, 0x0BU}; /* needs 8B > 6B */
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc0b, 2U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x8BU, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_DEVICE_FAILURE, mock_uart_tx_buf[2]);

    uint8_t small8[8];
    syn_modbus_init(&mb, &cfg_full, small8, 8U);
    uint8_t fc2b[5] = {1U, 0x2BU, 0x0EU, 0x01U, 0x00U}; /* header needs 10B > 8B */
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc2b, 5U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0xABU, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_DEVICE_FAILURE, mock_uart_tx_buf[2]);

    uint8_t small16[16];
    syn_modbus_init(&mb, &cfg_full, small16, 16U);
    uint8_t fc17_big_rd[13] = {1U, 0x17U, 0U, 0U, 0U, 6U, 0U, 0U, 0U, 1U, 2U, 0xAAU, 0xBBU};
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc17_big_rd, 13U); /* needs 17B > 16B */
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x97U, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_DEVICE_FAILURE, mock_uart_tx_buf[2]);
    TEST_ASSERT_EQUAL_HEX16(0x1111U, holding[0]);

    /* 3. NULL holding_regs / input_regs with non-zero counts -> SYN_MB_EX_ILLEGAL_ADDR */
    uint8_t std_buf[64];
    SYN_Modbus_Config cfg_null_regs = {
        .slave_addr = 1U,
        .holding_regs = NULL,
        .holding_count = 4U,
        .input_regs = NULL,
        .input_count = 4U,
    };
    syn_modbus_init(&mb, &cfg_null_regs, std_buf, sizeof(std_buf));

    uint8_t rd_hold[6] = {1U, 0x03U, 0U, 0U, 0U, 1U};
    mock_port_reset();
    feed_rtu_frame_direct(&mb, rd_hold, 6U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    uint8_t rd_in[6] = {1U, 0x04U, 0U, 0U, 0U, 1U};
    mock_port_reset();
    feed_rtu_frame_direct(&mb, rd_in, 6U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    uint8_t wr_sng[6] = {1U, 0x06U, 0U, 0U, 0x12U, 0x34U};
    mock_port_reset();
    feed_rtu_frame_direct(&mb, wr_sng, 6U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    uint8_t wr_mul[9] = {1U, 0x10U, 0U, 0U, 0U, 1U, 2U, 0x12U, 0x34U};
    mock_port_reset();
    feed_rtu_frame_direct(&mb, wr_mul, 9U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    uint8_t rw_mul[13] = {1U, 0x17U, 0U, 0U, 0U, 1U, 0U, 0U, 0U, 1U, 2U, 0x12U, 0x34U};
    mock_port_reset();
    feed_rtu_frame_direct(&mb, rw_mul, 13U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_ADDR, mock_uart_tx_buf[2]);

    /* MEI individual object > 0x06 (including 0xFF boundary) returns 0 objects without wraparound
     */
    uint8_t fc2b_high_id[5] = {1U, 0x2BU, 0x0EU, 0x04U, 0xFFU};
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc2b_high_id, 5U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0U, mock_uart_tx_buf[7]);

    /* 4. PRO-01: FC 0x14 resp_buf[256] stack overflow & PDU 253U clamp when buf_size > 258 */
    uint8_t huge_buf[512];
    syn_modbus_init(&mb, &cfg_full, huge_buf, sizeof(huge_buf));
    uint8_t fc14_overflow[17] = {
        1U,    0x14U, 14U, 0x06U, 0U, 1U, 0U, 0U, 0U, 100U, /* 202B sub-resp */
        0x06U, 0U,    1U,  0U,    1U, 0U, 30U               /* +62B -> 267B > 253U */
    };
    mock_port_reset();
    feed_rtu_frame_direct(&mb, fc14_overflow, 17U);
    TEST_ASSERT_TRUE(syn_modbus_process(&mb));
    TEST_ASSERT_EQUAL_HEX8(0x94U, mock_uart_tx_buf[1]);
    TEST_ASSERT_EQUAL_HEX8(SYN_MB_EX_ILLEGAL_VALUE, mock_uart_tx_buf[2]);

    /* 5. PRO-09: syn_modbus_poll honors custom cfg.silence_ms */
    SYN_Modbus_Config cfg_poll = {
        .slave_addr = 1U,
        .holding_regs = holding,
        .holding_count = 8U,
        .silence_ms = 25U,
    };
    mock_port_reset();
    syn_modbus_init(&mb, &cfg_poll, std_buf, sizeof(std_buf));
    uint8_t poll_req[8] = {1U, 0x03U, 0U, 0U, 0U, 1U, 0U, 0U};
    uint16_t pcrc = syn_crc16_modbus(poll_req, 6U);
    poll_req[6] = (uint8_t)(pcrc & 0xFFU);
    poll_req[7] = (uint8_t)((pcrc >> 8U) & 0xFFU);
    memcpy(mock_uart_rx_buf, poll_req, 8U);
    mock_uart_rx_len = 8U;
    mock_uart_rx_pos = 0U;
    mock_tick_ms = 100U;
    TEST_ASSERT_FALSE(syn_modbus_poll(&mb));
    mock_tick_advance(10U); /* >= MB_SILENCE_MS(5), but < silence_ms(25) */
    TEST_ASSERT_FALSE(syn_modbus_poll(&mb));
    TEST_ASSERT_EQUAL_UINT16(8U, mb.rx_len);
    mock_tick_advance(20U); /* 30ms >= 25ms */
    TEST_ASSERT_TRUE(syn_modbus_poll(&mb));
    TEST_ASSERT_EQUAL_UINT16(0U, mb.rx_len);
}

void run_modbus_tests(void)
{
    RUN_TEST(test_modbus_basic);
    RUN_TEST(test_modbus_extended);
    RUN_TEST(test_modbus_exceptions);
    RUN_TEST(test_modbus_broadcast);
    RUN_TEST(test_modbus_feed_timeout);
    RUN_TEST(test_modbus_polling_reset);
    RUN_TEST(test_modbus_custom_silence);
    RUN_TEST(test_modbus_read_exception_status);
    RUN_TEST(test_modbus_read_write_multiple);
    RUN_TEST(test_modbus_read_device_identification);
    RUN_TEST(test_modbus_file_records);
    RUN_TEST(test_modbus_ext_error_paths);
    RUN_TEST(test_modbus_new_function_codes);
    RUN_TEST(test_modbus_exception_code_responses);
    RUN_TEST(test_modbus_tight_loop_streaming);
    RUN_TEST(test_modbus_truncated_frame_protection);
    RUN_TEST(test_modbus_broadcast_read_and_diagnostic_rejection);
    RUN_TEST(test_modbus_broadcast_unsupported_fc_rejection);
    RUN_TEST(test_modbus_short_frame_exceptions);
    RUN_TEST(test_modbus_write_multiple_short_frame_exception);
    RUN_TEST(test_modbus_fc17_on_write_rejection_and_broadcast_exception);
    RUN_TEST(test_modbus_rtu_frame_crc_and_runt_buffer);
    RUN_TEST(test_modbus_truncated_payload_exceptions);
    RUN_TEST(test_pro01_pro09_tx_cap_null_regs_file_clamp_and_poll_silence);
}
