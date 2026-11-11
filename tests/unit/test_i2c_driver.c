/**
 * @file test_i2c_driver.c
 * @brief Unity unit tests for hardware-decoupled Master/Slave I2C driver (syn_i2c).
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_i2c.h"
#include "syntropic/drivers/syn_i2c_dev.h"
#include "unity/unity.h"

#include <string.h>

extern bool mock_i2c_init_ok;

static void test_i2c_init_and_transfer(void)
{
    SYN_I2C i2c;
    SYN_I2C_Config cfg = {.i2c_id = 0,
                          .clock_speed_hz = 400000,
                          .role = SYN_I2C_ROLE_MASTER,
                          .own_address = 0x50,
                          .use_dma = false};

    /* Invalid parameters */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_i2c_init(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_i2c_init(&i2c, NULL));

    /* Port init failure */
    mock_i2c_init_ok = false;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_i2c_init(&i2c, &cfg));
    mock_i2c_init_ok = true;

    /* Valid initialization */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_init(&i2c, &cfg));
    TEST_ASSERT_TRUE(i2c.initialized);
    TEST_ASSERT_EQUAL_UINT8(0, i2c.cfg.i2c_id);
    TEST_ASSERT_EQUAL_UINT32(400000, i2c.cfg.clock_speed_hz);

    /* Default clock speed fallback */
    SYN_I2C i2c_default;
    SYN_I2C_Config cfg_zero_speed = {.i2c_id = 1,
                                     .clock_speed_hz = 0,
                                     .role = SYN_I2C_ROLE_SLAVE,
                                     .own_address = 0x42,
                                     .use_dma = false};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_init(&i2c_default, &cfg_zero_speed));
    TEST_ASSERT_EQUAL_UINT32(100000, i2c_default.cfg.clock_speed_hz);
    syn_i2c_deinit(&i2c_default);

    /* Transfer raw */
    uint8_t tx_buf[] = {0x10, 0x20};
    uint8_t rx_buf[2] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_i2c_transfer(NULL, 0x50, tx_buf, 2, rx_buf, 2));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_transfer(&i2c, 0x50, tx_buf, 2, rx_buf, 2));

    /* Read & Write reg */
    uint8_t reg_val = 0;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_i2c_read_reg(&i2c, 0x50, 0x01, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_write_reg(&i2c, 0x50, 0x01, 0xAA));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_read_reg(&i2c, 0x50, 0x01, &reg_val));

    /* De-init */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_i2c_deinit(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_deinit(&i2c));
    TEST_ASSERT_FALSE(i2c.initialized);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_deinit(&i2c)); /* Uninitialized deinit */

    /* Read when uninitialized */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_i2c_transfer(&i2c, 0x50, tx_buf, 2, rx_buf, 2));
}

static uint8_t s_i2c_dev_regs[256];
static uint8_t s_i2c_last_bus;
static uint16_t s_i2c_last_addr;

static SYN_Status i2c_dev_mock_cb(uint8_t i2c_id, uint16_t addr, const uint8_t *tx, size_t tx_len,
                                  uint8_t *rx, size_t rx_len)
{
    s_i2c_last_bus = i2c_id;
    s_i2c_last_addr = addr;
    if (tx != NULL && tx_len > 0U) {
        uint8_t reg = tx[0];
        for (size_t i = 1U; i < tx_len; i++) {
            s_i2c_dev_regs[(uint8_t)(reg + (i - 1U))] = tx[i];
        }
        if (rx != NULL && rx_len > 0U) {
            for (size_t j = 0U; j < rx_len; j++) {
                rx[j] = s_i2c_dev_regs[(uint8_t)(reg + j)];
            }
        }
    }
    return SYN_OK;
}

static void test_i2c_dev_register_helpers(void)
{
    (void)memset(s_i2c_dev_regs, 0, sizeof(s_i2c_dev_regs));
    mock_i2c_set_transfer_callback(i2c_dev_mock_cb);

    SYN_I2CDev dev;
    syn_i2c_dev_init(&dev, 1U, 0x76U);
    TEST_ASSERT_EQUAL_UINT8(1U, dev.bus);
    TEST_ASSERT_EQUAL_HEX8(0x76U, dev.addr);

    /* write8 / read8 / modify8 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_dev_write8(&dev, 0xF4U, 0x27U));
    TEST_ASSERT_EQUAL_UINT8(1U, s_i2c_last_bus);
    TEST_ASSERT_EQUAL_HEX16(0x76U, s_i2c_last_addr);
    TEST_ASSERT_EQUAL_HEX8(0x27U, syn_i2c_dev_read8(&dev, 0xF4U));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_dev_modify8(&dev, 0xF4U, 0x0FU, 0x0AU));
    TEST_ASSERT_EQUAL_HEX8(0x2AU, syn_i2c_dev_read8(&dev, 0xF4U));

    /* write16_be / read16_be / read16_le (with MSB >= 0x80 for 16-bit shift safety) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_dev_write16_be(&dev, 0x10U, 0x89ABU));
    TEST_ASSERT_EQUAL_HEX16(0x89ABU, syn_i2c_dev_read16_be(&dev, 0x10U));
    TEST_ASSERT_EQUAL_HEX16(0xAB89U, syn_i2c_dev_read16_le(&dev, 0x10U));

    /* write_burst / read_burst and len > 32 guard */
    const uint8_t burst_in[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t burst_out[4] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_dev_write_burst(&dev, 0x20U, burst_in, 4U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_i2c_dev_read_burst(&dev, 0x20U, burst_out, 4U));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(burst_in, burst_out, 4U);

    uint8_t oversized[33] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_i2c_dev_write_burst(&dev, 0x20U, oversized, 33U));
}

void run_i2c_driver_tests(void)
{
    RUN_TEST(test_i2c_init_and_transfer);
    RUN_TEST(test_i2c_dev_register_helpers);
}
