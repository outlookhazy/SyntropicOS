/**
 * @file test_runner.c
 * @brief Unity test runner — calls all per-module test groups.
 *
 * Build:
 *   make test-unity
 */

#include "mocks/mock_port.h"
#include "syntropic/net/syn_transport_udp.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_metrics.h"
#include "unity/unity.h"

#include <stdio.h>

extern void semihosting_write0(const char *);

/* ── Unity hooks ────────────────────────────────────────────────────────── */

void setUp(void)
{
    mock_port_reset();
    syn_hpclock_msb = 0;
#if SYN_USE_METRICS && SYN_USE_ROUTER
    syn_metrics_set_router(NULL);
#endif
    syn_transport_udp_set_instance(NULL);
#if SYN_CLI_CMD_ERRORS
    syn_cli_set_errlog(NULL);
#endif
#if SYN_CLI_CMD_TASKS
    syn_cli_set_scheduler(NULL);
#endif
}
void tearDown(void)
{ /* nothing */
}

#ifdef SYN_HOST_UNIT_RUNNER
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static bool syn_contains_icase(const char *haystack, const char *needle, size_t needle_len)
{
    if (needle_len == 0U) {
        return true;
    }
    if (haystack == NULL) {
        return false;
    }
    for (const char *h = haystack; *h != '\0'; h++) {
        size_t i = 0U;
        while (i < needle_len && h[i] != '\0' &&
               tolower((unsigned char)h[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == needle_len) {
            return true;
        }
    }
    return false;
}

static bool syn_test_matches_filter(const char *file_name, const char *func_name)
{
    const char *filter = getenv("SYN_TEST_FILTER");
    if (filter == NULL || filter[0] == '\0') {
        return true;
    }
    const char *p = filter;
    while (*p != '\0') {
        while (*p == ',' || *p == ' ' || *p == '\t') {
            p++;
        }
        const char *start = p;
        while (*p != '\0' && *p != ',' && *p != ' ' && *p != '\t') {
            p++;
        }
        size_t len = (size_t)(p - start);
        if (len > 0U && (syn_contains_icase(file_name, start, len) ||
                         syn_contains_icase(func_name, start, len))) {
            return true;
        }
    }
    return false;
}

void syn_unit_run_test(void (*func)(void), const char *func_name, const char *file_name,
                       int line_num)
{
    if (!syn_test_matches_filter(file_name, func_name)) {
        return;
    }
    UnitySetTestFile(file_name);
    Unity.CurrentTestName = func_name;
    Unity.CurrentTestLineNumber = (UNITY_LINE_TYPE)line_num;
    Unity.NumberOfTests++;
#ifndef UNITY_EXCLUDE_DETAILS
#ifdef UNITY_DETAIL_STACK_SIZE
    Unity.CurrentDetailStackSize = 0;
#else
    UNITY_CLR_DETAILS();
#endif
#endif
    UNITY_EXEC_TIME_START();
    if (TEST_PROTECT()) {
        setUp();
        func();
    }
    if (TEST_PROTECT()) {
        tearDown();
    }
    UNITY_EXEC_TIME_STOP();
    UnityConcludeTest();
}
#endif

/* ── Per-module test declarations ───────────────────────────────────────── */

/* Each test_*.c file exposes a run_*_tests() function */
void run_tcp_tests(void);
void run_udp_tests(void);
void run_net_transport_udp_tests(void);
void run_ocpp_tests(void);
void run_kwp2000_tests(void);
void run_uds_util_tests(void);
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
void run_base64_tests(void);
void run_ramp_tests(void);
void run_pubsub_tests(void);
void run_pack_tests(void);
void run_cobs_tests(void);
void run_ymodem_tests(void);
void run_at_parser_tests(void);
void run_lin_tests(void);
void run_lintp_tests(void);
void run_gbt27930_tests(void);
void run_cannm_tests(void);
void run_smbus_tests(void);
void run_pmbus_tests(void);
void run_mbus_tests(void);
void run_timer_tests(void);
void run_timer_wheel_tests(void);
void run_mutex_tests(void);
void run_netbuf_tests(void);
void run_datalog_tests(void);
void run_soft_i2c_tests(void);
void run_soft_spi_tests(void);
void run_soft_spi_slave_tests(void);
void run_sd_tests(void);
void run_rtc_tests(void);
void run_hwwdt_tests(void);
void run_soft_onewire_tests(void);
void run_buzzer_tests(void);
void run_keypad_tests(void);
void run_touch_tests(void);
void run_dipswitch_tests(void);
void run_seg7_tests(void);
void run_shiftreg_tests(void);
void run_charlcd_tests(void);
void run_oled_tests(void);
void run_ioexp_tests(void);
void run_joystick_tests(void);
void run_powermon_tests(void);
void run_climate_tests(void);
void run_smartled_tests(void);
void run_distance_tests(void);
void run_scale_tests(void);
void run_lux_tests(void);
void run_rfid_tests(void);
void run_biometric_tests(void);
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
void run_nn_tests(void);
void run_boot_tests(void);
void run_errlog_tests(void);
void run_power_tests(void);
void run_modbus_tests(void);
void run_modbus_master_tests(void);
void run_canopen_tests(void);
void run_cia402_tests(void);
void run_lss_tests(void);
void run_ethercat_tests(void);
void run_cia401_tests(void);
void run_canopen_mgr_tests(void);
void run_dmx512_tests(void);
void run_ccp_tests(void);
void run_xcp_tests(void);
void run_uds_tests(void);
void run_doip_tests(void);
void run_devicenet_tests(void);
void run_isotp_tests(void);
void run_lin_tests(void);
void run_dali_tests(void);
void run_bacnet_tests(void);
void run_wasm_tests(void);

void run_ir_tests(void);
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
void run_geo_tests(void);
void run_hpclock_tests(void);
void run_timesync_tests(void);
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
void run_lwm2m_tests(void);
void run_lwm2m_task_tests(void);
void run_ota_tests(void);
void run_biquad_tests(void);
void run_fft_tests(void);
void run_filter_design_tests(void);
void run_foc_tests(void);
void run_foc_encoder_tests(void);
void run_foc_observer_tests(void);
void run_fault_tests(void);
void run_autotune_tests(void);
void run_metrics_tests(void);
void run_random_tests(void);

void run_gpio_tests(void);
void run_uart_tests(void);
void run_aes_tests(void);
void run_aes_cmac_tests(void);
void run_sha256_tests(void);
void run_pool_tests(void);
void run_coredump_tests(void);
void run_tickless_tests(void);
void run_dma_tests(void);
void run_i2c_async_tests(void);
void run_spi_async_tests(void);
void run_fwupdate_hmac_tests(void);
void run_fwupdate_ed25519_tests(void);
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
void run_p256_tests(void);
void run_dnssd_tests(void);
void run_ed25519_tests(void);
void run_cose_tests(void);
void run_sha512_tests(void);
void run_dtls_tests(void);
void run_xrce_dds_tests(void);
void run_opcua_tests(void);
void run_gcode_tests(void);
void run_iolink_tests(void);
void run_kinematics_tests(void);
void run_mqttsn_tests(void);
void run_tpm2_tests(void);
void run_backoff_tests(void);
void run_dsp_tests(void);
void run_eth_tests(void);
void run_dhcp_tests(void);
void run_vad_tests(void);
void run_str_tests(void);
void run_fat_tests(void);

#define RUN_TEST_GROUP(file, fn)                           \
    do {                                                   \
        printf("\n=== [Test Group Start] %s ===\n", file); \
        fflush(stdout);                                    \
        fn();                                              \
        printf("=== [Test Group End] %s ===\n", file);     \
        fflush(stdout);                                    \
    } while (0)

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    UNITY_BEGIN();

    /* Utilities */
    run_ringbuf_tests();
    run_backoff_tests();
    run_crc_tests();
    run_fsm_tests();
    run_fmt_tests();
    run_base64_tests();
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
    run_adc_tests();
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
    RUN_TEST_GROUP("test_canopen.c", run_canopen_tests);
    run_cia402_tests();
    RUN_TEST_GROUP("test_ethercat.c", run_ethercat_tests);
    run_lss_tests();
    run_cia401_tests();
    run_canopen_mgr_tests();
    run_dmx512_tests();
    run_ccp_tests();
    run_xcp_tests();
    run_uds_tests();
    run_doip_tests();
    run_devicenet_tests();
    run_isotp_tests();
    run_wasm_tests();
    run_nn_tests();
    run_j1939_tests();
    run_motor_ctrl_tests();
    run_actuator_tests();

    /* Motor */
    run_servo_tests();
    run_dc_motor_tests();

    /* Drivers */
    run_sensor_tests();
    run_exti_tests();
    run_can_tests();
    run_gpio_tests();
    run_uart_tests();

    /* Protocol */
    run_cobs_tests();
    run_ymodem_tests();
    run_lin_tests();
    run_lintp_tests();
    run_gbt27930_tests();
    run_cannm_tests();
    run_dali_tests();
    run_bacnet_tests();

    run_ir_tests();

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
    run_lwm2m_tests();
    run_lwm2m_task_tests();
    run_biquad_tests();
    run_fft_tests();
    run_fault_tests();
    run_autotune_tests();

    /* OTA / Firmware Update */
    run_fwupdate_tests();
    run_ota_tests();

    /* New modules */
    run_aes_cmac_tests();
    run_sha256_tests();
    run_pool_tests();
    run_coredump_tests();
    run_tickless_tests();

    /* New features: DMA, Async I2C/SPI, HMAC FW, Timer Expiry */
    run_dma_tests();
    run_i2c_async_tests();
    run_spi_async_tests();
    run_fwupdate_hmac_tests();
    run_fwupdate_ed25519_tests();
    run_timer_expiry_tests();

    /* Multicore (AMP) */
    run_multicore_tests();

    /* Crypto & WireGuard */
    run_crypto_tests();
    run_wg_tests();

    /* Networking */
    run_sntp_tests();
    run_dnssd_tests();

    /* Control */
    run_control_stats_tests();

    /* Storage */
    run_settings_tests();

    /* Streams & AT Parser */
    run_stream_tests();
    run_at_parser_tests();

    /* Fixed-point math, matrix, vector & quaternion */
    run_matrix_tests();
    void run_quaternion_tests(void);
    void run_vector_tests(void);
    void run_transform_tests(void);
    run_quaternion_tests();
    run_vector_tests();
    run_transform_tests();

    /* Kalman filter */
    run_kalman_tests();

    /* FOC transforms & Observer */
    run_foc_tests();
    run_foc_encoder_tests();
    run_foc_observer_tests();

    /* IMU Sensor Fusion */
    run_sensor_fusion_tests();

    /* Filter Design Generator */
    run_filter_design_tests();

    /* Modbus Master & TCP */
    run_modbus_tcp_tests();

    /* NMEA 0183 Navigation */
    run_nmea_tests();

    /* Multi-Axis Motion Interpolator */
    run_interpolator_tests();

    /* System Metrics */
    run_metrics_tests();

    /* Random Utilities */
    run_random_tests();

    /* CLI Shell */
    /* (run_cli_tests called above) */

    /* DMA Driver Engine */
    /* (run_dma_tests called above) */

    /* SAE J1939 Heavy Duty Vehicle Protocol */
    /* (run_j1939_tests called above) */

    /* NMEA 2000 Marine CAN Protocol */
    run_n2k_tests();

    /* SMBus, PMBus, M-Bus, DL/T 645 & CJ/T 188 Protocols */
    void run_dlt645_tests(void);
    void run_cjt188_tests(void);
    run_smbus_tests();
    run_pmbus_tests();
    run_mbus_tests();
    run_dlt645_tests();
    run_cjt188_tests();

    /* Task Profiler & DDS Synthesizer */
    run_task_profile_tests();
    run_dds_tests();

    /* OS Kernel Primitives */
    run_netbuf_tests();
    run_timer_wheel_tests();
    run_spsc_queue_tests();
    run_slab_tests();
    run_event_flags_tests();

    /* Stepper Motor Driver */
    run_stepper_tests();

    /* Geodetic, High-Precision Clock & TimeSync */
    run_geo_tests();
    run_hpclock_tests();
    run_timesync_tests();

    /* New Hardware Peripherals */
    run_buzzer_tests();
    run_keypad_tests();
    run_touch_tests();
    run_dipswitch_tests();
    run_seg7_tests();
    run_shiftreg_tests();
    run_charlcd_tests();
    run_oled_tests();
    run_ioexp_tests();
    run_joystick_tests();
    run_powermon_tests();
    run_climate_tests();
    run_smartled_tests();
    run_distance_tests();
    run_scale_tests();
    run_lux_tests();
    run_rfid_tests();
    run_biometric_tests();

    /* Fixed-Point DSP & TinyML Neural Network Engine Tests */
    extern void test_q7_math_boundaries_and_saturation(void);
    extern void test_q15_math_boundaries_and_saturation(void);
    extern void test_cross_format_conversions(void);
    extern void test_q7_mul_and_mac(void);
    extern void test_nn_activations(void);
    extern void test_nn_softmax(void);
    extern void test_nn_attention(void);
    extern void test_nn_dct_transformer_pipeline(void);
    extern void test_nn_protothread_coroutine(void);
    extern void test_nn_conv1d_and_coroutine(void);
    extern void test_nn_affine_quantization(void);
    extern void test_nn_pooling_layers(void);
    extern void test_nn_edge_cases_and_null_checks(void);
    extern void test_dsp_dct2_null_params(void);
    extern void test_dsp_dct2_dc_constant(void);
    extern void test_bldc_6step_init_defaults(void);
    extern void test_bldc_6step_hall_commutation_cw(void);
    extern void test_bldc_6step_invalid_hall_fault(void);
    extern void test_bldc_6step_direction_and_stop(void);
    extern void test_bldc_6step_speed_calculation(void);
    extern void test_bldc_6step_null_params_and_edge_cases(void);
    extern void test_bldc_6step_fault_latch_high_rpm_and_pid_clamp(void);
    extern void test_sbus_init(void);
    extern void test_sbus_decode_buffer(void);
    extern void test_sbus_streaming_parser(void);
    extern void test_sbus_raw_to_us_scaling(void);
    extern void test_sbus_null_and_error_handling(void);
    extern void test_dshot_crc_calculation(void);
    extern void test_dshot_encode(void);
    extern void test_dshot_us_to_throttle(void);
    extern void test_dshot_null_and_clamping(void);
    extern void test_ppm_init(void);
    extern void test_ppm_process_frame(void);
    extern void test_ppm_null_and_clamping(void);
    extern void test_crsf_init(void);
    extern void test_crsf_crc8_calculation(void);
    extern void test_crsf_parse_rc_channels(void);
    extern void test_crsf_raw_to_us_scaling(void);
    extern void test_crsf_null_and_error_handling(void);
    extern void test_ibus_init(void);
    extern void test_ibus_checksum_calculation(void);
    extern void test_ibus_streaming_parser(void);
    extern void test_ibus_null_and_error_handling(void);
    extern void test_rc_curve_linear_no_deadband(void);
    extern void test_rc_curve_deadband(void);
    extern void test_rc_curve_expo_and_dual_rate(void);
    extern void test_rc_curve_null_config(void);
    extern void test_rc_failsafe_init(void);
    extern void test_rc_failsafe_timeout_trigger(void);
    extern void test_rc_failsafe_null_and_error(void);
    extern void test_dshot_gcr_decode(void);
    extern void test_dshot_telemetry_erpm_parsing(void);
    extern void test_flight_init(void);
    extern void test_flight_hover(void);
    extern void test_flight_roll_correction(void);
    extern void test_flight_angle_mode(void);
    extern void test_flight_null_and_bounds(void);
    extern void test_crsf_parse_link_stats(void);
    extern void test_msp_init(void);
    extern void test_msp_encode_and_parse_response(void);
    extern void test_msp_null_and_error_handling(void);
    extern void test_mavlink_init(void);
    extern void test_mavlink_encode_and_parse_attitude(void);
    extern void test_mavlink_null_and_crc_error(void);
    extern void test_blackbox_init(void);
    extern void test_blackbox_varint(void);
    extern void test_blackbox_encode_intra_and_delta(void);
    extern void test_blackbox_null_checks(void);
    extern void test_usb_cdc_init(void);
    extern void test_usb_cdc_setup_requests(void);
    extern void test_usb_cdc_read_write(void);
    extern void test_usb_cdc_null_checks(void);
    extern void test_eth_init(void);
    extern void test_eth_arp_cache(void);
    extern void test_eth_build_frame(void);
    extern void test_eth_process_arp_request(void);
    extern void test_eth_coroutine_pt(void);
    extern void test_eth_null_checks(void);
    extern void test_eth_runt_and_oversized_frames(void);
    extern void test_eth_mac_filtering(void);
    extern void test_eth_arp_cache_eviction_overflow(void);
    extern void test_eth_multiprotocol_interleaving(void);
    extern void test_icmp_init(void);
    extern void test_icmp_checksum(void);
    extern void test_icmp_build_echo_request(void);
    extern void test_icmp_process_echo_request(void);
    extern void test_icmp_null_checks(void);
    extern void test_icmp_process_packet_invalid_headers(void);
    extern void test_autoip_init(void);
    extern void test_autoip_probe_and_announce(void);
    extern void test_autoip_process_arp_binding(void);
    extern void test_autoip_coroutine_pt(void);
    extern void test_autoip_null_checks(void);
    extern void test_netcfg_init_static(void);
    extern void test_netcfg_autoip_fallback(void);
    extern void test_netcfg_link_events(void);
    extern void test_netcfg_coroutine_pt(void);
    extern void test_netcfg_null_checks(void);
    extern void test_igmp_init(void);
    extern void test_igmp_join_and_leave(void);
    extern void test_igmp_process_query(void);
    extern void test_igmp_null_checks(void);
    extern void test_igmp_group_overflow_and_leaving_unjoined(void);
    extern void test_igmp_non_igmp_packets(void);

    RUN_TEST(test_q7_math_boundaries_and_saturation);
    RUN_TEST(test_q15_math_boundaries_and_saturation);
    RUN_TEST(test_cross_format_conversions);
    RUN_TEST(test_q7_mul_and_mac);
    run_dsp_tests();
    RUN_TEST(test_bldc_6step_init_defaults);
    RUN_TEST(test_bldc_6step_hall_commutation_cw);
    RUN_TEST(test_bldc_6step_invalid_hall_fault);
    RUN_TEST(test_bldc_6step_direction_and_stop);
    RUN_TEST(test_bldc_6step_speed_calculation);
    RUN_TEST(test_bldc_6step_null_params_and_edge_cases);
    RUN_TEST(test_bldc_6step_fault_latch_high_rpm_and_pid_clamp);
    RUN_TEST(test_sbus_init);
    RUN_TEST(test_sbus_decode_buffer);
    RUN_TEST(test_sbus_streaming_parser);
    RUN_TEST(test_sbus_raw_to_us_scaling);
    RUN_TEST(test_sbus_null_and_error_handling);
    RUN_TEST(test_dshot_crc_calculation);
    RUN_TEST(test_dshot_encode);
    RUN_TEST(test_dshot_us_to_throttle);
    RUN_TEST(test_dshot_null_and_clamping);
    RUN_TEST(test_ppm_init);
    RUN_TEST(test_ppm_process_frame);
    RUN_TEST(test_ppm_null_and_clamping);
    RUN_TEST(test_crsf_init);
    RUN_TEST(test_crsf_crc8_calculation);
    RUN_TEST(test_crsf_parse_rc_channels);
    RUN_TEST(test_crsf_parse_link_stats);
    RUN_TEST(test_crsf_raw_to_us_scaling);
    RUN_TEST(test_crsf_null_and_error_handling);
    RUN_TEST(test_ibus_init);
    RUN_TEST(test_ibus_checksum_calculation);
    RUN_TEST(test_ibus_streaming_parser);
    RUN_TEST(test_ibus_null_and_error_handling);
    RUN_TEST(test_rc_curve_linear_no_deadband);
    RUN_TEST(test_rc_curve_deadband);
    RUN_TEST(test_rc_curve_expo_and_dual_rate);
    RUN_TEST(test_rc_curve_null_config);
    extern void test_rc_curve_extended_edge_cases(void);
    RUN_TEST(test_rc_curve_extended_edge_cases);
    RUN_TEST(test_rc_failsafe_init);
    RUN_TEST(test_rc_failsafe_timeout_trigger);
    RUN_TEST(test_rc_failsafe_null_and_error);
    RUN_TEST(test_dshot_gcr_decode);
    RUN_TEST(test_dshot_telemetry_erpm_parsing);
    extern void test_dshot_telemetry_extended_edge_cases(void);
    RUN_TEST(test_dshot_telemetry_extended_edge_cases);
    RUN_TEST(test_flight_init);
    RUN_TEST(test_flight_hover);
    RUN_TEST(test_flight_roll_correction);
    RUN_TEST(test_flight_angle_mode);
    RUN_TEST(test_flight_null_and_bounds);
    extern void test_flight_clamp_motor_outputs_bounds(void);
    RUN_TEST(test_flight_clamp_motor_outputs_bounds);
    RUN_TEST(test_msp_init);
    RUN_TEST(test_msp_encode_and_parse_response);
    RUN_TEST(test_msp_null_and_error_handling);
    RUN_TEST(test_mavlink_init);
    RUN_TEST(test_mavlink_encode_and_parse_attitude);
    RUN_TEST(test_mavlink_null_and_crc_error);
    extern void test_mavlink_msg_ids_and_invalid_state_fallback(void);
    RUN_TEST(test_mavlink_msg_ids_and_invalid_state_fallback);
    extern void test_mavlink_stream_noise_and_post_error_resync(void);
    RUN_TEST(test_mavlink_stream_noise_and_post_error_resync);
    RUN_TEST(test_blackbox_init);
    RUN_TEST(test_blackbox_varint);
    RUN_TEST(test_blackbox_encode_intra_and_delta);
    RUN_TEST(test_blackbox_null_checks);
    extern void test_eth_generate_mac(void);
    extern void test_eth_init(void);

    extern void test_usb_init_and_state(void);
    extern void test_usb_set_address_and_config(void);
    extern void test_usb_get_descriptors(void);
    extern void test_usb_class_registration_and_routing(void);
    extern void test_usb_raw_config_override(void);
    extern void test_usb_null_checks(void);

    RUN_TEST(test_usb_init_and_state);
    RUN_TEST(test_usb_set_address_and_config);
    RUN_TEST(test_usb_get_descriptors);
    RUN_TEST(test_usb_class_registration_and_routing);
    RUN_TEST(test_usb_raw_config_override);
    RUN_TEST(test_usb_null_checks);

    extern void test_usb_cdc_registration(void);
    extern void test_usb_cdc_transport_bridge(void);

    RUN_TEST(test_usb_cdc_init);
    RUN_TEST(test_usb_cdc_registration);
    RUN_TEST(test_usb_cdc_setup_requests);
    RUN_TEST(test_usb_cdc_read_write);
    RUN_TEST(test_usb_cdc_transport_bridge);
    RUN_TEST(test_usb_cdc_null_checks);

    extern void test_usb_hid_init_and_register(void);
    extern void test_usb_hid_report_send_and_read(void);
    extern void test_usb_hid_class_requests(void);
    extern void test_usb_hid_keyboard_helpers(void);
    extern void test_usb_hid_mouse_helpers(void);
    extern void test_usb_hid_null_checks(void);

    RUN_TEST(test_usb_hid_init_and_register);
    RUN_TEST(test_usb_hid_report_send_and_read);
    RUN_TEST(test_usb_hid_class_requests);
    RUN_TEST(test_usb_hid_keyboard_helpers);
    RUN_TEST(test_usb_hid_mouse_helpers);
    RUN_TEST(test_usb_hid_null_checks);

    extern void run_usb_host_tests(void);
    run_usb_host_tests();

    run_eth_tests();
    run_dhcp_tests();
    RUN_TEST(test_icmp_init);
    RUN_TEST(test_icmp_checksum);
    RUN_TEST(test_icmp_build_echo_request);
    RUN_TEST(test_icmp_process_echo_request);
    RUN_TEST(test_icmp_null_checks);
    RUN_TEST(test_icmp_process_packet_invalid_headers);
    RUN_TEST(test_autoip_init);
    RUN_TEST(test_autoip_probe_and_announce);
    RUN_TEST(test_autoip_process_arp_binding);
    RUN_TEST(test_autoip_coroutine_pt);
    RUN_TEST(test_autoip_null_checks);
    RUN_TEST(test_netcfg_init_static);
    RUN_TEST(test_netcfg_autoip_fallback);
    RUN_TEST(test_netcfg_link_events);
    RUN_TEST(test_netcfg_coroutine_pt);
    RUN_TEST(test_netcfg_null_checks);
    RUN_TEST(test_igmp_init);
    RUN_TEST(test_igmp_join_and_leave);
    RUN_TEST(test_igmp_process_query);
    RUN_TEST(test_igmp_null_checks);
    RUN_TEST(test_igmp_group_overflow_and_leaving_unjoined);
    RUN_TEST(test_igmp_non_igmp_packets);
    extern void run_hkdf_tests(void);
    extern void run_hmac_drbg_tests(void);
    extern void run_asn1_x509_tests(void);
    extern void run_tls_tests(void);
    extern void run_pwm_tests(void);
    extern void run_comp_tests(void);
    extern void run_i2c_driver_tests(void);
    extern void run_spi_driver_tests(void);
    extern void run_i2c_queue_tests(void);
    extern void run_spi_queue_tests(void);

    run_tcp_tests();
    run_udp_tests();
    run_net_transport_udp_tests();
    run_ocpp_tests();
    run_kwp2000_tests();
    run_uds_util_tests();
    run_hkdf_tests();
    run_hmac_drbg_tests();
    run_asn1_x509_tests();
    run_tls_tests();
    run_pwm_tests();
    run_comp_tests();
    run_i2c_driver_tests();
    run_spi_driver_tests();
    run_i2c_queue_tests();
    run_spi_queue_tests();

    extern void run_adpcm_tests(void);
    extern void run_sbc_tests(void);
    extern void run_audio_tests(void);
    extern void run_audio_mixer_tests(void);
    extern void run_wav_tests(void);
    extern void run_goertzel_tests(void);
    extern void run_lz4_tests(void);
    extern void run_usb_midi_tests(void);
    extern void run_mfcc_tests(void);
    extern void run_usb_msc_tests(void);
    extern void run_ntp_server_tests(void);
    extern void run_protobuf_tests(void);
    extern void test_ble_hci_null_params(void);
    extern void test_ble_hci_encode_command(void);
    extern void test_ble_hci_encode_acl(void);
    extern void test_ble_hci_rx_event_dispatch(void);
    extern void test_ble_hci_rx_acl_dispatch(void);
    extern void test_ble_hci_rx_overflow_and_invalid(void);
    extern void test_ble_hci_cmd_complete_short_params(void);
    extern void test_ble_gap_null_params(void);
    extern void test_ble_gap_adv_data_encoding(void);
    extern void test_ble_gap_process_events(void);
    extern void test_ble_gatt_null_params(void);
    extern void test_ble_gatt_read_request(void);
    extern void test_ble_gatt_write_request(void);
    extern void test_ble_gatt_notification(void);
    extern void test_ble_l2cap_null_and_init(void);
    extern void test_ble_l2cap_connect_disconnect_pool(void);
    extern void test_ble_l2cap_single_and_fragmented_acl(void);
    extern void test_ble_l2cap_errors_and_edge_cases(void);
    extern void test_ble_l2cap_encode_pdu(void);
    extern void test_ble_att_encoders(void);

    run_adpcm_tests();
    run_sbc_tests();
    run_audio_tests();
    run_audio_mixer_tests();
    run_wav_tests();
    run_goertzel_tests();
    run_lz4_tests();
    run_usb_midi_tests();
    run_mfcc_tests();
    run_usb_msc_tests();
    run_ntp_server_tests();
    run_protobuf_tests();

    RUN_TEST(test_ble_hci_null_params);
    RUN_TEST(test_ble_hci_encode_command);
    RUN_TEST(test_ble_hci_encode_acl);
    RUN_TEST(test_ble_hci_rx_event_dispatch);
    RUN_TEST(test_ble_hci_rx_acl_dispatch);
    RUN_TEST(test_ble_hci_rx_overflow_and_invalid);
    RUN_TEST(test_ble_hci_cmd_complete_short_params);
    RUN_TEST(test_ble_gap_null_params);
    RUN_TEST(test_ble_gap_adv_data_encoding);
    RUN_TEST(test_ble_gap_process_events);
    RUN_TEST(test_ble_gatt_null_params);
    RUN_TEST(test_ble_gatt_read_request);
    RUN_TEST(test_ble_gatt_write_request);
    RUN_TEST(test_ble_gatt_notification);
    RUN_TEST(test_ble_l2cap_null_and_init);
    RUN_TEST(test_ble_l2cap_connect_disconnect_pool);
    RUN_TEST(test_ble_l2cap_single_and_fragmented_acl);
    RUN_TEST(test_ble_l2cap_errors_and_edge_cases);
    RUN_TEST(test_ble_l2cap_encode_pdu);
    RUN_TEST(test_ble_att_encoders);
    run_p256_tests();
    run_ed25519_tests();
    run_cose_tests();
    run_aes_tests();
    run_sha512_tests();
    run_dtls_tests();
    run_xrce_dds_tests();
    run_opcua_tests();
    run_gcode_tests();
    run_iolink_tests();
    run_kinematics_tests();
    run_mqttsn_tests();
    run_tpm2_tests();
    run_vad_tests();
    run_mutex_tests();
    run_str_tests();
    run_fat_tests();

    extern void test_r5_autoip_regressions(void);
    extern void test_r5_blackbox_regressions(void);
    extern void test_r5_ble_att_regressions(void);
    extern void test_r5_ble_gap_regressions(void);
    extern void test_r5_ble_gatt_regressions(void);
    extern void test_r5_ble_hci_regressions(void);
    extern void test_r5_ble_l2cap_regressions(void);
    extern void test_r5_flight_pid_regressions(void);
    extern void test_r5_icmp_regressions(void);
    extern void test_r5_igmp_regressions(void);
    extern void test_r5_rc_failsafe_regressions(void);
    extern void test_rc_failsafe_future_frame_timestamp_no_false_trip(void);
    extern void test_r5_usb_regressions(void);
    extern void test_r5_usb_cdc_regressions(void);
    extern void test_r5_usb_hid_regressions(void);
    extern void test_tier_a_usb_regressions(void);
    extern void test_tier_a_usb_cdc_regressions(void);
    extern void test_tier_a_usb_hid_regressions(void);
    extern void test_tier_a_igmp_regressions(void);
    extern void test_tier_a_icmp_regressions(void);
    extern void test_tier_a_autoip_regressions(void);
    extern void test_tier_a_crsf_short_payload_guards(void);
    extern void test_tier_a_sbus_footer_flags_and_resync(void);
    extern void test_tier_a_ibus_duplicate_header1_recovery(void);
    extern void test_tier_a_msp_header_resync_and_null_payload(void);
    extern void test_pro08_mavlink_large_payload_and_bounds(void);

    RUN_TEST(test_r5_autoip_regressions);
    RUN_TEST(test_r5_blackbox_regressions);
    RUN_TEST(test_r5_ble_att_regressions);
    RUN_TEST(test_r5_ble_gap_regressions);
    RUN_TEST(test_r5_ble_gatt_regressions);
    RUN_TEST(test_r5_ble_hci_regressions);
    RUN_TEST(test_r5_ble_l2cap_regressions);
    RUN_TEST(test_r5_flight_pid_regressions);
    RUN_TEST(test_r5_icmp_regressions);
    RUN_TEST(test_r5_igmp_regressions);
    RUN_TEST(test_r5_rc_failsafe_regressions);
    RUN_TEST(test_rc_failsafe_future_frame_timestamp_no_false_trip);
    RUN_TEST(test_r5_usb_regressions);
    RUN_TEST(test_r5_usb_cdc_regressions);
    RUN_TEST(test_r5_usb_hid_regressions);
    RUN_TEST(test_tier_a_usb_regressions);
    RUN_TEST(test_tier_a_usb_cdc_regressions);
    RUN_TEST(test_tier_a_usb_hid_regressions);
    RUN_TEST(test_tier_a_igmp_regressions);
    RUN_TEST(test_tier_a_icmp_regressions);
    RUN_TEST(test_tier_a_autoip_regressions);
    RUN_TEST(test_tier_a_crsf_short_payload_guards);
    RUN_TEST(test_tier_a_sbus_footer_flags_and_resync);
    RUN_TEST(test_tier_a_ibus_duplicate_header1_recovery);
    RUN_TEST(test_tier_a_msp_header_resync_and_null_payload);
    RUN_TEST(test_pro08_mavlink_large_payload_and_bounds);

    int failures = UNITY_END();
#ifdef SYN_HOST_UNIT_RUNNER
    {
        const char *filter = getenv("SYN_TEST_FILTER");
        if (filter != NULL && filter[0] != '\0' && Unity.NumberOfTests == (UNITY_COUNTER_TYPE)0) {
            fprintf(stderr, "ERROR: SYN_TEST_FILTER='%s' matched 0 tests\n", filter);
            return 1;
        }
    }
#endif
    return (failures == 0) ? 0 : 1;
}
