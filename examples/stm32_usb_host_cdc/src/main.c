/**
 * @file main.c
 * @brief SyntropicOS USB 2.0 Host CDC ACM STM32 HAL Integration Example.
 *
 * Demonstrates non-blocking USB 2.0 Host core enumeration and CDC ACM serial host
 * driver integrated with STM32 HAL HCD (Host Controller Driver).
 */

#include "syntropic/drivers/syn_transport_usb_host_cdc.h"
#include "syntropic/drivers/syn_usb_host.h"
#include "syntropic/drivers/syn_usb_host_cdc.h"
#include "syntropic/syntropic.h"

#include "stm32f4xx_hal.h" /* Or stm32f1xx_hal.h / stm32g4xx_hal.h depending on target MCU */

extern HCD_HandleTypeDef hhcd_USB_OTG_FS;

/* Static USB Host instance and CDC ACM Host class driver (zero heap allocation) */
static SYN_USB_Host usb_host;
static SYN_USB_HostCDC usb_host_cdc;
static SYN_Transport host_transport;

/**
 * @brief Initialize USB Host Core & Host CDC ACM Class Driver.
 */
void usb_host_app_init(void)
{
    /* Initialize USB Host Core */
    syn_usb_host_init(&usb_host);

    /* Initialize Host CDC ACM Driver */
    syn_usb_host_cdc_init(&usb_host_cdc);

    /* Register Host CDC driver with USB Host Core */
    syn_usb_host_cdc_register(&usb_host, &usb_host_cdc);

    /* Bind Host CDC driver to abstract SYN_Transport interface */
    syn_transport_from_usb_host_cdc(&host_transport, &usb_host_cdc);

    /* Start STM32 HAL HCD peripheral */
    HAL_HCD_Start(&hhcd_USB_OTG_FS);
}

/**
 * @brief STM32 HAL HCD Connect Callback (Device attached to host port).
 */
void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd)
{
    if (hhcd->Instance == USB_OTG_FS) {
        /* Device attached, driven on next syn_usb_host_process tick */
    }
}

/**
 * @brief STM32 HAL HCD Disconnect Callback (Device detached from host port).
 */
void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd)
{
    if (hhcd->Instance == USB_OTG_FS) {
        /* Device detached, driven on next syn_usb_host_process tick */
    }
}

/**
 * @brief Coroutine Task: Communicate with downstream USB serial device.
 */
static SYN_PT_Status usb_host_cdc_task(SYN_PT *pt)
{
    PT_BEGIN(pt);

    /* Block until downstream device is attached and fully enumerated */
    PT_USB_HOST_WAIT_READY(pt, &usb_host);

    /* Send AT command / greeting to downstream serial device */
    static const char hello_cmd[] = "AT\r\n";
    PT_USB_HOST_CDC_WAIT_TX_READY(pt, &usb_host_cdc);
    syn_transport_send(&host_transport, (const uint8_t *)hello_cmd, sizeof(hello_cmd) - 1U);

    while (1) {
        /* Wait for response bytes from attached downstream device */
        PT_USB_HOST_CDC_WAIT_RX(pt, &usb_host_cdc);

        char rx_buf[64];
        size_t read_bytes = 0;
        syn_usb_host_cdc_read(&usb_host_cdc, rx_buf, sizeof(rx_buf) - 1, &read_bytes);

        if (read_bytes > 0) {
            rx_buf[read_bytes] = '\0';
            /* Process response from downstream USB device */
        }
    }

    PT_END(pt);
}

int main(void)
{
    HAL_Init();
    usb_host_app_init();

    SYN_PT host_pt;
    PT_INIT(&host_pt);

    while (1) {
        /* Drive USB Host enumeration & transfer state machine */
        syn_usb_host_process(&usb_host);

        /* Run USB Host application coroutine task */
        usb_host_cdc_task(&host_pt);
    }
}
