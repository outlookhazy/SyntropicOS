

# Group syn\_system



[**Modules**](modules.md) **>** [**syn\_system**](group__syn__system.md)



_Bootloader integration, power management, watchdog, faults, and system templates._ 








## Files

| Type | Name |
| ---: | :--- |
| file | [**syn\_boot.h**](syn__boot_8h.md) <br>_Boot manager — reset reason, boot counter, safe-mode detection._  |
| file | [**syn\_coredump.h**](syn__coredump_8h.md) <br>_Persistent core dump — saves fault context to flash._  |
| file | [**syn\_fault.h**](syn__fault_8h.md) <br>_CPU Hard Fault / Post-Mortem Diagnostic log collector._  |
| file | [**syn\_fwboot.h**](syn__fwboot_8h.md) <br>_A/B firmware boot manager — slot selection, rollback, confirm._  |
| file | [**syn\_fwimage.h**](syn__fwimage_8h.md) <br>_Firmware image header — standardized format for OTA slots._  |
| file | [**syn\_fwupdate.h**](syn__fwupdate_8h.md) <br>_Streaming firmware updater — transport-agnostic, zero-alloc._  |
| file | [**syn\_port\_adc.h**](syn__port__adc_8h.md) <br>_ADC port interface — implement these for your platform._  |
| file | [**syn\_port\_can.h**](syn__port__can_8h.md) <br>_CAN bus port interface._  |
| file | [**syn\_port\_dac.h**](syn__port__dac_8h.md) <br>_Platform port: Digital-to-Analog Converter (DAC)._  |
| file | [**syn\_port\_exti.h**](syn__port__exti_8h.md) <br>_GPIO interrupt (EXTI) port interface — implement for your platform._  |
| file | [**syn\_port\_flash.h**](syn__port__flash_8h.md) <br>_Flash storage port interface — implement these for your platform._  |
| file | [**syn\_port\_gpio.h**](syn__port__gpio_8h.md) <br>_GPIO port interface — functions the user must implement._  |
| file | [**syn\_port\_i2c.h**](syn__port__i2c_8h.md) <br>_I2C port interface — implement these for your platform._  |
| file | [**syn\_port\_i2c\_async.h**](syn__port__i2c__async_8h.md) <br>_Async I2C port interface — implement these for your platform._  |
| file | [**syn\_port\_pwm.h**](syn__port__pwm_8h.md) <br>_Hardware PWM port interface — implement these for your platform._  |
| file | [**syn\_port\_rtc.h**](syn__port__rtc_8h.md) <br>_Platform port: Real-Time Clock (RTC)._  |
| file | [**syn\_port\_serial.h**](syn__port__serial_8h.md) <br>_Console serial port interface._  |
| file | [**syn\_port\_socket.h**](syn__port__socket_8h.md) <br>_TCP socket port interface — implement these for your platform._  |
| file | [**syn\_port\_spi.h**](syn__port__spi_8h.md) <br>_SPI port interface — implement these for your platform._  |
| file | [**syn\_port\_spi\_async.h**](syn__port__spi__async_8h.md) <br>_Async SPI port interface — implement these for your platform._  |
| file | [**syn\_port\_spinlock.h**](syn__port__spinlock_8h.md) <br>_Multicore spinlock and core-identity port functions._  |
| file | [**syn\_port\_system.h**](syn__port__system_8h.md) <br>_System-level port interface — functions the user must implement._  |
| file | [**syn\_port\_uart.h**](syn__port__uart_8h.md) <br>_UART port interface — functions the user must implement._  |
| file | [**syn\_port\_wdt.h**](syn__port__wdt_8h.md) <br>_Platform port: Hardware Watchdog Timer (WDT)._  |
| file | [**syn\_power.h**](syn__power_8h.md) <br>_Power / voltage monitor._  |
| file | [**syn\_sleep.h**](syn__sleep_8h.md) <br>_Sleep coordinator — low-power mode management._  |
| file | [**syn\_version.h**](syn__version_8h.md) <br>_Build version and metadata — compile-time constants._  |



























































------------------------------


