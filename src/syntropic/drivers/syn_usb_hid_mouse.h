/**
 * @file syn_usb_hid_mouse.h
 * @brief Zero-Heap USB HID Boot Mouse Device Helper Module.
 */

#ifndef SYN_USB_HID_MOUSE_H
#define SYN_USB_HID_MOUSE_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb_hid.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** USB HID Mouse Button Bitmasks */
/** @brief Left mouse button bitmask. */
#define SYN_USB_HID_MOUSE_BTN_LEFT (1U << 0)
/** @brief Right mouse button bitmask. */
#define SYN_USB_HID_MOUSE_BTN_RIGHT (1U << 1)
/** @brief Middle mouse button bitmask. */
#define SYN_USB_HID_MOUSE_BTN_MIDDLE (1U << 2)

/** Standard 4-byte USB HID Boot Mouse IN Report structure */
typedef struct SYN_PACKED {
    uint8_t buttons; /**< Button bitmask (Left=0x01, Right=0x02, Middle=0x04) */
    int8_t x;        /**< Relative X displacement (-127 to +127)               */
    int8_t y;        /**< Relative Y displacement (-127 to +127)               */
    int8_t wheel;    /**< Relative vertical scroll wheel (-127 to +127)        */
} SYN_USB_HID_MouseReport;

/** Standard USB HID Mouse Report Descriptor (54 Bytes) */
extern const uint8_t SYN_USB_HID_MOUSE_REPORT_DESC[54];

/**
 * @brief Send a USB HID Mouse report with relative movement and buttons.
 *
 * @param hid      Pointer to USB HID instance.
 * @param buttons  Button bitmask (Left=0x01, Right=0x02, Middle=0x04).
 * @param x        Relative X axis displacement.
 * @param y        Relative Y axis displacement.
 * @param wheel    Relative vertical scroll wheel displacement.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_hid_mouse_send(SYN_USB_HID *hid, uint8_t buttons, int8_t x, int8_t y,
                                  int8_t wheel);

/**
 * @brief Move the USB HID Mouse pointer relatively.
 *
 * @param hid  Pointer to USB HID instance.
 * @param x    Relative X axis displacement.
 * @param y    Relative Y axis displacement.
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_hid_mouse_move(SYN_USB_HID *hid, int8_t x, int8_t y);

/**
 * @brief Send a USB HID Mouse click (press button + release).
 *
 * @param hid      Pointer to USB HID instance.
 * @param buttons  Button bitmask (e.g. SYN_USB_HID_MOUSE_BTN_LEFT).
 * @return SYN_OK on success.
 */
SYN_Status syn_usb_hid_mouse_click(SYN_USB_HID *hid, uint8_t buttons);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_HID_MOUSE_H */
