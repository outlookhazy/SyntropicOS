/**
 * @file syntropic.h
 * @brief SyntropicOS umbrella header.
 *
 * Include this single header to pull in all enabled SyntropicOS modules.
 * Module selection is driven by syn_config.h (see syn_config_template.h).
 *
 * You can also include individual module headers directly for finer
 * control over what gets pulled in.
 *
 * @defgroup syn_core Core & Utilities
 * @brief Core types, status codes, and utility helpers.
 *
 * @defgroup syn_sched Scheduling
 * @brief Protothreads, task management, cooperative scheduler, timers, and mailboxes.
 *
 * @defgroup syn_drivers Drivers
 * @brief Hardware peripheral driver interfaces (GPIO, UART, I2C, SPI, EXTI, CAN, RTC).
 *
 * @defgroup syn_io Input / Output
 * @brief LED, button, and encoder interfaces.
 *
 * @defgroup syn_motor Motor & Control
 * @brief DC motor, servo, stepper, actuator, and PID control algorithms.
 *
 * @defgroup syn_display Display & UI
 * @brief Canvas graphics primitives and immediate-mode GUI components.
 *
 * @defgroup syn_net Networking
 * @brief Packet router, transports, DNS, HTTP, WebSocket, CoAP, and MQTT protocols.
 *
 * @defgroup syn_protocol Protocols
 * @brief COBS framing, Modbus, CBOR, and JSON serialization.
 *
 * @defgroup syn_storage Storage
 * @brief Virtual Filesystem (VFS) abstraction, settings, and logging.
 *
 * @defgroup syn_system System
 * @brief Bootloader integration, power management, watchdog, faults, and system templates.
 *
 * @defgroup syn_dsp DSP & Filters
 * @brief Signal filters, FFT, and data structures.
 *
 * @defgroup syn_debug Debug & Diagnostics
 * @brief Logging, profiler, runtime tracing, and CLI commands.
 */

#ifndef SYN_H
#define SYN_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Try to include user config; fall back to defaults ──────────────────── */
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

/* ── Common (always included) ───────────────────────────────────────────── */
#include "common/syn_defs.h"
#include "common/syn_compiler.h"

/* ── Utilities (always included — zero cost if unused) ──────────────────── */
#include "util/syn_assert.h"
#include "util/syn_bits.h"
#include "util/syn_ringbuf.h"

/* ── Drivers (conditional on config) ────────────────────────────────────── */

#if !defined(SYN_USE_GPIO) || SYN_USE_GPIO
  #include "drivers/syn_gpio.h"
#endif

#if !defined(SYN_USE_UART) || SYN_USE_UART
  #include "drivers/syn_uart.h"
#endif

/* ── Protothreads & Scheduler (conditional on config) ───────────────────── */

#if !defined(SYN_USE_PT) || SYN_USE_PT
  #include "pt/syn_pt.h"
  #include "pt/syn_pt_sem.h"
#endif

#if !defined(SYN_USE_SCHED) || SYN_USE_SCHED
  #include "sched/syn_sched.h"
#endif

#if !defined(SYN_USE_TIMER) || SYN_USE_TIMER
  #include "sched/syn_timer.h"
#endif

#if !defined(SYN_USE_EVENT) || SYN_USE_EVENT
  #include "util/syn_event.h"
#endif

/* ── Logging & CLI (conditional on config) ──────────────────────────────── */

#if !defined(SYN_USE_LOG) || SYN_USE_LOG
  #include "log/syn_log.h"
#endif

#if !defined(SYN_USE_CLI) || SYN_USE_CLI
  #include "cli/syn_cli.h"
#endif

/* ── Utilities (always included — zero cost if unused) ──────────────────── */
#include "util/syn_timeout.h"
#include "util/syn_crc.h"

/* ── Input / Output (conditional on config) ─────────────────────────────── */

#if !defined(SYN_USE_BUTTON) || SYN_USE_BUTTON
  #include "input/syn_button.h"
#endif

#if !defined(SYN_USE_LED) || SYN_USE_LED
  #include "output/syn_led.h"
#endif

#if !defined(SYN_USE_SOFT_PWM) || SYN_USE_SOFT_PWM
  #include "output/syn_soft_pwm.h"
#endif

/* ── State Machine & DSP (conditional on config) ────────────────────────── */

#if !defined(SYN_USE_FSM) || SYN_USE_FSM
  #include "util/syn_fsm.h"
#endif

#if !defined(SYN_USE_FILTER) || SYN_USE_FILTER
  #include "dsp/syn_filter.h"
#endif

/* ── Control (conditional on config) ────────────────────────────────────── */

#if !defined(SYN_USE_PID) || SYN_USE_PID
  #include "control/syn_pid.h"
#endif

/* ── Motor (conditional on config) ──────────────────────────────────────── */

#if !defined(SYN_USE_STEPPER) || SYN_USE_STEPPER
  #include "motor/syn_stepper.h"
#endif

#if !defined(SYN_USE_SERVO) || SYN_USE_SERVO
  #include "motor/syn_servo.h"
#endif

#if !defined(SYN_USE_DC_MOTOR) || SYN_USE_DC_MOTOR
  #include "motor/syn_dc_motor.h"
#endif

#if !defined(SYN_USE_MOTOR_CTRL) || SYN_USE_MOTOR_CTRL
  #include "motor/syn_motor_ctrl.h"
#endif

/* ── Protocol (conditional on config) ───────────────────────────────────── */

#if !defined(SYN_USE_COBS) || SYN_USE_COBS
  #include "proto/syn_cobs.h"
#endif

/* ── Sensor (conditional on config) ─────────────────────────────────────── */

#if !defined(SYN_USE_SENSOR) || SYN_USE_SENSOR
  #include "sensor/syn_sensor.h"
#endif

/* ── Watchdog (conditional on config) ───────────────────────────────────── */

#if !defined(SYN_USE_WATCHDOG) || SYN_USE_WATCHDOG
  #include "sched/syn_watchdog.h"
#endif

/* ── Storage (conditional on config) ────────────────────────────────────── */

#if !defined(SYN_USE_PARAM) || SYN_USE_PARAM
  #include "storage/syn_param.h"
#endif

#if !defined(SYN_USE_VFS) || SYN_USE_VFS
  #include "storage/syn_vfs.h"
  #include "storage/syn_lfs.h"
#endif

/* ── Utilities (always included — header-only, zero cost) ───────────────── */
#include "util/syn_hysteresis.h"
#include "util/syn_lut.h"
#include "util/syn_qmath.h"
#include "util/syn_rate_limit.h"
#include "util/syn_pingpong.h"
#include "util/syn_change_filter.h"

/* ── Input (conditional on config) ──────────────────────────────────────── */

#if !defined(SYN_USE_ENCODER) || SYN_USE_ENCODER
  #include "input/syn_encoder.h"
#endif

/* ── Drivers (conditional on config) ────────────────────────────────────── */

#if !defined(SYN_USE_ADC) || SYN_USE_ADC
  #include "drivers/syn_adc.h"
#endif

#include "drivers/syn_i2c_dev.h"
#include "drivers/syn_spi_dev.h"

#if !defined(SYN_USE_EXTI) || SYN_USE_EXTI
  #include "drivers/syn_exti.h"
#endif

#if !defined(SYN_USE_SD) || SYN_USE_SD
  #include "drivers/syn_sd.h"
#endif

#if !defined(SYN_USE_RTC) || SYN_USE_RTC
  #include "drivers/syn_rtc.h"
#endif

#if !defined(SYN_USE_DAC) || SYN_USE_DAC
  #include "drivers/syn_dac.h"
#endif

#if !defined(SYN_USE_ONEWIRE) || SYN_USE_ONEWIRE
  #include "drivers/syn_soft_onewire.h"
#endif

/* ── Formatting (conditional on config) ─────────────────────────────────── */

#if !defined(SYN_USE_FMT) || SYN_USE_FMT
  #include "util/syn_fmt.h"
#endif

#if !defined(SYN_USE_CBOR) || SYN_USE_CBOR
  #include "util/syn_cbor_write.h"
  #include "util/syn_cbor_read.h"
#endif

/* ── Scheduler extensions (conditional) ─────────────────────────────────── */

#if !defined(SYN_USE_SEQUENCER) || SYN_USE_SEQUENCER
  #include "sched/syn_sequencer.h"
#endif

#if !defined(SYN_USE_WORKQUEUE) || SYN_USE_WORKQUEUE
  #include "sched/syn_workqueue.h"
#endif

#include "sched/syn_mailbox.h"

#if !defined(SYN_USE_AO) || SYN_USE_AO
  #include "sched/syn_ao.h"
#endif

/* ── Protocol extensions (conditional) ──────────────────────────────────── */

#if !defined(SYN_USE_MODBUS) || SYN_USE_MODBUS
  #include "proto/syn_modbus.h"
#endif

#if !defined(SYN_USE_COAP) || SYN_USE_COAP
  #include "net/syn_coap.h"
#endif

/* ── Debug (conditional on config) ──────────────────────────────────────── */

#if !defined(SYN_USE_TRACE) || SYN_USE_TRACE
  #include "debug/syn_trace.h"
#endif

#if !defined(SYN_USE_PROFILER) || SYN_USE_PROFILER
  #include "debug/syn_profiler.h"
#endif

/* ── DSP extensions (conditional) ───────────────────────────────────────── */

#if !defined(SYN_USE_SIGNAL) || SYN_USE_SIGNAL
  #include "dsp/syn_signal.h"
#endif

#if !defined(SYN_USE_BIQUAD) || SYN_USE_BIQUAD
  #include "dsp/syn_biquad.h"
#endif

#if !defined(SYN_USE_FFT) || SYN_USE_FFT
  #include "dsp/syn_fft.h"
#endif

/* ── System (conditional on config) ─────────────────────────────────────── */

#if !defined(SYN_USE_BOOT) || SYN_USE_BOOT
  #include "system/syn_boot.h"
#endif

#if !defined(SYN_USE_ERRLOG) || SYN_USE_ERRLOG
  #include "system/syn_errlog.h"
#endif

#if !defined(SYN_USE_FAULT) || SYN_USE_FAULT
  #include "system/syn_fault.h"
#endif

#if !defined(SYN_USE_HWWDT) || SYN_USE_HWWDT
  #include "system/syn_hwwdt.h"
#endif

#include "system/syn_version.h"
#include "system/syn_sleep.h"

/* ── Utilities (conditional) ────────────────────────────────────────────── */

#if !defined(SYN_USE_RAMP) || SYN_USE_RAMP
  #include "util/syn_ramp.h"
#endif

#include "util/syn_pack.h"

/* ── Power management (conditional) ─────────────────────────────────────── */

#if !defined(SYN_USE_POWER) || SYN_USE_POWER
  #include "system/syn_power.h"
#endif

/* ── Actuator (conditional) ─────────────────────────────────────────────── */

#if !defined(SYN_USE_ACTUATOR) || SYN_USE_ACTUATOR
  #include "motor/syn_actuator.h"
#endif

/* ── Display / UI (conditional) ─────────────────────────────────────────── */

#if !defined(SYN_USE_CANVAS) || SYN_USE_CANVAS
  #include "display/syn_canvas.h"
  #include "display/syn_gfx.h"
#endif

#if !defined(SYN_USE_MENU) || SYN_USE_MENU
  #include "ui/syn_menu.h"
#endif

#if !defined(SYN_USE_IMGUI) || SYN_USE_IMGUI
  #include "ui/syn_imgui.h"
#endif

/* ── Networking (conditional) ───────────────────────────────────────────── */

#if !defined(SYN_USE_CAN) || SYN_USE_CAN
  #include "drivers/syn_can.h"
#endif

#include "net/syn_transport.h"

#if !defined(SYN_USE_ROUTER) || SYN_USE_ROUTER
  #include "net/syn_router.h"
#endif

#if !defined(SYN_USE_HEARTBEAT) || SYN_USE_HEARTBEAT
  #include "net/syn_heartbeat.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_H */
