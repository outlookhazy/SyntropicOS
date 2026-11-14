/**
 * @file test_conversions.c
 * @brief WASM test source for type casting, sign extension, and float truncations.
 */

static int t = 0;

static int fail(void)
{
    return t;
}

__attribute__((noinline)) static int ext8_i32(int x)
{
    return (int)(signed char)x;
}

__attribute__((noinline)) static int ext16_i32(int x)
{
    return (int)(short)x;
}

__attribute__((noinline)) static long long ext8_i64(long long x)
{
    return (long long)(signed char)x;
}

__attribute__((noinline)) static long long ext16_i64(long long x)
{
    return (long long)(short)x;
}

__attribute__((noinline)) static long long ext32_i64(long long x)
{
    return (long long)(int)x;
}

int run_tests(void)
{
    /* 1. Integer sign extension (i32/i64 extend8_s, extend16_s, extend32_s) */
    t++;
    if (ext8_i32(0x12AB) != -85 || ext8_i32(0x127F) != 127)
        return fail();

    t++;
    if (ext16_i32(0x1234CFDC) != -12324 || ext16_i32(0xFFFF1234) != 0x1234)
        return fail();

    t++;
    if (ext8_i64(0x123456789ABCDEF0LL) != -16LL || ext16_i64(0x12345678FFFF8001LL) != -32767LL)
        return fail();

    t++;
    if (ext32_i64(0x0000000180000005LL) != -2147483643LL ||
        ext32_i64(0xFFFFFFFF7FFFFFFFLL) != 2147483647LL)
        return fail();

    /* 2. Float to int truncation (trunc_sat) */
    t++;
    float f = 42.9f;
    int f_int = (int)f;
    if (f_int != 42)
        return fail();

    t++;
    double d = -99.8;
    int d_int = (int)d;
    if (d_int != -99)
        return fail();

    /* 3. Int to float conversion */
    t++;
    int val = 123456;
    float val_f = (float)val;
    if (val_f != 123456.0f)
        return fail();

    return 0;
}
