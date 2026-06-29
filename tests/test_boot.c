/**
 * @file test_boot.c
 * @brief Unity tests for syn_boot.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/system/syn_boot.h"

static void test_boot(void)
{

    /* Fresh flash */
    memset(mock_flash, 0xFF, sizeof(mock_flash));

    SYN_ParamStore boot_store;
    SYN_Status st = syn_param_init(&boot_store, 0, 2, sizeof(SYN_BootData));
    /* No data yet */

    SYN_Boot boot;
    st = syn_boot_init(&boot, &boot_store, 3);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_EQUAL_INT(1, syn_boot_count(&boot));
    TEST_ASSERT_FALSE(syn_boot_is_safe_mode(&boot));

    /* Mark healthy */
    st = syn_boot_mark_healthy(&boot);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Simulate reboot (re-init) */
    SYN_ParamStore boot_store2;
    syn_param_init(&boot_store2, 0, 2, sizeof(SYN_BootData));

    SYN_Boot boot2;
    st = syn_boot_init(&boot2, &boot_store2, 3);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(2, syn_boot_count(&boot2));
    TEST_ASSERT_EQUAL_INT(0, syn_boot_fail_count(&boot2));

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
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_FALSE(syn_boot_is_safe_mode(&bsafe));
}

void run_boot_tests(void)
{
    RUN_TEST(test_boot);
}
