/**
 * @file test_param.c
 * @brief Unity tests for syn_param.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/storage/syn_param.h"

typedef struct {
    uint16_t brightness;
    int16_t  offset;
    uint8_t  mode;
    uint8_t  _pad;
} TestParams;

static void test_param_store(void)
{

    /* Erase flash */
    memset(mock_flash, 0xFF, sizeof(mock_flash));

    SYN_ParamStore store;
    /* 4 sectors of 1024 bytes each */
    SYN_Status st = syn_param_init(&store, 0, 4, sizeof(TestParams));
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* Save defaults */
    TestParams params = { .brightness = 80, .offset = -10, .mode = 3 };
    st = syn_param_save(&store, &params);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Load back */
    TestParams loaded;
    memset(&loaded, 0, sizeof(loaded));
    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(80, loaded.brightness);
    TEST_ASSERT_EQUAL_INT(-10, loaded.offset);
    TEST_ASSERT_EQUAL_INT(3, loaded.mode);

    /* Save again (goes to next slot) */
    params.brightness = 90;
    st = syn_param_save(&store, &params);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Re-init and load (should find latest) */
    SYN_ParamStore store2;
    st = syn_param_init(&store2, 0, 4, sizeof(TestParams));
    TEST_ASSERT_EQUAL(SYN_OK, st);

    memset(&loaded, 0, sizeof(loaded));
    st = syn_param_load(&store2, &loaded);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(90, loaded.brightness);

    /* Wear leveling: save many times and verify it wraps sectors */
    int i;
    for (i = 0; i < 200; i++) {
        params.brightness = (uint16_t)(i & 0xFFFF);
        st = syn_param_save(&store, &params);
        TEST_ASSERT_EQUAL(SYN_OK, st);
    }

    memset(&loaded, 0, sizeof(loaded));
    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(199, loaded.brightness);

    /* Erase all */
    st = syn_param_erase_all(&store);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    st = syn_param_load(&store, &loaded);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    /* Write count */
    uint16_t wc = syn_param_write_count(&store);
    (void)wc; /* just verify it doesn't crash */
}

void run_param_tests(void)
{
    RUN_TEST(test_param_store);
}
