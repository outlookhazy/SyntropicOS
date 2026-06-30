/**
 * @file test_multicore.c
 * @brief Comprehensive tests for AMP multicore primitives.
 *
 * Covers: memory barriers, spinlock port, scoped guards, and the
 * barrier-upgraded mailbox for cross-core correctness. Tests simulate
 * interleaved single-producer/single-consumer patterns that exercise
 * every state transition in the SPSC ring buffer.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"

#include "syntropic/common/syn_barrier.h"
#include "syntropic/common/syn_compiler.h"
#include "syntropic/port/syn_port_spinlock.h"
#include "syntropic/util/syn_spinlock.h"
#include "syntropic/sched/syn_mailbox.h"

#include <string.h>

/* ════════════════════════════════════════════════════════════════════════
 * SECTION 1: Memory Barrier Primitives
 * ════════════════════════════════════════════════════════════════════════ */

void test_barrier_compiler_barrier_compiles(void)
{
    /* SYN_COMPILER_BARRIER must compile on all supported compilers */
    int x = 0;
    SYN_COMPILER_BARRIER();
    x = 1;
    SYN_COMPILER_BARRIER();
    TEST_ASSERT_EQUAL(1, x);
}

void test_barrier_store_release_writes_value(void)
{
    volatile size_t idx = 0;
    SYN_STORE_RELEASE(&idx, 42);
    TEST_ASSERT_EQUAL(42, idx);
}

void test_barrier_store_release_invokes_hardware_barrier(void)
{
    volatile size_t idx = 0;
    mock_barrier_count = 0;
    SYN_STORE_RELEASE(&idx, 1);
    TEST_ASSERT_TRUE(mock_barrier_count >= 1);
}

void test_barrier_load_acquire_reads_value(void)
{
    volatile size_t idx = 99;
    size_t val = SYN_LOAD_ACQUIRE(&idx);
    TEST_ASSERT_EQUAL(99, val);
}

void test_barrier_load_acquire_invokes_hardware_barrier(void)
{
    volatile size_t idx = 0;
    mock_barrier_count = 0;
    SYN_LOAD_ACQUIRE(&idx);
    TEST_ASSERT_TRUE(mock_barrier_count >= 1);
}

void test_barrier_store_then_load_roundtrip(void)
{
    volatile size_t idx = 0;
    SYN_STORE_RELEASE(&idx, 12345);
    size_t val = SYN_LOAD_ACQUIRE(&idx);
    TEST_ASSERT_EQUAL(12345, val);
}

void test_barrier_zero_value(void)
{
    /* Edge case: storing/loading zero must work (not confused with NULL) */
    volatile size_t idx = 42;
    SYN_STORE_RELEASE(&idx, 0);
    TEST_ASSERT_EQUAL(0, SYN_LOAD_ACQUIRE(&idx));
}

void test_barrier_max_value(void)
{
    volatile size_t idx = 0;
    SYN_STORE_RELEASE(&idx, SIZE_MAX);
    TEST_ASSERT_EQUAL(SIZE_MAX, SYN_LOAD_ACQUIRE(&idx));
}

void test_barrier_multiple_sequential(void)
{
    /* Multiple barriers in sequence must not corrupt state */
    volatile size_t idx = 0;
    mock_barrier_count = 0;
    for (int i = 0; i < 10; i++) {
        SYN_STORE_RELEASE(&idx, (size_t)i);
    }
    TEST_ASSERT_EQUAL(9, SYN_LOAD_ACQUIRE(&idx));
    /* At least 10 store barriers + 1 load barrier */
    TEST_ASSERT_TRUE(mock_barrier_count >= 11);
}

/* ════════════════════════════════════════════════════════════════════════
 * SECTION 2: Spinlock Port
 * ════════════════════════════════════════════════════════════════════════ */

void test_spinlock_acquire_release_basic(void)
{
    TEST_ASSERT_FALSE(mock_spinlock_held[0]);
    syn_port_spinlock_acquire(0);
    TEST_ASSERT_TRUE(mock_spinlock_held[0]);
    TEST_ASSERT_EQUAL(1, mock_spinlock_acquire_count[0]);
    syn_port_spinlock_release(0);
    TEST_ASSERT_FALSE(mock_spinlock_held[0]);
}

void test_spinlock_acquire_release_repeated(void)
{
    /* Acquire-release cycle 10 times — must work every time */
    for (int i = 0; i < 10; i++) {
        syn_port_spinlock_acquire(0);
        TEST_ASSERT_TRUE(mock_spinlock_held[0]);
        syn_port_spinlock_release(0);
        TEST_ASSERT_FALSE(mock_spinlock_held[0]);
    }
    TEST_ASSERT_EQUAL(10, mock_spinlock_acquire_count[0]);
}

void test_spinlock_try_acquire_free(void)
{
    TEST_ASSERT_TRUE(syn_port_spinlock_try_acquire(1));
    TEST_ASSERT_TRUE(mock_spinlock_held[1]);
    TEST_ASSERT_EQUAL(1, mock_spinlock_acquire_count[1]);
    syn_port_spinlock_release(1);
}

void test_spinlock_try_acquire_contended(void)
{
    mock_spinlock_held[2] = true;  /* simulate other core holding it */
    TEST_ASSERT_FALSE(syn_port_spinlock_try_acquire(2));
    TEST_ASSERT_EQUAL(0, mock_spinlock_acquire_count[2]);
    mock_spinlock_held[2] = false;
}

void test_spinlock_try_acquire_after_release(void)
{
    /* Lock, release, then try_acquire should succeed */
    syn_port_spinlock_acquire(0);
    syn_port_spinlock_release(0);
    TEST_ASSERT_TRUE(syn_port_spinlock_try_acquire(0));
    TEST_ASSERT_TRUE(mock_spinlock_held[0]);
    syn_port_spinlock_release(0);
}

void test_spinlock_multiple_ids_independent(void)
{
    syn_port_spinlock_acquire(0);
    syn_port_spinlock_acquire(1);
    TEST_ASSERT_TRUE(mock_spinlock_held[0]);
    TEST_ASSERT_TRUE(mock_spinlock_held[1]);
    TEST_ASSERT_FALSE(mock_spinlock_held[2]);
    TEST_ASSERT_FALSE(mock_spinlock_held[3]);

    syn_port_spinlock_release(0);
    TEST_ASSERT_FALSE(mock_spinlock_held[0]);
    TEST_ASSERT_TRUE(mock_spinlock_held[1]);
    syn_port_spinlock_release(1);
}

void test_spinlock_all_ids(void)
{
    /* Acquire all available spinlocks */
    for (int i = 0; i < SYN_SPINLOCK_COUNT; i++) {
        syn_port_spinlock_acquire((uint8_t)i);
        TEST_ASSERT_TRUE(mock_spinlock_held[i]);
    }
    /* Release in reverse order */
    for (int i = SYN_SPINLOCK_COUNT - 1; i >= 0; i--) {
        syn_port_spinlock_release((uint8_t)i);
        TEST_ASSERT_FALSE(mock_spinlock_held[i]);
    }
}

void test_spinlock_out_of_range_acquire(void)
{
    /* Out-of-range ID should not crash (mock silently ignores) */
    syn_port_spinlock_acquire(SYN_SPINLOCK_COUNT);
    /* Verify none of the valid locks were affected */
    for (int i = 0; i < SYN_SPINLOCK_COUNT; i++) {
        TEST_ASSERT_FALSE(mock_spinlock_held[i]);
    }
}

void test_spinlock_out_of_range_try_acquire(void)
{
    TEST_ASSERT_FALSE(syn_port_spinlock_try_acquire(SYN_SPINLOCK_COUNT));
}

void test_spinlock_core_id_default(void)
{
    mock_core_id = 0;
    TEST_ASSERT_EQUAL(0, syn_port_core_id());
}

void test_spinlock_core_id_secondary(void)
{
    mock_core_id = 1;
    TEST_ASSERT_EQUAL(1, syn_port_core_id());
}

void test_spinlock_ipc_notify_count(void)
{
    mock_ipc_notify_count = 0;
    syn_port_ipc_notify();
    syn_port_ipc_notify();
    syn_port_ipc_notify();
    TEST_ASSERT_EQUAL(3, mock_ipc_notify_count);
}

/* ════════════════════════════════════════════════════════════════════════
 * SECTION 3: Scoped Spinlock Guard (SYN_SPINLOCK_GUARD)
 * ════════════════════════════════════════════════════════════════════════ */

void test_guard_acquires_and_releases(void)
{
    TEST_ASSERT_FALSE(mock_spinlock_held[SYN_SPINLOCK_UART]);
    SYN_SPINLOCK_GUARD(SYN_SPINLOCK_UART) {
        TEST_ASSERT_TRUE(mock_spinlock_held[SYN_SPINLOCK_UART]);
    }
    TEST_ASSERT_FALSE(mock_spinlock_held[SYN_SPINLOCK_UART]);
    TEST_ASSERT_EQUAL(1, mock_spinlock_acquire_count[SYN_SPINLOCK_UART]);
}

void test_guard_executes_body(void)
{
    int executed = 0;
    SYN_SPINLOCK_GUARD(SYN_SPINLOCK_FLASH) {
        executed = 1;
    }
    TEST_ASSERT_EQUAL(1, executed);
    TEST_ASSERT_FALSE(mock_spinlock_held[SYN_SPINLOCK_FLASH]);
}

void test_guard_body_modifies_external_state(void)
{
    int counter = 0;
    SYN_SPINLOCK_GUARD(0) {
        counter += 10;
        counter *= 2;
    }
    TEST_ASSERT_EQUAL(20, counter);
}

void test_guard_nested_different_ids(void)
{
    SYN_SPINLOCK_GUARD(SYN_SPINLOCK_USER0) {
        TEST_ASSERT_TRUE(mock_spinlock_held[SYN_SPINLOCK_USER0]);
        SYN_SPINLOCK_GUARD(SYN_SPINLOCK_USER1) {
            TEST_ASSERT_TRUE(mock_spinlock_held[SYN_SPINLOCK_USER0]);
            TEST_ASSERT_TRUE(mock_spinlock_held[SYN_SPINLOCK_USER1]);
        }
        TEST_ASSERT_TRUE(mock_spinlock_held[SYN_SPINLOCK_USER0]);
        TEST_ASSERT_FALSE(mock_spinlock_held[SYN_SPINLOCK_USER1]);
    }
    TEST_ASSERT_FALSE(mock_spinlock_held[SYN_SPINLOCK_USER0]);
    TEST_ASSERT_FALSE(mock_spinlock_held[SYN_SPINLOCK_USER1]);
}

void test_guard_sequential_same_id(void)
{
    /* Two sequential guards on the same lock */
    SYN_SPINLOCK_GUARD(0) {
        TEST_ASSERT_TRUE(mock_spinlock_held[0]);
    }
    TEST_ASSERT_FALSE(mock_spinlock_held[0]);

    SYN_SPINLOCK_GUARD(0) {
        TEST_ASSERT_TRUE(mock_spinlock_held[0]);
    }
    TEST_ASSERT_FALSE(mock_spinlock_held[0]);
    TEST_ASSERT_EQUAL(2, mock_spinlock_acquire_count[0]);
}

void test_guard_empty_body(void)
{
    /* Empty body: lock acquired and released, no crash */
    SYN_SPINLOCK_GUARD(1) {
        /* intentionally empty */
    }
    TEST_ASSERT_FALSE(mock_spinlock_held[1]);
    TEST_ASSERT_EQUAL(1, mock_spinlock_acquire_count[1]);
}

void test_guard_with_break(void)
{
    /* Break inside a guard that's inside a loop.
     * The guard scope exits normally (break exits the for-loop of the
     * guard macro itself), releasing the lock. */
    int i;
    for (i = 0; i < 5; i++) {
        SYN_SPINLOCK_GUARD(0) {
            if (i == 2) break;
        }
    }
    /* Lock must be released regardless */
    TEST_ASSERT_FALSE(mock_spinlock_held[0]);
}

/* ════════════════════════════════════════════════════════════════════════
 * SECTION 4: Cross-Core Mailbox (upgraded syn_mailbox)
 * ════════════════════════════════════════════════════════════════════════ */

/* Small typed message for basic tests */
typedef struct {
    uint32_t seq;
    uint8_t  data[12];
} TestMsg;

/* Single-byte message for minimal size test */
typedef struct {
    uint8_t val;
} TinyMsg;

/* Large message for alignment/copy stress */
typedef struct {
    uint32_t id;
    uint8_t  payload[60];
} LargeMsg;

/* ── Basic operations ──────────────────────────────────────────────────── */

void test_mbox_post_receive_single(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 1, .data = {0xAA} };
    TestMsg rx;
    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));
    TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
    TEST_ASSERT_EQUAL(1, rx.seq);
    TEST_ASSERT_EQUAL(0xAA, rx.data[0]);
}

void test_mbox_empty_receive_fails(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg rx;
    TEST_ASSERT_FALSE(syn_mailbox_receive(&mb, &rx));
}

void test_mbox_empty_after_init(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));
    TEST_ASSERT_FALSE(syn_mailbox_full(&mb));
    TEST_ASSERT_EQUAL(0, syn_mailbox_pending(&mb));
    TEST_ASSERT_EQUAL(3, syn_mailbox_free(&mb)); /* 4 slots - 1 sentinel */
}

/* ── FIFO ordering ─────────────────────────────────────────────────────── */

void test_mbox_fifo_order(void)
{
    uint8_t buf[8 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 8);

    for (uint32_t i = 0; i < 7; i++) {
        TestMsg tx = { .seq = i };
        TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));
    }
    for (uint32_t i = 0; i < 7; i++) {
        TestMsg rx;
        TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
        TEST_ASSERT_EQUAL(i, rx.seq);
    }
}

/* ── Capacity and overflow ─────────────────────────────────────────────── */

void test_mbox_fill_to_capacity(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 0 };
    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));  /* 1/3 */
    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));  /* 2/3 */
    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));  /* 3/3 = full */

    TEST_ASSERT_TRUE(syn_mailbox_full(&mb));
    TEST_ASSERT_EQUAL(3, syn_mailbox_pending(&mb));
    TEST_ASSERT_EQUAL(0, syn_mailbox_free(&mb));
}

void test_mbox_overflow_counted(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 0 };
    syn_mailbox_post(&mb, &tx);
    syn_mailbox_post(&mb, &tx);
    syn_mailbox_post(&mb, &tx);

    /* 4th, 5th, 6th post fail */
    TEST_ASSERT_FALSE(syn_mailbox_post(&mb, &tx));
    TEST_ASSERT_FALSE(syn_mailbox_post(&mb, &tx));
    TEST_ASSERT_FALSE(syn_mailbox_post(&mb, &tx));

    TEST_ASSERT_EQUAL(3, syn_mailbox_overflows(&mb));
}

void test_mbox_overflow_no_data_corruption(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    /* Fill with known values */
    for (uint32_t i = 0; i < 3; i++) {
        TestMsg tx = { .seq = i + 100 };
        syn_mailbox_post(&mb, &tx);
    }

    /* Overflow attempt must NOT corrupt existing data */
    TestMsg bad = { .seq = 999 };
    syn_mailbox_post(&mb, &bad);

    /* Drain — original values intact */
    for (uint32_t i = 0; i < 3; i++) {
        TestMsg rx;
        syn_mailbox_receive(&mb, &rx);
        TEST_ASSERT_EQUAL(i + 100, rx.seq);
    }
}

void test_mbox_post_after_drain(void)
{
    /* Fill → drain → fill again: must work */
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 0 };
    syn_mailbox_post(&mb, &tx);
    syn_mailbox_post(&mb, &tx);
    syn_mailbox_post(&mb, &tx);

    TestMsg rx;
    syn_mailbox_receive(&mb, &rx);
    syn_mailbox_receive(&mb, &rx);
    syn_mailbox_receive(&mb, &rx);

    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));

    /* Can post again */
    tx.seq = 42;
    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));
    TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
    TEST_ASSERT_EQUAL(42, rx.seq);
}

/* ── Wrap-around ───────────────────────────────────────────────────────── */

void test_mbox_wraparound_single_slot(void)
{
    /* Capacity 2 = 1 usable slot, heavy wrapping */
    uint8_t buf[2 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 2);

    for (uint32_t i = 0; i < 50; i++) {
        TestMsg tx = { .seq = i };
        TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));
        TEST_ASSERT_TRUE(syn_mailbox_full(&mb));

        TestMsg rx;
        TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
        TEST_ASSERT_EQUAL(i, rx.seq);
        TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));
    }
}

void test_mbox_wraparound_multi_round(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    for (int round = 0; round < 10; round++) {
        for (uint32_t i = 0; i < 3; i++) {
            TestMsg tx = { .seq = (uint32_t)(round * 3 + i) };
            TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));
        }
        for (uint32_t i = 0; i < 3; i++) {
            TestMsg rx;
            TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
            TEST_ASSERT_EQUAL((uint32_t)(round * 3 + i), rx.seq);
        }
        TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));
    }
}

/* ── Interleaved post/receive (simulated cross-core) ───────────────────── */

void test_mbox_interleaved_one_at_a_time(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    for (uint32_t i = 0; i < 100; i++) {
        TestMsg tx = { .seq = i };
        TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));
        TestMsg rx;
        TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
        TEST_ASSERT_EQUAL(i, rx.seq);
    }
}

void test_mbox_interleaved_burst(void)
{
    /* Post 2, receive 1, post 1, receive 2 — variable patterns */
    uint8_t buf[8 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 8);

    uint32_t produced = 0;
    uint32_t consumed = 0;

    /* Burst 1: post 3 */
    for (int i = 0; i < 3; i++) {
        TestMsg tx = { .seq = produced++ };
        syn_mailbox_post(&mb, &tx);
    }
    TEST_ASSERT_EQUAL(3, syn_mailbox_pending(&mb));

    /* Consume 2 */
    for (int i = 0; i < 2; i++) {
        TestMsg rx;
        syn_mailbox_receive(&mb, &rx);
        TEST_ASSERT_EQUAL(consumed++, rx.seq);
    }
    TEST_ASSERT_EQUAL(1, syn_mailbox_pending(&mb));

    /* Post 4 more */
    for (int i = 0; i < 4; i++) {
        TestMsg tx = { .seq = produced++ };
        syn_mailbox_post(&mb, &tx);
    }
    TEST_ASSERT_EQUAL(5, syn_mailbox_pending(&mb));

    /* Drain all */
    while (!syn_mailbox_empty(&mb)) {
        TestMsg rx;
        syn_mailbox_receive(&mb, &rx);
        TEST_ASSERT_EQUAL(consumed++, rx.seq);
    }
    TEST_ASSERT_EQUAL(produced, consumed);
}

/* ── Data integrity ────────────────────────────────────────────────────── */

void test_mbox_struct_integrity(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 0xDEADBEEF };
    memset(tx.data, 0x42, sizeof(tx.data));
    syn_mailbox_post(&mb, &tx);

    TestMsg rx;
    memset(&rx, 0, sizeof(rx));
    syn_mailbox_receive(&mb, &rx);
    TEST_ASSERT_EQUAL(0xDEADBEEF, rx.seq);
    TEST_ASSERT_EQUAL_MEMORY(tx.data, rx.data, sizeof(tx.data));
}

void test_mbox_tiny_message(void)
{
    uint8_t buf[8 * sizeof(TinyMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TinyMsg), 8);

    for (uint8_t i = 0; i < 7; i++) {
        TinyMsg tx = { .val = i };
        TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));
    }
    for (uint8_t i = 0; i < 7; i++) {
        TinyMsg rx;
        TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
        TEST_ASSERT_EQUAL(i, rx.val);
    }
}

void test_mbox_large_message(void)
{
    uint8_t buf[4 * sizeof(LargeMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(LargeMsg), 4);

    LargeMsg tx = { .id = 0xCAFE };
    for (int i = 0; i < 60; i++) tx.payload[i] = (uint8_t)i;

    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));

    LargeMsg rx;
    memset(&rx, 0, sizeof(rx));
    TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
    TEST_ASSERT_EQUAL(0xCAFE, rx.id);
    TEST_ASSERT_EQUAL_MEMORY(tx.payload, rx.payload, 60);
}

void test_mbox_data_pattern_stress(void)
{
    /* Post messages with distinct byte patterns, verify after wraparound */
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    for (int round = 0; round < 20; round++) {
        TestMsg tx;
        tx.seq = (uint32_t)round;
        memset(tx.data, (uint8_t)round, sizeof(tx.data));
        syn_mailbox_post(&mb, &tx);

        TestMsg rx;
        syn_mailbox_receive(&mb, &rx);
        TEST_ASSERT_EQUAL((uint32_t)round, rx.seq);
        uint8_t expected[12];
        memset(expected, (uint8_t)round, 12);
        TEST_ASSERT_EQUAL_MEMORY(expected, rx.data, 12);
    }
}

/* ── Peek ──────────────────────────────────────────────────────────────── */

void test_mbox_peek_returns_data(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 77 };
    syn_mailbox_post(&mb, &tx);

    const TestMsg *p = (const TestMsg *)syn_mailbox_peek(&mb);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL(77, p->seq);

    /* Peek does NOT consume */
    TEST_ASSERT_EQUAL(1, syn_mailbox_pending(&mb));
}

void test_mbox_peek_empty_returns_null(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TEST_ASSERT_NULL(syn_mailbox_peek(&mb));
}

void test_mbox_peek_then_receive(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 88 };
    syn_mailbox_post(&mb, &tx);

    /* Peek, then receive — must get same data */
    const TestMsg *p = (const TestMsg *)syn_mailbox_peek(&mb);
    TEST_ASSERT_EQUAL(88, p->seq);

    TestMsg rx;
    syn_mailbox_receive(&mb, &rx);
    TEST_ASSERT_EQUAL(88, rx.seq);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));
}

/* ── State query accuracy ──────────────────────────────────────────────── */

void test_mbox_state_transitions(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);
    TestMsg tx = {0}, rx;

    /* Empty */
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));
    TEST_ASSERT_FALSE(syn_mailbox_full(&mb));
    TEST_ASSERT_EQUAL(0, syn_mailbox_pending(&mb));
    TEST_ASSERT_EQUAL(3, syn_mailbox_free(&mb));

    /* 1 message */
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_FALSE(syn_mailbox_empty(&mb));
    TEST_ASSERT_FALSE(syn_mailbox_full(&mb));
    TEST_ASSERT_EQUAL(1, syn_mailbox_pending(&mb));
    TEST_ASSERT_EQUAL(2, syn_mailbox_free(&mb));

    /* 2 messages */
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_EQUAL(2, syn_mailbox_pending(&mb));
    TEST_ASSERT_EQUAL(1, syn_mailbox_free(&mb));

    /* 3 messages (full) */
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_TRUE(syn_mailbox_full(&mb));
    TEST_ASSERT_EQUAL(3, syn_mailbox_pending(&mb));
    TEST_ASSERT_EQUAL(0, syn_mailbox_free(&mb));

    /* Consume 1 → not full */
    syn_mailbox_receive(&mb, &rx);
    TEST_ASSERT_FALSE(syn_mailbox_full(&mb));
    TEST_ASSERT_EQUAL(2, syn_mailbox_pending(&mb));
    TEST_ASSERT_EQUAL(1, syn_mailbox_free(&mb));

    /* Consume remaining */
    syn_mailbox_receive(&mb, &rx);
    syn_mailbox_receive(&mb, &rx);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));
    TEST_ASSERT_EQUAL(3, syn_mailbox_free(&mb));
}

/* ── Flush ─────────────────────────────────────────────────────────────── */

void test_mbox_flush(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 1 };
    syn_mailbox_post(&mb, &tx);
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_EQUAL(2, syn_mailbox_pending(&mb));

    syn_mailbox_flush(&mb);
    TEST_ASSERT_TRUE(syn_mailbox_empty(&mb));
    TEST_ASSERT_EQUAL(0, syn_mailbox_pending(&mb));
}

void test_mbox_flush_then_reuse(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 1 };
    syn_mailbox_post(&mb, &tx);
    syn_mailbox_flush(&mb);

    /* Post again after flush */
    tx.seq = 99;
    TEST_ASSERT_TRUE(syn_mailbox_post(&mb, &tx));
    TestMsg rx;
    TEST_ASSERT_TRUE(syn_mailbox_receive(&mb, &rx));
    TEST_ASSERT_EQUAL(99, rx.seq);
}

/* ── IPC notify ────────────────────────────────────────────────────────── */

void test_mbox_notify_fires_on_post(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);
    syn_mailbox_set_notify(&mb, true);
    mock_ipc_notify_count = 0;

    TestMsg tx = { .seq = 1 };
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_EQUAL(1, mock_ipc_notify_count);
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_EQUAL(2, mock_ipc_notify_count);
}

void test_mbox_notify_disabled_by_default(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);
    mock_ipc_notify_count = 0;

    TestMsg tx = { .seq = 1 };
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_EQUAL(0, mock_ipc_notify_count);
}

void test_mbox_notify_not_on_overflow(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);
    syn_mailbox_set_notify(&mb, true);

    TestMsg tx = { .seq = 0 };
    syn_mailbox_post(&mb, &tx);
    syn_mailbox_post(&mb, &tx);
    syn_mailbox_post(&mb, &tx);
    mock_ipc_notify_count = 0;

    /* Overflow: notify should NOT fire */
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_EQUAL(0, mock_ipc_notify_count);
}

void test_mbox_notify_toggle(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 0 };
    mock_ipc_notify_count = 0;

    /* Enable notify */
    syn_mailbox_set_notify(&mb, true);
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_EQUAL(1, mock_ipc_notify_count);

    /* Disable notify */
    syn_mailbox_set_notify(&mb, false);

    TestMsg rx;
    syn_mailbox_receive(&mb, &rx);
    syn_mailbox_post(&mb, &tx);
    TEST_ASSERT_EQUAL(1, mock_ipc_notify_count); /* unchanged */
}

/* ── Barrier invocation verification ───────────────────────────────────── */

void test_mbox_barriers_on_post(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    mock_barrier_count = 0;
    TestMsg tx = { .seq = 1 };
    syn_mailbox_post(&mb, &tx);
    /* LOAD_ACQUIRE(tail) + STORE_RELEASE(head) = at least 2 barriers */
    TEST_ASSERT_TRUE(mock_barrier_count >= 2);
}

void test_mbox_barriers_on_receive(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 1 };
    syn_mailbox_post(&mb, &tx);

    mock_barrier_count = 0;
    TestMsg rx;
    syn_mailbox_receive(&mb, &rx);
    /* LOAD_ACQUIRE(head) + STORE_RELEASE(tail) = at least 2 barriers */
    TEST_ASSERT_TRUE(mock_barrier_count >= 2);
}

void test_mbox_barriers_on_empty_receive(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    mock_barrier_count = 0;
    TestMsg rx;
    syn_mailbox_receive(&mb, &rx);
    /* LOAD_ACQUIRE(head) even on empty path = at least 1 barrier */
    TEST_ASSERT_TRUE(mock_barrier_count >= 1);
}

void test_mbox_barriers_on_peek(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    TestMsg tx = { .seq = 1 };
    syn_mailbox_post(&mb, &tx);

    mock_barrier_count = 0;
    syn_mailbox_peek(&mb);
    TEST_ASSERT_TRUE(mock_barrier_count >= 1);
}

void test_mbox_barriers_on_state_queries(void)
{
    uint8_t buf[4 * sizeof(TestMsg)];
    SYN_Mailbox mb;
    syn_mailbox_init(&mb, buf, sizeof(TestMsg), 4);

    mock_barrier_count = 0;
    syn_mailbox_empty(&mb);
    uint32_t empty_b = mock_barrier_count;
    TEST_ASSERT_TRUE(empty_b >= 1);

    mock_barrier_count = 0;
    syn_mailbox_full(&mb);
    TEST_ASSERT_TRUE(mock_barrier_count >= 1);

    mock_barrier_count = 0;
    syn_mailbox_pending(&mb);
    TEST_ASSERT_TRUE(mock_barrier_count >= 1);
}

/* ── Multiple independent mailboxes ────────────────────────────────────── */

void test_mbox_two_independent(void)
{
    uint8_t buf_a[4 * sizeof(TestMsg)];
    uint8_t buf_b[4 * sizeof(TestMsg)];
    SYN_Mailbox mb_a, mb_b;
    syn_mailbox_init(&mb_a, buf_a, sizeof(TestMsg), 4);
    syn_mailbox_init(&mb_b, buf_b, sizeof(TestMsg), 4);

    TestMsg tx_a = { .seq = 100 };
    TestMsg tx_b = { .seq = 200 };
    syn_mailbox_post(&mb_a, &tx_a);
    syn_mailbox_post(&mb_b, &tx_b);

    TestMsg rx;
    syn_mailbox_receive(&mb_a, &rx);
    TEST_ASSERT_EQUAL(100, rx.seq);

    syn_mailbox_receive(&mb_b, &rx);
    TEST_ASSERT_EQUAL(200, rx.seq);
}

/* ── DEFINE macro ──────────────────────────────────────────────────────── */

SYN_MAILBOX_DEFINE(test_static_mbox, TestMsg, 4);

void test_mbox_define_macro(void)
{
    /* Reset the static mailbox */
    test_static_mbox.head = 0;
    test_static_mbox.tail = 0;
    test_static_mbox.overflow = 0;

    TestMsg tx = { .seq = 42 };
    TEST_ASSERT_TRUE(syn_mailbox_post(&test_static_mbox, &tx));

    TestMsg rx;
    TEST_ASSERT_TRUE(syn_mailbox_receive(&test_static_mbox, &rx));
    TEST_ASSERT_EQUAL(42, rx.seq);
}

/* ════════════════════════════════════════════════════════════════════════
 * RUNNER
 * ════════════════════════════════════════════════════════════════════════ */

void run_multicore_tests(void)
{
    /* Barrier primitives */
    RUN_TEST(test_barrier_compiler_barrier_compiles);
    RUN_TEST(test_barrier_store_release_writes_value);
    RUN_TEST(test_barrier_store_release_invokes_hardware_barrier);
    RUN_TEST(test_barrier_load_acquire_reads_value);
    RUN_TEST(test_barrier_load_acquire_invokes_hardware_barrier);
    RUN_TEST(test_barrier_store_then_load_roundtrip);
    RUN_TEST(test_barrier_zero_value);
    RUN_TEST(test_barrier_max_value);
    RUN_TEST(test_barrier_multiple_sequential);

    /* Spinlock port */
    RUN_TEST(test_spinlock_acquire_release_basic);
    RUN_TEST(test_spinlock_acquire_release_repeated);
    RUN_TEST(test_spinlock_try_acquire_free);
    RUN_TEST(test_spinlock_try_acquire_contended);
    RUN_TEST(test_spinlock_try_acquire_after_release);
    RUN_TEST(test_spinlock_multiple_ids_independent);
    RUN_TEST(test_spinlock_all_ids);
    RUN_TEST(test_spinlock_out_of_range_acquire);
    RUN_TEST(test_spinlock_out_of_range_try_acquire);
    RUN_TEST(test_spinlock_core_id_default);
    RUN_TEST(test_spinlock_core_id_secondary);
    RUN_TEST(test_spinlock_ipc_notify_count);

    /* Scoped guard */
    RUN_TEST(test_guard_acquires_and_releases);
    RUN_TEST(test_guard_executes_body);
    RUN_TEST(test_guard_body_modifies_external_state);
    RUN_TEST(test_guard_nested_different_ids);
    RUN_TEST(test_guard_sequential_same_id);
    RUN_TEST(test_guard_empty_body);
    RUN_TEST(test_guard_with_break);

    /* Cross-core mailbox: basic */
    RUN_TEST(test_mbox_post_receive_single);
    RUN_TEST(test_mbox_empty_receive_fails);
    RUN_TEST(test_mbox_empty_after_init);

    /* Cross-core mailbox: FIFO */
    RUN_TEST(test_mbox_fifo_order);

    /* Cross-core mailbox: capacity and overflow */
    RUN_TEST(test_mbox_fill_to_capacity);
    RUN_TEST(test_mbox_overflow_counted);
    RUN_TEST(test_mbox_overflow_no_data_corruption);
    RUN_TEST(test_mbox_post_after_drain);

    /* Cross-core mailbox: wraparound */
    RUN_TEST(test_mbox_wraparound_single_slot);
    RUN_TEST(test_mbox_wraparound_multi_round);

    /* Cross-core mailbox: interleaved */
    RUN_TEST(test_mbox_interleaved_one_at_a_time);
    RUN_TEST(test_mbox_interleaved_burst);

    /* Cross-core mailbox: data integrity */
    RUN_TEST(test_mbox_struct_integrity);
    RUN_TEST(test_mbox_tiny_message);
    RUN_TEST(test_mbox_large_message);
    RUN_TEST(test_mbox_data_pattern_stress);

    /* Cross-core mailbox: peek */
    RUN_TEST(test_mbox_peek_returns_data);
    RUN_TEST(test_mbox_peek_empty_returns_null);
    RUN_TEST(test_mbox_peek_then_receive);

    /* Cross-core mailbox: state transitions */
    RUN_TEST(test_mbox_state_transitions);

    /* Cross-core mailbox: flush */
    RUN_TEST(test_mbox_flush);
    RUN_TEST(test_mbox_flush_then_reuse);

    /* Cross-core mailbox: IPC notify */
    RUN_TEST(test_mbox_notify_fires_on_post);
    RUN_TEST(test_mbox_notify_disabled_by_default);
    RUN_TEST(test_mbox_notify_not_on_overflow);
    RUN_TEST(test_mbox_notify_toggle);

    /* Cross-core mailbox: barrier verification */
    RUN_TEST(test_mbox_barriers_on_post);
    RUN_TEST(test_mbox_barriers_on_receive);
    RUN_TEST(test_mbox_barriers_on_empty_receive);
    RUN_TEST(test_mbox_barriers_on_peek);
    RUN_TEST(test_mbox_barriers_on_state_queries);

    /* Cross-core mailbox: independence and macro */
    RUN_TEST(test_mbox_two_independent);
    RUN_TEST(test_mbox_define_macro);
}
