/**
 * @file port_esp32.c
 * @brief SyntropicOS port layer for ESP32 (ESP-IDF).
 *
 * Implements syn_port_* interfaces using ESP-IDF APIs.
 */

#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_flash.h"
#include "syntropic/port/syn_port_socket.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_flash.h"

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

/* ── Assert ─────────────────────────────────────────────────────────────── */

#include "syntropic/util/syn_assert.h"
#include "esp_log.h"

SYN_NORETURN void syn_assert_failed(const char *file, int line)
{
    ESP_LOGE("SYN_ASSERT", "Assertion failed at %s:%d", file, line);
    abort();
}

/* ── System ─────────────────────────────────────────────────────────────── */

uint32_t syn_port_get_tick_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

void syn_port_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void syn_port_delay_us(uint32_t us)
{
    esp_rom_delay_us(us);
}

void syn_port_reset(void)
{
    esp_restart();
}

void syn_port_sleep(void)
{
    vTaskDelay(pdMS_TO_TICKS(100));
}

/* ── GPIO ───────────────────────────────────────────────────────────────── */

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << pin),
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };

    if (mode == SYN_GPIO_OUTPUT) {
        cfg.mode = GPIO_MODE_OUTPUT;
    } else {
        cfg.mode = GPIO_MODE_INPUT;
        cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    }

    return (gpio_config(&cfg) == ESP_OK) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    return (gpio_set_level(pin, state) == ESP_OK) ? SYN_OK : SYN_ERROR;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    return gpio_get_level(pin) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
}

/* ── Flash ──────────────────────────────────────────────────────────────── */

/* Uses the esp_flash_* API (ESP-IDF v5.x).
 * NULL chip handle = default main SPI flash. */

SYN_Status syn_port_flash_erase(uint32_t addr)
{
    esp_err_t err = esp_flash_erase_region(NULL, addr, 0x1000);
    return (err == ESP_OK) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len)
{
    esp_err_t err = esp_flash_read(NULL, buf, addr, len);
    return (err == ESP_OK) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len)
{
    esp_err_t err = esp_flash_write(NULL, buf, addr, len);
    return (err == ESP_OK) ? SYN_OK : SYN_ERROR;
}

uint32_t syn_port_flash_sector_size(uint32_t addr)
{
    (void)addr;
    return 0x1000; /* 4KB */
}

/* ── Socket ─────────────────────────────────────────────────────────────── */

SYN_Socket syn_port_sock_connect(const SYN_SockAddr *addr)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) return SYN_SOCKET_INVALID;

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(addr->port);
    memcpy(&dest.sin_addr.s_addr, addr->ip, 4);

    if (connect(sock, (struct sockaddr *)&dest, sizeof(dest)) != 0) {
        close(sock);
        return SYN_SOCKET_INVALID;
    }

    return sock;
}

SYN_Socket syn_port_sock_connect_host(const char *host, uint16_t port)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%u", port);

    if (getaddrinfo(host, port_str, &hints, &res) != 0 || res == NULL) {
        return SYN_SOCKET_INVALID;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        freeaddrinfo(res);
        return SYN_SOCKET_INVALID;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
        close(sock);
        freeaddrinfo(res);
        return SYN_SOCKET_INVALID;
    }

    freeaddrinfo(res);
    return sock;
}

int syn_port_sock_send(SYN_Socket sock, const void *data, size_t len)
{
    return (int)send(sock, data, len, 0);
}

int syn_port_sock_send_all(SYN_Socket sock, const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    size_t remaining = len;
    while (remaining > 0) {
        int n = (int)send(sock, p, remaining, 0);
        if (n <= 0) return -1;
        p += n;
        remaining -= (size_t)n;
    }
    return (int)len;
}

int syn_port_sock_recv(SYN_Socket sock, void *buf, size_t max_len,
                       uint32_t timeout_ms)
{
    struct timeval tv;
    tv.tv_sec  = (long)(timeout_ms / 1000);
    tv.tv_usec = (long)((timeout_ms % 1000) * 1000);
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    int n = (int)recv(sock, buf, max_len, 0);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return -1; /* timeout */
        return -1;
    }
    return n;
}

void syn_port_sock_close(SYN_Socket sock)
{
    close(sock);
}

SYN_Socket syn_port_udp_open(uint16_t port)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) return SYN_SOCKET_INVALID;

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(sock);
        return SYN_SOCKET_INVALID;
    }

    return sock;
}

int syn_port_udp_sendto(SYN_Socket sock, const void *data, size_t len,
                        const SYN_SockAddr *to)
{
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(to->port);
    memcpy(&dest.sin_addr.s_addr, to->ip, 4);

    return (int)sendto(sock, data, len, 0, (struct sockaddr *)&dest, sizeof(dest));
}

int syn_port_udp_recvfrom(SYN_Socket sock, void *buf, size_t max_len,
                          SYN_SockAddr *from, uint32_t timeout_ms)
{
    struct timeval tv;
    tv.tv_sec  = (long)(timeout_ms / 1000);
    tv.tv_usec = (long)((timeout_ms % 1000) * 1000);
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in src;
    socklen_t src_len = sizeof(src);
    int n = (int)recvfrom(sock, buf, max_len, 0, (struct sockaddr *)&src, &src_len);
    if (n < 0) {
        return -1;
    }

    if (from != NULL) {
        from->port = ntohs(src.sin_port);
        memcpy(from->ip, &src.sin_addr.s_addr, 4);
    }
    return n;
}

SYN_Status syn_port_udp_join_multicast(SYN_Socket sock, const char *multicast_ip)
{
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    int err = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    return (err == 0) ? SYN_OK : SYN_ERROR;
}


SYN_Socket syn_port_sock_listen(uint16_t port, int backlog)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) return SYN_SOCKET_INVALID;

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(sock);
        return SYN_SOCKET_INVALID;
    }

    if (listen(sock, backlog) != 0) {
        close(sock);
        return SYN_SOCKET_INVALID;
    }

    return sock;
}

SYN_Socket syn_port_sock_accept(SYN_Socket listener, uint32_t timeout_ms)
{
    /*
     * Use select() for portable accept timeout — lwIP's SO_RCVTIMEO
     * on ESP-IDF takes milliseconds as an int, not struct timeval,
     * which makes setsockopt unreliable for this purpose.
     */
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listener, &readfds);

    struct timeval tv;
    tv.tv_sec  = (long)(timeout_ms / 1000);
    tv.tv_usec = (long)((timeout_ms % 1000) * 1000);

    int ret = select(listener + 1, &readfds, NULL, NULL, &tv);
    if (ret <= 0) {
        return SYN_SOCKET_INVALID;  /* timeout or error */
    }

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client = accept(listener, (struct sockaddr *)&client_addr, &addr_len);

    return (client >= 0) ? client : SYN_SOCKET_INVALID;
}


/* ── UART (for CLI) — ESP-IDF uses stdout by default ───────────────────── */

SYN_Status syn_port_uart_init(uint8_t port, uint32_t baud)
{
    (void)port; (void)baud;
    /* ESP-IDF configures UART0 for console by default */
    return SYN_OK;
}

SYN_Status syn_port_uart_send(uint8_t port, const uint8_t *data, size_t len)
{
    (void)port;
    for (size_t i = 0; i < len; i++) {
        putchar(data[i]);
    }
    fflush(stdout);
    return SYN_OK;
}
