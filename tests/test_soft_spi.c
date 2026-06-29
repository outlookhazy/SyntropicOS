#include "unity/unity.h"
#include "syntropic/drivers/syn_soft_spi.h"
#include "mocks/mock_port.h"

#define PIN_SCK  10
#define PIN_MOSI 11
#define PIN_MISO 12

static SYN_SoftSPI spi;

static void soft_spi_setUp(void) {
    mock_port_reset();
    syn_soft_spi_init(&spi, PIN_SCK, PIN_MOSI, PIN_MISO, SYN_SPI_MODE_0, 1);
}

void test_soft_spi_init(void) {
    soft_spi_setUp();
    TEST_ASSERT_EQUAL(SYN_GPIO_OUTPUT, mock_gpio_modes[PIN_SCK]);
    TEST_ASSERT_EQUAL(SYN_GPIO_OUTPUT, mock_gpio_modes[PIN_MOSI]);
    TEST_ASSERT_EQUAL(SYN_GPIO_INPUT, mock_gpio_modes[PIN_MISO]);
    
    // Mode 0: CPOL = 0, so SCK is LOW
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[PIN_SCK]);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[PIN_MOSI]);
}

void test_soft_spi_transfer_mode0(void) {
    soft_spi_setUp();
    // Mode 0: CPOL=0, CPHA=0
    syn_soft_spi_init(&spi, PIN_SCK, PIN_MOSI, PIN_MISO, SYN_SPI_MODE_0, 1);
    
    // MISO reads all 1s
    mock_gpio_read_overrides[PIN_MISO] = SYN_GPIO_HIGH;
    
    uint8_t rx = syn_soft_spi_transfer(&spi, 0x5A);
    TEST_ASSERT_EQUAL(0xFF, rx);
    
    // MISO reads all 0s
    mock_gpio_read_overrides[PIN_MISO] = SYN_GPIO_LOW;
    rx = syn_soft_spi_transfer(&spi, 0x5A);
    TEST_ASSERT_EQUAL(0x00, rx);
    
    mock_gpio_read_overrides[PIN_MISO] = -1;
}

void test_soft_spi_transfer_mode3(void) {
    soft_spi_setUp();
    // Mode 3: CPOL=1, CPHA=1
    syn_soft_spi_init(&spi, PIN_SCK, PIN_MOSI, PIN_MISO, SYN_SPI_MODE_3, 1);
    
    // Mode 3 initial clock state should be HIGH
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[PIN_SCK]);
    
    mock_gpio_read_overrides[PIN_MISO] = SYN_GPIO_HIGH;
    uint8_t rx = syn_soft_spi_transfer(&spi, 0x5A);
    TEST_ASSERT_EQUAL(0xFF, rx);
    
    mock_gpio_read_overrides[PIN_MISO] = -1;
}

void test_soft_spi_transfer_bulk(void) {
    soft_spi_setUp();
    syn_soft_spi_init(&spi, PIN_SCK, PIN_MOSI, PIN_MISO, SYN_SPI_MODE_0, 1);
    
    uint8_t tx[3] = {0x01, 0x02, 0x03};
    uint8_t rx[3] = {0};
    
    mock_gpio_read_overrides[PIN_MISO] = SYN_GPIO_LOW;
    syn_soft_spi_transfer_bulk(&spi, tx, rx, 3);
    
    TEST_ASSERT_EQUAL(0, rx[0]);
    TEST_ASSERT_EQUAL(0, rx[1]);
    TEST_ASSERT_EQUAL(0, rx[2]);
    
    mock_gpio_read_overrides[PIN_MISO] = -1;
}

void run_soft_spi_tests(void) {
    RUN_TEST(test_soft_spi_init);
    RUN_TEST(test_soft_spi_transfer_mode0);
    RUN_TEST(test_soft_spi_transfer_mode3);
    RUN_TEST(test_soft_spi_transfer_bulk);
}
