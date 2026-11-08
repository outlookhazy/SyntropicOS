/**
 * @file syn_usb_hid_mouse.c
 * @brief Zero-Heap USB HID Boot Mouse Device Helper Module Implementation.
 */

#include "syntropic/drivers/syn_usb_hid_mouse.h"

#include <string.h>

const uint8_t SYN_USB_HID_MOUSE_REPORT_DESC[54] = {
    0x05, 0x01, /* Usage Page (Generic Desktop) */
    0x09, 0x02, /* Usage (Mouse) */
    0xA1, 0x01, /* Collection (Application) */
    0x09, 0x01, /*   Usage (Pointer) */
    0xA1, 0x00, /*   Collection (Physical) */
    0x05, 0x09, /*     Usage Page (Buttons) */
    0x19, 0x01, /*     Usage Minimum (1) */
    0x29, 0x03, /*     Usage Maximum (3) */
    0x15, 0x00, /*     Logical Minimum (0) */
    0x25, 0x01, /*     Logical Maximum (1) */
    0x95, 0x03, /*     Report Count (3 buttons) */
    0x75, 0x01, /*     Report Size (1 bit) */
    0x81, 0x02, /*     Input (Data, Variable, Absolute) */
    0x95, 0x01, /*     Report Count (1 padding byte) */
    0x75, 0x05, /*     Report Size (5 bits) */
    0x81, 0x01, /*     Input (Constant) */
    0x05, 0x01, /*     Usage Page (Generic Desktop) */
    0x09, 0x30, /*     Usage (X) */
    0x09, 0x31, /*     Usage (Y) */
    0x09, 0x38, /*     Usage (Wheel) */
    0x15, 0x81, /*     Logical Minimum (-127) */
    0x25, 0x7F, /*     Logical Maximum (+127) */
    0x75, 0x08, /*     Report Size (8 bits) */
    0x95, 0x03, /*     Report Count (3 fields: X, Y, Wheel) */
    0x81, 0x06, /*     Input (Data, Variable, Relative) */
    0xC0,       /*   End Collection */
    0xC0        /* End Collection */
};

SYN_Status syn_usb_hid_mouse_send(SYN_USB_HID *hid, uint8_t buttons, int8_t x, int8_t y,
                                  int8_t wheel)
{
    if (!hid) {
        return SYN_INVALID_PARAM;
    }

    SYN_USB_HID_MouseReport report;
    report.buttons = buttons;
    report.x = x;
    report.y = y;
    report.wheel = wheel;

    return syn_usb_hid_send_report(hid, &report, sizeof(report));
}

SYN_Status syn_usb_hid_mouse_move(SYN_USB_HID *hid, int8_t x, int8_t y)
{
    return syn_usb_hid_mouse_send(hid, 0x00, x, y, 0);
}

SYN_Status syn_usb_hid_mouse_click(SYN_USB_HID *hid, uint8_t buttons)
{
    SYN_Status st = syn_usb_hid_mouse_send(hid, buttons, 0, 0, 0);
    if ((st == SYN_OK) && (buttons != 0U)) {
        hid->release_pending = true;
    }
    return st;
}
