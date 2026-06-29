/**
 * @file test_multitask.c
 * @brief Host-side test for the SyntropicOS cooperative multitasking framework.
 *
 * This program provides mock implementations of the port layer and
 * exercises protothreads, the scheduler, software timers, semaphores,
 * and event flags — all running on the host (no MCU required).
 *
 * Compile:
 *   gcc -std=c99 -pedantic -Wall -Wextra -Werror -I. \
 *       tests/test_multitask.c \
 *       syntropic/util/syn_ringbuf.c \
 *       syntropic/drivers/syn_gpio.c \
 *       syntropic/drivers/syn_uart.c \
 *       syntropic/sched/syn_sched.c \
 *       syntropic/sched/syn_timer.c \
 *       -o test_multitask && ./test_multitask
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* ── Mock tick source ───────────────────────────────────────────────────── */

static uint32_t mock_tick_ms = 0;

void mock_tick_advance(uint32_t ms)
{
    mock_tick_ms += ms;
}

/* ── Port layer mock implementations ────────────────────────────────────── */

#include "syntropic/common/syn_defs.h"
#include "syntropic/common/syn_compiler.h"

/* System port */
uint32_t syn_port_get_tick_ms(void)          { return mock_tick_ms; }
void     syn_port_delay_ms(uint32_t ms)      { mock_tick_ms += ms; }
void     syn_port_enter_critical(void)       { /* no-op on host */ }
void     syn_port_exit_critical(void)        { /* no-op on host */ }

/* GPIO port (track pin states in a small array) */
static uint8_t gpio_states[32];
static uint8_t gpio_modes[32];

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    if (pin >= 32) return SYN_INVALID_PARAM;
    gpio_modes[pin] = (uint8_t)mode;
    gpio_states[pin] = 0;
    return SYN_OK;
}

SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin)
{
    (void)pin;
    return SYN_OK;
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    if (pin >= 32) return SYN_INVALID_PARAM;
    gpio_states[pin] = (uint8_t)state;
    return SYN_OK;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    if (pin >= 32) return SYN_GPIO_LOW;
    return (SYN_GPIO_State)gpio_states[pin];
}

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    if (pin >= 32) return SYN_INVALID_PARAM;
    gpio_states[pin] ^= 1;
    return SYN_OK;
}

/* UART port (stub — just discard output) */
SYN_Status syn_port_uart_init(SYN_UARTInstance i, uint32_t b)    { (void)i; (void)b; return SYN_OK; }
SYN_Status syn_port_uart_deinit(SYN_UARTInstance i)              { (void)i; return SYN_OK; }
SYN_Status syn_port_uart_transmit(SYN_UARTInstance i, const uint8_t *d, size_t l, uint32_t t)
    { (void)i; (void)d; (void)l; (void)t; return SYN_OK; }
SYN_Status syn_port_uart_receive(SYN_UARTInstance i, uint8_t *d, size_t l, size_t *r, uint32_t t)
    { (void)i; (void)d; (void)l; (void)r; (void)t; return SYN_OK; }
SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance i, uint8_t b)
    { (void)i; (void)b; return SYN_OK; }
SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance i, uint8_t *b, uint32_t t)
    { (void)i; (void)b; (void)t; return SYN_OK; }

/* Assert handler */
void syn_assert_failed(const char *file, int line)
{
    fprintf(stderr, "ASSERT FAILED: %s:%d\n", file, line);
    exit(1);
}

/* Flash port (RAM-backed mock for parameter store testing) */
#define MOCK_FLASH_SIZE     4096
#define MOCK_FLASH_SECTOR   1024
static uint8_t mock_flash[MOCK_FLASH_SIZE];

SYN_Status syn_port_flash_erase(uint32_t addr)
{
    if (addr + MOCK_FLASH_SECTOR > MOCK_FLASH_SIZE) return SYN_ERROR;
    memset(&mock_flash[addr], 0xFF, MOCK_FLASH_SECTOR);
    return SYN_OK;
}

SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len)
{
    if (addr + len > MOCK_FLASH_SIZE) return SYN_ERROR;
    memcpy(buf, &mock_flash[addr], len);
    return SYN_OK;
}

SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len)
{
    if (addr + len > MOCK_FLASH_SIZE) return SYN_ERROR;
    /* Flash can only clear bits (AND semantics) */
    const uint8_t *src = (const uint8_t *)buf;
    for (size_t i = 0; i < len; i++) {
        mock_flash[addr + i] &= src[i];
    }
    return SYN_OK;
}

uint32_t syn_port_flash_sector_size(uint32_t addr)
{
    (void)addr;
    return MOCK_FLASH_SECTOR;
}

/* SPI port (stubs — not exercised in unit tests) */
#include "syntropic/port/syn_port_spi.h"
SYN_Status syn_port_spi_init(const SYN_SPI_Config *c)     { (void)c; return SYN_OK; }
SYN_Status syn_port_spi_deinit(uint8_t b)                  { (void)b; return SYN_OK; }
SYN_Status syn_port_spi_transfer(uint8_t b, const uint8_t *t, uint8_t *r, size_t l)
    { (void)b; (void)t; (void)r; (void)l; return SYN_OK; }
SYN_Status syn_port_spi_cs_assert(uint8_t b, SYN_GPIO_Pin p)   { (void)b; (void)p; return SYN_OK; }
SYN_Status syn_port_spi_cs_deassert(uint8_t b, SYN_GPIO_Pin p) { (void)b; (void)p; return SYN_OK; }

/* I2C port (stubs) */
#include "syntropic/port/syn_port_i2c.h"
SYN_Status syn_port_i2c_init(const SYN_I2C_Config *c)     { (void)c; return SYN_OK; }
SYN_Status syn_port_i2c_deinit(uint8_t b)                  { (void)b; return SYN_OK; }
SYN_Status syn_port_i2c_write(uint8_t b, uint8_t a, const uint8_t *d, size_t l)
    { (void)b; (void)a; (void)d; (void)l; return SYN_OK; }
SYN_Status syn_port_i2c_read(uint8_t b, uint8_t a, uint8_t *d, size_t l)
    { (void)b; (void)a; (void)d; (void)l; return SYN_OK; }
SYN_Status syn_port_i2c_write_read(uint8_t b, uint8_t a, const uint8_t *t, size_t tl, uint8_t *r, size_t rl)
    { (void)b; (void)a; (void)t; (void)tl; (void)r; (void)rl; return SYN_OK; }

/* ADC port (mock for testing) */
#include "syntropic/port/syn_port_adc.h"
static uint16_t mock_adc_value = 2048;
SYN_Status syn_port_adc_init(uint8_t ch)   { (void)ch; return SYN_OK; }
uint16_t syn_port_adc_read(uint8_t ch)      { (void)ch; return mock_adc_value; }
uint8_t  syn_port_adc_resolution(void)      { return 12; }
uint16_t syn_port_adc_reference_mv(void)    { return 3300; }

/* Sleep port (mock for testing) */
#include "syntropic/system/syn_sleep.h"
static int mock_sleep_count = 0;
void syn_port_sleep(SYN_SleepMode m) { (void)m; mock_sleep_count++; }

/* EXTI port (mock for testing) */
#include "syntropic/port/syn_port_exti.h"
SYN_Status syn_port_exti_configure(SYN_GPIO_Pin p, SYN_EXTI_Edge e)
    { (void)p; (void)e; return SYN_OK; }
void syn_port_exti_enable(SYN_GPIO_Pin p)         { (void)p; }
void syn_port_exti_disable(SYN_GPIO_Pin p)        { (void)p; }
void syn_port_exti_clear_pending(SYN_GPIO_Pin p)  { (void)p; }

/* ── Include everything ─────────────────────────────────────────────────── */

#include "syntropic/syntropic.h"

/* ── Test helpers ───────────────────────────────────────────────────────── */

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(expr, msg)                                      \
    do {                                                             \
        if (expr) {                                                  \
            tests_passed++;                                          \
        } else {                                                     \
            tests_failed++;                                          \
            printf("  FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__);\
        }                                                            \
    } while (0)

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 1: Basic protothread — yield and exit                                */
/* ══════════════════════════════════════════════════════════════════════════ */

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
    printf("Test: Basic protothread\n");

    SYN_PT pt;
    PT_INIT(&pt);
    pt_basic_counter = 0;

    SYN_PT_Status s;

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_YIELDED);
    TEST_ASSERT_TRUE(pt_basic_counter == 1);

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_YIELDED);
    TEST_ASSERT_TRUE(pt_basic_counter == 2);

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_EXITED);
    TEST_ASSERT_TRUE(pt_basic_counter == 3);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 2: PT_WAIT_UNTIL                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

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
    printf("Test: PT_WAIT_UNTIL\n");

    SYN_PT pt;
    PT_INIT(&pt);
    wait_condition = 0;

    SYN_PT_Status s;

    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_WAITING);

    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_WAITING);

    wait_condition = 1;
    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_EXITED);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 3: PT_DELAY_MS                                                       */
/* ══════════════════════════════════════════════════════════════════════════ */

static int delay_done = 0;

static SYN_PT_Status pt_delay_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    PT_TASK_DELAY_MS(pt, task, 100);
    delay_done = 1;

    PT_END(pt);
}

static void test_delay_ms(void)
{
    printf("Test: PT_DELAY_MS\n");

    SYN_Task task;
    syn_task_create(&task, "delay_test", pt_delay_func, 0, NULL);
    delay_done = 0;
    mock_tick_ms = 0;

    SYN_PT_Status s;

    s = task.func(&task.pt, &task);
    TEST_ASSERT_TRUE(s == PT_WAITING);
    TEST_ASSERT_TRUE(delay_done == 0);

    mock_tick_advance(50);
    s = task.func(&task.pt, &task);
    TEST_ASSERT_TRUE(s == PT_WAITING);

    mock_tick_advance(50);
    s = task.func(&task.pt, &task);
    TEST_ASSERT_TRUE(s == PT_EXITED);
    TEST_ASSERT_TRUE(delay_done == 1);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 4: Semaphore                                                         */
/* ══════════════════════════════════════════════════════════════════════════ */

static SYN_PT_Sem test_sem;
static int sem_acquired = 0;

static SYN_PT_Status pt_sem_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    PT_SEM_WAIT(pt, &test_sem);
    sem_acquired = 1;

    PT_END(pt);
}

static void test_semaphore(void)
{
    printf("Test: Semaphore\n");

    SYN_PT pt;
    PT_INIT(&pt);
    PT_SEM_INIT(&test_sem, 0);
    sem_acquired = 0;

    SYN_PT_Status s;

    s = pt_sem_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_WAITING);
    TEST_ASSERT_TRUE(sem_acquired == 0);

    PT_SEM_SIGNAL(&test_sem);
    s = pt_sem_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_EXITED);
    TEST_ASSERT_TRUE(sem_acquired == 1);
    TEST_ASSERT_TRUE(PT_SEM_COUNT(&test_sem) == 0);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 5: Event flags                                                       */
/* ══════════════════════════════════════════════════════════════════════════ */

#define EVT_A  SYN_BIT(0)
#define EVT_B  SYN_BIT(1)

static SYN_EventGroup test_events;
static int events_received = 0;

static SYN_PT_Status pt_event_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    PT_WAIT_EVENT(pt, &test_events, EVT_A | EVT_B);
    events_received = 1;

    PT_END(pt);
}

static void test_event_flags(void)
{
    printf("Test: Event flags\n");

    SYN_PT pt;
    PT_INIT(&pt);
    syn_event_init(&test_events);
    events_received = 0;

    SYN_PT_Status s;

    s = pt_event_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_WAITING);

    syn_event_set(&test_events, EVT_A);
    s = pt_event_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_WAITING);

    syn_event_set(&test_events, EVT_B);
    s = pt_event_func(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_EXITED);
    TEST_ASSERT_TRUE(events_received == 1);
    TEST_ASSERT_TRUE(syn_event_get(&test_events) == 0);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 6: Scheduler — multiple tasks                                        */
/* ══════════════════════════════════════════════════════════════════════════ */

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
    printf("Test: Scheduler\n");

    SYN_Task tasks[2];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "a", sched_task_a, 0, NULL);
    syn_task_create(&tasks[1], "b", sched_task_b, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    sched_order_idx = 0;
    memset(sched_order, 0, sizeof(sched_order));

    bool alive;

    alive = syn_sched_run(&sched);
    TEST_ASSERT_TRUE(alive == true);
    TEST_ASSERT_TRUE(sched_order_idx == 2);

    alive = syn_sched_run(&sched);
    TEST_ASSERT_TRUE(sched_order_idx == 4);

    alive = syn_sched_run(&sched);
    TEST_ASSERT_TRUE(alive == false);
    TEST_ASSERT_TRUE(syn_sched_alive_count(&sched) == 0);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 7: Software timers                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

static int timer_fire_count = 0;

static void timer_callback(SYN_Timer *t, void *ctx)
{
    (void)t; (void)ctx;
    timer_fire_count++;
}

static void test_software_timer(void)
{
    printf("Test: Software timer\n");

    SYN_Timer tmr;
    mock_tick_ms = 0;
    timer_fire_count = 0;

    syn_timer_init(&tmr, 100, true, timer_callback, NULL);
    syn_timer_start(&tmr);

    /* Service at t=50 — should not fire */
    mock_tick_advance(50);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_TRUE(timer_fire_count == 0);

    /* Service at t=100 — should fire */
    mock_tick_advance(50);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_TRUE(timer_fire_count == 1);

    /* Service at t=200 — should fire again (periodic) */
    mock_tick_advance(100);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_TRUE(timer_fire_count == 2);

    /* Stop and verify no more fires */
    syn_timer_stop(&tmr);
    mock_tick_advance(200);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_TRUE(timer_fire_count == 2);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 8: Task suspend / resume / restart                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

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
    printf("Test: Task suspend/resume/restart\n");

    SYN_Task tasks[1];
    SYN_Sched sched;
    suspend_counter = 0;

    syn_task_create(&tasks[0], "cnt", suspend_task_func, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    syn_sched_run(&sched);
    TEST_ASSERT_TRUE(suspend_counter == 1);

    syn_task_suspend(&tasks[0]);
    syn_sched_run(&sched);
    TEST_ASSERT_TRUE(suspend_counter == 1);

    syn_task_resume(&tasks[0]);
    syn_sched_run(&sched);
    TEST_ASSERT_TRUE(suspend_counter == 2);

    syn_task_restart(&tasks[0]);
    syn_sched_run(&sched);
    TEST_ASSERT_TRUE(suspend_counter == 3);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 9: PT_SPAWN (child protothread)                                      */
/* ══════════════════════════════════════════════════════════════════════════ */

static int spawn_child_ran = 0;
static int spawn_parent_done = 0;

static SYN_PT_Status spawn_child(SYN_PT *pt)
{
    PT_BEGIN(pt);
    spawn_child_ran = 1;
    PT_YIELD(pt);
    spawn_child_ran = 2;
    PT_END(pt);
}

static SYN_PT child_pt;

static SYN_PT_Status spawn_parent(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    PT_SPAWN(pt, &child_pt, spawn_child(&child_pt));
    spawn_parent_done = 1;

    PT_END(pt);
}

static void test_spawn(void)
{
    printf("Test: PT_SPAWN\n");

    SYN_PT pt;
    PT_INIT(&pt);
    PT_INIT(&child_pt);
    spawn_child_ran = 0;
    spawn_parent_done = 0;

    SYN_PT_Status s;

    s = spawn_parent(&pt, NULL);
    TEST_ASSERT_TRUE(s == PT_WAITING);
    TEST_ASSERT_TRUE(spawn_child_ran == 1);

    s = spawn_parent(&pt, NULL);
    TEST_ASSERT_TRUE(spawn_child_ran == 2);
    TEST_ASSERT_TRUE(spawn_parent_done == 1);
    TEST_ASSERT_TRUE(s == PT_EXITED);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 10: Ring buffer (existing util)                                      */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_ringbuf(void)
{
    printf("Test: Ring buffer\n");

    uint8_t backing[8];
    SYN_RingBuf rb;
    syn_ringbuf_init(&rb, backing, sizeof(backing));

    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
    TEST_ASSERT_TRUE(syn_ringbuf_count(&rb) == 0);
    TEST_ASSERT_TRUE(syn_ringbuf_free(&rb) == 7);

    /* Fill it up */
    int i;
    for (i = 0; i < 7; i++) {
        TEST_ASSERT(syn_ringbuf_put(&rb, (uint8_t)(0xA0 + i)), "put succeeds");
    }
    TEST_ASSERT_TRUE(syn_ringbuf_full(&rb));
    TEST_ASSERT(!syn_ringbuf_put(&rb, 0xFF), "put fails when full");

    /* Drain */
    uint8_t byte;
    for (i = 0; i < 7; i++) {
        TEST_ASSERT(syn_ringbuf_get(&rb, &byte), "get succeeds");
        TEST_ASSERT_TRUE(byte == (uint8_t)(0xA0 + i));
    }
    TEST_ASSERT_TRUE(syn_ringbuf_empty(&rb));
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 11: Logging system                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

static char log_capture_buf[1024];
static size_t log_capture_pos = 0;

static void log_capture_output(const char *str, size_t len)
{
    size_t space = sizeof(log_capture_buf) - log_capture_pos - 1;
    if (len > space) len = space;
    memcpy(log_capture_buf + log_capture_pos, str, len);
    log_capture_pos += len;
    log_capture_buf[log_capture_pos] = '\0';
}

static void test_logging(void)
{
    printf("Test: Logging system\n");

    mock_tick_ms = 1234;
    log_capture_pos = 0;
    log_capture_buf[0] = '\0';

    syn_log_init(log_capture_output, SYN_LOG_DEBUG);

    syn_log(SYN_LOG_DEBUG, "test", "hello %d", 42);
    TEST_ASSERT_TRUE(log_capture_pos > 0);
    TEST_ASSERT_NOT_NULL(strstr(log_capture_buf, "D/test:"));
    TEST_ASSERT_NOT_NULL(strstr(log_capture_buf, "hello 42"));
    TEST_ASSERT_NOT_NULL(strstr(log_capture_buf, "1234"));

    log_capture_pos = 0;
    syn_log(SYN_LOG_INFO, "net", "connected");
    TEST_ASSERT_NOT_NULL(strstr(log_capture_buf, "I/net:"));

    log_capture_pos = 0;
    SYN_LOG_T("test", "%s", "this should not appear");
    TEST_ASSERT_TRUE(log_capture_pos == 0);

    syn_log_set_level(SYN_LOG_ERROR);
    log_capture_pos = 0;
    syn_log(SYN_LOG_WARN, "test", "warn msg");
    TEST_ASSERT_TRUE(log_capture_pos == 0);

    log_capture_pos = 0;
    syn_log(SYN_LOG_ERROR, "test", "error msg");
    TEST_ASSERT_TRUE(log_capture_pos > 0);

    TEST_ASSERT_TRUE(syn_log_get_level() == SYN_LOG_ERROR);

    syn_log_set_level(SYN_LOG_DEBUG);
    log_capture_pos = 0;
    syn_log_raw("raw text\n");
    TEST_ASSERT_TRUE(strcmp(log_capture_buf) == 0,
                "raw output has no prefix");

    syn_log_init(NULL, SYN_LOG_DEBUG);
    syn_log(SYN_LOG_INFO, "test", "no crash");
    TEST_ASSERT_TRUE(1);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 12: CLI                                                              */
/* ══════════════════════════════════════════════════════════════════════════ */

static char cli_output_buf[1024];
static size_t cli_output_pos = 0;

static void cli_test_putchar(char ch)
{
    if (cli_output_pos < sizeof(cli_output_buf) - 1) {
        cli_output_buf[cli_output_pos++] = ch;
        cli_output_buf[cli_output_pos] = '\0';
    }
}

static int led_handler_called = 0;
static int led_handler_argc = 0;
static char led_handler_arg1[32];

static int cmd_led(int argc, char *argv[])
{
    led_handler_called = 1;
    led_handler_argc = argc;
    if (argc > 1) {
        strncpy(led_handler_arg1, argv[1], sizeof(led_handler_arg1) - 1);
        led_handler_arg1[sizeof(led_handler_arg1) - 1] = '\0';
    }
    return 0;
}

static int cmd_status(int argc, char *argv[])
{
    (void)argc; (void)argv;
    return 0;
}

static const SYN_CLI_Command test_commands[] = {
    { "led",    "led <on|off>  - Control LED",   cmd_led    },
    { "status", "status        - Show status",   cmd_status },
};

static void test_cli(void)
{
    printf("Test: CLI\n");

    SYN_CLI cli;
    syn_cli_init(&cli, test_commands, 2, cli_test_putchar, "> ");

    led_handler_called = 0;
    led_handler_argc = 0;
    led_handler_arg1[0] = '\0';
    syn_cli_process_line(&cli, "led on");
    TEST_ASSERT_TRUE(led_handler_called == 1);
    TEST_ASSERT_TRUE(led_handler_argc == 2);
    TEST_ASSERT_TRUE(strcmp(led_handler_arg1) == 0, "argv[1] == on");

    cli_output_pos = 0;
    syn_cli_process_line(&cli, "bogus");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Unknown command"));

    cli_output_pos = 0;
    syn_cli_process_line(&cli, "help");
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "Available commands"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "led"));
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "status"));

    led_handler_called = 0;
    cli_output_pos = 0;
    syn_cli_set_echo(&cli, true);
    syn_cli_print_prompt(&cli);
    {
        const char *input = "led off\r";
        while (*input) {
            syn_cli_process_char(&cli, *input++);
        }
    }
    TEST_ASSERT_TRUE(led_handler_called == 1);
    TEST_ASSERT_TRUE(strcmp(led_handler_arg1) == 0,
                "argv[1] == off from char input");

    led_handler_called = 0;
    led_handler_arg1[0] = '\0';
    {
        const char *bs_input = "lex\bd off\r";
        while (*bs_input) {
            syn_cli_process_char(&cli, *bs_input++);
        }
    }
    TEST_ASSERT_TRUE(led_handler_called == 1);
    TEST_ASSERT_TRUE(strcmp(led_handler_arg1) == 0,
                "backspace corrected to led off");

    led_handler_called = 0;
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_TRUE(led_handler_called == 0);

    led_handler_called = 0;
    syn_cli_process_char(&cli, 'l');
    syn_cli_process_char(&cli, 'e');
    syn_cli_process_char(&cli, 0x03);
    syn_cli_process_char(&cli, '\r');
    TEST_ASSERT_TRUE(led_handler_called == 0);

    led_handler_called = 0;
    led_handler_arg1[0] = '\0';
    syn_cli_process_line(&cli, "led \"hello world\"");
    TEST_ASSERT_TRUE(led_handler_called == 1);
    TEST_ASSERT_TRUE(strcmp(led_handler_arg1) == 0,
                "quoted arg preserved spaces");

    cli_output_pos = 0;
    syn_cli_printf(&cli, "value=%d\r\n", 99);
    TEST_ASSERT_NOT_NULL(strstr(cli_output_buf, "value=99"));
}


/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 13: Timeout helper                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_timeout(void)
{
    printf("Test: Timeout helper\n");

    mock_tick_ms = 0;
    SYN_Timeout to;

    syn_timeout_start(&to, 100);
    TEST_ASSERT_TRUE(!syn_timeout_expired(&to));
    TEST_ASSERT_TRUE(syn_timeout_remaining(&to) == 100);

    mock_tick_advance(50);
    TEST_ASSERT_TRUE(!syn_timeout_expired(&to));
    TEST_ASSERT_TRUE(syn_timeout_elapsed(&to) == 50);
    TEST_ASSERT_TRUE(syn_timeout_remaining(&to) == 50);

    mock_tick_advance(50);
    TEST_ASSERT_TRUE(syn_timeout_expired(&to));
    TEST_ASSERT_TRUE(syn_timeout_remaining(&to) == 0);

    /* Restart */
    syn_timeout_restart(&to);
    TEST_ASSERT_TRUE(!syn_timeout_expired(&to));

    /* Periodic */
    mock_tick_ms = 0;
    syn_timeout_start(&to, 50);
    TEST_ASSERT_TRUE(!syn_timeout_periodic(&to));

    mock_tick_advance(50);
    TEST_ASSERT_TRUE(syn_timeout_periodic(&to));
    TEST_ASSERT_TRUE(!syn_timeout_periodic(&to));

    mock_tick_advance(50);
    TEST_ASSERT_TRUE(syn_timeout_periodic(&to));
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 14: Button debouncer                                                 */
/* ══════════════════════════════════════════════════════════════════════════ */



static int btn_press_count = 0;
static int btn_release_count = 0;
static int btn_long_count = 0;

static void btn_on_press(SYN_Button *b, void *ctx)
    { (void)b; (void)ctx; btn_press_count++; }
static void btn_on_release(SYN_Button *b, void *ctx)
    { (void)b; (void)ctx; btn_release_count++; }
static void btn_on_long(SYN_Button *b, void *ctx)
    { (void)b; (void)ctx; btn_long_count++; }

static void test_button(void)
{
    printf("Test: Button debouncer\n");

    mock_tick_ms = 0;
    btn_press_count = 0;
    btn_release_count = 0;
    btn_long_count = 0;

    /* Pin 0 — we control it via gpio_states (from mock GPIO port) */
    gpio_states[0] = 0; /* not pressed */

    SYN_Button btn;
    syn_button_init(&btn, 0, SYN_BUTTON_ACTIVE_HIGH, 50);
    syn_button_on_press(&btn, btn_on_press, NULL);
    syn_button_on_release(&btn, btn_on_release, NULL);
    syn_button_on_long_press(&btn, btn_on_long, 500, NULL);

    /* No press — should stay idle */
    syn_button_update(&btn);
    TEST_ASSERT_TRUE(btn_press_count == 0);
    TEST_ASSERT_TRUE(!syn_button_is_pressed(&btn));

    /* Press the button */
    gpio_states[0] = 1;
    syn_button_update(&btn);
    TEST_ASSERT_TRUE(btn_press_count == 0);

    /* Advance past debounce window */
    mock_tick_advance(60);
    syn_button_update(&btn);
    TEST_ASSERT_TRUE(btn_press_count == 1);
    TEST_ASSERT_TRUE(syn_button_is_pressed(&btn));

    /* Hold for long press */
    mock_tick_advance(500);
    syn_button_update(&btn);
    TEST_ASSERT_TRUE(btn_long_count == 1);

    /* Release */
    gpio_states[0] = 0;
    syn_button_update(&btn);
    TEST_ASSERT_TRUE(btn_release_count == 1);
    TEST_ASSERT_TRUE(!syn_button_is_pressed(&btn));

    /* Bounce rejection: press then release before debounce */
    btn_press_count = 0;
    gpio_states[0] = 1;
    mock_tick_advance(10);
    syn_button_update(&btn);
    gpio_states[0] = 0; /* bounce off */
    mock_tick_advance(10);
    syn_button_update(&btn);
    TEST_ASSERT_TRUE(btn_press_count == 0);

    /* Event polling */
    btn.events = 0;
    gpio_states[0] = 1;
    syn_button_update(&btn);
    mock_tick_advance(60);
    syn_button_update(&btn);
    uint8_t evts = syn_button_poll_events(&btn);
    TEST_ASSERT_TRUE(evts & SYN_BUTTON_EVT_PRESS);
    TEST_ASSERT_TRUE(syn_button_poll_events(&btn) == 0);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 15: LED controller                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_led(void)
{
    printf("Test: LED controller\n");

    mock_tick_ms = 0;
    gpio_states[1] = 0;

    SYN_LED led;
    syn_led_init(&led, 1, SYN_LED_ACTIVE_HIGH);
    TEST_ASSERT_TRUE(!syn_led_is_on(&led));

    syn_led_on(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    TEST_ASSERT_TRUE(gpio_states[1] == SYN_GPIO_HIGH);

    syn_led_off(&led);
    TEST_ASSERT_TRUE(!syn_led_is_on(&led));

    syn_led_toggle(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));
    syn_led_toggle(&led);
    TEST_ASSERT_TRUE(!syn_led_is_on(&led));

    /* Blink */
    syn_led_blink(&led, 100, 100);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(100);
    syn_led_update(&led);
    TEST_ASSERT_TRUE(!syn_led_is_on(&led));

    mock_tick_advance(100);
    syn_led_update(&led);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    /* Flash N times */
    syn_led_flash(&led, 50, 50, 2);
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(50);
    syn_led_update(&led);  /* on->off, remain=1 */
    TEST_ASSERT_TRUE(!syn_led_is_on(&led));

    mock_tick_advance(50);
    syn_led_update(&led);  /* off->on for flash 2 */
    TEST_ASSERT_TRUE(syn_led_is_on(&led));

    mock_tick_advance(50);
    syn_led_update(&led);  /* on->off, remain=0, mode=OFF */
    TEST_ASSERT_TRUE(!syn_led_is_on(&led));
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 16: FSM                                                              */
/* ══════════════════════════════════════════════════════════════════════════ */

enum { FSM_ST_IDLE, FSM_ST_RUNNING, FSM_ST_ERROR };
enum { FSM_EV_START, FSM_EV_STOP, FSM_EV_FAULT };

static int fsm_action_called = 0;
static void fsm_on_start(void *ctx) { (void)ctx; fsm_action_called = 1; }
static void fsm_on_stop(void *ctx)  { (void)ctx; fsm_action_called = 2; }

static bool fsm_guard_deny(void *ctx) { (void)ctx; return false; }

static const SYN_FSM_Transition test_fsm_table[] = {
    { FSM_ST_IDLE,    FSM_EV_START, FSM_ST_RUNNING, NULL,           fsm_on_start },
    { FSM_ST_RUNNING, FSM_EV_STOP,  FSM_ST_IDLE,    NULL,           fsm_on_stop  },
    { FSM_ST_RUNNING, FSM_EV_FAULT, FSM_ST_ERROR,   NULL,           NULL         },
    { FSM_ST_IDLE,    FSM_EV_FAULT, FSM_ST_IDLE,    fsm_guard_deny, NULL         },
    SYN_FSM_END
};

static void test_fsm(void)
{
    printf("Test: FSM\n");

    /* Re-init log so FSM can log transitions */
    log_capture_pos = 0;
    syn_log_init(log_capture_output, SYN_LOG_DEBUG);

    SYN_FSM fsm;
    syn_fsm_init(&fsm, test_fsm_table, FSM_ST_IDLE, "fsm");

    TEST_ASSERT_TRUE(syn_fsm_state(&fsm) == FSM_ST_IDLE);
    TEST_ASSERT(syn_fsm_in_state(&fsm, FSM_ST_IDLE), "in_state check");

    /* Dispatch START */
    fsm_action_called = 0;
    bool took = syn_fsm_dispatch(&fsm, FSM_EV_START);
    TEST_ASSERT_TRUE(took);
    TEST_ASSERT_TRUE(syn_fsm_state(&fsm) == FSM_ST_RUNNING);
    TEST_ASSERT_TRUE(fsm_action_called == 1);

    /* Dispatch STOP */
    took = syn_fsm_dispatch(&fsm, FSM_EV_STOP);
    TEST_ASSERT_TRUE(took);
    TEST_ASSERT_TRUE(syn_fsm_state(&fsm) == FSM_ST_IDLE);
    TEST_ASSERT_TRUE(fsm_action_called == 2);

    /* Guard blocks transition */
    took = syn_fsm_dispatch(&fsm, FSM_EV_FAULT);
    TEST_ASSERT_TRUE(!took);
    TEST_ASSERT_TRUE(syn_fsm_state(&fsm) == FSM_ST_IDLE);

    /* No matching transition */
    took = syn_fsm_dispatch(&fsm, FSM_EV_STOP);
    TEST_ASSERT_TRUE(!took);

    /* Force state */
    syn_fsm_set_state(&fsm, FSM_ST_ERROR);
    TEST_ASSERT_TRUE(syn_fsm_state(&fsm) == FSM_ST_ERROR);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 17: CRC                                                              */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_crc(void)
{
    printf("Test: CRC\n");

    const uint8_t test_data[] = "123456789";
    size_t len = 9;

    /* CRC-16 CCITT — standard check value is 0x29B1 */
    uint16_t crc16 = syn_crc16_ccitt(test_data, len);
    TEST_ASSERT_TRUE(crc16 == 0x29B1);

    /* CRC-16 Modbus — standard check value is 0x4B37 */
    uint16_t crcmod = syn_crc16_modbus(test_data, len);
    TEST_ASSERT_TRUE(crcmod == 0x4B37);

    /* CRC-32 — standard check value is 0xCBF43926 */
    uint32_t crc32 = syn_crc32(test_data, len);
    TEST_ASSERT_TRUE(crc32 == 0xCBF43926u);

    /* Incremental CRC-16 */
    uint16_t inc = SYN_CRC16_CCITT_INIT;
    inc = syn_crc16_ccitt_update(inc, test_data, 5);
    inc = syn_crc16_ccitt_update(inc, test_data + 5, 4);
    TEST_ASSERT_TRUE(inc == 0x29B1);

    /* Empty data */
    TEST_ASSERT(syn_crc32(NULL, 0) == 0x00000000u, "CRC-32 of empty data");
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 18: Filters                                                          */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_filters(void)
{
    printf("Test: Filters\n");

    /* Moving average */
    SYN_FilterMA ma;
    syn_filter_ma_init(&ma, 4);

    int16_t v;
    v = syn_filter_ma_update(&ma, 100);
    TEST_ASSERT_TRUE(v == 100);
    v = syn_filter_ma_update(&ma, 200);
    TEST_ASSERT_TRUE(v == 150);
    v = syn_filter_ma_update(&ma, 300);
    TEST_ASSERT_TRUE(v == 200);
    v = syn_filter_ma_update(&ma, 400);
    TEST_ASSERT_TRUE(v == 250);
    /* Window full — oldest drops */
    v = syn_filter_ma_update(&ma, 400);
    TEST_ASSERT_TRUE(v == 325);

    syn_filter_ma_reset(&ma);
    v = syn_filter_ma_update(&ma, 50);
    TEST_ASSERT_TRUE(v == 50);

    /* EMA */
    SYN_FilterEMA ema;
    syn_filter_ema_init(&ema, 128); /* alpha = 0.5 */
    v = syn_filter_ema_update(&ema, 100);
    TEST_ASSERT_TRUE(v == 100);
    v = syn_filter_ema_update(&ema, 200);
    TEST_ASSERT_TRUE(v == 150);

    /* Median */
    SYN_FilterMedian med;
    syn_filter_median_init(&med, 5);
    syn_filter_median_update(&med, 10);
    syn_filter_median_update(&med, 50);
    syn_filter_median_update(&med, 20);
    syn_filter_median_update(&med, 90);
    v = syn_filter_median_update(&med, 30);
    /* sorted: 10,20,30,50,90 -> median = 30 */
    TEST_ASSERT_TRUE(v == 30);

    /* Spike rejection */
    syn_filter_median_init(&med, 3);
    syn_filter_median_update(&med, 100);
    syn_filter_median_update(&med, 100);
    v = syn_filter_median_update(&med, 9999);
    /* sorted: 100,100,9999 -> median = 100 */
    TEST_ASSERT_TRUE(v == 100);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 19: Soft PWM                                                         */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_soft_pwm(void)
{
    printf("Test: Soft PWM\n");

    gpio_states[2] = 0;

    SYN_SoftPWM pwm;
    syn_soft_pwm_init(&pwm, 2, 10); /* resolution = 10 steps */
    syn_soft_pwm_set_duty(&pwm, 3);  /* 30% duty */

    int on_count = 0;
    int i;
    for (i = 0; i < 10; i++) {
        syn_soft_pwm_tick(&pwm);
        if (gpio_states[2] == SYN_GPIO_HIGH) on_count++;
    }
    TEST_ASSERT_TRUE(on_count == 3);

    /* 0% duty */
    syn_soft_pwm_set_duty(&pwm, 0);
    on_count = 0;
    for (i = 0; i < 10; i++) {
        syn_soft_pwm_tick(&pwm);
        if (gpio_states[2] == SYN_GPIO_HIGH) on_count++;
    }
    TEST_ASSERT_TRUE(on_count == 0);

    /* 100% duty */
    syn_soft_pwm_set_duty(&pwm, 10);
    on_count = 0;
    for (i = 0; i < 10; i++) {
        syn_soft_pwm_tick(&pwm);
        if (gpio_states[2] == SYN_GPIO_HIGH) on_count++;
    }
    TEST_ASSERT_TRUE(on_count == 10);

    /* Percent API */
    syn_soft_pwm_set_percent(&pwm, 50);
    TEST_ASSERT_TRUE(syn_soft_pwm_get_duty(&pwm) == 5);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Main                                                                      */
/* ══════════════════════════════════════════════════════════════════════════ */

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 20: PID controller                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_pid(void)
{
    printf("Test: PID controller\n");

    SYN_PID pid;
    SYN_PID_Config cfg = {
        .kp = 100, .ki = 10, .kd = 50,
        .scale = 100,
        .out_min = -1000, .out_max = 1000,
        .integral_max = 0,
        .d_filter_alpha = 0,
    };
    syn_pid_init(&pid, &cfg);

    /* Proportional only (first call, I=0, D=0) */
    int32_t out = syn_pid_update(&pid, 100, 0, 10);
    TEST_ASSERT_TRUE(out == 100);

    /* At setpoint → output drops to near zero */
    syn_pid_reset(&pid);
    out = syn_pid_update(&pid, 50, 50, 10);
    TEST_ASSERT_TRUE(out == 0);

    /* Output clamping */
    syn_pid_reset(&pid);
    out = syn_pid_update(&pid, 5000, 0, 10);
    TEST_ASSERT_TRUE(out == 1000);

    syn_pid_reset(&pid);
    out = syn_pid_update(&pid, -5000, 0, 10);
    TEST_ASSERT_TRUE(out == -1000);

    /* Set gains at runtime */
    syn_pid_set_gains(&pid, 200, 0, 0);
    syn_pid_reset(&pid);
    out = syn_pid_update(&pid, 100, 0, 10);
    TEST_ASSERT_TRUE(out == 200);

    /* PID output getter */
    TEST_ASSERT_TRUE(syn_pid_output(&pid) == out);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 21: Hysteresis                                                       */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_hysteresis(void)
{
    printf("Test: Hysteresis\n");

    SYN_Hysteresis h;
    syn_hyst_init(&h, 1000, 50, false);  /* threshold=1000, band=±50 */

    TEST_ASSERT_TRUE(!syn_hyst_state(&h));

    /* Value below threshold — no change */
    syn_hyst_update(&h, 900);
    TEST_ASSERT_TRUE(!syn_hyst_state(&h));

    /* Value in deadband — no change */
    syn_hyst_update(&h, 1030);
    TEST_ASSERT_TRUE(!syn_hyst_state(&h));

    /* Value crosses high trip (1050) */
    syn_hyst_update(&h, 1051);
    TEST_ASSERT_TRUE(syn_hyst_state(&h));

    /* Value drops but stays above low trip (950) */
    syn_hyst_update(&h, 980);
    TEST_ASSERT_TRUE(syn_hyst_state(&h));

    /* Value drops below low trip */
    syn_hyst_update(&h, 940);
    TEST_ASSERT_TRUE(!syn_hyst_state(&h));
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 22: Lookup table                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_lut(void)
{
    printf("Test: Lookup table\n");

    static const SYN_LUT_Entry table[] = {
        {   0, 3300 },
        {  25, 2048 },
        {  50, 1200 },
        { 100,  400 },
    };
    const size_t count = sizeof(table) / sizeof(table[0]);

    /* Forward: x→y */
    TEST_ASSERT(syn_lut_forward(table, count, 0) == 3300, "forward: x=0 → y=3300");
    TEST_ASSERT(syn_lut_forward(table, count, 100) == 400, "forward: x=100 → y=400");
    TEST_ASSERT(syn_lut_forward(table, count, 25) == 2048, "forward: exact point");

    /* Interpolation */
    int32_t y = syn_lut_forward(table, count, 75);
    /* Between (50,1200) and (100,400): y = 1200 + (400-1200)*(75-50)/(100-50) = 1200 - 400 = 800 */
    TEST_ASSERT_TRUE(y == 800);

    /* Clamping */
    TEST_ASSERT(syn_lut_forward(table, count, -10) == 3300, "forward: clamp below");
    TEST_ASSERT(syn_lut_forward(table, count, 200) == 400, "forward: clamp above");

    /* Reverse: y→x (y is descending in this table) */
    int32_t x = syn_lut_reverse(table, count, 2048);
    TEST_ASSERT_TRUE(x == 25);

    x = syn_lut_reverse(table, count, 800);
    TEST_ASSERT_TRUE(x == 75);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 23: Servo                                                            */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_servo(void)
{
    printf("Test: Servo\n");

    SYN_Servo servo;
    syn_servo_init(&servo, 1000, 2000, 180);

    /* Default is center */
    TEST_ASSERT_TRUE(syn_servo_get_pulse_us(&servo) == 1500);

    /* Set angle */
    syn_servo_set_angle(&servo, 0);
    TEST_ASSERT_TRUE(syn_servo_get_pulse_us(&servo) == 1000);

    syn_servo_set_angle(&servo, 180);
    TEST_ASSERT_TRUE(syn_servo_get_pulse_us(&servo) == 2000);

    syn_servo_set_angle(&servo, 90);
    TEST_ASSERT_TRUE(syn_servo_get_pulse_us(&servo) == 1500);

    /* Get angle */
    TEST_ASSERT_TRUE(syn_servo_get_angle(&servo) == 90);

    /* Set raw pulse */
    syn_servo_set_pulse(&servo, 1250);
    TEST_ASSERT_TRUE(syn_servo_get_pulse_us(&servo) == 1250);

    /* At target (immediate moves) */
    TEST_ASSERT_TRUE(syn_servo_at_target(&servo));

    /* Smooth move */
    mock_tick_ms = 0;
    syn_servo_set_angle(&servo, 0);   /* start at 0° (1000µs) */
    syn_servo_move_to(&servo, 180, 1000);  /* move to 180° over 1s */
    TEST_ASSERT_TRUE(!syn_servo_at_target(&servo));

    mock_tick_advance(500);
    syn_servo_update(&servo);
    uint16_t mid = syn_servo_get_pulse_us(&servo);
    /* Should be roughly halfway: ~1500µs */
    TEST_ASSERT_TRUE(mid >= 1400 && mid <= 1600);

    mock_tick_advance(600);
    syn_servo_update(&servo);
    TEST_ASSERT_TRUE(syn_servo_at_target(&servo));
    TEST_ASSERT_TRUE(syn_servo_get_pulse_us(&servo) == 2000);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 24: DC Motor                                                         */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_dc_motor(void)
{
    printf("Test: DC motor\n");

    SYN_DCMotor motor;
    syn_dc_motor_init(&motor, 3, 4, SYN_DC_MODE_PWM_DIR);

    /* Set speed */
    syn_dc_motor_set_speed(&motor, 75);
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) == 75);
    TEST_ASSERT_TRUE(gpio_states[4] == SYN_GPIO_HIGH);

    syn_dc_motor_set_speed(&motor, -50);
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) == -50);
    TEST_ASSERT_TRUE(gpio_states[4] == SYN_GPIO_LOW);

    /* Clamping */
    syn_dc_motor_set_speed(&motor, 200);
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) == 100);

    /* Coast */
    syn_dc_motor_coast(&motor);
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) == 0);
    TEST_ASSERT_TRUE(gpio_states[3] == SYN_GPIO_LOW);
    TEST_ASSERT_TRUE(gpio_states[4] == SYN_GPIO_LOW);

    /* Brake */
    syn_dc_motor_brake(&motor);
    TEST_ASSERT_TRUE(gpio_states[3] == SYN_GPIO_HIGH);
    TEST_ASSERT_TRUE(gpio_states[4] == SYN_GPIO_HIGH);

    /* Ramp */
    mock_tick_ms = 0;
    syn_dc_motor_set_speed(&motor, 0);
    syn_dc_motor_ramp_to(&motor, 100, 100);
    TEST_ASSERT_TRUE(!syn_dc_motor_at_target(&motor));

    mock_tick_advance(50);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) > 0);

    mock_tick_advance(60);
    syn_dc_motor_update(&motor);
    TEST_ASSERT_TRUE(syn_dc_motor_at_target(&motor));
    TEST_ASSERT_TRUE(syn_dc_motor_get_speed(&motor) == 100);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 25: COBS framing                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

static uint8_t cobs_rx_buf[256];
static size_t  cobs_rx_len = 0;

static void cobs_on_packet(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    memcpy(cobs_rx_buf, data, len);
    cobs_rx_len = len;
}

static void test_cobs(void)
{
    printf("Test: COBS framing\n");

    /* Encode / decode roundtrip */
    uint8_t orig[] = { 0x00, 0x11, 0x00, 0x00, 0x22, 0x33 };
    uint8_t encoded[16], decoded[16];
    size_t enc_len = syn_cobs_encode(orig, sizeof(orig), encoded);
    TEST_ASSERT_TRUE(enc_len > 0);

    size_t dec_len = syn_cobs_decode(encoded, enc_len, decoded);
    TEST_ASSERT_TRUE(dec_len == sizeof(orig));
    TEST_ASSERT(memcmp(orig, decoded, sizeof(orig)) == 0, "decoded matches original");

    /* Simple data (no zeros) */
    uint8_t simple[] = { 0x01, 0x02, 0x03 };
    enc_len = syn_cobs_encode(simple, 3, encoded);
    dec_len = syn_cobs_decode(encoded, enc_len, decoded);
    TEST_ASSERT_TRUE(dec_len == 3);
    TEST_ASSERT(memcmp(simple, decoded, 3) == 0, "simple roundtrip");

    /* All zeros */
    uint8_t zeros[] = { 0x00, 0x00, 0x00 };
    enc_len = syn_cobs_encode(zeros, 3, encoded);
    dec_len = syn_cobs_decode(encoded, enc_len, decoded);
    TEST_ASSERT_TRUE(dec_len == 3);
    TEST_ASSERT(memcmp(zeros, decoded, 3) == 0, "all-zeros roundtrip");

    /* Streaming decoder */
    SYN_COBS_Decoder dec;
    uint8_t stream_buf[128];
    syn_cobs_decoder_init(&dec, stream_buf, sizeof(stream_buf),
                           cobs_on_packet, NULL);

    /* Feed the encoded packet byte-by-byte, then delimiter */
    enc_len = syn_cobs_encode(simple, 3, encoded);
    cobs_rx_len = 0;
    size_t i;
    for (i = 0; i < enc_len; i++) {
        syn_cobs_decoder_feed(&dec, encoded[i]);
    }
    TEST_ASSERT_TRUE(cobs_rx_len == 0);

    syn_cobs_decoder_feed(&dec, 0x00); /* delimiter */
    TEST_ASSERT_TRUE(cobs_rx_len == 3);
    TEST_ASSERT(memcmp(cobs_rx_buf, simple, 3) == 0, "streaming matches");
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 26: Sensor poller                                                    */
/* ══════════════════════════════════════════════════════════════════════════ */

static int16_t mock_sensor_value = 0;
static int sensor_high_count = 0;
static int sensor_low_count = 0;

static int16_t mock_sensor_read(void *ctx)
{
    (void)ctx;
    return mock_sensor_value;
}

static void sensor_on_high(SYN_Sensor *s, int16_t v, void *ctx)
{
    (void)s; (void)v; (void)ctx;
    sensor_high_count++;
}

static void sensor_on_low(SYN_Sensor *s, int16_t v, void *ctx)
{
    (void)s; (void)v; (void)ctx;
    sensor_low_count++;
}

static void test_sensor(void)
{
    printf("Test: Sensor poller\n");

    mock_tick_ms = 0;
    sensor_high_count = 0;
    sensor_low_count  = 0;

    SYN_Sensor sensor;
    SYN_FilterEMA ema;
    syn_filter_ema_init(&ema, 255);  /* alpha=1.0 (no filtering for testing) */

    syn_sensor_init(&sensor, "test", mock_sensor_read, NULL);
    syn_sensor_set_interval(&sensor, 100);
    syn_sensor_set_filter_ema(&sensor, &ema);
    syn_sensor_set_threshold(&sensor, 500, 50, sensor_on_high, sensor_on_low, NULL);

    /* Not time yet */
    mock_sensor_value = 0;
    TEST_ASSERT_TRUE(!syn_sensor_update(&sensor));

    /* Time elapsed */
    mock_tick_advance(100);
    TEST_ASSERT_TRUE(syn_sensor_update(&sensor));
    TEST_ASSERT_TRUE(syn_sensor_value(&sensor) == 0);
    TEST_ASSERT_TRUE(sensor_high_count == 0);

    /* Value crosses high threshold */
    mock_sensor_value = 600;
    mock_tick_advance(100);
    syn_sensor_update(&sensor);
    TEST_ASSERT_TRUE(sensor_high_count == 1);

    /* Value drops below low threshold */
    mock_sensor_value = 400;
    mock_tick_advance(100);
    syn_sensor_update(&sensor);
    TEST_ASSERT_TRUE(sensor_low_count == 1);

    /* Disable/enable */
    syn_sensor_enable(&sensor, false);
    mock_tick_advance(100);
    TEST_ASSERT_TRUE(!syn_sensor_update(&sensor));

    /* Force read (clear filter first so EMA state doesn't interfere) */
    syn_sensor_clear_filter(&sensor);
    mock_sensor_value = 123;
    int16_t v = syn_sensor_read_now(&sensor);
    TEST_ASSERT_TRUE(v == 123);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 27: Watchdog monitor                                                 */
/* ══════════════════════════════════════════════════════════════════════════ */

static int wdt_timeout_count = 0;
static const char *wdt_timeout_name = NULL;

static void wdt_on_timeout(SYN_Watchdog *wdt, const SYN_WDT_Entry *entry,
                           void *ctx)
{
    (void)wdt; (void)ctx;
    wdt_timeout_count++;
    wdt_timeout_name = entry->name;
}

static void test_watchdog(void)
{
    printf("Test: Watchdog monitor\n");

    mock_tick_ms = 0;
    wdt_timeout_count = 0;

    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[4];
    syn_watchdog_init(&wdt, entries, 4, wdt_on_timeout, NULL);

    int8_t id0 = syn_watchdog_register(&wdt, "task_a", 100);
    int8_t id1 = syn_watchdog_register(&wdt, "task_b", 200);
    TEST_ASSERT_TRUE(id0 >= 0);
    TEST_ASSERT_TRUE(id1 >= 0);

    /* No timeout yet */
    mock_tick_advance(50);
    syn_watchdog_update(&wdt);
    TEST_ASSERT_TRUE(wdt_timeout_count == 0);

    /* Check in task_a */
    syn_watchdog_checkin(&wdt, id0);

    /* task_a deadline at 150ms, task_b at 200ms */
    mock_tick_advance(60);  /* now 110ms */
    syn_watchdog_update(&wdt);
    TEST_ASSERT_TRUE(wdt_timeout_count == 0);

    mock_tick_advance(100); /* now 210ms — task_b should time out */
    syn_watchdog_update(&wdt);
    TEST_ASSERT_TRUE(wdt_timeout_count >= 1);
    TEST_ASSERT_TRUE(strcmp(wdt_timeout_name) == 0, "correct task name");

    /* Unregister */
    syn_watchdog_unregister(&wdt, id1);
    wdt_timeout_count = 0;
    mock_tick_advance(500);
    syn_watchdog_update(&wdt);
    /* Only id0 should fire (id1 unregistered) */
    TEST_ASSERT_TRUE(wdt_timeout_count >= 1);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 28: Parameter store with wear leveling                               */
/* ══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    uint16_t brightness;
    int16_t  offset;
    uint8_t  mode;
    uint8_t  _pad;
} TestParams;

static void test_param_store(void)
{
    printf("Test: Parameter store\n");

    /* Erase flash */
    memset(mock_flash, 0xFF, sizeof(mock_flash));

    SYN_ParamStore store;
    /* 4 sectors of 1024 bytes each */
    SYN_Status st = syn_param_init(&store, 0, 4, sizeof(TestParams));
    TEST_ASSERT_TRUE(st == SYN_ERROR);

    /* Save defaults */
    TestParams params = { .brightness = 80, .offset = -10, .mode = 3 };
    st = syn_param_save(&store, &params);
    TEST_ASSERT_TRUE(st == SYN_OK);

    /* Load back */
    TestParams loaded;
    memset(&loaded, 0, sizeof(loaded));
    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_TRUE(st == SYN_OK);
    TEST_ASSERT_TRUE(loaded.brightness == 80);
    TEST_ASSERT_TRUE(loaded.offset == -10);
    TEST_ASSERT_TRUE(loaded.mode == 3);

    /* Save again (goes to next slot) */
    params.brightness = 90;
    st = syn_param_save(&store, &params);
    TEST_ASSERT_TRUE(st == SYN_OK);

    /* Re-init and load (should find latest) */
    SYN_ParamStore store2;
    st = syn_param_init(&store2, 0, 4, sizeof(TestParams));
    TEST_ASSERT_TRUE(st == SYN_OK);

    memset(&loaded, 0, sizeof(loaded));
    st = syn_param_load(&store2, &loaded);
    TEST_ASSERT_TRUE(st == SYN_OK);
    TEST_ASSERT_TRUE(loaded.brightness == 90);

    /* Wear leveling: save many times and verify it wraps sectors */
    int i;
    for (i = 0; i < 200; i++) {
        params.brightness = (uint16_t)(i & 0xFFFF);
        st = syn_param_save(&store, &params);
        TEST_ASSERT_TRUE(st == SYN_OK);
    }

    memset(&loaded, 0, sizeof(loaded));
    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_TRUE(st == SYN_OK);
    TEST_ASSERT_TRUE(loaded.brightness == 199);

    /* Erase all */
    st = syn_param_erase_all(&store);
    TEST_ASSERT_TRUE(st == SYN_OK);

    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_TRUE(st == SYN_ERROR);

    /* Write count */
    uint16_t wc = syn_param_write_count(&store);
    (void)wc; /* just verify it doesn't crash */
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 29: Stepper motor                                                    */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_stepper(void)
{
    printf("Test: Stepper motor\n");

    mock_tick_ms = 0;

    SYN_Stepper stepper;
    syn_stepper_init(&stepper, 5, 6);
    syn_stepper_set_speed(&stepper, 100, 1000);

    TEST_ASSERT_TRUE(!syn_stepper_is_moving(&stepper));
    TEST_ASSERT_TRUE(syn_stepper_position(&stepper) == 0);

    /* Move 10 steps forward */
    syn_stepper_move(&stepper, 10);
    TEST_ASSERT_TRUE(syn_stepper_is_moving(&stepper));

    /* Tick until complete */
    int ticks = 0;
    while (syn_stepper_is_moving(&stepper) && ticks < 10000) {
        mock_tick_advance(1);
        syn_stepper_tick(&stepper);
        ticks++;
    }
    TEST_ASSERT_TRUE(!syn_stepper_is_moving(&stepper));
    TEST_ASSERT_TRUE(syn_stepper_position(&stepper) == 10);

    /* Move to absolute position */
    syn_stepper_move_to(&stepper, 0);
    while (syn_stepper_is_moving(&stepper) && ticks < 20000) {
        mock_tick_advance(1);
        syn_stepper_tick(&stepper);
        ticks++;
    }
    TEST_ASSERT_TRUE(syn_stepper_position(&stepper) == 0);

    /* Emergency stop */
    syn_stepper_move(&stepper, 1000);
    mock_tick_advance(5);
    syn_stepper_tick(&stepper);
    syn_stepper_stop(&stepper);
    TEST_ASSERT_TRUE(!syn_stepper_is_moving(&stepper));

    /* Set position */
    syn_stepper_set_position(&stepper, 500);
    TEST_ASSERT_TRUE(syn_stepper_position(&stepper) == 500);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Main                                                                      */
/* ══════════════════════════════════════════════════════════════════════════ */

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 30: Q16.16 fixed-point math                                          */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_qmath(void)
{
    printf("Test: Q16.16 math\n");

    q16_t a = Q16_FROM_INT(3);
    q16_t b = Q16_FROM_INT(2);
    TEST_ASSERT_TRUE(Q16_TO_INT(a) == 3);

    /* Add/sub */
    TEST_ASSERT(Q16_TO_INT(q16_add(a, b)) == 5, "3 + 2 = 5");
    TEST_ASSERT(Q16_TO_INT(q16_sub(a, b)) == 1, "3 - 2 = 1");

    /* Multiply */
    q16_t c = q16_mul(a, Q16_FROM_FRAC(1, 2));
    TEST_ASSERT_TRUE(Q16_TO_INT(c) == 1);
    TEST_ASSERT_TRUE(Q16_TO_INT_ROUND(c) == 2);

    /* Divide */
    q16_t d = q16_div(Q16_FROM_INT(10), Q16_FROM_INT(4));
    TEST_ASSERT_TRUE(Q16_TO_INT(d) == 2);
    TEST_ASSERT_TRUE(Q16_FRAC_1000(d) == 500);

    /* Abs */
    TEST_ASSERT_TRUE(q16_abs(Q16_FROM_INT(-5)) == Q16_FROM_INT(5));

    /* Clamp */
    q16_t lo = Q16_FROM_INT(0);
    q16_t hi = Q16_FROM_INT(100);
    TEST_ASSERT(q16_clamp(Q16_FROM_INT(50), lo, hi) == Q16_FROM_INT(50), "clamp middle");
    TEST_ASSERT(q16_clamp(Q16_FROM_INT(-10), lo, hi) == lo, "clamp low");
    TEST_ASSERT(q16_clamp(Q16_FROM_INT(200), lo, hi) == hi, "clamp high");

    /* Lerp */
    q16_t r = q16_lerp(Q16_FROM_INT(0), Q16_FROM_INT(100), Q16_FROM_FRAC(1, 2));
    TEST_ASSERT_TRUE(Q16_TO_INT(r) == 50);

    /* Saturating add */
    q16_t big = INT32_MAX - 1000;
    TEST_ASSERT(q16_add_sat(big, Q16_FROM_INT(1)) == INT32_MAX, "sat add clamp");
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 31: Rate limiter                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_rate_limit(void)
{
    printf("Test: Rate limiter\n");

    mock_tick_ms = 0;

    SYN_RateLimit rl;
    syn_rate_limit_init(&rl, 3, 1000);  /* 3 per second */

    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));
    TEST_ASSERT_TRUE(!syn_rate_limit_allow(&rl));

    TEST_ASSERT_TRUE(syn_rate_limit_remaining(&rl) == 0);

    /* After full interval, tokens refill */
    mock_tick_advance(1000);
    TEST_ASSERT_TRUE(syn_rate_limit_allow(&rl));

    /* Reset */
    syn_rate_limit_reset(&rl);
    TEST_ASSERT_TRUE(syn_rate_limit_remaining(&rl) == 3);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 32: Rotary encoder                                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_encoder(void)
{
    printf("Test: Rotary encoder\n");

    /* Init with both pins low */
    gpio_states[10] = 0;
    gpio_states[11] = 0;

    SYN_Encoder enc;
    syn_encoder_init(&enc, 10, 11);
    TEST_ASSERT_TRUE(syn_encoder_position(&enc) == 0);

    /* Simulate CW rotation: 00 → 10 → 11 → 01 → 00 */
    gpio_states[10] = 1; gpio_states[11] = 0;  /* 10 */
    syn_encoder_update(&enc);

    gpio_states[10] = 1; gpio_states[11] = 1;  /* 11 */
    syn_encoder_update(&enc);

    gpio_states[10] = 0; gpio_states[11] = 1;  /* 01 */
    syn_encoder_update(&enc);

    gpio_states[10] = 0; gpio_states[11] = 0;  /* 00 */
    syn_encoder_update(&enc);

    int32_t delta = syn_encoder_get_delta(&enc);
    TEST_ASSERT_TRUE(delta == 4);
    TEST_ASSERT_TRUE(syn_encoder_position(&enc) == 4);

    /* Simulate CCW: 00 → 01 → 11 → 10 → 00 */
    gpio_states[10] = 0; gpio_states[11] = 1;  /* 01 */
    syn_encoder_update(&enc);

    gpio_states[10] = 1; gpio_states[11] = 1;  /* 11 */
    syn_encoder_update(&enc);

    gpio_states[10] = 1; gpio_states[11] = 0;  /* 10 */
    syn_encoder_update(&enc);

    gpio_states[10] = 0; gpio_states[11] = 0;  /* 00 */
    syn_encoder_update(&enc);

    delta = syn_encoder_get_delta(&enc);
    TEST_ASSERT_TRUE(delta == -4);
    TEST_ASSERT_TRUE(syn_encoder_position(&enc) == 0);

    /* Delta auto-resets */
    TEST_ASSERT_TRUE(syn_encoder_get_delta(&enc) == 0);

    /* Set position */
    syn_encoder_set_position(&enc, 100);
    TEST_ASSERT_TRUE(syn_encoder_position(&enc) == 100);

    /* Steps-per-detent */
    syn_encoder_set_steps_per_detent(&enc, 4);
    syn_encoder_set_position(&enc, 0);

    /* 4 CW state changes = 1 detent */
    gpio_states[10] = 1; gpio_states[11] = 0;
    syn_encoder_update(&enc);
    gpio_states[10] = 1; gpio_states[11] = 1;
    syn_encoder_update(&enc);
    gpio_states[10] = 0; gpio_states[11] = 1;
    syn_encoder_update(&enc);
    gpio_states[10] = 0; gpio_states[11] = 0;
    syn_encoder_update(&enc);

    TEST_ASSERT_TRUE(syn_encoder_position(&enc) == 1);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 33: Trace buffer                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_trace(void)
{
    printf("Test: Trace buffer\n");

    mock_tick_ms = 0;

    SYN_TraceEntry entries[4];
    SYN_Trace trace;
    syn_trace_init(&trace, entries, 4);

    TEST_ASSERT_TRUE(syn_trace_count(&trace) == 0);

    /* Record some events */
    mock_tick_ms = 100;
    syn_trace_record(&trace, 0x01, 42);
    mock_tick_ms = 200;
    syn_trace_record(&trace, 0x02, 99);
    TEST_ASSERT_TRUE(syn_trace_count(&trace) == 2);

    /* Read back */
    SYN_TraceEntry e;
    TEST_ASSERT(syn_trace_read(&trace, 0, &e), "read index 0");
    TEST_ASSERT_TRUE(e.event_id == 0x01);
    TEST_ASSERT_TRUE(e.value == 42);
    TEST_ASSERT_TRUE(e.timestamp == 100);

    TEST_ASSERT(syn_trace_read(&trace, 1, &e), "read index 1");
    TEST_ASSERT_TRUE(e.event_id == 0x02);

    /* Overflow wraps (capacity=4, add 4 more → wraps) */
    mock_tick_ms = 300;
    syn_trace_record(&trace, 0x03, 0);
    syn_trace_record(&trace, 0x04, 0);
    syn_trace_record(&trace, 0x05, 0);  /* wraps, overwrites slot 0 */
    syn_trace_record(&trace, 0x06, 0);

    TEST_ASSERT_TRUE(syn_trace_count(&trace) == 6);
    /* Oldest available should now be event 0x03 */
    TEST_ASSERT(syn_trace_read(&trace, 0, &e), "read after wrap");
    TEST_ASSERT_TRUE(e.event_id == 0x03);

    /* Disable */
    syn_trace_enable(&trace, false);
    syn_trace_record(&trace, 0xFF, 0);
    TEST_ASSERT_TRUE(syn_trace_count(&trace) == 6);

    /* Clear */
    syn_trace_enable(&trace, true);
    syn_trace_clear(&trace);
    TEST_ASSERT_TRUE(syn_trace_count(&trace) == 0);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 34: Task profiler                                                    */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_profiler(void)
{
    printf("Test: Task profiler\n");

    mock_tick_ms = 0;

    SYN_ProfileEntry prof_entries[3];
    SYN_Profiler prof;
    syn_profiler_init(&prof, prof_entries, 3);

    syn_profiler_register(&prof, 0, "task_a");
    syn_profiler_register(&prof, 1, "task_b");

    /* Simulate task_a taking 5ms */
    syn_profiler_task_begin(&prof, 0);
    mock_tick_advance(5);
    syn_profiler_task_end(&prof, 0);

    /* Simulate task_b taking 2ms */
    syn_profiler_task_begin(&prof, 1);
    mock_tick_advance(2);
    syn_profiler_task_end(&prof, 1);

    /* Run again */
    syn_profiler_task_begin(&prof, 0);
    mock_tick_advance(3);
    syn_profiler_task_end(&prof, 0);

    /* Update stats (total period = 10ms) */
    syn_profiler_update(&prof);

    const SYN_ProfileEntry *e0 = syn_profiler_get(&prof, 0);
    TEST_ASSERT_TRUE(e0 != NULL);
    TEST_ASSERT_TRUE(e0->peak_us == 5000);
    /* CPU%: 8ms total_us / 10ms period × 100 × 10 = 800 → 80.0% */
    /* But our run_count was reset by update, check it was 2 before */
    TEST_ASSERT_TRUE(e0->cpu_percent_x10 > 0);

    const SYN_ProfileEntry *e1 = syn_profiler_get(&prof, 1);
    TEST_ASSERT_TRUE(e1 != NULL);
    TEST_ASSERT_TRUE(e1->peak_us == 2000);

    /* Enable/disable */
    syn_profiler_enable(&prof, false);
    syn_profiler_task_begin(&prof, 0);
    mock_tick_advance(100);
    syn_profiler_task_end(&prof, 0);
    TEST_ASSERT_TRUE(e0->peak_us == 5000);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 35: Sequencer                                                        */
/* ══════════════════════════════════════════════════════════════════════════ */

static int seq_action_count = 0;
static void seq_action_a(void *ctx) { (void)ctx; seq_action_count++; }

static int seq_complete_count = 0;
static void seq_on_done(SYN_Sequencer *seq, void *ctx)
{
    (void)seq; (void)ctx;
    seq_complete_count++;
}

static void test_sequencer(void)
{
    printf("Test: Sequencer\n");

    mock_tick_ms = 0;
    seq_action_count = 0;
    seq_complete_count = 0;

    static const SYN_SeqStep steps[] = {
        { seq_action_a, NULL, 0 },     /* step 0: action, no delay */
        { NULL,         NULL, 100 },   /* step 1: delay 100ms      */
        { seq_action_a, NULL, 50 },    /* step 2: action + 50ms    */
        { seq_action_a, NULL, 0 },     /* step 3: action, finish   */
    };

    SYN_Sequencer seq;
    syn_seq_init(&seq, steps, 4);
    syn_seq_on_complete(&seq, seq_on_done, NULL);

    TEST_ASSERT_TRUE(syn_seq_is_done(&seq));

    syn_seq_start(&seq);
    TEST_ASSERT_TRUE(!syn_seq_is_done(&seq));

    /* First update: runs step 0 (action), step 1 starts waiting */
    syn_seq_update(&seq);
    TEST_ASSERT_TRUE(seq_action_count == 1);

    /* Not enough time for step 1's delay */
    mock_tick_advance(50);
    syn_seq_update(&seq);
    TEST_ASSERT_TRUE(seq_action_count == 1);

    /* Step 1 delay complete, step 2 runs (action + starts delay) */
    mock_tick_advance(60);
    syn_seq_update(&seq);
    TEST_ASSERT_TRUE(seq_action_count == 2);

    /* Step 2 delay done, step 3 runs, sequence done */
    mock_tick_advance(55);
    bool done = syn_seq_update(&seq);
    TEST_ASSERT_TRUE(seq_action_count == 3);
    TEST_ASSERT_TRUE(done);
    TEST_ASSERT_TRUE(seq_complete_count == 1);
    TEST_ASSERT_TRUE(syn_seq_is_done(&seq));
    TEST_ASSERT_TRUE(syn_seq_loops(&seq) == 1);

    /* Loop mode */
    seq_action_count = 0;
    syn_seq_set_loop(&seq, true);
    syn_seq_start(&seq);
    syn_seq_update(&seq);
    mock_tick_advance(200);
    syn_seq_update(&seq);
    mock_tick_advance(200);
    syn_seq_update(&seq);
    /* Should have looped at least once */
    TEST_ASSERT_TRUE(syn_seq_loops(&seq) >= 2);
    syn_seq_stop(&seq);
    TEST_ASSERT_TRUE(syn_seq_is_done(&seq));
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 36: ADC abstraction                                                  */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_adc(void)
{
    printf("Test: ADC abstraction\n");

    SYN_ADC adc;
    SYN_ADC_Config cfg = {
        .channel = 0,
        .oversample = 4,
        .filter = NULL,
        .filter_type = SYN_ADC_FILTER_NONE,
        .cal_offset = 0,
        .cal_scale = 1,
        .cal_scale_shift = 0,
    };

    mock_adc_value = 2048;
    syn_adc_init(&adc, &cfg);

    int32_t val = syn_adc_read(&adc);
    TEST_ASSERT_TRUE(val == 2048);
    TEST_ASSERT_TRUE(syn_adc_raw(&adc) == 2048);

    /* mV conversion: 2048/4095 * 3300 ≈ 1649 */
    int32_t mv = syn_adc_read_mv(&adc);
    TEST_ASSERT_TRUE(mv >= 1640 && mv <= 1660);

    /* With calibration offset */
    syn_adc_set_calibration(&adc, 100, 1, 0);
    val = syn_adc_read(&adc);
    TEST_ASSERT_TRUE(val == 2148);

    /* With scale */
    syn_adc_set_calibration(&adc, 0, 2048, 10);  /* × 2.0 */
    val = syn_adc_read(&adc);
    TEST_ASSERT_TRUE(val == 4096);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 37: Boot manager                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_boot(void)
{
    printf("Test: Boot manager\n");

    /* Fresh flash */
    memset(mock_flash, 0xFF, sizeof(mock_flash));

    SYN_ParamStore boot_store;
    SYN_Status st = syn_param_init(&boot_store, 0, 2, sizeof(SYN_BootData));
    /* No data yet */

    SYN_Boot boot;
    st = syn_boot_init(&boot, &boot_store, 3);
    TEST_ASSERT_TRUE(st == SYN_ERROR);
    TEST_ASSERT_TRUE(syn_boot_count(&boot) == 1);
    TEST_ASSERT_TRUE(!syn_boot_is_safe_mode(&boot));

    /* Mark healthy */
    st = syn_boot_mark_healthy(&boot);
    TEST_ASSERT_TRUE(st == SYN_OK);

    /* Simulate reboot (re-init) */
    SYN_ParamStore boot_store2;
    syn_param_init(&boot_store2, 0, 2, sizeof(SYN_BootData));

    SYN_Boot boot2;
    st = syn_boot_init(&boot2, &boot_store2, 3);
    TEST_ASSERT_TRUE(st == SYN_OK);
    TEST_ASSERT_TRUE(syn_boot_count(&boot2) == 2);
    TEST_ASSERT_TRUE(syn_boot_fail_count(&boot2) == 0);

    /* Simulate crash loops (don't mark healthy) */
    int i;
    for (i = 0; i < 3; i++) {
        SYN_ParamStore bs;
        syn_param_init(&bs, 0, 2, sizeof(SYN_BootData));
        SYN_Boot b;
        syn_boot_init(&b, &bs, 3);
        /* don't mark healthy */
    }

    /* Next boot should be safe mode */
    SYN_ParamStore bs;
    syn_param_init(&bs, 0, 2, sizeof(SYN_BootData));
    SYN_Boot bsafe;
    syn_boot_init(&bsafe, &bs, 3);
    TEST_ASSERT_TRUE(syn_boot_is_safe_mode(&bsafe));

    /* Clear safe mode */
    st = syn_boot_clear_safe_mode(&bsafe);
    TEST_ASSERT_TRUE(st == SYN_OK);
    TEST_ASSERT_TRUE(!syn_boot_is_safe_mode(&bsafe));
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 38: Modbus RTU (frame processing)                                    */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_modbus(void)
{
    printf("Test: Modbus RTU\n");

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
    TEST_ASSERT_TRUE(mb.frames_rx == 1);

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
    TEST_ASSERT_TRUE(holding[0] == 999);

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
    TEST_ASSERT_TRUE(!processed);

    /* Bad CRC */
    req[0] = 1;
    req[6] = 0xFF; req[7] = 0xFF; /* corrupt CRC */
    for (i = 0; i < 8; i++) mb.buf[i] = req[i];
    mb.rx_len = 8;

    processed = syn_modbus_process(&mb);
    TEST_ASSERT_TRUE(!processed);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 39: Closed-loop motor controller                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

static int32_t mock_ctrl_position = 0;

static int32_t mock_encoder_feedback(void *ctx)
{
    (void)ctx;
    return mock_ctrl_position;
}

/* Simulate pot reading for linear actuator */
static int32_t mock_pot_position = 2048;

static int32_t mock_pot_feedback(void *ctx)
{
    (void)ctx;
    return mock_pot_position;
}

static int mock_stall_count = 0;
static void mock_stall_cb(SYN_MotorCtrl *c, void *ctx)
{
    (void)c; (void)ctx;
    mock_stall_count++;
}

static int mock_target_count = 0;
static void mock_target_cb(SYN_MotorCtrl *c, void *ctx)
{
    (void)c; (void)ctx;
    mock_target_count++;
}

static void test_motor_ctrl(void)
{
    printf("Test: Closed-loop motor controller\n");

    mock_tick_ms = 0;
    mock_ctrl_position = 0;
    mock_stall_count = 0;
    mock_target_count = 0;

    /* ── Test 1: Encoder-based velocity control ─────────────── */

    SYN_DCMotor dc;
    syn_dc_motor_init(&dc, 6, 7, SYN_DC_MODE_PWM_DIR);

    SYN_MotorCtrl ctrl;
    SYN_MotorCtrl_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.type          = SYN_MCTRL_DC;
    cfg.read_pos      = mock_encoder_feedback;
    cfg.read_pos_ctx  = NULL;
    cfg.dc_motor      = &dc;
    cfg.pid_kp        = 100;
    cfg.pid_ki        = 10;
    cfg.pid_kd        = 5;
    cfg.pid_scale     = 6;
    cfg.update_hz     = 100;
    cfg.output_min    = -255;
    cfg.output_max    = 255;

    syn_motor_ctrl_init(&ctrl, &cfg);
    TEST_ASSERT_TRUE(syn_motor_ctrl_state(&ctrl) == SYN_MCTRL_STOPPED);
    TEST_ASSERT_TRUE(syn_motor_ctrl_mode(&ctrl) == SYN_MCTRL_MODE_IDLE);

    /* Set velocity target */
    syn_motor_ctrl_set_velocity(&ctrl, 1000);
    TEST_ASSERT_TRUE(syn_motor_ctrl_mode(&ctrl) == SYN_MCTRL_MODE_VELOCITY);
    TEST_ASSERT_TRUE(syn_motor_ctrl_state(&ctrl) == SYN_MCTRL_RUNNING);

    /* Simulate: feedback = 10 counts per update at 100Hz = 1000 cnt/s */
    mock_ctrl_position = 10;
    mock_tick_advance(10);
    SYN_MotorCtrl_State st = syn_motor_ctrl_update(&ctrl);
    TEST_ASSERT_TRUE(st == SYN_MCTRL_RUNNING);
    TEST_ASSERT_TRUE(syn_motor_ctrl_velocity(&ctrl) == 1000);

    /* Stop */
    syn_motor_ctrl_stop(&ctrl);
    TEST_ASSERT_TRUE(syn_motor_ctrl_state(&ctrl) == SYN_MCTRL_STOPPED);
    TEST_ASSERT_TRUE(syn_motor_ctrl_output(&ctrl) == 0);

    /* ── Test 2: Pot-based position control (linear actuator) ── */

    mock_pot_position = 1000;

    SYN_MotorCtrl act;
    SYN_MotorCtrl_Config acfg;
    memset(&acfg, 0, sizeof(acfg));
    acfg.type              = SYN_MCTRL_DC;
    acfg.read_pos          = mock_pot_feedback;
    acfg.read_pos_ctx      = NULL;
    acfg.dc_motor          = &dc;
    acfg.pid_kp            = 50;
    acfg.pid_ki            = 5;
    acfg.pid_kd            = 10;
    acfg.pid_scale         = 6;
    acfg.update_hz         = 50;
    acfg.output_min        = -255;
    acfg.output_max        = 255;
    acfg.position_deadband = 10;
    acfg.position_min      = 100;
    acfg.position_max      = 3900;

    syn_motor_ctrl_init(&act, &acfg);
    syn_motor_ctrl_on_target(&act, mock_target_cb, NULL);

    /* Set position target */
    syn_motor_ctrl_set_position(&act, 2048);
    TEST_ASSERT_TRUE(syn_motor_ctrl_mode(&act) == SYN_MCTRL_MODE_POSITION);

    /* Simulate: still at 1000, far from 2048 */
    mock_tick_advance(20);
    st = syn_motor_ctrl_update(&act);
    TEST_ASSERT_TRUE(st == SYN_MCTRL_RUNNING);
    TEST_ASSERT_TRUE(syn_motor_ctrl_output(&act) > 0);

    /* Simulate: arrived at target */
    mock_pot_position = 2050;  /* within deadband of 10 */
    mock_tick_advance(20);
    st = syn_motor_ctrl_update(&act);
    TEST_ASSERT_TRUE(st == SYN_MCTRL_ON_TARGET);
    TEST_ASSERT_TRUE(mock_target_count == 1);

    /* ── Test 3: Soft position limits ───────────────────────── */

    mock_pot_position = 50;  /* below position_min (100) */
    syn_motor_ctrl_set_position(&act, 500);
    mock_tick_advance(20);
    st = syn_motor_ctrl_update(&act);
    /* PID wants positive output (moving up), which is allowed from below min */
    TEST_ASSERT_TRUE(st == SYN_MCTRL_RUNNING);

    /* Target beyond max gets clamped */
    syn_motor_ctrl_set_position(&act, 5000);
    TEST_ASSERT_TRUE(act.target_position == 3900);

    /* ── Test 4: Stall detection ────────────────────────────── */

    mock_ctrl_position = 0;
    mock_stall_count = 0;

    SYN_MotorCtrl stctrl;
    SYN_MotorCtrl_Config scfg;
    memset(&scfg, 0, sizeof(scfg));
    scfg.type              = SYN_MCTRL_DC;
    scfg.read_pos          = mock_encoder_feedback;
    scfg.read_pos_ctx      = NULL;
    scfg.dc_motor          = &dc;
    scfg.pid_kp            = 200;
    scfg.pid_ki            = 0;
    scfg.pid_kd            = 0;
    scfg.pid_scale         = 6;
    scfg.update_hz         = 100;
    scfg.output_min        = -255;
    scfg.output_max        = 255;
    scfg.stall_timeout_ms  = 100;
    scfg.stall_threshold   = 0;

    syn_motor_ctrl_init(&stctrl, &scfg);
    syn_motor_ctrl_on_stall(&stctrl, mock_stall_cb, NULL);

    /* Set velocity but don't move feedback → stall */
    syn_motor_ctrl_set_velocity(&stctrl, 5000);
    mock_ctrl_position = 0; /* not moving! */

    /* Pump updates without moving */
    int i;
    for (i = 0; i < 12; i++) {
        mock_tick_advance(10);
        syn_motor_ctrl_update(&stctrl);
    }
    /* After 120ms > stall_timeout_ms(100), should be stalled */
    TEST_ASSERT_TRUE(syn_motor_ctrl_state(&stctrl) == SYN_MCTRL_STALLED);
    TEST_ASSERT_TRUE(mock_stall_count == 1);

    /* ── Test 5: E-stop ─────────────────────────────────────── */
    syn_motor_ctrl_estop(&stctrl);
    TEST_ASSERT_TRUE(syn_motor_ctrl_state(&stctrl) == SYN_MCTRL_STOPPED);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 40: Lightweight formatting                                           */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_fmt(void)
{
    printf("Test: Lightweight formatting\n");

    char buf[64];
    size_t n;

    /* Signed integer */
    n = syn_fmt_int(buf, sizeof(buf), 12345);
    TEST_ASSERT_TRUE(n == 5);
    TEST_ASSERT_TRUE(strcmp(buf) == 0, "12345");

    n = syn_fmt_int(buf, sizeof(buf), -42);
    TEST_ASSERT_TRUE(strcmp(buf) == 0, "-42");

    n = syn_fmt_int(buf, sizeof(buf), 0);
    TEST_ASSERT_TRUE(strcmp(buf) == 0, "0");

    /* Unsigned */
    n = syn_fmt_uint(buf, sizeof(buf), 4294967295u);
    TEST_ASSERT_TRUE(strcmp(buf) == 0, "uint max");

    /* Hex */
    n = syn_fmt_hex(buf, sizeof(buf), 0xDEAD, 4);
    TEST_ASSERT_TRUE(strcmp(buf) == 0, "hex DEAD");

    n = syn_fmt_hex(buf, sizeof(buf), 0x0A, 4);
    TEST_ASSERT_TRUE(strcmp(buf) == 0, "hex 000A");

    /* Q16.16 */
    int32_t q = Q16_FROM_FRAC(355, 113); /* ≈ π ≈ 3.141 */
    n = syn_fmt_q16(buf, sizeof(buf), q, 3);
    TEST_ASSERT_TRUE(buf[0] == '3');
    TEST_ASSERT_TRUE(buf[1] == '.');
    /* 355/113 ≈ 3.14159 → should be "3.141" */
    TEST_ASSERT_TRUE(buf[2] == '1');

    /* Negative Q16 */
    n = syn_fmt_q16(buf, sizeof(buf), Q16_FROM_INT(-7), 2);
    TEST_ASSERT_TRUE(buf[0] == '-');
    TEST_ASSERT_TRUE(buf[1] == '7');

    /* Fixed decimal */
    n = syn_fmt_fixed(buf, sizeof(buf), 12345, 3);
    TEST_ASSERT_TRUE(strcmp(buf) == 0, "fixed 12.345");

    /* Hex dump */
    uint8_t data[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    n = syn_fmt_hexdump(buf, sizeof(buf), data, 4);
    TEST_ASSERT_TRUE(strcmp(buf) == 0, "hexdump");

    /* Concat */
    const char *parts[] = { "Hello", " ", "World" };
    n = syn_fmt_concat(buf, sizeof(buf), parts, 3);
    TEST_ASSERT_TRUE(strcmp(buf) == 0, "concat");

    /* Truncation safety */
    char tiny[4];
    syn_fmt_int(tiny, sizeof(tiny), 12345);
    TEST_ASSERT_TRUE(tiny[3] == '\0');

    (void)n;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 41: Ping-pong buffer                                                 */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_pingpong(void)
{
    printf("Test: Ping-pong buffer\n");

    uint8_t a[8], b[8];
    SYN_PingPong pp;
    syn_pingpong_init(&pp, a, b, 8);

    TEST_ASSERT_TRUE(syn_pingpong_active(&pp) == a);
    TEST_ASSERT_TRUE(!syn_pingpong_ready(&pp));
    TEST_ASSERT_TRUE(syn_pingpong_size(&pp) == 8);

    /* Write to active, swap */
    memset(syn_pingpong_active(&pp), 0xAA, 8);
    syn_pingpong_swap(&pp);

    TEST_ASSERT_TRUE(syn_pingpong_ready(&pp));
    TEST_ASSERT_TRUE(syn_pingpong_active(&pp) == b);
    TEST_ASSERT_TRUE(syn_pingpong_ready_buf(&pp) == a);
    TEST_ASSERT_TRUE(syn_pingpong_ready_buf(&pp)[0] == 0xAA);

    /* Consume */
    syn_pingpong_consume(&pp);
    TEST_ASSERT_TRUE(!syn_pingpong_ready(&pp));

    /* Write to b, swap back */
    memset(syn_pingpong_active(&pp), 0xBB, 8);
    syn_pingpong_swap(&pp);
    TEST_ASSERT_TRUE(syn_pingpong_active(&pp) == a);
    TEST_ASSERT_TRUE(syn_pingpong_ready_buf(&pp)[0] == 0xBB);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 42: Work queue                                                       */
/* ══════════════════════════════════════════════════════════════════════════ */

static int wq_sum = 0;
static void wq_handler(void *ctx) { wq_sum += *(int *)ctx; }

static void test_workqueue(void)
{
    printf("Test: Work queue\n");

    wq_sum = 0;

    SYN_WorkItem items[4];
    SYN_WorkQueue wq;
    syn_workqueue_init(&wq, items, 4);

    TEST_ASSERT_TRUE(syn_workqueue_empty(&wq));
    TEST_ASSERT_TRUE(syn_workqueue_pending(&wq) == 0);

    static int v1 = 10, v2 = 20, v3 = 30;

    TEST_ASSERT(syn_workqueue_post(&wq, wq_handler, &v1), "post 1");
    TEST_ASSERT(syn_workqueue_post(&wq, wq_handler, &v2), "post 2");
    TEST_ASSERT(syn_workqueue_post(&wq, wq_handler, &v3), "post 3");
    /* Queue capacity = 4, but SPSC ring uses one slot as sentinel,
       so max usable = 3 */
    TEST_ASSERT(!syn_workqueue_post(&wq, wq_handler, &v1), "post 4 fails (full)");
    TEST_ASSERT_TRUE(syn_workqueue_overflows(&wq) == 1);

    TEST_ASSERT_TRUE(!syn_workqueue_empty(&wq));
    TEST_ASSERT_TRUE(syn_workqueue_pending(&wq) == 3);

    size_t processed = syn_workqueue_process(&wq);
    TEST_ASSERT_TRUE(processed == 3);
    TEST_ASSERT_TRUE(wq_sum == 60);
    TEST_ASSERT_TRUE(syn_workqueue_empty(&wq));

    /* Process empty queue = 0 */
    processed = syn_workqueue_process(&wq);
    TEST_ASSERT_TRUE(processed == 0);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 43: Error log                                                        */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_errlog(void)
{
    printf("Test: Error log\n");

    mock_tick_ms = 0;

    SYN_ErrEntry entries[4];
    SYN_ErrLog elog;
    syn_errlog_init(&elog, entries, 4, 5); /* boot count 5 */

    TEST_ASSERT_TRUE(syn_errlog_count(&elog) == 0);
    TEST_ASSERT_TRUE(syn_errlog_latest(&elog) == NULL);

    /* Record some errors */
    mock_tick_ms = 100;
    syn_errlog_record(&elog, 0x0001, SYN_ERR_WARNING, 42);
    mock_tick_ms = 200;
    syn_errlog_record(&elog, 0x0002, SYN_ERR_ERROR, 99);

    TEST_ASSERT_TRUE(syn_errlog_count(&elog) == 2);
    TEST_ASSERT_TRUE(syn_errlog_available(&elog) == 2);

    /* Read back */
    SYN_ErrEntry e;
    TEST_ASSERT(syn_errlog_read(&elog, 0, &e), "read 0");
    TEST_ASSERT_TRUE(e.code == 0x0001);
    TEST_ASSERT_TRUE(e.severity == SYN_ERR_WARNING);
    TEST_ASSERT_TRUE(e.context == 42);
    TEST_ASSERT_TRUE(e.timestamp == 100);
    TEST_ASSERT_TRUE(e.boot_count == 5);

    /* Latest */
    const SYN_ErrEntry *latest = syn_errlog_latest(&elog);
    TEST_ASSERT_TRUE(latest != NULL);
    TEST_ASSERT_TRUE(latest->code == 0x0002);

    /* Severity count */
    TEST_ASSERT(syn_errlog_count_severity(&elog, SYN_ERR_WARNING) == 1, "1 warning");
    TEST_ASSERT(syn_errlog_count_severity(&elog, SYN_ERR_ERROR) == 1, "1 error");
    TEST_ASSERT(syn_errlog_count_severity(&elog, SYN_ERR_FATAL) == 0, "0 fatal");

    /* Overflow wraps (capacity=4) */
    syn_errlog_record(&elog, 0x0003, SYN_ERR_INFO, 0);
    syn_errlog_record(&elog, 0x0004, SYN_ERR_INFO, 0);
    syn_errlog_record(&elog, 0x0005, SYN_ERR_FATAL, 0); /* wraps */

    TEST_ASSERT_TRUE(syn_errlog_count(&elog) == 5);
    TEST_ASSERT_TRUE(syn_errlog_available(&elog) == 4);

    /* Oldest should now be 0x0002 */
    TEST_ASSERT(syn_errlog_read(&elog, 0, &e), "read after wrap");
    TEST_ASSERT_TRUE(e.code == 0x0002);

    /* Disable */
    syn_errlog_enable(&elog, false);
    syn_errlog_record(&elog, 0xFFFF, SYN_ERR_FATAL, 0);
    TEST_ASSERT_TRUE(syn_errlog_count(&elog) == 5);

    /* Clear */
    syn_errlog_enable(&elog, true);
    syn_errlog_clear(&elog);
    TEST_ASSERT_TRUE(syn_errlog_count(&elog) == 0);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 44: Version info                                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_version(void)
{
    printf("Test: Version info\n");

    const SYN_Version *v = syn_version();
    TEST_ASSERT_TRUE(v != NULL);
    TEST_ASSERT_TRUE(v->major == SYN_VERSION_MAJOR);
    TEST_ASSERT_TRUE(v->minor == SYN_VERSION_MINOR);
    TEST_ASSERT_TRUE(v->patch == SYN_VERSION_PATCH);
    TEST_ASSERT_TRUE(v->code == SYN_VERSION_CODE);
    TEST_ASSERT_TRUE(v->date != NULL);
    TEST_ASSERT_TRUE(v->time != NULL);
    TEST_ASSERT_TRUE(v->git_hash != NULL);
    TEST_ASSERT_TRUE(v->app_name != NULL);

    TEST_ASSERT(syn_version_at_least(0, 0, 0), "at_least 0.0.0");
    TEST_ASSERT(syn_version_at_least(0, 1, 0), "at_least 0.1.0");
    TEST_ASSERT(!syn_version_at_least(255, 0, 0), "not at_least 255.0.0");
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 45: Sleep coordinator                                                */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_sleep(void)
{
    printf("Test: Sleep coordinator\n");

    mock_sleep_count = 0;

    SYN_Sleep sl;
    syn_sleep_init(&sl, SYN_SLEEP_LIGHT);

    TEST_ASSERT_TRUE(!syn_sleep_any_locked(&sl));

    /* Sleep should work with no locks */
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));
    TEST_ASSERT_TRUE(mock_sleep_count == 1);

    /* Acquire lock → sleep should be vetoed */
    syn_sleep_lock(&sl, SYN_SLEEP_LOCK_UART);
    TEST_ASSERT_TRUE(syn_sleep_any_locked(&sl));
    TEST_ASSERT(syn_sleep_is_locked(&sl, SYN_SLEEP_LOCK_UART), "UART lock check");

    TEST_ASSERT_TRUE(!syn_sleep_enter(&sl));
    TEST_ASSERT_TRUE(mock_sleep_count == 1);

    /* Release lock → sleep works again */
    syn_sleep_unlock(&sl, SYN_SLEEP_LOCK_UART);
    TEST_ASSERT_TRUE(!syn_sleep_any_locked(&sl));
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));
    TEST_ASSERT_TRUE(mock_sleep_count == 2);

    /* Multiple locks */
    syn_sleep_lock(&sl, SYN_SLEEP_LOCK_UART);
    syn_sleep_lock(&sl, SYN_SLEEP_LOCK_SPI);
    TEST_ASSERT_TRUE(!syn_sleep_enter(&sl));

    syn_sleep_unlock(&sl, SYN_SLEEP_LOCK_UART);
    TEST_ASSERT_TRUE(!syn_sleep_enter(&sl));

    syn_sleep_unlock(&sl, SYN_SLEEP_LOCK_SPI);
    TEST_ASSERT_TRUE(syn_sleep_enter(&sl));

    /* Disable */
    syn_sleep_enable(&sl, false);
    TEST_ASSERT_TRUE(!syn_sleep_enter(&sl));

    /* Stats */
    TEST_ASSERT_TRUE(sl.enter_count == 3);
    TEST_ASSERT_TRUE(sl.veto_count > 0);
    TEST_ASSERT_TRUE(syn_sleep_locks(&sl) == 0);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 46: GPIO interrupt dispatcher (EXTI)                                 */
/* ══════════════════════════════════════════════════════════════════════════ */

static int exti_fire_count = 0;
static SYN_GPIO_Pin exti_last_pin = 255;

static void exti_callback(SYN_GPIO_Pin pin, void *ctx)
{
    (void)ctx;
    exti_fire_count++;
    exti_last_pin = pin;
}

static int exti_ctx_value = 0;
static void exti_callback_ctx(SYN_GPIO_Pin pin, void *ctx)
{
    (void)pin;
    exti_ctx_value = *(int *)ctx;
}

static void test_exti(void)
{
    printf("Test: GPIO interrupt dispatcher\n");

    exti_fire_count = 0;
    exti_last_pin = 255;
    exti_ctx_value = 0;

    syn_exti_init();
    TEST_ASSERT_TRUE(syn_exti_count() == 0);

    /* Register pin 3 */
    SYN_Status st = syn_exti_register(3, SYN_EXTI_FALLING,
                                        exti_callback, NULL);
    TEST_ASSERT_TRUE(st == SYN_OK);
    TEST_ASSERT_TRUE(syn_exti_count() == 1);

    /* Simulate ISR firing */
    syn_exti_irq_handler(3);
    TEST_ASSERT_TRUE(exti_fire_count == 1);
    TEST_ASSERT_TRUE(exti_last_pin == 3);

    /* Fire again */
    syn_exti_irq_handler(3);
    TEST_ASSERT_TRUE(exti_fire_count == 2);

    /* Unregistered pin — no crash */
    syn_exti_irq_handler(99);
    TEST_ASSERT_TRUE(exti_fire_count == 2);

    /* Register with context */
    static int val = 42;
    syn_exti_register(7, SYN_EXTI_RISING, exti_callback_ctx, &val);
    syn_exti_irq_handler(7);
    TEST_ASSERT_TRUE(exti_ctx_value == 42);

    /* Unregister */
    syn_exti_unregister(3);
    syn_exti_irq_handler(3);
    TEST_ASSERT_TRUE(exti_fire_count == 2);

    /* Re-register same pin (update in place) */
    syn_exti_register(3, SYN_EXTI_BOTH, exti_callback, NULL);
    syn_exti_irq_handler(3);
    TEST_ASSERT_TRUE(exti_fire_count == 3);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 47: Typed message mailbox                                            */
/* ══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    uint16_t id;
    int32_t  value;
} TestMsg;

static void test_mailbox(void)
{
    printf("Test: Typed message mailbox\n");

    /* Static definition */
    SYN_MAILBOX_DEFINE(mbox, TestMsg, 4);

    TEST_ASSERT_TRUE(syn_mailbox_empty(&mbox));
    TEST_ASSERT_TRUE(!syn_mailbox_full(&mbox));
    TEST_ASSERT_TRUE(syn_mailbox_pending(&mbox) == 0);
    TEST_ASSERT_TRUE(syn_mailbox_free(&mbox) == 3);

    /* Post messages */
    TestMsg m1 = { .id = 1, .value = 100 };
    TestMsg m2 = { .id = 2, .value = 200 };
    TestMsg m3 = { .id = 3, .value = 300 };

    TEST_ASSERT(syn_mailbox_post(&mbox, &m1), "post 1");
    TEST_ASSERT(syn_mailbox_post(&mbox, &m2), "post 2");
    TEST_ASSERT(syn_mailbox_post(&mbox, &m3), "post 3");
    TEST_ASSERT(!syn_mailbox_post(&mbox, &m1), "post 4 fails (full)");
    TEST_ASSERT_TRUE(syn_mailbox_overflows(&mbox) == 1);

    TEST_ASSERT_TRUE(syn_mailbox_pending(&mbox) == 3);
    TEST_ASSERT_TRUE(syn_mailbox_full(&mbox));

    /* Peek */
    const TestMsg *peek = (const TestMsg *)syn_mailbox_peek(&mbox);
    TEST_ASSERT_TRUE(peek != NULL);
    TEST_ASSERT_TRUE(peek->id == 1);

    /* Receive */
    TestMsg rx;
    TEST_ASSERT(syn_mailbox_receive(&mbox, &rx), "receive 1");
    TEST_ASSERT_TRUE(rx.id == 1 && rx.value == 100);

    TEST_ASSERT(syn_mailbox_receive(&mbox, &rx), "receive 2");
    TEST_ASSERT_TRUE(rx.id == 2 && rx.value == 200);

    TEST_ASSERT(syn_mailbox_receive(&mbox, &rx), "receive 3");
    TEST_ASSERT_TRUE(rx.id == 3 && rx.value == 300);

    TEST_ASSERT(!syn_mailbox_receive(&mbox, &rx), "receive empty");
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mbox));

    /* Runtime init */
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb2;
    syn_mailbox_init(&mb2, buf, sizeof(TestMsg), 4);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb2));

    TestMsg m = { .id = 99, .value = -1 };
    syn_mailbox_post(&mb2, &m);
    TEST_ASSERT_TRUE(syn_mailbox_pending(&mb2) == 1);

    /* Flush */
    syn_mailbox_flush(&mb2);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb2));
}

/* ══════════════════════════════════════════════════════════════════════════ */
/* Test 48: Signal statistics                                                */
/* ══════════════════════════════════════════════════════════════════════════ */

static void test_signal(void)
{
    printf("Test: Signal statistics\n");

    int32_t samples[8];
    SYN_Signal sig;
    syn_signal_init(&sig, samples, 8);

    TEST_ASSERT_TRUE(syn_signal_count(&sig) == 0);
    TEST_ASSERT_TRUE(!syn_signal_full(&sig));
    TEST_ASSERT_TRUE(syn_signal_mean(&sig) == 0);

    /* Push some values: 10, 20, 30, 40, 50 */
    syn_signal_push(&sig, 10);
    syn_signal_push(&sig, 20);
    syn_signal_push(&sig, 30);
    syn_signal_push(&sig, 40);
    syn_signal_push(&sig, 50);

    TEST_ASSERT_TRUE(syn_signal_count(&sig) == 5);
    TEST_ASSERT_TRUE(syn_signal_min(&sig) == 10);
    TEST_ASSERT_TRUE(syn_signal_max(&sig) == 50);
    TEST_ASSERT_TRUE(syn_signal_mean(&sig) == 30);
    TEST_ASSERT_TRUE(syn_signal_peak_to_peak(&sig) == 40);
    TEST_ASSERT_TRUE(syn_signal_latest(&sig) == 50);

    /* Delta (50 - 40 = 10) */
    TEST_ASSERT_TRUE(syn_signal_delta(&sig) == 10);

    /* Access by index */
    TEST_ASSERT(syn_signal_at(&sig, 0) == 10, "at[0]=10 (oldest)");
    TEST_ASSERT(syn_signal_at(&sig, 4) == 50, "at[4]=50 (newest)");

    /* Sum */
    TEST_ASSERT_TRUE(syn_signal_sum(&sig) == 150);

    /* Variance: Var(10,20,30,40,50) = 200.0 → Q16.16 = 200 << 16 */
    int32_t var = syn_signal_variance_q16(&sig);
    /* 200 in Q16.16 = 200 * 65536 = 13107200 */
    TEST_ASSERT_TRUE(var == 13107200);

    /* Fill to capacity (window = 8) then overflow */
    syn_signal_push(&sig, 60);
    syn_signal_push(&sig, 70);
    syn_signal_push(&sig, 80);
    TEST_ASSERT_TRUE(syn_signal_full(&sig));
    TEST_ASSERT_TRUE(syn_signal_count(&sig) == 8);

    /* Push one more → evicts oldest (10) */
    syn_signal_push(&sig, 100);
    TEST_ASSERT_TRUE(syn_signal_count(&sig) == 8);
    TEST_ASSERT_TRUE(syn_signal_min(&sig) == 20);
    TEST_ASSERT_TRUE(syn_signal_max(&sig) == 100);

    /* Clear */
    syn_signal_clear(&sig);
    TEST_ASSERT_TRUE(syn_signal_count(&sig) == 0);
    TEST_ASSERT_TRUE(syn_signal_mean(&sig) == 0);

    /* Negative values */
    syn_signal_push(&sig, -10);
    syn_signal_push(&sig, -20);
    syn_signal_push(&sig, -30);
    TEST_ASSERT_TRUE(syn_signal_min(&sig) == -30);
    TEST_ASSERT_TRUE(syn_signal_max(&sig) == -10);
    TEST_ASSERT_TRUE(syn_signal_mean(&sig) == -20);
}

/* ── New module tests ───────────────────────────────────────────────────── */

#include "tests/test_new_modules.inc.c"

/* ══════════════════════════════════════════════════════════════════════════ */
/* Main                                                                      */
/* ══════════════════════════════════════════════════════════════════════════ */

int main(void)
{
    printf("\n=== SyntropicOS Framework Tests ===\n\n");

    test_basic_protothread();
    test_wait_until();
    test_delay_ms();
    test_semaphore();
    test_event_flags();
    test_scheduler();
    test_software_timer();
    test_suspend_resume();
    test_spawn();
    test_ringbuf();
    test_logging();
    test_cli();
    test_timeout();
    test_button();
    test_led();
    test_fsm();
    test_crc();
    test_filters();
    test_soft_pwm();
    test_pid();
    test_hysteresis();
    test_lut();
    test_servo();
    test_dc_motor();
    test_cobs();
    test_sensor();
    test_watchdog();
    test_param_store();
    test_stepper();
    test_qmath();
    test_rate_limit();
    test_encoder();
    test_trace();
    test_profiler();
    test_sequencer();
    test_adc();
    test_boot();
    test_modbus();
    test_motor_ctrl();
    test_fmt();
    test_pingpong();
    test_workqueue();
    test_errlog();
    test_version();
    test_sleep();
    test_exti();
    test_mailbox();
    test_signal();
    test_ramp();
    test_pack();
    test_power();
    test_actuator();
    test_canvas();
    test_menu();
    test_can();
    test_router();
    test_heartbeat();

    printf("\n=== Results: %d passed, %d failed ===\n\n",
           tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
