/**
 * @file hil_suite.ino
 * @brief Comprehensive Hardware-in-the-Loop (HIL) System Test on AVR & ESP32 targets.
 *
 * Exercises hardware-specific paths on real 8/16-bit AVR and 32-bit RISC-V/Xtensa silicon:
 *   1. Critical section nesting (both from interrupts-enabled and pre-masked contexts)
 *   2. Hardware Timer (2 kHz periodic ISR) preempting cooperative scheduler +
 *      transitive Mutex Priority Inheritance + ISR-to-task RingBuf/Sem/EventFlags/Mailbox
 *   3. Hardware GPIO output readback/toggle + EXTI self-triggered external interrupts +
 *      out-of-bounds pin rejection without slot leakage
 *   4. Crypto, Codec & Fixed-Point KATs (CRC32, SHA-256, multi-block AES-128-GCM
 *      SP 800-38D Case 4, cooperative X25519 RFC 7748 §6.1, PMBus exponent=-16,
 *      LwM2M TLV 64-bit int, Q16.16) + on ESP32: Hardware TRNG + cooperative TLS 1.3 mTLS
 *      P-256 (AES-256-GCM), DTLS 1.3 mTLS Ed25519, and WireGuard Noise_IKpsk2 <= 1000 us slice gate
 *   5. Non-volatile storage wear-leveled & dual-bank failover + bounds check +
 *      real Hardware Watchdog (WDT / TWDT) MCU reset recovery
 */

#include <SyntropicOS.h>
#include <syntropic/storage/syn_settings.h>
#include <syntropic/port/syn_port_wdt.h>
#if defined(__AVR__)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#if !defined(ARDUINO_ARCH_MEGAAVR)
#include <avr/wdt.h>
#endif
#include <avr/eeprom.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <pgmspace.h>
#include <esp_netif.h>
#include <syntropic/port/syn_port_socket.h>
#include <syntropic/util/syn_random.h>
#include <syntropic/crypto/syn_p256.h>
#include <syntropic/crypto/syn_ed25519.h>
#include <syntropic/crypto/syn_x509.h>
#include <syntropic/net/syn_tls.h>
#include <syntropic/net/syn_dtls.h>
#include <syntropic/net/syn_wg.h>
#endif
#include <string.h>

#if defined(__AVR__)
extern char __heap_start;
extern char *__brkval;

static uint16_t get_free_sram(void)
{
    char top;
    return (uint16_t)(&top - (__brkval ? __brkval : &__heap_start));
}
#elif defined(ARDUINO_ARCH_ESP32)
static uint32_t get_free_sram(void)
{
    return ESP.getFreeHeap();
}
#endif

/* ── Persistent State Across Hardware WDT Reboot ────────────────────────── */

#define HIL_EEPROM_ADDR_BANK_A  128U
#if defined(ARDUINO_ARCH_MEGAAVR)
#define HIL_EEPROM_ADDR_BANK_B  192U
#else
#define HIL_EEPROM_ADDR_BANK_B  256U
#endif
#define HIL_MAGIC_WDT_PENDING   0x53594E31UL /* "SYN1" */

#if defined(ARDUINO_ARCH_ESP32)
#define HIL_FLASH_END           (1024UL - 1UL)
#define HIL_WDT_TIMEOUT_MS      50U
#else
#define HIL_FLASH_END           ((uint32_t)E2END)
#define HIL_WDT_TIMEOUT_MS      15U
#endif

typedef struct {
    uint32_t magic;
    uint32_t boot_seq;
    uint16_t kat_crc;
} HilPersistentState;

static const HilPersistentState k_default_state = {0U, 0U, 0U};
static HilPersistentState g_state;
static SYN_Settings g_settings;

#if defined(ARDUINO_ARCH_ESP32)
static bool g_wdt_recovered = false;
static uint32_t g_wdt_rec_seq = 0U;
static uint16_t g_wdt_rec_crc = 0U;
#endif

/* ── Test 1: Critical Section Nesting ───────────────────────────────────── */

#if defined(__AVR__)
static bool test_sreg_critical_sections(void)
{
    /* Case A: Called with interrupts enabled (SEI) */
    interrupts();
    if ((SREG & 0x80U) == 0U) return false;

    syn_port_enter_critical();
    if ((SREG & 0x80U) != 0U) { interrupts(); return false; }

    syn_port_enter_critical();
    if ((SREG & 0x80U) != 0U) { interrupts(); return false; }

    syn_port_exit_critical();
    /* Still at depth 1 -> interrupts MUST remain disabled */
    if ((SREG & 0x80U) != 0U) { interrupts(); return false; }

    syn_port_exit_critical();
    /* Back to depth 0 -> interrupts MUST be restored */
    if ((SREG & 0x80U) == 0U) { interrupts(); return false; }

    /* Case B: Called with interrupts ALREADY disabled (CLI) */
    noInterrupts();
    if ((SREG & 0x80U) != 0U) { interrupts(); return false; }

    syn_port_enter_critical();
    syn_port_enter_critical();
    syn_port_exit_critical();
    syn_port_exit_critical();

    /* Caller had interrupts masked -> exit_critical MUST NOT enable them! */
    bool remained_masked = ((SREG & 0x80U) == 0U);
    interrupts();
    return remained_masked;
}
#elif defined(ARDUINO_ARCH_ESP32)
static bool esp32_irqs_at_level(int expected_level)
{
    int cur = portSET_INTERRUPT_MASK_FROM_ISR();
    portCLEAR_INTERRUPT_MASK_FROM_ISR(cur);
    return (cur == expected_level);
}

static bool test_sreg_critical_sections(void)
{
    interrupts();
    int unmasked = portSET_INTERRUPT_MASK_FROM_ISR();
    portCLEAR_INTERRUPT_MASK_FROM_ISR(unmasked);

    syn_port_enter_critical();
    if (esp32_irqs_at_level(unmasked)) { interrupts(); return false; }

    syn_port_enter_critical();
    if (esp32_irqs_at_level(unmasked)) { interrupts(); return false; }

    syn_port_exit_critical();
    if (esp32_irqs_at_level(unmasked)) { interrupts(); return false; }

    syn_port_exit_critical();
    if (!esp32_irqs_at_level(unmasked)) { interrupts(); return false; }

    /* Case B: Caller already masked interrupts prior to enter_critical */
    int saved = portSET_INTERRUPT_MASK_FROM_ISR();
    if (esp32_irqs_at_level(unmasked)) {
        portCLEAR_INTERRUPT_MASK_FROM_ISR(saved);
        return false;
    }

    syn_port_enter_critical();
    syn_port_enter_critical();
    syn_port_exit_critical();
    syn_port_exit_critical();

    bool remained_masked = !esp32_irqs_at_level(unmasked);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(saved);
    return remained_masked && esp32_irqs_at_level(unmasked);
}
#endif

/* ── Test 2: Hardware Timer ISR + Mutex PI + IPC Stress ─────────────────── */

static SYN_Sched g_sched;
static SYN_Task g_tasks[4];
static SYN_Mutex g_mtx_a;
static SYN_Mutex g_mtx_b;
static SYN_PT_Sem g_isr_sem;
static SYN_EventFlags g_isr_ev;
static SYN_Mailbox g_isr_mbx;
static uint16_t g_mbx_storage[9];
static SYN_RingBuf g_isr_rb;
static uint8_t g_rb_storage[32];

static volatile uint16_t g_timer1_isr_fires = 0U;
static volatile uint16_t g_isr_pushed = 0U;
static uint16_t g_task_popped = 0U;
static uint16_t g_sem_taken = 0U;
static uint16_t g_ev_received = 0U;
static uint32_t g_rb_sum_isr = 0U;
static uint32_t g_rb_sum_task = 0U;

static volatile uint8_t g_pi_stage = 0U;
static bool g_pi_boost_observed_low = false;
static bool g_pi_boost_observed_norm = false;
static bool g_pi_demote_clean = false;

#if defined(ARDUINO_ARCH_ESP32)
static hw_timer_t *g_esp_timer = NULL;
static volatile bool g_isr_in_hw_irq = true;
#endif

static void timer_isr_step(void)
{
    uint16_t n = (uint16_t)(g_timer1_isr_fires + 1U);
    g_timer1_isr_fires = n;
    if (n <= 64U) {
        uint8_t b = (uint8_t)(n & 0xFFU);
        if (syn_ringbuf_write(&g_isr_rb, &b, 1U) == 1U) {
            g_rb_sum_isr += b;
        }
        if (syn_mailbox_post(&g_isr_mbx, &n)) {
            g_isr_pushed = (uint16_t)(g_isr_pushed + 1U);
        }
        PT_SEM_SIGNAL(&g_isr_sem);
        syn_event_flags_set(&g_isr_ev, 0x01U);
    } else {
        /* Stop hardware timer after 64 ISR bursts */
#if defined(ARDUINO_ARCH_MEGAAVR)
        TCB0.CTRLA = 0U;
        TCB0.INTCTRL = 0U;
#elif defined(__AVR__)
        TCCR1B = 0U;
        TIMSK1 = 0U;
#endif
    }
}

#if defined(ARDUINO_ARCH_MEGAAVR)
ISR(TCB0_INT_vect)
{
    TCB0.INTFLAGS = TCB_CAPT_bm;
    timer_isr_step();
}
#elif defined(__AVR__)
ISR(TIMER1_COMPA_vect)
{
    timer_isr_step();
}
#elif defined(ARDUINO_ARCH_ESP32)
static void esp32_gptimer_isr(void)
{
    if (!xPortInIsrContext()) {
        g_isr_in_hw_irq = false;
    }
    if (g_timer1_isr_fires <= 64U) {
        timer_isr_step();
    }
}
#endif

/* LOW priority task: acquires mtx_a, waits for NORMAL and HIGH to contend */
static SYN_PT_Status task_low_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_MUTEX_LOCK(pt, task, &g_mtx_a);
    g_pi_stage = 1U;

    /* Yield while NORMAL locks mtx_b & contends on mtx_a, and HIGH contends on mtx_b */
    while (g_pi_stage < 3U) {
        PT_PASS(pt);
    }

    /* At stage 3, HIGH -> mtx_b (held by NORMAL) -> mtx_a (held by LOW).
     * Both LOW and NORMAL must have inherited SYN_PRIO_HIGH (1)! */
    if (g_tasks[3].priority == SYN_PRIO_HIGH) {
        g_pi_boost_observed_low = true;
    }
    if (g_tasks[2].priority == SYN_PRIO_HIGH) {
        g_pi_boost_observed_norm = true;
    }

    syn_mutex_unlock(&g_mtx_a, task);
    /* LOW must demote back to SYN_PRIO_LOW immediately */
    if (g_tasks[3].priority == SYN_PRIO_LOW) {
        g_pi_demote_clean = true;
    }
    PT_EXIT(pt);
    PT_END(pt);
}

/* NORMAL priority task: acquires mtx_b, then blocks on mtx_a (transitive chain) */
static SYN_PT_Status task_norm_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, g_pi_stage >= 1U);
    PT_MUTEX_LOCK(pt, task, &g_mtx_b);
    g_pi_stage = 2U;

    /* Block on mtx_a held by LOW */
    PT_MUTEX_LOCK(pt, task, &g_mtx_a);
    syn_mutex_unlock(&g_mtx_a, task);
    syn_mutex_unlock(&g_mtx_b, task);
    if (g_tasks[2].priority != SYN_PRIO_NORMAL) {
        g_pi_demote_clean = false;
    }
    g_pi_stage = 4U;
    PT_EXIT(pt);
    PT_END(pt);
}

/* HIGH priority task: blocks on mtx_b held by NORMAL while NORMAL waits on mtx_a */
static SYN_PT_Status task_high_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, g_pi_stage >= 2U);
    g_pi_stage = 3U;
    PT_MUTEX_LOCK(pt, task, &g_mtx_b);
    syn_mutex_unlock(&g_mtx_b, task);
    g_pi_stage = 5U;
    PT_EXIT(pt);
    PT_END(pt);
}

static void drain_isr_queues(void)
{
    uint8_t b;
    while (syn_ringbuf_read(&g_isr_rb, &b, 1U) == 1U) {
        g_rb_sum_task += b;
    }
    uint16_t msg;
    while (syn_mailbox_receive(&g_isr_mbx, &msg)) {
        g_task_popped++;
    }
    while (PT_SEM_TRYWAIT(&g_isr_sem) != 0) {
        g_sem_taken++;
    }
    uint32_t matched = 0U;
    if (syn_event_flags_wait(&g_isr_ev, 0x01U,
                             (uint32_t)SYN_EVENT_FLAGS_WAIT_ANY |
                                 (uint32_t)SYN_EVENT_FLAGS_AUTO_CLEAR,
                             &matched) == SYN_OK) {
        g_ev_received++;
    }
}

/* CRITICAL priority task: drains ISR RingBuf, Mailbox, Sem, EventFlags */
static SYN_PT_Status task_isr_consumer_fn(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    while (g_timer1_isr_fires <= 64U || !syn_mailbox_empty(&g_isr_mbx) ||
           syn_ringbuf_count(&g_isr_rb) > 0U || PT_SEM_COUNT(&g_isr_sem) > 0) {
        drain_isr_queues();
        PT_PASS(pt);
    }
    PT_EXIT(pt);
    PT_END(pt);
}

static bool test_timer1_isr_and_mutex_pi(void)
{
    SYN_TaskProfileManager prof;
    if (syn_task_profile_init(&prof) != SYN_OK) {
        return false;
    }

    g_timer1_isr_fires = 0U;
    g_isr_pushed = 0U;
    g_task_popped = 0U;
    g_sem_taken = 0U;
    g_ev_received = 0U;
    g_rb_sum_isr = 0U;
    g_rb_sum_task = 0U;
    g_pi_stage = 0U;
    g_pi_boost_observed_low = false;
    g_pi_boost_observed_norm = false;
    g_pi_demote_clean = false;

    syn_mutex_init(&g_mtx_a);
    syn_mutex_init(&g_mtx_b);
    PT_SEM_INIT(&g_isr_sem, 0);
    syn_event_flags_init(&g_isr_ev);
    syn_mailbox_init(&g_isr_mbx, g_mbx_storage, sizeof(uint16_t), 9U);
    syn_ringbuf_init(&g_isr_rb, g_rb_storage, sizeof(g_rb_storage));

    syn_task_create(&g_tasks[0], "isr_c", task_isr_consumer_fn, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&g_tasks[1], "high",  task_high_fn,         SYN_PRIO_HIGH,     NULL);
    syn_task_create(&g_tasks[2], "norm",  task_norm_fn,         SYN_PRIO_NORMAL,   NULL);
    syn_task_create(&g_tasks[3], "low",   task_low_fn,          SYN_PRIO_LOW,      NULL);
    syn_sched_init(&g_sched, g_tasks, 4U);
    syn_sched_set_profiler(&g_sched, &prof);

    /* Configure Hardware Timer for 2 kHz periodic ISR */
#if defined(ARDUINO_ARCH_MEGAAVR)
    noInterrupts();
    TCB0.CTRLA = 0U;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;
    TCB0.CNT = 0U;
    TCB0.CCMP = (uint16_t)((F_CPU / 2UL / 2000UL) - 1UL);
    TCB0.INTFLAGS = TCB_CAPT_bm;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm;
    interrupts();
#elif defined(__AVR__)
    noInterrupts();
    TCCR1A = 0U;
    TCCR1B = 0U;
    TCNT1  = 0U;
    OCR1A  = 999U;
    TCCR1B = (1U << WGM12) | (1U << CS11); /* CTC mode, /8 prescaler */
    TIMSK1 = (1U << OCIE1A);
    interrupts();
#elif defined(ARDUINO_ARCH_ESP32)
    g_isr_in_hw_irq = true;
    g_esp_timer = timerBegin(1000000UL);
    if (g_esp_timer == NULL) {
        syn_sched_set_profiler(&g_sched, NULL);
        return false;
    }
    timerAttachInterrupt(g_esp_timer, &esp32_gptimer_isr);
    timerAlarm(g_esp_timer, 500ULL, true, 0ULL);
#endif

    uint32_t t0 = millis();
    while ((millis() - t0) < 500UL) {
        syn_sched_run(&g_sched);
        if (g_pi_stage == 5U && g_timer1_isr_fires > 64U &&
            g_task_popped == g_isr_pushed && syn_ringbuf_count(&g_isr_rb) == 0U) {
            break;
        }
    }

#if defined(ARDUINO_ARCH_MEGAAVR)
    TCB0.CTRLA = 0U;
    TCB0.INTCTRL = 0U;
#elif defined(__AVR__)
    TCCR1B = 0U;
    TIMSK1 = 0U;
#elif defined(ARDUINO_ARCH_ESP32)
    timerDetachInterrupt(g_esp_timer);
    timerEnd(g_esp_timer);
    g_esp_timer = NULL;
#endif
    syn_sched_set_profiler(&g_sched, NULL);

    size_t budget_violations = 1U;
    bool budget_ok =
        (syn_task_profile_check_budgets(&prof, &g_sched, &budget_violations, NULL) == SYN_OK) &&
        (budget_violations == 0U);

    return (g_timer1_isr_fires >= 64U) &&
           (g_isr_pushed > 0U) &&
           (g_task_popped == g_isr_pushed) &&
           (g_rb_sum_isr > 0U) &&
           (g_rb_sum_task == g_rb_sum_isr) &&
           (g_sem_taken == 64U) &&
           (g_ev_received > 0U) &&
           g_pi_boost_observed_low &&
           g_pi_boost_observed_norm &&
           g_pi_demote_clean &&
           (g_pi_stage == 5U) &&
#if defined(ARDUINO_ARCH_ESP32)
           g_isr_in_hw_irq &&
#endif
           budget_ok;
}

/* ── Test 3: Hardware GPIO Toggle + EXTI Self-Loopback Interrupt ────────── */

#if defined(ARDUINO_ARCH_ESP32)
#define HIL_EXTI_PIN 4U
#else
#define HIL_EXTI_PIN 2U
#endif

static volatile uint16_t g_exti_cb_count = 0U;
static volatile SYN_GPIO_Pin g_exti_last_pin = 0xFFU;

static void on_pin2_exti(SYN_GPIO_Pin pin, void *ctx)
{
    g_exti_last_pin = pin;
    volatile uint16_t *cnt = (volatile uint16_t *)ctx;
    *cnt = (uint16_t)(*cnt + 1U);
}

static bool test_hardware_exti_int0(void)
{
#if defined(ARDUINO_ARCH_ESP32)
    /* Regression check: SYN_GPIO_OUTPUT must keep IO_MUX input path active so
     * syn_port_gpio_read() and syn_port_gpio_toggle() observe driven state! */
    if (syn_port_gpio_init(HIL_EXTI_PIN, SYN_GPIO_OUTPUT) != SYN_OK) return false;
    if (syn_port_gpio_write(HIL_EXTI_PIN, SYN_GPIO_LOW) != SYN_OK) return false;
    delayMicroseconds(10);
    if (syn_port_gpio_read(HIL_EXTI_PIN) != SYN_GPIO_LOW) return false;
    if (syn_port_gpio_toggle(HIL_EXTI_PIN) != SYN_OK) return false;
    delayMicroseconds(10);
    if (syn_port_gpio_read(HIL_EXTI_PIN) != SYN_GPIO_HIGH) return false;
    if (syn_port_gpio_toggle(HIL_EXTI_PIN) != SYN_OK) return false;
    delayMicroseconds(10);
    if (syn_port_gpio_read(HIL_EXTI_PIN) != SYN_GPIO_LOW) return false;
#endif

    syn_exti_init();

    /* Probe 6 non-EXTI pins: all must return SYN_NOT_IMPLEMENTED
     * and MUST NOT leak the 6 hardware EXTI trampoline slots! */
#if defined(ARDUINO_ARCH_MEGAAVR) || defined(ARDUINO_ARCH_ESP32)
    const SYN_GPIO_Pin bad_pin_base = (SYN_GPIO_Pin)NUM_DIGITAL_PINS;
#else
    const SYN_GPIO_Pin bad_pin_base = 4U;
#endif
    for (SYN_GPIO_Pin p = bad_pin_base; p < (SYN_GPIO_Pin)(bad_pin_base + 6U); p++) {
        if (syn_port_exti_configure(p, SYN_EXTI_RISING) != SYN_NOT_IMPLEMENTED) {
            return false;
        }
    }

    g_exti_cb_count = 0U;
    if (syn_exti_register(HIL_EXTI_PIN, SYN_EXTI_BOTH, on_pin2_exti,
                          (void *)&g_exti_cb_count) != SYN_OK) {
        return false;
    }
    if (syn_port_exti_configure(HIL_EXTI_PIN, SYN_EXTI_BOTH) != SYN_OK) {
        return false;
    }

    /* Drive pin in software while keeping input path connected to interrupt matrix */
#if defined(ARDUINO_ARCH_ESP32)
    pinMode(HIL_EXTI_PIN, INPUT | OUTPUT);
#else
    pinMode(HIL_EXTI_PIN, OUTPUT);
#endif
    digitalWrite(HIL_EXTI_PIN, HIGH);
    delayMicroseconds(20);
    syn_exti_enable(HIL_EXTI_PIN);
    delayMicroseconds(20);
    uint16_t baseline = g_exti_cb_count;

    for (uint8_t i = 0U; i < 4U; i++) {
        digitalWrite(HIL_EXTI_PIN, LOW);
        delayMicroseconds(20);
        digitalWrite(HIL_EXTI_PIN, HIGH);
        delayMicroseconds(20);
    }

    uint16_t after_toggles = g_exti_cb_count;
    if ((after_toggles - baseline) != 8U || g_exti_last_pin != HIL_EXTI_PIN) {
        syn_exti_disable(HIL_EXTI_PIN);
        return false;
    }

    /* Disable EXTI and confirm no further interrupts fire */
    syn_exti_disable(HIL_EXTI_PIN);
    digitalWrite(HIL_EXTI_PIN, LOW);
    delayMicroseconds(20);
    digitalWrite(HIL_EXTI_PIN, HIGH);
    delayMicroseconds(20);

    return (g_exti_cb_count == after_toggles);
}

/* ── Test 4: Crypto, Codec & Fixed-Point KATs ───────────────────────────── */

static __attribute__((noinline)) bool test_kat_sha256(void)
{
    uint8_t digest[32];
    syn_sha256((const uint8_t *)"abc", 3U, digest);
    return (digest[0] == 0xBAU && digest[1] == 0x78U && digest[2] == 0x16U &&
            digest[3] == 0xBFU && digest[31] == 0xADU);
}

/* NIST SP 800-38D Test Case 4: 60B plaintext (3 full + 12B tail), 20B AAD (1 full + 4B tail) */
static const uint8_t k_gcm_key4[16] PROGMEM = {
    0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
    0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08
};
static const uint8_t k_gcm_iv4[12] PROGMEM = {
    0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88
};
static const uint8_t k_gcm_aad4[20] PROGMEM = {
    0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed,
    0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xab, 0xad, 0xda, 0xd2
};
static const uint8_t k_gcm_pt4[60] PROGMEM = {
    0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5, 0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26,
    0x9a, 0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda, 0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31,
    0x8a, 0x72, 0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53, 0x2f, 0xcf, 0x0e, 0x24, 0x49,
    0xa6, 0xb5, 0x25, 0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57, 0xba, 0x63, 0x7b, 0x39
};
static const uint8_t k_gcm_tag4[16] PROGMEM = {
    0x5b, 0xc9, 0x4f, 0xbc, 0x32, 0x21, 0xa5, 0xdb,
    0x94, 0xfa, 0xe9, 0x5a, 0xe7, 0x12, 0x1a, 0x47
};

static __attribute__((noinline)) bool test_kat_aes_gcm(void)
{
    uint8_t key[16];
    uint8_t iv[12];
    uint8_t aad[20];
    uint8_t pt[60];
    uint8_t ct[60];
    uint8_t dec[60];
    uint8_t tag[16];
    SYN_AES_GCM_Context gcm;

    memcpy_P(key, k_gcm_key4, sizeof(key));
    memcpy_P(iv, k_gcm_iv4, sizeof(iv));
    memcpy_P(aad, k_gcm_aad4, sizeof(aad));
    memcpy_P(pt, k_gcm_pt4, sizeof(pt));

    if (syn_aes_gcm_init(&gcm, key, sizeof(key)) != SYN_OK) {
        return false;
    }
    if (syn_aes_gcm_encrypt(&gcm, iv, sizeof(iv), aad, sizeof(aad),
                            pt, sizeof(pt), ct, tag) != SYN_OK) {
        return false;
    }
    if (memcmp_P(tag, k_gcm_tag4, sizeof(tag)) != 0) {
        return false;
    }
    if (syn_aes_gcm_decrypt(&gcm, iv, sizeof(iv), aad, sizeof(aad),
                            ct, sizeof(ct), dec, tag) != SYN_OK) {
        return false;
    }
    if (memcmp(pt, dec, sizeof(pt)) != 0) {
        return false;
    }

    /* Tamper partial AAD tail, partial ciphertext tail, and tag */
    aad[19] ^= 0x01U;
    if (syn_aes_gcm_decrypt(&gcm, iv, sizeof(iv), aad, sizeof(aad),
                            ct, sizeof(ct), dec, tag) == SYN_OK) {
        return false;
    }
    aad[19] ^= 0x01U;

    ct[59] ^= 0x80U;
    if (syn_aes_gcm_decrypt(&gcm, iv, sizeof(iv), aad, sizeof(aad),
                            ct, sizeof(ct), dec, tag) == SYN_OK) {
        return false;
    }
    ct[59] ^= 0x80U;

    tag[0] ^= 0x01U;
    if (syn_aes_gcm_decrypt(&gcm, iv, sizeof(iv), aad, sizeof(aad),
                            ct, sizeof(ct), dec, tag) == SYN_OK) {
        return false;
    }
    return true;
}

/* X25519 runs on >32 KB AVR targets (Nano Every 48 KB, Mega 256 KB) and all 32-bit targets */
#if !defined(__AVR__) || defined(ARDUINO_ARCH_MEGAAVR) || (defined(FLASHEND) && (FLASHEND > 0x7FFFUL))
#define HIL_HAS_X25519 1
#else
#define HIL_HAS_X25519 0
#endif

#if HIL_HAS_X25519
/* RFC 7748 §6.1 Test Vector 1 */
static const uint8_t k_x25519_scalar[32] PROGMEM = {
    0xa5, 0x46, 0xe3, 0x6b, 0xf0, 0x52, 0x7c, 0x9d,
    0x3b, 0x16, 0x15, 0x4b, 0x82, 0x46, 0x5e, 0xdd,
    0x62, 0x14, 0x4c, 0x0a, 0xc1, 0xfc, 0x5a, 0x18,
    0x50, 0x6a, 0x22, 0x44, 0xba, 0x44, 0x9a, 0xc4
};
static const uint8_t k_x25519_point[32] PROGMEM = {
    0xe6, 0xdb, 0x68, 0x67, 0x58, 0x30, 0x30, 0xdb,
    0x35, 0x94, 0xc1, 0xa4, 0x24, 0xb1, 0x5f, 0x7c,
    0x72, 0x66, 0x24, 0xec, 0x26, 0xb3, 0x35, 0x3b,
    0x10, 0xa9, 0x03, 0xa6, 0xd0, 0xab, 0x1c, 0x4c
};
static const uint8_t k_x25519_expected[32] PROGMEM = {
    0xc3, 0xda, 0x55, 0x37, 0x9d, 0xe9, 0xc6, 0x90,
    0x8e, 0x94, 0xea, 0x4d, 0xf2, 0x8d, 0x08, 0x4f,
    0x32, 0xec, 0xcf, 0x03, 0x49, 0x1c, 0x71, 0xf7,
    0x54, 0xb4, 0x07, 0x55, 0x77, 0xa2, 0x85, 0x52
};

static __attribute__((noinline)) bool init_x25519_kat_ctx(SYN_X25519_Ctx *ctx)
{
    uint8_t scalar[32];
    uint8_t point[32];
    memcpy_P(scalar, k_x25519_scalar, sizeof(scalar));
    memcpy_P(point, k_x25519_point, sizeof(point));
    return syn_x25519_init(ctx, scalar, point);
}

static __attribute__((noinline)) bool test_kat_x25519_pt(uint16_t *out_slices, uint32_t *out_max_us)
{
    SYN_X25519_Ctx ctx;
    SYN_PT pt;
    uint8_t shared[32];
    uint16_t slices = 0U;
    uint32_t max_us = 0UL;

    if (!init_x25519_kat_ctx(&ctx)) {
        return false;
    }
    PT_INIT(&pt);

    for (;;) {
        uint32_t t0 = micros();
        SYN_PT_Status st = syn_x25519_pt(&pt, &ctx, shared);
        uint32_t dt = micros() - t0;
        if (dt > max_us) {
            max_us = dt;
        }
        slices++;
        if (st == PT_ENDED) {
            break;
        }
        if (st != PT_MORE || slices > 600U) {
            return false;
        }
    }

    *out_slices = slices;
    *out_max_us = max_us;

    if (ctx.active || slices != 510U || !syn_x25519_is_nonzero(shared)) {
        return false;
    }
    return (memcmp_P(shared, k_x25519_expected, sizeof(shared)) == 0);
}
#endif

static __attribute__((noinline)) bool test_avr16_kats(uint16_t *out_crc,
                                                      uint16_t *out_x25519_slices,
                                                      uint32_t *out_x25519_max_us)
{
    *out_x25519_slices = 0U;
    *out_x25519_max_us = 0UL;

    /* 1. IEEE 802.3 CRC-32 ("123456789" -> 0xCBF43926) & CRC-16-CCITT (0x29B1) */
    const uint8_t nine[] = "123456789";
    uint32_t crc32 = syn_crc32(nine, 9U);
    uint16_t crc16 = syn_crc16_ccitt(nine, 9U);
    if (crc32 != 0xCBF43926UL || crc16 != 0x29B1U) {
        return false;
    }
    *out_crc = crc16;

    /* 2. FIPS 180-4 SHA-256 ("abc" -> ba7816bf...) */
    if (!test_kat_sha256()) {
        return false;
    }

    /* 3. Multi-block + partial-tail AES-128-GCM (NIST SP 800-38D Case 4) + Tamper Reject */
    if (!test_kat_aes_gcm()) {
        return false;
    }

#if HIL_HAS_X25519
    /* 4. Cooperative X25519 scalar mult (RFC 7748 §6.1) */
    if (!test_kat_x25519_pt(out_x25519_slices, out_x25519_max_us)) {
        return false;
    }
#endif

    /* 5. PMBus Linear16 & Linear11 at exponent = -16 (vout_mode = 0x10, 16-bit shift boundary) */
    float vout = syn_pmbus_linear16_to_float(65535U, 0x10U);
    uint16_t raw16 = syn_pmbus_float_to_linear16(0.5f, 0x10U);
    if (vout < 0.999f || vout > 1.001f || raw16 != 32768U) {
        return false;
    }

    /* 6. LwM2M 16-bit TLV & 64-bit Int Codec */
    uint8_t tlv_buf[12];
    size_t tlv_len = syn_lwm2m_tlv_encode_int(1U, 0x0102030405LL, tlv_buf, sizeof(tlv_buf));
    SYN_LwM2M_TLV tlv;
    size_t consumed = 0U;
    int64_t parsed64 = 0;
    if (tlv_len == 0U || !syn_lwm2m_tlv_decode(tlv_buf, tlv_len, &tlv, &consumed) ||
        consumed != tlv_len || !syn_lwm2m_tlv_decode_int(&tlv, &parsed64) ||
        parsed64 != 0x0102030405LL) {
        return false;
    }

    /* 7. Fixed-Point Q16.16 Math */
    int32_t a = Q16_FROM_INT(12);
    int32_t b = Q16_FROM_INT(3);
    if (q16_mul(a, b) != Q16_FROM_INT(36) ||
        q16_div(a, b) != Q16_FROM_INT(4)) {
        return false;
    }

    return true;
}

/* ── Test 4c (ESP32): Hardware TRNG + Cooperative mTLS/DTLS/WG Budget Gate ── */

#if defined(ARDUINO_ARCH_ESP32)

typedef struct {
    uint8_t buf[2560];
    size_t len;
    size_t dgram_off[8];
    size_t dgram_len[8];
    uint8_t dgram_head;
    uint8_t dgram_count;
} HilMemChannel;

typedef struct {
    HilMemChannel *tx;
    HilMemChannel *rx;
    bool datagram;
} HilEndpointPair;

static HilMemChannel s_c2s;
static HilMemChannel s_s2c;
static HilEndpointPair s_cli_ep;
static HilEndpointPair s_srv_ep;
static SYN_Transport s_cli_tr;
static SYN_Transport s_srv_tr;

static uint8_t s_c_rx[2560];
static uint8_t s_c_tx[2560];
static uint8_t s_s_rx[2560];
static uint8_t s_s_tx[2560];

static union {
    struct {
        SYN_TLS_Context cli;
        SYN_TLS_Context srv;
    } tls;
    struct {
        SYN_DTLS_Context cli;
        SYN_DTLS_Context srv;
    } dtls;
    SYN_WG wg;
} s_proto;

static SYN_X509_Cert s_root_ca;
static uint8_t s_srv_cert[256];
static uint8_t s_cli_cert[256];
static uint8_t s_ca_pub[65];
static uint8_t s_ca_sec[32];
static uint8_t s_srv_pub[65];
static uint8_t s_srv_sec[32];
static uint8_t s_cli_pub[65];
static uint8_t s_cli_sec[32];
static uint8_t s_app_tx[512];
static uint8_t s_app_rx[512];

static bool hil_mem_send_stream(const uint8_t *data, size_t len, void *ctx)
{
    HilEndpointPair *ep = (HilEndpointPair *)ctx;
    if (ep->tx->len + len > sizeof(ep->tx->buf)) {
        return false;
    }
    memcpy(ep->tx->buf + ep->tx->len, data, len);
    ep->tx->len += len;
    return true;
}

static bool hil_mem_send_dgram(const uint8_t *data, size_t len, void *ctx)
{
    HilEndpointPair *ep = (HilEndpointPair *)ctx;
    HilMemChannel *ch = ep->tx;
    if (ch->dgram_count >= 8U) {
        return false;
    }
    if (ch->dgram_count == 0U) {
        ch->len = 0U;
        ch->dgram_head = 0U;
    }
    if (ch->len + len > sizeof(ch->buf)) {
        return false;
    }
    uint8_t slot = (uint8_t)((ch->dgram_head + ch->dgram_count) & 7U);
    ch->dgram_off[slot] = ch->len;
    ch->dgram_len[slot] = len;
    memcpy(ch->buf + ch->len, data, len);
    ch->len += len;
    ch->dgram_count++;
    return true;
}

static bool hil_mem_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    HilEndpointPair *ep = (HilEndpointPair *)ctx;
    HilMemChannel *ch = ep->rx;
    if (ep->datagram) {
        if (ch->dgram_count == 0U) {
            return false;
        }
        size_t off = ch->dgram_off[ch->dgram_head];
        size_t pkt_len = ch->dgram_len[ch->dgram_head];
        size_t n = (pkt_len > max_len) ? max_len : pkt_len;
        memcpy(data, ch->buf + off, n);
        *out_len = n;
        ch->dgram_head = (uint8_t)((ch->dgram_head + 1U) & 7U);
        ch->dgram_count--;
        if (ch->dgram_count == 0U) {
            ch->len = 0U;
            ch->dgram_head = 0U;
        }
        return true;
    }
    if (ch->len == 0U) {
        return false;
    }
    size_t n = (ch->len > max_len) ? max_len : ch->len;
    memcpy(data, ch->buf, n);
    *out_len = n;
    ch->len = 0U;
    return true;
}

static void hil_init_mem_transports(bool datagram)
{
    memset(&s_c2s, 0, sizeof(s_c2s));
    memset(&s_s2c, 0, sizeof(s_s2c));
    s_cli_ep.tx = &s_c2s;
    s_cli_ep.rx = &s_s2c;
    s_cli_ep.datagram = datagram;
    s_srv_ep.tx = &s_s2c;
    s_srv_ep.rx = &s_c2s;
    s_srv_ep.datagram = datagram;
    s_cli_tr.send = datagram ? hil_mem_send_dgram : hil_mem_send_stream;
    s_cli_tr.recv = hil_mem_recv;
    s_cli_tr.has_data = NULL;
    s_cli_tr.ctx = &s_cli_ep;
    s_srv_tr.send = datagram ? hil_mem_send_dgram : hil_mem_send_stream;
    s_srv_tr.recv = hil_mem_recv;
    s_srv_tr.has_data = NULL;
    s_srv_tr.ctx = &s_srv_ep;
}

static bool hil_build_ed25519_cert(const uint8_t leaf_pub[32], const uint8_t ca_sec[32],
                                   const uint8_t ca_pub[32], uint8_t out_der[184])
{
    static const uint8_t hdr[78] = {
        0x30, 0x81, 0xB5, 0x30, 0x69, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01,
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x30, 0x12, 0x31, 0x10, 0x30, 0x0E,
        0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x07, 'T',  'e',  's',  't',  ' ',  'C',
        'A',  0x30, 0x00, 0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04,
        0x03, 0x13, 0x0B, 'T',  'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',
        'r',  0x30, 0x2A, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x21, 0x00};
    static const uint8_t sig_hdr[10] = {0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x41, 0x00};

    memcpy(out_der, hdr, sizeof(hdr));
    memcpy(out_der + 78U, leaf_pub, 32U);
    memcpy(out_der + 110U, sig_hdr, sizeof(sig_hdr));
    return syn_ed25519_sign(out_der + 3U, 107U, ca_sec, ca_pub, out_der + 120U);
}

static size_t hil_build_p256_cert(const uint8_t leaf_pub65[65], const uint8_t ca_priv[32],
                                  uint8_t out_der[256])
{
    static const uint8_t tbs_prefix[66] = {
        0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86,
        0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03,
        0x55, 0x04, 0x03, 0x13, 0x07, 'T',  'e',  's',  't',  ' ',  'C',  'A',  0x30, 0x00,
        0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0B, 'T',
        'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r'};
    static const uint8_t spki_alg[21] = {0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48,
                                         0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
                                         0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07};
    static const uint8_t sig_hdr[15] = {0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE,
                                        0x3D, 0x04, 0x03, 0x02, 0x03, 0x41, 0x00};

    size_t spki_body = sizeof(spki_alg) + 3U + 65U;
    size_t tbs_body = sizeof(tbs_prefix) + 2U + spki_body;
    size_t tbs_total = 3U + tbs_body;
    size_t outer_body = tbs_total + sizeof(sig_hdr) + 64U;

    size_t pos = 0U;
    out_der[pos++] = 0x30U;
    out_der[pos++] = 0x81U;
    out_der[pos++] = (uint8_t)outer_body;

    size_t tbs_start = pos;
    out_der[pos++] = 0x30U;
    out_der[pos++] = 0x81U;
    out_der[pos++] = (uint8_t)tbs_body;
    memcpy(out_der + pos, tbs_prefix, sizeof(tbs_prefix));
    pos += sizeof(tbs_prefix);
    out_der[pos++] = 0x30U;
    out_der[pos++] = (uint8_t)spki_body;
    memcpy(out_der + pos, spki_alg, sizeof(spki_alg));
    pos += sizeof(spki_alg);
    out_der[pos++] = 0x03U;
    out_der[pos++] = 66U;
    out_der[pos++] = 0x00U;
    memcpy(out_der + pos, leaf_pub65, 65U);
    pos += 65U;

    uint8_t tbs_hash[32];
    syn_sha256(out_der + tbs_start, tbs_total, tbs_hash);
    memcpy(out_der + pos, sig_hdr, sizeof(sig_hdr));
    pos += sizeof(sig_hdr);
    if (!syn_p256_sign_ecdsa_deterministic(ca_priv, tbs_hash, out_der + pos, out_der + pos + 32U)) {
        return 0U;
    }
    return pos + 64U;
}

static SYN_PT_Status hil_crit_sentinel_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        PT_PASS(pt);
    }
    PT_END(pt);
}

static SYN_PT_Status hil_contend_peer_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static bool hil_check_budget(SYN_TaskProfileManager *prof, SYN_Sched *sched, uint32_t *out_max_us)
{
    uint32_t max_us = 0U;
    for (size_t i = 1U; i < sched->task_count && i < prof->count; i++) {
        const char *tname = prof->profiles[i].task_name;
        if (tname != NULL && strcmp(tname, "contend_peer") == 0) {
            continue;
        }
        if (prof->profiles[i].max_yield_us > max_us) {
            max_us = prof->profiles[i].max_yield_us;
        }
    }
    *out_max_us = max_us;

    size_t violations = 1U;
    return (syn_task_profile_check_budgets(prof, sched, &violations, NULL) == SYN_OK &&
            violations == 0U);
}

static void hil_wg_mix_hash(uint8_t h[32], const uint8_t *data, size_t len)
{
    SYN_BLAKE2s bctx;
    syn_blake2s_init(&bctx, 32U);
    syn_blake2s_update(&bctx, h, 32U);
    syn_blake2s_update(&bctx, data, len);
    syn_blake2s_final(&bctx, h);
}

static void hil_wg_hkdf(const uint8_t ck[32], const uint8_t *in, size_t in_len, uint8_t out1[32],
                        uint8_t out2[32], uint8_t out3[32])
{
    uint8_t prk[32];
    uint8_t buf[33];
    uint8_t b1 = 0x01U;
    syn_hmac_blake2s(ck, 32U, in, in_len, prk);
    syn_hmac_blake2s(prk, 32U, &b1, 1U, out1);
    if (out2 != NULL) {
        memcpy(buf, out1, 32U);
        buf[32] = 0x02U;
        syn_hmac_blake2s(prk, 32U, buf, 33U, out2);
    }
    if (out3 != NULL && out2 != NULL) {
        memcpy(buf, out2, 32U);
        buf[32] = 0x03U;
        syn_hmac_blake2s(prk, 32U, buf, 33U, out3);
    }
}

static bool hil_wg_build_server_response(const SYN_WgConfig *cfg, const uint8_t resp_s_priv[32],
                                         const uint8_t init_pkt[148], uint8_t out_resp[92])
{
    static const char k_cons[] = "Noise_IKpsk2_25519_ChaChaPoly_BLAKE2s";
    static const char k_id[] = "WireGuard v1 zx2c4 Jason@zx2c4.com";
    static const char k_mac1[] = "mac1----";
    uint8_t ck[32], h[32], k[32], dh[32], tau[32], dec_pub[32], dec_ts[12];
    uint8_t e_priv_r[32], e_pub_r[32], mac_key[32];
    uint8_t zero_nonce[12] = {0};

    syn_blake2s((const uint8_t *)k_cons, sizeof(k_cons) - 1U, ck, 32U);
    memcpy(h, ck, 32U);
    hil_wg_mix_hash(h, (const uint8_t *)k_id, sizeof(k_id) - 1U);
    hil_wg_mix_hash(h, cfg->peer_public_key, 32U);

    const uint8_t *e_pub_i = init_pkt + 8U;
    hil_wg_hkdf(ck, e_pub_i, 32U, ck, NULL, NULL);
    hil_wg_mix_hash(h, e_pub_i, 32U);

    syn_x25519(dh, resp_s_priv, e_pub_i);
    hil_wg_hkdf(ck, dh, 32U, ck, k, NULL);
    if (!syn_aead_decrypt(k, zero_nonce, h, 32U, init_pkt + 40U, 32U, init_pkt + 72U, dec_pub)) {
        return false;
    }
    hil_wg_mix_hash(h, init_pkt + 40U, 48U);

    syn_x25519(dh, resp_s_priv, dec_pub);
    hil_wg_hkdf(ck, dh, 32U, ck, k, NULL);
    if (!syn_aead_decrypt(k, zero_nonce, h, 32U, init_pkt + 88U, 12U, init_pkt + 100U, dec_ts)) {
        return false;
    }
    hil_wg_mix_hash(h, init_pkt + 88U, 28U);

    memset(out_resp, 0, 92U);
    out_resp[0] = SYN_WG_MSG_RESPONSE;
    out_resp[4] = 0x78U;
    out_resp[5] = 0x56U;
    memcpy(out_resp + 8U, init_pkt + 4U, 4U);

    memset(e_priv_r, 0x33, 32U);
    syn_x25519_clamp(e_priv_r);
    syn_x25519_pubkey(e_pub_r, e_priv_r);
    memcpy(out_resp + 12U, e_pub_r, 32U);

    hil_wg_hkdf(ck, e_pub_r, 32U, ck, NULL, NULL);
    hil_wg_mix_hash(h, e_pub_r, 32U);
    syn_x25519(dh, e_priv_r, e_pub_i);
    hil_wg_hkdf(ck, dh, 32U, ck, NULL, NULL);
    syn_x25519(dh, e_priv_r, dec_pub);
    hil_wg_hkdf(ck, dh, 32U, ck, NULL, NULL);
    hil_wg_hkdf(ck, cfg->preshared_key, 32U, ck, tau, k);
    hil_wg_mix_hash(h, tau, 32U);
    syn_aead_encrypt(k, zero_nonce, h, 32U, NULL, 0U, NULL, out_resp + 44U);

    SYN_BLAKE2s mctx;
    syn_blake2s_init(&mctx, 32U);
    syn_blake2s_update(&mctx, (const uint8_t *)k_mac1, 8U);
    syn_blake2s_update(&mctx, dec_pub, 32U);
    syn_blake2s_final(&mctx, mac_key);
    syn_blake2s_mac(mac_key, 32U, out_resp, 60U, out_resp + 60U, 16U);
    return true;
}

static bool test_esp32_coop_protocol_budgets(uint32_t *out_tls_us, uint32_t *out_dtls_us,
                                             uint32_t *out_wg_us)
{
    *out_tls_us = 0U;
    *out_dtls_us = 0U;
    *out_wg_us = 0U;

    /* 1. Hardware TRNG verification (distinct non-zero entropy, not XOR-shift fallback) */
    uint8_t r1[32], r2[32], fb[32], zero32[32];
    memset(zero32, 0, sizeof(zero32));
    if (syn_random_fill(r1, sizeof(r1)) != SYN_OK || syn_random_fill(r2, sizeof(r2)) != SYN_OK) {
        return false;
    }
    (void)syn_random_fallback_fill(fb, sizeof(fb));
    if (memcmp(r1, zero32, sizeof(r1)) == 0 || memcmp(r2, zero32, sizeof(r2)) == 0 ||
        memcmp(r1, r2, sizeof(r1)) == 0 || memcmp(r1, fb, sizeof(r1)) == 0) {
        return false;
    }

    /* 2. Cooperative TLS 1.3 mTLS ECDSA P-256 + AES-256-GCM-SHA384 + 512B app data round-trip */
    static const uint8_t ca_priv[32] = {0xC9, 0xAF, 0xA9, 0xD8, 0x45, 0xBA, 0x75, 0x16,
                                        0x6B, 0x5C, 0x21, 0x57, 0x67, 0xB1, 0xD6, 0x93,
                                        0x4E, 0x50, 0xC3, 0xDB, 0x36, 0xE8, 0x9B, 0x12,
                                        0x7B, 0x8A, 0x62, 0x2B, 0x12, 0x0F, 0x67, 0x21};
    static const uint8_t srv_priv[32] = {0x7D, 0x40, 0xE0, 0x7E, 0x74, 0xF4, 0x9A, 0x71,
                                         0x48, 0xC3, 0x32, 0x10, 0xCA, 0x19, 0xBD, 0x9B,
                                         0x9D, 0x04, 0x50, 0x77, 0x92, 0xEC, 0xF7, 0x3E,
                                         0x87, 0x7E, 0x10, 0x64, 0x79, 0x80, 0x0C, 0x85};
    static const uint8_t cli_priv[32] = {0x3B, 0x91, 0x8F, 0xC2, 0x3A, 0xA4, 0x77, 0xD8,
                                         0xDE, 0x9B, 0x28, 0x12, 0xF3, 0xBE, 0x60, 0xCE,
                                         0x8B, 0x7E, 0x45, 0x26, 0xA3, 0x81, 0x25, 0x60,
                                         0xB9, 0x92, 0x21, 0x3F, 0x19, 0x33, 0xAE, 0x71};

    memcpy(s_ca_sec, ca_priv, 32U);
    memcpy(s_srv_sec, srv_priv, 32U);
    memcpy(s_cli_sec, cli_priv, 32U);
    s_ca_pub[0] = 0x04U;
    s_srv_pub[0] = 0x04U;
    s_cli_pub[0] = 0x04U;
    if (!syn_p256_base_mul(s_ca_sec, s_ca_pub + 1U, s_ca_pub + 33U) ||
        !syn_p256_base_mul(s_srv_sec, s_srv_pub + 1U, s_srv_pub + 33U) ||
        !syn_p256_base_mul(s_cli_sec, s_cli_pub + 1U, s_cli_pub + 33U)) {
        return false;
    }
    size_t srv_cert_len = hil_build_p256_cert(s_srv_pub, s_ca_sec, s_srv_cert);
    size_t cli_cert_len = hil_build_p256_cert(s_cli_pub, s_ca_sec, s_cli_cert);
    if (srv_cert_len == 0U || cli_cert_len == 0U) {
        return false;
    }

    memset(&s_root_ca, 0, sizeof(s_root_ca));
    memcpy(s_root_ca.pubkey, s_ca_pub, 65U);
    s_root_ca.pubkey_len = 65U;
    s_root_ca.pubkey_algo = SYN_X509_ALGO_ECDSA_P256;
    s_root_ca.is_ca = true;
    strcpy(s_root_ca.subject_cn, "Test CA");

    hil_init_mem_transports(false);
    SYN_TLS_Config c_cfg;
    SYN_TLS_Config s_cfg;
    memset(&c_cfg, 0, sizeof(c_cfg));
    memset(&s_cfg, 0, sizeof(s_cfg));
    c_cfg.mode = SYN_TLS_AUTH_MODE_MTLS;
    c_cfg.cipher_suite = SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384;
    c_cfg.server_name = "Test Server";
    c_cfg.root_ca = &s_root_ca;
    c_cfg.peer_cert_der = s_srv_cert;
    c_cfg.peer_cert_len = srv_cert_len;
    c_cfg.client_cert_der = s_cli_cert;
    c_cfg.client_cert_len = cli_cert_len;
    c_cfg.client_privkey = s_cli_sec;

    s_cfg.mode = SYN_TLS_AUTH_MODE_MTLS;
    s_cfg.cipher_suite = SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384;
    s_cfg.is_server = true;
    s_cfg.root_ca = &s_root_ca;
    s_cfg.peer_cert_der = s_cli_cert;
    s_cfg.peer_cert_len = cli_cert_len;
    s_cfg.client_cert_der = s_srv_cert;
    s_cfg.client_cert_len = srv_cert_len;
    s_cfg.client_privkey = s_srv_sec;

    memset(&s_proto, 0, sizeof(s_proto));
    if (!syn_tls_init(&s_proto.tls.cli, &c_cfg, &s_cli_tr, s_c_rx, sizeof(s_c_rx), s_c_tx,
                      sizeof(s_c_tx)) ||
        !syn_tls_init(&s_proto.tls.srv, &s_cfg, &s_srv_tr, s_s_rx, sizeof(s_s_rx), s_s_tx,
                      sizeof(s_s_tx))) {
        return false;
    }

    SYN_TaskProfileManager prof;
    if (syn_task_profile_init(&prof) != SYN_OK) {
        return false;
    }
    syn_task_create(&g_tasks[0], "crit_ctrl", hil_crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&g_tasks[1], "tls_cli_p256", syn_tls_task, SYN_PRIO_NORMAL, &s_proto.tls.cli);
    syn_task_create(&g_tasks[2], "tls_srv_p256", syn_tls_task, SYN_PRIO_NORMAL, &s_proto.tls.srv);
    syn_task_create(&g_tasks[3], "contend_peer", hil_contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&g_sched, g_tasks, 4U);
    syn_sched_set_profiler(&g_sched, &prof);

    uint32_t t0_ms = millis();
    while ((uint32_t)(millis() - t0_ms) < 6000UL &&
           (s_proto.tls.cli.state != SYN_TLS_STATE_ESTABLISHED ||
            s_proto.tls.srv.state != SYN_TLS_STATE_ESTABLISHED)) {
        (void)syn_sched_run(&g_sched);
    }
    if (s_proto.tls.cli.state != SYN_TLS_STATE_ESTABLISHED ||
        s_proto.tls.srv.state != SYN_TLS_STATE_ESTABLISHED) {
        syn_sched_set_profiler(&g_sched, NULL);
        return false;
    }

    /* Send 512B application payload over bitsliced AES-256-GCM + Karatsuba GHASH */
    for (size_t i = 0U; i < sizeof(s_app_tx); i++) {
        s_app_tx[i] = (uint8_t)(i ^ 0xA5U);
    }
    memset(s_app_rx, 0, sizeof(s_app_rx));
    if (!syn_tls_send(&s_proto.tls.cli, s_app_tx, sizeof(s_app_tx))) {
        syn_sched_set_profiler(&g_sched, NULL);
        return false;
    }
    for (uint8_t tick = 0U; tick < 16U; tick++) {
        (void)syn_sched_run(&g_sched);
    }
    size_t rx_got = 0U;
    if (!syn_tls_recv(&s_proto.tls.srv, s_app_rx, sizeof(s_app_rx), &rx_got) ||
        rx_got != sizeof(s_app_tx) || memcmp(s_app_tx, s_app_rx, sizeof(s_app_tx)) != 0) {
        syn_sched_set_profiler(&g_sched, NULL);
        return false;
    }
    syn_sched_set_profiler(&g_sched, NULL);
    if (!hil_check_budget(&prof, &g_sched, out_tls_us)) {
        return false;
    }

    /* 3. Cooperative DTLS 1.3 mTLS Ed25519 + ChaCha20-Poly1305 datagram round-trip */
    static const uint8_t ed_ca_seed[32] = {0x11U, 0x22U, 0x33U, 0x44U};
    static const uint8_t ed_srv_seed[32] = {0x51U, 0x52U, 0x53U, 0x54U};
    static const uint8_t ed_cli_seed[32] = {0x61U, 0x62U, 0x63U, 0x64U};

    if (!syn_ed25519_create_keypair(s_ca_pub, s_ca_sec, ed_ca_seed) ||
        !syn_ed25519_create_keypair(s_srv_pub, s_srv_sec, ed_srv_seed) ||
        !syn_ed25519_create_keypair(s_cli_pub, s_cli_sec, ed_cli_seed) ||
        !hil_build_ed25519_cert(s_srv_pub, s_ca_sec, s_ca_pub, s_srv_cert) ||
        !hil_build_ed25519_cert(s_cli_pub, s_ca_sec, s_ca_pub, s_cli_cert)) {
        return false;
    }

    memset(&s_root_ca, 0, sizeof(s_root_ca));
    memcpy(s_root_ca.pubkey, s_ca_pub, 32U);
    s_root_ca.pubkey_len = 32U;
    s_root_ca.pubkey_algo = SYN_X509_ALGO_ED25519;
    s_root_ca.is_ca = true;
    strcpy(s_root_ca.subject_cn, "Test CA");

    hil_init_mem_transports(true);
    SYN_DTLS_Config dc_cfg;
    SYN_DTLS_Config ds_cfg;
    memset(&dc_cfg, 0, sizeof(dc_cfg));
    memset(&ds_cfg, 0, sizeof(ds_cfg));
    dc_cfg.mode = SYN_DTLS_AUTH_MODE_MTLS;
    dc_cfg.server_name = "Test Server";
    dc_cfg.root_ca = &s_root_ca;
    dc_cfg.peer_cert_der = s_srv_cert;
    dc_cfg.peer_cert_len = 184U;
    dc_cfg.client_cert_der = s_cli_cert;
    dc_cfg.client_cert_len = 184U;
    dc_cfg.client_privkey = s_cli_sec;

    ds_cfg.mode = SYN_DTLS_AUTH_MODE_MTLS;
    ds_cfg.is_server = true;
    ds_cfg.root_ca = &s_root_ca;
    ds_cfg.peer_cert_der = s_cli_cert;
    ds_cfg.peer_cert_len = 184U;
    ds_cfg.client_cert_der = s_srv_cert;
    ds_cfg.client_cert_len = 184U;
    ds_cfg.client_privkey = s_srv_sec;

    memset(&s_proto, 0, sizeof(s_proto));
    if (!syn_dtls_init(&s_proto.dtls.cli, &dc_cfg, &s_cli_tr, s_c_rx, sizeof(s_c_rx), s_c_tx,
                       sizeof(s_c_tx)) ||
        !syn_dtls_init(&s_proto.dtls.srv, &ds_cfg, &s_srv_tr, s_s_rx, sizeof(s_s_rx), s_s_tx,
                       sizeof(s_s_tx))) {
        return false;
    }

    (void)syn_task_profile_init(&prof);
    syn_task_create(&g_tasks[0], "crit_ctrl", hil_crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&g_tasks[1], "dtls_cli_ed", syn_dtls_task, SYN_PRIO_NORMAL, &s_proto.dtls.cli);
    syn_task_create(&g_tasks[2], "dtls_srv_ed", syn_dtls_task, SYN_PRIO_NORMAL, &s_proto.dtls.srv);
    syn_task_create(&g_tasks[3], "contend_peer", hil_contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&g_sched, g_tasks, 4U);
    syn_sched_set_profiler(&g_sched, &prof);

    t0_ms = millis();
    while ((uint32_t)(millis() - t0_ms) < 6000UL &&
           (s_proto.dtls.cli.state != SYN_DTLS_STATE_ESTABLISHED ||
            s_proto.dtls.srv.state != SYN_DTLS_STATE_ESTABLISHED)) {
        (void)syn_sched_run(&g_sched);
    }
    if (s_proto.dtls.cli.state != SYN_DTLS_STATE_ESTABLISHED ||
        s_proto.dtls.srv.state != SYN_DTLS_STATE_ESTABLISHED) {
        syn_sched_set_profiler(&g_sched, NULL);
        return false;
    }

    memset(s_app_rx, 0, 64U);
    if (!syn_dtls_send(&s_proto.dtls.cli, s_app_tx, 64U) ||
        !syn_dtls_recv(&s_proto.dtls.srv, s_app_rx, sizeof(s_app_rx), &rx_got) ||
        rx_got != 64U || memcmp(s_app_tx, s_app_rx, 64U) != 0) {
        syn_sched_set_profiler(&g_sched, NULL);
        return false;
    }
    syn_sched_set_profiler(&g_sched, NULL);
    bool ok_dtls = hil_check_budget(&prof, &g_sched, out_dtls_us);

    /* 4. Cooperative WireGuard Noise_IKpsk2 Handshake + Tunnel Packet over real lwIP UDP */
    (void)esp_netif_init();
    SYN_Socket wg_peer = syn_port_udp_open(15182U);
    if (wg_peer == SYN_SOCKET_INVALID) {
        return false;
    }

    SYN_SNTP wg_sntp;
    SYN_WgConfig wg_cfg;
    uint8_t wg_srv_priv[32];
    uint8_t wg_rx_pkt[256];
    uint8_t wg_resp[92];
    SYN_SockAddr wg_cli_addr;
    memset(&wg_sntp, 0, sizeof(wg_sntp));
    wg_sntp.synced = true;
    wg_sntp.epoch_s = 1700001000U;

    memset(wg_srv_priv, 0x22, sizeof(wg_srv_priv));
    syn_x25519_clamp(wg_srv_priv);

    memset(&wg_cfg, 0, sizeof(wg_cfg));
    memset(wg_cfg.private_key, 0x21, 32U);
    syn_x25519_clamp(wg_cfg.private_key);
    syn_x25519_pubkey(wg_cfg.peer_public_key, wg_srv_priv);
    memset(wg_cfg.preshared_key, 0x29, 32U);
    wg_cfg.endpoint.ip[0] = 127U;
    wg_cfg.endpoint.ip[3] = 1U;
    wg_cfg.endpoint.port = 15182U;

    memset(&s_proto, 0, sizeof(s_proto));
    syn_wg_init(&s_proto.wg, &wg_cfg, &wg_sntp, s_c_rx, 512U, s_c_tx, 512U);

    (void)syn_task_profile_init(&prof);
    /* Real lwIP UDP sockets preempt into ESP-IDF's prio-18 tcpip_thread (SYN_PRIO_HIGH = 5ms I/O budget) */
    syn_task_create(&g_tasks[0], "io_ctrl", hil_crit_sentinel_task, SYN_PRIO_HIGH, NULL);
    syn_task_create(&g_tasks[1], "wg_vpn", syn_wg_task, SYN_PRIO_NORMAL, &s_proto.wg);
    syn_task_create(&g_tasks[2], "contend_peer", hil_contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&g_sched, g_tasks, 3U);
    syn_sched_set_profiler(&g_sched, &prof);

    bool resp_injected = false;
    t0_ms = millis();
    while ((uint32_t)(millis() - t0_ms) < 6000UL && s_proto.wg.state != SYN_WG_ESTABLISHED) {
        (void)syn_sched_run(&g_sched);
        if (s_proto.wg.state == SYN_WG_HANDSHAKE_INIT && !resp_injected) {
            int got = syn_port_udp_recvfrom(wg_peer, wg_rx_pkt, sizeof(wg_rx_pkt), &wg_cli_addr, 0U);
            if (got == 148) {
                if (!hil_wg_build_server_response(&wg_cfg, wg_srv_priv, wg_rx_pkt, wg_resp) ||
                    syn_port_udp_sendto(wg_peer, wg_resp, sizeof(wg_resp), &wg_cli_addr) !=
                        (int)sizeof(wg_resp)) {
                    syn_sched_set_profiler(&g_sched, NULL);
                    syn_wg_disconnect(&s_proto.wg);
                    syn_port_sock_close(wg_peer);
                    return false;
                }
                resp_injected = true;
            }
        }
    }
    syn_sched_set_profiler(&g_sched, NULL);
    if (s_proto.wg.state != SYN_WG_ESTABLISHED) {
        syn_wg_disconnect(&s_proto.wg);
        syn_port_sock_close(wg_peer);
        return false;
    }
    if (syn_wg_send(&s_proto.wg, s_app_tx, 64U) != SYN_OK) {
        syn_wg_disconnect(&s_proto.wg);
        syn_port_sock_close(wg_peer);
        return false;
    }
    int tp_got = syn_port_udp_recvfrom(wg_peer, wg_rx_pkt, sizeof(wg_rx_pkt), &wg_cli_addr, 50U);
    if (tp_got == (int)SYN_WG_TRANSPORT_OVERHEAD && wg_rx_pkt[0] == SYN_WG_MSG_TRANSPORT) {
        /* Drain RFC keepalive datagram queued ahead of the 64B tunnel packet */
        tp_got = syn_port_udp_recvfrom(wg_peer, wg_rx_pkt, sizeof(wg_rx_pkt), &wg_cli_addr, 50U);
    }
    if (tp_got != (int)(64U + SYN_WG_TRANSPORT_OVERHEAD) || wg_rx_pkt[0] != SYN_WG_MSG_TRANSPORT) {
        syn_wg_disconnect(&s_proto.wg);
        syn_port_sock_close(wg_peer);
        return false;
    }
    syn_wg_disconnect(&s_proto.wg);
    syn_port_sock_close(wg_peer);

    bool ok_wg = hil_check_budget(&prof, &g_sched, out_wg_us);
    return ok_dtls && ok_wg;
}

#endif /* ARDUINO_ARCH_ESP32 */

/* ── Test 5: Dual-Bank Flash/EEPROM Failover & Out-of-Bounds Guard ──────── */

static bool test_dual_bank_eeprom_failover(void)
{
    uint8_t dummy = 0x55U;
    if (syn_port_flash_write(HIL_FLASH_END + 1UL, &dummy, 1U) != SYN_INVALID_PARAM) {
        return false;
    }
    if (syn_port_flash_read(HIL_FLASH_END + 1UL, &dummy, 1U) != SYN_INVALID_PARAM) {
        return false;
    }
    if (syn_port_flash_erase(HIL_FLASH_END + 1UL) != SYN_INVALID_PARAM) {
        return false;
    }

    SYN_DualBankSettings db;
    HilPersistentState s_ram = {0U, 0U, 0U};

    /* Erase both banks first */
    syn_port_flash_erase(HIL_EEPROM_ADDR_BANK_A);
    syn_port_flash_erase(HIL_EEPROM_ADDR_BANK_B);

    if (syn_settings_dual_bank_init(&db, HIL_EEPROM_ADDR_BANK_A, HIL_EEPROM_ADDR_BANK_B, 1U,
                                    &s_ram, sizeof(s_ram), &k_default_state) != SYN_OK) {
        return false;
    }

    /* Save state #1 and state #2 so both Bank A and Bank B have valid commits */
    s_ram.magic = 0xCAFEBABEUL;
    s_ram.boot_seq = 10U;
    s_ram.kat_crc = 0x1111U;
    if (syn_settings_dual_bank_save(&db) != SYN_OK) return false;

    s_ram.boot_seq = 20U;
    s_ram.kat_crc = 0x2222U;
    if (syn_settings_dual_bank_save(&db) != SYN_OK) return false;

    /* Corrupt Bank A Slot 1 payload and verify dual_bank_init falls back to Bank B */
    uint8_t garbage[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    syn_port_flash_write(HIL_EEPROM_ADDR_BANK_A + 32U + (uint32_t)sizeof(SYN_ParamSlotHeader),
                         garbage, sizeof(garbage));

    memset(&s_ram, 0, sizeof(s_ram));
    if (syn_settings_dual_bank_init(&db, HIL_EEPROM_ADDR_BANK_A, HIL_EEPROM_ADDR_BANK_B, 1U,
                                    &s_ram, sizeof(s_ram), &k_default_state) != SYN_OK) {
        return false;
    }

    return (s_ram.magic == 0xCAFEBABEUL && s_ram.boot_seq == 10U && s_ram.kat_crc == 0x1111U);
}

/* ── Entry Points ───────────────────────────────────────────────────────── */

#if defined(ARDUINO_ARCH_ESP32)
static void print_wdt_recovery_banner(uint32_t seq, uint16_t crc)
{
    Serial.print(F("\r\n[HIL-5/5] WDT_REBOOT_RECOVERY: PASS (seq="));
    Serial.print(seq);
    Serial.print(F(" crc=0x"));
    Serial.print(crc, HEX);
    Serial.print(F(" free_sram="));
    Serial.print(get_free_sram());
    Serial.println(F("B)"));
    Serial.println(F("=== ALL 5 AVR HIL SUBSYSTEM TESTS PASSED ==="));
    Serial.flush();
}
#endif

static void print_ready_banner(void)
{
    Serial.println(F("\r\n=== SyntropicOS AVR HIL Test Suite Ready ==="));
    Serial.print(F("Free SRAM at boot: "));
    Serial.print(get_free_sram());
    Serial.println(F(" bytes"));
    Serial.flush();
}

void setup(void)
{
#if defined(ARDUINO_ARCH_MEGAAVR)
    _PROTECTED_WRITE(WDT.CTRLA, 0U);
    RSTCTRL.RSTFR = RSTCTRL.RSTFR;
#elif defined(__AVR__)
    MCUSR = 0U;
    wdt_disable();
#endif

    Serial.begin(115200);
    while (!Serial && millis() < 1500UL) {}

    /* Check if we just rebooted from the Hardware Watchdog test */
    syn_settings_init(&g_settings, 0U, 2U, &g_state, sizeof(g_state), &k_default_state);
    if (g_state.magic == HIL_MAGIC_WDT_PENDING) {
        uint32_t recovered_seq = g_state.boot_seq + 1U;
        uint16_t recovered_crc = g_state.kat_crc;

        /* Clean up sectors used by test */
        syn_port_flash_erase(0U);
        syn_port_flash_erase(64U);
        syn_port_flash_erase(HIL_EEPROM_ADDR_BANK_A);
        syn_port_flash_erase(HIL_EEPROM_ADDR_BANK_B);

#if defined(ARDUINO_ARCH_ESP32)
        g_wdt_recovered = true;
        g_wdt_rec_seq = recovered_seq;
        g_wdt_rec_crc = recovered_crc;
        delay(80);
        print_wdt_recovery_banner(recovered_seq, recovered_crc);
#else
        Serial.print(F("\r\n[HIL-5/5] WDT_REBOOT_RECOVERY: PASS (seq="));
        Serial.print(recovered_seq);
        Serial.print(F(" crc=0x"));
        Serial.print(recovered_crc, HEX);
        Serial.print(F(" free_sram="));
        Serial.print(get_free_sram());
        Serial.println(F("B)"));
        Serial.println(F("=== ALL 5 AVR HIL SUBSYSTEM TESTS PASSED ==="));
#endif
        return;
    }

    print_ready_banner();
}

void loop(void)
{
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd == "banner") {
            print_ready_banner();
#if defined(ARDUINO_ARCH_ESP32)
        } else if (cmd == "wdt_check") {
            if (g_wdt_recovered) {
                print_wdt_recovery_banner(g_wdt_rec_seq, g_wdt_rec_crc);
            }
#endif
        } else if (cmd == "run_hil") {
            bool ok1 = test_sreg_critical_sections();
            Serial.print(F("[HIL-1/5] SREG_CRITICAL_NESTING: "));
            Serial.println(ok1 ? F("PASS") : F("FAIL"));
            Serial.flush();

            bool ok2 = test_timer1_isr_and_mutex_pi();
            Serial.print(F("[HIL-2/5] TIMER1_ISR_AND_MUTEX_PI: "));
            Serial.print(ok2 ? F("PASS") : F("FAIL"));
            Serial.print(F(" (isr="));
            Serial.print(g_timer1_isr_fires);
            Serial.print(F(" mbx="));
            Serial.print(g_task_popped);
            Serial.print(F(" rb_sum="));
            Serial.print(g_rb_sum_task);
            Serial.println(F(")"));
            Serial.flush();

            bool ok3 = test_hardware_exti_int0();
            Serial.print(F("[HIL-3/5] EXTI_INT0_SELF_TRIGGER: "));
            Serial.print(ok3 ? F("PASS") : F("FAIL"));
            Serial.print(F(" (edges="));
            Serial.print(g_exti_cb_count);
            Serial.println(F(")"));
            Serial.flush();

            uint16_t kat_crc = 0U;
            uint16_t x25519_slices = 0U;
            uint32_t x25519_max_us = 0UL;
            bool ok4 = test_avr16_kats(&kat_crc, &x25519_slices, &x25519_max_us);
            Serial.print(F("[HIL-4/5] AVR16_CRYPTO_PROTO_KATS: "));
            Serial.print(ok4 ? F("PASS") : F("FAIL"));
            Serial.print(F(" (crc16=0x"));
            Serial.print(kat_crc, HEX);
            Serial.print(F(" x25519_slices="));
            Serial.print(x25519_slices);
            Serial.print(F(" max_us="));
            Serial.print(x25519_max_us);
            Serial.println(F(")"));
            Serial.flush();

            bool ok_db = test_dual_bank_eeprom_failover();
            Serial.print(F("[HIL-4b]  EEPROM_DUAL_BANK_FAILOVER: "));
            Serial.println(ok_db ? F("PASS") : F("FAIL"));
            Serial.flush();

#if defined(ARDUINO_ARCH_ESP32)
            uint32_t tls_us = 0U;
            uint32_t dtls_us = 0U;
            uint32_t wg_us = 0U;
            bool ok_rv = test_esp32_coop_protocol_budgets(&tls_us, &dtls_us, &wg_us);
            Serial.print(F("[HIL-4c]  RISCV_COOP_PROTOCOL_BUDGET: "));
            Serial.print(ok_rv ? F("PASS") : F("FAIL"));
            Serial.print(F(" (tls_p256_us="));
            Serial.print(tls_us);
            Serial.print(F(" dtls_ed_us="));
            Serial.print(dtls_us);
            Serial.print(F(" wg_us="));
            Serial.print(wg_us);
            Serial.println(F(" budget_us=1000)"));
            Serial.flush();
#else
            bool ok_rv = true;
#endif

            if (!ok1 || !ok2 || !ok3 || !ok4 || !ok_db || !ok_rv) {
                Serial.println(F("FAILURE: HIL check failed before WDT test"));
                return;
            }

            /* Save state to non-volatile storage and trigger real hardware Watchdog reset */
            syn_port_flash_erase(0U);
            syn_port_flash_erase(64U);
            syn_settings_init(&g_settings, 0U, 2U, &g_state, sizeof(g_state), &k_default_state);
            g_state.magic = HIL_MAGIC_WDT_PENDING;
            g_state.boot_seq = 41U;
            g_state.kat_crc = kat_crc;
            syn_settings_save(&g_settings);

            Serial.println(F("[HIL-5/5] Arming Hardware Watchdog for live MCU reset..."));
            Serial.flush();
            syn_port_wdt_init(HIL_WDT_TIMEOUT_MS);
            for (;;) {}
        }
    }
}
