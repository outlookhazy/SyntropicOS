/**
 * @file test_host_api.c
 * @brief WASM test source for host imports and global variable state.
 */

extern int host_log(int code, int val);
extern unsigned int host_millis(void);
extern void host_sleep_ms(unsigned int ms);
extern int host_uart_read(char *buf, unsigned int len);
extern void host_yield(void);

static int g_counter = 100;

int get_counter(void)
{
    return g_counter;
}

void inc_counter(int delta)
{
    g_counter += delta;
}

int run_tests(void)
{
    g_counter = 100;
    inc_counter(50);
    if (get_counter() != 150)
        return 1;

    int res = host_log(42, g_counter);
    if (res != 192)
        return 2; /* Expected host return: 42 + 150 = 192 */

    return 0;
}

int run_coro_pipeline(void)
{
    char rx_buf[16];
    unsigned int t0 = host_millis();
    host_sleep_ms(25U);
    unsigned int t1 = host_millis();
    if ((t1 - t0) < 25U) {
        return -10;
    }

    int n = host_uart_read(rx_buf, 8U);
    if (n != 8) {
        return -20;
    }

    host_yield();

    unsigned int acc = 0U;
    for (int i = 0; i < n; i++) {
        acc = (acc * 31U) + (unsigned int)(unsigned char)rx_buf[i];
    }
    return (int)(acc + (t1 - t0));
}
