/**
 * @file test_soft_onewire.c
 * @brief Unity tests for syn_soft_onewire (software 1-Wire driver).
 *
 * GPIO mock design for 1-Wire:
 *   - syn_port_gpio_read(pin) returns mock_gpio_read_overrides[pin] when >= 0.
 *   - Reset: override = 0 -> device present (low during presence pulse).
 *   - Reset: override = 1 -> no device (line stays high).
 *   - Read bit: override = 1 -> each bit = 1 -> read_byte() = 0xFF.
 *   - Read bit: override = 0 -> each bit = 0 -> read_byte() = 0x00.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/drivers/syn_soft_onewire.h"

#define OW_PIN  ((SYN_GPIO_Pin)5)
#define OW_DLY  1u   /* minimal delay — no real timing in tests */

/* ── test 1: init configures pin ────────────────────────────────────────── */

static void test_onewire_init(void)
{
    SYN_SoftOneWire ow;
    syn_soft_onewire_init(&ow, OW_PIN, OW_DLY);
    TEST_ASSERT_EQUAL_UINT8(OW_PIN, (uint8_t)ow.pin);
    TEST_ASSERT_EQUAL_UINT32(OW_DLY, ow.delay_loops);
    /* Pin configured as OUTPUT, driven high (open-drain idle) */
    TEST_ASSERT_EQUAL_UINT8(SYN_GPIO_OUTPUT, mock_gpio_modes[OW_PIN]);
    TEST_ASSERT_EQUAL_UINT8(1u, mock_gpio_states[OW_PIN]);
}

/* ── test 2: reset returns false when no device ─────────────────────────── */

static void test_onewire_reset_no_device(void)
{
    SYN_SoftOneWire ow;
    syn_soft_onewire_init(&ow, OW_PIN, OW_DLY);
    /* Set override AFTER init (gpio_init only called once -> won't clear it)
     * Line stays high (1) during presence window -> no device */
    mock_gpio_read_overrides[OW_PIN] = 1;
    TEST_ASSERT_FALSE(syn_soft_onewire_reset(&ow));
}

/* ── test 3: reset returns true when device present ─────────────────────── */

static void test_onewire_reset_device_present(void)
{
    SYN_SoftOneWire ow;
    syn_soft_onewire_init(&ow, OW_PIN, OW_DLY);
    /* Device pulls line low (0) during presence window */
    mock_gpio_read_overrides[OW_PIN] = 0;
    TEST_ASSERT_TRUE(syn_soft_onewire_reset(&ow));
}

/* ── test 4: write_byte drives GPIO without crash ───────────────────────── */

static void test_onewire_write_byte(void)
{
    SYN_SoftOneWire ow;
    syn_soft_onewire_init(&ow, OW_PIN, OW_DLY);
    /* Should not assert/crash; GPIO transitions happen silently in mock */
    syn_soft_onewire_write_byte(&ow, 0xCCu);  /* SKIP ROM */
    syn_soft_onewire_write_byte(&ow, 0x44u);  /* CONVERT T */
    TEST_PASS();  /* no crash = pass */
}

/* ── test 5: read_byte returns 0xFF when line held high ─────────────────── */

static void test_onewire_read_byte_ones(void)
{
    SYN_SoftOneWire ow;
    syn_soft_onewire_init(&ow, OW_PIN, OW_DLY);
    mock_gpio_read_overrides[OW_PIN] = 1;  /* all bits = 1 */
    TEST_ASSERT_EQUAL_HEX8(0xFF, syn_soft_onewire_read_byte(&ow));
}

/* ── test 6: read_byte returns 0x00 when line held low ──────────────────── */

static void test_onewire_read_byte_zeros(void)
{
    SYN_SoftOneWire ow;
    syn_soft_onewire_init(&ow, OW_PIN, OW_DLY);
    mock_gpio_read_overrides[OW_PIN] = 0;  /* all bits = 0 */
    TEST_ASSERT_EQUAL_HEX8(0x00, syn_soft_onewire_read_byte(&ow));
}

/* ── test 7: read_rom reads 8 bytes ─────────────────────────────────────── */

static void test_onewire_read_rom(void)
{
    SYN_SoftOneWire ow;
    syn_soft_onewire_init(&ow, OW_PIN, OW_DLY);
    mock_gpio_read_overrides[OW_PIN] = 1;  /* all bits = 1 -> all bytes = 0xFF */
    uint8_t rom[8] = {0};
    syn_soft_onewire_read_rom(&ow, rom);
    uint8_t i;
    for (i = 0; i < 8u; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xFF, rom[i]);
    }
}

/* ── Registration ────────────────────────────────────────────────────────── */

void run_soft_onewire_tests(void)
{
    RUN_TEST(test_onewire_init);
    RUN_TEST(test_onewire_reset_no_device);
    RUN_TEST(test_onewire_reset_device_present);
    RUN_TEST(test_onewire_write_byte);
    RUN_TEST(test_onewire_read_byte_ones);
    RUN_TEST(test_onewire_read_byte_zeros);
    RUN_TEST(test_onewire_read_rom);
}
