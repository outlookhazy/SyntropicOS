/**
 * @file test_spi_driver.c
 * @brief Unity unit tests for hardware-decoupled Master/Slave SPI driver (syn_spi).
 */

#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_spi.h"
#include "syntropic/drivers/syn_spi_dev.h"
#include "unity/unity.h"

#include <string.h>

extern bool mock_spi_init_ok;

static void test_spi_init_and_transfer(void)
{
    SYN_SPI spi;
    SYN_SPI_Config cfg = {.spi_id = 0,
                          .baudrate_hz = 2000000,
                          .mode = SYN_SPI_MODE_0,
                          .role = SYN_SPI_ROLE_MASTER,
                          .use_dma = false};

    /* Invalid parameters */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_init(NULL, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_init(&spi, NULL));

    /* Port init failure */
    mock_spi_init_ok = false;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_spi_init(&spi, &cfg));
    mock_spi_init_ok = true;

    /* Valid initialization */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_init(&spi, &cfg));
    TEST_ASSERT_TRUE(spi.initialized);
    TEST_ASSERT_EQUAL_UINT8(0, spi.cfg.spi_id);
    TEST_ASSERT_EQUAL_UINT32(2000000, spi.cfg.baudrate_hz);

    /* Default baudrate fallback */
    SYN_SPI spi_default;
    SYN_SPI_Config cfg_zero_baud = {.spi_id = 1,
                                    .baudrate_hz = 0,
                                    .mode = SYN_SPI_MODE_3,
                                    .role = SYN_SPI_ROLE_SLAVE,
                                    .use_dma = false};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_init(&spi_default, &cfg_zero_baud));
    TEST_ASSERT_EQUAL_UINT32(1000000, spi_default.cfg.baudrate_hz);
    syn_spi_deinit(&spi_default);

    /* Full duplex transfer */
    uint8_t tx_buf[] = {0x01, 0x02, 0x03};
    uint8_t rx_buf[3] = {0};
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_transfer(NULL, tx_buf, rx_buf, 3));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_transfer(&spi, tx_buf, rx_buf, 0));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_transfer(&spi, tx_buf, rx_buf, 3));

    /* Write & Read convenience wrappers */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_write(&spi, tx_buf, 3));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_read(&spi, rx_buf, 3));

    /* De-init */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_deinit(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_deinit(&spi));
    TEST_ASSERT_FALSE(spi.initialized);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_deinit(&spi)); /* Uninitialized deinit */

    /* Transfer when uninitialized */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_spi_transfer(&spi, tx_buf, rx_buf, 3));
}

static int s_spi_fail_on_call = -1;
static int s_spi_call_idx = 0;

static SYN_Status spi_dev_fail_cb(uint8_t bus, const uint8_t *tx_buf, uint8_t *rx_buf, size_t len)
{
    (void)bus;
    (void)tx_buf;
    if (rx_buf != NULL) {
        (void)memset(rx_buf, 0x5AU, len);
    }
    int cur = s_spi_call_idx++;
    return (cur == s_spi_fail_on_call) ? SYN_ERROR : SYN_OK;
}

static void test_spi_dev_register_helpers(void)
{
    SYN_SPIDev dev;
    syn_spi_dev_init(&dev, 0U, 4U, 0x80U);
    TEST_ASSERT_EQUAL_UINT8(0U, dev.bus);
    TEST_ASSERT_EQUAL_UINT16(4U, dev.cs);
    TEST_ASSERT_EQUAL_HEX8(0x80U, dev.read_bit);

    /* read8: sends (reg | 0x80), 0x00 and returns second RX byte */
    const uint8_t rx_canned[2] = {0x00U, 0x68U};
    mock_spi_set_response(rx_canned, sizeof(rx_canned));
    uint8_t who = syn_spi_dev_read8(&dev, 0x75U);
    TEST_ASSERT_EQUAL_HEX8(0x68U, who);
    TEST_ASSERT_EQUAL_HEX8(0xF5U, mock_spi_tx_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_HIGH, mock_gpio_states[4]);

    /* write8 */
    mock_spi_tx_len = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_dev_write8(&dev, 0x6BU, 0x01U));
    TEST_ASSERT_EQUAL_UINT32(2U, (uint32_t)mock_spi_tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x6BU, mock_spi_tx_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01U, mock_spi_tx_buf[1]);
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_HIGH, mock_gpio_states[4]);

    /* read_burst > 32 bytes (multi-chunk) and error paths */
    uint8_t burst_rx[40] = {0};
    mock_spi_set_transfer_callback(spi_dev_fail_cb);
    s_spi_fail_on_call = -1;
    s_spi_call_idx = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_dev_read_burst(&dev, 0x3BU, burst_rx, sizeof(burst_rx)));
    TEST_ASSERT_EQUAL_HEX8(0x5AU, burst_rx[0]);
    TEST_ASSERT_EQUAL_HEX8(0x5AU, burst_rx[39]);
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_HIGH, mock_gpio_states[4]);

    s_spi_fail_on_call = 0;
    s_spi_call_idx = 0;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_spi_dev_read_burst(&dev, 0x3BU, burst_rx, 8U));
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_HIGH, mock_gpio_states[4]);

    s_spi_fail_on_call = 1;
    s_spi_call_idx = 0;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_spi_dev_read_burst(&dev, 0x3BU, burst_rx, 8U));
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_HIGH, mock_gpio_states[4]);

    /* write_burst: len > 0, len == 0, and reg-phase error */
    const uint8_t burst_tx[4] = {1U, 2U, 3U, 4U};
    s_spi_fail_on_call = -1;
    s_spi_call_idx = 0;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_dev_write_burst(&dev, 0x10U, burst_tx, 4U));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_spi_dev_write_burst(&dev, 0x10U, burst_tx, 0U));
    s_spi_fail_on_call = 0;
    s_spi_call_idx = 0;
    TEST_ASSERT_EQUAL_INT(SYN_ERROR, syn_spi_dev_write_burst(&dev, 0x10U, burst_tx, 4U));
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_HIGH, mock_gpio_states[4]);
    mock_spi_set_transfer_callback(NULL);
}

void run_spi_driver_tests(void)
{
    RUN_TEST(test_spi_init_and_transfer);
    RUN_TEST(test_spi_dev_register_helpers);
}
