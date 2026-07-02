

# File syntropic.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**syntropic.h**](syntropic_8h.md)

[Go to the source code of this file](syntropic_8h_source.md)

_SyntropicOS umbrella header._ [More...](#detailed-description)

* `#include "common/syn_defs.h"`
* `#include "common/syn_compiler.h"`
* `#include "util/syn_assert.h"`
* `#include "util/syn_bits.h"`
* `#include "util/syn_ringbuf.h"`
* `#include "drivers/syn_gpio.h"`
* `#include "drivers/syn_uart.h"`
* `#include "pt/syn_pt.h"`
* `#include "pt/syn_pt_sem.h"`
* `#include "sched/syn_sched.h"`
* `#include "sched/syn_timer.h"`
* `#include "util/syn_event.h"`
* `#include "log/syn_log.h"`
* `#include "cli/syn_cli.h"`
* `#include "util/syn_timeout.h"`
* `#include "util/syn_crc.h"`
* `#include "input/syn_button.h"`
* `#include "output/syn_led.h"`
* `#include "output/syn_soft_pwm.h"`
* `#include "util/syn_fsm.h"`
* `#include "dsp/syn_filter.h"`
* `#include "control/syn_pid.h"`
* `#include "motor/syn_stepper.h"`
* `#include "motor/syn_servo.h"`
* `#include "motor/syn_dc_motor.h"`
* `#include "motor/syn_motor_ctrl.h"`
* `#include "proto/syn_cobs.h"`
* `#include "sensor/syn_sensor.h"`
* `#include "sched/syn_watchdog.h"`
* `#include "storage/syn_param.h"`
* `#include "storage/syn_vfs.h"`
* `#include "storage/syn_lfs.h"`
* `#include "util/syn_hysteresis.h"`
* `#include "util/syn_lut.h"`
* `#include "util/syn_qmath.h"`
* `#include "util/syn_rate_limit.h"`
* `#include "util/syn_pingpong.h"`
* `#include "util/syn_change_filter.h"`
* `#include "input/syn_encoder.h"`
* `#include "drivers/syn_adc.h"`
* `#include "drivers/syn_i2c_dev.h"`
* `#include "drivers/syn_spi_dev.h"`
* `#include "drivers/syn_exti.h"`
* `#include "drivers/syn_sd.h"`
* `#include "drivers/syn_rtc.h"`
* `#include "drivers/syn_dac.h"`
* `#include "drivers/syn_soft_onewire.h"`
* `#include "util/syn_fmt.h"`
* `#include "util/syn_cbor_write.h"`
* `#include "util/syn_cbor_read.h"`
* `#include "sched/syn_sequencer.h"`
* `#include "sched/syn_workqueue.h"`
* `#include "sched/syn_mailbox.h"`
* `#include "sched/syn_ao.h"`
* `#include "proto/syn_modbus.h"`
* `#include "net/syn_coap.h"`
* `#include "debug/syn_trace.h"`
* `#include "debug/syn_profiler.h"`
* `#include "dsp/syn_signal.h"`
* `#include "dsp/syn_biquad.h"`
* `#include "dsp/syn_fft.h"`
* `#include "system/syn_boot.h"`
* `#include "system/syn_errlog.h"`
* `#include "system/syn_fault.h"`
* `#include "system/syn_hwwdt.h"`
* `#include "system/syn_coredump.h"`
* `#include "system/syn_version.h"`
* `#include "system/syn_sleep.h"`
* `#include "util/syn_ramp.h"`
* `#include "util/syn_pack.h"`
* `#include "util/syn_sha256.h"`
* `#include "util/syn_hmac.h"`
* `#include "util/syn_pool.h"`
* `#include "system/syn_power.h"`
* `#include "motor/syn_actuator.h"`
* `#include "display/syn_canvas.h"`
* `#include "display/syn_gfx.h"`
* `#include "ui/syn_menu.h"`
* `#include "ui/syn_imgui.h"`
* `#include "drivers/syn_can.h"`
* `#include "net/syn_transport.h"`
* `#include "net/syn_router.h"`
* `#include "net/syn_heartbeat.h"`

































































## Detailed Description


Include this single header to pull in all enabled SyntropicOS modules. Module selection is driven by syn\_config.h (see [**syn\_config\_template.h**](syn__config__template_8h.md)).


You can also include individual module headers directly for finer control over what gets pulled in. 


    

------------------------------
The documentation for this class was generated from the following file `src/syntropic/syntropic.h`

