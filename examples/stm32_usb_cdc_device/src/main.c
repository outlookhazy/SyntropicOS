/**
 * @file main.c
 * @brief SyntropicOS USB 2.0 CDC ACM Device STM32 HAL Integration Example.
 *
 * Demonstrates non-blocking USB CDC ACM Virtual COM Port device integrated with
 * STM32 HAL PCD (Peripheral Controller Driver) interrupts.
 */

#include "syntropic/drivers/syn_transport_usb_cdc.h"
#include "syntropic/drivers/syn_usb.h"
#include "syntropic/drivers/syn_usb_cdc.h"
#include "syntropic/syntropic.h"

#include "stm32f4xx_hal.h" /* Or stm32f1xx_hal.h / stm32g4xx_hal.h depending on target MCU */

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* Standard 18-byte USB Device Descriptor */
static const uint8_t device_desc[18] = {
    0x12, 0x01, 0x00, 0x02, 0x02, 0x00, 0x00, 0x40, /* bMaxPacketSize0 = 64 */
    0xFE, 0xCA, 0xEF, 0xBE, 0x00, 0x01, 0x01, 0x02, 0x00, 0x01};

/* Static USB Device instance and CDC ACM class driver (zero heap allocation) */
static SYN_USB_Device usb_dev;
static SYN_USB_CDC usb_cdc;
static SYN_Transport usb_transport;

/**
 * @brief Initialize USB Device Core & CDC ACM Class Driver.
 */
void usb_device_app_init(void)
{
    /* Initialize USB Device Core */
    syn_usb_init(&usb_dev, device_desc);

    /* Initialize CDC ACM Class driver */
    syn_usb_cdc_init(&usb_cdc);

    /* Register CDC class with USB Device Core */
    syn_usb_cdc_register(&usb_dev, &usb_cdc);

    /* Bind CDC driver to abstract SYN_Transport interface */
    syn_transport_from_usb_cdc(&usb_transport, &usb_cdc);

    /* Start STM32 HAL USB peripheral */
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

        uint8_t resp[64];
        uint16_t rlen = 0;

        if (syn_usb_process_setup(&usb_dev, &pkt, resp, &rlen) == SYN_OK) {
            if (rlen > 0) {
                HAL_PCD_EP_Transmit(hpcd, 0x00, resp, rlen);
            }
        } else {
            HAL_PCD_EP_SetStall(hpcd, 0x00);
        }
    }
}

/**
 * @brief STM32 HAL USB Data OUT Callback (Host -> Device data received).
 */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    if (hpcd->Instance == USB_OTG_FS && epnum == 0x01) { /* Bulk OUT Endpoint */
        uint32_t len = HAL_PCD_EP_GetRxCount(hpcd, epnum);
        uint8_t rx_buf[64];
        HAL_PCD_EP_ReadPacket(hpcd, rx_buf, len);

        /* Write received bytes into CDC RX buffer */
        syn_usb_cdc_write(&usb_cdc, rx_buf, len);

        /* Re-prepare Bulk OUT EP to accept next packet */
        HAL_PCD_EP_Receive(hpcd, 0x01, rx_buf, sizeof(rx_buf));
    }
}

/**
 * @brief Coroutine Task: Echo received bytes back over USB CDC Virtual COM port.
 */
static SYN_PT_Status usb_cdc_echo_task(SYN_PT *pt)
{
    static char echo_buf[64];
    static size_t read_bytes;

    PT_BEGIN(pt);

    /* Wait until USB host connects and configures the device */
    PT_USB_WAIT_CONFIGURED(pt, &usb_dev);

    while (1) {
        /* Block until CDC bytes arrive from host */
        PT_USB_CDC_WAIT_RX(pt, &usb_cdc);

        read_bytes = 0;
        syn_usb_cdc_read(&usb_cdc, echo_buf, sizeof(echo_buf), &read_bytes);

        if (read_bytes > 0) {
            /* Wait until TX buffer is ready and transmit echo */
            PT_USB_CDC_WAIT_TX_READY(pt, &usb_cdc);
            syn_transport_send(&usb_transport, (const uint8_t *)echo_buf, read_bytes);
        }
    }

    PT_END(pt);
}

int main(void)
{
    HAL_Init();
    usb_device_app_init();

    SYN_PT echo_pt;
    PT_INIT(&echo_pt);

    while (1) {
        usb_cdc_echo_task(&echo_pt);
    }
}
