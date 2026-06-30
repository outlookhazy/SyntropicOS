/**
 * @file port_rp2040_multicore.c
 * @brief SyntropicOS AMP multicore port for RP2040/RP2350 (Pico SDK).
 *
 * Implements the spinlock, memory barrier, core identity, and IPC
 * port functions using the RP2040 hardware spinlock peripheral and
 * Cortex-M0+ barrier instructions.
 *
 * Hardware spinlocks are dynamically allocated via spin_lock_claim_unused()
 * to avoid conflicts with SDK-internal usage.
 *
 * Compiled when: PICO_BOARD && !ARDUINO && SYN_USE_MULTICORE
 */

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(PICO_BOARD) && !defined(ARDUINO) && \
    defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_spinlock.h"
#include "syntropic/common/syn_barrier.h"

#include "hardware/sync.h"
#include "pico/multicore.h"

/* ── Per-lock saved interrupt state ────────────────────────────────────── */

static uint32_t s_saved_irq[SYN_SPINLOCK_COUNT];

/* ── Initialization (lazy, called on first acquire) ────────────────────── */

static bool s_spinlocks_initialized = false;

/*
 * Maps SYN spinlock ID (0..SYN_SPINLOCK_COUNT-1) to actual hardware
 * lock number, dynamically allocated to avoid conflicts with the
 * Pico SDK's own claims.
 */
static uint s_hw_lock_num[SYN_SPINLOCK_COUNT];

static void __attribute__((constructor)) ensure_spinlocks_init(void)
{
    if (!s_spinlocks_initialized) {
        for (uint8_t i = 0; i < SYN_SPINLOCK_COUNT; i++) {
            int hw = spin_lock_claim_unused(true);
            s_hw_lock_num[i] = (uint)hw;
            spin_lock_init((uint)hw);
        }
        s_spinlocks_initialized = true;
    }
}

/* ── Memory barrier ────────────────────────────────────────────────────── */

void syn_port_memory_barrier(void)
{
    __dmb();
}

/* ── Spinlock port ─────────────────────────────────────────────────────── */

void syn_port_spinlock_acquire(uint8_t id)
{
    if (id >= SYN_SPINLOCK_COUNT) return;
    ensure_spinlocks_init();

    spin_lock_t *lock = spin_lock_instance(s_hw_lock_num[id]);
    s_saved_irq[id] = spin_lock_blocking(lock);
}

void syn_port_spinlock_release(uint8_t id)
{
    if (id >= SYN_SPINLOCK_COUNT) return;

    spin_lock_t *lock = spin_lock_instance(s_hw_lock_num[id]);
    spin_unlock(lock, s_saved_irq[id]);
}

bool syn_port_spinlock_try_acquire(uint8_t id)
{
    if (id >= SYN_SPINLOCK_COUNT) return false;
    ensure_spinlocks_init();

    spin_lock_t *lock = spin_lock_instance(s_hw_lock_num[id]);

    if (is_spin_locked(lock)) return false;

    uint32_t save = save_and_disable_interrupts();
    bool acquired = spin_try_lock_unsafe(lock);
    if (acquired) {
        s_saved_irq[id] = save;
    } else {
        restore_interrupts(save);
    }
    return acquired;
}

/* ── Core identity ─────────────────────────────────────────────────────── */

uint8_t syn_port_core_id(void)
{
    return (uint8_t)get_core_num();
}

/* ── Inter-core notification ───────────────────────────────────────────── */

void syn_port_ipc_notify(void)
{
    __sev();
}

#endif /* PICO_BOARD && !ARDUINO && SYN_USE_MULTICORE */
