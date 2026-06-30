/**
 * @file mock_port.c
 * @brief Shared mock port layer implementation.
 *
 * Provides all syn_port_* function implementations backed by simple
 * in-memory state that tests can control via the mock_* variables.
 */

#include "mock_port.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syntropic/common/syn_defs.h"
#include "syntropic/common/syn_compiler.h"

/* ── Tick source ────────────────────────────────────────────────────────── */

uint32_t mock_tick_ms = 0;

void mock_tick_advance(uint32_t ms) { mock_tick_ms += ms; }

uint32_t syn_port_get_tick_ms(void)     { return mock_tick_ms; }
void     syn_port_delay_ms(uint32_t ms) { mock_tick_ms += ms; }
void     syn_port_enter_critical(void)  { /* no-op on host */ }
void     syn_port_exit_critical(void)   { /* no-op on host */ }

/* ── GPIO ───────────────────────────────────────────────────────────────── */

uint8_t mock_gpio_states[32];
uint8_t mock_gpio_modes[32];
int16_t mock_gpio_read_overrides[32];

static MockGpioWriteCallback s_gpio_write_cb = NULL;
static void *s_gpio_write_ctx = NULL;

void mock_gpio_set_write_callback(MockGpioWriteCallback cb, void *ctx)
{
    s_gpio_write_cb = cb;
    s_gpio_write_ctx = ctx;
}

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    if (pin >= 32) return SYN_INVALID_PARAM;
    mock_gpio_modes[pin] = (uint8_t)mode;
    mock_gpio_states[pin] = 0;
    mock_gpio_read_overrides[pin] = -1;
    return SYN_OK;
}

SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin)
{
    (void)pin;
    return SYN_OK;
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    if (pin >= 32) return SYN_INVALID_PARAM;
    mock_gpio_states[pin] = (uint8_t)state;
    if (s_gpio_write_cb != NULL) {
        s_gpio_write_cb(pin, (uint8_t)state, s_gpio_write_ctx);
    }
    return SYN_OK;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    if (pin >= 32) return SYN_GPIO_LOW;
    if (mock_gpio_read_overrides[pin] != -1) {
        return (SYN_GPIO_State)mock_gpio_read_overrides[pin];
    }
    return (SYN_GPIO_State)mock_gpio_states[pin];
}

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    if (pin >= 32) return SYN_INVALID_PARAM;
    mock_gpio_states[pin] ^= 1;
    return SYN_OK;
}

/* ── UART ───────────────────────────────────────────────────────────────── */

uint8_t mock_uart_rx_buf[MOCK_UART_BUF_SIZE];
size_t  mock_uart_rx_len = 0;
size_t  mock_uart_rx_pos = 0;
uint8_t mock_uart_tx_buf[MOCK_UART_BUF_SIZE];
size_t  mock_uart_tx_len = 0;

bool    mock_uart_init_fail = false;
SYN_Status syn_port_uart_init(SYN_UARTInstance i, uint32_t b)    { (void)i; (void)b; return mock_uart_init_fail ? SYN_ERROR : SYN_OK; }
SYN_Status syn_port_uart_deinit(SYN_UARTInstance i)              { (void)i; return SYN_OK; }

SYN_Status syn_port_uart_transmit(SYN_UARTInstance i, const uint8_t *d, size_t l, uint32_t t)
{
    (void)i; (void)t;
    for (size_t idx = 0; idx < l; idx++) {
        if (mock_uart_tx_len < MOCK_UART_BUF_SIZE) {
            mock_uart_tx_buf[mock_uart_tx_len++] = d[idx];
        }
    }
    return SYN_OK;
}

SYN_Status syn_port_uart_receive(SYN_UARTInstance i, uint8_t *d, size_t l, size_t *r, uint32_t t)
{
    (void)i; (void)t;
    size_t idx = 0;
    while (idx < l && mock_uart_rx_pos < mock_uart_rx_len) {
        d[idx++] = mock_uart_rx_buf[mock_uart_rx_pos++];
    }
    if (r) *r = idx;
    return (idx == l) ? SYN_OK : SYN_TIMEOUT;
}

SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance i, uint8_t b)
{
    return syn_port_uart_transmit(i, &b, 1, 0);
}

SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance i, uint8_t *b, uint32_t t)
{
    (void)i; (void)t;
    if (mock_uart_rx_pos < mock_uart_rx_len) {
        *b = mock_uart_rx_buf[mock_uart_rx_pos++];
        return SYN_OK;
    }
    return SYN_TIMEOUT;
}

/* ── Assert handler ─────────────────────────────────────────────────────── */

void syn_assert_failed(const char *file, int line)
{
    fprintf(stderr, "ASSERT FAILED: %s:%d\n", file, line);
    exit(1);
}

/* ── Flash ──────────────────────────────────────────────────────────────── */

uint8_t mock_flash[MOCK_FLASH_SIZE];
int32_t mock_flash_fail_at = -1;
bool    mock_flash_write_fail_next = false; /* fails the next write only (one-shot) */

SYN_Status syn_port_flash_erase(uint32_t addr)
{
    if (mock_flash_fail_at >= 0 &&
        (uint32_t)mock_flash_fail_at >= addr &&
        (uint32_t)mock_flash_fail_at < addr + MOCK_FLASH_SECTOR) {
        mock_flash_fail_at = -1; /* one-shot */
        return SYN_ERROR;
    }
    if (addr + MOCK_FLASH_SECTOR > MOCK_FLASH_SIZE) return SYN_ERROR;
    memset(&mock_flash[addr], 0xFF, MOCK_FLASH_SECTOR);
    return SYN_OK;
}

SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len)
{
    if (mock_flash_fail_at >= 0 &&
        (uint32_t)mock_flash_fail_at >= addr &&
        (uint32_t)mock_flash_fail_at < addr + len) {
        mock_flash_fail_at = -1;
        return SYN_ERROR;
    }
    if (addr + len > MOCK_FLASH_SIZE) return SYN_ERROR;
    memcpy(buf, &mock_flash[addr], len);
    return SYN_OK;
}

SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len)
{
    if (mock_flash_write_fail_next) {
        mock_flash_write_fail_next = false; /* one-shot */
        return SYN_ERROR;
    }
    if (mock_flash_fail_at >= 0 &&
        (uint32_t)mock_flash_fail_at >= addr &&
        (uint32_t)mock_flash_fail_at < addr + len) {
        mock_flash_fail_at = -1;
        return SYN_ERROR;
    }
    if (addr + len > MOCK_FLASH_SIZE) return SYN_ERROR;
    const uint8_t *src = (const uint8_t *)buf;
    for (size_t i = 0; i < len; i++) {
        mock_flash[addr + i] &= src[i];
    }
    return SYN_OK;
}

uint32_t syn_port_flash_sector_size(uint32_t addr)
{
    (void)addr;
    return MOCK_FLASH_SECTOR;
}


/* ── I2C ────────────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_i2c.h"
SYN_Status syn_port_i2c_init(const SYN_I2C_Config *c)     { (void)c; return SYN_OK; }
SYN_Status syn_port_i2c_deinit(uint8_t b)                  { (void)b; return SYN_OK; }
SYN_Status syn_port_i2c_write(uint8_t b, uint8_t a, const uint8_t *d, size_t l)
    { (void)b; (void)a; (void)d; (void)l; return SYN_OK; }
SYN_Status syn_port_i2c_read(uint8_t b, uint8_t a, uint8_t *d, size_t l)
    { (void)b; (void)a; (void)d; (void)l; return SYN_OK; }
SYN_Status syn_port_i2c_write_read(uint8_t b, uint8_t a, const uint8_t *t, size_t tl, uint8_t *r, size_t rl)
    { (void)b; (void)a; (void)t; (void)tl; (void)r; (void)rl; return SYN_OK; }

/* ── ADC ────────────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_adc.h"
uint16_t mock_adc_value = 2048;
SYN_Status syn_port_adc_init(uint8_t ch)   { (void)ch; return SYN_OK; }
uint16_t syn_port_adc_read(uint8_t ch)      { (void)ch; return mock_adc_value; }
uint8_t  syn_port_adc_resolution(void)      { return 12; }
uint16_t syn_port_adc_reference_mv(void)    { return 3300; }

/* ── Sleep ──────────────────────────────────────────────────────────────── */

#include "syntropic/system/syn_sleep.h"
int mock_sleep_count = 0;
int mock_sleep_until_count = 0;
uint32_t mock_sleep_until_tick = 0;
void syn_port_sleep(SYN_SleepMode m) { (void)m; mock_sleep_count++; }
void syn_port_sleep_until(uint32_t wake_tick_ms) {
    mock_sleep_until_count++;
    mock_sleep_until_tick = wake_tick_ms;
    /* Advance mock tick to the wake time */
    if ((int32_t)(wake_tick_ms - mock_tick_ms) > 0) {
        mock_tick_ms = wake_tick_ms;
    }
}

/* ── EXTI ───────────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_exti.h"
SYN_Status syn_port_exti_configure(SYN_GPIO_Pin p, SYN_EXTI_Edge e)
    { (void)p; (void)e; return SYN_OK; }
void syn_port_exti_enable(SYN_GPIO_Pin p)         { (void)p; }
void syn_port_exti_disable(SYN_GPIO_Pin p)        { (void)p; }
void syn_port_exti_clear_pending(SYN_GPIO_Pin p)  { (void)p; }

/* ── CAN ────────────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_can.h"
SYN_CAN_Frame mock_can_rx;
bool           mock_can_rx_avail = false;
bool           mock_can_tx_ok = true;

bool mock_can_init_fail = false;
bool syn_port_can_init(uint8_t p, uint32_t br)
    { (void)p; (void)br; if (mock_can_init_fail) { mock_can_init_fail = false; return false; } return true; }

bool syn_port_can_send(uint8_t p, uint32_t id, bool ext, const uint8_t *d, uint8_t dl)
    { (void)p; (void)id; (void)ext; (void)d; (void)dl; return mock_can_tx_ok; }

bool syn_port_can_receive(uint8_t p, uint32_t *id, bool *ext, uint8_t *d, uint8_t *dl)
{
    (void)p;
    if (!mock_can_rx_avail) return false;
    *id = mock_can_rx.id; *ext = mock_can_rx.extended; *dl = mock_can_rx.dlc;
    memcpy(d, mock_can_rx.data, mock_can_rx.dlc);
    mock_can_rx_avail = false;
    return true;
}

void syn_port_can_set_filter(uint8_t p, uint32_t id, uint32_t m)
    { (void)p; (void)id; (void)m; }

/* ── PWM port (mock for DC motor/servo) ─────────────────────────────────── */

void syn_port_pwm_set_duty(uint8_t ch, uint16_t duty) { (void)ch; (void)duty; }

/* ── RTC port ───────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_rtc.h"

SYN_RTC_DateTime mock_rtc_time;
bool             mock_rtc_init_ok = true;

SYN_Status syn_port_rtc_init(void)
{
    return mock_rtc_init_ok ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_rtc_get(SYN_RTC_DateTime *dt)
{
    if (dt == NULL) return SYN_INVALID_PARAM;
    *dt = mock_rtc_time;
    return SYN_OK;
}

SYN_Status syn_port_rtc_set(const SYN_RTC_DateTime *dt)
{
    if (dt == NULL) return SYN_INVALID_PARAM;
    mock_rtc_time = *dt;
    return SYN_OK;
}

/* ── Hardware Watchdog port ─────────────────────────────────────────────── */

#include "syntropic/port/syn_port_wdt.h"

bool     mock_wdt_init_ok    = true;
uint32_t mock_wdt_timeout_ms = 0u;
uint32_t mock_wdt_feed_count = 0u;

SYN_Status syn_port_wdt_init(uint32_t timeout_ms)
{
    mock_wdt_timeout_ms = timeout_ms;
    return mock_wdt_init_ok ? SYN_OK : SYN_ERROR;
}

void syn_port_wdt_feed(void)
{
    mock_wdt_feed_count++;
}

/* ── DAC port ───────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_dac.h"

uint16_t mock_dac_values[MOCK_DAC_MAX_CHANNELS];
bool     mock_dac_init_ok = true;

SYN_Status syn_port_dac_init(uint8_t channel)
{
    (void)channel;
    return mock_dac_init_ok ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_dac_write(uint8_t channel, uint16_t value)
{
    if (channel < MOCK_DAC_MAX_CHANNELS) {
        mock_dac_values[channel] = value;
    }
    return SYN_OK;
}

uint8_t  syn_port_dac_resolution(void)  { return 12u; }
uint16_t syn_port_dac_reference_mv(void){ return 3300u; }

/* ── SPI port (mock for SD card driver) ─────────────────────────────────── */

#include "syntropic/port/syn_port_spi.h"

uint8_t mock_spi_rx_buf[MOCK_SPI_BUF_SIZE];
size_t  mock_spi_rx_len = 0;
size_t  mock_spi_rx_pos = 0;
uint8_t mock_spi_tx_buf[MOCK_SPI_BUF_SIZE];
size_t  mock_spi_tx_len = 0;
bool    mock_spi_init_ok = true;
bool mock_spi_infinite = false;
uint8_t mock_spi_infinite_byte = 0xFF;

void mock_spi_set_response(const void *data, size_t len)
{
    if (len > MOCK_SPI_BUF_SIZE) len = MOCK_SPI_BUF_SIZE;
    memcpy(mock_spi_rx_buf, data, len);
    mock_spi_rx_len = len;
    mock_spi_rx_pos = 0;
}

SYN_Status syn_port_spi_init(const SYN_SPI_Config *cfg)
{
    (void)cfg;
    return mock_spi_init_ok ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_spi_deinit(uint8_t bus)
{
    (void)bus;
    return SYN_OK;
}

SYN_Status syn_port_spi_transfer(uint8_t bus,
                                  const uint8_t *tx_buf,
                                  uint8_t *rx_buf,
                                  size_t len)
{
    size_t i;
    (void)bus;
    for (i = 0; i < len; i++) {
        /* Capture transmitted bytes */
        if (tx_buf != NULL && mock_spi_tx_len < MOCK_SPI_BUF_SIZE) {
            mock_spi_tx_buf[mock_spi_tx_len++] = tx_buf[i];
        }
        /* Provide canned receive bytes; return 0xFF when buffer exhausted.
         * Only advance the cursor when the caller provides an rx buffer —
         * matching the SD protocol where command bytes (tx only, rx=NULL)
         * do not consume canned receive slots. */
        if (rx_buf != NULL) {
            if (mock_spi_rx_pos < mock_spi_rx_len) {
                rx_buf[i] = mock_spi_rx_buf[mock_spi_rx_pos++];
            } else {
                rx_buf[i] = mock_spi_infinite ? mock_spi_infinite_byte : 0xFFu;
            }
        }
    }
    return SYN_OK;
}

SYN_Status syn_port_spi_cs_assert(uint8_t bus, SYN_GPIO_Pin cs_pin)
{
    (void)bus; (void)cs_pin;
    return SYN_OK;
}

SYN_Status syn_port_spi_cs_deassert(uint8_t bus, SYN_GPIO_Pin cs_pin)
{
    (void)bus; (void)cs_pin;
    return SYN_OK;
}

/* ── Socket port (mock for HTTP client) ─────────────────────────────────── */

#include "syntropic/port/syn_port_socket.h"

uint8_t  mock_sock_rx_buf[MOCK_SOCK_BUF_SIZE];
size_t   mock_sock_rx_len;
size_t   mock_sock_rx_pos;
uint8_t  mock_sock_tx_buf[MOCK_SOCK_BUF_SIZE];
size_t   mock_sock_tx_len;
bool     mock_sock_connected;
bool     mock_sock_eof_on_empty = false;
bool     mock_sock_connect_fail = false;
bool     mock_sock_send_fail = false;
int      mock_sock_send_fail_after_bytes = -1;

/* Server-side mock */
bool     mock_sock_listen_ok = true;    /**< syn_port_sock_listen returns ok */
bool     mock_sock_accept_ok = true;    /**< syn_port_sock_accept returns ok */

void mock_sock_set_response(const void *data, size_t len)
{
    if (len > MOCK_SOCK_BUF_SIZE) len = MOCK_SOCK_BUF_SIZE;
    memcpy(mock_sock_rx_buf, data, len);
    mock_sock_rx_len = len;
    mock_sock_rx_pos = 0;
}

void (*mock_sock_connect_cb)(const char *host, uint16_t port) = NULL;

SYN_Socket syn_port_sock_connect(const SYN_SockAddr *addr)
{
    if (mock_sock_connect_fail) {
        mock_sock_connected = false;
        return -1;
    }
    mock_sock_connected = true;
    mock_sock_rx_pos = 0;
    mock_sock_tx_len = 0;
    if (mock_sock_connect_cb != NULL && addr != NULL) {
        char host_ip[16];
        snprintf(host_ip, sizeof(host_ip), "%d.%d.%d.%d", addr->ip[0], addr->ip[1], addr->ip[2], addr->ip[3]);
        mock_sock_connect_cb(host_ip, addr->port);
    }
    return 0;
}

SYN_Socket syn_port_sock_connect_host(const char *host, uint16_t port)
{
    if (mock_sock_connect_fail) {
        mock_sock_connected = false;
        return -1;
    }
    mock_sock_connected = true;
    mock_sock_rx_pos = 0;
    mock_sock_tx_len = 0;
    if (mock_sock_connect_cb != NULL) {
        mock_sock_connect_cb(host, port);
    }
    return 0;
}

int syn_port_sock_send(SYN_Socket sock, const void *data, size_t len)
{
    (void)sock;
    if (!mock_sock_connected || mock_sock_send_fail) return -1;
    if (mock_sock_send_fail_after_bytes >= 0 && (int)mock_sock_tx_len >= mock_sock_send_fail_after_bytes) return -1;
    size_t space = MOCK_SOCK_BUF_SIZE - mock_sock_tx_len;
    if (mock_sock_send_fail_after_bytes >= 0) {
        size_t limit = (size_t)(mock_sock_send_fail_after_bytes - (int)mock_sock_tx_len);
        if (space > limit) space = limit;
    }
    if (len > space) len = space;
    memcpy(mock_sock_tx_buf + mock_sock_tx_len, data, len);
    mock_sock_tx_len += len;
    return (int)len;
}

int syn_port_sock_send_all(SYN_Socket sock, const void *data, size_t len)
{
    (void)sock;
    if (!mock_sock_connected || mock_sock_send_fail) return -1;
    if (mock_sock_send_fail_after_bytes >= 0 && (int)mock_sock_tx_len + (int)len > mock_sock_send_fail_after_bytes) return -1;
    size_t space = MOCK_SOCK_BUF_SIZE - mock_sock_tx_len;
    if (len > space) return -1;
    memcpy(mock_sock_tx_buf + mock_sock_tx_len, data, len);
    mock_sock_tx_len += len;
    return (int)len;
}

int syn_port_sock_recv(SYN_Socket sock, void *buf, size_t max_len,
                       uint32_t timeout_ms)
{
    (void)sock; (void)timeout_ms;
    if (!mock_sock_connected) return -1;
    size_t avail = mock_sock_rx_len - mock_sock_rx_pos;
    if (avail == 0) {
        if (mock_sock_eof_on_empty) return 0;
        return -1; /* timeout / no data */
    }
    if (max_len > avail) max_len = avail;
    memcpy(buf, mock_sock_rx_buf + mock_sock_rx_pos, max_len);
    mock_sock_rx_pos += max_len;
    return (int)max_len;
}

void syn_port_sock_close(SYN_Socket sock)
{
    (void)sock;
    mock_sock_connected = false;
}

/* ── UDP Mocks ──────────────────────────────────────────────────────────── */

uint8_t      mock_udp_rx_buf[MOCK_UDP_BUF_SIZE];
size_t       mock_udp_rx_len = 0;
size_t       mock_udp_rx_pos = 0;
SYN_SockAddr mock_udp_rx_from;
uint8_t      mock_udp_tx_buf[MOCK_UDP_BUF_SIZE];
size_t       mock_udp_tx_len = 0;
SYN_SockAddr mock_udp_tx_to;
bool         mock_udp_open_ok = true;
bool         mock_udp_multicast_join_ok = true;
bool         mock_udp_sendto_fail = false;

void mock_udp_set_response(const void *data, size_t len, const SYN_SockAddr *from)
{
    if (len > MOCK_UDP_BUF_SIZE) len = MOCK_UDP_BUF_SIZE;
    memcpy(mock_udp_rx_buf, data, len);
    mock_udp_rx_len = len;
    mock_udp_rx_pos = 0;
    if (from != NULL) {
        mock_udp_rx_from = *from;
    } else {
        memset(&mock_udp_rx_from, 0, sizeof(mock_udp_rx_from));
    }
}

SYN_Socket syn_port_udp_open(uint16_t port)
{
    (void)port;
    return mock_udp_open_ok ? 20 : SYN_SOCKET_INVALID;
}

int syn_port_udp_sendto(SYN_Socket sock, const void *data, size_t len,
                        const SYN_SockAddr *to)
{
    (void)sock;
    if (mock_udp_sendto_fail) return -1;
    if (to != NULL) {
        mock_udp_tx_to = *to;
    }
    size_t space = MOCK_UDP_BUF_SIZE - mock_udp_tx_len;
    if (len > space) len = space;
    memcpy(mock_udp_tx_buf + mock_udp_tx_len, data, len);
    mock_udp_tx_len += len;
    return (int)len;
}

int syn_port_udp_recvfrom(SYN_Socket sock, void *buf, size_t max_len,
                          SYN_SockAddr *from, uint32_t timeout_ms)
{
    (void)sock; (void)timeout_ms;
    size_t avail = mock_udp_rx_len - mock_udp_rx_pos;
    if (avail == 0) return -1; /* Timeout/no data */
    if (max_len > avail) max_len = avail;
    memcpy(buf, mock_udp_rx_buf + mock_udp_rx_pos, max_len);
    mock_udp_rx_pos += max_len;
    if (from != NULL) {
        *from = mock_udp_rx_from;
    }
    return (int)max_len;
}

SYN_Status syn_port_udp_join_multicast(SYN_Socket sock, const char *multicast_ip)
{
    (void)sock; (void)multicast_ip;
    return mock_udp_multicast_join_ok ? SYN_OK : SYN_ERROR;
}

SYN_Socket syn_port_sock_listen(uint16_t port, int backlog)
{
    (void)port; (void)backlog;
    return mock_sock_listen_ok ? 10 : SYN_SOCKET_INVALID;
}

SYN_Socket syn_port_sock_accept(SYN_Socket listener, uint32_t timeout_ms)
{
    (void)listener; (void)timeout_ms;
    if (!mock_sock_accept_ok) return SYN_SOCKET_INVALID;
    mock_sock_connected = true;
    /* Reset rx pos so the canned data can be re-read */
    mock_sock_rx_pos = 0;
    return 11;
}

/* ── Reset all mock state ───────────────────────────────────────────────── */

void mock_port_reset(void)
{
    mock_tick_ms = 0;
    memset(mock_gpio_states, 0, sizeof(mock_gpio_states));
    memset(mock_gpio_modes, 0, sizeof(mock_gpio_modes));
    for (int i = 0; i < 32; i++) {
        mock_gpio_read_overrides[i] = -1;
    }
    mock_adc_value = 2048;
    memset(mock_flash, 0xFF, sizeof(mock_flash));
    mock_flash_fail_at = -1;
    mock_sleep_count = 0;
    mock_sleep_until_count = 0;
    mock_sleep_until_tick = 0;
    memset(&mock_can_rx, 0, sizeof(mock_can_rx));
    mock_can_rx_avail = false;
    mock_can_tx_ok = true;
    s_gpio_write_cb = NULL;
    s_gpio_write_ctx = NULL;

    /* SPI */
    memset(mock_spi_rx_buf, 0, sizeof(mock_spi_rx_buf));
    mock_spi_rx_len = 0;
    mock_spi_rx_pos = 0;
    memset(mock_spi_tx_buf, 0, sizeof(mock_spi_tx_buf));
    mock_spi_tx_len = 0;
    mock_spi_init_ok = true;
    mock_spi_infinite = false;
    mock_spi_infinite_byte = 0xFF;

    /* RTC */
    memset(&mock_rtc_time, 0, sizeof(mock_rtc_time));
    mock_rtc_init_ok = true;

    /* Hardware Watchdog */
    mock_wdt_init_ok    = true;
    mock_wdt_timeout_ms = 0u;
    mock_wdt_feed_count = 0u;

    /* DAC */
    memset(mock_dac_values, 0, sizeof(mock_dac_values));
    mock_dac_init_ok = true;

    /* Socket */
    memset(mock_sock_rx_buf, 0, sizeof(mock_sock_rx_buf));
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    memset(mock_sock_tx_buf, 0, sizeof(mock_sock_tx_buf));
    mock_sock_tx_len = 0;
    mock_sock_connected = false;
    mock_sock_listen_ok = true;
    mock_sock_accept_ok = true;
    mock_sock_connect_cb = NULL;
    mock_sock_eof_on_empty = false;
    mock_sock_connect_fail = false;
    mock_sock_send_fail = false;
    mock_sock_send_fail_after_bytes = -1;

    /* UDP Mock */
    memset(mock_udp_rx_buf, 0, sizeof(mock_udp_rx_buf));
    mock_udp_rx_len = 0;
    mock_udp_rx_pos = 0;
    memset(&mock_udp_rx_from, 0, sizeof(mock_udp_rx_from));
    memset(mock_udp_tx_buf, 0, sizeof(mock_udp_tx_buf));
    mock_udp_tx_len = 0;
    memset(&mock_udp_tx_to, 0, sizeof(mock_udp_tx_to));
    mock_udp_open_ok = true;
    mock_udp_multicast_join_ok = true;
    mock_udp_sendto_fail = false;

    /* UART Mock */
    memset(mock_uart_rx_buf, 0, sizeof(mock_uart_rx_buf));
    mock_uart_rx_len = 0;
    mock_uart_rx_pos = 0;
    memset(mock_uart_tx_buf, 0, sizeof(mock_uart_tx_buf));
    mock_uart_tx_len = 0;
    mock_uart_init_fail = false;
}

