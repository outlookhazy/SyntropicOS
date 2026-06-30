/**
 * @file syn_config.h
 * @brief Lean SyntropicOS config for Pi Pico bring-up test.
 *
 * Enables only the modules needed for the blink + serial diagnostics
 * sketch.  Keeps compile time fast and avoids pulling in port functions
 * that aren't implemented yet.
 */

#ifndef SYN_CONFIG_H
#define SYN_CONFIG_H

/* ── Drivers ────────────────────────────────────────────────────────────── */

#define SYN_USE_GPIO           1
#define SYN_USE_UART           1
#define SYN_USE_ADC            0   /* not used in this test */
#define SYN_USE_EXTI           0

/* ── Multitasking ───────────────────────────────────────────────────────── */

#define SYN_USE_PT             1
#define SYN_USE_SCHED          1
#define SYN_USE_TIMER          1
#define SYN_USE_EVENT          1
#define SYN_USE_WATCHDOG       0
#define SYN_USE_SEQUENCER      0
#define SYN_USE_WORKQUEUE      0
#define SYN_USE_TICKLESS       0

/* ── Services ───────────────────────────────────────────────────────────── */

#define SYN_USE_LOG            1
#define SYN_USE_CLI            0

/* ── Logging tuning ─────────────────────────────────────────────────────── */

#define SYN_LOG_LEVEL          1   /* DEBUG */
#define SYN_LOG_BUF_SIZE     192
#define SYN_LOG_TIMESTAMP      1
#define SYN_LOG_COLOR          0

/* ── Input / Output ─────────────────────────────────────────────────────── */

#define SYN_USE_BUTTON         0
#define SYN_USE_ENCODER        0
#define SYN_USE_LED            1
#define SYN_USE_SOFT_PWM       0

/* ── Control & Motor ────────────────────────────────────────────────────── */

#define SYN_USE_PID            0
#define SYN_USE_STEPPER        0
#define SYN_USE_SERVO          0
#define SYN_USE_DC_MOTOR       0
#define SYN_USE_MOTOR_CTRL     0
#define SYN_USE_AUTOTUNE       0
#define SYN_USE_ACTUATOR       0

/* ── DSP / Filters ──────────────────────────────────────────────────────── */

#define SYN_USE_FILTER         0
#define SYN_USE_SIGNAL         0
#define SYN_USE_BIQUAD         0
#define SYN_USE_FFT            0

/* ── State Machines ─────────────────────────────────────────────────────── */

#define SYN_USE_FSM            0
#define SYN_USE_AO             0

/* ── Communication / Protocols ──────────────────────────────────────────── */

#define SYN_USE_COBS           0
#define SYN_USE_MODBUS         0
#define SYN_USE_MQTT           0
#define SYN_USE_HTTP           0
#define SYN_USE_HTTPD          0
#define SYN_USE_WEBSOCKET      0
#define SYN_USE_COAP           0
#define SYN_USE_DNS            0
#define SYN_USE_ROUTER         0
#define SYN_USE_HEARTBEAT      0
#define SYN_USE_TRANSPORT_TCP  0

/* ── Sensor ─────────────────────────────────────────────────────────────── */

#define SYN_USE_SENSOR         0

/* ── Storage ────────────────────────────────────────────────────────────── */

#define SYN_USE_PARAM          0
#define SYN_USE_SETTINGS       0
#define SYN_USE_VFS            0
#define SYN_USE_LFS            0
#define SYN_USE_FAT            0

/* ── Display / UI ───────────────────────────────────────────────────────── */

#define SYN_USE_CANVAS         0
#define SYN_USE_MENU           0
#define SYN_USE_IMGUI          0

/* ── System ─────────────────────────────────────────────────────────────── */

#define SYN_USE_BOOT           0
#define SYN_USE_ERRLOG         0
#define SYN_USE_FAULT          0
#define SYN_USE_HWWDT          0
#define SYN_USE_POWER          0
#define SYN_USE_COREDUMP       0

/* ── DMA / Async / Multicore ────────────────────────────────────────────── */

#define SYN_USE_DMA            0
#define SYN_USE_I2C_ASYNC      0
#define SYN_USE_SPI_ASYNC      0
#define SYN_USE_MULTICORE      0

/* ── Debug ──────────────────────────────────────────────────────────────── */

#define SYN_USE_TRACE          0
#define SYN_USE_PROFILER       0
#define SYN_USE_DATALOG        0

/* ── Utilities ──────────────────────────────────────────────────────────── */

#define SYN_USE_FMT            1   /* needed by LOG */
#define SYN_USE_CBOR           0
#define SYN_USE_JSON           0
#define SYN_USE_SHA256         0
#define SYN_USE_POOL           0
#define SYN_USE_PUBSUB         0
#define SYN_USE_RAMP           0
#define SYN_USE_SCURVE         0
#define SYN_CRC_USE_TABLE      1

#endif /* SYN_CONFIG_H */
