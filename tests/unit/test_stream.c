/**
 * @file test_stream.c
 * @brief Unit tests for SYN_Stream — cooperative byte stream.
 */

#include "syntropic/util/syn_stream.h"
#include "unity/unity.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static uint8_t s_buf[32];
static SYN_Stream s_stream;

static void stream_setup(void)
{
    syn_stream_init(&s_stream, s_buf, sizeof(s_buf));
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

void test_stream_init(void)
{
    stream_setup();
    TEST_ASSERT_EQUAL(0, syn_stream_count(&s_stream));
    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));
    TEST_ASSERT_EQUAL(sizeof(s_buf) - 1, syn_stream_free(&s_stream));
}

void test_stream_write_read(void)
{
    stream_setup();
    const uint8_t data[] = {0xAA, 0xBB, 0xCC};
    uint8_t out[8];

    size_t w = syn_stream_write(&s_stream, data, sizeof(data));
    TEST_ASSERT_EQUAL(3, w);
    TEST_ASSERT_EQUAL(3, syn_stream_count(&s_stream));

    size_t r = syn_stream_read(&s_stream, out, sizeof(out));
    TEST_ASSERT_EQUAL(3, r);
    TEST_ASSERT_EQUAL_HEX8(0xAA, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, out[2]);
    TEST_ASSERT_EQUAL(0, syn_stream_count(&s_stream));
}

void test_stream_partial_write(void)
{
    stream_setup();
    /* Buffer capacity is 31 (size - 1 sentinel). Write 40 bytes. */
    uint8_t big[40];
    memset(big, 0x55, sizeof(big));

    size_t w = syn_stream_write(&s_stream, big, sizeof(big));
    TEST_ASSERT_EQUAL(sizeof(s_buf) - 1, w); /* Only capacity fits */
    TEST_ASSERT_EQUAL(0, syn_stream_free(&s_stream));
}

void test_stream_put_get(void)
{
    stream_setup();
    TEST_ASSERT_TRUE(syn_stream_put(&s_stream, 0x42));
    TEST_ASSERT_EQUAL(1, syn_stream_count(&s_stream));

    uint8_t out;
    size_t r = syn_stream_read(&s_stream, &out, 1);
    TEST_ASSERT_EQUAL(1, r);
    TEST_ASSERT_EQUAL_HEX8(0x42, out);
}

void test_stream_readable_default(void)
{
    stream_setup();
    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));

    syn_stream_put(&s_stream, 0x01);
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));

    uint8_t tmp;
    syn_stream_read(&s_stream, &tmp, 1);
    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));
}

void test_stream_threshold(void)
{
    stream_setup();
    syn_stream_set_threshold(&s_stream, 4);

    /* Write 1, 2, 3 bytes — not yet readable */
    syn_stream_put(&s_stream, 'a');
    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));

    syn_stream_put(&s_stream, 'b');
    syn_stream_put(&s_stream, 'c');
    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));
    TEST_ASSERT_EQUAL(3, syn_stream_count(&s_stream));

    /* Write 4th byte — now readable */
    syn_stream_put(&s_stream, 'd');
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));
    TEST_ASSERT_EQUAL(4, syn_stream_count(&s_stream));
}

void test_stream_threshold_reset(void)
{
    stream_setup();
    syn_stream_set_threshold(&s_stream, 10);

    syn_stream_put(&s_stream, 'x');
    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));

    /* Reset threshold to 0 (any bytes = readable) */
    syn_stream_set_threshold(&s_stream, 0);
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));
}

void test_stream_delimiter(void)
{
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');

    /* Write partial line — not readable */
    const uint8_t partial[] = "hel";
    syn_stream_write(&s_stream, partial, 3);
    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));

    /* Write rest including newline — now readable */
    const uint8_t rest[] = "lo\n";
    syn_stream_write(&s_stream, rest, 3);
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));
}

void test_stream_delimiter_wrap(void)
{
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');

    /* Fill most of the buffer, then drain — forces wrap */
    uint8_t filler[25];
    memset(filler, 'X', sizeof(filler));
    syn_stream_write(&s_stream, filler, sizeof(filler));

    uint8_t drain[25];
    syn_stream_read(&s_stream, drain, sizeof(drain));
    TEST_ASSERT_EQUAL(0, syn_stream_count(&s_stream));

    /* Now write across the wrap boundary: "AB\nCD" */
    const uint8_t wrapped[] = "AB\nCD";
    syn_stream_write(&s_stream, (const uint8_t *)wrapped, 5);
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));
}

void test_stream_read_line(void)
{
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');

    const uint8_t input[] = "hello\nworld\n";
    syn_stream_write(&s_stream, input, sizeof(input) - 1); /* no NUL */

    uint8_t line[32];

    /* First line: "hello\n" */
    size_t n = syn_stream_read_line(&s_stream, line, sizeof(line));
    TEST_ASSERT_EQUAL(6, n);
    TEST_ASSERT_EQUAL_MEMORY("hello\n", line, 6);

    /* Second line: "world\n" */
    n = syn_stream_read_line(&s_stream, line, sizeof(line));
    TEST_ASSERT_EQUAL(6, n);
    TEST_ASSERT_EQUAL_MEMORY("world\n", line, 6);

    /* No more data */
    TEST_ASSERT_EQUAL(0, syn_stream_count(&s_stream));
}

void test_stream_read_line_no_delim(void)
{
    stream_setup();
    /* No delimiter */
    const uint8_t data[] = "plain read";
    syn_stream_write(&s_stream, data, sizeof(data) - 1);

    uint8_t line[32];
    size_t n = syn_stream_read_line(&s_stream, line, sizeof(line));
    TEST_ASSERT_EQUAL(10, n);
    TEST_ASSERT_EQUAL_MEMORY("plain read", line, 10);
    TEST_ASSERT_EQUAL(0, syn_stream_count(&s_stream));
}

void test_stream_read_line_no_match(void)
{
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');

    /* Write data without newline */
    const uint8_t data[] = "no newline";
    syn_stream_write(&s_stream, data, sizeof(data) - 1);

    uint8_t line[32];
    size_t n = syn_stream_read_line(&s_stream, line, sizeof(line));
    TEST_ASSERT_EQUAL(0, n);
    TEST_ASSERT_EQUAL(10, syn_stream_count(&s_stream));
}

void test_stream_read_line_max_len(void)
{
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');

    const uint8_t input[] = "long line here\n";
    syn_stream_write(&s_stream, input, sizeof(input) - 1);

    /* Read with small buffer — should cap at max_len */
    uint8_t line[6];
    size_t n = syn_stream_read_line(&s_stream, line, sizeof(line));
    TEST_ASSERT_EQUAL(6, n); /* Capped */
    TEST_ASSERT_EQUAL_MEMORY("long l", line, 6);

    /* Remaining bytes still in buffer */
    TEST_ASSERT_EQUAL(9, syn_stream_count(&s_stream)); /* "ine here\n" */
}

void test_stream_delimiter_overrides_threshold(void)
{
    stream_setup();
    syn_stream_set_threshold(&s_stream, 100);  /* Very high threshold */
    syn_stream_set_delimiter(&s_stream, '\n'); /* Delimiter takes precedence */

    const uint8_t input[] = "hi\n";
    syn_stream_write(&s_stream, input, 3);
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream)); /* Delimiter wins */
}

void test_stream_flush(void)
{
    stream_setup();
    const uint8_t data[] = "some data";
    syn_stream_write(&s_stream, data, sizeof(data) - 1);
    TEST_ASSERT_EQUAL(9, syn_stream_count(&s_stream));

    syn_stream_flush(&s_stream);
    TEST_ASSERT_EQUAL(0, syn_stream_count(&s_stream));
    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));
}

void test_stream_free(void)
{
    stream_setup();
    size_t capacity = syn_stream_free(&s_stream);
    TEST_ASSERT_EQUAL(sizeof(s_buf) - 1, capacity);

    syn_stream_put(&s_stream, 0x01);
    TEST_ASSERT_EQUAL(capacity - 1, syn_stream_free(&s_stream));

    syn_stream_put(&s_stream, 0x02);
    TEST_ASSERT_EQUAL(capacity - 2, syn_stream_free(&s_stream));
}

void test_stream_clear_delimiter(void)
{
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');

    const uint8_t data[] = "no newline";
    syn_stream_write(&s_stream, data, sizeof(data) - 1);
    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));

    /* Clear delimiter — reverts to default (any bytes = readable) */
    syn_stream_clear_delimiter(&s_stream);
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));
}

void test_stream_delimiter_overflow_resync(void)
{
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');

    /* Buffer capacity is sizeof(s_buf) - 1 = 31 bytes */
    /* Fill buffer with 31 non-delimiter bytes */
    uint8_t garbage[31];
    memset(garbage, 'A', sizeof(garbage));
    syn_stream_write(&s_stream, garbage, 31);

    TEST_ASSERT_FALSE(syn_stream_readable(&s_stream));
    TEST_ASSERT_EQUAL(31, syn_stream_count(&s_stream));

    /* Push 32nd byte (without delimiter) -> triggers overflow flush & resync state */
    TEST_ASSERT_FALSE(syn_stream_put(&s_stream, 'B'));
    TEST_ASSERT_EQUAL(0, syn_stream_count(&s_stream));
    TEST_ASSERT_EQUAL(1, s_stream.overflow_count);
    TEST_ASSERT_TRUE(s_stream.overflowing);

    /* Push more bytes while in resync state -> dropped */
    TEST_ASSERT_FALSE(syn_stream_put(&s_stream, 'C'));
    TEST_ASSERT_EQUAL(0, syn_stream_count(&s_stream));

    /* Trailing delimiter for broken frame arrives -> completes resync */
    TEST_ASSERT_FALSE(syn_stream_put(&s_stream, '\n'));
    TEST_ASSERT_FALSE(s_stream.overflowing);
    TEST_ASSERT_EQUAL(0, syn_stream_count(&s_stream));

    /* Next fresh frame accumulates cleanly */
    const uint8_t valid_frame[] = "OK\n";
    syn_stream_write(&s_stream, valid_frame, 3);
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));

    uint8_t line[32];
    size_t n = syn_stream_read_line(&s_stream, line, sizeof(line));
    TEST_ASSERT_EQUAL(3, n);
    TEST_ASSERT_EQUAL_MEMORY("OK\n", line, 3);
}

/* ── Test runner ────────────────────────────────────────────────────────── */

static void test_stream_null_and_wrapped_delim(void)
{
    uint8_t tmp[8];
    SYN_Stream bad_stream;

    /* NULL / invalid parameter guards across all public APIs */
    syn_stream_init(NULL, s_buf, sizeof(s_buf));
    syn_stream_init(&bad_stream, NULL, sizeof(s_buf));
    TEST_ASSERT_EQUAL(0U, syn_stream_count(&bad_stream));
    syn_stream_init(&bad_stream, s_buf, 1U);
    TEST_ASSERT_EQUAL(0U, syn_stream_count(&bad_stream));
    syn_stream_set_delimiter(&bad_stream, '\n');
    TEST_ASSERT_FALSE(syn_stream_put(&bad_stream, 'A'));

    syn_stream_set_threshold(NULL, 4U);
    syn_stream_set_delimiter(NULL, '\n');
    syn_stream_clear_delimiter(NULL);
    TEST_ASSERT_EQUAL(0U, syn_stream_write(NULL, tmp, 4U));
    TEST_ASSERT_EQUAL(0U, syn_stream_write(&s_stream, NULL, 4U));
    TEST_ASSERT_EQUAL(0U, syn_stream_write(&s_stream, tmp, 0U));
    TEST_ASSERT_FALSE(syn_stream_put(NULL, 'A'));
    TEST_ASSERT_FALSE(syn_stream_readable(NULL));
    TEST_ASSERT_EQUAL(0U, syn_stream_read(NULL, tmp, sizeof(tmp)));
    TEST_ASSERT_EQUAL(0U, syn_stream_read(&s_stream, NULL, sizeof(tmp)));
    TEST_ASSERT_EQUAL(0U, syn_stream_read(&s_stream, tmp, 0U));
    TEST_ASSERT_EQUAL(0U, syn_stream_read_line(NULL, tmp, sizeof(tmp)));
    TEST_ASSERT_EQUAL(0U, syn_stream_read_line(&s_stream, NULL, sizeof(tmp)));
    TEST_ASSERT_EQUAL(0U, syn_stream_read_line(&s_stream, tmp, 0U));
    TEST_ASSERT_EQUAL(0U, syn_stream_count(NULL));
    TEST_ASSERT_EQUAL(0U, syn_stream_free(NULL));
    syn_stream_flush(NULL);

    /* Wrapped ringbuf delimiter scan */
    stream_setup();
    s_stream.rb.tail = 30U;
    s_stream.rb.head = 30U;
    syn_stream_set_delimiter(&s_stream, '\n');
    syn_stream_write(&s_stream, (const uint8_t *)"abc\n", 4U);
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));
}

/** R4-K06: Preserve unread complete lines on frame overflow without mutating tail; flush resets. */
static void test_r4_stream_overflow_preserves_lines_and_flush_resets(void)
{
    uint8_t line[32];
    uint8_t garbage[32];
    memset(garbage, 'Z', sizeof(garbage));

    /* 1. Post-overflow syn_stream_flush clears s->overflowing and frame_head */
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');
    TEST_ASSERT_EQUAL(31U, syn_stream_write(&s_stream, garbage, 32U));
    TEST_ASSERT_TRUE(s_stream.overflowing);
    syn_stream_flush(&s_stream);
    TEST_ASSERT_FALSE(s_stream.overflowing);
    TEST_ASSERT_EQUAL(0U, s_stream.frame_head);
    TEST_ASSERT_EQUAL(3U, syn_stream_write(&s_stream, (const uint8_t *)"OK\n", 3U));
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));
    TEST_ASSERT_EQUAL(3U, syn_stream_read_line(&s_stream, line, sizeof(line)));
    TEST_ASSERT_EQUAL_MEMORY("OK\n", line, 3U);

    /* 2. Two unread complete lines ("L1\n" + "L2\n" = 6B) survive subsequent frame overflow */
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');
    /* Advance tail/head to 26 so "L2\n" ends at slot 31 -> frame_head wraps to 0 (prev = 31) */
    s_stream.rb.tail = 26U;
    s_stream.rb.head = 26U;
    s_stream.frame_head = 26U;
    TEST_ASSERT_EQUAL(6U, syn_stream_write(&s_stream, (const uint8_t *)"L1\nL2\n", 6U));
    TEST_ASSERT_EQUAL(0U, s_stream.frame_head);
    size_t saved_tail = s_stream.rb.tail;

    /* Remaining capacity is 31 - 6 = 25B; writing 26 'Z's overflows partial 3rd frame */
    TEST_ASSERT_EQUAL(25U, syn_stream_write(&s_stream, garbage, 26U));
    TEST_ASSERT_TRUE(s_stream.overflowing);
    TEST_ASSERT_EQUAL(1U, s_stream.overflow_count);
    /* Consumer-owned tail untouched; head rewound to frame_head (0), keeping 6B of L1+L2 */
    TEST_ASSERT_EQUAL(saved_tail, s_stream.rb.tail);
    TEST_ASSERT_EQUAL(6U, syn_stream_count(&s_stream));
    TEST_ASSERT_TRUE(syn_stream_readable(&s_stream));

    /* Trailing bytes + '\n' resync without disturbing unread complete lines */
    TEST_ASSERT_FALSE(syn_stream_put(&s_stream, 'Q'));
    TEST_ASSERT_FALSE(syn_stream_put(&s_stream, '\n'));
    TEST_ASSERT_FALSE(s_stream.overflowing);
    TEST_ASSERT_EQUAL(3U, syn_stream_write(&s_stream, (const uint8_t *)"L3\n", 3U));

    TEST_ASSERT_EQUAL(3U, syn_stream_read_line(&s_stream, line, sizeof(line)));
    TEST_ASSERT_EQUAL_MEMORY("L1\n", line, 3U);
    TEST_ASSERT_EQUAL(3U, syn_stream_read_line(&s_stream, line, sizeof(line)));
    TEST_ASSERT_EQUAL_MEMORY("L2\n", line, 3U);
    TEST_ASSERT_EQUAL(3U, syn_stream_read_line(&s_stream, line, sizeof(line)));
    TEST_ASSERT_EQUAL_MEMORY("L3\n", line, 3U);
    TEST_ASSERT_EQUAL(0U, syn_stream_count(&s_stream));

    /* 3. Overflow where overflowing byte itself is '\n' & raw read past frame_head */
    stream_setup();
    syn_stream_set_delimiter(&s_stream, '\n');
    TEST_ASSERT_EQUAL(4U, syn_stream_write(&s_stream, (const uint8_t *)"A\nBC", 4U));
    /* Consumer drains all 4 bytes via raw syn_stream_read (tail = 4 > frame_head = 2) */
    TEST_ASSERT_EQUAL(4U, syn_stream_read(&s_stream, line, 4U));
    TEST_ASSERT_EQUAL(31U, syn_stream_write(&s_stream, garbage, 31U));
    /* 32nd byte is '\n': rewinds to tail (4), clears overflowing immediately */
    TEST_ASSERT_FALSE(syn_stream_put(&s_stream, '\n'));
    TEST_ASSERT_FALSE(s_stream.overflowing);
    TEST_ASSERT_EQUAL(4U, s_stream.frame_head);
    TEST_ASSERT_EQUAL(4U, s_stream.rb.tail);
    TEST_ASSERT_EQUAL(0U, syn_stream_count(&s_stream));
}

void run_stream_tests(void)
{
    RUN_TEST(test_stream_init);
    RUN_TEST(test_stream_write_read);
    RUN_TEST(test_stream_partial_write);
    RUN_TEST(test_stream_put_get);
    RUN_TEST(test_stream_readable_default);
    RUN_TEST(test_stream_threshold);
    RUN_TEST(test_stream_threshold_reset);
    RUN_TEST(test_stream_delimiter);
    RUN_TEST(test_stream_delimiter_wrap);
    RUN_TEST(test_stream_read_line);
    RUN_TEST(test_stream_read_line_no_delim);
    RUN_TEST(test_stream_read_line_no_match);
    RUN_TEST(test_stream_read_line_max_len);
    RUN_TEST(test_stream_delimiter_overrides_threshold);
    RUN_TEST(test_stream_flush);
    RUN_TEST(test_stream_free);
    RUN_TEST(test_stream_clear_delimiter);
    RUN_TEST(test_stream_delimiter_overflow_resync);
    RUN_TEST(test_stream_null_and_wrapped_delim);
    RUN_TEST(test_r4_stream_overflow_preserves_lines_and_flush_resets);
}
