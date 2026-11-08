/**
 * @file test_at_parser.c
 * @brief Unity tests for syn_at_parser.
 */

#include "mocks/mock_port.h"
#include "syntropic/proto/syn_at_parser.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

static char line_buf[128];
static SYN_AtParser parser;

static void test_at_parser_init(void)
{
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_at_parser_init(&parser, line_buf, sizeof(line_buf)));
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, parser.last_resp);
    TEST_ASSERT_EQUAL_INT(0, parser.line_len);
    TEST_ASSERT_EQUAL_INT(-1, syn_at_parser_get_cme_error(&parser));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_at_parser_init(NULL, line_buf, sizeof(line_buf)));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_at_parser_init(&parser, NULL, 0));
}

static void test_at_parser_null_safety(void)
{
    syn_at_parser_reset(NULL);
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, syn_at_parser_feed_char(NULL, 'A'));

    SYN_Stream stream;
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, syn_at_parser_feed_stream(NULL, &stream));
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, syn_at_parser_feed_stream(&parser, NULL));

    TEST_ASSERT_EQUAL_STRING("", syn_at_parser_get_line(NULL));
    TEST_ASSERT_EQUAL_INT(-1, syn_at_parser_get_cme_error(NULL));

    int val = 0;
    char buf[16];
    TEST_ASSERT_FALSE(syn_at_parser_get_param_int(NULL, 0, &val));
    TEST_ASSERT_FALSE(syn_at_parser_get_param_int("10,20", 0, NULL));

    TEST_ASSERT_FALSE(syn_at_parser_get_param_str(NULL, 0, buf, sizeof(buf)));
    TEST_ASSERT_FALSE(syn_at_parser_get_param_str("OK", 0, NULL, sizeof(buf)));
    TEST_ASSERT_FALSE(syn_at_parser_get_param_str("OK", 0, buf, 0));
}

static void test_at_parser_ok_error(void)
{
    syn_at_parser_init(&parser, line_buf, sizeof(line_buf));

    /* OK feed */
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, syn_at_parser_feed_char(&parser, 'O'));
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, syn_at_parser_feed_char(&parser, 'K'));
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, syn_at_parser_feed_char(&parser, '\r'));
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_OK, syn_at_parser_feed_char(&parser, '\n'));
    TEST_ASSERT_EQUAL_STRING("OK", syn_at_parser_get_line(&parser));

    /* ERROR feed */
    const char *err_str = "ERROR\r\n";
    SYN_AtRespType resp = SYN_AT_RESP_NONE;
    for (size_t i = 0; err_str[i] != '\0'; i++) {
        resp = syn_at_parser_feed_char(&parser, err_str[i]);
    }
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_ERROR, resp);
    TEST_ASSERT_EQUAL_STRING("ERROR", syn_at_parser_get_line(&parser));
}

static void test_at_parser_cme_cms_error(void)
{
    syn_at_parser_init(&parser, line_buf, sizeof(line_buf));

    const char *cme_str = "+CME ERROR: 10\r\n";
    SYN_AtRespType resp = SYN_AT_RESP_NONE;
    for (size_t i = 0; cme_str[i] != '\0'; i++) {
        resp = syn_at_parser_feed_char(&parser, cme_str[i]);
    }

    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_CME_ERROR, resp);
    TEST_ASSERT_EQUAL_INT(10, syn_at_parser_get_cme_error(&parser));

    /* CMS ERROR */
    const char *cms_str = "+CMS ERROR: 301\r\n";
    for (size_t i = 0; cms_str[i] != '\0'; i++) {
        resp = syn_at_parser_feed_char(&parser, cms_str[i]);
    }

    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_CME_ERROR, resp);
    TEST_ASSERT_EQUAL_INT(301, syn_at_parser_get_cme_error(&parser));
}

static void test_at_parser_prompt(void)
{
    syn_at_parser_init(&parser, line_buf, sizeof(line_buf));

    SYN_AtRespType resp = syn_at_parser_feed_char(&parser, '>');
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_PROMPT, resp);
    TEST_ASSERT_TRUE(parser.prompt_detected);
}

static void test_at_parser_urc(void)
{
    syn_at_parser_init(&parser, line_buf, sizeof(line_buf));

    const char *urc_list[] = {"+RECEIVE,0,17\r\n", "+IPD,5:hello\r\n", "CLOSED\r\n",
                              "SHUT OK\r\n",       "RING\r\n",         "+CMTI: \"SM\",1\r\n"};

    for (size_t k = 0; k < sizeof(urc_list) / sizeof(urc_list[0]); k++) {
        SYN_AtRespType resp = SYN_AT_RESP_NONE;
        for (size_t i = 0; urc_list[k][i] != '\0'; i++) {
            resp = syn_at_parser_feed_char(&parser, urc_list[k][i]);
        }
        TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_URC, resp);
    }
}

static void test_at_parser_stream(void)
{
    syn_at_parser_init(&parser, line_buf, sizeof(line_buf));

    uint8_t stream_backing[64];
    SYN_Stream stream;
    syn_stream_init(&stream, stream_backing, sizeof(stream_backing));

    const char *msg = "+CSQ: 20,0\r\n";
    for (size_t i = 0; msg[i] != '\0'; i++) {
        syn_stream_put(&stream, (uint8_t)msg[i]);
    }

    SYN_AtRespType resp = syn_at_parser_feed_stream(&parser, &stream);
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_LINE, resp);
    TEST_ASSERT_EQUAL_STRING("+CSQ: 20,0", syn_at_parser_get_line(&parser));

    /* Param extraction */
    int rssi = 0, ber = 0;
    TEST_ASSERT_TRUE(syn_at_parser_get_param_int(syn_at_parser_get_line(&parser), 0, &rssi));
    TEST_ASSERT_TRUE(syn_at_parser_get_param_int(syn_at_parser_get_line(&parser), 1, &ber));
    TEST_ASSERT_EQUAL_INT(20, rssi);
    TEST_ASSERT_EQUAL_INT(0, ber);
}

static void test_at_parser_param_str(void)
{
    const char *line = "+CPIN: \"READY\", 1";

    char status[16];
    int extra = 0;

    TEST_ASSERT_TRUE(syn_at_parser_get_param_str(line, 0, status, sizeof(status)));
    TEST_ASSERT_EQUAL_STRING("READY", status);

    TEST_ASSERT_TRUE(syn_at_parser_get_param_int(line, 1, &extra));
    TEST_ASSERT_EQUAL_INT(1, extra);

    TEST_ASSERT_FALSE(syn_at_parser_get_param_str(line, 5, status, sizeof(status)));

    /* Param without colon prefix */
    const char *no_colon = "100, 200";
    int val1 = 0, val2 = 0;
    TEST_ASSERT_TRUE(syn_at_parser_get_param_int(no_colon, 0, &val1));
    TEST_ASSERT_TRUE(syn_at_parser_get_param_int(no_colon, 1, &val2));
    TEST_ASSERT_EQUAL_INT(100, val1);
    TEST_ASSERT_EQUAL_INT(200, val2);
}

static void test_at_parser_overflow(void)
{
    char small_buf[8];
    SYN_AtParser small_parser;
    syn_at_parser_init(&small_parser, small_buf, sizeof(small_buf));

    /* Feed string longer than 8 bytes */
    const char *long_msg = "VERYLONGSTRING\r\n";
    SYN_AtRespType resp = SYN_AT_RESP_NONE;
    for (size_t i = 0; long_msg[i] != '\0'; i++) {
        resp = syn_at_parser_feed_char(&small_parser, long_msg[i]);
    }

    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_LINE, resp);

    /* Feed empty line leading \n (line 75) */
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, syn_at_parser_feed_char(&small_parser, '\n'));

    /* CME ERROR parsing (lines 87-90) */
    char buf[64];
    SYN_AtParser p;
    syn_at_parser_init(&p, buf, sizeof(buf));
    const char *cme_err = "+CME ERROR: 100\r\n";
    for (size_t i = 0; cme_err[i] != '\0'; i++) {
        resp = syn_at_parser_feed_char(&p, cme_err[i]);
    }
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_CME_ERROR, resp);
    TEST_ASSERT_EQUAL_INT(100, p.cme_error_code);

    /* Stream end returning SYN_AT_RESP_NONE (line 128) */
    uint8_t dummy_buf[16];
    SYN_Stream empty_stream;
    syn_stream_init(&empty_stream, dummy_buf, sizeof(dummy_buf));
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, syn_at_parser_feed_stream(&p, &empty_stream));
    TEST_ASSERT_EQUAL_STRING("VERYLON", syn_at_parser_get_line(&small_parser));
}

static void test_at_parser_cmd_formatting_and_overflow(void)
{
    /* 1. Uninitialized parser line_buf == NULL */
    SYN_AtParser null_buf_parser;
    memset(&null_buf_parser, 0, sizeof(null_buf_parser));
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_NONE, syn_at_parser_feed_char(&null_buf_parser, 'A'));

    /* 2. CME ERROR without colon (categorized as SYN_AT_RESP_LINE) */
    syn_at_parser_init(&parser, line_buf, sizeof(line_buf));
    const char *cme_no_colon = "+CME ERROR\r\n";
    SYN_AtRespType resp = SYN_AT_RESP_NONE;
    for (size_t i = 0; cme_no_colon[i] != '\0'; i++) {
        resp = syn_at_parser_feed_char(&parser, cme_no_colon[i]);
    }
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_LINE, resp);

    /* 3. CME ERROR with colon but no number */
    syn_at_parser_init(&parser, line_buf, sizeof(line_buf));
    const char *cme_no_code = "+CME ERROR:\r\n";
    resp = SYN_AT_RESP_NONE;
    for (size_t i = 0; cme_no_code[i] != '\0'; i++) {
        resp = syn_at_parser_feed_char(&parser, cme_no_code[i]);
    }
    TEST_ASSERT_EQUAL_INT(SYN_AT_RESP_CME_ERROR, resp);
    TEST_ASSERT_EQUAL_INT(0, syn_at_parser_get_cme_error(&parser));

    /* 4. Empty parameter in string/int parsing */
    int val = 0;
    TEST_ASSERT_FALSE(syn_at_parser_get_param_int("+CSQ: ,1", 0, &val));
    TEST_ASSERT_TRUE(syn_at_parser_get_param_int("+CSQ: ,1", 1, &val));
    TEST_ASSERT_EQUAL_INT(1, val);
    TEST_ASSERT_FALSE(syn_at_parser_get_param_int("+CSQ: 1", 5, &val));
    TEST_ASSERT_FALSE(syn_at_parser_get_param_int("", 0, &val));
}

static void test_at_parser_response_delimiter_handling(void)
{
    /* 1. Unquoted string parameter extraction */
    char str_buf[32];
    TEST_ASSERT_TRUE(syn_at_parser_get_param_str("+CMGR: READ, 100", 0, str_buf, sizeof(str_buf)));
    TEST_ASSERT_EQUAL_STRING("READ", str_buf);

    /* 2. Multiple parameter index seeking without colon */
    TEST_ASSERT_TRUE(syn_at_parser_get_param_str("VER1, VER2, VER3", 2, str_buf, sizeof(str_buf)));
    TEST_ASSERT_EQUAL_STRING("VER3", str_buf);

    /* 3. Parameter index out of bounds */
    TEST_ASSERT_FALSE(syn_at_parser_get_param_str("VAL1, VAL2", 5, str_buf, sizeof(str_buf)));
}

static void test_r5_at_parser_quoted_delims_and_strtol(void)
{
    char str_buf[64];
    int val = 0;

    /* 1. Quoted string containing both ',' and ':' after header prefix */
    const char *cclk = "+CCLK: \"26/11/07,13:40:00+00\",1";
    TEST_ASSERT_TRUE(syn_at_parser_get_param_str(cclk, 0U, str_buf, sizeof(str_buf)));
    TEST_ASSERT_EQUAL_STRING("26/11/07,13:40:00+00", str_buf);
    TEST_ASSERT_TRUE(syn_at_parser_get_param_int(cclk, 1U, &val));
    TEST_ASSERT_EQUAL_INT(1, val);

    /* 2. Quoted string containing ':' and ',' with NO header prefix */
    const char *raw_quoted = "\"http://host:8080/a,b\", 443";
    TEST_ASSERT_TRUE(syn_at_parser_get_param_str(raw_quoted, 0U, str_buf, sizeof(str_buf)));
    TEST_ASSERT_EQUAL_STRING("http://host:8080/a,b", str_buf);
    TEST_ASSERT_TRUE(syn_at_parser_get_param_int(raw_quoted, 1U, &val));
    TEST_ASSERT_EQUAL_INT(443, val);

    /* 3. Strict strtol validation: non-digits, trailing garbage, overflow, whitespace */
    TEST_ASSERT_FALSE(syn_at_parser_get_param_int("+CSQ: abc,1", 0U, &val));
    TEST_ASSERT_FALSE(syn_at_parser_get_param_int("+CSQ: 12abc,1", 0U, &val));
    TEST_ASSERT_FALSE(syn_at_parser_get_param_int("+CSQ: 3000000000,1", 0U, &val));
    TEST_ASSERT_FALSE(syn_at_parser_get_param_int("+CSQ: 99999999999999999999999,1", 0U, &val));
    TEST_ASSERT_TRUE(syn_at_parser_get_param_int("+CSQ: \t-42\t , 7\r\n", 0U, &val));
    TEST_ASSERT_EQUAL_INT(-42, val);
    TEST_ASSERT_TRUE(syn_at_parser_get_param_int("+CSQ: \t-42\t , 7\r\n", 1U, &val));
    TEST_ASSERT_EQUAL_INT(7, val);
}

void run_at_parser_tests(void)
{
    RUN_TEST(test_at_parser_init);
    RUN_TEST(test_at_parser_null_safety);
    RUN_TEST(test_at_parser_ok_error);
    RUN_TEST(test_at_parser_cme_cms_error);
    RUN_TEST(test_at_parser_prompt);
    RUN_TEST(test_at_parser_urc);
    RUN_TEST(test_at_parser_stream);
    RUN_TEST(test_at_parser_param_str);
    RUN_TEST(test_at_parser_overflow);
    RUN_TEST(test_at_parser_cmd_formatting_and_overflow);
    RUN_TEST(test_at_parser_response_delimiter_handling);
    RUN_TEST(test_r5_at_parser_quoted_delims_and_strtol);
}
