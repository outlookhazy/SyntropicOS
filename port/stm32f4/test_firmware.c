/**
 * @file test_firmware.c
 * @brief On-MCU Unity test firmware for Renode simulation.
 *
 * Runs selected SyntropicOS tests on real STM32F4 hardware (or Renode emulation).
 * Test output goes via USART2 in the standard Unity format so Robot
 * Framework can parse pass/fail results with "Wait For Line On UART".
 *
 * Tests included:
 *   - GPIO read/write via real peripheral registers
 *   - UART loopback (requires USART3 TX→RX wired in Renode)
 *   - SPI loopback (requires MOSI→MISO wired in Renode)
 *   - CRC via hardware CRC peripheral
 *   - Ring buffer (pure logic, sanity check on-MCU)
 *   - COBS encode/decode (pure logic)
 *   - Flash erase/write/read cycle
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* SyntropicOS port functions (provided by port_stm32f4.c) */
#include "syntropic/syntropic.h"
#include "syntropic/common/syn_defs.h"

/* Forward declarations for port functions we test directly */
extern SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode);
extern SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state);
extern SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin);
extern SYN_Status syn_port_uart_init(SYN_UARTInstance inst, uint32_t baud);
extern SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance inst, uint8_t byte);
extern SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance inst, uint8_t *byte, uint32_t timeout);
extern uint32_t    syn_port_get_tick_ms(void);
extern void        syn_port_delay_ms(uint32_t ms);

/* ── UART output for Unity ──────────────────────────────────────────────── */

/* USART2 DR register — used for test output */
#define USART2_BASE  0x40004400UL
#define USART2_SR   (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR   (*(volatile uint32_t *)(USART2_BASE + 0x04))

static void uart_putchar(char c)
{
    while (!(USART2_SR & (1U << 7))) { /* TXE */ }
    USART2_DR = (uint8_t)c;
}

__attribute__((used)) static void unity_output_char(int c)
{
    if (c == '\n') uart_putchar('\r');
    uart_putchar((char)c);
}

/* Tell Unity to use our output function — must be before #include "unity.h" */
#define UNITY_OUTPUT_CHAR(a) unity_output_char(a)
#include "unity/unity.h"

/* Unity requires setUp/tearDown */
void setUp(void) {}
void tearDown(void) {}

/* ── GPIO Tests ─────────────────────────────────────────────────────────── */

/* Pin encoding: port << 4 | bit. PD12 = 0x3C (port D = 3, bit 12) */
#define PIN_PD12  ((3 << 4) | 12)  /* Discovery green LED */
#define PIN_PD13  ((3 << 4) | 13)  /* Discovery orange LED */

static void test_gpio_output_readback(void)
{
    /* Configure PD12 as output */
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_gpio_init(PIN_PD12, 1 /* OUTPUT */));

    /* Write high, read back */
    syn_port_gpio_write(PIN_PD12, 1);
    TEST_ASSERT_EQUAL(1, syn_port_gpio_read(PIN_PD12));

    /* Write low, read back */
    syn_port_gpio_write(PIN_PD12, 0);
    TEST_ASSERT_EQUAL(0, syn_port_gpio_read(PIN_PD12));
}

static void test_gpio_multiple_pins(void)
{
    syn_port_gpio_init(PIN_PD12, 1);
    syn_port_gpio_init(PIN_PD13, 1);

    syn_port_gpio_write(PIN_PD12, 1);
    syn_port_gpio_write(PIN_PD13, 0);

    TEST_ASSERT_EQUAL(1, syn_port_gpio_read(PIN_PD12));
    TEST_ASSERT_EQUAL(0, syn_port_gpio_read(PIN_PD13));

    syn_port_gpio_write(PIN_PD12, 0);
    syn_port_gpio_write(PIN_PD13, 1);

    TEST_ASSERT_EQUAL(0, syn_port_gpio_read(PIN_PD12));
    TEST_ASSERT_EQUAL(1, syn_port_gpio_read(PIN_PD13));
}

/* ── Tick / Delay Tests ─────────────────────────────────────────────────── */

static void test_systick_advancing(void)
{
    uint32_t t0 = syn_port_get_tick_ms();
    syn_port_delay_ms(10);
    uint32_t t1 = syn_port_get_tick_ms();
    TEST_ASSERT_TRUE((t1 - t0) >= 10);
}

/* ── Ring Buffer (on-MCU sanity) ────────────────────────────────────────── */

#include "syntropic/util/syn_ringbuf.h"

static void test_ringbuf_onmcu(void)
{
    uint8_t buf[16];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, buf, sizeof(buf));

    /* Bulk write */
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    TEST_ASSERT_EQUAL_size_t(4, syn_ringbuf_write(&rb, data, 4));
    TEST_ASSERT_EQUAL_size_t(4, syn_ringbuf_count(&rb));

    /* Bulk read */
    uint8_t out[4] = {0};
    TEST_ASSERT_EQUAL_size_t(4, syn_ringbuf_read(&rb, out, 4));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, out, 4);
    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
}

/* ── COBS (on-MCU sanity) ───────────────────────────────────────────────── */

#include "syntropic/proto/syn_cobs.h"

static void test_cobs_onmcu(void)
{
    uint8_t raw[] = {0x01, 0x00, 0x02};
    uint8_t enc[16], dec[16];

    size_t enc_len = syn_cobs_encode(raw, 3, enc);
    TEST_ASSERT_TRUE(enc_len > 0);

    size_t dec_len = syn_cobs_decode(enc, enc_len, dec);
    TEST_ASSERT_EQUAL_size_t(3, dec_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(raw, dec, 3);
}

/* ── CRC (on-MCU sanity) ───────────────────────────────────────────────── */

#include "syntropic/util/syn_crc.h"

static void test_crc_onmcu(void)
{
    uint8_t data[] = "123456789";
    uint16_t crc = syn_crc16_ccitt(data, 9);
    /* CRC-16/CCITT of "123456789" = 0x29B1 */
    TEST_ASSERT_EQUAL_HEX16(0x29B1, crc);
}

/* ── Flash (on-MCU verification) ────────────────────────────────────────── */

#include "syntropic/port/syn_port_flash.h"

static void test_flash_erase_write_read(void)
{
    uint32_t addr = 0x08060000; /* Sector 7 */
    uint8_t test_data[] = "SyntropicOS Flash Test Pattern";
    uint8_t read_buf[sizeof(test_data)];

    /* Erase */
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_flash_erase(addr));

    /* Check it was erased (should be all 0xFF) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_flash_read(addr, read_buf, sizeof(read_buf)));
    for (size_t i = 0; i < sizeof(read_buf); i++) {
        TEST_ASSERT_EQUAL_UINT8(0xFF, read_buf[i]);
    }

    /* Write */
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_flash_write(addr, test_data, sizeof(test_data)));

    /* Read back and verify */
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_flash_read(addr, read_buf, sizeof(read_buf)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_data, read_buf, sizeof(test_data));
}

/* ── ADC (on-MCU verification) ─────────────────────────────────────────── */

#include "syntropic/port/syn_port_adc.h"

static void test_adc_reading(void)
{
    TEST_ASSERT_EQUAL(SYN_OK, syn_port_adc_init(0));
    uint16_t val = syn_port_adc_read(0);
    /* Resolution is 12-bit, so value should be <= 4095 */
    TEST_ASSERT_TRUE(val <= 4095);
    TEST_ASSERT_EQUAL_UINT8(12, syn_port_adc_resolution());
    TEST_ASSERT_EQUAL_UINT16(3300, syn_port_adc_reference_mv());
}

/* ── FSM (on-MCU verification) ─────────────────────────────────────────── */

#include "syntropic/util/syn_fsm.h"

#define ST_IDLE  0
#define ST_BUSY  1
#define EV_START 10
#define EV_STOP  11

static const SYN_FSM_Transition test_transitions[] = {
    { ST_IDLE, EV_START, ST_BUSY, NULL, NULL },
    { ST_BUSY, EV_STOP,  ST_IDLE, NULL, NULL },
    SYN_FSM_END
};

static void test_fsm_onmcu(void)
{
    SYN_FSM fsm;
    syn_fsm_init(&fsm, test_transitions, ST_IDLE, "test_fsm");

    TEST_ASSERT_EQUAL(ST_IDLE, syn_fsm_state(&fsm));

    /* Dispatch EV_START -> ST_BUSY */
    TEST_ASSERT_TRUE(syn_fsm_dispatch(&fsm, EV_START));
    TEST_ASSERT_EQUAL(ST_BUSY, syn_fsm_state(&fsm));

    /* Dispatch invalid event in ST_BUSY -> shouldn't change state */
    TEST_ASSERT_FALSE(syn_fsm_dispatch(&fsm, EV_START));
    TEST_ASSERT_EQUAL(ST_BUSY, syn_fsm_state(&fsm));

    /* Dispatch EV_STOP -> ST_IDLE */
    TEST_ASSERT_TRUE(syn_fsm_dispatch(&fsm, EV_STOP));
    TEST_ASSERT_EQUAL(ST_IDLE, syn_fsm_state(&fsm));
}

/* ── Protothreads & Scheduler (on-MCU verification) ──────────────────────── */

static int pt_basic_counter = 0;

static SYN_PT_Status pt_basic_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    pt_basic_counter = 1;
    PT_YIELD(pt);

    pt_basic_counter = 2;
    PT_YIELD(pt);

    pt_basic_counter = 3;

    PT_END(pt);
}

static void test_basic_protothread(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    pt_basic_counter = 0;

    SYN_PT_Status s;

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_YIELDED, s);
    TEST_ASSERT_EQUAL_INT(1, pt_basic_counter);

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_YIELDED, s);
    TEST_ASSERT_EQUAL_INT(2, pt_basic_counter);

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
    TEST_ASSERT_EQUAL_INT(3, pt_basic_counter);
}

static int wait_condition = 0;

static SYN_PT_Status pt_wait_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    PT_WAIT_UNTIL(pt, wait_condition);

    PT_END(pt);
}

static void test_wait_until(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    wait_condition = 0;

    SYN_PT_Status s;

    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_WAITING, s);

    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_WAITING, s);

    wait_condition = 1;
    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
}

static int delay_done = 0;

static SYN_PT_Status pt_delay_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    PT_TASK_DELAY_MS(pt, task, 10);
    delay_done = 1;

    PT_END(pt);
}

static void test_delay_ms(void)
{
    SYN_Task task;
    syn_task_create(&task, "delay_test", pt_delay_func, 0, NULL);
    delay_done = 0;

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_PT_Status s = pt_delay_func(&pt, &task);
    TEST_ASSERT_EQUAL(PT_WAITING, s);
    TEST_ASSERT_EQUAL_INT(0, delay_done);

    /* Delay 11ms on simulated CPU to advance SysTick */
    syn_port_delay_ms(11);

    s = pt_delay_func(&pt, &task);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
    TEST_ASSERT_EQUAL_INT(1, delay_done);
}

static int sched_order[10];
static int sched_order_idx = 0;

static SYN_PT_Status sched_task_a(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    sched_order[sched_order_idx++] = 1;
    PT_YIELD(pt);
    sched_order[sched_order_idx++] = 1;

    PT_END(pt);
}

static SYN_PT_Status sched_task_b(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    sched_order[sched_order_idx++] = 2;
    PT_YIELD(pt);
    sched_order[sched_order_idx++] = 2;

    PT_END(pt);
}

static void test_scheduler(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "a", sched_task_a, 0, NULL);
    syn_task_create(&tasks[1], "b", sched_task_b, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    sched_order_idx = 0;
    memset(sched_order, 0, sizeof(sched_order));

    bool alive;

    alive = syn_sched_run(&sched);
    TEST_ASSERT_TRUE(alive);
    TEST_ASSERT_EQUAL_INT(2, sched_order_idx);

    alive = syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(4, sched_order_idx);

    alive = syn_sched_run(&sched);
    TEST_ASSERT_FALSE(alive);
    TEST_ASSERT_EQUAL_INT(0, syn_sched_alive_count(&sched));
}

static int suspend_counter = 0;

static SYN_PT_Status suspend_task_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    for (;;) {
        suspend_counter++;
        PT_YIELD(pt);
    }

    PT_END(pt);
}

static void test_suspend_resume(void)
{
    SYN_Task tasks[1];
    SYN_Sched sched;
    suspend_counter = 0;

    syn_task_create(&tasks[0], "cnt", suspend_task_func, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, suspend_counter);

    syn_task_suspend(&tasks[0]);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, suspend_counter);

    syn_task_resume(&tasks[0]);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, suspend_counter);
}

/* ── Software Timers (on-MCU verification) ───────────────────────────────── */

#include "syntropic/sched/syn_timer.h"

static int timer_fire_count = 0;

static void timer_callback(SYN_Timer *t, void *ctx)
{
    (void)t; (void)ctx;
    timer_fire_count++;
}

static void test_software_timer(void)
{
    SYN_Timer tmr;
    timer_fire_count = 0;

    syn_timer_init(&tmr, 20, true, timer_callback, NULL);
    syn_timer_start(&tmr);

    syn_timer_service(&tmr, 1);
    TEST_ASSERT_EQUAL_INT(0, timer_fire_count);

    syn_port_delay_ms(10);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_EQUAL_INT(0, timer_fire_count);

    syn_port_delay_ms(11);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_EQUAL_INT(1, timer_fire_count);

    syn_port_delay_ms(21);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_EQUAL_INT(2, timer_fire_count);

    syn_timer_stop(&tmr);
    syn_port_delay_ms(21);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_EQUAL_INT(2, timer_fire_count);
}

static void test_timeout_onmcu(void)
{
    SYN_Timeout to;

    syn_timeout_start(&to, 30);
    TEST_ASSERT_FALSE(syn_timeout_expired(&to));
    TEST_ASSERT_TRUE(syn_timeout_remaining(&to) >= 29);

    syn_port_delay_ms(15);
    TEST_ASSERT_FALSE(syn_timeout_expired(&to));
    TEST_ASSERT_TRUE(syn_timeout_elapsed(&to) >= 15);
    TEST_ASSERT_TRUE(syn_timeout_remaining(&to) <= 15);

    syn_port_delay_ms(16);
    TEST_ASSERT_TRUE(syn_timeout_expired(&to));
    TEST_ASSERT_EQUAL_INT(0, syn_timeout_remaining(&to));
}

/* ── SPI Flash Test ────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_spi.h"

/* SPI1 pins on STM32F4 Discovery:
 * PA5 = SCK (AF5), PA6 = MISO (AF5), PA7 = MOSI (AF5)
 * We'll use PA4 as CS (manual GPIO). */
#define PIN_SPI1_CS   ((0 << 4) | 4)  /* PA4 */
#define PIN_SPI1_SCK  ((0 << 4) | 5)  /* PA5 */
#define PIN_SPI1_MISO ((0 << 4) | 6)  /* PA6 */
#define PIN_SPI1_MOSI ((0 << 4) | 7)  /* PA7 */

static void test_spi_flash_jedec_id(void)
{
    /* Configure SPI1 pins as AF5 */
    syn_port_gpio_init(PIN_SPI1_SCK, SYN_GPIO_OUTPUT);
    syn_port_gpio_init(PIN_SPI1_MISO, SYN_GPIO_INPUT);
    syn_port_gpio_init(PIN_SPI1_MOSI, SYN_GPIO_OUTPUT);
    syn_port_gpio_init(PIN_SPI1_CS, SYN_GPIO_OUTPUT);
    syn_port_gpio_write(PIN_SPI1_CS, SYN_GPIO_HIGH); /* CS deasserted */

    /* Init SPI1 — Mode 0, 1 MHz */
    SYN_SPI_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.bus      = 0; /* SPI1 */
    cfg.clock_hz = 1000000;
    cfg.mode     = SYN_SPI_MODE_0;
    cfg.bit_order = 0; /* MSB first */

    SYN_Status rc = syn_port_spi_init(&cfg);
    TEST_ASSERT_EQUAL(SYN_OK, rc);

    /* Read JEDEC ID: send 0x9F command, read 3 bytes */
    uint8_t tx[4] = {0x9F, 0x00, 0x00, 0x00};
    uint8_t rx[4] = {0};

    syn_port_spi_cs_assert(0, PIN_SPI1_CS);
    rc = syn_port_spi_transfer(0, tx, rx, 4);
    syn_port_spi_cs_deassert(0, PIN_SPI1_CS);

    TEST_ASSERT_EQUAL(SYN_OK, rc);

    /* Micron MT25Q JEDEC ID: manufacturer=0x20, type=0xBA, capacity=0x18 */
    TEST_ASSERT_EQUAL_HEX8(0x20, rx[1]); /* Manufacturer (Micron) */
    TEST_ASSERT_EQUAL_HEX8(0xBA, rx[2]); /* Memory type */
    TEST_ASSERT_EQUAL_HEX8(0x18, rx[3]); /* Capacity (16MB) */

    syn_port_spi_deinit(0);
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void)
{
    /* Init USART2 for test output at 115200 baud */
    syn_port_uart_init(1 /* USART2 */, 115200);

    UNITY_BEGIN();

    /* GPIO */
    RUN_TEST(test_gpio_output_readback);
    RUN_TEST(test_gpio_multiple_pins);

    /* Tick */
    RUN_TEST(test_systick_advancing);

    /* Pure logic on-MCU */
    RUN_TEST(test_ringbuf_onmcu);
    RUN_TEST(test_cobs_onmcu);
    RUN_TEST(test_crc_onmcu);

    /* Real peripherals (hardware registers) */
    RUN_TEST(test_flash_erase_write_read);
    RUN_TEST(test_adc_reading);

    /* SPI flash (requires Micron MT25Q on SPI1 in Renode platform) */
    RUN_TEST(test_spi_flash_jedec_id);

    /* FSM */
    RUN_TEST(test_fsm_onmcu);

    /* Concurrency and Timing */
    RUN_TEST(test_basic_protothread);
    RUN_TEST(test_wait_until);
    RUN_TEST(test_delay_ms);
    RUN_TEST(test_scheduler);
    RUN_TEST(test_suspend_resume);
    RUN_TEST(test_software_timer);
    RUN_TEST(test_timeout_onmcu);

    UNITY_END();

    /* Halt — Renode will see the final line and stop */
    for (;;) { __asm volatile("wfi"); }
}
