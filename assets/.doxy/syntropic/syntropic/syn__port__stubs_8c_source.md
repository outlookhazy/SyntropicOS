

# File syn\_port\_stubs.c

[**File List**](files.md) **>** [**port\_stubs**](dir_faf66d44489c906207a31f15a9acbcc2.md) **>** [**syn\_port\_stubs.c**](syn__port__stubs_8c.md)

[Go to the documentation of this file](syn__port__stubs_8c.md)


```C++

#include "../common/syn_compiler.h"
#include "../common/syn_defs.h"
#include "../util/syn_assert.h"

/* ── Assert handler default ─────────────────────────────────────────────── */

SYN_WEAK SYN_NORETURN void syn_assert_failed(const char *file, int line)
{
    (void)file;
    (void)line;
    /* Default: spin forever. Override to log, blink LED, enter debugger. */
    for (;;) {}
}

/* ── GPIO stubs ─────────────────────────────────────────────────────────── */

SYN_WEAK SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    (void)pin; (void)mode;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED; /* unreachable */
}

SYN_WEAK SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin)
{
    (void)pin;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    (void)pin; (void)state;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    (void)pin;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_GPIO_LOW;
}

SYN_WEAK SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    (void)pin;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

/* ── UART stubs ─────────────────────────────────────────────────────────── */

SYN_WEAK SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate)
{
    (void)instance; (void)baudrate;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance)
{
    (void)instance;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance,
                                               const uint8_t *data,
                                               size_t len,
                                               uint32_t timeout_ms)
{
    (void)instance; (void)data; (void)len; (void)timeout_ms;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_uart_receive(SYN_UARTInstance instance,
                                              uint8_t *data,
                                              size_t len,
                                              size_t *received,
                                              uint32_t timeout_ms)
{
    (void)instance; (void)data; (void)len; (void)received; (void)timeout_ms;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance instance, uint8_t byte)
{
    (void)instance; (void)byte;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance instance,
                                                   uint8_t *byte,
                                                   uint32_t timeout_ms)
{
    (void)instance; (void)byte; (void)timeout_ms;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

/* ── System stubs ───────────────────────────────────────────────────────── */

SYN_WEAK void syn_port_enter_critical(void)
{
    syn_assert_failed(__FILE__, __LINE__);
}

SYN_WEAK void syn_port_exit_critical(void)
{
    syn_assert_failed(__FILE__, __LINE__);
}

SYN_WEAK uint32_t syn_port_get_tick_ms(void)
{
    syn_assert_failed(__FILE__, __LINE__);
    return 0;
}

SYN_WEAK void syn_port_delay_ms(uint32_t ms)
{
    (void)ms;
    syn_assert_failed(__FILE__, __LINE__);
}

SYN_WEAK SYN_NORETURN void syn_port_system_reset(void)
{
    syn_assert_failed(__FILE__, __LINE__);
    for (;;) {} /* satisfy noreturn */
}

/* ── SPI stubs ──────────────────────────────────────────────────────────── */

#include "../port/syn_port_spi.h"

SYN_WEAK SYN_Status syn_port_spi_init(const SYN_SPI_Config *cfg)
{
    (void)cfg;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_spi_deinit(uint8_t bus)
{
    (void)bus;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_spi_transfer(uint8_t bus,
                                              const uint8_t *tx_buf,
                                              uint8_t *rx_buf,
                                              size_t len)
{
    (void)bus; (void)tx_buf; (void)rx_buf; (void)len;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_spi_cs_assert(uint8_t bus, SYN_GPIO_Pin cs_pin)
{
    (void)bus; (void)cs_pin;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_spi_cs_deassert(uint8_t bus, SYN_GPIO_Pin cs_pin)
{
    (void)bus; (void)cs_pin;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

/* ── I2C stubs ──────────────────────────────────────────────────────────── */

#include "../port/syn_port_i2c.h"

SYN_WEAK SYN_Status syn_port_i2c_init(const SYN_I2C_Config *cfg)
{
    (void)cfg;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_i2c_deinit(uint8_t bus)
{
    (void)bus;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_i2c_write(uint8_t bus, uint8_t addr,
                                           const uint8_t *data, size_t len)
{
    (void)bus; (void)addr; (void)data; (void)len;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_i2c_read(uint8_t bus, uint8_t addr,
                                          uint8_t *data, size_t len)
{
    (void)bus; (void)addr; (void)data; (void)len;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_i2c_write_read(uint8_t bus, uint8_t addr,
                                                const uint8_t *tx_data, size_t tx_len,
                                                uint8_t *rx_data, size_t rx_len)
{
    (void)bus; (void)addr; (void)tx_data; (void)tx_len; (void)rx_data; (void)rx_len;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

/* ── Flash stubs ────────────────────────────────────────────────────────── */

#include "../port/syn_port_flash.h"

SYN_WEAK SYN_Status syn_port_flash_erase(uint32_t addr)
{
    (void)addr;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len)
{
    (void)addr; (void)buf; (void)len;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len)
{
    (void)addr; (void)buf; (void)len;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK uint32_t syn_port_flash_sector_size(uint32_t addr)
{
    (void)addr;
    syn_assert_failed(__FILE__, __LINE__);
    return 0;
}

/* ── ADC stubs ──────────────────────────────────────────────────────────── */

#include "../port/syn_port_adc.h"

SYN_WEAK SYN_Status syn_port_adc_init(uint8_t channel)
{
    (void)channel;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK uint16_t syn_port_adc_read(uint8_t channel)
{
    (void)channel;
    syn_assert_failed(__FILE__, __LINE__);
    return 0;
}

SYN_WEAK uint8_t syn_port_adc_resolution(void)
{
    syn_assert_failed(__FILE__, __LINE__);
    return 0;
}

SYN_WEAK uint16_t syn_port_adc_reference_mv(void)
{
    syn_assert_failed(__FILE__, __LINE__);
    return 0;
}

/* ── PWM stubs ──────────────────────────────────────────────────────────── */

#include "../port/syn_port_pwm.h"

SYN_WEAK SYN_Status syn_port_pwm_init(uint8_t channel, uint32_t freq_hz)
{
    (void)channel; (void)freq_hz;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK void syn_port_pwm_set_duty(uint8_t channel, uint8_t duty_pct)
{
    (void)channel; (void)duty_pct;
    syn_assert_failed(__FILE__, __LINE__);
}

SYN_WEAK void syn_port_pwm_set_duty_raw(uint8_t channel, uint16_t duty_u16)
{
    (void)channel; (void)duty_u16;
    syn_assert_failed(__FILE__, __LINE__);
}

SYN_WEAK void syn_port_pwm_enable(uint8_t channel, bool enable)
{
    (void)channel; (void)enable;
    syn_assert_failed(__FILE__, __LINE__);
}

SYN_WEAK void syn_port_pwm_set_freq(uint8_t channel, uint32_t freq_hz)
{
    (void)channel; (void)freq_hz;
    syn_assert_failed(__FILE__, __LINE__);
}

/* ── Sleep stub ─────────────────────────────────────────────────────────── */

#include "../system/syn_sleep.h"

SYN_WEAK void syn_port_sleep(SYN_SleepMode mode)
{
    (void)mode;
    /* Default: no-op (busy wait) */
}

SYN_WEAK void syn_port_sleep_until(uint32_t wake_tick_ms)
{
    (void)wake_tick_ms;
    /* Default: fall back to light sleep */
    syn_port_sleep(SYN_SLEEP_LIGHT);
}

/* ── EXTI stubs ─────────────────────────────────────────────────────────── */

#include "../port/syn_port_exti.h"

SYN_WEAK SYN_Status syn_port_exti_configure(SYN_GPIO_Pin pin,
                                                 SYN_EXTI_Edge edge)
{
    (void)pin; (void)edge;
    return SYN_OK;
}

SYN_WEAK void syn_port_exti_enable(SYN_GPIO_Pin pin)
{
    (void)pin;
}

SYN_WEAK void syn_port_exti_disable(SYN_GPIO_Pin pin)
{
    (void)pin;
}

SYN_WEAK void syn_port_exti_clear_pending(SYN_GPIO_Pin pin)
{
    (void)pin;
}

/* ── CAN stubs ──────────────────────────────────────────────────────────── */

#include "../port/syn_port_can.h"

SYN_WEAK bool syn_port_can_init(uint8_t port, uint32_t bitrate)
{
    (void)port; (void)bitrate;
    return true;
}

SYN_WEAK bool syn_port_can_send(uint8_t port, uint32_t id, bool extended,
                                    const uint8_t *data, uint8_t dlc)
{
    (void)port; (void)id; (void)extended; (void)data; (void)dlc;
    return true;
}

SYN_WEAK bool syn_port_can_receive(uint8_t port, uint32_t *id, bool *extended,
                                       uint8_t *data, uint8_t *dlc)
{
    (void)port; (void)id; (void)extended; (void)data; (void)dlc;
    return false;  /* no frame available */
}

SYN_WEAK void syn_port_can_set_filter(uint8_t port, uint32_t id, uint32_t mask)
{
    (void)port; (void)id; (void)mask;
}

/* ── DMA stubs ──────────────────────────────────────────────────────────── */

#include "../port/syn_port_dma.h"

#if defined(SYN_USE_DMA) && SYN_USE_DMA

SYN_WEAK SYN_Status syn_port_dma_init(const SYN_DMA_Config *cfg)
{
    (void)cfg;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_dma_start(uint8_t channel,
                                        const volatile void *src,
                                        volatile void *dst,
                                        size_t count)
{
    (void)channel; (void)src; (void)dst; (void)count;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_dma_stop(uint8_t channel)
{
    (void)channel;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK bool syn_port_dma_busy(uint8_t channel)
{
    (void)channel;
    syn_assert_failed(__FILE__, __LINE__);
    return false;
}

SYN_WEAK size_t syn_port_dma_remaining(uint8_t channel)
{
    (void)channel;
    syn_assert_failed(__FILE__, __LINE__);
    return 0;
}

#endif /* SYN_USE_DMA */

/* ── Async I2C stubs ────────────────────────────────────────────────────── */

#include "../port/syn_port_i2c_async.h"

#if defined(SYN_USE_I2C_ASYNC) && SYN_USE_I2C_ASYNC

SYN_WEAK SYN_Status syn_port_i2c_xfer_async(const SYN_I2C_Xfer *xfer)
{
    (void)xfer;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_i2c_cancel(uint8_t bus)
{
    (void)bus;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

#endif /* SYN_USE_I2C_ASYNC */

/* ── Async SPI stubs ────────────────────────────────────────────────────── */

#include "../port/syn_port_spi_async.h"

#if defined(SYN_USE_SPI_ASYNC) && SYN_USE_SPI_ASYNC

SYN_WEAK SYN_Status syn_port_spi_xfer_async(const SYN_SPI_Xfer *xfer)
{
    (void)xfer;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK SYN_Status syn_port_spi_cancel(uint8_t bus)
{
    (void)bus;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

#endif /* SYN_USE_SPI_ASYNC */

/* ── Multicore stubs ───────────────────────────────────────────────────── */

#include "../common/syn_barrier.h"

#if defined(SYN_USE_MULTICORE) && SYN_USE_MULTICORE

#include "../port/syn_port_spinlock.h"

SYN_WEAK void syn_port_memory_barrier(void)
{
    /* Fallback: compiler barrier only. Safe on single-core, but a real
     * multicore port MUST override this with a hardware fence. */
    SYN_COMPILER_BARRIER();
}

SYN_WEAK void syn_port_spinlock_acquire(uint8_t id)
{
    (void)id;
    syn_assert_failed(__FILE__, __LINE__);
}

SYN_WEAK void syn_port_spinlock_release(uint8_t id)
{
    (void)id;
    syn_assert_failed(__FILE__, __LINE__);
}

SYN_WEAK bool syn_port_spinlock_try_acquire(uint8_t id)
{
    (void)id;
    syn_assert_failed(__FILE__, __LINE__);
    return false;
}

SYN_WEAK uint8_t syn_port_core_id(void)
{
    return 0;
}

SYN_WEAK void syn_port_ipc_notify(void)
{
    /* No-op: consumer core polls at next scheduler tick. */
}

#else /* !SYN_USE_MULTICORE */

/* Memory barrier stub when multicore is off — never actually called
 * because syn_barrier.h inlines plain volatile access, but some
 * toolchains may still emit a reference. */
SYN_WEAK void syn_port_memory_barrier(void)
{
    SYN_COMPILER_BARRIER();
}
#endif /* SYN_USE_MULTICORE */

/* ── Socket stubs ───────────────────────────────────────────────────────── */

#include "../port/syn_port_socket.h"

SYN_WEAK SYN_Socket syn_port_udp_open(uint16_t port)
{
    (void)port;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_SOCKET_INVALID;
}

SYN_WEAK int syn_port_udp_sendto(SYN_Socket sock, const void *data, size_t len,
                                  const SYN_SockAddr *to)
{
    (void)sock; (void)data; (void)len; (void)to;
    syn_assert_failed(__FILE__, __LINE__);
    return -1;
}

SYN_WEAK int syn_port_udp_recvfrom(SYN_Socket sock, void *buf, size_t max_len,
                                    SYN_SockAddr *from, uint32_t timeout_ms)
{
    (void)sock; (void)buf; (void)max_len; (void)from; (void)timeout_ms;
    syn_assert_failed(__FILE__, __LINE__);
    return -1;
}

SYN_WEAK SYN_Status syn_port_udp_join_multicast(SYN_Socket sock, const char *multicast_ip)
{
    (void)sock; (void)multicast_ip;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK void syn_port_sock_close(SYN_Socket sock)
{
    (void)sock;
    syn_assert_failed(__FILE__, __LINE__);
}

SYN_WEAK SYN_Socket syn_port_sock_connect(const SYN_SockAddr *addr)
{
    (void)addr;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_SOCKET_INVALID;
}

SYN_WEAK SYN_Socket syn_port_sock_connect_host(const char *host, uint16_t port)
{
    (void)host; (void)port;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_SOCKET_INVALID;
}

SYN_WEAK int syn_port_sock_send(SYN_Socket sock, const void *data, size_t len)
{
    (void)sock; (void)data; (void)len;
    syn_assert_failed(__FILE__, __LINE__);
    return -1;
}

SYN_WEAK int syn_port_sock_send_all(SYN_Socket sock, const void *data, size_t len)
{
    (void)sock; (void)data; (void)len;
    syn_assert_failed(__FILE__, __LINE__);
    return -1;
}

SYN_WEAK int syn_port_sock_recv(SYN_Socket sock, void *buf, size_t max_len,
                                 uint32_t timeout_ms)
{
    (void)sock; (void)buf; (void)max_len; (void)timeout_ms;
    syn_assert_failed(__FILE__, __LINE__);
    return -1;
}

SYN_WEAK SYN_Socket syn_port_sock_listen(uint16_t port, int backlog)
{
    (void)port; (void)backlog;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_SOCKET_INVALID;
}

SYN_WEAK SYN_Socket syn_port_sock_accept(SYN_Socket listener, uint32_t timeout_ms)
{
    (void)listener; (void)timeout_ms;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_SOCKET_INVALID;
}

/* ── Console serial stubs ───────────────────────────────────────────────── */

#include "../port/syn_port_serial.h"

SYN_WEAK SYN_Status syn_port_serial_init(uint32_t baudrate)
{
    (void)baudrate;
    syn_assert_failed(__FILE__, __LINE__);
    return SYN_NOT_IMPLEMENTED;
}

SYN_WEAK int syn_port_serial_write(const uint8_t *data, size_t len)
{
    (void)data; (void)len;
    syn_assert_failed(__FILE__, __LINE__);
    return -1;
}

SYN_WEAK int syn_port_serial_read(uint8_t *buf, size_t max_len)
{
    (void)buf; (void)max_len;
    syn_assert_failed(__FILE__, __LINE__);
    return -1;
}
```


