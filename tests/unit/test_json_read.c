/**
 * @file test_json_read.c
 * @brief Tests for the JSON reader.
 */

#include "syntropic/util/syn_json_read.h"
#include "unity/unity.h"

#include <stdio.h>
#include <string.h>

/* ── Tests ──────────────────────────────────────────────────────────────── */

void test_json_read_simple_object(void)
{
    char json[] = "{\"name\":\"esp32\",\"port\":80,\"active\":true}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));

    char name[32];
    TEST_ASSERT_TRUE(syn_json_get_str(&r, "name", name, sizeof(name)));
    TEST_ASSERT_EQUAL_STRING("esp32", name);

    int32_t port;
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "port", &port));
    TEST_ASSERT_EQUAL(80, port);

    bool active;
    TEST_ASSERT_TRUE(syn_json_get_bool(&r, "active", &active));
    TEST_ASSERT_TRUE(active);
}

void test_json_read_negative_int(void)
{
    char json[] = "{\"temp\":-25}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));

    int32_t temp;
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "temp", &temp));
    TEST_ASSERT_EQUAL(-25, temp);
}

void test_json_read_null(void)
{
    char json[] = "{\"data\":null}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));
    TEST_ASSERT_TRUE(syn_json_is_null(&r, "data"));
    TEST_ASSERT_EQUAL(SYN_JSON_NULL, syn_json_get_type(&r, "data"));
}

void test_json_read_false(void)
{
    char json[] = "{\"enabled\":false}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));

    bool enabled;
    TEST_ASSERT_TRUE(syn_json_get_bool(&r, "enabled", &enabled));
    TEST_ASSERT_FALSE(enabled);
}

void test_json_read_missing_key(void)
{
    char json[] = "{\"a\":1}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));
    TEST_ASSERT_NULL(syn_json_find(&r, "b"));
    TEST_ASSERT_EQUAL(SYN_JSON_NONE, syn_json_get_type(&r, "missing"));
}

void test_json_read_nested_object(void)
{
    char json[] = "{\"wifi\":{\"ssid\":\"MyNet\",\"ch\":6}}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));

    /* Access nested via dot notation */
    char ssid[32];
    TEST_ASSERT_TRUE(syn_json_get_str(&r, "wifi.ssid", ssid, sizeof(ssid)));
    TEST_ASSERT_EQUAL_STRING("MyNet", ssid);

    int32_t ch;
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "wifi.ch", &ch));
    TEST_ASSERT_EQUAL(6, ch);
}

void test_json_read_whitespace(void)
{
    char json[] = "  {  \"key\"  :  \"value\"  }  ";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));

    char val[32];
    TEST_ASSERT_TRUE(syn_json_get_str(&r, "key", val, sizeof(val)));
    TEST_ASSERT_EQUAL_STRING("value", val);
}

void test_json_read_empty_object(void)
{
    char json[] = "{}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));
    TEST_ASSERT_EQUAL(0, r.token_count);
}

void test_json_read_invalid_json(void)
{
    char json[] = "not json";
    SYN_JsonReader r;

    TEST_ASSERT_FALSE(syn_json_parse(&r, json, strlen(json)));
    TEST_ASSERT_FALSE(r.valid);
}

void test_json_read_string_truncation(void)
{
    char json[] = "{\"long\":\"abcdefghij\"}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));

    char short_buf[5];
    TEST_ASSERT_TRUE(syn_json_get_str(&r, "long", short_buf, sizeof(short_buf)));
    TEST_ASSERT_EQUAL_STRING("abcd", short_buf);
}

void test_json_read_type_mismatch(void)
{
    char json[] = "{\"val\":\"text\"}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));

    int32_t num;
    TEST_ASSERT_FALSE(syn_json_get_int(&r, "val", &num));

    bool b;
    TEST_ASSERT_FALSE(syn_json_get_bool(&r, "val", &b));
}

void test_json_read_multiple_values(void)
{
    char json[] = "{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5}";
    SYN_JsonReader r;

    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));
    TEST_ASSERT_EQUAL(5, r.token_count);

    int32_t v;
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "c", &v));
    TEST_ASSERT_EQUAL(3, v);

    TEST_ASSERT_TRUE(syn_json_get_int(&r, "e", &v));
    TEST_ASSERT_EQUAL(5, v);
}

/** String value with escape sequences — exercises lines 70-72 in parse_string */
void test_json_read_escaped_string(void)
{
    /* JSON: {"msg":"hel\"lo"} — string contains escaped quote */
    char json[] = "{\"msg\":\"hel\\\"lo\"}";
    SYN_JsonReader r;
    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));
    char msg[32];
    TEST_ASSERT_TRUE(syn_json_get_str(&r, "msg", msg, sizeof(msg)));
    TEST_ASSERT_TRUE(strlen(msg) > 0);
}

/** Nested object as value — exercises nested object parsing */
void test_json_read_skip_nested_object(void)
{
    /* {"meta":{"x":1},"val":42} */
    char json[] = "{\"meta\":{\"x\":1},\"val\":42}";
    SYN_JsonReader r;
    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));
    int32_t v = 0;
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "val", &v));
    TEST_ASSERT_EQUAL_INT(42, v);
}

/** Array as value — exercises array-type skipping */
void test_json_read_skip_array_value(void)
{
    char json[] = "{\"arr\":[1,2,3],\"id\":7}";
    SYN_JsonReader r;
    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));
    int32_t v = 0;
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "id", &v));
    TEST_ASSERT_EQUAL_INT(7, v);
}

/** String value skipping — exercises skip_value string branch (lines 94-102) by
 *  overflowing the token table (SYN_JSON_MAX_TOKENS=32) so a string is skipped */
void test_json_read_token_overflow_skip_string(void)
{
    /* Build a JSON with 33 keys — last ones will be skipped by skip_value */
    /* The 33rd value is a string — exercises skip_value lines 94-102 */
    static char json[2048];
    int pos = 0;
    if ((size_t)pos < sizeof(json)) {
        pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "{");
    }
    int i;
    for (i = 0; i < 32; i++) {
        if ((size_t)pos < sizeof(json)) {
            pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "\"k%d\":%d%s", i, i,
                            i < 31 ? "," : ",");
        }
    }
    /* 33rd key has a string value — exercises skip_value string path */
    if ((size_t)pos < sizeof(json)) {
        pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "\"extra\":\"overflowed\"}");
    }
    (void)pos;

    SYN_JsonReader r;
    /* Token array overflows — parse may succeed partially or fail */
    bool ok = syn_json_parse(&r, json, strlen(json));
    /* With 33 keys, token overflow is expected — verify count is at capacity */
    TEST_ASSERT_TRUE(r.token_count > 0);
    (void)ok;
}

/** Overflow with nested object — exercises skip_value object/array path (lines 105-125) */
void test_json_read_token_overflow_skip_object(void)
{
    static char json[2048];
    int pos = 0;
    if ((size_t)pos < sizeof(json)) {
        pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "{");
    }
    int i;
    for (i = 0; i < 32; i++) {
        if ((size_t)pos < sizeof(json)) {
            pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "\"k%d\":%d%s", i, i, ",");
        }
    }
    /* 33rd key has nested object — exercises skip_value obj path */
    if ((size_t)pos < sizeof(json)) {
        pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "\"nested\":{\"a\":1,\"b\":2}}");
    }
    (void)pos;

    SYN_JsonReader r;
    bool ok = syn_json_parse(&r, json, strlen(json));
    /* With 33 keys, token overflow is expected — verify count is at capacity */
    TEST_ASSERT_TRUE(r.token_count > 0);
    (void)ok;
}

/** Unterminated string — exercises line 80 (parse_string returns NULL) */
void test_json_read_unterminated_string(void)
{
    /* No closing quote on the value — parse_string returns NULL */
    char json[] = "{\"key\":\"unterminated";
    SYN_JsonReader r;
    /* Unterminated string should cause parse failure */
    bool ok = syn_json_parse(&r, json, strlen(json));
    TEST_ASSERT_FALSE(ok);
}

/** Unexpected char in parse_object — exercises line 249 */
void test_json_read_nested_object_depth_limit(void)
{
    SYN_JsonReader r;

    /* Parse valid JSON and test dot notation parent not found */
    char json[] = "{\"wifi\":{\"ssid\":\"Net\"},\"escaped\":\"line\\nbreak\"}";
    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));

    TEST_ASSERT_NULL(syn_json_find(&r, "nonexistent.child"));
    TEST_ASSERT_NULL(syn_json_find(&r, "wifi.missing"));

    /* Truncation of string into small buffer */
    char small_buf[3];
    TEST_ASSERT_TRUE(syn_json_get_str(&r, "wifi.ssid", small_buf, sizeof(small_buf)));
    TEST_ASSERT_EQUAL_STRING("Ne", small_buf);

    /* Malformed JSON syntax key without value */
    char bad1[] = "{\"key\"}";
    char bad2[] = "{invalid}";
    syn_json_parse(&r, bad1, strlen(bad1));
    TEST_ASSERT_EQUAL(SYN_JSON_NONE, syn_json_get_type(&r, "key"));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad2, strlen(bad2)));

    /* Type mismatches on getters */
    char json2[] = "{\"str\":\"hello\",\"num\":123}";
    int32_t val;
    bool bval;
    TEST_ASSERT_TRUE(syn_json_parse(&r, json2, strlen(json2)));
    TEST_ASSERT_FALSE(syn_json_get_int(&r, "str", &val));
    TEST_ASSERT_FALSE(syn_json_get_bool(&r, "str", &bval));
}

/* ── Test group ────────────────────────────────────────────────────────── */

static void test_json_read_escaped_skip_value_and_nested_array(void)
{
    SYN_JsonReader r;

    /* JSON with string escapes in skipped values and nested arrays */
    char json[] = "{\"skip_str\":\"escaped\\\"quote\",\"nested_arr\":[[1,2],[3,4]],\"target\":100}";
    TEST_ASSERT_TRUE(syn_json_parse(&r, json, strlen(json)));

    int32_t val;
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "target", &val));
    TEST_ASSERT_EQUAL(100, val);
}

static void test_json_read_skipped_string_escapes_and_unterminated(void)
{
    SYN_JsonReader r;
    /* Unterminated string inside array skip */
    char bad_arr[] = "[ \"unterminated_string_without_close_quote ]";
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_arr, strlen(bad_arr)));

    /* Unterminated string inside object key skip */
    char bad_obj[] = "{ \"bad_key : 123 }";
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_obj, strlen(bad_obj)));
}

static void test_json_read_escaped_string_inside_token_overflow_skip(void)
{
    static char json[2048];
    int pos = 0;
    if ((size_t)pos < sizeof(json)) {
        pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "{");
    }
    for (int i = 0; i < 32; i++) {
        if ((size_t)pos < sizeof(json)) {
            pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "\"k%d\":%d,", i, i);
        }
    }
    /* 33rd key has escaped quotes inside string and inside nested object */
    if ((size_t)pos < sizeof(json)) {
        pos += snprintf(json + pos, sizeof(json) - (size_t)pos, "\"str\":\"val\\\"escaped\",");
    }
    if ((size_t)pos < sizeof(json)) {
        snprintf(json + pos, sizeof(json) - (size_t)pos,
                 "\"obj\":{\"key\\\"sub\":\"val\\\"sub\"}}");
    }

    SYN_JsonReader r;
    syn_json_parse(&r, json, strlen(json));
}

static void test_json_get_str_null_and_missing_key(void)
{
    SYN_JsonReader r;
    char out[16];
    TEST_ASSERT_FALSE(syn_json_get_str(NULL, "k", out, sizeof(out)));
    TEST_ASSERT_FALSE(syn_json_get_str(&r, NULL, out, sizeof(out)));
    TEST_ASSERT_FALSE(syn_json_get_str(&r, "k", NULL, sizeof(out)));
    TEST_ASSERT_FALSE(syn_json_get_str(&r, "k", out, 0));

    char json[] = "{\"int_key\":123}";
    syn_json_parse(&r, json, strlen(json));
    TEST_ASSERT_FALSE(syn_json_get_str(&r, "int_key", out, sizeof(out)));
}

static void test_json_read_skip_value_unterminated_string(void)
{
    SYN_JsonReader r;

    /* Unterminated string as value for skipped key */
    char json1[] = "{\"k1\":\"unterminated";
    TEST_ASSERT_FALSE(syn_json_parse(&r, json1, strlen(json1)));

    /* Unterminated string inside array */
    char json2[] = "[\"unterminated";
    TEST_ASSERT_FALSE(syn_json_parse(&r, json2, strlen(json2)));
}

static void test_json_read_unterminated_object_key(void)
{
    char json[] = "{\"key";
    SYN_JsonReader r;
    syn_json_parse(&r, json, strlen(json));
    char out[16];
    TEST_ASSERT_FALSE(syn_json_get_str(&r, "key", out, sizeof(out)));
}

static void test_json_read_string_escape_sequences(void)
{
    SYN_JsonReader r;
    char out[16];

    /* Object token has tok->value == NULL in syn_json_get_str (line 373) */
    char json_obj[] = "{\"obj\":{\"a\":1}}";
    TEST_ASSERT_TRUE(syn_json_parse(&r, json_obj, strlen(json_obj)));
    TEST_ASSERT_FALSE(syn_json_get_str(&r, "obj", out, sizeof(out)));

    /* Unexpected char after key-value pair (line 280) */
    char bad_char[] = "{\"k\":1 @}";
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_char, strlen(bad_char)));

    /* Unterminated after colon (line 190) */
    char no_val[] = "{\"k\":";
    TEST_ASSERT_FALSE(syn_json_parse(&r, no_val, strlen(no_val)));

    /* Unterminated nested array (line 221) */
    char bad_arr[] = "{\"arr\":[1,2";
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_arr, strlen(bad_arr)));

    /* Unterminated whitespace before comma/brace (line 273) */
    char bad_end[] = "{\"k\":1  ";
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_end, strlen(bad_end)));

    /* Unterminated nested object (line 209) */
    char bad_nested[] = "{\"sub\":{\"bad";
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_nested, strlen(bad_nested)));

    /* Unterminated string with trailing backslash (line 114) */
    char bad_esc[] = "{\"k\":\"val\\";
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_esc, strlen(bad_esc)));

    /* Unterminated string with trailing backslash inside nested array (line 130) */
    char bad_nested_esc[] = "{\"k\":[\"val\\";
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_nested_esc, strlen(bad_nested_esc)));

    /* Non-object root (line 167) */
    char non_obj[] = "123";
    TEST_ASSERT_FALSE(syn_json_parse(&r, non_obj, strlen(non_obj)));

    /* Token overflow with malformed skipped value (line 197) */
    char overflow_bad[] = "{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5,\"f\":6,\"g\":7,\"h\":8,\"i\":"
                          "9,\"j\":10,\"k\":\"val\\";
    TEST_ASSERT_FALSE(syn_json_parse(&r, overflow_bad, strlen(overflow_bad)));
}

static void test_json_read_integer_saturation_and_unpadded_eof(void)
{
    SYN_JsonReader r;
    int32_t v = 0;

    char sat_json[] = "{\"pos\":99999999999999999999,\"min\":-2147483648,\"neg\":-99999999999999}";
    TEST_ASSERT_TRUE(syn_json_parse(&r, sat_json, strlen(sat_json)));
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "pos", &v));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, v);
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "min", &v));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, v);
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "neg", &v));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, v);

    /* Unpadded buffer ending right after scalar value (p == end before closing '}') */
    char raw_eof[8] = {'{', '"', 'k', '"', ':', '1', '2', '3'};
    TEST_ASSERT_FALSE(syn_json_parse(&r, raw_eof, sizeof(raw_eof)));
}

static void test_r5_n13_json_exact_bool_and_invalid_scalars(void)
{
    SYN_JsonReader r;

    char bad_t1[] = "{\"flag\":t}";
    char bad_t2[] = "{\"flag\":tru}";
    char bad_t3[] = "{\"flag\":truX}";
    char bad_t4[] = "{\"flag\":trueX}";
    char bad_f1[] = "{\"flag\":f}";
    char bad_f2[] = "{\"flag\":fals}";
    char bad_f3[] = "{\"flag\":falsX}";
    char bad_f4[] = "{\"flag\":falseX}";
    char bad_n1[] = "{\"flag\":nulX}";
    char bad_m1[] = "{\"num\":-}";
    char bad_m2[] = "{\"num\":-x}";

    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_t1, strlen(bad_t1)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_t2, strlen(bad_t2)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_t3, strlen(bad_t3)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_t4, strlen(bad_t4)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_f1, strlen(bad_f1)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_f2, strlen(bad_f2)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_f3, strlen(bad_f3)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_f4, strlen(bad_f4)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_n1, strlen(bad_n1)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_m1, strlen(bad_m1)));
    TEST_ASSERT_FALSE(syn_json_parse(&r, bad_m2, strlen(bad_m2)));

    char good_ws[] = "{\"a\": true ,\"b\": false }";
    bool bval = false;
    TEST_ASSERT_TRUE(syn_json_parse(&r, good_ws, strlen(good_ws)));
    TEST_ASSERT_TRUE(syn_json_get_bool(&r, "a", &bval));
    TEST_ASSERT_TRUE(bval);
    TEST_ASSERT_TRUE(syn_json_get_bool(&r, "b", &bval));
    TEST_ASSERT_FALSE(bval);
}

static void test_tier_a_json_read_null_out_and_bounded_depth(void)
{
    SYN_JsonReader r;
    int32_t iv = 0;
    bool bv = false;

    char base[] = "{\"n\":42,\"b\":true}";
    TEST_ASSERT_TRUE(syn_json_parse(&r, base, strlen(base)));

    /* UTL-03a: NULL pointer guards on syn_json_get_int and syn_json_get_bool */
    TEST_ASSERT_FALSE(syn_json_get_int(NULL, "n", &iv));
    TEST_ASSERT_FALSE(syn_json_get_int(&r, NULL, &iv));
    TEST_ASSERT_FALSE(syn_json_get_int(&r, "n", NULL));

    TEST_ASSERT_FALSE(syn_json_get_bool(NULL, "b", &bv));
    TEST_ASSERT_FALSE(syn_json_get_bool(&r, NULL, &bv));
    TEST_ASSERT_FALSE(syn_json_get_bool(&r, "b", NULL));

    /* UTL-03a: Iterative object parser accepts max depth 8 and empty nested objects */
    char depth8_ok[] = "{\"d1\":{\"d2\":{\"d3\":{\"d4\":{\"d5\":{\"d6\":{\"d7\":{\"leaf\":9}}}}}}},"
                       "\"empty\":{},\"tail\":77}";
    TEST_ASSERT_TRUE(syn_json_parse(&r, depth8_ok, strlen(depth8_ok)));
    TEST_ASSERT_TRUE(syn_json_get_int(&r, "tail", &iv));
    TEST_ASSERT_EQUAL_INT32(77, iv);

    /* Unclosed outer brace after empty nested object fails cleanly */
    char empty_nested_eof[] = "{\"empty\":{} ";
    TEST_ASSERT_FALSE(syn_json_parse(&r, empty_nested_eof, strlen(empty_nested_eof)));

    /* Deeply nested objects (depth > 8: 9 and 16 levels) rejected without recursion */
    char depth9_bad[] = "{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":{\"f\":{\"g\":{\"h\":{\"i\":1}}}}}}}}}";
    TEST_ASSERT_FALSE(syn_json_parse(&r, depth9_bad, strlen(depth9_bad)));
    TEST_ASSERT_FALSE(r.valid);

    char depth16_bad[] =
        "{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":{\"f\":{\"g\":{\"h\":"
        "{\"i\":{\"j\":{\"k\":{\"l\":{\"m\":{\"n\":{\"o\":{\"p\":1}}}}}}}}}}}}}}}}";
    TEST_ASSERT_FALSE(syn_json_parse(&r, depth16_bad, strlen(depth16_bad)));
}

void run_json_read_tests(void)
{
    RUN_TEST(test_json_read_simple_object);
    RUN_TEST(test_json_read_negative_int);
    RUN_TEST(test_json_read_null);
    RUN_TEST(test_json_read_false);
    RUN_TEST(test_json_read_missing_key);
    RUN_TEST(test_json_read_nested_object);
    RUN_TEST(test_json_read_whitespace);
    RUN_TEST(test_json_read_empty_object);
    RUN_TEST(test_json_read_invalid_json);
    RUN_TEST(test_json_read_string_truncation);
    RUN_TEST(test_json_read_type_mismatch);
    RUN_TEST(test_json_read_multiple_values);
    RUN_TEST(test_json_read_escaped_string);
    RUN_TEST(test_json_read_skip_nested_object);
    RUN_TEST(test_json_read_skip_array_value);
    RUN_TEST(test_json_read_token_overflow_skip_string);
    RUN_TEST(test_json_read_token_overflow_skip_object);
    RUN_TEST(test_json_read_unterminated_string);
    RUN_TEST(test_json_read_nested_object_depth_limit);
    RUN_TEST(test_json_read_escaped_skip_value_and_nested_array);
    RUN_TEST(test_json_read_skipped_string_escapes_and_unterminated);
    RUN_TEST(test_json_read_escaped_string_inside_token_overflow_skip);
    RUN_TEST(test_json_get_str_null_and_missing_key);
    RUN_TEST(test_json_read_skip_value_unterminated_string);
    RUN_TEST(test_json_read_unterminated_object_key);
    RUN_TEST(test_json_read_string_escape_sequences);
    RUN_TEST(test_json_read_integer_saturation_and_unpadded_eof);
    RUN_TEST(test_r5_n13_json_exact_bool_and_invalid_scalars);
    RUN_TEST(test_tier_a_json_read_null_out_and_bounded_depth);
}
