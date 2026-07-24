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
void run_spsc_queue_tests(void);
void run_slab_tests(void);
void run_event_flags_tests(void);
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
void run_timer_wheel_tests(void);
void run_netbuf_tests(void);
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
void run_sensor_fusion_tests(void);
void run_adc_tests(void);
void run_log_tests(void);
void run_cli_tests(void);
void run_param_tests(void);
void run_trace_tests(void);
void run_profiler_tests(void);
void run_task_profile_tests(void);
void run_dds_tests(void);
void run_boot_tests(void);
void run_errlog_tests(void);
void run_power_tests(void);
void run_modbus_tests(void);
void run_modbus_master_tests(void);
void run_canopen_tests(void);
void run_cia402_tests(void);
void run_lss_tests(void);
void run_cia401_tests(void);
void run_canopen_mgr_tests(void);
void run_dmx512_tests(void);
void run_j1939_tests(void);
void run_n2k_tests(void);
void run_canvas_tests(void);
void run_menu_tests(void);
void run_imgui_tests(void);
void run_can_tests(void);
void run_router_tests(void);
void run_heartbeat_tests(void);
void run_protothread_tests(void);
void run_sched_tests(void);
void run_exti_tests(void);
void run_dma_tests(void);
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
void run_lfs_tests(void);
void run_coap_tests(void);
void run_biquad_tests(void);
void run_fft_tests(void);
void run_filter_design_tests(void);
void run_foc_tests(void);
void run_foc_observer_tests(void);
void run_fault_tests(void);
void run_autotune_tests(void);
void run_metrics_tests(void);
void run_random_tests(void);

void run_gpio_tests(void);
void run_uart_tests(void);
void run_sha256_tests(void);
void run_pool_tests(void);
void run_coredump_tests(void);
void run_tickless_tests(void);
void run_dma_tests(void);
void run_i2c_async_tests(void);
void run_spi_async_tests(void);
void run_fwupdate_hmac_tests(void);
void run_timer_expiry_tests(void);
void run_multicore_tests(void);
void run_crypto_tests(void);
void run_wg_tests(void);
void run_sntp_tests(void);
void run_control_stats_tests(void);
void run_settings_tests(void);
void run_stream_tests(void);
void run_matrix_tests(void);
void run_kalman_tests(void);
void run_foc_tests(void);
void run_modbus_tcp_tests(void);
void run_nmea_tests(void);
void run_interpolator_tests(void);

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

    /* Modbus & CANopen */
    run_modbus_tests();
    run_modbus_master_tests();
    run_canopen_tests();
    run_cia402_tests();
    run_lss_tests();
    run_cia401_tests();
    run_canopen_mgr_tests();
    run_dmx512_tests();
    run_j1939_tests();
    run_motor_ctrl_tests();
    run_actuator_tests();

    /* Motor */
    run_servo_tests();
    run_dc_motor_tests();

    /* Drivers */
    run_adc_tests();
    run_sensor_tests();
    run_exti_tests();
    run_can_tests();
    run_gpio_tests();
    run_uart_tests();

    /* Protocol */
    run_cobs_tests();

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
    run_lfs_tests();
    run_coap_tests();
    run_biquad_tests();
    run_fft_tests();
    run_fault_tests();
    run_autotune_tests();


    /* OTA / Firmware Update */
    run_fwupdate_tests();

    /* New modules */
    run_sha256_tests();
    run_pool_tests();
    run_coredump_tests();
    run_tickless_tests();

    /* New features: DMA, Async I2C/SPI, HMAC FW, Timer Expiry */
    run_dma_tests();
    run_i2c_async_tests();
    run_spi_async_tests();
    run_fwupdate_hmac_tests();
    run_timer_expiry_tests();

    /* Multicore (AMP) */
    run_multicore_tests();

    /* Crypto & WireGuard */
    run_crypto_tests();
    run_wg_tests();

    /* Networking */
    run_sntp_tests();

    /* Control */
    run_control_stats_tests();

    /* Storage */
    run_settings_tests();

    /* Streams */
    run_stream_tests();

    /* Fixed-point math & matrix */
    run_matrix_tests();

    /* Kalman filter */
    run_kalman_tests();

    /* FOC transforms & Observer */
    run_foc_tests();
    run_foc_observer_tests();

    /* IMU Sensor Fusion */
    run_sensor_fusion_tests();

    /* Filter Design Generator */
    run_filter_design_tests();

    /* Modbus Master & TCP */
    run_modbus_master_tests();
    run_modbus_tcp_tests();

    /* NMEA 0183 Navigation */
    run_nmea_tests();

    /* Multi-Axis Motion Interpolator */
    run_interpolator_tests();

    /* System Metrics */
    run_metrics_tests();

    /* Random Utilities */
    run_random_tests();

    /* DMA Driver Engine */
    run_dma_tests();

    /* SAE J1939 Heavy Duty Vehicle Protocol */
    run_j1939_tests();

    /* NMEA 2000 Marine CAN Protocol */
    run_n2k_tests();

    /* Task Profiler & DDS Synthesizer */
    run_task_profile_tests();
    run_dds_tests();

    /* OS Kernel Primitives */
    run_netbuf_tests();
    run_timer_wheel_tests();
    run_spsc_queue_tests();
    run_slab_tests();
    run_event_flags_tests();

    return UNITY_END();
}
