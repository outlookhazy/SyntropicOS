

# File syntropic.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**syntropic.h**](syntropic_8h.md)

[Go to the source code of this file](syntropic_8h_source.md)

_SyntropicOS umbrella header._ [More...](#detailed-description)

* `#include "common/syn_compiler.h"`
* `#include "common/syn_defs.h"`
* `#include "util/syn_assert.h"`
* `#include "util/syn_bits.h"`
* `#include "util/syn_change_filter.h"`
* `#include "util/syn_crc.h"`
* `#include "util/syn_hysteresis.h"`
* `#include "util/syn_lut.h"`
* `#include "util/syn_pack.h"`
* `#include "util/syn_pingpong.h"`
* `#include "util/syn_qmath.h"`
* `#include "util/syn_rate_limit.h"`
* `#include "util/syn_ringbuf.h"`
* `#include "util/syn_str.h"`
* `#include "util/syn_stream.h"`
* `#include "util/syn_timeout.h"`
* `#include "util/syn_event.h"`
* `#include "util/syn_fsm.h"`
* `#include "util/syn_fmt.h"`
* `#include "util/syn_cbor_read.h"`
* `#include "util/syn_cbor_write.h"`
* `#include "util/syn_ramp.h"`
* `#include "util/syn_aes128.h"`
* `#include "util/syn_hmac.h"`
* `#include "util/syn_sha256.h"`
* `#include "util/syn_pool.h"`
* `#include "util/syn_lz4.h"`
* `#include "drivers/syn_gpio.h"`
* `#include "drivers/syn_uart.h"`
* `#include "drivers/syn_adc.h"`
* `#include "drivers/syn_exti.h"`
* `#include "drivers/syn_sd.h"`
* `#include "drivers/syn_rtc.h"`
* `#include "drivers/syn_dac.h"`
* `#include "drivers/syn_dma.h"`
* `#include "drivers/syn_i2c.h"`
* `#include "drivers/syn_i2c_queue.h"`
* `#include "drivers/syn_spi.h"`
* `#include "drivers/syn_spi_queue.h"`
* `#include "drivers/syn_soft_onewire.h"`
* `#include "drivers/syn_shiftreg.h"`
* `#include "drivers/syn_ioexp.h"`
* `#include "drivers/syn_rfid.h"`
* `#include "drivers/syn_can.h"`
* `#include "drivers/syn_hpclock.h"`
* `#include "drivers/syn_timesync.h"`
* `#include "util/syn_geo.h"`
* `#include "pt/syn_pt.h"`
* `#include "pt/syn_pt_sem.h"`
* `#include "sched/syn_sched.h"`
* `#include "sched/syn_timer.h"`
* `#include "sched/syn_watchdog.h"`
* `#include "sched/syn_sequencer.h"`
* `#include "sched/syn_workqueue.h"`
* `#include "sched/syn_mailbox.h"`
* `#include "sched/syn_ao.h"`
* `#include "sched/syn_event_flags.h"`
* `#include "sched/syn_timer_wheel.h"`
* `#include "util/syn_netbuf.h"`
* `#include "util/syn_slab.h"`
* `#include "util/syn_spsc_queue.h"`
* `#include "log/syn_log.h"`
* `#include "cli/syn_cli.h"`
* `#include "input/syn_button.h"`
* `#include "input/syn_encoder.h"`
* `#include "input/syn_keypad.h"`
* `#include "input/syn_touch.h"`
* `#include "input/syn_dipswitch.h"`
* `#include "input/syn_joystick.h"`
* `#include "output/syn_led.h"`
* `#include "output/syn_smartled.h"`
* `#include "output/syn_buzzer.h"`
* `#include "output/syn_soft_pwm.h"`
* `#include "sensor/syn_sensor.h"`
* `#include "sensor/syn_powermon.h"`
* `#include "sensor/syn_climate.h"`
* `#include "sensor/syn_distance.h"`
* `#include "sensor/syn_scale.h"`
* `#include "sensor/syn_lux.h"`
* `#include "sensor/syn_biometric.h"`
* `#include "dsp/syn_filter.h"`
* `#include "dsp/syn_signal.h"`
* `#include "dsp/syn_biquad.h"`
* `#include "dsp/syn_fft.h"`
* `#include "dsp/syn_dds.h"`
* `#include "dsp/syn_goertzel.h"`
* `#include "audio/syn_adpcm.h"`
* `#include "audio/syn_sbc.h"`
* `#include "audio/syn_audio.h"`
* `#include "audio/syn_audio_mixer.h"`
* `#include "audio/syn_wav.h"`
* `#include "control/syn_pid.h"`
* `#include "motor/syn_stepper.h"`
* `#include "motor/syn_servo.h"`
* `#include "motor/syn_dc_motor.h"`
* `#include "motor/syn_bldc_6step.h"`
* `#include "motor/syn_motor_ctrl.h"`
* `#include "motor/syn_actuator.h"`
* `#include "proto/syn_at_parser.h"`
* `#include "proto/syn_cobs.h"`
* `#include "proto/syn_lin.h"`
* `#include "proto/syn_dali.h"`
* `#include "proto/syn_bacnet.h"`
* `#include "proto/syn_smbus.h"`
* `#include "proto/syn_pmbus.h"`
* `#include "proto/syn_mbus.h"`
* `#include "proto/syn_modbus.h"`
* `#include "proto/syn_sbus.h"`
* `#include "output/syn_dshot.h"`
* `#include "input/syn_ppm.h"`
* `#include "proto/syn_crsf.h"`
* `#include "proto/syn_ibus.h"`
* `#include "control/syn_rc_curve.h"`
* `#include "control/syn_rc_failsafe.h"`
* `#include "output/syn_dshot_telemetry.h"`
* `#include "control/syn_flight_pid.h"`
* `#include "proto/syn_msp.h"`
* `#include "proto/syn_mavlink.h"`
* `#include "log/syn_blackbox.h"`
* `#include "drivers/syn_usb_cdc.h"`
* `#include "net/syn_eth.h"`
* `#include "net/syn_dhcp.h"`
* `#include "net/syn_icmp.h"`
* `#include "net/syn_autoip.h"`
* `#include "net/syn_netcfg.h"`
* `#include "net/syn_igmp.h"`
* `#include "proto/syn_nmea.h"`
* `#include "proto/syn_j1939.h"`
* `#include "proto/syn_gbt27930.h"`
* `#include "proto/syn_cannm.h"`
* `#include "proto/syn_n2k.h"`
* `#include "proto/syn_cia303.h"`
* `#include "proto/syn_cia418.h"`
* `#include "proto/syn_dmx512.h"`
* `#include "proto/syn_isotp.h"`
* `#include "proto/syn_cjt188.h"`
* `#include "proto/syn_ethercat.h"`
* `#include "proto/syn_ccp.h"`
* `#include "proto/syn_xcp.h"`
* `#include "proto/syn_uds.h"`
* `#include "proto/syn_devicenet.h"`
* `#include "storage/syn_param.h"`
* `#include "storage/syn_lfs.h"`
* `#include "storage/syn_vfs.h"`
* `#include "display/syn_canvas.h"`
* `#include "display/syn_gfx.h"`
* `#include "display/syn_charlcd.h"`
* `#include "display/syn_oled.h"`
* `#include "display/syn_seg7.h"`
* `#include "ui/syn_menu.h"`
* `#include "ui/syn_imgui.h"`
* `#include "net/syn_transport.h"`
* `#include "net/syn_router.h"`
* `#include "net/syn_heartbeat.h"`
* `#include "net/syn_coap.h"`
* `#include "net/syn_sntp.h"`
* `#include "net/syn_wg.h"`
* `#include "crypto/syn_blake2s.h"`
* `#include "crypto/syn_chacha20poly1305.h"`
* `#include "crypto/syn_x25519.h"`
* `#include "debug/syn_trace.h"`
* `#include "debug/syn_profiler.h"`
* `#include "system/syn_boot.h"`
* `#include "system/syn_errlog.h"`
* `#include "system/syn_fault.h"`
* `#include "system/syn_hwwdt.h"`
* `#include "system/syn_coredump.h"`
* `#include "system/syn_power.h"`
* `#include "vm/syn_wasm.h"`
* `#include "system/syn_sleep.h"`
* `#include "system/syn_version.h"`
* `#include "drivers/syn_usb_midi.h"`
* `#include "drivers/syn_usb_msc.h"`
* `#include "dsp/syn_mfcc.h"`
* `#include "net/syn_ntp_server.h"`
* `#include "util/syn_protobuf.h"`

































































## Detailed Description


Include this single header to pull in all enabled SyntropicOS modules. Module selection is driven by syn\_config.h (see [**syn\_config\_template.h**](syn__config__template_8h.md)).


You can also include individual module headers directly for finer control over what gets pulled in. 


    

------------------------------
The documentation for this class was generated from the following file `src/syntropic/syntropic.h`

