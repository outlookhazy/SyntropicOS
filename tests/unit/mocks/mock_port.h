/**
 * @file mock_port.h
 * @brief Shared mock port layer for host-side testing.
 *
 * Provides injectable mock state for all port interfaces so tests can
 * control hardware behavior without real MCU peripherals.
 */

#ifndef MOCK_PORT_H
#define MOCK_PORT_H

#include "syntropic/port/syn_port_comp.h"
#include "syntropic/port/syn_port_socket.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Tick source ────────────────────────────────────────────────────────── */

extern uint32_t mock_tick_ms;
extern uint32_t mock_tick_us;
extern volatile uint32_t mock_hpclock_lsb;
extern uint32_t mock_hpclock_freq_hz;

/** Advance the mock system tick by @p ms milliseconds. */
void mock_tick_advance(uint32_t ms);

/** Advance the mock system tick by @p us microseconds. */
void mock_tick_advance_us(uint32_t us);

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
extern uint8_t mock_adc_resolution;

/* ── Flash ──────────────────────────────────────────────────────────────── */

#define MOCK_FLASH_SIZE 4096
#define MOCK_FLASH_SECTOR 1024

extern uint8_t mock_flash[MOCK_FLASH_SIZE];

/**
 * If set >= 0, the next flash read/write/erase touching this byte offset
 * returns SYN_ERROR (one-shot: reset to -1 after triggering).
 */
extern int32_t mock_flash_fail_at;
/** If set >= 0, decrements on each syn_port_flash_read and returns SYN_ERROR at 0 (one-shot). */
extern int32_t mock_flash_read_fail_after;
/** Set true to make the next flash write (only) fail. One-shot. */
extern bool mock_flash_write_fail_next;

/* ── Critical sections ──────────────────────────────────────────────────── */

/** Current syn_port_enter/exit_critical nesting depth (0 = interrupts on). */
extern int mock_critical_depth;
/** Total syn_port_enter_critical calls since reset. */
extern int mock_critical_enter_count;
/** Optional deterministic hook invoked on syn_port_enter_critical (reset to NULL). */
extern void (*mock_critical_enter_cb)(void);

/* ── Sleep ──────────────────────────────────────────────────────────────── */

extern int mock_sleep_count;
extern int mock_sleep_until_count;
extern uint32_t mock_sleep_until_tick;
/** mock_critical_depth captured when syn_port_sleep was last called. */
extern int mock_sleep_critical_depth;
/** mock_critical_depth captured when syn_port_sleep_until was last called. */
extern int mock_sleep_until_critical_depth;

/* ── UART ───────────────────────────────────────────────────────────────── */

#define MOCK_UART_BUF_SIZE 256

extern uint8_t mock_uart_rx_buf[MOCK_UART_BUF_SIZE];
extern size_t mock_uart_rx_len;
extern size_t mock_uart_rx_pos;
extern uint8_t mock_uart_tx_buf[MOCK_UART_BUF_SIZE];
extern size_t mock_uart_tx_len;
extern bool mock_uart_init_fail;

/* ── Console serial ────────────────────────────────────────────────────── */

#define MOCK_SERIAL_BUF_SIZE 4096

extern uint8_t mock_serial_tx_buf[MOCK_SERIAL_BUF_SIZE]; /**< Captured console output */
extern size_t mock_serial_tx_len;
extern uint8_t mock_serial_rx_buf[MOCK_SERIAL_BUF_SIZE]; /**< Canned console input    */
extern size_t mock_serial_rx_len;
extern size_t mock_serial_rx_pos;

/* ── CAN ────────────────────────────────────────────────────────────────── */

#include "syntropic/drivers/syn_can.h"

extern SYN_CAN_Frame mock_can_rx;
extern bool mock_can_rx_avail;
extern bool mock_can_tx_ok;
/** Set true to make the next syn_port_can_init() return false (one-shot). */
extern bool mock_can_init_fail;

/* ── SPI ────────────────────────────────────────────────────────────────── */

#define MOCK_SPI_BUF_SIZE 600

extern uint8_t mock_spi_rx_buf[MOCK_SPI_BUF_SIZE]; /**< Canned bytes returned by transfer */
extern size_t mock_spi_rx_len;                     /**< Total bytes loaded                */
extern size_t mock_spi_rx_pos;                     /**< Read cursor                       */
extern uint8_t mock_spi_tx_buf[MOCK_SPI_BUF_SIZE]; /**< Captured bytes sent               */
extern size_t mock_spi_tx_len;                     /**< Bytes captured so far             */
extern bool mock_spi_init_ok;                      /**< Controls syn_port_spi_init result */
extern bool mock_spi_infinite;
extern uint8_t mock_spi_infinite_byte; /**< If true, returns 0x00 when buffer empty */

/** Load canned response bytes into the mock SPI receive buffer. */
void mock_spi_set_response(const void *data, size_t len);

/** Optional SPI transfer hook — overrides default canned buffer when non-NULL. */
typedef SYN_Status (*MockSpiTransferCallback)(uint8_t bus, const uint8_t *tx_buf, uint8_t *rx_buf,
                                              size_t len);
void mock_spi_set_transfer_callback(MockSpiTransferCallback cb);

/** Optional I2C transfer hook — overrides default no-op when non-NULL. */
typedef SYN_Status (*MockI2cTransferCallback)(uint8_t i2c_id, uint16_t addr, const uint8_t *tx,
                                              size_t tx_len, uint8_t *rx, size_t rx_len);
void mock_i2c_set_transfer_callback(MockI2cTransferCallback cb);

/* ── Socket ─────────────────────────────────────────────────────────────── */

#define MOCK_SOCK_BUF_SIZE 4096

extern uint8_t mock_sock_rx_buf[MOCK_SOCK_BUF_SIZE]; /**< Canned recv data */
extern size_t mock_sock_rx_len;                      /**< Total bytes      */
extern size_t mock_sock_rx_pos;                      /**< Read cursor      */
extern uint8_t mock_sock_tx_buf[MOCK_SOCK_BUF_SIZE]; /**< Captured sends   */
extern size_t mock_sock_tx_len;
extern bool mock_sock_connected;
extern void (*mock_sock_connect_cb)(const char *host, uint16_t port);
extern bool mock_sock_eof_on_empty;
extern bool mock_sock_connect_fail;
extern bool mock_sock_send_fail;
extern bool mock_sock_recv_fail;
extern int mock_sock_send_fail_after_bytes;
extern int mock_sock_close_count;
extern SYN_Socket mock_sock_last_closed;

bool syn_port_sock_readable(SYN_Socket sock);
bool syn_port_udp_readable(SYN_Socket sock);

/* Server-side mock */
extern bool mock_sock_listen_ok;
extern bool mock_sock_accept_ok;

/* UDP mock */
#define MOCK_UDP_BUF_SIZE 2048
extern uint8_t mock_udp_tx_buf[MOCK_UDP_BUF_SIZE];
extern size_t mock_udp_tx_len;
extern SYN_SockAddr mock_udp_tx_to;
extern bool mock_udp_open_ok;
extern bool mock_udp_multicast_join_ok;
extern bool mock_udp_sendto_fail;
extern bool mock_udp_recv_fail;

void mock_udp_set_response(const void *data, size_t len, const SYN_SockAddr *from);

/** Load canned data into the mock socket receive buffer. */
void mock_sock_set_response(const void *data, size_t len);

/* ── RTC ────────────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_rtc.h" /* SYN_RTC_DateTime */

extern SYN_RTC_DateTime mock_rtc_time; /**< Current time returned by syn_port_rtc_get */
extern bool mock_rtc_init_ok;          /**< Controls syn_port_rtc_init result          */

/* ── Hardware Watchdog ────────────────────────────────────────────────────── */

extern bool mock_wdt_init_ok;        /**< Controls syn_port_wdt_init result            */
extern uint32_t mock_wdt_timeout_ms; /**< Timeout configured by syn_port_wdt_init      */
extern uint32_t mock_wdt_feed_count; /**< Number of times syn_port_wdt_feed was called  */

/* ── DAC ────────────────────────────────────────────────────────────────── */

#define MOCK_DAC_MAX_CHANNELS 8u

extern uint16_t mock_dac_values[MOCK_DAC_MAX_CHANNELS]; /**< Last raw value written per channel */
extern bool mock_dac_init_ok;                           /**< Controls syn_port_dac_init result  */
extern uint16_t mock_dac_ref_mv;                        /**< Configurable DAC reference mV */

/* ── DMA ────────────────────────────────────────────────────────────────── */

#include "syntropic/drivers/syn_dma.h"

#if defined(SYN_USE_DMA) && SYN_USE_DMA

/** @brief Mock DMA channel state (up to 4 channels). */
typedef struct {
    bool initialized;
    bool busy;
    size_t remaining;
    SYN_DMA_Config cfg;
} MockDmaChannel;

#define MOCK_DMA_MAX_CHANNELS 4

extern MockDmaChannel mock_dma[MOCK_DMA_MAX_CHANNELS];
extern int mock_dma_start_count;
extern int mock_dma_stop_count;
extern uint32_t mock_cache_invalidate_count;

/** Fire the DMA completion callback for a channel (simulates ISR). */
void mock_dma_complete(uint8_t channel, SYN_Status result);

#endif /* SYN_USE_DMA */

/* ── Async I2C ──────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_i2c_async.h"

#if defined(SYN_USE_I2C_ASYNC) && SYN_USE_I2C_ASYNC

extern int mock_i2c_async_count;
extern bool mock_i2c_async_busy;
extern SYN_Status mock_i2c_async_result;

/** Simulate completion — fires the callback from the last xfer. */
void mock_i2c_async_complete(void);

#endif /* SYN_USE_I2C_ASYNC */

/* ── Async SPI ──────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_spi_async.h"

#if defined(SYN_USE_SPI_ASYNC) && SYN_USE_SPI_ASYNC

extern int mock_spi_async_count;
extern bool mock_spi_async_busy;
extern SYN_Status mock_spi_async_result;

/** Simulate completion — fires the callback from the last xfer. */
void mock_spi_async_complete(void);

#endif /* SYN_USE_SPI_ASYNC */

/* ── Multicore mock ────────────────────────────────────────────────────── */

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

#include "syntropic/port/syn_port_spinlock.h"

/** Spinlock held state (true = locked). */
extern bool mock_spinlock_held[SYN_SPINLOCK_COUNT];

/** Number of times each spinlock was acquired. */
extern uint32_t mock_spinlock_acquire_count[SYN_SPINLOCK_COUNT];

/** Simulated core ID (set by test). */
extern uint8_t mock_core_id;

/** Number of times syn_port_ipc_notify() was called. */
extern uint32_t mock_ipc_notify_count;

/** Number of times syn_port_memory_barrier() was called. */
extern uint32_t mock_barrier_count;

#endif /* SYN_USE_MULTICORE */

/* ── Random ───────────────────────────────────────────────────────────────── */

/** Set true to make syn_port_random_fill fail, triggering fallback. */
extern bool mock_random_skip;

/* ── Ethernet (PHY tx/rx) ────────────────────────────────────────────────── */

#define MOCK_ETH_TX_BUF_SIZE 2048

extern uint8_t mock_eth_tx_buf[MOCK_ETH_TX_BUF_SIZE]; /**< Last frame passed to syn_port_eth_tx */
extern size_t mock_eth_tx_len;                        /**< Length of last transmitted frame     */
extern int mock_eth_tx_count;                         /**< Total calls to syn_port_eth_tx       */

/* ── UDP mock ───────────────────────────────────────────────────────────── */

#define MOCK_UDP_MAX_PACKETS 4
#define MOCK_UDP_BUF_SIZE 2048

typedef struct {
    uint8_t data[MOCK_UDP_BUF_SIZE];
    size_t len;
    SYN_SockAddr from;
} MockUdpPacket;

extern MockUdpPacket mock_udp_rx_queue[MOCK_UDP_MAX_PACKETS];
extern int mock_udp_rx_count;
extern int mock_udp_rx_pos;

/** Compatibility wrapper for old tests. */
void mock_udp_set_response(const void *data, size_t len, const SYN_SockAddr *from);

/** Add a packet to the mock RX queue for the next recvfrom() call. */
void mock_udp_inject_packet(const void *data, size_t len, const SYN_SockAddr *from);

/* ── USB Device & Host mocks ─────────────────────────────────────────────── */

extern uint32_t mock_usb_ep_stall_count;
extern uint8_t mock_usb_last_stalled_ep;

#include "port/syn_port_usb_host.h"

extern bool mock_usb_host_attached;
extern bool mock_usb_host_vbus_enabled;
extern uint8_t mock_usb_host_xfer_buf[256];
extern uint16_t mock_usb_host_xfer_len;
extern bool mock_usb_host_xfer_complete;
extern SYN_Status mock_usb_host_xfer_status;

/* Reset USB Host mock state */
void mock_usb_host_reset(void);

/* ── Analog Comparator mock ──────────────────────────────────────────────── */

extern bool mock_comp_output_state;

/* ── AES / Crypto Accelerator mock ───────────────────────────────────────── */

extern bool mock_aes_hw_enabled;
extern uint32_t mock_aes_encrypt_calls;
extern uint32_t mock_aes_decrypt_calls;
extern uint32_t mock_ghash_calls;

/* ── Reset ────────────────────────────────────────────────────────────────── */

/** Reset all mock state to defaults. Call from setUp(). */
void mock_port_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_PORT_H */
