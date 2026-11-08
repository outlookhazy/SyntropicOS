/**
 * @file main.c
 * @brief SyntropicOS USB 2.0 HID Keyboard Device STM32 HAL Integration Example.
 *
 * Demonstrates an ergonomic, zero-heap USB HID Boot Keyboard device integrated
 * with STM32 HAL PCD (Peripheral Controller Driver) hardware interrupts.
 */

#include "syntropic/drivers/syn_usb.h"
#include "syntropic/drivers/syn_usb_hid.h"
#include "syntropic/drivers/syn_usb_hid_keyboard.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/syntropic.h"

#include "stm32f4xx_hal.h" /* Or stm32f1xx_hal.h / stm32g4xx_hal.h depending on target MCU */

#include <string.h>

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* Standard 18-byte USB Device Descriptor (HID Device) */
static const uint8_t device_desc[18] = {
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, /* bMaxPacketSize0 = 64 */
    0xFE, 0xCA, 0x01, 0x10, 0x00, 0x01, 0x01, 0x02, 0x00, 0x01
};

/* Static USB Device Core and HID Class context (zero heap allocation) */
static SYN_USB_Device usb_dev;
static SYN_USB_HID    usb_hid;

/**
 * @brief Initialize USB Device Core & USB HID Keyboard Driver.
 */
void usb_hid_app_init(void)
{
    /* Initialize USB Device Core */
    syn_usb_init(&usb_dev, device_desc);

    /* Initialize HID Class driver */
    syn_usb_hid_init(&usb_hid);

    /* Register HID class using SyntropicOS standard keyboard report descriptor */
    syn_usb_hid_register(&usb_dev, &usb_hid,
                         SYN_USB_HID_KEYBOARD_REPORT_DESC,
                         sizeof(SYN_USB_HID_KEYBOARD_REPORT_DESC));

    /* Start STM32 HAL USB Peripheral Controller */
    HAL_PCD_Start(&hpcd_USB_OTG_FS);
}

/**
 * @brief STM32 HAL USB Setup Stage Callback.
 *
 * Invoked by STM32 PCD ISR when a Setup packet arrives on EP0.
 */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
    if (hpcd->Instance == USB_OTG_FS) {
        SYN_USB_SetupPacket pkt;
        memcpy(&pkt, hpcd->Setup, sizeof(pkt));

        uint16_t rlen = 0;

        if (syn_usb_process_setup(&usb_dev, &pkt, usb_dev.ep0_buf,
                                  (uint16_t)sizeof(usb_dev.ep0_buf), &rlen) == SYN_OK) {
            if (rlen > 0) {
                HAL_PCD_EP_Transmit(hpcd, 0x00, usb_dev.ep0_buf, rlen);
            }
        } else {
            HAL_PCD_EP_SetStall(hpcd, 0x00);
        }
    }
}

/**
 * @brief STM32 HAL USB Data Out Stage Callback.
 */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    if (hpcd->Instance == USB_OTG_FS) {
        uint32_t len = HAL_PCD_EP_GetRxCount(hpcd, epnum);
        uint8_t  buf[64];
        HAL_PCD_EP_ReadPacket(hpcd, buf, len);
    }
}

/**
 * @brief STM32 HAL USB Data In Stage Callback.
 */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    (void)hpcd;
    (void)epnum;
}

int main(void)
{
    HAL_Init();
    usb_hid_app_init();

    uint32_t last_keystroke_ms = syn_port_get_tick_ms();

    while (1) {
        uint32_t now_ms = syn_port_get_tick_ms();

        /* Periodically type 'A' (with Left Shift) every 2 seconds when USB TX is ready */
        if (now_ms - last_keystroke_ms >= 2000) {
            last_keystroke_ms = now_ms;

            if (syn_usb_hid_tx_ready(&usb_hid)) {
                /* Type Capital 'A' (Shift + Key A) using high-level SyntropicOS API */
                syn_usb_hid_keyboard_press(&usb_hid, SYN_USB_HID_MOD_LSHIFT, SYN_USB_HID_KEY_A);
            }
        }
    }

    return 0;
}
