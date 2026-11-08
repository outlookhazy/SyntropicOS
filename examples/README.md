# SyntropicOS Examples Directory

This directory contains MCU HAL and bare-metal C example projects demonstrating SyntropicOS non-blocking drivers, protocol stacks, and coroutine scheduling across multiple target platforms (**Arduino / Portable C**, **STM32 Bare-Metal**, **WCH CH32V307 RISC-V**, **Raspberry Pi Pico RP2040**, and **ESP32**).

## Target Platform Legend

- **`[Arduino / Portable C]`** — Cross-platform C99/C++ code compatible with Arduino IDE, PlatformIO, AVR, and bare-metal MCUs.
- **`[STM32 Bare-Metal]`** — Native STM32 HAL / CMSIS C projects.
- **`[CH32V307 RISC-V]`** — WCH CH32V307 QingKe V4F RISC-V bare-metal C projects.
- **`[Pico RP2040]`** — Raspberry Pi Pico SDK / Dual-Core SMP projects.
- **`[ESP32]`** — ESP32 / ESP-IDF / Arduino-ESP32 OTA projects.

---

## Categories & Examples

### Industrial Protocols & Lighting
- **[`stm32_modbus_tcp`](stm32_modbus_tcp)** `[STM32 Bare-Metal]` — Dual Modbus TCP Server (port 502) & Client (Master) in a single project.
- **[`stm32_modbus_master`](stm32_modbus_master)** `[STM32 Bare-Metal]` — STM32 RS485 Modbus RTU Master querying field slave registers.
- **[`stm32_modbus_slave`](stm32_modbus_slave)** `[STM32 Bare-Metal]` — STM32 RS485 Modbus RTU Slave register map & exception handler.
- **[`ModbusSlave`](ModbusSlave)** `[Arduino / Portable C]` — Generic bare-metal Modbus RTU Slave implementation.
- **[`stm32_ethercat_servo`](stm32_ethercat_servo)** `[STM32 Bare-Metal]` — EtherCAT (IEEE 802.3 EtherType 0x88A4) Slave node & CiA 402 drive control.
- **[`stm32_bacnet_mstp`](stm32_bacnet_mstp)** `[STM32 Bare-Metal]` — STM32 RS485 BACnet MS/TP Smart Thermostat / HVAC Sensor node.
- **[`stm32_dali_lighting`](stm32_dali_lighting)** `[STM32 Bare-Metal]` — STM32 DALI (IEC 62386) LED Dimmer / Control Gear node.
- **[`stm32_dmx512`](stm32_dmx512)** `[STM32 Bare-Metal]` — STM32 DMX512 stage lighting receiver & PWM output controller.

### Automotive & Marine Fieldbus
- **[`stm32_canopen`](stm32_canopen)** `[STM32 Bare-Metal]` — STM32 CANopen (CiA 301) Node with SDO/PDO dictionary maps.
- **[`stm32_canopen_master`](stm32_canopen_master)** `[STM32 Bare-Metal]` — STM32 CANopen (CiA 302/301) Network Manager, SDO Client & NMT Master.
- **[`stm32_canopen_nmea2k_gateway`](stm32_canopen_nmea2k_gateway)** `[STM32 Bare-Metal]` — CANopen to NMEA 2000 Marine CAN gateway broadcasting battery, DC status, and alerts (1500ms).
- **[`stm32_can_isotp`](stm32_can_isotp)** `[STM32 Bare-Metal]` — STM32 ISO-TP (ISO 15765-2) CAN multi-frame transport layer.
- **[`stm32_j1939`](stm32_j1939)** `[STM32 Bare-Metal]` — SAE J1939 Heavy-Duty Vehicle CAN protocol & DM1 diagnostics.
- **[`stm32_lin_bus`](stm32_lin_bus)** `[STM32 Bare-Metal]` — LIN 2.1 Automotive Single-Wire Bus Master & Slave state machine.
- **[`stm32_nmea2k`](stm32_nmea2k)** `[STM32 Bare-Metal]` — NMEA 2000 (IEC 61162-3) Marine CAN PGN encoder & decoder.

- **[`stm32_cannm_demo`](stm32_cannm_demo)** `[STM32 Bare-Metal]` — AUTOSAR CAN Network Management (CanNm) 5-state FSM demo.
- **[`stm32_dtc_manager_demo`](stm32_dtc_manager_demo)** `[STM32 Bare-Metal]` — ISO 14229-1 UDS Application-Layer DTC Manager & Freeze-Frame Demo.
- **[`stm32_kwp2000_diag`](stm32_kwp2000_diag)** `[STM32 Bare-Metal]` — ISO 14230-3 KWP2000 diagnostic service stack demo.

### Smart Energy & Power Management
- **[`stm32_ocpp_evse`](stm32_ocpp_evse)** `[STM32 Bare-Metal]` — OCPP 2.1 / 1.6-J EVSE Charging Station protocol controller.
- **[`stm32_mbus_meter`](stm32_mbus_meter)** `[STM32 Bare-Metal]` — M-Bus (Meter-Bus EN 13757) Utility Meter Reader (Water, Gas, Heat).
- **[`stm32_pmbus_power`](stm32_pmbus_power)** `[STM32 Bare-Metal]` — PMBus 1.2/1.3 Digital Power Supply Telemetry & Linear11/16 format decoding.
- **[`stm32_smbus_battery`](stm32_smbus_battery)** `[STM32 Bare-Metal]` — SMBus 2.0 / SBS 1.1 Smart Battery System Telemetry & Alert Handler.
- **[`PmbusTelemetry`](PmbusTelemetry)** `[Arduino / Portable C]` — Generic PMBus power telemetry converter.
- **[`stm32_dlt645_meter`](stm32_dlt645_meter)** `[STM32 Bare-Metal]` — DLT645 Smart Electricity Meter protocol parser.

### Wireless & Audio Processing
- **[`stm32_ble_sensor`](stm32_ble_sensor)** `[STM32 Bare-Metal]` — BLE Host Stack (HCI H:4 UART) GAP/GATT sensor server.
- **[`stm32_audio_player`](stm32_audio_player)** `[STM32 Bare-Metal]` — Software audio player with WAV parsing, IMA-ADPCM, and SBC decoding.

### Microcontroller Peripheral HAL & CLI Shell
- **[`stm32f7_uds_bootloader`](stm32f7_uds_bootloader)** `[STM32 Bare-Metal]` — ISO 14229-1 UDS 17-step secure bootloader with A/B dual-bank swap architecture.
- **[`stm32_ymodem_bootloader`](stm32_ymodem_bootloader)** `[STM32 Bare-Metal]` — Serial YMODEM / XMODEM-1K firmware update bootloader.
- **[`stm32_usb_cdc_device`](stm32_usb_cdc_device)** `[STM32 Bare-Metal]` — USB 2.0 CDC ACM Virtual COM Port Device & coroutine echo task.
- **[`stm32_usb_host_cdc`](stm32_usb_host_cdc)** `[STM32 Bare-Metal]` — USB 2.0 Host Core enumeration state machine & CDC ACM serial host driver.
- **[`stm32_multitask_demo`](stm32_multitask_demo)** `[STM32 Bare-Metal]` — Integrated 4-task protothreads (`SYN_PT`) demo (LED, USART CLI, Button Gestures, Global IPC).
- **[`stm32_cli_shell`](stm32_cli_shell)** `[STM32 Bare-Metal]` — Interactive USART CLI Shell (`led`, `status`, `temp`).
- **[`SerialCLI`](SerialCLI)** `[Arduino / Portable C]` — Generic serial command-line interpreter over UART.
- **[`stm32_rtc`](stm32_rtc)** `[STM32 Bare-Metal]` — Real-Time Clock (RTC) perpetual calendar & USART protocol reader/setter.
- **[`stm32_encoder_button`](stm32_encoder_button)** `[STM32 Bare-Metal]` — EC11 Rotary Encoder & push-button debounced menu controller.
- **[`stm32_joystick`](stm32_joystick)** `[STM32 Bare-Metal]` — Dual-axis analog joystick ADC sampler & 8-way D-Pad decoder.
- **[`stm32_keypad`](stm32_keypad)** `[STM32 Bare-Metal]` — 4x4 Matrix keypad scanner & PIN security entry.
- **[`stm32_touch_key`](stm32_touch_key)** `[STM32 Bare-Metal]` — 4-channel capacitive touch sensing key pad & baseline calibration.
- **[`stm32_dipswitch`](stm32_dipswitch)** `[STM32 Bare-Metal]` — 8-position DIP switch address reader & baud rate selector.
- **[`stm32_soft_pwm`](stm32_soft_pwm)** `[STM32 Bare-Metal]` — Multi-channel software PWM LED dimmer & motor driver.
- **[`stm32_led`](stm32_led)** `[STM32 Bare-Metal]` — GPIO status LED heartbeat, blinking, and error patterns.
- **[`stm32_smart_led`](stm32_smart_led)** `[STM32 Bare-Metal]` — WS2812B / Neopixel Smart RGB LED strip rainbow animator.
- **[`stm32_buzzer`](stm32_buzzer)** `[STM32 Bare-Metal]` — Piezo buzzer audio tone, chime arpeggio, and siren alarm.
- **[`stm32_button`](stm32_button)** `[STM32 Bare-Metal]` — Multi-tap button gesture & combo handler.
- **[`stm32_spsc_usart`](stm32_spsc_usart)** `[STM32 Bare-Metal]` — Single-Producer Single-Consumer lock-free ring queue for USART RX ISR.
- **[`stm32_ringbuf_usart`](stm32_ringbuf_usart)** `[STM32 Bare-Metal]` — Non-blocking ring buffer USART RX ingestion.
- **[`stm32_uart_mcu_comm`](stm32_uart_mcu_comm)** `[STM32 Bare-Metal]` — Inter-MCU UART packet routing & COBS framing.
- **[`stm32_crypto_usart`](stm32_crypto_usart)** `[STM32 Bare-Metal]` — Encrypted USART receiver with SHA-256 digest & AES-128.
- **[`stm32_flash`](stm32_flash)** `[STM32 Bare-Metal]` — Internal MCU flash erase, sector write, and parameter store.
- **[`stm32_profiler`](stm32_profiler)** `[STM32 Bare-Metal]` — CPU usage & execution time task profiler (`syn_profiler`).
- **[`stm32_log_console`](stm32_log_console)** `[STM32 Bare-Metal]` — Asynchronous ring-buffered log console.
- **[`stm32_serial`](stm32_serial)** `[STM32 Bare-Metal]` — Bare-metal serial UART transmit & receive.
- **[`stm32_json`](stm32_json)** `[STM32 Bare-Metal]` — Zero-malloc JSON parsing & encoding.
- **[`stm32_ir_remote`](stm32_ir_remote)** `[STM32 Bare-Metal]` — NEC protocol Infrared Remote Control decoder.
- **[`stm32_ppm`](stm32_ppm)** `[STM32 Bare-Metal]` — Pulse-Position Modulation (PPM) Multi-Channel RC Receiver decoder (`syn_ppm`).
- **[`PicoBlink`](PicoBlink)** `[Pico RP2040 / Arduino]` — Raspberry Pi Pico bare-metal GPIO blink.
- **[`PicoDualCore`](PicoDualCore)** / **[`pico_dual_core`](pico_dual_core)** `[Pico RP2040 / Arduino]` — RP2040 SMP dual-core cooperative task execution.
- **[`ch32v307_demo`](ch32v307_demo)** `[CH32V307 RISC-V]` — WCH CH32V307V-EVT-R1 RISC-V demo (GPIO LED, ADC telemetry, CAN 2.0B TX).
- **[`esp32_ota`](esp32_ota)** `[ESP32 / Arduino]` — ESP32 Firmware Over-The-Air (OTA) update task.

### DSP & Motion Control
- **[`stm32_dshot`](stm32_dshot)** `[STM32 Bare-Metal]` — DShot150/300/600 Digital ESC command encoder & BDShot GCR motor telemetry decoder (`syn_dshot`).
- **[`stm32_dshot_telemetry`](stm32_dshot_telemetry)** `[STM32 Bare-Metal]` — Bidirectional DShot (BDShot) 20-bit GCR motor telemetry & RPM decoder (`syn_dshot_telemetry`).
- **[`stm32_stepper`](stm32_stepper)** `[STM32 Bare-Metal]` — Stepper motor trapezoidal speed ramp & position controller.
- **[`stm32_servo`](stm32_servo)** `[STM32 Bare-Metal]` — RC servo motor 50Hz PWM pulse-width & smooth angle ramp controller.
- **[`MotionPlanner`](MotionPlanner)** `[Arduino / Portable C]` — Trapezoidal S-curve motor motion planner.
- **[`MotorFSM`](MotorFSM)** `[Arduino / Portable C]` — Finite state machine controlling a DC motor ramp profile.
- **[`PID_TempControl`](PID_TempControl)** `[Arduino / Portable C]` — Closed-loop integer PID temperature controller.
- **[`BiquadFilter`](BiquadFilter)** `[Arduino / Portable C]` — Audio & sensor digital biquad IIR filtering.
- **[`FftSpectrumAnalyzer`](FftSpectrumAnalyzer)** `[Arduino / Portable C]` — Real-time FFT spectral decomposition.

### IoT & Network Protocol Stacks
- **[`MqttClient`](MqttClient)** `[Arduino / Portable C]` — MQTT v3.1.1 network client with QoS0/QoS1 support.
- **[`CoapClient`](CoapClient)** `[Arduino / Portable C]` — CoAP (RFC 7252) UDP client with Option header encoding.
- **[`WebsocketServer`](WebsocketServer)** `[Arduino / Portable C]` — Lightweight WebSocket server handling frames & handshakes.
- **[`EthernetWebServer`](EthernetWebServer)** `[Arduino / Portable C]` — Embedded HTTP 1.1 Web Server.
- **[`HttpClient`](HttpClient)** `[Arduino / Portable C]` — Non-blocking HTTP GET/POST client.
- **[`DnsResolver`](DnsResolver)** `[Arduino / Portable C]` — DNS hostname resolution client.
- **[`SntpClock`](SntpClock)** `[Arduino / Portable C]` — SNTP Network Time Protocol client sync.
- **[`Telemetry_CBOR`](Telemetry_CBOR)** `[Arduino / Portable C]` — Concise Binary Object Representation (CBOR RFC 8949) encoder.
- **[`stm32_sim800_mqtt`](stm32_sim800_mqtt)** `[STM32 Bare-Metal]` — SIM800 GSM/GPRS Cellular Modem MQTT Client.

### System Core & Utilities
- **[`Blink`](Blink)** `[Arduino / Portable C]` — Basic protothread LED blinker.
- **[`ButtonEvents`](ButtonEvents)** `[Arduino / Portable C]` — Tap gestures, double clicks, and chorded button combos.
- **[`SensorLogger`](SensorLogger)** `[Arduino / Portable C]` — Dual-channel ADC sampling, EMA filtering, and Serial CLI.
- **[`PersistentSettings`](PersistentSettings)** `[Arduino / Portable C]` — Wear-leveled key-value parameter storage.
- **[`SysMonitor`](SysMonitor)** `[Arduino / Portable C]` — Task execution monitor & health logger.
- **[`TaskMailbox`](TaskMailbox)** `[Arduino / Portable C]` — Inter-task message passing via Mailbox queue.
- **[`TaskWatchdog`](TaskWatchdog)** `[Arduino / Portable C]` — Hardware & software multi-task watchdog supervisor.
- **[`GpsNmeaParser`](GpsNmeaParser)** `[Arduino / Portable C]` — NMEA 0183 GPS sentence stream parser.
