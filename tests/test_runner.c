/**
 * @file test_runner.c
 * @brief Unity test runner — calls all per-module test groups.
 *
 * Build:
 *   make test-unity
 */

#include <stdio.h>
#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

/* ── Unity hooks ────────────────────────────────────────────────────────── */

void setUp(void)    { mock_port_reset(); }
void tearDown(void) { /* nothing */ }

/* ── Per-module test declarations ───────────────────────────────────────── */

/* Each test_*.c file exposes a run_*_tests() function */
void run_ringbuf_tests(void);
void run_crc_tests(void);
void run_pid_tests(void);
void run_hysteresis_tests(void);
void run_lut_tests(void);
void run_fsm_tests(void);
void run_filter_tests(void);
void run_signal_tests(void);
void run_fmt_tests(void);
void run_ramp_tests(void);
void run_pubsub_tests(void);
void run_pack_tests(void);
void run_cobs_tests(void);
void run_timer_tests(void);
void run_datalog_tests(void);
void run_soft_i2c_tests(void);
void run_soft_spi_tests(void);
void run_soft_spi_slave_tests(void);
void run_sd_tests(void);
void run_rtc_tests(void);
void run_hwwdt_tests(void);
void run_soft_onewire_tests(void);
void run_dac_tests(void);
void run_scurve_tests(void);

void run_watchdog_tests(void);
void run_sequencer_tests(void);
void run_workqueue_tests(void);
void run_button_tests(void);
void run_encoder_tests(void);
void run_led_tests(void);
void run_soft_pwm_tests(void);
void run_servo_tests(void);
void run_dc_motor_tests(void);
void run_stepper_tests(void);
void run_motor_ctrl_tests(void);
void run_actuator_tests(void);
void run_sensor_tests(void);
void run_adc_tests(void);
void run_log_tests(void);
void run_cli_tests(void);
void run_param_tests(void);
void run_trace_tests(void);
void run_profiler_tests(void);
void run_boot_tests(void);
void run_errlog_tests(void);
void run_power_tests(void);
void run_modbus_tests(void);
void run_canvas_tests(void);
void run_menu_tests(void);
void run_imgui_tests(void);
void run_can_tests(void);
void run_router_tests(void);
void run_heartbeat_tests(void);
void run_protothread_tests(void);
void run_sched_tests(void);
void run_exti_tests(void);
void run_mailbox_tests(void);
void run_sleep_tests(void);
void run_version_tests(void);
void run_math_tests(void);
void run_pingpong_tests(void);
void run_http_tests(void);
void run_httpd_tests(void);
void run_fwupdate_tests(void);
void run_json_write_tests(void);
void run_json_read_tests(void);
void run_cbor_tests(void);
void run_transport_tcp_tests(void);
void run_websocket_tests(void);
void run_dns_tests(void);
void run_mqtt_tests(void);
void run_ao_tests(void);
void run_vfs_tests(void);
void run_coap_tests(void);
void run_biquad_tests(void);
void run_fft_tests(void);
void run_fault_tests(void);

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();

    /* Utilities */
    run_ringbuf_tests();
    run_crc_tests();
    run_fsm_tests();
    run_fmt_tests();
    run_ramp_tests();
    run_pubsub_tests();
    run_pack_tests();
    run_math_tests();
    run_pingpong_tests();
    run_datalog_tests();
    run_soft_i2c_tests();
    run_soft_spi_tests();
    run_soft_spi_slave_tests();
    run_sd_tests();
    run_rtc_tests();
    run_hwwdt_tests();
    run_soft_onewire_tests();
    run_dac_tests();
    run_scurve_tests();

    /* DSP / Control */
    run_pid_tests();
    run_hysteresis_tests();
    run_lut_tests();
    run_filter_tests();
    run_signal_tests();

    /* Scheduling */
    run_protothread_tests();
    run_sched_tests();
    run_timer_tests();
    run_watchdog_tests();
    run_sequencer_tests();
    run_workqueue_tests();
    run_mailbox_tests();
    run_sleep_tests();

    /* Input / Output */
    run_button_tests();
    run_encoder_tests();
    run_led_tests();
    run_soft_pwm_tests();

    /* Motor */
    run_servo_tests();
    run_dc_motor_tests();
    run_stepper_tests();
    run_motor_ctrl_tests();
    run_actuator_tests();

    /* Drivers */
    run_adc_tests();
    run_sensor_tests();
    run_exti_tests();
    run_can_tests();

    /* Protocol */
    run_cobs_tests();
    run_modbus_tests();

    /* Storage / Config */
    run_param_tests();

    /* Debug */
    run_trace_tests();
    run_profiler_tests();
    run_log_tests();
    run_cli_tests();

    /* System */
    run_boot_tests();
    run_errlog_tests();
    run_power_tests();
    run_version_tests();

    /* Display / UI */
    run_canvas_tests();
    run_menu_tests();
    run_imgui_tests();

    /* Networking */
    run_router_tests();
    run_heartbeat_tests();
    run_http_tests();
    run_httpd_tests();
    run_json_write_tests();
    run_json_read_tests();
    run_cbor_tests();
    run_transport_tcp_tests();
    run_websocket_tests();
    run_dns_tests();
    run_mqtt_tests();
    run_ao_tests();
    run_vfs_tests();
    run_coap_tests();
    run_biquad_tests();
    run_fft_tests();
    run_fault_tests();

    /* OTA / Firmware Update */
    run_fwupdate_tests();

    return UNITY_END();
}
