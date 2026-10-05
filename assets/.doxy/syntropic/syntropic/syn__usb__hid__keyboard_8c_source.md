

# File syn\_usb\_hid\_keyboard.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_hid\_keyboard.c**](syn__usb__hid__keyboard_8c.md)

[Go to the documentation of this file](syn__usb__hid__keyboard_8c.md)


```C++

#include "syntropic/drivers/syn_usb_hid_keyboard.h"

#include <string.h>

const uint8_t SYN_USB_HID_KEYBOARD_REPORT_DESC[63] = {
    0x05, 0x01, /* Usage Page (Generic Desktop Ctrls) */
    0x09, 0x06, /* Usage (Keyboard) */
    0xA1, 0x01, /* Collection (Application) */
    0x05, 0x07, /*   Usage Page (Kbrd/Keypad) */
    0x19, 0xE0, /*   Usage Minimum (0xE0 - LeftControl) */
    0x29, 0xE7, /*   Usage Maximum (0xE7 - Right GUI) */
    0x15, 0x00, /*   Logical Minimum (0) */
    0x25, 0x01, /*   Logical Maximum (1) */
    0x75, 0x01, /*   Report Size (1 bit) */
    0x95, 0x08, /*   Report Count (8 bits for modifier keys) */
    0x81, 0x02, /*   Input (Data,Var,Abs) */
    0x95, 0x01, /*   Report Count (1 byte for reserved padding) */
    0x75, 0x08, /*   Report Size (8 bits) */
    0x81, 0x03, /*   Input (Cnst,Var,Abs) */
    0x95, 0x05, /*   Report Count (5 LED status indicator bits) */
    0x75, 0x01, /*   Report Size (1 bit) */
    0x05, 0x08, /*   Usage Page (LEDs) */
    0x19, 0x01, /*   Usage Minimum (Num Lock) */
    0x29, 0x05, /*   Usage Maximum (Kana) */
    0x91, 0x02, /*   Output (Data,Var,Abs) */
    0x95, 0x01, /*   Report Count (1 padding byte for LED alignment) */
    0x75, 0x03, /*   Report Size (3 bits) */
    0x91, 0x03, /*   Output (Cnst,Var,Abs) */
    0x95, 0x06, /*   Report Count (6 simultaneous keycodes) */
    0x75, 0x08, /*   Report Size (8 bits) */
    0x15, 0x00, /*   Logical Minimum (0) */
    0x25, 0x65, /*   Logical Maximum (101 keys) */
    0x05, 0x07, /*   Usage Page (Kbrd/Keypad) */
    0x19, 0x00, /*   Usage Minimum (0) */
    0x29, 0x65, /*   Usage Maximum (101) */
    0x81, 0x00, /*   Input (Data,Array,Abs) */
    0xC0        /* End Collection */
};

SYN_Status syn_usb_hid_keyboard_send(SYN_USB_HID *hid, uint8_t modifier, const uint8_t keycodes[6])
{
    if (!hid) {
        return SYN_INVALID_PARAM;
    }

    SYN_USB_HID_KeyboardReport report;
    memset(&report, 0, sizeof(report));
    report.modifier = modifier;

    if (keycodes) {
        memcpy(report.keycode, keycodes, sizeof(report.keycode));
    }

    return syn_usb_hid_send_report(hid, &report, sizeof(report));
}

SYN_Status syn_usb_hid_keyboard_press(SYN_USB_HID *hid, uint8_t modifier, uint8_t keycode)
{
    uint8_t keycodes[6] = {keycode, 0, 0, 0, 0, 0};
    return syn_usb_hid_keyboard_send(hid, modifier, keycodes);
}

SYN_Status syn_usb_hid_keyboard_release(SYN_USB_HID *hid)
{
    return syn_usb_hid_keyboard_send(hid, 0x00, NULL);
}
```


