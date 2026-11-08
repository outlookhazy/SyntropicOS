# STM32 USB 2.0 Host CDC ACM (Serial Host) Example

This example demonstrates how to integrate SyntropicOS USB Host Core (`syn_usb_host.h`) and Host CDC ACM Driver (`syn_usb_host_cdc.h`) with STM32 HAL USB Host peripherals (`HAL_HCD_...`).

## Features
- **Zero dynamic memory allocation (`malloc`)**.
- **Tick-Driven State Machine**: non-blocking device attach detection, VBUS 5V power control, 10ms bus reset, descriptor reads, `SET_ADDRESS`, `SET_CONFIGURATION`, and CDC interface probing.
- **Protothread Integration**: non-blocking `PT_USB_HOST_WAIT_READY()` and `PT_USB_HOST_CDC_WAIT_RX()` coroutines.
- **Transport Abstraction**: bound directly to `SYN_Transport` interface for non-blocking serial communication with downstream devices.
