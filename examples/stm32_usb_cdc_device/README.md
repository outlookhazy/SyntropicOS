# STM32 USB 2.0 Device CDC ACM (Virtual COM Port) Example

This example demonstrates how to integrate SyntropicOS USB Device Core (`syn_usb.h`) and CDC ACM Class Driver (`syn_usb_cdc.h`) with STM32 HAL USB Device peripherals (`HAL_PCD_...`).

## Features
- **Zero dynamic memory allocation (`malloc`)**.
- **Full-Speed (12 Mbps)** USB 2.0 CDC ACM Virtual COM port device.
- **Protothread integration**: non-blocking `PT_USB_CDC_WAIT_RX()` and `PT_USB_CDC_WAIT_TX_READY()` coroutines.
- **Transport Abstraction**: bound directly to `SYN_Transport` interface for framed packet routing.
