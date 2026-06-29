/**
 * @file test_version.c
 * @brief Unity tests for syn_version.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/system/syn_version.h"

static void test_version(void)
{

    const SYN_Version *v = syn_version();
    TEST_ASSERT_TRUE(v != NULL);
    TEST_ASSERT_EQUAL(SYN_VERSION_MAJOR, v->major);
    TEST_ASSERT_EQUAL(SYN_VERSION_MINOR, v->minor);
    TEST_ASSERT_EQUAL(SYN_VERSION_PATCH, v->patch);
    TEST_ASSERT_EQUAL(SYN_VERSION_CODE, v->code);
    TEST_ASSERT_TRUE(v->date != NULL);
    TEST_ASSERT_TRUE(v->time != NULL);
    TEST_ASSERT_TRUE(v->git_hash != NULL);
    TEST_ASSERT_TRUE(v->app_name != NULL);

    TEST_ASSERT_TRUE(syn_version_at_least(0, 0, 0));
    TEST_ASSERT_TRUE(syn_version_at_least(0, 1, 0));
    TEST_ASSERT_FALSE(syn_version_at_least(255, 0, 0));
}

void run_version_tests(void)
{
    RUN_TEST(test_version);
}
