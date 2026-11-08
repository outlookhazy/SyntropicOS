/**
 * @file test_cbor.c
 * @brief Unity tests for syn_cbor_write and syn_cbor_read.
 *
 * Encoding test vectors verified against RFC 8949 Appendix A.
 *
 * Notation: all expected arrays are literal byte sequences.
 *   uint(0)    = {0x00}
 *   uint(23)   = {0x17}
 *   uint(24)   = {0x18, 0x18}
 *   uint(255)  = {0x18, 0xFF}
 *   uint(256)  = {0x19, 0x01, 0x00}
 *   uint(65535)= {0x19, 0xFF, 0xFF}
 *   uint(65536)= {0x1A, 0x00, 0x01, 0x00, 0x00}
 *   int(-1)    = {0x20}
 *   int(-24)   = {0x37}
 *   int(-25)   = {0x38, 0x18}
 *   true       = {0xF5}
 *   false      = {0xF4}
 *   null       = {0xF6}
 *   float(23.5)= {0xFA, 0x41, 0xBC, 0x00, 0x00}
 *   text("hi") = {0x62, 0x68, 0x69}
 *   bytes(AB CD)= {0x42, 0xAB, 0xCD}
 *   array(2){1,2}= {0x82, 0x01, 0x02}
 *   map(1){1:2} = {0xA1, 0x01, 0x02}
 */

#include "syntropic/util/syn_cbor_read.h"
#include "syntropic/util/syn_cbor_write.h"
#include "unity/unity.h"

#include <string.h>

static uint8_t buf[128];
static SYN_CborWriter w;
static SYN_CborReader r;

/* ════════════════════════════════════════════════════════════════════════════
 * ENCODER TESTS
 * ════════════════════════════════════════════════════════════════════════════ */

/* ── uint ───────────────────────────────────────────────────────────────── */

static void test_cbor_write_uint_zero(void)
{
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_uint(&w, 0u);
    TEST_ASSERT_TRUE(syn_cbor_writer_ok(&w));
    TEST_ASSERT_EQUAL_size_t(1u, syn_cbor_writer_len(&w));
    TEST_ASSERT_EQUAL_HEX8(0x00u, buf[0]);
}

static void test_cbor_write_uint_23(void)
{
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_uint(&w, 23u);
    TEST_ASSERT_EQUAL_size_t(1u, syn_cbor_writer_len(&w));
    TEST_ASSERT_EQUAL_HEX8(0x17u, buf[0]);
}

static void test_cbor_write_uint_24(void)
{
    static const uint8_t expected[] = {0x18u, 0x18u};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_uint(&w, 24u);
    TEST_ASSERT_EQUAL_size_t(2u, syn_cbor_writer_len(&w));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 2u);
}

static void test_cbor_write_uint_255(void)
{
    static const uint8_t expected[] = {0x18u, 0xFFu};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_uint(&w, 255u);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 2u);
}

static void test_cbor_write_uint_256(void)
{
    static const uint8_t expected[] = {0x19u, 0x01u, 0x00u};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_uint(&w, 256u);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 3u);
}

static void test_cbor_write_uint_65536(void)
{
    static const uint8_t expected[] = {0x1Au, 0x00u, 0x01u, 0x00u, 0x00u};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_uint(&w, 65536u);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 5u);
}

/* ── int ────────────────────────────────────────────────────────────────── */

static void test_cbor_write_int_neg1(void)
{
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_int(&w, -1);
    TEST_ASSERT_EQUAL_size_t(1u, syn_cbor_writer_len(&w));
    TEST_ASSERT_EQUAL_HEX8(0x20u, buf[0]); /* major=1, arg=0, -1-0=-1 */
}

static void test_cbor_write_int_neg24(void)
{
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_int(&w, -24);
    TEST_ASSERT_EQUAL_size_t(1u, syn_cbor_writer_len(&w));
    TEST_ASSERT_EQUAL_HEX8(0x37u, buf[0]); /* 001 10111 = major1, arg=23 */
}

static void test_cbor_write_int_neg25(void)
{
    static const uint8_t expected[] = {0x38u, 0x18u};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_int(&w, -25); /* -1-24=arg24, needs 1 extra byte */
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 2u);
}

static void test_cbor_write_int_positive(void)
{
    /* Positive int encodes as uint (major type 0) */
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_int(&w, 42);
    TEST_ASSERT_EQUAL_HEX8(0x18u, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x2Au, buf[1]);
}

/* ── bool / null ────────────────────────────────────────────────────────── */

static void test_cbor_write_true(void)
{
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_bool(&w, true);
    TEST_ASSERT_EQUAL_size_t(1u, syn_cbor_writer_len(&w));
    TEST_ASSERT_EQUAL_HEX8(0xF5u, buf[0]);
}

static void test_cbor_write_false(void)
{
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_bool(&w, false);
    TEST_ASSERT_EQUAL_HEX8(0xF4u, buf[0]);
}

static void test_cbor_write_null(void)
{
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_null(&w);
    TEST_ASSERT_EQUAL_HEX8(0xF6u, buf[0]);
}

/* ── float ──────────────────────────────────────────────────────────────── */

static void test_cbor_write_float(void)
{
    /* 23.5f = 0x41BC0000 big-endian */
    static const uint8_t expected[] = {0xFAu, 0x41u, 0xBCu, 0x00u, 0x00u};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_float(&w, 23.5f);
    TEST_ASSERT_EQUAL_size_t(5u, syn_cbor_writer_len(&w));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 5u);
}

/* ── text string ─────────────────────────────────────────────────────────── */

static void test_cbor_write_text(void)
{
    /* text("hi") = 0x62 0x68 0x69 */
    static const uint8_t expected[] = {0x62u, 0x68u, 0x69u};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_text_cstr(&w, "hi");
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 3u);
}

static void test_cbor_write_text_empty(void)
{
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_text_cstr(&w, "");
    TEST_ASSERT_EQUAL_HEX8(0x60u, buf[0]); /* major=3, len=0 */
    TEST_ASSERT_EQUAL_size_t(1u, syn_cbor_writer_len(&w));
}

/* ── byte string ─────────────────────────────────────────────────────────── */

static void test_cbor_write_bytes(void)
{
    static const uint8_t data[] = {0xABu, 0xCDu};
    static const uint8_t expected[] = {0x42u, 0xABu, 0xCDu};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_bytes(&w, data, 2u);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 3u);
}

/* ── array ───────────────────────────────────────────────────────────────── */

static void test_cbor_write_array(void)
{
    /* [1, 2] = 0x82 0x01 0x02 */
    static const uint8_t expected[] = {0x82u, 0x01u, 0x02u};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_array_begin(&w, 2u);
    syn_cbor_write_uint(&w, 1u);
    syn_cbor_write_uint(&w, 2u);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 3u);
}

/* ── map ─────────────────────────────────────────────────────────────────── */

static void test_cbor_write_map_int_keys(void)
{
    /* {1: 2} = 0xA1 0x01 0x02 */
    static const uint8_t expected[] = {0xA1u, 0x01u, 0x02u};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_map_begin(&w, 1u);
    syn_cbor_write_uint(&w, 1u);
    syn_cbor_write_uint(&w, 2u);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 3u);
}

static void test_cbor_write_map_string_keys(void)
{
    /* {"a": 1} = 0xA1 0x61 0x61 0x01 */
    static const uint8_t expected[] = {0xA1u, 0x61u, 0x61u, 0x01u};
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_map_begin(&w, 1u);
    syn_cbor_write_text_cstr(&w, "a");
    syn_cbor_write_uint(&w, 1u);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buf, 4u);
}

/* ── overflow ─────────────────────────────────────────────────────────────── */

static void test_cbor_write_overflow(void)
{
    uint8_t tiny[2];
    syn_cbor_writer_init(&w, tiny, sizeof(tiny));
    syn_cbor_write_uint(&w, 1000u); /* needs 3 bytes, only 2 available */
    TEST_ASSERT_FALSE(syn_cbor_writer_ok(&w));
}

/* ════════════════════════════════════════════════════════════════════════════
 * DECODER TESTS
 * ════════════════════════════════════════════════════════════════════════════ */

static void test_cbor_read_uint_small(void)
{
    static const uint8_t data[] = {0x17u}; /* uint(23) */
    syn_cbor_reader_init(&r, data, sizeof(data));
    TEST_ASSERT_EQUAL(SYN_CBOR_UINT, syn_cbor_peek_type(&r));
    TEST_ASSERT_EQUAL_UINT64(23u, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_uint_extended(void)
{
    static const uint8_t data[] = {0x18u, 0x18u}; /* uint(24) */
    syn_cbor_reader_init(&r, data, sizeof(data));
    TEST_ASSERT_EQUAL_UINT64(24u, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_done(&r));
}

static void test_cbor_read_int_neg1(void)
{
    static const uint8_t data[] = {0x20u}; /* int(-1) */
    syn_cbor_reader_init(&r, data, sizeof(data));
    TEST_ASSERT_EQUAL(SYN_CBOR_INT, syn_cbor_peek_type(&r));
    TEST_ASSERT_EQUAL_INT64(-1, syn_cbor_read_int(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // Positive int read via syn_cbor_read_int
    static const uint8_t pos_data[] = {0x17u}; /* uint(23) */
    syn_cbor_reader_init(&r, pos_data, sizeof(pos_data));
    TEST_ASSERT_EQUAL(SYN_CBOR_UINT, syn_cbor_peek_type(&r));
    TEST_ASSERT_EQUAL_INT64(23, syn_cbor_read_int(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_bool_true(void)
{
    static const uint8_t data[] = {0xF5u};
    syn_cbor_reader_init(&r, data, sizeof(data));
    TEST_ASSERT_EQUAL(SYN_CBOR_BOOL, syn_cbor_peek_type(&r));
    TEST_ASSERT_TRUE(syn_cbor_read_bool(&r));
}

static void test_cbor_read_bool_false(void)
{
    static const uint8_t data[] = {0xF4u};
    syn_cbor_reader_init(&r, data, sizeof(data));
    TEST_ASSERT_FALSE(syn_cbor_read_bool(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_float(void)
{
    static const uint8_t data[] = {0xFAu, 0x41u, 0xBCu, 0x00u, 0x00u};
    syn_cbor_reader_init(&r, data, sizeof(data));
    TEST_ASSERT_EQUAL(SYN_CBOR_FLOAT, syn_cbor_peek_type(&r));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 23.5f, syn_cbor_read_float(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_text(void)
{
    static const uint8_t data[] = {0x62u, 0x68u, 0x69u}; /* "hi" */
    syn_cbor_reader_init(&r, data, sizeof(data));
    TEST_ASSERT_EQUAL(SYN_CBOR_TEXT, syn_cbor_peek_type(&r));
    char out[8];
    size_t n = syn_cbor_read_text(&r, out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(2u, n);
    TEST_ASSERT_EQUAL_STRING("hi", out);
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_map(void)
{
    static const uint8_t data[] = {0xA1u, 0x01u, 0x02u}; /* {1: 2} */
    syn_cbor_reader_init(&r, data, sizeof(data));
    TEST_ASSERT_EQUAL(SYN_CBOR_MAP, syn_cbor_peek_type(&r));
    size_t pairs = syn_cbor_read_map_begin(&r);
    TEST_ASSERT_EQUAL_size_t(1u, pairs);
    TEST_ASSERT_EQUAL_UINT64(1u, syn_cbor_read_uint(&r)); /* key */
    TEST_ASSERT_EQUAL_UINT64(2u, syn_cbor_read_uint(&r)); /* value */
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_type_mismatch_sets_error(void)
{
    static const uint8_t data[] = {0x01u}; /* uint(1) */
    syn_cbor_reader_init(&r, data, sizeof(data));
    (void)syn_cbor_read_float(&r); /* wrong type — expect error */
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_skip_unknown_key(void)
{
    /* Map {1: true, 99: "ignored_value"} — read key 1, skip the rest */
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_map_begin(&w, 2u);
    syn_cbor_write_uint(&w, 1u);
    syn_cbor_write_bool(&w, true);
    syn_cbor_write_uint(&w, 99u);
    syn_cbor_write_text_cstr(&w, "ignored_value");

    syn_cbor_reader_init(&r, buf, syn_cbor_writer_len(&w));
    size_t pairs = syn_cbor_read_map_begin(&r);
    TEST_ASSERT_EQUAL_size_t(2u, pairs);

    uint64_t key1 = syn_cbor_read_uint(&r);
    bool val1 = syn_cbor_read_bool(&r);
    TEST_ASSERT_EQUAL_UINT64(1u, key1);
    TEST_ASSERT_TRUE(val1);

    /* key 2: unknown — skip key then value */
    syn_cbor_skip(&r); /* key 99 */
    syn_cbor_skip(&r); /* "ignored_value" */

    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_done(&r));
}

/* ── roundtrip: write then read back ─────────────────────────────────────── */

static void test_cbor_roundtrip_sensor_map(void)
{
    /* Encode {1: 23.5, 2: 60} */
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_map_begin(&w, 2u);
    syn_cbor_write_uint(&w, 1u);
    syn_cbor_write_float(&w, 23.5f);
    syn_cbor_write_uint(&w, 2u);
    syn_cbor_write_uint(&w, 60u);
    TEST_ASSERT_TRUE(syn_cbor_writer_ok(&w));

    /* Decode */
    float temp = 0.0f;
    uint64_t hum = 0u;
    syn_cbor_reader_init(&r, buf, syn_cbor_writer_len(&w));
    size_t pairs = syn_cbor_read_map_begin(&r);
    size_t i;
    for (i = 0u; i < pairs; i++) {
        uint64_t key = syn_cbor_read_uint(&r);
        if (key == 1u)
            temp = syn_cbor_read_float(&r);
        else if (key == 2u)
            hum = syn_cbor_read_uint(&r);
        else {
            syn_cbor_skip(&r);
        }
    }
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 23.5f, temp);
    TEST_ASSERT_EQUAL_UINT64(60u, hum);
}

static void test_cbor_read_args_16_32_64_bit(void)
{
    // 16-bit uint
    static const uint8_t u16[] = {0x19u, 0x12u, 0x34u};
    syn_cbor_reader_init(&r, u16, sizeof(u16));
    TEST_ASSERT_EQUAL_UINT64(4660u, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // 32-bit uint
    static const uint8_t u32[] = {0x1Au, 0x12u, 0x34u, 0x56u, 0x78u};
    syn_cbor_reader_init(&r, u32, sizeof(u32));
    TEST_ASSERT_EQUAL_UINT64(305419896u, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // 64-bit uint
    static const uint8_t u64[] = {0x1Bu, 0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0xDEu, 0xF0u};
    syn_cbor_reader_init(&r, u64, sizeof(u64));
    TEST_ASSERT_EQUAL_UINT64(0x123456789ABCDEF0ULL, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // 16-bit negative int
    static const uint8_t i16[] = {0x39u, 0x03u, 0xE7u}; // -1 - 999 = -1000
    syn_cbor_reader_init(&r, i16, sizeof(i16));
    TEST_ASSERT_EQUAL_INT64(-1000, syn_cbor_read_int(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // 32-bit negative int
    static const uint8_t i32[] = {0x3Au, 0x00u, 0x0Fu, 0x42u, 0x3Fu}; // -1 - 999999 = -1000000
    syn_cbor_reader_init(&r, i32, sizeof(i32));
    TEST_ASSERT_EQUAL_INT64(-1000000, syn_cbor_read_int(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_reserved_info(void)
{
    static const uint8_t res[] = {0x1Cu}; // info = 28 (reserved)
    syn_cbor_reader_init(&r, res, sizeof(res));
    (void)syn_cbor_read_uint(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_underrun(void)
{
    // Empty read
    syn_cbor_reader_init(&r, buf, 0);
    (void)syn_cbor_read_uint(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    // Incomplete 16-bit uint
    static const uint8_t inc16[] = {0x19u, 0x01u};
    syn_cbor_reader_init(&r, inc16, sizeof(inc16));
    (void)syn_cbor_read_uint(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    // Incomplete 32-bit uint
    static const uint8_t inc32[] = {0x1Au, 0x01u, 0x02u};
    syn_cbor_reader_init(&r, inc32, sizeof(inc32));
    (void)syn_cbor_read_uint(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    // Incomplete 64-bit uint
    static const uint8_t inc64[] = {0x1Bu, 0x01u, 0x02u, 0x03u};
    syn_cbor_reader_init(&r, inc64, sizeof(inc64));
    (void)syn_cbor_read_uint(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_peek_null_and_invalid(void)
{
    // Peek null
    static const uint8_t null_data[] = {0xF6u};
    syn_cbor_reader_init(&r, null_data, sizeof(null_data));
    TEST_ASSERT_EQUAL(SYN_CBOR_NULL, syn_cbor_peek_type(&r));

    // Peek invalid tag
    static const uint8_t invalid_data[] = {0xFCu};
    syn_cbor_reader_init(&r, invalid_data, sizeof(invalid_data));
    TEST_ASSERT_EQUAL(SYN_CBOR_ERROR, syn_cbor_peek_type(&r));

    // Peek EOF
    syn_cbor_reader_init(&r, buf, 0);
    TEST_ASSERT_EQUAL(SYN_CBOR_ERROR, syn_cbor_peek_type(&r));
}

static void test_cbor_read_array(void)
{
    static const uint8_t array_data[] = {0x82u, 0x01u, 0x02u};
    syn_cbor_reader_init(&r, array_data, sizeof(array_data));
    TEST_ASSERT_EQUAL_size_t(2u, syn_cbor_read_array_begin(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // Type mismatch
    syn_cbor_reader_init(&r, array_data, sizeof(array_data));
    (void)syn_cbor_read_map_begin(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    // End of stream
    syn_cbor_reader_init(&r, buf, 0);
    (void)syn_cbor_read_array_begin(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_null_value(void)
{
    static const uint8_t null_data[] = {0xF6u};
    syn_cbor_reader_init(&r, null_data, sizeof(null_data));
    syn_cbor_read_null(&r);
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // Non-null mismatch
    static const uint8_t true_data[] = {0xF5u};
    syn_cbor_reader_init(&r, true_data, sizeof(true_data));
    syn_cbor_read_null(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    // EOF
    syn_cbor_reader_init(&r, buf, 0);
    syn_cbor_read_null(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_bytes_value(void)
{
    static const uint8_t byte_data[] = {0x42u, 0xABu, 0xCDu};
    uint8_t out[4];

    // Success
    syn_cbor_reader_init(&r, byte_data, sizeof(byte_data));
    TEST_ASSERT_EQUAL_size_t(2u, syn_cbor_read_bytes(&r, out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0xABu, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0xCDu, out[1]);
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // Truncation (cap too small)
    syn_cbor_reader_init(&r, byte_data, sizeof(byte_data));
    TEST_ASSERT_EQUAL_size_t(2u, syn_cbor_read_bytes(&r, out, 1u));
    TEST_ASSERT_EQUAL_HEX8(0xABu, out[0]);
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // Mismatch
    static const uint8_t text_data[] = {0x62u, 0x68u, 0x69u}; // "hi"
    syn_cbor_reader_init(&r, text_data, sizeof(text_data));
    (void)syn_cbor_read_bytes(&r, out, sizeof(out));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    // EOF / incomplete data bytes
    static const uint8_t inc_bytes[] = {0x45u, 0x01u, 0x02u}; // bytes(5), only 2 provided
    syn_cbor_reader_init(&r, inc_bytes, sizeof(inc_bytes));
    (void)syn_cbor_read_bytes(&r, out, sizeof(out));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_scalar_mismatch(void)
{
    static const uint8_t null_data[] = {0xF6u};
    syn_cbor_reader_init(&r, null_data, sizeof(null_data));
    (void)syn_cbor_read_int(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    syn_cbor_reader_init(&r, null_data, sizeof(null_data));
    (void)syn_cbor_read_bool(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_skip_edge_cases(void)
{
    // float16 skip
    static const uint8_t f16[] = {0xF9u, 0x3Cu, 0x00u, 0x01u}; // float16 followed by uint(1)
    syn_cbor_reader_init(&r, f16, sizeof(f16));
    syn_cbor_skip(&r);
    TEST_ASSERT_EQUAL_UINT64(1u, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // float32 skip
    static const uint8_t f32[] = {0xFAu, 0x41u, 0xBCu,
                                  0x00u, 0x00u, 0x02u}; // float32 followed by uint(2)
    syn_cbor_reader_init(&r, f32, sizeof(f32));
    syn_cbor_skip(&r);
    TEST_ASSERT_EQUAL_UINT64(2u, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // float64 skip
    static const uint8_t f64[] = {0xFBu, 0, 0, 0, 0,
                                  0,     0, 0, 0, 0x03u}; // float64 followed by uint(3)
    syn_cbor_reader_init(&r, f64, sizeof(f64));
    syn_cbor_skip(&r);
    TEST_ASSERT_EQUAL_UINT64(3u, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    // Nested array overflow (SKIP_MAX_DEPTH is 8)
    // We send 8 nested arrays of 1 item, ending with uint(0)
    static const uint8_t nested_overflow[] = {0x81u, // depth 1
                                              0x81u, // depth 2
                                              0x81u, // depth 3
                                              0x81u, // depth 4
                                              0x81u, // depth 5
                                              0x81u, // depth 6
                                              0x81u, // depth 7
                                              0x81u, // depth 8 (exceeds SKIP_MAX_DEPTH limit)
                                              0x00u};
    syn_cbor_reader_init(&r, nested_overflow, sizeof(nested_overflow));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    // Nested map overflow (SKIP_MAX_DEPTH is 8)
    // Map with 1 key-value pair, nested 8 levels
    static const uint8_t nested_map_overflow[] = {
        0xA1u, 0x01u, // level 1 (key=1, val=nested map)
        0xA1u, 0x01u, // level 2
        0xA1u, 0x01u, // level 3
        0xA1u, 0x01u, // level 4
        0xA1u, 0x01u, // level 5
        0xA1u, 0x01u, // level 6
        0xA1u, 0x01u, // level 7
        0xA1u, 0x01u, // level 8 (overflows depth limit of 8)
        0x00u};
    syn_cbor_reader_init(&r, nested_map_overflow, sizeof(nested_map_overflow));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    // Premature EOF during skip (e.g. array of 5, but empty)
    static const uint8_t inc_skip[] = {0x85u};
    syn_cbor_reader_init(&r, inc_skip, sizeof(inc_skip));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    // Successful skip of array and map (popping level / depth--)
    static const uint8_t skip_coll[] = {0x81u, 0x01u, 0xA1u, 0x01u,
                                        0x02u}; // array of [1], then map of {1: 2}
    syn_cbor_reader_init(&r, skip_coll, sizeof(skip_coll));
    syn_cbor_skip(&r); // skips [1]
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
    syn_cbor_skip(&r); // skips {1: 2}
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_done(&r));
}

/** uint > 0xFFFFFFFF — exercises 8-byte encoding path (lines 74-82 in cbor_write) */
static void test_cbor_write_uint64(void)
{
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    /* 0x0001_0000_0000 = 4294967296 — requires 8-byte uint64 encoding */
    syn_cbor_write_uint(&w, UINT64_C(0x100000000));
    TEST_ASSERT_TRUE(syn_cbor_writer_ok(&w));
    /* Major type 0, additional info 27 (8-byte follows) */
    TEST_ASSERT_EQUAL_HEX8(0x1Bu, buf[0]);
    /* Value: 0x00 0x00 0x00 0x01 0x00 0x00 0x00 0x00 */
    TEST_ASSERT_EQUAL_HEX8(0x00u, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00u, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00u, buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0x01u, buf[4]);
    TEST_ASSERT_EQUAL_HEX8(0x00u, buf[5]);
    TEST_ASSERT_EQUAL_HEX8(0x00u, buf[6]);
    TEST_ASSERT_EQUAL_HEX8(0x00u, buf[7]);
    TEST_ASSERT_EQUAL_HEX8(0x00u, buf[8]);
    TEST_ASSERT_EQUAL_size_t(9u, syn_cbor_writer_len(&w));
}

static void test_cbor_skip_floats_and_nested(void)
{
    SYN_CborReader cr;
    /* Float16 (0xF9 0x00 0x00), Float64 (0xFB 0x00..0x00 8 bytes) */
    uint8_t float16_buf[] = {0xF9, 0x3C, 0x00};
    syn_cbor_reader_init(&cr, float16_buf, sizeof(float16_buf));
    syn_cbor_skip(&cr);
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&cr));

    uint8_t float64_buf[] = {0xFB, 0x40, 0x09, 0x21, 0xFB, 0x54, 0x44, 0x2D, 0x18};
    syn_cbor_reader_init(&cr, float64_buf, sizeof(float64_buf));
    syn_cbor_skip(&cr);
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&cr));
}

/* ── Registration ────────────────────────────────────────────────────────── */

static void test_cbor_read_overflows_and_mismatches(void)
{
    /* Empty reader (pos >= len) */
    SYN_CborReader er;
    syn_cbor_reader_init(&er, buf, 0);

    TEST_ASSERT_EQUAL_size_t(0, syn_cbor_read_map_begin(&er));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&er));

    syn_cbor_reader_init(&er, buf, 0);
    TEST_ASSERT_EQUAL_size_t(0, syn_cbor_read_array_begin(&er));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&er));

    syn_cbor_reader_init(&er, buf, 0);
    TEST_ASSERT_EQUAL_UINT64(0, syn_cbor_read_uint(&er));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&er));

    syn_cbor_reader_init(&er, buf, 0);
    TEST_ASSERT_EQUAL_INT64(0, syn_cbor_read_int(&er));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&er));

    syn_cbor_reader_init(&er, buf, 0);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, syn_cbor_read_float(&er));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&er));
}

static void test_cbor_read_text_and_bytes_overrun_and_mismatch(void)
{
    /* Read text when pos >= len */
    SYN_CborReader cr;
    syn_cbor_reader_init(&cr, buf, 0);
    char text_out[16];
    TEST_ASSERT_EQUAL_size_t(0, syn_cbor_read_text(&cr, text_out, sizeof(text_out)));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&cr));

    /* Read text with major type mismatch (e.g. integer 0x01) */
    uint8_t not_text[] = {0x01};
    syn_cbor_reader_init(&cr, not_text, sizeof(not_text));
    TEST_ASSERT_EQUAL_size_t(0, syn_cbor_read_text(&cr, text_out, sizeof(text_out)));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&cr));

    /* Read bytes when pos >= len */
    syn_cbor_reader_init(&cr, buf, 0);
    uint8_t bytes_out[16];
    TEST_ASSERT_EQUAL_size_t(0, syn_cbor_read_bytes(&cr, bytes_out, sizeof(bytes_out)));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&cr));
}

static void test_cbor_write_text_cstr(void)
{
    uint8_t buf[32];
    SYN_CborWriter w;
    syn_cbor_writer_init(&w, buf, sizeof(buf));
    syn_cbor_write_text_cstr(&w, "hello");
    TEST_ASSERT_EQUAL(6, syn_cbor_writer_len(&w));
}

static void test_cbor_skip_truncated_strings_and_floats(void)
{
    /* Truncated float (major type 7, info 26 -> 4 bytes payload required, only 2 provided) */
    uint8_t trunc_float[] = {0xFA, 0x41, 0xBC};
    syn_cbor_reader_init(&r, trunc_float, sizeof(trunc_float));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    /* Truncated string (major type 3, text string length 10, only 3 payload bytes provided) */
    uint8_t trunc_str[] = {0x6A, 0x61, 0x62, 0x63};
    syn_cbor_reader_init(&r, trunc_str, sizeof(trunc_str));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_type_mismatch_type_error_branches(void)
{
    /* Try reading array_len on a CBOR uint byte (0x17) */
    uint8_t uint_byte[] = {0x17};
    syn_cbor_reader_init(&r, uint_byte, sizeof(uint_byte));
    TEST_ASSERT_EQUAL(0, syn_cbor_read_array_begin(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    /* Try reading uint on a CBOR array byte (0x82) */
    uint8_t array_byte[] = {0x82};
    syn_cbor_reader_init(&r, array_byte, sizeof(array_byte));
    TEST_ASSERT_EQUAL(0, syn_cbor_read_uint(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_read_bool_and_float_underrun(void)
{
    uint8_t empty[1] = {0};
    syn_cbor_reader_init(&r, empty, 0);
    TEST_ASSERT_FALSE(syn_cbor_read_bool(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    uint8_t trunc_float[] = {0xFA, 0x00, 0x00}; /* float32 needs 4 header bytes */
    syn_cbor_reader_init(&r, trunc_float, sizeof(trunc_float));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, syn_cbor_read_float(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    uint8_t trunc_text[] = {0x6A, 0x61}; /* declares 10 byte text, only 1 available */
    char str[16];
    syn_cbor_reader_init(&r, trunc_text, sizeof(trunc_text));
    TEST_ASSERT_EQUAL(10, syn_cbor_read_text(&r, str, sizeof(str)));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

static void test_cbor_peek_at_eof_and_write_already_overflowed(void)
{
    /* 1. peek_byte at EOF (line 38 of syn_cbor_read.c) */
    SYN_CborReader reader;
    syn_cbor_reader_init(&reader, buf, 0);
    TEST_ASSERT_EQUAL(SYN_CBOR_ERROR, syn_cbor_peek_type(&reader));

    /* 2. emit_byte on already overflowed writer (line 24 of syn_cbor_write.c) */
    SYN_CborWriter writer;
    syn_cbor_writer_init(&writer, buf, 1);
    syn_cbor_write_uint(
        &writer, 1000); /* 1000 needs 3 bytes: 0x19 0x03 0xE8. 2nd byte hits overflow line 24 */
    TEST_ASSERT_TRUE(writer.overflow);
}

static void test_r5_n01_cbor_collection_bounds_and_skip(void)
{
    /* R5-N01: Oversized map count (0xBA FF FF FF FF -> 4.29B pairs in 5-byte buffer) */
    static const uint8_t huge_map[] = {0xBAu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
    syn_cbor_reader_init(&r, huge_map, sizeof(huge_map));
    TEST_ASSERT_EQUAL_size_t(0u, syn_cbor_read_map_begin(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    /* Map declaring 1 pair (needs >= 2 bytes) with only 1 byte remaining */
    static const uint8_t short_map[] = {0xA1u, 0x01u};
    syn_cbor_reader_init(&r, short_map, sizeof(short_map));
    TEST_ASSERT_EQUAL_size_t(0u, syn_cbor_read_map_begin(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    /* Oversized array count (0x9A FF FF FF FF) and 2-item array with 1 byte remaining */
    static const uint8_t huge_arr[] = {0x9Au, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
    syn_cbor_reader_init(&r, huge_arr, sizeof(huge_arr));
    TEST_ASSERT_EQUAL_size_t(0u, syn_cbor_read_array_begin(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    static const uint8_t short_arr[] = {0x82u, 0x01u};
    syn_cbor_reader_init(&r, short_arr, sizeof(short_arr));
    TEST_ASSERT_EQUAL_size_t(0u, syn_cbor_read_array_begin(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    /* syn_cbor_skip on empty buffer, mid-array EOF, reserved scalar, and huge map */
    syn_cbor_reader_init(&r, buf, 0u);
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    static const uint8_t mid_eof_arr[] = {0x82u, 0x42u, 0x61u, 0x62u};
    syn_cbor_reader_init(&r, mid_eof_arr, sizeof(mid_eof_arr));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    static const uint8_t reserved_scalar[] = {0x1Cu};
    syn_cbor_reader_init(&r, reserved_scalar, sizeof(reserved_scalar));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    syn_cbor_reader_init(&r, huge_map, sizeof(huge_map));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    /* 64-bit text/bytes length (0x0000000100000002) must NOT truncate to 2 on 32-bit targets */
    static const uint8_t u64_text[] = {0x7Bu, 0x00u, 0x00u, 0x00u, 0x01u, 0x00u,
                                       0x00u, 0x00u, 0x02u, 'a',   'b',   0x01u};
    char tbuf[8] = {0};
    syn_cbor_reader_init(&r, u64_text, sizeof(u64_text));
    (void)syn_cbor_read_text(&r, tbuf, sizeof(tbuf));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
    TEST_ASSERT_EQUAL_size_t(sizeof(u64_text), r.pos);

    static const uint8_t u64_bytes[] = {0x5Bu, 0x00u, 0x00u, 0x00u, 0x01u, 0x00u,
                                        0x00u, 0x00u, 0x02u, 0xAAu, 0xBBu, 0x01u};
    uint8_t bbuf[8] = {0};
    syn_cbor_reader_init(&r, u64_bytes, sizeof(u64_bytes));
    (void)syn_cbor_read_bytes(&r, bbuf, sizeof(bbuf));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
    TEST_ASSERT_EQUAL_size_t(sizeof(u64_bytes), r.pos);
}

static void test_tier_a_cbor_skip_simple_tag_and_int64_bounds(void)
{
    /* UTL-02a: skip_major7 info==24 (0xF8 1-byte simple value) advances 1 byte cleanly */
    static const uint8_t simple24_ok[] = {0xF8u, 0x20u, 0x05u};
    syn_cbor_reader_init(&r, simple24_ok, sizeof(simple24_ok));
    syn_cbor_skip(&r);
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
    TEST_ASSERT_EQUAL_UINT64(5u, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_done(&r));

    /* Truncated 0xF8 simple value fails cleanly */
    static const uint8_t simple24_trunc[] = {0xF8u};
    syn_cbor_reader_init(&r, simple24_trunc, sizeof(simple24_trunc));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    /* Reserved major 7 info 28..31 (0xFC..0xFF) rejected by skip_major7 */
    for (uint8_t hdr = 0xFCu; hdr != 0x00u; hdr++) {
        uint8_t bad_m7[2] = {hdr, 0x01u};
        syn_cbor_reader_init(&r, bad_m7, sizeof(bad_m7));
        syn_cbor_skip(&r);
        TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
    }

    /* UTL-02b: Major 6 semantic tag (e.g. Tag 1 epoch + Tag 32 URI) skips enclosed item */
    static const uint8_t tagged_epoch[] = {0xC1u, 0x1Au, 0x51u, 0x4Bu, 0x67u, 0xB0u, 0x09u};
    syn_cbor_reader_init(&r, tagged_epoch, sizeof(tagged_epoch));
    syn_cbor_skip(&r);
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
    TEST_ASSERT_EQUAL_UINT64(9u, syn_cbor_read_uint(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_done(&r));

    static const uint8_t tagged_uri[] = {0xD8u, 0x20u, 0x63u, 'f', 'o', 'o', 0x07u};
    syn_cbor_reader_init(&r, tagged_uri, sizeof(tagged_uri));
    syn_cbor_skip(&r);
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));
    TEST_ASSERT_EQUAL_UINT64(7u, syn_cbor_read_uint(&r));

    /* Truncated Major 6 tag with no enclosed item or reserved info fails */
    static const uint8_t trunc_tag[] = {0xC1u};
    syn_cbor_reader_init(&r, trunc_tag, sizeof(trunc_tag));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    static const uint8_t reserved_tag[] = {0xDCu, 0x01u};
    syn_cbor_reader_init(&r, reserved_tag, sizeof(reserved_tag));
    syn_cbor_skip(&r);
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    /* UTL-02c: syn_cbor_read_int boundary and overflow rejection */
    const uint8_t u_max[9] = {0x1Bu, 0x7Fu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
    syn_cbor_reader_init(&r, u_max, sizeof(u_max));
    TEST_ASSERT_EQUAL_INT64(INT64_MAX, syn_cbor_read_int(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    const uint8_t u_ovr[9] = {0x1Bu, 0x80u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u};
    syn_cbor_reader_init(&r, u_ovr, sizeof(u_ovr));
    TEST_ASSERT_EQUAL_INT64(0, syn_cbor_read_int(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    const uint8_t n_min[9] = {0x3Bu, 0x7Fu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
    syn_cbor_reader_init(&r, n_min, sizeof(n_min));
    TEST_ASSERT_EQUAL_INT64(INT64_MIN, syn_cbor_read_int(&r));
    TEST_ASSERT_TRUE(syn_cbor_reader_ok(&r));

    const uint8_t n_ovr[9] = {0x3Bu, 0x80u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u};
    syn_cbor_reader_init(&r, n_ovr, sizeof(n_ovr));
    TEST_ASSERT_EQUAL_INT64(0, syn_cbor_read_int(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));

    static const uint8_t n_trunc[] = {0x38u};
    syn_cbor_reader_init(&r, n_trunc, sizeof(n_trunc));
    TEST_ASSERT_EQUAL_INT64(0, syn_cbor_read_int(&r));
    TEST_ASSERT_FALSE(syn_cbor_reader_ok(&r));
}

void run_cbor_tests(void)
{
    /* Encoder */
    RUN_TEST(test_cbor_write_uint_zero);
    RUN_TEST(test_cbor_write_uint_23);
    RUN_TEST(test_cbor_write_uint_24);
    RUN_TEST(test_cbor_write_uint_255);
    RUN_TEST(test_cbor_write_uint_256);
    RUN_TEST(test_cbor_write_uint_65536);
    RUN_TEST(test_cbor_write_uint64); /* NEW: 8-byte encoding */
    RUN_TEST(test_cbor_write_int_neg1);
    RUN_TEST(test_cbor_write_int_neg24);
    RUN_TEST(test_cbor_write_int_neg25);
    RUN_TEST(test_cbor_write_int_positive);
    RUN_TEST(test_cbor_write_true);
    RUN_TEST(test_cbor_write_false);
    RUN_TEST(test_cbor_write_null);
    RUN_TEST(test_cbor_write_float);
    RUN_TEST(test_cbor_write_text);
    RUN_TEST(test_cbor_write_text_empty);
    RUN_TEST(test_cbor_write_bytes);
    RUN_TEST(test_cbor_write_array);
    RUN_TEST(test_cbor_write_map_int_keys);
    RUN_TEST(test_cbor_write_map_string_keys);
    RUN_TEST(test_cbor_write_overflow);
    /* Decoder */
    RUN_TEST(test_cbor_read_uint_small);
    RUN_TEST(test_cbor_read_uint_extended);
    RUN_TEST(test_cbor_read_int_neg1);
    RUN_TEST(test_cbor_read_bool_true);
    RUN_TEST(test_cbor_read_bool_false);
    RUN_TEST(test_cbor_read_float);
    RUN_TEST(test_cbor_read_text);
    RUN_TEST(test_cbor_read_map);
    RUN_TEST(test_cbor_read_type_mismatch_sets_error);
    RUN_TEST(test_cbor_skip_unknown_key);
    RUN_TEST(test_cbor_roundtrip_sensor_map);
    RUN_TEST(test_cbor_read_args_16_32_64_bit);
    RUN_TEST(test_cbor_read_reserved_info);
    RUN_TEST(test_cbor_read_underrun);
    RUN_TEST(test_cbor_peek_null_and_invalid);
    RUN_TEST(test_cbor_read_array);
    RUN_TEST(test_cbor_read_null_value);
    RUN_TEST(test_cbor_read_bytes_value);
    RUN_TEST(test_cbor_read_scalar_mismatch);
    RUN_TEST(test_cbor_skip_edge_cases);
    RUN_TEST(test_cbor_skip_floats_and_nested);
    RUN_TEST(test_cbor_read_overflows_and_mismatches);
    RUN_TEST(test_cbor_read_text_and_bytes_overrun_and_mismatch);
    RUN_TEST(test_cbor_skip_truncated_strings_and_floats);
    RUN_TEST(test_cbor_read_type_mismatch_type_error_branches);
    RUN_TEST(test_cbor_read_bool_and_float_underrun);
    RUN_TEST(test_cbor_write_text_cstr);
    RUN_TEST(test_cbor_peek_at_eof_and_write_already_overflowed);
    RUN_TEST(test_r5_n01_cbor_collection_bounds_and_skip);
    RUN_TEST(test_tier_a_cbor_skip_simple_tag_and_int64_bounds);
}
