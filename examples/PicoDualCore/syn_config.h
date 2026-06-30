/**
 * @file syn_config.h
 * @brief SyntropicOS config for Pi Pico dual-core AMP test.
 *
 * Enables multicore support plus the minimal modules needed
 * for the cross-core mailbox demonstration.
 */

#ifndef SYN_CONFIG_H
#define SYN_CONFIG_H

/* ── Multicore (AMP) ────────────────────────────────────────────────────── */

#define SYN_USE_MULTICORE      1
#define SYN_SPINLOCK_COUNT     4

/* ── Drivers ────────────────────────────────────────────────────────────── */

#define SYN_USE_GPIO           1
#define SYN_USE_UART           1
#define SYN_USE_ADC            0
#define SYN_USE_EXTI           0
#define SYN_USE_SD             0
#define SYN_USE_RTC            0
#define SYN_USE_DAC            0
#define SYN_USE_ONEWIRE        0
#define SYN_USE_CAN            0

/* ── Multitasking ───────────────────────────────────────────────────────── */

#define SYN_USE_PT             1
#define SYN_USE_SCHED          1
#define SYN_USE_TIMER          1
#define SYN_USE_EVENT          1
#define SYN_USE_WATCHDOG       0
#define SYN_USE_SEQUENCER      0
#define SYN_USE_WORKQUEUE      0
#define SYN_USE_TICKLESS       0
#define SYN_USE_AO             0

/* ── Services ───────────────────────────────────────────────────────────── */

#define SYN_USE_LOG            1
#define SYN_USE_CLI            1

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
#define SYN_USE_ACTUATOR       0

/* ── DSP / Filters ──────────────────────────────────────────────────────── */

#define SYN_USE_FILTER         0
#define SYN_USE_SIGNAL         0
#define SYN_USE_BIQUAD         0
#define SYN_USE_FFT            0

/* ── State Machines ─────────────────────────────────────────────────────── */

#define SYN_USE_FSM            0

/* ── Communication / Protocols ──────────────────────────────────────────── */

#define SYN_USE_COBS           0
#define SYN_USE_MODBUS         0
#define SYN_USE_COAP           0
#define SYN_USE_ROUTER         0
#define SYN_USE_HEARTBEAT      0

/* ── Sensor ─────────────────────────────────────────────────────────────── */

#define SYN_USE_SENSOR         0

/* ── Storage ────────────────────────────────────────────────────────────── */

#define SYN_USE_PARAM          0
#define SYN_USE_VFS            0

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

/* ── Debug ──────────────────────────────────────────────────────────────── */

#define SYN_USE_TRACE          0
#define SYN_USE_PROFILER       0

/* ── Utilities ──────────────────────────────────────────────────────────── */

#define SYN_USE_FMT            1   /* needed by LOG */
#define SYN_USE_CBOR           0
#define SYN_USE_SHA256         0
#define SYN_USE_POOL           0
#define SYN_USE_RAMP           0
#define SYN_CRC_USE_TABLE      1

#endif /* SYN_CONFIG_H */
