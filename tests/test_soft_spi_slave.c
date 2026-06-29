/**
 * @file test_soft_spi_slave.c
 * @brief Virtual SPI slave + comprehensive soft SPI protocol tests.
 *
 * Uses a GPIO write callback to simulate a SPI slave shift register that
 * reacts to clock edges in real time during a soft SPI transfer. This
 * validates bit-level protocol correctness: clock polarity, phase, bit
 * ordering, and full-duplex data exchange across all 4 SPI modes.
 */

#include "unity/unity.h"
#include "syntropic/drivers/syn_soft_spi.h"
#include "mocks/mock_port.h"

#include <string.h>

/* ── Pin assignments ────────────────────────────────────────────────────── */

#define PIN_SCK   10
#define PIN_MOSI  11
#define PIN_MISO  12
#define PIN_CS    13

/* ── Virtual SPI slave ──────────────────────────────────────────────────── */

typedef struct {
    /* Configuration */
    uint16_t sck_pin;
    uint16_t mosi_pin;
    uint16_t miso_pin;
    bool     cpha;       /* false = sample on first edge, true = second edge */
    bool     cpol;       /* false = idle low, true = idle high */

    /* Shift register state */
    uint8_t  tx_byte;    /* byte slave is shifting out */
    uint8_t  rx_byte;    /* byte slave is shifting in  */
    uint8_t  tx_mask;    /* current bit position for output */
    uint8_t  rx_mask;    /* current bit position for input  */
    uint8_t  bit_count;  /* bits shifted so far */

    /* Results */
    uint8_t  received;   /* last fully received byte */
    uint8_t  rx_count;   /* number of complete bytes received */
} VirtualSPISlave;

static VirtualSPISlave s_slave;

/**
 * GPIO write callback — fired on every pin state change.
 * The slave reacts to clock edges by shifting data.
 */
static void spi_slave_gpio_callback(uint16_t pin, uint8_t state, void *ctx)
{
    VirtualSPISlave *slave = (VirtualSPISlave *)ctx;
    if (pin != slave->sck_pin) return;

    bool clk_went_active;
    if (slave->cpol) {
        /* CPOL=1: active edge = falling */
        clk_went_active = (state == 0);
    } else {
        /* CPOL=0: active edge = rising */
        clk_went_active = (state == 1);
    }

    if (!slave->cpha) {
        /* CPHA=0: Sample on first (active) edge, shift on second (idle) edge */
        if (clk_went_active) {
            /* Sample MOSI */
            if (mock_gpio_states[slave->mosi_pin]) {
                slave->rx_byte |= slave->rx_mask;
            }
            slave->rx_mask >>= 1;
        } else {
            /* Shift: update MISO for next bit */
            slave->tx_mask >>= 1;
            mock_gpio_read_overrides[slave->miso_pin] =
                (slave->tx_byte & slave->tx_mask) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;

            slave->bit_count++;
            if (slave->bit_count >= 8) {
                slave->received = slave->rx_byte;
                slave->rx_count++;
                slave->bit_count = 0;
                slave->rx_byte = 0;
                slave->rx_mask = 0x80;
                /* In a real slave you'd load the next TX byte here */
            }
        }
    } else {
        /* CPHA=1: Shift on first (active) edge, sample on second (idle) edge */
        if (clk_went_active) {
            /* Shift: update MISO */
            mock_gpio_read_overrides[slave->miso_pin] =
                (slave->tx_byte & slave->tx_mask) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
            slave->tx_mask >>= 1;
        } else {
            /* Sample MOSI */
            if (mock_gpio_states[slave->mosi_pin]) {
                slave->rx_byte |= slave->rx_mask;
            }
            slave->rx_mask >>= 1;

            slave->bit_count++;
            if (slave->bit_count >= 8) {
                slave->received = slave->rx_byte;
                slave->rx_count++;
                slave->bit_count = 0;
                slave->rx_byte = 0;
                slave->rx_mask = 0x80;
            }
        }
    }
}

static void slave_init(VirtualSPISlave *slave, SYN_SPIMode mode, uint8_t tx_byte)
{
    memset(slave, 0, sizeof(*slave));
    slave->sck_pin  = PIN_SCK;
    slave->mosi_pin = PIN_MOSI;
    slave->miso_pin = PIN_MISO;
    slave->cpol     = (mode == SYN_SPI_MODE_2 || mode == SYN_SPI_MODE_3);
    slave->cpha     = (mode == SYN_SPI_MODE_1 || mode == SYN_SPI_MODE_3);
    slave->tx_byte  = tx_byte;
    slave->tx_mask  = 0x80;
    slave->rx_mask  = 0x80;
    slave->rx_byte  = 0;

    /* Set initial MISO state for CPHA=0 (data valid before first clock edge) */
    if (!slave->cpha) {
        mock_gpio_read_overrides[PIN_MISO] =
            (tx_byte & 0x80) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
    }

    mock_gpio_set_write_callback(spi_slave_gpio_callback, slave);
}

static void slave_teardown(void)
{
    mock_gpio_set_write_callback(NULL, NULL);
    mock_gpio_read_overrides[PIN_MISO] = -1;
}

/* ── Test helpers ───────────────────────────────────────────────────────── */

static SYN_SoftSPI spi;

static void do_full_duplex_test(SYN_SPIMode mode, uint8_t master_tx,
                                uint8_t slave_tx)
{
    mock_port_reset();
    syn_soft_spi_init(&spi, PIN_SCK, PIN_MOSI, PIN_MISO, mode, 0);
    slave_init(&s_slave, mode, slave_tx);

    uint8_t master_rx = syn_soft_spi_transfer(&spi, master_tx);

    /* Master should have received the slave's TX byte */
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(slave_tx, master_rx,
        "Master did not receive correct data from slave");

    /* Slave should have received the master's TX byte */
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(master_tx, s_slave.received,
        "Slave did not receive correct data from master");

    TEST_ASSERT_EQUAL_MESSAGE(1, s_slave.rx_count,
        "Slave should have received exactly 1 byte");

    slave_teardown();
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

void test_soft_spi_slave_mode0_basic(void)
{
    do_full_duplex_test(SYN_SPI_MODE_0, 0xA5, 0x5A);
}

void test_soft_spi_slave_mode1_basic(void)
{
    do_full_duplex_test(SYN_SPI_MODE_1, 0xA5, 0x5A);
}

void test_soft_spi_slave_mode2_basic(void)
{
    do_full_duplex_test(SYN_SPI_MODE_2, 0xA5, 0x5A);
}

void test_soft_spi_slave_mode3_basic(void)
{
    do_full_duplex_test(SYN_SPI_MODE_3, 0xA5, 0x5A);
}

void test_soft_spi_slave_all_ones(void)
{
    do_full_duplex_test(SYN_SPI_MODE_0, 0xFF, 0xFF);
}

void test_soft_spi_slave_all_zeros(void)
{
    do_full_duplex_test(SYN_SPI_MODE_0, 0x00, 0x00);
}

void test_soft_spi_slave_alternating(void)
{
    do_full_duplex_test(SYN_SPI_MODE_0, 0xAA, 0x55);
}

void test_soft_spi_slave_bulk(void)
{
    mock_port_reset();
    syn_soft_spi_init(&spi, PIN_SCK, PIN_MOSI, PIN_MISO, SYN_SPI_MODE_0, 0);
    slave_init(&s_slave, SYN_SPI_MODE_0, 0x42);

    uint8_t tx[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rx[4] = {0};

    /* Transfer byte-by-byte so the slave can verify each */
    for (int i = 0; i < 4; i++) {
        s_slave.tx_byte  = (uint8_t)(0x42 + i);
        s_slave.tx_mask  = 0x80;
        /* Pre-set MISO for CPHA=0 */
        mock_gpio_read_overrides[PIN_MISO] =
            (s_slave.tx_byte & 0x80) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;

        rx[i] = syn_soft_spi_transfer(&spi, tx[i]);
        TEST_ASSERT_EQUAL_HEX8(0x42 + i, rx[i]);
    }

    /* Last byte the slave received should be 0x04 */
    TEST_ASSERT_EQUAL_HEX8(0x04, s_slave.received);
    TEST_ASSERT_EQUAL(4, s_slave.rx_count);

    slave_teardown();
}

void test_soft_spi_slave_cs_management(void)
{
    mock_port_reset();
    syn_soft_spi_init(&spi, PIN_SCK, PIN_MOSI, PIN_MISO, SYN_SPI_MODE_0, 0);
    syn_soft_spi_set_cs(&spi, PIN_CS, true /* active low */);

    /* CS should start deasserted (high for active-low) */
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[PIN_CS]);

    syn_soft_spi_select(&spi);
    TEST_ASSERT_EQUAL(SYN_GPIO_LOW, mock_gpio_states[PIN_CS]);

    syn_soft_spi_deselect(&spi);
    TEST_ASSERT_EQUAL(SYN_GPIO_HIGH, mock_gpio_states[PIN_CS]);
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

void run_soft_spi_slave_tests(void)
{
    RUN_TEST(test_soft_spi_slave_mode0_basic);
    RUN_TEST(test_soft_spi_slave_mode1_basic);
    RUN_TEST(test_soft_spi_slave_mode2_basic);
    RUN_TEST(test_soft_spi_slave_mode3_basic);
    RUN_TEST(test_soft_spi_slave_all_ones);
    RUN_TEST(test_soft_spi_slave_all_zeros);
    RUN_TEST(test_soft_spi_slave_alternating);
    RUN_TEST(test_soft_spi_slave_bulk);
    RUN_TEST(test_soft_spi_slave_cs_management);
}
