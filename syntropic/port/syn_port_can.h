/**
 * @file syn_port_can.h
 * @brief CAN bus port interface.
 *
 * Implement these functions for your CAN peripheral.
 * @ingroup syn_system
 */

#ifndef SYN_PORT_CAN_H
#define SYN_PORT_CAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize CAN peripheral.
 *
 * @param port     CAN port number (0, 1, ...).
 * @param bitrate  Bitrate in bits/sec (e.g. 500000 for 500kbps).
 * @return true on success.
 */
bool syn_port_can_init(uint8_t port, uint32_t bitrate);

/**
 * @brief Send a CAN frame.
 *
 * @param port     CAN port.
 * @param id       CAN identifier (11-bit or 29-bit).
 * @param extended true for 29-bit ID.
 * @param data     Frame data (up to 8 bytes).
 * @param dlc      Data length code (0-8).
 * @return true if frame was queued for transmission.
 */
bool syn_port_can_send(uint8_t port, uint32_t id, bool extended,
                         const uint8_t *data, uint8_t dlc);

/**
 * @brief Receive a CAN frame (non-blocking).
 *
 * @param port     CAN port.
 * @param id       [out] CAN identifier.
 * @param extended [out] true if 29-bit ID.
 * @param data     [out] Frame data buffer (at least 8 bytes).
 * @param dlc      [out] Data length code.
 * @return true if a frame was received.
 */
bool syn_port_can_receive(uint8_t port, uint32_t *id, bool *extended,
                            uint8_t *data, uint8_t *dlc);

/**
 * @brief Set hardware acceptance filter.
 *
 * @param port   CAN port.
 * @param id     Filter ID.
 * @param mask   Filter mask (1 = must match, 0 = don't care).
 */
void syn_port_can_set_filter(uint8_t port, uint32_t id, uint32_t mask);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_CAN_H */
