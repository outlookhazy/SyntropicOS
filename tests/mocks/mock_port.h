/**
 * @file mock_port.h
 * @brief Shared mock port layer for host-side testing.
 *
 * Provides injectable mock state for all port interfaces so tests can
 * control hardware behavior without real MCU peripherals.
 */

#ifndef MOCK_PORT_H
#define MOCK_PORT_H

#include <stdint.h>
#include <stdbool.h>
#include "syntropic/port/syn_port_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Tick source ────────────────────────────────────────────────────────── */

extern uint32_t mock_tick_ms;

/** Advance the mock system tick by @p ms milliseconds. */
void mock_tick_advance(uint32_t ms);

/* ── GPIO ───────────────────────────────────────────────────────────────── */

extern uint8_t mock_gpio_states[32];
extern uint8_t mock_gpio_modes[32];
extern int16_t mock_gpio_read_overrides[32]; // -1 for no override, else state

/** GPIO write callback — called whenever syn_port_gpio_write is invoked. */
typedef void (*MockGpioWriteCallback)(uint16_t pin, uint8_t state, void *ctx);
void mock_gpio_set_write_callback(MockGpioWriteCallback cb, void *ctx);


/* ── ADC ────────────────────────────────────────────────────────────────── */

/** Set the raw ADC value returned by syn_port_adc_read(). */
extern uint16_t mock_adc_value;

/* ── Flash ──────────────────────────────────────────────────────────────── */

#define MOCK_FLASH_SIZE     4096
#define MOCK_FLASH_SECTOR   1024

extern uint8_t mock_flash[MOCK_FLASH_SIZE];

/**
 * If set >= 0, the next flash read/write/erase touching this byte offset
 * returns SYN_ERROR (one-shot: reset to -1 after triggering).
 */
extern int32_t mock_flash_fail_at;
/** Set true to make the next flash write (only) fail. One-shot. */
extern bool    mock_flash_write_fail_next;

/* ── Sleep ──────────────────────────────────────────────────────────────── */

extern int mock_sleep_count;

/* ── UART ───────────────────────────────────────────────────────────────── */

#define MOCK_UART_BUF_SIZE 256

extern uint8_t mock_uart_rx_buf[MOCK_UART_BUF_SIZE];
extern size_t  mock_uart_rx_len;
extern size_t  mock_uart_rx_pos;
extern uint8_t mock_uart_tx_buf[MOCK_UART_BUF_SIZE];
extern size_t  mock_uart_tx_len;
extern bool    mock_uart_init_fail;

/* ── CAN ────────────────────────────────────────────────────────────────── */

#include "syntropic/drivers/syn_can.h"

extern SYN_CAN_Frame mock_can_rx;
extern bool           mock_can_rx_avail;
extern bool           mock_can_tx_ok;
/** Set true to make the next syn_port_can_init() return false (one-shot). */
extern bool           mock_can_init_fail;

/* ── SPI ────────────────────────────────────────────────────────────────── */

#define MOCK_SPI_BUF_SIZE 600

extern uint8_t mock_spi_rx_buf[MOCK_SPI_BUF_SIZE]; /**< Canned bytes returned by transfer */
extern size_t  mock_spi_rx_len;                     /**< Total bytes loaded                */
extern size_t  mock_spi_rx_pos;                     /**< Read cursor                       */
extern uint8_t mock_spi_tx_buf[MOCK_SPI_BUF_SIZE]; /**< Captured bytes sent               */
extern size_t  mock_spi_tx_len;                     /**< Bytes captured so far             */
extern bool    mock_spi_init_ok;                    /**< Controls syn_port_spi_init result */
extern bool mock_spi_infinite;
extern uint8_t mock_spi_infinite_byte;                    /**< If true, returns 0x00 when buffer empty */

/** Load canned response bytes into the mock SPI receive buffer. */
void mock_spi_set_response(const void *data, size_t len);

/* ── Socket ─────────────────────────────────────────────────────────────── */

#define MOCK_SOCK_BUF_SIZE  4096

extern uint8_t  mock_sock_rx_buf[MOCK_SOCK_BUF_SIZE]; /**< Canned recv data */
extern size_t   mock_sock_rx_len;                      /**< Total bytes      */
extern size_t   mock_sock_rx_pos;                      /**< Read cursor      */
extern uint8_t  mock_sock_tx_buf[MOCK_SOCK_BUF_SIZE]; /**< Captured sends   */
extern size_t   mock_sock_tx_len;
extern bool     mock_sock_connected;
extern void (*mock_sock_connect_cb)(const char *host, uint16_t port);
extern bool     mock_sock_eof_on_empty;
extern bool     mock_sock_connect_fail;
extern bool     mock_sock_send_fail;
extern int      mock_sock_send_fail_after_bytes;

/* Server-side mock */
extern bool     mock_sock_listen_ok;
extern bool     mock_sock_accept_ok;

/* UDP mock */
#define MOCK_UDP_BUF_SIZE 2048
extern uint8_t      mock_udp_rx_buf[MOCK_UDP_BUF_SIZE];
extern size_t       mock_udp_rx_len;
extern size_t       mock_udp_rx_pos;
extern SYN_SockAddr mock_udp_rx_from;
extern uint8_t      mock_udp_tx_buf[MOCK_UDP_BUF_SIZE];
extern size_t       mock_udp_tx_len;
extern SYN_SockAddr mock_udp_tx_to;
extern bool         mock_udp_open_ok;
extern bool         mock_udp_multicast_join_ok;
extern bool         mock_udp_sendto_fail;

void mock_udp_set_response(const void *data, size_t len, const SYN_SockAddr *from);

/** Load canned data into the mock socket receive buffer. */
void mock_sock_set_response(const void *data, size_t len);

/* ── RTC ────────────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_rtc.h"  /* SYN_RTC_DateTime */

extern SYN_RTC_DateTime mock_rtc_time;    /**< Current time returned by syn_port_rtc_get */
extern bool             mock_rtc_init_ok; /**< Controls syn_port_rtc_init result          */

/* ── Hardware Watchdog ────────────────────────────────────────────────────── */

extern bool     mock_wdt_init_ok;       /**< Controls syn_port_wdt_init result            */
extern uint32_t mock_wdt_timeout_ms;    /**< Timeout configured by syn_port_wdt_init      */
extern uint32_t mock_wdt_feed_count;    /**< Number of times syn_port_wdt_feed was called  */

/* ── DAC ────────────────────────────────────────────────────────────────── */

#define MOCK_DAC_MAX_CHANNELS 8u

extern uint16_t mock_dac_values[MOCK_DAC_MAX_CHANNELS]; /**< Last raw value written per channel */
extern bool     mock_dac_init_ok;                       /**< Controls syn_port_dac_init result  */

/* ── Reset ────────────────────────────────────────────────────────────────── */

/** Reset all mock state to defaults. Call from setUp(). */
void mock_port_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_PORT_H */
