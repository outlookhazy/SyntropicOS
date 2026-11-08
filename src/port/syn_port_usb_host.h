/**
 * @file syn_port_usb_host.h
 * @brief USB Host HAL hardware port interface contract.
 *
 * Provides the hardware abstraction for USB Host controllers.
 * Transfer model is non-blocking: submit a transfer, poll for
 * completion, then read the result. Fits cooperative protothreads.
 */

#ifndef SYN_PORT_USB_HOST_H
#define SYN_PORT_USB_HOST_H

#include "syn_port_usb.h"
#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize USB Host controller hardware.
 *
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_host_init(void);

/**
 * @brief Enable or disable VBUS 5V power supply to downstream port.
 *
 * @param enable true to power on, false to power off.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_host_vbus(bool enable);

/**
 * @brief Issue USB bus reset (10ms SE0 signaling).
 *
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_host_bus_reset(void);

/**
 * @brief Check if a downstream USB device is physically attached.
 *
 * @return true if device detected on the port.
 */
bool syn_port_usb_host_device_attached(void);

/**
 * @brief Open a host pipe to a device endpoint.
 *
 * @param pipe     Pipe index (0..SYN_USB_HOST_MAX_PIPES-1).
 * @param dev_addr Target device bus address (0 during initial enum).
 * @param ep_addr  Target endpoint address (bit 7 = direction).
 * @param ep_type  Endpoint type (SYN_USB_EP_TYPE_*).
 * @param max_pkt  Maximum packet size.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_host_pipe_open(uint8_t pipe, uint8_t dev_addr, uint8_t ep_addr,
                                       uint8_t ep_type, uint16_t max_pkt);

/**
 * @brief Close a host pipe.
 *
 * @param pipe Pipe index.
 * @return SYN_OK on success.
 */
SYN_Status syn_port_usb_host_pipe_close(uint8_t pipe);

/**
 * @brief Submit a control setup packet on a pipe (non-blocking).
 *
 * @param pipe Pipe index (must be open as Control type).
 * @param pkt  Pointer to 8-byte USB Setup Packet.
 * @return SYN_OK if submitted.
 */
SYN_Status syn_port_usb_host_submit_setup(uint8_t pipe, const SYN_USB_SetupPacket *pkt);

/**
 * @brief Submit a data stage transfer on a pipe (non-blocking).
 *
 * @param pipe  Pipe index.
 * @param buf   Data buffer (IN: filled on completion, OUT: sent).
 * @param len   Byte length.
 * @param is_in true for IN (device-to-host), false for OUT.
 * @return SYN_OK if submitted.
 */
SYN_Status syn_port_usb_host_submit_data(uint8_t pipe, uint8_t *buf, uint16_t len, bool is_in);

/**
 * @brief Check if a submitted transfer has completed.
 *
 * @param pipe Pipe index.
 * @return true if the last submitted transfer is complete.
 */
bool syn_port_usb_host_xfer_done(uint8_t pipe);

/**
 * @brief Get the result of a completed transfer.
 *
 * @param pipe       Pipe index.
 * @param actual_len [out] Actual bytes transferred.
 * @return SYN_OK on success, SYN_ERROR on transfer error.
 */
SYN_Status syn_port_usb_host_xfer_result(uint8_t pipe, uint16_t *actual_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_USB_HOST_H */
