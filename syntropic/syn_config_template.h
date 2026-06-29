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

#define SYN_USE_GPIO           1   /**< GPIO pin control                    */
#define SYN_USE_UART           1   /**< Buffered UART with ISR feed         */
#define SYN_USE_ADC            1   /**< ADC with oversampling + calibration */
#define SYN_USE_EXTI           1   /**< GPIO interrupt dispatcher           */
/* I2C/SPI device helpers are header-only — always available.               */

/* ── UART tuning ────────────────────────────────────────────────────────── */

#define SYN_UART_TX_BUF_SIZE   128 /**< TX ring buffer size (bytes)         */
#define SYN_UART_RX_BUF_SIZE   128 /**< RX ring buffer size (bytes)         */
#define SYN_UART_MAX_INSTANCES   2 /**< Max simultaneous UART handles       */

/* ── Multitasking ───────────────────────────────────────────────────────── */

#define SYN_USE_PT             1   /**< Protothreads (cooperative coroutines) */
#define SYN_USE_SCHED          1   /**< Cooperative task scheduler          */
#define SYN_USE_TIMER          1   /**< Software timers (one-shot + periodic) */
#define SYN_USE_EVENT          1   /**< Event flag groups (32-bit bitmask)  */
#define SYN_USE_WATCHDOG       1   /**< Task-level watchdog monitor         */
#define SYN_USE_SEQUENCER      1   /**< Timed action sequencer              */
#define SYN_USE_WORKQUEUE      1   /**< Deferred work queue (ISR→main)      */
/* Mailbox is header-only — always available.                                */

/* ── Services ───────────────────────────────────────────────────────────── */

#define SYN_USE_LOG            1   /**< Logging system (requires: FMT)      */
#define SYN_USE_CLI            1   /**< Command-line interface              */

/* ── Logging tuning ─────────────────────────────────────────────────────── */

#define SYN_LOG_LEVEL          1   /**< Compile-time min level (0=TRACE..5=FATAL, 6=NONE) */
#define SYN_LOG_BUF_SIZE     192   /**< Log output buffer size (bytes)      */
#define SYN_LOG_TIMESTAMP      1   /**< Include [tick] timestamp in output  */
#define SYN_LOG_COLOR          0   /**< Enable ANSI color codes             */

/* ── CLI tuning ─────────────────────────────────────────────────────────── */

#define SYN_CLI_LINE_BUF_SIZE 128  /**< Max command line length             */
#define SYN_CLI_MAX_ARGS       16  /**< Max argc (including command name)   */
#define SYN_CLI_HISTORY_DEPTH   0  /**< Command history depth (0=disabled)  */

/* ── Input / Output ─────────────────────────────────────────────────────── */

#define SYN_USE_BUTTON         1   /**< Button debouncer (requires: FSM)    */
#define SYN_USE_ENCODER        1   /**< Rotary encoder (quadrature)         */
#define SYN_USE_LED            1   /**< LED controller (blink/flash/pattern)*/
#define SYN_USE_SOFT_PWM       1   /**< Software PWM on any GPIO            */

/* ── Control & Motor ────────────────────────────────────────────────────── */

#define SYN_USE_PID            1   /**< PID controller                      */
#define SYN_USE_STEPPER        1   /**< Stepper motor (step/dir + accel)    */
#define SYN_USE_SERVO          1   /**< Hobby servo (pulse-width control)   */
#define SYN_USE_DC_MOTOR       1   /**< DC motor (H-bridge + ramp)          */
#define SYN_USE_MOTOR_CTRL     1   /**< Closed-loop motor controller (requires: PID) */
#define SYN_USE_AUTOTUNE       1   /**< Motor auto-tuner (requires: MOTOR_CTRL) */
#define SYN_USE_ACTUATOR       1   /**< Linear actuator (requires: PID)     */

/* ── DSP / Filters ──────────────────────────────────────────────────────── */

#define SYN_USE_FILTER         1   /**< Digital filters (MA, EMA, Median)   */
#define SYN_USE_SIGNAL         1   /**< Signal statistics (sliding window)  */
#define SYN_USE_BIQUAD         1   /**< Biquad digital filter               */
#define SYN_USE_FFT            1   /**< Fast Fourier Transform (FFT)        */
#define SYN_FILTER_MAX_WINDOW 32   /**< Max filter window size              */

/* ── State Machines / Concurrency ───────────────────────────────────────── */

#define SYN_USE_FSM            1   /**< Table-driven finite state machine   */
#define SYN_USE_AO             1   /**< Active Object (requires: FSM)       */

/* ── Communication / Protocols ──────────────────────────────────────────── */

#define SYN_USE_COBS           1   /**< COBS packet framing                 */
#define SYN_USE_MODBUS         1   /**< Modbus RTU slave (requires: CRC)    */
#define SYN_USE_MQTT           1   /**< MQTT 3.1.1 client                   */
#define SYN_USE_HTTP           1   /**< HTTP client                         */
#define SYN_USE_HTTPD          1   /**< HTTP server (embedded web server)   */
#define SYN_USE_WEBSOCKET      1   /**< WebSocket client                    */
#define SYN_USE_COAP           1   /**< CoAP protocol client                */
#define SYN_USE_DNS            1   /**< DNS resolver                        */
#define SYN_USE_ROUTER         1   /**< Network router / dispatcher         */
#define SYN_USE_HEARTBEAT      1   /**< Heartbeat / keep-alive service      */
#define SYN_USE_TRANSPORT_TCP  1   /**< TCP transport layer                 */

/* ── Sensor ─────────────────────────────────────────────────────────────── */

#define SYN_USE_SENSOR         1   /**< Sensor polling framework            */

/* ── Storage ────────────────────────────────────────────────────────────── */

#define SYN_USE_PARAM          1   /**< Wear-leveled parameter store (requires: CRC) */
#define SYN_USE_SETTINGS       1   /**< Settings manager (requires: PARAM, CRC) */
#define SYN_USE_VFS            1   /**< Virtual File System (VFS)           */
#define SYN_USE_LFS            1   /**< LittleFS adapter (requires: VFS)    */
#define SYN_USE_FAT            1   /**< FAT filesystem adapter (requires: VFS) */

/* ── Display / UI ───────────────────────────────────────────────────────── */

#define SYN_GFX_BACKEND_CANVAS  0   /**< Framebuffer canvas (default)       */
#define SYN_GFX_BACKEND_DIRECT  1   /**< Direct-draw (no framebuffer)       */
#define SYN_GFX_BACKEND         SYN_GFX_BACKEND_CANVAS /**< Active renderer */

#define SYN_USE_CANVAS         1   /**< Display canvas / framebuffer        */
#define SYN_USE_MENU           1   /**< UI menu system                      */
#define SYN_USE_IMGUI          1   /**< Immediate-mode GUI                  */

/* ── System ─────────────────────────────────────────────────────────────── */

#define SYN_USE_BOOT           1   /**< Boot manager (crash recovery)       */
#define SYN_USE_ERRLOG         1   /**< Persistent error registry           */
#define SYN_USE_FAULT          1   /**< Hard Fault diagnostics              */
#define SYN_USE_HWWDT          1   /**< Hardware watchdog timer              */
#define SYN_USE_POWER          1   /**< Power management (sleep/wake)       */
/* Version and Sleep are header-only — always available.                     */

/* ── Debug ──────────────────────────────────────────────────────────────── */

#define SYN_USE_TRACE          1   /**< Event trace buffer                  */
#define SYN_USE_PROFILER       1   /**< Task CPU profiler                   */

/* ── Logging ────────────────────────────────────────────────────────────── */

#define SYN_USE_DATALOG        1   /**< Data logger (ring buffer to flash)  */

/* ── Utilities ──────────────────────────────────────────────────────────── */

#define SYN_USE_FMT            1   /**< Lightweight printf alternative      */
#define SYN_USE_CBOR           1   /**< CBOR binary serializer              */
#define SYN_USE_JSON           1   /**< JSON reader/writer                  */
#define SYN_USE_PUBSUB         1   /**< Publish-subscribe message bus       */
#define SYN_USE_RAMP           1   /**< Ramp / slew rate generator          */
#define SYN_USE_SCURVE         1   /**< S-curve motion profile              */
#define SYN_CRC_USE_TABLE      1   /**< 1=lookup table (fast), 0=bitwise    */
/* Ring buffer, assert, bits, timeout, hysteresis, change filter, LUT,        */
/* Q-math, rate limiter, ping-pong, CRC, and mailbox are always available.    */

/* ── Assert configuration ───────────────────────────────────────────────── */

/* Uncomment to compile out all SYN_ASSERT() calls in release builds. */
/* #define SYN_DISABLE_ASSERT */

#endif /* SYN_CONFIG_H */
