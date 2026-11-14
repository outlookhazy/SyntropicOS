/**
 * @file hil_suite.ino
 * @brief Comprehensive Hardware-in-the-Loop (HIL) System Test on AVR targets.
 *
 * Exercises hardware-specific paths on real 8-bit/16-bit AVR silicon:
 *   1. SREG critical section nesting (both from SEI and CLI contexts)
 *   2. Hardware Timer1 (2 kHz CTC ISR) preempting cooperative scheduler +
 *      transitive Mutex Priority Inheritance + ISR-to-task RingBuf/Sem/EventFlags/Mailbox
 *   3. Hardware INT0 (Pin 2) self-triggered external interrupts via syn_exti +
 *      non-interrupt pin rejection without slot leakage
 *   4. 8/16-bit AVR Crypto, Codec & Fixed-Point KATs (CRC32, SHA-256, AES-128-GCM,
 *      PMBus exponent=-16, LwM2M TLV 64-bit int, Q16.16)
 *   5. Hardware EEPROM wear-leveled & dual-bank failover + E2END bounds check +
 *      real Hardware Watchdog (WDT) MCU reset recovery
 */

#include <SyntropicOS.h>
#include <syntropic/storage/syn_settings.h>
#include <syntropic/port/syn_port_wdt.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#if !defined(ARDUINO_ARCH_MEGAAVR)
#include <avr/wdt.h>
#endif
#include <avr/eeprom.h>
#include <string.h>

extern char __heap_start;
extern char *__brkval;

static uint16_t get_free_sram(void)
{
    char top;
    return (uint16_t)(&top - (__brkval ? __brkval : &__heap_start));
}

/* ── Persistent State Across Hardware WDT Reboot ────────────────────────── */

#define HIL_EEPROM_ADDR_BANK_A  128U
#if defined(ARDUINO_ARCH_MEGAAVR)
#define HIL_EEPROM_ADDR_BANK_B  192U
#else
#define HIL_EEPROM_ADDR_BANK_B  256U
#endif
#define HIL_MAGIC_WDT_PENDING   0x53594E31UL /* "SYN1" */

typedef struct {
    uint32_t magic;
    uint32_t boot_seq;
    uint16_t kat_crc;
} HilPersistentState;

static const HilPersistentState k_default_state = {0U, 0U, 0U};
static HilPersistentState g_state;
static SYN_Settings g_settings;

/* ── Test 1: SREG Critical Section Nesting ──────────────────────────────── */

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

/* ── Test 2: Hardware Timer1 ISR + Mutex PI + IPC Stress ────────────────── */

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

static void timer_isr_step(void)
{
    uint16_t n = ++g_timer1_isr_fires;
    if (n <= 64U) {
        uint8_t b = (uint8_t)(n & 0xFFU);
        if (syn_ringbuf_write(&g_isr_rb, &b, 1U) == 1U) {
            g_rb_sum_isr += b;
        }
        if (syn_mailbox_post(&g_isr_mbx, &n)) {
            g_isr_pushed++;
        }
        PT_SEM_SIGNAL(&g_isr_sem);
        syn_event_flags_set(&g_isr_ev, 0x01U);
    } else {
        /* Stop hardware timer after 64 ISR bursts */
#if defined(ARDUINO_ARCH_MEGAAVR)
        TCB0.CTRLA = 0U;
        TCB0.INTCTRL = 0U;
#else
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
#else
ISR(TIMER1_COMPA_vect)
{
    timer_isr_step();
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
    noInterrupts();
#if defined(ARDUINO_ARCH_MEGAAVR)
    TCB0.CTRLA = 0U;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;
    TCB0.CNT = 0U;
    TCB0.CCMP = (uint16_t)((F_CPU / 2UL / 2000UL) - 1UL);
    TCB0.INTFLAGS = TCB_CAPT_bm;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm;
#else
    TCCR1A = 0U;
    TCCR1B = 0U;
    TCNT1  = 0U;
    OCR1A  = 999U;
    TCCR1B = (1U << WGM12) | (1U << CS11); /* CTC mode, /8 prescaler */
    TIMSK1 = (1U << OCIE1A);
#endif
    interrupts();

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
#else
    TCCR1B = 0U;
    TIMSK1 = 0U;
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
           budget_ok;
}

/* ── Test 3: Hardware INT0 (Pin 2) Self-Loopback External Interrupt ─────── */

static volatile uint16_t g_exti_cb_count = 0U;
static volatile SYN_GPIO_Pin g_exti_last_pin = 0xFFU;

static void on_pin2_exti(SYN_GPIO_Pin pin, void *ctx)
{
    g_exti_last_pin = pin;
    (*(volatile uint16_t *)ctx)++;
}

static bool test_hardware_exti_int0(void)
{
    syn_exti_init();

    /* Probe 6 non-EXTI pins: all must return SYN_NOT_IMPLEMENTED
     * and MUST NOT leak the 6 hardware EXTI trampoline slots! */
#if defined(ARDUINO_ARCH_MEGAAVR)
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
    if (syn_exti_register(2U, SYN_EXTI_BOTH, on_pin2_exti, (void *)&g_exti_cb_count) != SYN_OK) {
        return false;
    }
    /* Verify syn_port_exti_configure on valid Pin 2 succeeds even after 6 invalid probes */
    if (syn_port_exti_configure(2U, SYN_EXTI_BOTH) != SYN_OK) {
        return false;
    }

    /* Drive Pin 2 in software; AVR/megaAVR pin interrupts fire on output toggles */
    pinMode(2U, OUTPUT);
    digitalWrite(2U, HIGH);
    delayMicroseconds(20);
    syn_exti_enable(2U);
    delayMicroseconds(20);
    uint16_t baseline = g_exti_cb_count;

    for (uint8_t i = 0U; i < 4U; i++) {
        digitalWrite(2U, LOW);
        delayMicroseconds(20);
        digitalWrite(2U, HIGH);
        delayMicroseconds(20);
    }

    uint16_t after_toggles = g_exti_cb_count;
    if ((after_toggles - baseline) != 8U || g_exti_last_pin != 2U) {
        syn_exti_disable(2U);
        return false;
    }

    /* Disable EXTI and confirm no further interrupts fire */
    syn_exti_disable(2U);
    digitalWrite(2U, LOW);
    delayMicroseconds(20);
    digitalWrite(2U, HIGH);
    delayMicroseconds(20);

    return (g_exti_cb_count == after_toggles);
}

/* ── Test 4: 8/16-Bit AVR Crypto, Codec & Fixed-Point KATs ──────────────── */

static __attribute__((noinline)) bool test_kat_sha256(void)
{
    uint8_t digest[32];
    syn_sha256((const uint8_t *)"abc", 3U, digest);
    return (digest[0] == 0xBAU && digest[1] == 0x78U && digest[2] == 0x16U &&
            digest[3] == 0xBFU && digest[31] == 0xADU);
}

static __attribute__((noinline)) bool test_kat_aes_gcm(void)
{
    static const uint8_t key[16] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F
    };
    static const uint8_t iv[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    static const uint8_t aad[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    const uint8_t pt[16] = "SyntropicOS-AVR";
    uint8_t ct[16];
    uint8_t dec[16];
    uint8_t tag[16];
    SYN_AES_GCM_Context gcm;

    if (syn_aes_gcm_init(&gcm, key, 16U) != SYN_OK) {
        return false;
    }
    if (syn_aes_gcm_encrypt(&gcm, iv, 12U, aad, sizeof(aad), pt, 16U, ct, tag) != SYN_OK) {
        return false;
    }
    if (syn_aes_gcm_decrypt(&gcm, iv, 12U, aad, sizeof(aad), ct, 16U, dec, tag) != SYN_OK) {
        return false;
    }
    if (memcmp(pt, dec, 16U) != 0) {
        return false;
    }
    tag[0] ^= 0x01U;
    if (syn_aes_gcm_decrypt(&gcm, iv, 12U, aad, sizeof(aad), ct, 16U, dec, tag) == SYN_OK) {
        return false;
    }
    return true;
}

static __attribute__((noinline)) bool test_avr16_kats(uint16_t *out_crc)
{
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

    /* 3. AES-128-GCM Encrypt + Authenticate + Tamper Reject */
    if (!test_kat_aes_gcm()) {
        return false;
    }

    /* 4. PMBus Linear16 & Linear11 at exponent = -16 (vout_mode = 0x10, 16-bit shift boundary) */
    float vout = syn_pmbus_linear16_to_float(65535U, 0x10U);
    uint16_t raw16 = syn_pmbus_float_to_linear16(0.5f, 0x10U);
    if (vout < 0.999f || vout > 1.001f || raw16 != 32768U) {
        return false;
    }

    /* 5. LwM2M 16-bit TLV & 64-bit Int Codec */
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

    /* 6. Fixed-Point Q16.16 Math */
    int32_t a = Q16_FROM_INT(12);
    int32_t b = Q16_FROM_INT(3);
    if (q16_mul(a, b) != Q16_FROM_INT(36) ||
        q16_div(a, b) != Q16_FROM_INT(4)) {
        return false;
    }

    return true;
}

/* ── Test 5: Dual-Bank EEPROM Failover & Out-of-Bounds Guard ───────────── */

static bool test_dual_bank_eeprom_failover(void)
{
    /* Out-of-bounds EEPROM write past E2END (1023 on ATmega328P) must be rejected
     * so it cannot wrap 10-bit EEAR and clobber address 0! */
    uint8_t dummy = 0x55U;
    if (syn_port_flash_write((uint32_t)E2END + 1UL, &dummy, 1U) != SYN_INVALID_PARAM) {
        return false;
    }
    if (syn_port_flash_read((uint32_t)E2END + 1UL, &dummy, 1U) != SYN_INVALID_PARAM) {
        return false;
    }
    if (syn_port_flash_erase((uint32_t)E2END + 1UL) != SYN_INVALID_PARAM) {
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

    /* Corrupt Bank A Slot 1 payload (offset +32 + sizeof(SYN_ParamSlotHeader), which holds
     * seq=3, boot_seq=20) in EEPROM and verify dual_bank_init falls back to Bank B (seq=2, boot_seq=10)! */
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
#else
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

        /* Clean up EEPROM sectors used by test */
        syn_port_flash_erase(0U);
        syn_port_flash_erase(64U);
        syn_port_flash_erase(HIL_EEPROM_ADDR_BANK_A);
        syn_port_flash_erase(HIL_EEPROM_ADDR_BANK_B);

        Serial.print(F("\r\n[HIL-5/5] WDT_REBOOT_RECOVERY: PASS (seq="));
        Serial.print(recovered_seq);
        Serial.print(F(" crc=0x"));
        Serial.print(recovered_crc, HEX);
        Serial.print(F(" free_sram="));
        Serial.print(get_free_sram());
        Serial.println(F("B)"));
        Serial.println(F("=== ALL 5 AVR HIL SUBSYSTEM TESTS PASSED ==="));
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
            bool ok4 = test_avr16_kats(&kat_crc);
            Serial.print(F("[HIL-4/5] AVR16_CRYPTO_PROTO_KATS: "));
            Serial.print(ok4 ? F("PASS") : F("FAIL"));
            Serial.print(F(" (crc16=0x"));
            Serial.print(kat_crc, HEX);
            Serial.println(F(")"));
            Serial.flush();

            bool ok_db = test_dual_bank_eeprom_failover();
            Serial.print(F("[HIL-4b]  EEPROM_DUAL_BANK_FAILOVER: "));
            Serial.println(ok_db ? F("PASS") : F("FAIL"));
            Serial.flush();

            if (!ok1 || !ok2 || !ok3 || !ok4 || !ok_db) {
                Serial.println(F("FAILURE: HIL check failed before WDT test"));
                return;
            }

            /* Save state to EEPROM and trigger real hardware Watchdog reset (15ms) */
            syn_port_flash_erase(0U);
            syn_port_flash_erase(64U);
            syn_settings_init(&g_settings, 0U, 2U, &g_state, sizeof(g_state), &k_default_state);
            g_state.magic = HIL_MAGIC_WDT_PENDING;
            g_state.boot_seq = 41U;
            g_state.kat_crc = kat_crc;
            syn_settings_save(&g_settings);

            Serial.println(F("[HIL-5/5] Arming Hardware Watchdog (15ms) for live MCU reset..."));
            Serial.flush();
            syn_port_wdt_init(15U);
            for (;;) {}
        }
    }
}
