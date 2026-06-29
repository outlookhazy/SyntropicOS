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
    // Check initial state
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
    
    // START: SDA goes low while SCL is high, then SCL goes low
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[PIN_SCL]);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[PIN_SDA]);
    
    syn_soft_i2c_stop(&i2c);
    
    // STOP: SDA goes high while SCL is high
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[PIN_SCL]);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[PIN_SDA]);
}

void test_soft_i2c_write_ack(void) {
    soft_i2c_setUp();
    // Slave will pull SDA low during ACK
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_LOW;
    
    bool ack = syn_soft_i2c_write(&i2c, 0xA5); // 1010 0101
    
    TEST_ASSERT_TRUE(ack);
    mock_gpio_read_overrides[PIN_SDA] = -1;
}

void test_soft_i2c_write_nack(void) {
    soft_i2c_setUp();
    // Slave will leave SDA high during ACK
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_HIGH;
    
    bool ack = syn_soft_i2c_write(&i2c, 0xA5);
    
    TEST_ASSERT_FALSE(ack);
    mock_gpio_read_overrides[PIN_SDA] = -1;
}

void test_soft_i2c_read(void) {
    soft_i2c_setUp();
    // Slave returns 0xFF (always high)
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_HIGH;
    uint8_t data = syn_soft_i2c_read(&i2c, true);
    TEST_ASSERT_EQUAL(0xFF, data);
    
    // Slave returns 0x00 (always low)
    mock_gpio_read_overrides[PIN_SDA] = SYN_GPIO_LOW;
    data = syn_soft_i2c_read(&i2c, false);
    TEST_ASSERT_EQUAL(0x00, data);
    mock_gpio_read_overrides[PIN_SDA] = -1;
}

void run_soft_i2c_tests(void) {
    RUN_TEST(test_soft_i2c_init);
    RUN_TEST(test_soft_i2c_start_stop);
    RUN_TEST(test_soft_i2c_write_ack);
    RUN_TEST(test_soft_i2c_write_nack);
    RUN_TEST(test_soft_i2c_read);
}
