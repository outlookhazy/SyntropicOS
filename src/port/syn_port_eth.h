/**
 * @file syn_port_eth.h
 * @brief Ethernet HAL hardware port interface contract.
 */

#ifndef SYN_PORT_ETH_H
#define SYN_PORT_ETH_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize low-level Ethernet MAC hardware (RMII/MII/SPI PHY).
 *
 * @param mac_addr 6-byte hardware MAC address.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_eth_init(const uint8_t mac_addr[6]);

/**
 * @brief Transmit raw Ethernet frame over PHY.
 *
 * @param frame Pointer to frame bytes.
 * @param len   Frame length (60..1514 bytes).
 * @return SYN_OK on success.
 */
SYN_Status syn_port_eth_tx(const void *frame, size_t len);

/**
 * @brief Receive raw Ethernet frame from PHY buffer.
 *
 * @param buf     Pointer to destination buffer.
 * @param max_len Max capacity (must be >= 1514 bytes).
 * @param out_len Pointer to receive frame byte length.
 * @return SYN_OK on success, SYN_BUSY if no frame available.
 */
SYN_Status syn_port_eth_rx(void *buf, size_t max_len, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_ETH_H */
