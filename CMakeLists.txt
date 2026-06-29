cmake_minimum_required(VERSION 3.10)
project(SyntropicOS LANGUAGES C)

# ── Collect source files ────────────────────────────────────────────────────
set(SYN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/drivers/syn_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/drivers/syn_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/util/syn_ringbuf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/util/syn_fsm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/util/syn_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/sched/syn_sched.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/sched/syn_timer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/sched/syn_watchdog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/log/syn_log.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/cli/syn_cli.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/input/syn_button.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/output/syn_led.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/output/syn_soft_pwm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/dsp/syn_filter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/control/syn_pid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/motor/syn_stepper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/motor/syn_servo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/motor/syn_dc_motor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/motor/syn_motor_ctrl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/proto/syn_cobs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/sensor/syn_sensor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/storage/syn_param.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/input/syn_encoder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/drivers/syn_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/sched/syn_sequencer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/proto/syn_modbus.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/debug/syn_trace.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/debug/syn_profiler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/system/syn_boot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/system/syn_errlog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/util/syn_fmt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/sched/syn_workqueue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/drivers/syn_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/dsp/syn_signal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/util/syn_ramp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/system/syn_power.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/motor/syn_actuator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/display/syn_canvas.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/ui/syn_menu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/ui/syn_imgui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/drivers/syn_can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/net/syn_router.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/net/syn_heartbeat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/sched/syn_ao.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/storage/syn_vfs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/storage/syn_lfs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/net/syn_coap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/dsp/syn_biquad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/dsp/syn_fft.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/system/syn_fault.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/control/syn_autotune.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/storage/syn_settings.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/log/syn_datalog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/util/syn_scurve.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/util/syn_pubsub.c
)

# Optional: weak stubs (link these so unimplemented port functions assert
# at runtime instead of giving a linker error)
set(SYN_STUB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/syntropic/port_stubs/syn_port_stubs.c
)

# ── Library target ──────────────────────────────────────────────────────────
# INTERFACE-only by default so the parent project controls compilation flags.
# If you prefer a static library, change INTERFACE to STATIC below.

add_library(syntropic INTERFACE)

target_sources(syntropic INTERFACE
    ${SYN_SOURCES}
)

target_include_directories(syntropic INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ── Optional: static library variant ────────────────────────────────────────
# Uncomment the block below (and comment out the INTERFACE block above) if
# you prefer SyntropicOS compiled as a separate static library.
#
# add_library(syn_static STATIC ${SYN_SOURCES})
# target_include_directories(syn_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_compile_features(syn_static PUBLIC c_std_99)

# ── Convenience target for stubs ────────────────────────────────────────────
add_library(syn_stubs INTERFACE)
target_sources(syn_stubs INTERFACE ${SYN_STUB_SOURCES})
target_link_libraries(syn_stubs INTERFACE syntropic)

# ── Tests (only when built standalone, not as a submodule) ──────────────────
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()
    add_executable(test_multitask tests/test_multitask.c ${SYN_SOURCES})
    target_include_directories(test_multitask PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(test_multitask PRIVATE -Wall -Wextra -pedantic)
    add_test(NAME syn_tests COMMAND test_multitask)
endif()
