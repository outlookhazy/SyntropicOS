/**
 * @file test_coredump.c
 * @brief Unity tests for syn_coredump — persistent core dump to flash.
 *
 * Uses the mock flash backend to verify save/read/clear and CRC
 * integrity checking.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"

/*
 * SYN_USE_COREDUMP=1 and SYN_COREDUMP_FLASH_ADDR=0 are defined in
 * Makefile.unity CFLAGS. Stack size uses the default (128).
 */

#include "syntropic/system/syn_coredump.h"
#include "syntropic/util/syn_crc.h"

#include <string.h>

/* ── Tests ──────────────────────────────────────────────────────────────── */

/** Save a core dump and read it back successfully. */
static void test_coredump_save_and_read(void)
{
    /* Provide a fake stack buffer for the coredump to capture from */
    static uint8_t fake_stack[256];
    memset(fake_stack, 0x42, sizeof(fake_stack));

    SYN_FaultContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.pc   = 0x08001234;
    ctx.lr   = 0x08001200;
    ctx.sp   = (uint32_t)(uintptr_t)fake_stack;
    ctx.r0   = 0x11;
    ctx.r1   = 0x22;
    ctx.r2   = 0x33;
    ctx.r3   = 0x44;
    ctx.r12  = 0x55;
    ctx.xpsr = 0x01000000;

    mock_tick_ms = 42000;

    syn_coredump_save(&ctx);

    SYN_CoreDump dump;
    bool found = syn_coredump_read(&dump);
    TEST_ASSERT_TRUE(found);

    TEST_ASSERT_EQUAL_HEX32(SYN_COREDUMP_MAGIC, dump.magic);
    TEST_ASSERT_EQUAL_HEX32(0x08001234, dump.regs.pc);
    TEST_ASSERT_EQUAL_HEX32(0x08001200, dump.regs.lr);
    TEST_ASSERT_EQUAL_HEX32(0x11, dump.regs.r0);
    TEST_ASSERT_EQUAL(42000, dump.uptime_ms);
    /* Stack capture is disabled on 64-bit hosts (pointer truncation) */
#if UINTPTR_MAX == 0xFFFFFFFFu
    TEST_ASSERT_EQUAL(128, dump.stack_bytes);
#else
    TEST_ASSERT_EQUAL(0, dump.stack_bytes);
#endif
}

/** Read with no valid dump returns false. */
static void test_coredump_read_empty(void)
{
    /* Flash is all 0xFF after reset — no valid dump */
    memset(mock_flash, 0xFF, MOCK_FLASH_SIZE);

    SYN_CoreDump dump;
    bool found = syn_coredump_read(&dump);
    TEST_ASSERT_FALSE(found);
}

/** Corrupt the CRC → read should fail. */
static void test_coredump_corrupt_crc(void)
{
    SYN_FaultContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.pc = 0x08002000;
    ctx.sp = (uint32_t)(uintptr_t)mock_flash;

    syn_coredump_save(&ctx);

    /* Corrupt one byte in flash (in the CRC field area) */
    SYN_CoreDump *raw = (SYN_CoreDump *)mock_flash;
    raw->crc ^= 0x01;

    SYN_CoreDump dump;
    bool found = syn_coredump_read(&dump);
    TEST_ASSERT_FALSE(found);
}

/** Corrupt the magic → read should fail. */
static void test_coredump_corrupt_magic(void)
{
    SYN_FaultContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.pc = 0x08002000;
    ctx.sp = (uint32_t)(uintptr_t)mock_flash;

    syn_coredump_save(&ctx);

    /* Corrupt magic */
    SYN_CoreDump *raw = (SYN_CoreDump *)mock_flash;
    raw->magic = 0xDEADBEEF;

    SYN_CoreDump dump;
    bool found = syn_coredump_read(&dump);
    TEST_ASSERT_FALSE(found);
}

/** Clear erases the dump so subsequent reads fail. */
static void test_coredump_clear(void)
{
    SYN_FaultContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.pc = 0x08003000;
    ctx.sp = (uint32_t)(uintptr_t)mock_flash;

    syn_coredump_save(&ctx);

    SYN_CoreDump dump;
    TEST_ASSERT_TRUE(syn_coredump_read(&dump));

    syn_coredump_clear();
    TEST_ASSERT_FALSE(syn_coredump_read(&dump));
}

/** Save with NULL context is a safe no-op. */
static void test_coredump_save_null(void)
{
    memset(mock_flash, 0xFF, MOCK_FLASH_SIZE);
    syn_coredump_save(NULL);

    SYN_CoreDump dump;
    TEST_ASSERT_FALSE(syn_coredump_read(&dump));
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

void run_coredump_tests(void)
{
    RUN_TEST(test_coredump_save_and_read);
    RUN_TEST(test_coredump_read_empty);
    RUN_TEST(test_coredump_corrupt_crc);
    RUN_TEST(test_coredump_corrupt_magic);
    RUN_TEST(test_coredump_clear);
    RUN_TEST(test_coredump_save_null);
}
