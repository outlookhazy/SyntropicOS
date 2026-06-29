/**
 * @file mock_transport.h
 * @brief Shared mock transport for router/heartbeat tests.
 */

#ifndef MOCK_TRANSPORT_H
#define MOCK_TRANSPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static uint8_t rt_tx_buf[128];
static size_t  rt_tx_len = 0;
static uint8_t rt_rx_buf[128];
static size_t  rt_rx_len = 0;
static bool    rt_rx_rdy = false;

static bool rt_send(const uint8_t *d, size_t l, void *c)
{
    (void)c;
    memcpy(rt_tx_buf, d, l);
    rt_tx_len = l;
    return true;
}

static bool rt_recv(uint8_t *d, size_t mx, size_t *ol, void *c)
{
    (void)c;
    if (!rt_rx_rdy) return false;
    if (rt_rx_len > mx) return false;
    memcpy(d, rt_rx_buf, rt_rx_len);
    *ol = rt_rx_len;
    rt_rx_rdy = false;
    return true;
}

static inline void mock_transport_reset(void)
{
    rt_tx_len = 0;
    rt_rx_len = 0;
    rt_rx_rdy = false;
    memset(rt_tx_buf, 0, sizeof(rt_tx_buf));
    memset(rt_rx_buf, 0, sizeof(rt_rx_buf));
}

#endif /* MOCK_TRANSPORT_H */
