/**
 * @file test_json_read.c
 * @brief Tests for the JSON reader.
 */

#include "unity/unity.h"
#include "syntropic/util/syn_json_read.h"

#include <string.h>
#include <stdio.h>

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

/* ── Test group ────────────────────────────────────────────────────────── */

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
}
