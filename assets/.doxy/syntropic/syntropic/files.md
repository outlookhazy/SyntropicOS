
# File List

Here is a list of all files with brief descriptions:


* **dir** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md)     
    * **dir** [**port**](dir_fd8b95245ffcce776715f180c056b450.md)     
        * **dir** [**esp32**](dir_5fe4984ab7fe7841faedbf54ff0b3c4c.md)     
            * **file** [**port\_esp32.c**](port__esp32_8c.md) _SyntropicOS port layer for ESP32 (ESP-IDF)._ 
        * **dir** [**posix**](dir_6d923334ab2bb155e44e019dd9da5e9c.md)     
            * **file** [**port\_posix\_socket.c**](port__posix__socket_8c.md) _POSIX socket implementation of the SyntropicOS socket port layer._ 
        * **dir** [**rp2040**](dir_d9ab0060999f493ad92c011306b81fae.md)     
            * **file** [**port\_rp2040.c**](port__rp2040_8c.md) _SyntropicOS port layer for Raspberry Pi RP2040/RP2350 (Pico SDK)._ 
            * **file** [**port\_rp2040\_multicore.c**](port__rp2040__multicore_8c.md) _SyntropicOS AMP multicore port for RP2040/RP2350 (Pico SDK)._ 
        * **dir** [**stm32\_hal**](dir_beb8b7258a6e20be6ee3b2db46ef4785.md)     
            * **file** [**port\_stm32\_hal.c**](port__stm32__hal_8c.md) _SyntropicOS port layer for STM32 microcontrollers using STM32Cube HAL._ 
        * **dir** [**stm32f4**](dir_110bdd47148ae3321ee97f39905b089a.md)     
            * **file** [**port\_stm32f4.c**](port__stm32f4_8c.md) _SyntropicOS port layer for STM32F407 (bare-metal, direct register access)._ 
    * **dir** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md)     
        * **dir** [**cli**](dir_da6bc3c87aefe1086f828540bad8d0dd.md)     
            * **file** [**syn\_cli.c**](syn__cli_8c.md) _Command-line interpreter implementation._     
            * **file** [**syn\_cli.h**](syn__cli_8h.md) _Command-line interpreter for embedded systems._     
        * **dir** [**common**](dir_b4b1f229ee23b400200c904b1998093f.md)     
            * **file** [**syn\_barrier.h**](syn__barrier_8h.md) _Memory ordering primitives for cross-core and ISR safety._     
            * **file** [**syn\_compiler.h**](syn__compiler_8h.md) _Compiler-portable macros for SyntropicOS._     
            * **file** [**syn\_defs.h**](syn__defs_8h.md) _Common type definitions and status codes for SyntropicOS._     
        * **dir** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md)     
            * **file** [**syn\_autotune.c**](syn__autotune_8c.md)     
            * **file** [**syn\_autotune.h**](syn__autotune_8h.md) _Motor controller auto-tuner — feedforward ID + relay PID tuning._     
            * **file** [**syn\_control\_stats.c**](syn__control__stats_8c.md)     
            * **file** [**syn\_control\_stats.h**](syn__control__stats_8h.md) _Performance monitoring and "Tuning Scorecard" for control loops._     
            * **file** [**syn\_pid.c**](syn__pid_8c.md) _PID controller implementation._     
            * **file** [**syn\_pid.h**](syn__pid_8h.md) _General-purpose PID controller — integer arithmetic._     
        * **dir** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md)     
            * **file** [**syn\_blake2s.c**](syn__blake2s_8c.md) _BLAKE2s implementation — RFC 7693._     
            * **file** [**syn\_blake2s.h**](syn__blake2s_8h.md) _BLAKE2s cryptographic hash — RFC 7693, pure C99._     
            * **file** [**syn\_chacha20poly1305.c**](syn__chacha20poly1305_8c.md) _ChaCha20-Poly1305 AEAD — RFC 8439._     
            * **file** [**syn\_chacha20poly1305.h**](syn__chacha20poly1305_8h.md) _ChaCha20-Poly1305 AEAD — RFC 8439, pure C99._     
            * **file** [**syn\_x25519.c**](syn__x25519_8c.md) _X25519 Diffie-Hellman — RFC 7748._     
            * **file** [**syn\_x25519.h**](syn__x25519_8h.md) _X25519 Diffie-Hellman key exchange — RFC 7748, pure C99._     
        * **dir** [**debug**](dir_6f573363694cd388330fe66cf20dcff2.md)     
            * **file** [**syn\_profiler.c**](syn__profiler_8c.md) _Task profiler implementation._     
            * **file** [**syn\_profiler.h**](syn__profiler_8h.md) _Task profiler — CPU time tracking per scheduler task._     
            * **file** [**syn\_task\_profile.c**](syn__task__profile_8c.md) _Opt-In Task Execution & CPU Jitter Profiler implementation._     
            * **file** [**syn\_task\_profile.h**](syn__task__profile_8h.md) _Opt-In Task Execution & CPU Jitter Profiler._     
            * **file** [**syn\_trace.c**](syn__trace_8c.md) _Trace buffer implementation._     
            * **file** [**syn\_trace.h**](syn__trace_8h.md) _Lightweight trace buffer — timestamped event recorder._     
        * **dir** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md)     
            * **file** [**syn\_canvas.c**](syn__canvas_8c.md) _Display canvas implementation._     
            * **file** [**syn\_canvas.h**](syn__canvas_8h.md) _Display canvas — hardware-independent framebuffer + drawing._     
            * **file** [**syn\_gfx.h**](syn__gfx_8h.md) _Compile-time graphics renderer abstraction._     
        * **dir** [**drivers**](dir_5897a976633784d8820096ec35a94718.md)     
            * **file** [**syn\_adc.c**](syn__adc_8c.md) _ADC abstraction implementation._     
            * **file** [**syn\_adc.h**](syn__adc_8h.md) _ADC abstraction — oversampling, calibration, unit conversion._     
            * **file** [**syn\_can.c**](syn__can_8c.md) _CAN bus driver implementation._     
            * **file** [**syn\_can.h**](syn__can_8h.md) _CAN bus driver abstraction._     
            * **file** [**syn\_dac.c**](syn__dac_8c.md) _DAC driver implementation._     
            * **file** [**syn\_dac.h**](syn__dac_8h.md) _DAC (Digital-to-Analog Converter) driver._     
            * **file** [**syn\_dma.c**](syn__dma_8c.md) _Bare-Metal Safe DMA Transaction Engine implementation._     
            * **file** [**syn\_dma.h**](syn__dma_8h.md) _Bare-Metal Safe DMA Transaction Engine._     
            * **file** [**syn\_exti.c**](syn__exti_8c.md) _GPIO interrupt dispatcher implementation._     
            * **file** [**syn\_exti.h**](syn__exti_8h.md) _GPIO interrupt dispatcher — register callbacks per pin._     
            * **file** [**syn\_gpio.c**](syn__gpio_8c.md) _GPIO driver implementation._     
            * **file** [**syn\_gpio.h**](syn__gpio_8h.md) _GPIO driver — high-level convenience functions._     
            * **file** [**syn\_i2c\_dev.h**](syn__i2c__dev_8h.md) _I2C device register helpers — thin layer over port I2C._     
            * **file** [**syn\_rtc.c**](syn__rtc_8c.md) _RTC driver implementation — port delegation + epoch math._     
            * **file** [**syn\_rtc.h**](syn__rtc_8h.md) _RTC (Real-Time Clock) driver._     
            * **file** [**syn\_sd.c**](syn__sd_8c.md) _SD card SPI block driver implementation._     
            * **file** [**syn\_sd.h**](syn__sd_8h.md) _SD card SPI block driver._     
            * **file** [**syn\_soft\_i2c.c**](syn__soft__i2c_8c.md)     
            * **file** [**syn\_soft\_i2c.h**](syn__soft__i2c_8h.md) _Software I2C (bit-banging) driver._     
            * **file** [**syn\_soft\_onewire.c**](syn__soft__onewire_8c.md) _Software bit-bang 1-Wire master implementation._     
            * **file** [**syn\_soft\_onewire.h**](syn__soft__onewire_8h.md) _Software bit-bang 1-Wire master driver._     
            * **file** [**syn\_soft\_spi.c**](syn__soft__spi_8c.md)     
            * **file** [**syn\_soft\_spi.h**](syn__soft__spi_8h.md) _Software SPI (bit-banging) driver._     
            * **file** [**syn\_spi\_dev.h**](syn__spi__dev_8h.md) _SPI device register helpers — thin layer over port SPI._     
            * **file** [**syn\_uart.c**](syn__uart_8c.md) _UART driver implementation._     
            * **file** [**syn\_uart.h**](syn__uart_8h.md) _UART driver — buffered I/O and formatted output._     
        * **dir** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md)     
            * **file** [**syn\_biquad.c**](syn__biquad_8c.md) _Fixed-point Q16.16 Biquad filter implementation._     
            * **file** [**syn\_biquad.h**](syn__biquad_8h.md) _Fixed-point Q16.16 Biquad filter (Direct Form I)._     
            * **file** [**syn\_dds.c**](syn__dds_8c.md) _Direct Digital Synthesis (DDS) Waveform Synthesizer implementation._     
            * **file** [**syn\_dds.h**](syn__dds_8h.md) _Direct Digital Synthesis (DDS) Waveform Synthesizer._     
            * **file** [**syn\_fft.c**](syn__fft_8c.md) _Fixed-point Q16.16 Radix-2 Decimation-in-Time Fast Fourier Transform implementation._     
            * **file** [**syn\_fft.h**](syn__fft_8h.md) _Fixed-point Q16.16 Fast Fourier Transform (FFT)._     
            * **file** [**syn\_filter.c**](syn__filter_8c.md) _Digital filter implementations._     
            * **file** [**syn\_filter.h**](syn__filter_8h.md) _Integer-only digital filters for sensor signal conditioning._     
            * **file** [**syn\_filter\_design.c**](syn__filter__design_8c.md) _Dynamic runtime IIR Biquad filter coefficient generator implementation._     
            * **file** [**syn\_filter\_design.h**](syn__filter__design_8h.md) _Dynamic runtime IIR Biquad filter coefficient generator (Q16.16 fixed-point)._     
            * **file** [**syn\_kalman.c**](syn__kalman_8c.md) _General-purpose fixed-point Kalman filter implementation._     
            * **file** [**syn\_kalman.h**](syn__kalman_8h.md) _General-purpose fixed-point Kalman filter._     
            * **file** [**syn\_signal.c**](syn__signal_8c.md) _Signal statistics implementation._     
            * **file** [**syn\_signal.h**](syn__signal_8h.md) _Signal statistics — sliding window min/max/mean/variance/RMS._     
        * **dir** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md)     
            * **file** [**syn\_button.c**](syn__button_8c.md) _Debounced button implementation using syn\_fsm._     
            * **file** [**syn\_button.h**](syn__button_8h.md) _Debounced button input with press/release/long-press/repeat._     
            * **file** [**syn\_encoder.c**](syn__encoder_8c.md) _Quadrature rotary encoder implementation._     
            * **file** [**syn\_encoder.h**](syn__encoder_8h.md) _Quadrature rotary encoder decoder._     
        * **dir** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md)     
            * **file** [**syn\_datalog.c**](syn__datalog_8c.md)     
            * **file** [**syn\_datalog.h**](syn__datalog_8h.md) _High-speed binary telemetry logger._     
            * **file** [**syn\_log.c**](syn__log_8c.md) _Logging system implementation._     
            * **file** [**syn\_log.h**](syn__log_8h.md) _Severity-filtered logging system._     
        * **dir** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md)     
            * **file** [**syn\_actuator.c**](syn__actuator_8c.md) _Linear actuator implementation._     
            * **file** [**syn\_actuator.h**](syn__actuator_8h.md) _Linear actuator with potentiometer feedback._     
            * **file** [**syn\_dc\_motor.c**](syn__dc__motor_8c.md) _DC motor controller implementation._     
            * **file** [**syn\_dc\_motor.h**](syn__dc__motor_8h.md) _DC motor controller for H-bridge drivers._     
            * **file** [**syn\_foc.c**](syn__foc_8c.md) _Fixed-point Field-Oriented Control transforms._     
            * **file** [**syn\_foc.h**](syn__foc_8h.md) _Fixed-point Field-Oriented Control transforms (Clarke & Park)._     
            * **file** [**syn\_foc\_observer.c**](syn__foc__observer_8c.md) _Sensorless FOC Rotor Position & Speed Sliding Mode Observer implementation._     
            * **file** [**syn\_foc\_observer.h**](syn__foc__observer_8h.md) _Sensorless FOC Rotor Position & Speed Sliding Mode Observer (SMO) (Q16.16 fixed-point)._     
            * **file** [**syn\_interpolator.c**](syn__interpolator_8c.md) _Coordinated multi-axis linear & circular arc trajectory interpolator implementation._     
            * **file** [**syn\_interpolator.h**](syn__interpolator_8h.md) _Coordinated multi-axis linear & circular arc trajectory interpolator._     
            * **file** [**syn\_motor\_ctrl.c**](syn__motor__ctrl_8c.md) _Closed-loop motor controller implementation._     
            * **file** [**syn\_motor\_ctrl.h**](syn__motor__ctrl_8h.md) _Closed-loop motor controller — generic feedback + PID + motor._     
            * **file** [**syn\_motor\_output.h**](syn__motor__output_8h.md) _Abstract motor output interface._     
            * **file** [**syn\_servo.c**](syn__servo_8c.md) _Servo controller implementation._     
            * **file** [**syn\_servo.h**](syn__servo_8h.md) _Hobby servo controller — pulse-width positioning._     
            * **file** [**syn\_stepper.c**](syn__stepper_8c.md) _Stepper motor driver implementation._     
            * **file** [**syn\_stepper.h**](syn__stepper_8h.md) _Stepper motor driver — step/direction with acceleration ramps._     
        * **dir** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md)     
            * **file** [**syn\_coap.c**](syn__coap_8c.md) _CoAP message serialization, parsing, and cooperative client task implementation._     
            * **file** [**syn\_coap.h**](syn__coap_8h.md) _Constrained Application Protocol (CoAP - RFC 7252) implementation._     
            * **file** [**syn\_dns.c**](syn__dns_8c.md) _UDP DNS resolver and mDNS responder implementation._     
            * **file** [**syn\_dns.h**](syn__dns_8h.md) _UDP DNS resolver and mDNS responder._     
            * **file** [**syn\_heartbeat.c**](syn__heartbeat_8c.md) _Heartbeat / keep-alive implementation._     
            * **file** [**syn\_heartbeat.h**](syn__heartbeat_8h.md) _Heartbeat / keep-alive monitor._     
            * **file** [**syn\_http.c**](syn__http_8c.md) _Cooperative HTTP/1.1 client implementation._     
            * **file** [**syn\_http.h**](syn__http_8h.md) _Cooperative HTTP/1.1 client — streaming, zero-alloc, non-blocking._     
            * **file** [**syn\_httpd.c**](syn__httpd_8c.md) _Minimal HTTP/1.1 server — fully non-blocking implementation._     
            * **file** [**syn\_httpd.h**](syn__httpd_8h.md) _Minimal HTTP/1.1 server — route-based, zero-alloc._     
            * **file** [**syn\_mqtt.c**](syn__mqtt_8c.md) _Lightweight MQTT 3.1.1 client implementation — fully non-blocking._     
            * **file** [**syn\_mqtt.h**](syn__mqtt_8h.md) _Lightweight MQTT 3.1.1 client._     
            * **file** [**syn\_router.c**](syn__router_8c.md) _Packet router implementation._     
            * **file** [**syn\_router.h**](syn__router_8h.md) _Packet router — addressed message dispatch._     
            * **file** [**syn\_sntp.c**](syn__sntp_8c.md) _SNTP client implementation — RFC 4330 subset._     
            * **file** [**syn\_sntp.h**](syn__sntp_8h.md) _SNTP client — lightweight time synchronization over UDP._     
            * **file** [**syn\_transport.h**](syn__transport_8h.md) _Transport abstraction — pluggable send/receive (header-only)._     
            * **file** [**syn\_transport\_tcp.c**](syn__transport__tcp_8c.md) _Bridge for syn\_transport over a TCP socket._     
            * **file** [**syn\_transport\_tcp.h**](syn__transport__tcp_8h.md) _Bridge for syn\_transport over a TCP socket._     
            * **file** [**syn\_websocket.c**](syn__websocket_8c.md) _WebSocket protocol implementation._     
            * **file** [**syn\_websocket.h**](syn__websocket_8h.md) _WebSocket protocol support on httpd._     
            * **file** [**syn\_wg.c**](syn__wg_8c.md) _WireGuard client — Noise\_IKpsk2 handshake + transport._     
            * **file** [**syn\_wg.h**](syn__wg_8h.md) _WireGuard VPN client — Noise IK, pure C99, cooperative._     
        * **dir** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md)     
            * **file** [**syn\_led.c**](syn__led_8c.md) _LED controller implementation._     
            * **file** [**syn\_led.h**](syn__led_8h.md) _Non-blocking LED controller — blink, flash, patterns._     
            * **file** [**syn\_soft\_pwm.c**](syn__soft__pwm_8c.md) _Software PWM implementation._     
            * **file** [**syn\_soft\_pwm.h**](syn__soft__pwm_8h.md) _Software PWM on any GPIO pin._     
        * **dir** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md)     
            * **file** [**syn\_port\_adc.h**](syn__port__adc_8h.md) _ADC port interface — implement these for your platform._     
            * **file** [**syn\_port\_can.h**](syn__port__can_8h.md) _CAN bus port interface._     
            * **file** [**syn\_port\_dac.h**](syn__port__dac_8h.md) _Platform port: Digital-to-Analog Converter (DAC)._     
            * **file** [**syn\_port\_dma.h**](syn__port__dma_8h.md) _Hardware port abstraction interface for DMA controller._     
            * **file** [**syn\_port\_exti.h**](syn__port__exti_8h.md) _GPIO interrupt (EXTI) port interface — implement for your platform._     
            * **file** [**syn\_port\_flash.h**](syn__port__flash_8h.md) _Flash storage port interface — implement these for your platform._     
            * **file** [**syn\_port\_gpio.h**](syn__port__gpio_8h.md) _GPIO port interface — functions the user must implement._     
            * **file** [**syn\_port\_i2c.h**](syn__port__i2c_8h.md) _I2C port interface — implement these for your platform._     
            * **file** [**syn\_port\_i2c\_async.h**](syn__port__i2c__async_8h.md) _Async I2C port interface — implement these for your platform._ 
            * **file** [**syn\_port\_pwm.h**](syn__port__pwm_8h.md) _Hardware PWM port interface — implement these for your platform._     
            * **file** [**syn\_port\_rtc.h**](syn__port__rtc_8h.md) _Platform port: Real-Time Clock (RTC)._     
            * **file** [**syn\_port\_serial.h**](syn__port__serial_8h.md) _Console serial port interface._     
            * **file** [**syn\_port\_socket.h**](syn__port__socket_8h.md) _TCP socket port interface — implement these for your platform._     
            * **file** [**syn\_port\_spi.h**](syn__port__spi_8h.md) _SPI port interface — implement these for your platform._     
            * **file** [**syn\_port\_spi\_async.h**](syn__port__spi__async_8h.md) _Async SPI port interface — implement these for your platform._ 
            * **file** [**syn\_port\_spinlock.h**](syn__port__spinlock_8h.md) _Multicore spinlock and core-identity port functions._ 
            * **file** [**syn\_port\_system.h**](syn__port__system_8h.md) _System-level port interface — functions the user must implement._     
            * **file** [**syn\_port\_uart.h**](syn__port__uart_8h.md) _UART port interface — functions the user must implement._     
            * **file** [**syn\_port\_wdt.h**](syn__port__wdt_8h.md) _Platform port: Hardware Watchdog Timer (WDT)._     
        * **dir** [**port\_stubs**](dir_faf66d44489c906207a31f15a9acbcc2.md)     
            * **file** [**syn\_port\_stubs.c**](syn__port__stubs_8c.md) _Weak default implementations for all port functions._     
        * **dir** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md)     
            * **file** [**syn\_canopen.c**](syn__canopen_8c.md) _CANopen DS301 Slave Protocol Engine implementation._     
            * **file** [**syn\_canopen.h**](syn__canopen_8h.md) _CANopen DS301 Slave Protocol Engine._     
            * **file** [**syn\_canopen\_mgr.c**](syn__canopen__mgr_8c.md) _Implementation of CANopen CiA 302 Manager, SDO Client & NMT Master._     
            * **file** [**syn\_canopen\_mgr.h**](syn__canopen__mgr_8h.md) _CANopen CiA 302 Network Manager, SDO Client & NMT Master._     
            * **file** [**syn\_cia303.c**](syn__cia303_8c.md) _CANopen CiA 303-3 Indicator & Status LED implementation._     
            * **file** [**syn\_cia303.h**](syn__cia303_8h.md) _CANopen CiA 303-3 Indicator & Status LED Specification implementation._     
            * **file** [**syn\_cia401.c**](syn__cia401_8c.md) _Implementation of CANopen CiA 401 Generic I/O Device Profile._     
            * **file** [**syn\_cia401.h**](syn__cia401_8h.md) _CANopen CiA 401 Device Profile for Generic I/O Modules._     
            * **file** [**syn\_cia402.c**](syn__cia402_8c.md) _CANopen CiA 402 Servo Drive Profile Engine implementation._     
            * **file** [**syn\_cia402.h**](syn__cia402_8h.md) _CANopen CiA 402 Servo Drive & Motion Control Profile Engine._     
            * **file** [**syn\_cia418.h**](syn__cia418_8h.md) _CANopen CiA 418 Device Profile Object Dictionary Definitions._ 
            * **file** [**syn\_cobs.c**](syn__cobs_8c.md) _COBS packet framing implementation._     
            * **file** [**syn\_cobs.h**](syn__cobs_8h.md) _COBS (Consistent Overhead Byte Stuffing) packet framing._     
            * **file** [**syn\_dali.c**](syn__dali_8c.md) _Digital Addressable Lighting Interface (DALI / IEC 62386) Protocol Stack implementation._     
            * **file** [**syn\_dali.h**](syn__dali_8h.md) _Digital Addressable Lighting Interface (DALI / IEC 62386) Protocol Stack._     
            * **file** [**syn\_dmx512.c**](syn__dmx512_8c.md) _USITT DMX512-A Stage Lighting Protocol implementation._     
            * **file** [**syn\_dmx512.h**](syn__dmx512_8h.md) _USITT DMX512-A Stage & Architectural Lighting Protocol Stack._     
            * **file** [**syn\_isotp.c**](syn__isotp_8c.md) _ISO 15765-2 (ISO-TP) Multi-Frame CAN Transport implementation._     
            * **file** [**syn\_isotp.h**](syn__isotp_8h.md) _ISO 15765-2 (DoCAN / ISO-TP) Multi-Frame CAN Transport Protocol._     
            * **file** [**syn\_j1939.c**](syn__j1939_8c.md) _SAE J1939 Heavy-Duty Vehicle CAN Protocol Stack implementation._     
            * **file** [**syn\_j1939.h**](syn__j1939_8h.md) _SAE J1939 Heavy-Duty Vehicle CAN Protocol Stack._     
            * **file** [**syn\_lin.c**](syn__lin_8c.md) _LIN Protocol Engine implementation._ 
            * **file** [**syn\_lin.h**](syn__lin_8h.md) _LIN (Local Interconnect Network 2.1 / 2.2a & ISO 17987) Protocol Engine._ 
            * **file** [**syn\_lss.c**](syn__lss_8c.md) _CANopen CiA 305 Layer Setting Services (LSS) Implementation._     
            * **file** [**syn\_lss.h**](syn__lss_8h.md) _CANopen CiA 305 Layer Setting Services (LSS) & Protocols._     
            * **file** [**syn\_modbus.c**](syn__modbus_8c.md) _Modbus RTU slave implementation._     
            * **file** [**syn\_modbus.h**](syn__modbus_8h.md) _Modbus RTU slave implementation._     
            * **file** [**syn\_modbus\_master.c**](syn__modbus__master_8c.md) _Non-blocking Modbus RTU Master protocol implementation._     
            * **file** [**syn\_modbus\_master.h**](syn__modbus__master_8h.md) _Non-blocking Modbus RTU Master / Client protocol engine._     
            * **file** [**syn\_modbus\_tcp.c**](syn__modbus__tcp_8c.md) _Modbus TCP ADU (MBAP) framing implementation._     
            * **file** [**syn\_modbus\_tcp.h**](syn__modbus__tcp_8h.md) _Modbus TCP ADU (MBAP) framing layer._     
            * **file** [**syn\_n2k.c**](syn__n2k_8c.md) _NMEA 2000 Marine CAN Protocol Stack implementation._     
            * **file** [**syn\_n2k.h**](syn__n2k_8h.md) _NMEA 2000 Marine CAN Protocol Stack._     
            * **file** [**syn\_nmea.c**](syn__nmea_8c.md) _NMEA 0183 / GNSS sentence parser and encoder implementation._     
            * **file** [**syn\_nmea.h**](syn__nmea_8h.md) _NMEA 0183 / GNSS sentence parser and encoder._     
        * **dir** [**pt**](dir_a1b594c7ca8b98b889036d2814e4f947.md)     
            * **file** [**syn\_pt.h**](syn__pt_8h.md) _Protothreads — stackless cooperative coroutines for C._     
            * **file** [**syn\_pt\_sem.h**](syn__pt__sem_8h.md) _Lightweight counting semaphores for protothreads._     
        * **dir** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md)     
            * **file** [**syn\_ao.c**](syn__ao_8c.md) _Active Object framework implementation._     
            * **file** [**syn\_ao.h**](syn__ao_8h.md) _Active Object (Actor model) framework wrapper._     
            * **file** [**syn\_event\_flags.c**](syn__event__flags_8c.md) _32-bit Event Flag Group Synchronization implementation._     
            * **file** [**syn\_event\_flags.h**](syn__event__flags_8h.md) _32-bit Event Flag Group Synchronization._     
            * **file** [**syn\_mailbox.h**](syn__mailbox_8h.md) _Typed message queue — fixed-size SPSC ring for inter-task IPC._     
            * **file** [**syn\_sched.c**](syn__sched_8c.md) _Cooperative scheduler implementation._     
            * **file** [**syn\_sched.h**](syn__sched_8h.md) _Cooperative scheduler for protothread tasks._     
            * **file** [**syn\_sequencer.c**](syn__sequencer_8c.md) _Timed action sequencer implementation._     
            * **file** [**syn\_sequencer.h**](syn__sequencer_8h.md) _Timed action sequencer — choreograph steps with delays._     
            * **file** [**syn\_task.h**](syn__task_8h.md) _Task descriptor for the cooperative scheduler._     
            * **file** [**syn\_timer.c**](syn__timer_8c.md) _Software timer implementation._     
            * **file** [**syn\_timer.h**](syn__timer_8h.md) _Software timer service — one-shot and periodic timers._     
            * **file** [**syn\_timer\_wheel.c**](syn__timer__wheel_8c.md) _Hashed Timing Wheel Scheduler implementation._     
            * **file** [**syn\_timer\_wheel.h**](syn__timer__wheel_8h.md) _Hashed Timing Wheel Scheduler._     
            * **file** [**syn\_watchdog.c**](syn__watchdog_8c.md) _Task-level watchdog implementation._     
            * **file** [**syn\_watchdog.h**](syn__watchdog_8h.md) _Task-level watchdog monitor._     
            * **file** [**syn\_workqueue.c**](syn__workqueue_8c.md) _Deferred work queue implementation._     
            * **file** [**syn\_workqueue.h**](syn__workqueue_8h.md) _Deferred work queue — ISR-safe function dispatch._     
        * **dir** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md)     
            * **file** [**syn\_sensor.c**](syn__sensor_8c.md) _Sensor polling framework implementation._     
            * **file** [**syn\_sensor.h**](syn__sensor_8h.md) _Sensor polling framework — periodic read → filter → threshold → event._     
            * **file** [**syn\_sensor\_fusion.c**](syn__sensor__fusion_8c.md) _6-DOF IMU Mahony Sensor Fusion & AHRS Filter implementation._     
            * **file** [**syn\_sensor\_fusion.h**](syn__sensor__fusion_8h.md) _6-DOF IMU Sensor Fusion & AHRS Complementary / Mahony Filter (Q16.16 fixed-point)._     
        * **dir** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md)     
            * **file** [**syn\_fat.c**](syn__fat_8c.md)     
            * **file** [**syn\_fat.h**](syn__fat_8h.md) _Custom lightweight FAT16/FAT32 VFS adapter._     
            * **file** [**syn\_lfs.c**](syn__lfs_8c.md) _LittleFS filesystem VFS adapter implementation._     
            * **file** [**syn\_lfs.h**](syn__lfs_8h.md) _LittleFS filesystem VFS adapter._     
            * **file** [**syn\_param.c**](syn__param_8c.md) _Persistent parameter store with wear leveling._     
            * **file** [**syn\_param.h**](syn__param_8h.md) _Persistent parameter store with wear leveling._     
            * **file** [**syn\_settings.c**](syn__settings_8c.md)     
            * **file** [**syn\_settings.h**](syn__settings_8h.md) _Persistent settings manager with change detection._     
            * **file** [**syn\_vfs.c**](syn__vfs_8c.md) _Virtual File System (VFS) abstraction implementation._     
            * **file** [**syn\_vfs.h**](syn__vfs_8h.md) _Virtual File System (VFS) abstraction layer._     
        * **dir** [**system**](dir_e076fcb374a73e741803d1638643d902.md)     
            * **file** [**syn\_boot.c**](syn__boot_8c.md) _Boot manager implementation._     
            * **file** [**syn\_boot.h**](syn__boot_8h.md) _Boot manager — reset reason, boot counter, safe-mode detection._     
            * **file** [**syn\_coredump.c**](syn__coredump_8c.md) _Persistent core dump implementation._     
            * **file** [**syn\_coredump.h**](syn__coredump_8h.md) _Persistent core dump — saves fault context to flash._     
            * **file** [**syn\_errlog.c**](syn__errlog_8c.md) _Persistent error registry implementation._     
            * **file** [**syn\_errlog.h**](syn__errlog_8h.md) _Persistent error registry — survives resets via param store._     
            * **file** [**syn\_fault.c**](syn__fault_8c.md) _CPU Hard Fault / Post-Mortem Diagnostics implementation._     
            * **file** [**syn\_fault.h**](syn__fault_8h.md) _CPU Hard Fault / Post-Mortem Diagnostic log collector._     
            * **file** [**syn\_fwboot.c**](syn__fwboot_8c.md) _A/B firmware boot manager implementation._     
            * **file** [**syn\_fwboot.h**](syn__fwboot_8h.md) _A/B firmware boot manager — slot selection, rollback, confirm._     
            * **file** [**syn\_fwimage.h**](syn__fwimage_8h.md) _Firmware image header — standardized format for OTA slots._     
            * **file** [**syn\_fwupdate.c**](syn__fwupdate_8c.md) _Streaming firmware updater implementation._     
            * **file** [**syn\_fwupdate.h**](syn__fwupdate_8h.md) _Streaming firmware updater — transport-agnostic, zero-alloc._     
            * **file** [**syn\_hwwdt.c**](syn__hwwdt_8c.md) _Hardware Watchdog Timer driver implementation._     
            * **file** [**syn\_hwwdt.h**](syn__hwwdt_8h.md) _Hardware Watchdog Timer driver._     
            * **file** [**syn\_power.c**](syn__power_8c.md) _Power / voltage monitor implementation._     
            * **file** [**syn\_power.h**](syn__power_8h.md) _Power / voltage monitor._     
            * **file** [**syn\_sleep.h**](syn__sleep_8h.md) _Sleep coordinator — low-power mode management._     
            * **file** [**syn\_version.h**](syn__version_8h.md) _Build version and metadata — compile-time constants._     
        * **dir** [**ui**](dir_5167a572f0687ac609ba099b941e0ec0.md)     
            * **file** [**syn\_imgui.c**](syn__imgui_8c.md) _Immediate Mode GUI (IMGUI) implementation._     
            * **file** [**syn\_imgui.h**](syn__imgui_8h.md) _Lightweight, zero-allocation Immediate Mode GUI (IMGUI) for embedded systems._     
            * **file** [**syn\_menu.c**](syn__menu_8c.md) _Menu system implementation._     
            * **file** [**syn\_menu.h**](syn__menu_8h.md) _Tree-structured menu system._     
        * **dir** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md)     
            * **file** [**syn\_assert.h**](syn__assert_8h.md) _Configurable assertion macro for SyntropicOS._     
            * **file** [**syn\_backoff.c**](syn__backoff_8c.md) _Exponential backoff implementation._     
            * **file** [**syn\_backoff.h**](syn__backoff_8h.md) _Exponential backoff helper with jitter (RFC 7252 style)._     
            * **file** [**syn\_bits.h**](syn__bits_8h.md) _Bit manipulation macros._     
            * **file** [**syn\_cbor\_read.c**](syn__cbor__read_8c.md) _CBOR decoder implementation._     
            * **file** [**syn\_cbor\_read.h**](syn__cbor__read_8h.md) _CBOR decoder — zero-alloc, streaming read from a byte buffer._     
            * **file** [**syn\_cbor\_write.c**](syn__cbor__write_8c.md) _CBOR streaming encoder implementation._     
            * **file** [**syn\_cbor\_write.h**](syn__cbor__write_8h.md) _Streaming CBOR encoder — zero-alloc, caller-provided buffer._     
            * **file** [**syn\_change\_filter.h**](syn__change__filter_8h.md) _Dead-band + rate-limited change detector (header-only)._     
            * **file** [**syn\_crc.c**](syn__crc_8c.md) _CRC calculation implementations._     
            * **file** [**syn\_crc.h**](syn__crc_8h.md) _CRC calculation for communication protocols._     
            * **file** [**syn\_event.h**](syn__event_8h.md) _Event flag groups — 32-bit bitmask signaling._     
            * **file** [**syn\_fmt.c**](syn__fmt_8c.md) _Lightweight formatting — no libc printf dependency._     
            * **file** [**syn\_fmt.h**](syn__fmt_8h.md) _Lightweight formatting — no libc printf dependency._     
            * **file** [**syn\_fsm.c**](syn__fsm_8c.md) _Finite state machine implementation._     
            * **file** [**syn\_fsm.h**](syn__fsm_8h.md) _Lightweight table-driven finite state machine._     
            * **file** [**syn\_hmac.h**](syn__hmac_8h.md) _HMAC-SHA256 — keyed-hash message authentication code._     
            * **file** [**syn\_hysteresis.h**](syn__hysteresis_8h.md) _Threshold comparator with hysteresis (deadband)._     
            * **file** [**syn\_json\_read.c**](syn__json__read_8c.md) _Minimal JSON reader — in-place tokenizer._     
            * **file** [**syn\_json\_read.h**](syn__json__read_8h.md) _Minimal JSON reader — in-place, zero-alloc._     
            * **file** [**syn\_json\_write.c**](syn__json__write_8c.md) _Streaming JSON builder implementation._     
            * **file** [**syn\_json\_write.h**](syn__json__write_8h.md) _Streaming JSON builder — zero-alloc, caller-provided buffer._     
            * **file** [**syn\_lut.h**](syn__lut_8h.md) _Lookup table with linear interpolation._     
            * **file** [**syn\_matrix.c**](syn__matrix_8c.md) _Fixed-point Q16.16 matrix operations implementation._     
            * **file** [**syn\_matrix.h**](syn__matrix_8h.md) _Fixed-point Q16.16 matrix operations — zero heap allocation._     
            * **file** [**syn\_metrics.c**](syn__metrics_8c.md) _Implementation of system-wide metrics registry._ 
            * **file** [**syn\_metrics.h**](syn__metrics_8h.md) _Lightweight telemetry and metrics (Counter/Gauge)._     
            * **file** [**syn\_netbuf.c**](syn__netbuf_8c.md) _Zero-Copy Packet Buffer Pool implementation._     
            * **file** [**syn\_netbuf.h**](syn__netbuf_8h.md) _Zero-Copy Packet Buffer Pool._     
            * **file** [**syn\_pack.h**](syn__pack_8h.md) _Binary message packer / unpacker (header-only)._     
            * **file** [**syn\_pingpong.h**](syn__pingpong_8h.md) _Ping-pong (double) buffer — header-only, zero-copy DMA pattern._     
            * **file** [**syn\_pool.h**](syn__pool_8h.md) _Fixed-size block memory pool — zero-heap, header-only._     
            * **file** [**syn\_pubsub.c**](syn__pubsub_8c.md)     
            * **file** [**syn\_pubsub.h**](syn__pubsub_8h.md) _Synchronous publish/subscribe event broker._     
            * **file** [**syn\_qmath.c**](syn__qmath_8c.md) _Q16.16 fixed-point transcendental functions and string I/O._     
            * **file** [**syn\_qmath.h**](syn__qmath_8h.md) _Fixed-point Q16.16 arithmetic — no floating point._     
            * **file** [**syn\_ramp.c**](syn__ramp_8c.md) _Ramp / motion profile generator implementation._     
            * **file** [**syn\_ramp.h**](syn__ramp_8h.md) _Ramp / motion profile generator._     
            * **file** [**syn\_random.c**](syn__random_8c.md) _Random number utility implementation._     
            * **file** [**syn\_random.h**](syn__random_8h.md) _Hardware-agnostic random number generation._     
            * **file** [**syn\_rate\_limit.h**](syn__rate__limit_8h.md) _Token bucket rate limiter — header-only._     
            * **file** [**syn\_ringbuf.c**](syn__ringbuf_8c.md) _Ring buffer implementation._     
            * **file** [**syn\_ringbuf.h**](syn__ringbuf_8h.md) _Fixed-size, statically-allocated ring buffer._     
            * **file** [**syn\_scurve.c**](syn__scurve_8c.md)     
            * **file** [**syn\_scurve.h**](syn__scurve_8h.md) _Jerk-limited S-curve trajectory generator._     
            * **file** [**syn\_sha256.c**](syn__sha256_8c.md) _SHA-256 implementation — FIPS 180-4 compliant, pure C99._     
            * **file** [**syn\_sha256.h**](syn__sha256_8h.md) _SHA-256 cryptographic hash — pure C99, zero dependencies._     
            * **file** [**syn\_slab.c**](syn__slab_8c.md) _Multi-Class Slab Memory Allocator implementation._     
            * **file** [**syn\_slab.h**](syn__slab_8h.md) _Multi-Class Slab Memory Allocator._     
            * **file** [**syn\_spinlock.h**](syn__spinlock_8h.md) _Scoped spinlock helper for shared resource protection._ 
            * **file** [**syn\_spsc\_queue.c**](syn__spsc__queue_8c.md) _Lock-free Single-Producer Single-Consumer (SPSC) queue implementation._     
            * **file** [**syn\_spsc\_queue.h**](syn__spsc__queue_8h.md) _Lock-free Single-Producer Single-Consumer (SPSC) queue._     
            * **file** [**syn\_str.c**](syn__str_8c.md) _Zero-allocation string parsing, trimming, and tokenization utilities._     
            * **file** [**syn\_str.h**](syn__str_8h.md) _Zero-allocation string parsing, trimming, and tokenization utilities._     
            * **file** [**syn\_stream.c**](syn__stream_8c.md) _Cooperative byte stream implementation._     
            * **file** [**syn\_stream.h**](syn__stream_8h.md) _Cooperative byte stream — protothread-aware ringbuf wrapper._     
            * **file** [**syn\_timeout.h**](syn__timeout_8h.md) _Non-blocking timeout helper — tick wrap-around safe._     
        * **file** [**syn\_config\_template.h**](syn__config__template_8h.md) _SyntropicOS configuration template._     
        * **file** [**syntropic.h**](syntropic_8h.md) _SyntropicOS umbrella header._ 

