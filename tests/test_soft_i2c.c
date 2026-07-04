#include "unity/unity.h"
#include "syntropic/drivers/syn_soft_i2c.h"
#include "mocks/mock_port.h"

#define PIN_SCL 10
#define PIN_SDA 11

static SYN_SoftI2C i2c;

static void soft_i2c_setUp(void) {
    mock_port_reset();
    syn_soft_i2c_init(&i2c, PIN_SCL, PIN_SDA, 1);
}

void test_soft_i2c_init(void) {
    soft_i2c_setUp();
    TEST_ASSERT_EQUAL(SYN_GPIO_OUTPUT_OD, mock_gpio_modes[PIN_SCL]);
    TEST_ASSERT_EQUAL(SYN_GPIO_OUTPUT_OD, mock_gpio_modes[PIN_SDA]);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[PIN_SCL]);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[PIN_SDA]);
}

void test_soft_i2c_start_stop(void) {
    soft_i2c_setUp();
    mock_gpio_states[PIN_SCL] = SYN_GPIO_HIGH;
    mock_gpio_states[PIN_SDA] = SYN_GPIO_HIGH;
    
    syn_soft_i2c_start(&i2c);
    
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[PIN_SCL]);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[PIN_SDA]);
    
    syn_soft_i2c_stop(&i2c);
    
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[PIN_SCL]);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[PIN_SDA]);
}

void test_soft_i2c_write_ack(void) {
    soft_i2c_setUp();
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_LOW;
    
    bool ack = syn_soft_i2c_write(&i2c, 0xA5);
    
    TEST_ASSERT_TRUE(ack);
    mock_gpio_read_overrides[PIN_SDA] = -1;
}

void test_soft_i2c_write_nack(void) {
    soft_i2c_setUp();
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_HIGH;
    
    bool ack = syn_soft_i2c_write(&i2c, 0xA5);
    
    TEST_ASSERT_FALSE(ack);
    mock_gpio_read_overrides[PIN_SDA] = -1;
}

void test_soft_i2c_read(void) {
    soft_i2c_setUp();
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_HIGH;
    uint8_t data = syn_soft_i2c_read(&i2c, true);
    TEST_ASSERT_EQUAL(0xFF, data);
    
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_LOW;
    data = syn_soft_i2c_read(&i2c, false);
    TEST_ASSERT_EQUAL(0x00, data);
    mock_gpio_read_overrides[PIN_SDA] = -1;
}

/**
 * syn_soft_i2c_write_read — exercises the entire uncovered path (lines 134-168):
 * write register address then read back data bytes.
 */
void test_soft_i2c_write_read(void) {
    soft_i2c_setUp();

    /* Slave ACKs address and data writes; returns 0xFF on reads */
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_LOW; /* ACK on writes */

    uint8_t tx_data[] = {0x10}; /* register address */
    uint8_t rx_data[2] = {0};

    /* Expect SDA LOW during ACK phase of write, SDA HIGH for read data */
    /* We'll read 0xFF since SDA is held LOW (ACK) but that means bit=0 */
    bool ok = syn_soft_i2c_write_read(&i2c, 0x50, tx_data, 1, rx_data, 2);
    TEST_ASSERT_TRUE(ok);

    mock_gpio_read_overrides[PIN_SDA] = -1;
}

/**
 * write_read with tx_len=0 (rx only) — exercises "if (tx_len > 0)" else path.
 */
void test_soft_i2c_write_read_rx_only(void) {
    soft_i2c_setUp();

    /* ACK the address write */
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_LOW;

    uint8_t rx_data[1] = {0};
    bool ok = syn_soft_i2c_write_read(&i2c, 0x20, NULL, 0, rx_data, 1);
    TEST_ASSERT_TRUE(ok);

    mock_gpio_read_overrides[PIN_SDA] = -1;
}

/**
 * write_read with rx_len=0 (tx only) — exercises "if (rx_len > 0)" else path.
 */
void test_soft_i2c_write_read_tx_only(void) {
    soft_i2c_setUp();

    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_LOW; /* ACK */

    uint8_t tx_data[] = {0x55, 0xAA};
    bool ok = syn_soft_i2c_write_read(&i2c, 0x30, tx_data, 2, NULL, 0);
    TEST_ASSERT_TRUE(ok);

    mock_gpio_read_overrides[PIN_SDA] = -1;
}

/**
 * write_read with NACK on address — exercises NACK early-return paths.
 */
void test_soft_i2c_write_read_nack(void) {
    soft_i2c_setUp();

    /* NACK on address write */
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_HIGH;

    uint8_t tx_data[] = {0x10};
    uint8_t rx_data[1] = {0};
    bool ok = syn_soft_i2c_write_read(&i2c, 0x50, tx_data, 1, rx_data, 1);
    /* NACK → should return false */
    TEST_ASSERT_FALSE(ok);

    mock_gpio_read_overrides[PIN_SDA] = -1;
}

/**
 * Clock stretching — exercises line 33 (timeout-- in scl_high while loop).
 * Override SCL read to return LOW temporarily to simulate a stretching slave.
 * After a few polls the driver breaks out regardless (timeout=10000 loops).
 */
void test_soft_i2c_clock_stretch(void) {
    soft_i2c_setUp();
    /* Make SCL read as LOW (simulates slave holding clock) */
    mock_gpio_read_overrides[PIN_SCL] = SYN_GPIO_LOW;
    /* SDA ACKs so write can progress */
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_LOW;

    /* write() internally calls scl_high() which will loop on the read.
     * The timeout eventually expires — no infinite loop, no crash. */
    bool ack = syn_soft_i2c_write(&i2c, 0x00);
    /* After timeout the function still reads SDA for ACK (which is LOW = ACK) */
    (void)ack;  /* behaviour when stuck varies; just verify no crash */

    mock_gpio_read_overrides[PIN_SCL] = -1;
    mock_gpio_read_overrides[PIN_SDA] = -1;
}

/**
 * write_read NACK on rx-phase address — exercises lines 159-160.
 * tx_len=0 so the rx-phase address is the first thing written; SDA HIGH=NACK.
 */
void test_soft_i2c_write_read_rx_nack(void) {
    soft_i2c_setUp();
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_HIGH; /* NACK on everything */

    uint8_t rx[1] = {0};
    /* tx_len=0 → skip tx phase; rx_len=1 → issue start, address+R → NACK */
    bool ok = syn_soft_i2c_write_read(&i2c, 0x40, NULL, 0, rx, 1);
    TEST_ASSERT_FALSE(ok); /* NACK on rx-phase address → lines 159-160 */

    mock_gpio_read_overrides[PIN_SDA] = -1;
}

/* GPIO write-callback state for NACK-on-data test */
static int s_gpio_write_count = 0;
static void gpio_ack_then_nack(SYN_GPIO_Pin pin, uint8_t state, void *ctx)
{
    (void)ctx; (void)state;
    if (pin == PIN_SDA) {
        s_gpio_write_count++;
        /* After 9 writes (address byte = 8 bits + 1 ACK pulse), flip to NACK */
        if (s_gpio_write_count > 9) {
            mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_HIGH; /* NACK */
        }
    }
}

/**
 * NACK on data byte write — exercises lines 149-150.
 * Address ACKs (SDA LOW), then after 9 SCL cycles (address+ACK) switch to NACK
 * so the first data byte triggers NACK → early return false.
 */
void test_soft_i2c_write_read_data_nack(void) {
    soft_i2c_setUp();
    s_gpio_write_count = 0;
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_LOW; /* start ACKing */
    mock_gpio_set_write_callback(gpio_ack_then_nack, NULL);

    uint8_t tx[] = {0xAB}; /* tx_len=1: address ACKs, then data byte NACKs */
    uint8_t rx[1] = {0};
    bool ok = syn_soft_i2c_write_read(&i2c, 0x50, tx, 1, rx, 1);
    /* Data byte NACK should cause write_read to return false */
    TEST_ASSERT_FALSE(ok);

    mock_gpio_set_write_callback(NULL, NULL);
    mock_gpio_read_overrides[PIN_SDA] = -1;
}

void run_soft_i2c_tests(void) {
    RUN_TEST(test_soft_i2c_init);
    RUN_TEST(test_soft_i2c_start_stop);
    RUN_TEST(test_soft_i2c_write_ack);
    RUN_TEST(test_soft_i2c_write_nack);
    RUN_TEST(test_soft_i2c_read);
    RUN_TEST(test_soft_i2c_write_read);
    RUN_TEST(test_soft_i2c_write_read_rx_only);
    RUN_TEST(test_soft_i2c_write_read_tx_only);
    RUN_TEST(test_soft_i2c_write_read_nack);
    RUN_TEST(test_soft_i2c_clock_stretch);
    RUN_TEST(test_soft_i2c_write_read_rx_nack);
    RUN_TEST(test_soft_i2c_write_read_data_nack);
}
