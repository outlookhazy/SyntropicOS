/**
 * @file syn_config_template.h
 * @brief SyntropicOS configuration template.
 *
 * HOW TO USE:
 *   1. Copy this file to your project's include path.
 *   2. Rename the copy to "syn_config.h".
 *   3. Edit the copy to enable/disable modules and set buffer sizes.
 *
 * SyntropicOS headers look for "syn_config.h" on the include path. If the
 * file is not found, sensible defaults are used.
 */

#ifndef SYN_CONFIG_H
#define SYN_CONFIG_H

/* ── Drivers ────────────────────────────────────────────────────────────── */

#define SYN_USE_GPIO 1     /**< GPIO pin control                    */
#define SYN_USE_UART 1     /**< Peripheral UARTs (Modbus, GPS, etc) */
#define SYN_USE_ADC 1      /**< ADC with oversampling + calibration */
#define SYN_USE_EXTI 1     /**< GPIO interrupt dispatcher           */
#define SYN_USE_HPCLOCK 1  /**< High-precision 64-bit system clock  */
#define SYN_USE_TIMESYNC 1 /**< GPS/RTC time discipline service     */
#define SYN_USE_GEO 1      /**< WGS84 & 3D local ENU geodetic math  */
#define SYN_GNSS_USE_FIXED_POINT \
    0 /**< 0 = 64-bit double, 1 = 32-bit Micro-Degree (1e-7 deg) integer */

/* Console serial (syn_port_serial) is always available — used by CLI/log.  */
/* I2C/SPI device helpers are header-only — always available.               */

/* ── UART tuning ────────────────────────────────────────────────────────── */

#define SYN_UART_TX_BUF_SIZE 128 /**< TX ring buffer size (bytes)         */
#define SYN_UART_RX_BUF_SIZE 128 /**< RX ring buffer size (bytes)         */
#define SYN_UART_MAX_INSTANCES 2 /**< Max simultaneous UART handles       */

/* ── Multitasking ───────────────────────────────────────────────────────── */

#define SYN_USE_PT 1                      /**< Protothreads (cooperative coroutines) */
#define SYN_USE_SCHED 1                   /**< Cooperative task scheduler          */
#define SYN_SCHED_PRIO_LEVELS 4           /**< Priority levels (0..N-1)          */
#define SYN_SCHED_MAX_WAIT_CRITICAL_MS 1U /**< Starvation window for CRITICAL    */
#define SYN_SCHED_MAX_WAIT_HIGH_MS 5U     /**< Starvation window for HIGH        */
#define SYN_SCHED_MAX_WAIT_NORMAL_MS 15U  /**< Starvation window for NORMAL      */
#define SYN_SCHED_MAX_WAIT_LOW_MS 50U     /**< Starvation window for LOW         */
#define SYN_USE_TIMER 1                   /**< Software timers (one-shot + periodic) */
#define SYN_USE_EVENT 1                   /**< Event flag groups (32-bit bitmask)  */
#define SYN_USE_MUTEX 1                   /**< Priority-inheriting mutex           */
#define SYN_MUTEX_MAX_WAITERS 4U          /**< Tracked contended waiters per mutex */
#define SYN_USE_WATCHDOG 1                /**< Task-level watchdog monitor         */
#define SYN_USE_SEQUENCER 1               /**< Timed action sequencer              */
#define SYN_USE_WORKQUEUE 1               /**< Deferred work queue (ISR→main)      */
#define SYN_USE_TICKLESS 0 /**< Tickless idle scheduler (requires: SCHED, port sleep_until) */
/* Mailbox is header-only — always available.                                */

/* ── Services ───────────────────────────────────────────────────────────── */

#define SYN_USE_LOG 1        /**< Logging system (requires: FMT)      */
#define SYN_USE_CLI 1        /**< Command-line interface (requires: FMT) */
#define SYN_USE_WASM 1       /**< [EXPERIMENTAL] WebAssembly MVP interpreter */
#define SYN_WASM_USE_FIXED 0 /**< f32 opcodes use Q16.16 fixed-point */

/* ── Logging tuning ─────────────────────────────────────────────────────── */

#define SYN_LOG_LEVEL 1      /**< Compile-time min level (0=TRACE..5=FATAL, 6=NONE) */
#define SYN_LOG_BUF_SIZE 192 /**< Log output buffer size (bytes)      */
#define SYN_LOG_TIMESTAMP 1  /**< Include [tick] timestamp in output  */
#define SYN_LOG_COLOR 0      /**< Enable ANSI color codes             */

/* ── CLI tuning ─────────────────────────────────────────────────────────── */

#define SYN_CLI_LINE_BUF_SIZE 128 /**< Max command line length             */
#define SYN_CLI_MAX_ARGS 16       /**< Max argc (including command name)   */
#define SYN_CLI_HISTORY_DEPTH 0   /**< Command history depth (0=disabled)  */

/* ── Input / Output ─────────────────────────────────────────────────────── */

#define SYN_USE_BUTTON 1   /**< Button debouncer (requires: FSM)    */
#define SYN_USE_ENCODER 1  /**< Rotary encoder (quadrature)         */
#define SYN_USE_LED 1      /**< LED controller (blink/flash/pattern)*/
#define SYN_USE_SOFT_PWM 1 /**< Software PWM on any GPIO            */

/* ── Control & Motor ────────────────────────────────────────────────────── */

#define SYN_USE_PID 1        /**< PID controller                      */
#define SYN_USE_STEPPER 1    /**< Stepper motor (step/dir + accel)    */
#define SYN_USE_SERVO 1      /**< Hobby servo (pulse-width control)   */
#define SYN_USE_DC_MOTOR 1   /**< DC motor (H-bridge + ramp)          */
#define SYN_USE_MOTOR_CTRL 1 /**< Closed-loop motor controller (requires: PID) */
#define SYN_USE_AUTOTUNE 1   /**< Motor auto-tuner (requires: MOTOR_CTRL) */
#define SYN_USE_ACTUATOR 1   /**< Linear actuator (requires: PID)     */
#define SYN_USE_GCODE 1      /**< G-Code RS-274 motion interpreter (requires: INTERPOLATOR) */
#define SYN_USE_KINEMATICS 1 /**< Multi-axis robot forward & inverse kinematics */

/* ── DSP / Filters ──────────────────────────────────────────────────────── */

#define SYN_USE_FILTER 1         /**< Digital filters (MA, EMA, Median)   */
#define SYN_USE_SIGNAL 1         /**< Signal statistics (sliding window)  */
#define SYN_USE_BIQUAD 1         /**< Biquad digital filter               */
#define SYN_USE_FFT 1            /**< Fast Fourier Transform (FFT)        */
#define SYN_FILTER_MAX_WINDOW 32 /**< Max filter window size              */

/* ── State Machines / Concurrency ───────────────────────────────────────── */

#define SYN_USE_FSM 1 /**< Table-driven finite state machine   */
#define SYN_USE_AO 1  /**< Active Object (requires: FSM, SCHED) */

/* ── Cryptography ───────────────────────────────────────────────────────── */

#define SYN_USE_BLAKE2S 1          /**< BLAKE2s hash + keyed MAC (RFC 7693) */
#define SYN_USE_CHACHA20POLY1305 1 /**< ChaCha20-Poly1305 AEAD (RFC 8439)  */
#define SYN_USE_X25519 1           /**< X25519 Diffie-Hellman (RFC 7748)   */
#define SYN_USE_P256 1             /**< NIST P-256 / secp256r1 ECC          */
#define SYN_P256_USE_COMB_TABLE 0  /**< Precomputed G comb table (opt-in)   */
#define SYN_USE_HMAC_DRBG 1        /**< NIST SP 800-90A HMAC-DRBG (SHA-256) */
#define SYN_USE_BASE64 1           /**< RFC 4648 Base64 & Base64URL codec   */
#define SYN_USE_SHA512 1           /**< SHA-512 / SHA-384 / HMAC-SHA512/384 */
#define SYN_USE_TPM2 1             /**< TCG TPM 2.0 command & measured boot engine */

/* ── Bluetooth Low Energy ───────────────────────────────────────────────── */

#define SYN_USE_BLE 1                /**< BLE Host Stack (HCI/GAP/GATT)       */
#define SYN_BLE_MAX_CONNECTIONS 2U   /**< Max simultaneous BLE connections   */
#define SYN_BLE_MAX_ATTRIBUTES 32U   /**< Max GATT attribute table capacity   */
#define SYN_BLE_HCI_RX_BUF_SIZE 260U /**< HCI RX packet capacity              */
#define SYN_BLE_HCI_TX_BUF_SIZE 261U /**< HCI TX packet capacity              */

/* ── Communication / Protocols ──────────────────────────────────────────── */

#define SYN_USE_CANOPEN 1        /**< CANopen DS301 slave protocol engine */
#define SYN_USE_CIA402 1         /**< CiA 402 Drive & Motion profile engine */
#define SYN_USE_COBS 1           /**< COBS packet framing                 */
#define SYN_USE_COSE 1           /**< COSE (RFC 9052) signing & encryption */
#define SYN_USE_ETHERCAT 1       /**< EtherCAT bare-metal protocol stack  */
#define SYN_USE_LIN 1            /**< LIN bus 2.1/2.2a & ISO 17987 stack  */
#define SYN_USE_DALI 1           /**< DALI / IEC 62386 lighting stack    */
#define SYN_USE_SMBUS 1          /**< SMBus 1.1/2.0/3.0 protocol engine   */
#define SYN_USE_PMBUS 1          /**< PMBus 1.2/1.3 protocol & linear fmt */
#define SYN_USE_MBUS 1           /**< M-Bus (EN 13757-2/3) protocol engine */
#define SYN_USE_MODBUS 1         /**< Modbus RTU slave (requires: CRC)    */
#define SYN_USE_MQTT 1           /**< MQTT 3.1.1 client                   */
#define SYN_USE_MQTTSN 1         /**< MQTT-SN v1.2 datagram client engine */
#define SYN_USE_HTTP 1           /**< HTTP client                         */
#define SYN_USE_HTTPD 1          /**< HTTP server (embedded web server)   */
#define SYN_USE_WEBSOCKET 1      /**< WebSocket client                    */
#define SYN_USE_COAP 1           /**< CoAP protocol client                */
#define SYN_USE_DTLS 1           /**< DTLS 1.3 datagram security (RFC 9147) */
#define SYN_USE_LWM2M 1          /**< OMA Lightweight M2M client engine   */
#define SYN_USE_LWM2M_TASK 1     /**< LwM2M Autonomous Client Task engine */
#define SYN_USE_XRCE_DDS 1       /**< Micro-ROS / XRCE-DDS v1.2 client engine */
#define SYN_USE_OPCUA 1          /**< Micro OPC UA Binary server engine (IEC 62541) */
#define SYN_USE_IOLINK 1         /**< IO-Link (IEC 61131-9) Master & Device protocol */
#define SYN_USE_DNS 1            /**< DNS resolver                        */
#define SYN_USE_ETH 1            /**< Zero-heap Ethernet II & ARP engine  */
#define SYN_ETH_ARP_CACHE_SIZE 8 /**< ARP table cache size (entries)      */
#define SYN_USE_DHCP 1           /**< DHCP client protocol engine         */
#define SYN_USE_ICMP 1           /**< ICMP Echo Ping protocol engine      */
#define SYN_USE_AUTOIP 1         /**< RFC 3927 AutoIP Link-Local engine   */
#define SYN_USE_NETCFG 1         /**< Network IP Manager & Link Events    */
#define SYN_USE_IGMP 1           /**< IGMPv2 Multicast protocol engine    */
#define SYN_USE_ROUTER 1         /**< Network router / dispatcher         */
#define SYN_USE_HEARTBEAT 1      /**< Heartbeat / keep-alive service      */
#define SYN_USE_SNTP 1           /**< SNTP time synchronization client    */
#define SYN_USE_WG \
    1 /**< WireGuard VPN client (requires: BLAKE2S, CHACHA20POLY1305, X25519, SNTP) */
#define SYN_WG_MTU 1420         /**< WireGuard tunnel MTU (inner payload) */
#define SYN_USE_TRANSPORT_TCP 1 /**< TCP transport layer                 */

/* ── Sensor ─────────────────────────────────────────────────────────────── */

#define SYN_USE_SENSOR 1 /**< Sensor polling framework            */

/* ── Storage ────────────────────────────────────────────────────────────── */

#define SYN_USE_PARAM 1    /**< Wear-leveled parameter store (requires: CRC) */
#define SYN_USE_SETTINGS 1 /**< Settings manager (requires: PARAM, CRC) */
#define SYN_USE_VFS 1      /**< Virtual File System (VFS)           */
#define SYN_USE_LFS 1      /**< LittleFS adapter (requires: VFS)    */
#define SYN_USE_FAT 1      /**< FAT filesystem adapter (requires: VFS) */

/* ── Display / UI ───────────────────────────────────────────────────────── */

#define SYN_GFX_BACKEND_CANVAS 0               /**< Framebuffer canvas (default)       */
#define SYN_GFX_BACKEND_DIRECT 1               /**< Direct-draw (no framebuffer)       */
#define SYN_GFX_BACKEND SYN_GFX_BACKEND_CANVAS /**< Active renderer */

#define SYN_USE_CANVAS 1 /**< Display canvas / framebuffer        */
#define SYN_USE_MENU 1   /**< UI menu system                      */
#define SYN_USE_IMGUI 1  /**< Immediate-mode GUI                  */

/* ── System ─────────────────────────────────────────────────────────────── */

#define SYN_USE_BOOT 1     /**< Boot manager (crash recovery)       */
#define SYN_USE_OTA 1      /**< Secure Streaming OTA Orchestrator   */
#define SYN_USE_ERRLOG 1   /**< Persistent error registry           */
#define SYN_USE_FAULT 1    /**< Hard Fault diagnostics              */
#define SYN_USE_HWWDT 1    /**< Hardware watchdog timer              */
#define SYN_USE_POWER 1    /**< Power management (sleep/wake)       */
#define SYN_USE_COREDUMP 0 /**< Persistent core dump to flash (requires: FAULT, CRC) */
/* #define SYN_COREDUMP_FLASH_ADDR 0x0803F800 */ /**< Flash address for core dump sector */
#define SYN_COREDUMP_STACK_SIZE 128              /**< Bytes of stack to capture           */
/* Version and Sleep are header-only — always available.                     */

/* ── Firmware Security ─────────────────────────────────────────────────── */

#define SYN_FW_USE_HMAC 0    /**< HMAC-signed firmware images (requires: SHA256, BOOT) */
#define SYN_FW_USE_ED25519 1 /**< Ed25519-signed firmware images (requires: ED25519, BOOT) */

/* ── DMA ───────────────────────────────────────────────────────────────── */

#define SYN_USE_DMA 0 /**< DMA port abstraction                */

/* ── Async Peripherals ─────────────────────────────────────────────────── */

#define SYN_USE_I2C_ASYNC 0 /**< Async I2C transactions              */
#define SYN_USE_SPI_ASYNC 0 /**< Async SPI transactions              */

/* ── Multicore (AMP) ───────────────────────────────────────────────────── */

#define SYN_USE_MULTICORE 0  /**< AMP multicore support               */
#define SYN_SPINLOCK_COUNT 4 /**< Number of spinlock IDs              */

/* ── Debug ──────────────────────────────────────────────────────────────── */

#define SYN_USE_TRACE 1    /**< Event trace buffer                  */
#define SYN_USE_PROFILER 1 /**< Task CPU profiler                   */

/* ── Logging ────────────────────────────────────────────────────────────── */

#define SYN_USE_DATALOG 1 /**< Data logger (ring buffer to flash)  */

/* ── Cryptography ───────────────────────────────────────────────────────── */

#define SYN_USE_AES 1            /**< Unified AES block cipher & AEAD     */
#define SYN_USE_PORT_AES 0       /**< Hardware AES/crypto port offload    */
#define SYN_AES_MAX_KEY_BITS 256 /**< Max AES key bits: 128, 192, or 256  */
#define SYN_USE_AES_DECRYPT 1    /**< AES ECB/CBC decryption (+256B Flash)*/
#define SYN_USE_AES_CBC 1        /**< AES-CBC mode with PKCS#7            */
#define SYN_USE_AES_CTR 1        /**< AES-CTR stream cipher mode          */
#define SYN_USE_AES_GCM 1        /**< AES-GCM AEAD authenticated cipher   */
#define SYN_USE_AES_CCM 1        /**< AES-CCM AEAD (NIST SP 800-38C / BLE)*/
#define SYN_AES_GCM_TABLE 0      /**< GCM GHASH table: 0=none, 4=256B, 8=4KB*/
#define SYN_USE_AES_CMAC 1       /**< AES-CMAC message authentication     */
#define SYN_USE_SHA256 1         /**< SHA-256 hash + HMAC-SHA256          */

/* ── Utilities ──────────────────────────────────────────────────────────── */

#define SYN_USE_FMT 1           /**< Lightweight printf alternative      */
#define SYN_USE_CBOR 1          /**< CBOR binary serializer              */
#define SYN_USE_JSON 1          /**< JSON reader/writer                  */
#define SYN_USE_POOL 1          /**< Fixed-size block memory pool        */
#define SYN_USE_PUBSUB 1        /**< Publish-subscribe message bus       */
#define SYN_USE_RAMP 1          /**< Ramp / slew rate generator          */
#define SYN_USE_SCURVE 1        /**< S-curve motion profile              */
#define SYN_USE_GOERTZEL 1      /**< Goertzel tone detector              */
#define SYN_USE_LZ4 1           /**< Zero-heap LZ4 compression           */
#define SYN_USE_USB_MIDI 1      /**< USB MIDI Class 1.0 device           */
#define SYN_USE_MFCC 1          /**< Fixed-point MFCC feature extractor  */
#define SYN_USE_VAD 1           /**< Zero-heap Voice Activity Detector   */
#define SYN_USE_USB_MSC 1       /**< USB Mass Storage Class BOT device   */
#define SYN_USE_NTP_SERVER 1    /**< NTP v4 time server                  */
#define SYN_USE_PROTOBUF 1      /**< Protocol Buffers wire format        */
#define SYN_LZ4_HASH_SIZE 4096U /**< 4096 entries (8 KB)              */
#define SYN_CRC_USE_TABLE 1     /**< 1=lookup table (fast), 0=bitwise    */
/* Ring buffer, assert, bits, timeout, hysteresis, change filter, LUT,        */
/* Q-math, rate limiter, ping-pong, CRC, and mailbox are always available.    */

/* ── Assert configuration ───────────────────────────────────────────────── */

/* Uncomment to compile out all SYN_ASSERT() calls in release builds. */
/* #define SYN_DISABLE_ASSERT */

/* ── Audio configuration ────────────────────────────────────────────────── */
#ifndef SYN_AUDIO_MIXER_MAX_CHANNELS
#define SYN_AUDIO_MIXER_MAX_CHANNELS 4U /**< Maximum concurrent mixer channels */
#endif

#endif /* SYN_CONFIG_H */
