

# File syntropic.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**syntropic.h**](syntropic_8h.md)

[Go to the documentation of this file](syntropic_8h.md)


```C++

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

#include "common/syn_compiler.h"
#include "common/syn_defs.h"

/* ── Utilities — always included (header-only, zero cost) ───────────────── */

#include "util/syn_assert.h"
#include "util/syn_bits.h"
#include "util/syn_change_filter.h"
#include "util/syn_crc.h"
#include "util/syn_hysteresis.h"
#include "util/syn_lut.h"
#include "util/syn_pack.h"
#include "util/syn_pingpong.h"
#include "util/syn_qmath.h"
#include "util/syn_rate_limit.h"
#include "util/syn_ringbuf.h"
#include "util/syn_str.h"
#include "util/syn_stream.h"
#include "util/syn_timeout.h"

/* ── Utilities — conditional ────────────────────────────────────────────── */

#if !defined(SYN_USE_EVENT) || SYN_USE_EVENT
#include "util/syn_event.h"
#endif

#if !defined(SYN_USE_FSM) || SYN_USE_FSM
#include "util/syn_fsm.h"
#endif

#if !defined(SYN_USE_FMT) || SYN_USE_FMT
#include "util/syn_fmt.h"
#endif

#if !defined(SYN_USE_CBOR) || SYN_USE_CBOR
#include "util/syn_cbor_read.h"
#include "util/syn_cbor_write.h"
#endif

#if !defined(SYN_USE_RAMP) || SYN_USE_RAMP
#include "util/syn_ramp.h"
#endif

#if !defined(SYN_USE_SHA256) || SYN_USE_SHA256
#include "util/syn_aes128.h"
#include "util/syn_hmac.h"
#include "util/syn_sha256.h"
#endif

#if !defined(SYN_USE_POOL) || SYN_USE_POOL
#include "util/syn_pool.h"
#endif

#if !defined(SYN_USE_LZ4) || SYN_USE_LZ4
#include "util/syn_lz4.h"
#endif

/* ── Drivers ────────────────────────────────────────────────────────────── */

#if !defined(SYN_USE_GPIO) || SYN_USE_GPIO
#include "drivers/syn_gpio.h"
#endif

#if !defined(SYN_USE_UART) || SYN_USE_UART
#include "drivers/syn_uart.h"
#endif

#if !defined(SYN_USE_ADC) || SYN_USE_ADC
#include "drivers/syn_adc.h"
#endif

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

#if !defined(SYN_USE_DMA) || SYN_USE_DMA
#include "drivers/syn_dma.h"
#endif

#if !defined(SYN_USE_ADC) || SYN_USE_ADC
#include "drivers/syn_adc.h"
#endif

#if !defined(SYN_USE_I2C) || SYN_USE_I2C
#include "drivers/syn_i2c.h"
#include "drivers/syn_i2c_queue.h"
#endif

#if !defined(SYN_USE_SPI) || SYN_USE_SPI
#include "drivers/syn_spi.h"
#include "drivers/syn_spi_queue.h"
#endif

#if !defined(SYN_USE_ONEWIRE) || SYN_USE_ONEWIRE
#include "drivers/syn_soft_onewire.h"
#endif

#if !defined(SYN_USE_SHIFTREG) || SYN_USE_SHIFTREG
#include "drivers/syn_shiftreg.h"
#endif

#if !defined(SYN_USE_IOEXP) || SYN_USE_IOEXP
#include "drivers/syn_ioexp.h"
#endif

#if !defined(SYN_USE_RFID) || SYN_USE_RFID
#include "drivers/syn_rfid.h"
#endif

#if !defined(SYN_USE_CAN) || SYN_USE_CAN
#include "drivers/syn_can.h"
#endif

#if !defined(SYN_USE_HPCLOCK) || SYN_USE_HPCLOCK
#include "drivers/syn_hpclock.h"
#endif

#if !defined(SYN_USE_TIMESYNC) || SYN_USE_TIMESYNC
#include "drivers/syn_timesync.h"
#endif

#if !defined(SYN_USE_GEO) || SYN_USE_GEO
#include "util/syn_geo.h"
#endif

/* ── Protothreads & Scheduler ───────────────────────────────────────────── */

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

#if !defined(SYN_USE_WATCHDOG) || SYN_USE_WATCHDOG
#include "sched/syn_watchdog.h"
#endif

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

#include "sched/syn_event_flags.h"
#include "sched/syn_timer_wheel.h"
#include "util/syn_netbuf.h"
#include "util/syn_slab.h"
#include "util/syn_spsc_queue.h"

/* ── Logging & CLI ──────────────────────────────────────────────────────── */

#if !defined(SYN_USE_LOG) || SYN_USE_LOG
#include "log/syn_log.h"
#endif

#if !defined(SYN_USE_CLI) || SYN_USE_CLI
#include "cli/syn_cli.h"
#endif

/* ── Input / Output ─────────────────────────────────────────────────────── */

#if !defined(SYN_USE_BUTTON) || SYN_USE_BUTTON
#include "input/syn_button.h"
#endif

#if !defined(SYN_USE_ENCODER) || SYN_USE_ENCODER
#include "input/syn_encoder.h"
#endif

#if !defined(SYN_USE_KEYPAD) || SYN_USE_KEYPAD
#include "input/syn_keypad.h"
#endif

#if !defined(SYN_USE_TOUCH) || SYN_USE_TOUCH
#include "input/syn_touch.h"
#endif

#if !defined(SYN_USE_DIPSWITCH) || SYN_USE_DIPSWITCH
#include "input/syn_dipswitch.h"
#endif

#if !defined(SYN_USE_JOYSTICK) || SYN_USE_JOYSTICK
#include "input/syn_joystick.h"
#endif

#if !defined(SYN_USE_LED) || SYN_USE_LED
#include "output/syn_led.h"
#endif

#if !defined(SYN_USE_SMARTLED) || SYN_USE_SMARTLED
#include "output/syn_smartled.h"
#endif

#if !defined(SYN_USE_BUZZER) || SYN_USE_BUZZER
#include "output/syn_buzzer.h"
#endif

#if !defined(SYN_USE_SOFT_PWM) || SYN_USE_SOFT_PWM
#include "output/syn_soft_pwm.h"
#endif

/* ── Sensor ─────────────────────────────────────────────────────────────── */

#if !defined(SYN_USE_SENSOR) || SYN_USE_SENSOR
#include "sensor/syn_sensor.h"
#endif

#if !defined(SYN_USE_POWERMON) || SYN_USE_POWERMON
#include "sensor/syn_powermon.h"
#endif

#if !defined(SYN_USE_CLIMATE) || SYN_USE_CLIMATE
#include "sensor/syn_climate.h"
#endif

#if !defined(SYN_USE_DISTANCE) || SYN_USE_DISTANCE
#include "sensor/syn_distance.h"
#endif

#if !defined(SYN_USE_SCALE) || SYN_USE_SCALE
#include "sensor/syn_scale.h"
#endif

#if !defined(SYN_USE_LUX) || SYN_USE_LUX
#include "sensor/syn_lux.h"
#endif

#if !defined(SYN_USE_BIOMETRIC) || SYN_USE_BIOMETRIC
#include "sensor/syn_biometric.h"
#endif

/* ── DSP & Filters ──────────────────────────────────────────────────────── */

#if !defined(SYN_USE_FILTER) || SYN_USE_FILTER
#include "dsp/syn_filter.h"
#endif

#if !defined(SYN_USE_SIGNAL) || SYN_USE_SIGNAL
#include "dsp/syn_signal.h"
#endif

#if !defined(SYN_USE_BIQUAD) || SYN_USE_BIQUAD
#include "dsp/syn_biquad.h"
#endif

#if !defined(SYN_USE_FFT) || SYN_USE_FFT
#include "dsp/syn_fft.h"
#endif

#if !defined(SYN_USE_DDS) || SYN_USE_DDS
#include "dsp/syn_dds.h"
#endif

#if !defined(SYN_USE_GOERTZEL) || SYN_USE_GOERTZEL
#include "dsp/syn_goertzel.h"
#endif

/* ── Audio & Codecs ─────────────────────────────────────────────────────── */

#if !defined(SYN_USE_ADPCM) || SYN_USE_ADPCM
#include "audio/syn_adpcm.h"
#endif

#if !defined(SYN_USE_SBC) || SYN_USE_SBC
#include "audio/syn_sbc.h"
#endif

#if !defined(SYN_USE_AUDIO) || SYN_USE_AUDIO
#include "audio/syn_audio.h"
#include "audio/syn_audio_mixer.h"
#include "audio/syn_wav.h"
#endif

/* ── Control ────────────────────────────────────────────────────────────── */

#if !defined(SYN_USE_PID) || SYN_USE_PID
#include "control/syn_pid.h"
#endif

/* ── Motor & Actuator ───────────────────────────────────────────────────── */

#if !defined(SYN_USE_STEPPER) || SYN_USE_STEPPER
#include "motor/syn_stepper.h"
#endif

#if !defined(SYN_USE_SERVO) || SYN_USE_SERVO
#include "motor/syn_servo.h"
#endif

#if !defined(SYN_USE_DC_MOTOR) || SYN_USE_DC_MOTOR
#include "motor/syn_dc_motor.h"
#endif

#if !defined(SYN_USE_BLDC_6STEP) || SYN_USE_BLDC_6STEP
#include "motor/syn_bldc_6step.h"
#endif

#if !defined(SYN_USE_MOTOR_CTRL) || SYN_USE_MOTOR_CTRL
#include "motor/syn_motor_ctrl.h"
#endif

#if !defined(SYN_USE_ACTUATOR) || SYN_USE_ACTUATOR
#include "motor/syn_actuator.h"
#endif

/* ── Protocols ──────────────────────────────────────────────────────────── */

#if !defined(SYN_USE_AT_PARSER) || SYN_USE_AT_PARSER
#include "proto/syn_at_parser.h"
#endif

#if !defined(SYN_USE_COBS) || SYN_USE_COBS
#include "proto/syn_cobs.h"
#endif

#if !defined(SYN_USE_LIN) || SYN_USE_LIN
#include "proto/syn_lin.h"
#endif

#if !defined(SYN_USE_DALI) || SYN_USE_DALI
#include "proto/syn_dali.h"
#endif

#if !defined(SYN_USE_BACNET) || SYN_USE_BACNET
#include "proto/syn_bacnet.h"
#endif

#if !defined(SYN_USE_SMBUS) || SYN_USE_SMBUS
#include "proto/syn_smbus.h"
#endif

#if !defined(SYN_USE_PMBUS) || SYN_USE_PMBUS
#include "proto/syn_pmbus.h"
#endif

#if !defined(SYN_USE_MBUS) || SYN_USE_MBUS
#include "proto/syn_mbus.h"
#endif

#if !defined(SYN_USE_MODBUS) || SYN_USE_MODBUS
#include "proto/syn_modbus.h"
#endif

#if !defined(SYN_USE_SBUS) || SYN_USE_SBUS
#include "proto/syn_sbus.h"
#endif

#if !defined(SYN_USE_DSHOT) || SYN_USE_DSHOT
#include "output/syn_dshot.h"
#endif

#if !defined(SYN_USE_PPM) || SYN_USE_PPM
#include "input/syn_ppm.h"
#endif

#if !defined(SYN_USE_CRSF) || SYN_USE_CRSF
#include "proto/syn_crsf.h"
#endif

#if !defined(SYN_USE_IBUS) || SYN_USE_IBUS
#include "proto/syn_ibus.h"
#endif

#if !defined(SYN_USE_RC_CURVE) || SYN_USE_RC_CURVE
#include "control/syn_rc_curve.h"
#endif

#if !defined(SYN_USE_RC_FAILSAFE) || SYN_USE_RC_FAILSAFE
#include "control/syn_rc_failsafe.h"
#endif

#if !defined(SYN_USE_DSHOT_TELEMETRY) || SYN_USE_DSHOT_TELEMETRY
#include "output/syn_dshot_telemetry.h"
#endif

#if !defined(SYN_USE_FLIGHT_PID) || SYN_USE_FLIGHT_PID
#include "control/syn_flight_pid.h"
#endif

#if !defined(SYN_USE_MSP) || SYN_USE_MSP
#include "proto/syn_msp.h"
#endif

#if !defined(SYN_USE_MAVLINK) || SYN_USE_MAVLINK
#include "proto/syn_mavlink.h"
#endif

#if !defined(SYN_USE_BLACKBOX) || SYN_USE_BLACKBOX
#include "log/syn_blackbox.h"
#endif

#if !defined(SYN_USE_USB_CDC) || SYN_USE_USB_CDC
#include "drivers/syn_usb_cdc.h"
#endif

#if !defined(SYN_USE_ETH) || SYN_USE_ETH
#include "net/syn_eth.h"
#endif

#if !defined(SYN_USE_DHCP) || SYN_USE_DHCP
#include "net/syn_dhcp.h"
#endif

#if !defined(SYN_USE_ICMP) || SYN_USE_ICMP
#include "net/syn_icmp.h"
#endif

#if !defined(SYN_USE_AUTOIP) || SYN_USE_AUTOIP
#include "net/syn_autoip.h"
#endif

#if !defined(SYN_USE_NETCFG) || SYN_USE_NETCFG
#include "net/syn_netcfg.h"
#endif

#if !defined(SYN_USE_IGMP) || SYN_USE_IGMP
#include "net/syn_igmp.h"
#endif

#if !defined(SYN_USE_NMEA) || SYN_USE_NMEA
#include "proto/syn_nmea.h"
#endif

#if !defined(SYN_USE_J1939) || SYN_USE_J1939
#include "proto/syn_j1939.h"
#endif

#if !defined(SYN_USE_GBT27930) || SYN_USE_GBT27930
#include "proto/syn_gbt27930.h"
#endif

#if !defined(SYN_USE_CANNM) || SYN_USE_CANNM
#include "proto/syn_cannm.h"
#endif

#if !defined(SYN_USE_N2K) || SYN_USE_N2K
#include "proto/syn_n2k.h"
#endif

#include "proto/syn_cia303.h"
#include "proto/syn_cia418.h"

#if !defined(SYN_USE_DMX512) || SYN_USE_DMX512
#include "proto/syn_dmx512.h"
#endif

#if !defined(SYN_USE_ISOTP) || SYN_USE_ISOTP
#include "proto/syn_isotp.h"
#endif

#if !defined(SYN_USE_CJT188) || SYN_USE_CJT188
#include "proto/syn_cjt188.h"
#endif

#if !defined(SYN_USE_ETHERCAT) || SYN_USE_ETHERCAT
#include "proto/syn_ethercat.h"
#endif

#if !defined(SYN_USE_CCP) || SYN_USE_CCP
#include "proto/syn_ccp.h"
#endif

#if !defined(SYN_USE_XCP) || SYN_USE_XCP
#include "proto/syn_xcp.h"
#endif

#if !defined(SYN_USE_UDS) || SYN_USE_UDS
#include "proto/syn_uds.h"
#endif

#if !defined(SYN_USE_DEVICENET) || SYN_USE_DEVICENET
#include "proto/syn_devicenet.h"
#endif

/* ── Storage ────────────────────────────────────────────────────────────── */

#if !defined(SYN_USE_PARAM) || SYN_USE_PARAM
#include "storage/syn_param.h"
#endif

#if !defined(SYN_USE_VFS) || SYN_USE_VFS
#include "storage/syn_lfs.h"
#include "storage/syn_vfs.h"
#endif

/* ── Display & UI ───────────────────────────────────────────────────────── */

#if !defined(SYN_USE_CANVAS) || SYN_USE_CANVAS
#include "display/syn_canvas.h"
#include "display/syn_gfx.h"
#endif

#if !defined(SYN_USE_CHARLCD) || SYN_USE_CHARLCD
#include "display/syn_charlcd.h"
#endif

#if !defined(SYN_USE_OLED) || SYN_USE_OLED
#include "display/syn_oled.h"
#endif

#if !defined(SYN_USE_SEG7) || SYN_USE_SEG7
#include "display/syn_seg7.h"
#endif

#if !defined(SYN_USE_MENU) || SYN_USE_MENU
#include "ui/syn_menu.h"
#endif

#if !defined(SYN_USE_IMGUI) || SYN_USE_IMGUI
#include "ui/syn_imgui.h"
#endif

/* ── Networking ─────────────────────────────────────────────────────────── */

#include "net/syn_transport.h"

#if !defined(SYN_USE_ROUTER) || SYN_USE_ROUTER
#include "net/syn_router.h"
#endif

#if !defined(SYN_USE_HEARTBEAT) || SYN_USE_HEARTBEAT
#include "net/syn_heartbeat.h"
#endif

#if !defined(SYN_USE_COAP) || SYN_USE_COAP
#include "net/syn_coap.h"
#endif

#if !defined(SYN_USE_SNTP) || SYN_USE_SNTP
#include "net/syn_sntp.h"
#endif

#if !defined(SYN_USE_WG) || SYN_USE_WG
#include "net/syn_wg.h"
#endif

/* ── Cryptography ───────────────────────────────────────────────────────── */

#if !defined(SYN_USE_BLAKE2S) || SYN_USE_BLAKE2S
#include "crypto/syn_blake2s.h"
#endif

#if !defined(SYN_USE_CHACHA20POLY1305) || SYN_USE_CHACHA20POLY1305
#include "crypto/syn_chacha20poly1305.h"
#endif

#if !defined(SYN_USE_X25519) || SYN_USE_X25519
#include "crypto/syn_x25519.h"
#endif

/* ── Debug & Diagnostics ────────────────────────────────────────────────── */

#if !defined(SYN_USE_TRACE) || SYN_USE_TRACE
#include "debug/syn_trace.h"
#endif

#if !defined(SYN_USE_PROFILER) || SYN_USE_PROFILER
#include "debug/syn_profiler.h"
#endif

/* ── System ─────────────────────────────────────────────────────────────── */

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

#if defined(SYN_USE_COREDUMP) && SYN_USE_COREDUMP
#include "system/syn_coredump.h"
#endif

#if !defined(SYN_USE_POWER) || SYN_USE_POWER
#include "system/syn_power.h"
#endif

#if !defined(SYN_USE_WASM) || SYN_USE_WASM
#include "vm/syn_wasm.h"
#endif

#include "system/syn_sleep.h"
#include "system/syn_version.h"

#if !defined(SYN_USE_USB_MIDI) || SYN_USE_USB_MIDI
#include "drivers/syn_usb_midi.h"
#endif

#if !defined(SYN_USE_USB_MSC) || SYN_USE_USB_MSC
#include "drivers/syn_usb_msc.h"
#endif

#if !defined(SYN_USE_MFCC) || SYN_USE_MFCC
#include "dsp/syn_mfcc.h"
#endif

#if !defined(SYN_USE_NTP_SERVER) || SYN_USE_NTP_SERVER
#include "net/syn_ntp_server.h"
#endif

#if !defined(SYN_USE_PROTOBUF) || SYN_USE_PROTOBUF
#include "util/syn_protobuf.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_H */
```


