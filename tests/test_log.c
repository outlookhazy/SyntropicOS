/**
 * @file test_log.c
 * @brief Unity tests for syn_log.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/log/syn_log.h"

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
    TEST_ASSERT_EQUAL_STRING("raw text\n", log_capture_buf);

    syn_log_init(NULL, SYN_LOG_DEBUG);
    syn_log(SYN_LOG_INFO, "test", "no crash");
    TEST_ASSERT_TRUE(1);
}

void run_log_tests(void)
{
    RUN_TEST(test_logging);
}
