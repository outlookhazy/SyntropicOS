
# Class Hierarchy

This inheritance list is sorted roughly, but not completely, alphabetically:


* **struct** [**DirEntryLoc**](structDirEntryLoc.md) _Result of a directory scan — location of a matching entry._ 
* **struct** [**EXTI\_Entry**](structEXTI__Entry.md) _EXTI callback table entry._ 
* **struct** [**FAT\_Volume**](structFAT__Volume.md) _Internal FAT volume metadata parsed from the boot sector._ 
* **struct** [**SYN\_ADC**](structSYN__ADC.md) _ADC channel instance — config + last readings._ 
* **struct** [**SYN\_ADC\_Config**](structSYN__ADC__Config.md) _ADC channel configuration._ 
* **struct** [**SYN\_AO\_Event**](structSYN__AO__Event.md) _Active Object Event._ 
* **struct** [**SYN\_ActiveObject**](structSYN__ActiveObject.md) _Active Object structure._ 
* **struct** [**SYN\_Actuator**](structSYN__Actuator.md) _Linear actuator instance — motor controller + stroke mapping._ 
* **struct** [**SYN\_Actuator\_Config**](structSYN__Actuator__Config.md) _Actuator configuration._ 
* **struct** [**SYN\_AutoTune**](structSYN__AutoTune.md) _Auto-tuner instance._ 
* **struct** [**SYN\_AutoTune\_Config**](structSYN__AutoTune__Config.md) _Auto-tuner configuration._ 
* **struct** [**SYN\_AutoTune\_Limits**](structSYN__AutoTune__Limits.md) _Physical constraints for the one-call auto-tune._ 
* **struct** [**SYN\_AutoTune\_LogFrame**](structSYN__AutoTune__LogFrame.md) _Telemetry frame for Auto-Tune capture._ 
* **struct** [**SYN\_AutoTune\_Result**](structSYN__AutoTune__Result.md) _Auto-tune results (valid when state == DONE)._ 
* **struct** [**SYN\_Boot**](structSYN__Boot.md) _Boot manager — crash-loop detection and safe-mode state._ 
* **struct** [**SYN\_BootData**](structSYN__BootData.md) _Boot data persisted to flash across resets._ 
* **struct** [**SYN\_Button**](structSYN__Button.md) _Button descriptor — owns the FSM, debounce, and callback state._ 
* **struct** [**SYN\_CAN**](structSYN__CAN.md) _CAN bus instance — port, bitrate, callbacks, stats._ 
* **struct** [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) _CAN bus frame — standard or extended._ 
* **struct** [**SYN\_CLI**](structSYN__CLI.md) _CLI instance — command table, line buffer, I/O, and history._ 
* **struct** [**SYN\_CLI\_Command**](structSYN__CLI__Command.md) _Command descriptor._ 
* **struct** [**SYN\_COBS\_Decoder**](structSYN__COBS__Decoder.md) _Streaming COBS frame decoder._ 
* **struct** [**SYN\_Canvas**](structSYN__Canvas.md) _Canvas instance — framebuffer + drawing state._ 
* **struct** [**SYN\_CborReader**](structSYN__CborReader.md) _CBOR decoder state. Caller-allocated; zero heap._ 
* **struct** [**SYN\_CborWriter**](structSYN__CborWriter.md) _CBOR encoder state. Caller-allocated; zero heap._ 
* **struct** [**SYN\_ChangeFilter**](structSYN__ChangeFilter.md) _Dead-band + rate-limited change filter._ 
* **struct** [**SYN\_CoapMsg**](structSYN__CoapMsg.md) _Parsed or to-be-serialized CoAP message header._ 
* **struct** [**SYN\_CoapOption**](structSYN__CoapOption.md) _Single CoAP option (number + opaque value)._ 
* **struct** [**SYN\_CoapRequest**](structSYN__CoapRequest.md) _CoAP client request context._ 
* **struct** [**SYN\_ControlReport**](structSYN__ControlReport.md) _Human-readable performance report._ 
* **struct** [**SYN\_ControlStats**](structSYN__ControlStats.md) _Raw accumulators for control performance analysis._ 
* **struct** [**SYN\_CoreDump**](structSYN__CoreDump.md) _Persistent core dump — written to flash on fault._ 
* **struct** [**SYN\_DAC**](structSYN__DAC.md) _DAC channel handle. Caller allocates; zero heap._ 
* **struct** [**SYN\_DCMotor**](structSYN__DCMotor.md) _DC motor instance — pins, speed, ramp state._ 
* **struct** [**SYN\_DMA\_Config**](structSYN__DMA__Config.md) _DMA channel configuration — caller-owned._ 
* **struct** [**SYN\_DataLog**](structSYN__DataLog.md) _Data logger control block — ring buffer + drop counter._ 
* **struct** [**SYN\_DataLogHeader**](structSYN__DataLogHeader.md) _Telemetry frame header._ 
* **struct** [**SYN\_DnsResolver**](structSYN__DnsResolver.md) _DNS resolver context._ 
* **struct** [**SYN\_Encoder**](structSYN__Encoder.md) _Quadrature encoder instance — pins, position, delta._ 
* **struct** [**SYN\_ErrEntry**](structSYN__ErrEntry.md) _Error log entry — code + severity + context + timestamps._ 
* **struct** [**SYN\_ErrLog**](structSYN__ErrLog.md) _Error log instance — circular buffer of error entries._ 
* **struct** [**SYN\_EventGroup**](structSYN__EventGroup.md) _Event flag group — a 32-bit bitmask of event flags._ 
* **struct** [**SYN\_FSM**](structSYN__FSM.md) _FSM instance — transition table, state descriptors, current state._ 
* **struct** [**SYN\_FSM\_StateDesc**](structSYN__FSM__StateDesc.md) _Per-state entry/exit actions._ 
* **struct** [**SYN\_FSM\_Transition**](structSYN__FSM__Transition.md) _Transition descriptor — (from, event) → to, with optional guard/action._ 
* **struct** [**SYN\_FatFileCtx**](structSYN__FatFileCtx.md) _Open file tracking context (pooled)._ 
* **struct** [**SYN\_FaultContext**](structSYN__FaultContext.md) _Register context dumped on CPU hard faults (ARM Cortex-M style)._ 
* **struct** [**SYN\_FaultDump**](structSYN__FaultDump.md) _Fault dump stored in noinit RAM across resets._ 
* **struct** [**SYN\_FilterBiquad**](structSYN__FilterBiquad.md) _Fixed-point Q16.16 Biquad filter state (Direct Form I)._ 
* **struct** [**SYN\_FilterEMA**](structSYN__FilterEMA.md) _Exponential moving average filter state._ 
* **struct** [**SYN\_FilterMA**](structSYN__FilterMA.md) _Moving average filter state._ 
* **struct** [**SYN\_FilterMedian**](structSYN__FilterMedian.md) _Median filter state._ 
* **struct** [**SYN\_Font**](structSYN__Font.md) _Font descriptor for text rendering._ 
* **struct** [**SYN\_FwBootManager**](structSYN__FwBootManager.md) _A/B firmware boot manager — slot selection and rollback state._ 
* **struct** [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) _Firmware image header — stored at the start of each flash slot._ 
* **struct** [**SYN\_FwUpdate**](structSYN__FwUpdate.md) _Firmware update context - manages streaming writes to flash._ 
* **struct** [**SYN\_HB\_Peer**](structSYN__HB__Peer.md) _Tracked peer entry._ 
* **struct** [**SYN\_HMAC\_SHA256**](structSYN__HMAC__SHA256.md) _HMAC-SHA256 context — caller-owned._ 
* **struct** [**SYN\_Heartbeat**](structSYN__Heartbeat.md) _Heartbeat monitor — send keepalives, track peer liveness._ 
* **struct** [**SYN\_HttpClient**](structSYN__HttpClient.md) _HTTP client context structure._ 
* **struct** [**SYN\_HttpHeader**](structSYN__HttpHeader.md) _Key-value pair representing an HTTP header._ 
* **struct** [**SYN\_HttpResponse**](structSYN__HttpResponse.md) _HTTP response metadata._ 
* **struct** [**SYN\_Httpd**](structSYN__Httpd.md) _HTTP server context structure._ 
* **struct** [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) _Parsed HTTP request container presented to route handlers._ 
* **struct** [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) _HTTP response formatting state._ 
* **struct** [**SYN\_HttpdRoute**](structSYN__HttpdRoute.md) _An HTTP routing entry configuration._ 
* **struct** [**SYN\_Hysteresis**](structSYN__Hysteresis.md) _Hysteresis comparator with configurable dead-band._ 
* **struct** [**SYN\_I2CDev**](structSYN__I2CDev.md) _I2C device descriptor — bus number + 7-bit address._ 
* **struct** [**SYN\_I2C\_Config**](structSYN__I2C__Config.md) _I2C bus configuration._ 
* **struct** [**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) _Immediate-mode GUI context — inputs, navigation, style, layout._ 
* **struct** [**SYN\_IMGUI\_Layout**](structSYN__IMGUI__Layout.md) _Auto-layout state. Managed by syn\_imgui\_layout\_begin/end._ 
* **struct** [**SYN\_IMGUI\_Scroll**](structSYN__IMGUI__Scroll.md) _Scroll region state for scrollable IMGUI panels._ 
* **struct** [**SYN\_IMGUI\_Style**](structSYN__IMGUI__Style.md) _Visual style parameters for IMGUI widgets._ 
* **struct** [**SYN\_JsonReader**](structSYN__JsonReader.md) _JSON reader — token array + parse state._ 
* **struct** [**SYN\_JsonToken**](structSYN__JsonToken.md) _Parsed JSON token — key + value + type._ 
* **struct** [**SYN\_JsonWriter**](structSYN__JsonWriter.md) _Streaming JSON writer — tracks nesting, commas, and overflow._ 
* **struct** [**SYN\_LED**](structSYN__LED.md) _LED instance — pin, mode, blink/flash/pattern state._ 
* **struct** [**SYN\_LUT\_Entry**](structSYN__LUT__Entry.md) _Lookup table entry (x, y pair)._ 
* **struct** [**SYN\_LfsConfig**](structSYN__LfsConfig.md) _LittleFS partition configuration._ 
* **struct** [**SYN\_Mailbox**](structSYN__Mailbox.md) _Typed message queue — fixed-size SPSC ring buffer._ 
* **struct** [**SYN\_Mdns**](structSYN__Mdns.md) _mDNS responder instance._ 
* **struct** [**SYN\_Menu**](structSYN__Menu.md) _Menu runtime state — navigation stack and rendering._ 
* **struct** [**SYN\_MenuItem**](structSYN__MenuItem.md) _Menu item — label + action type + union payload._ 
* **struct** [**SYN\_MenuValueCfg**](structSYN__MenuValueCfg.md) _Value adjustment config for SYN\_MENU\_ACTION\_VALUE items._ 
* **struct** [**SYN\_Modbus**](structSYN__Modbus.md) _Modbus RTU slave instance — frame buffer, config, and statistics._ 
* **struct** [**SYN\_Modbus\_Config**](structSYN__Modbus__Config.md) _Modbus RTU slave configuration._ 
* **struct** [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) _Motor controller instance (opaque — use API to access)._ 
* **struct** [**SYN\_MotorCtrl\_Config**](structSYN__MotorCtrl__Config.md) _Motor controller configuration (passed to init, copied internally)._ 
* **struct** [**SYN\_MotorCtrl\_Metrics**](structSYN__MotorCtrl__Metrics.md) _Accumulated metrics for a single move/trajectory._ 
* **struct** [**SYN\_MotorCtrl\_Sample**](structSYN__MotorCtrl__Sample.md) _One sample of control-loop telemetry._ 
* **struct** [**SYN\_MotorCtrl\_Trajectory**](structSYN__MotorCtrl__Trajectory.md) _Trajectory setpoint for feedforward control._ 
* **struct** [**SYN\_MotorOutput**](structSYN__MotorOutput.md) _Abstract motor output interface (vtable)._ 
* **struct** [**SYN\_MqttClient**](structSYN__MqttClient.md) _MQTT client context structure._ 
* **struct** [**SYN\_PID**](structSYN__PID.md) _PID controller instance — config + accumulated state._ 
* **struct** [**SYN\_PID\_Config**](structSYN__PID__Config.md) _PID controller configuration._ 
* **struct** [**SYN\_PT**](structSYN__PT.md) _Protothread control block._ 
* **struct** [**SYN\_PT\_Sem**](structSYN__PT__Sem.md) _Counting semaphore for protothreads._ 
* **struct** [**SYN\_Packet**](structSYN__Packet.md) _Router packet — wire format header + payload buffer._ 
* **struct** [**SYN\_ParamSlotHeader**](structSYN__ParamSlotHeader.md) _Slot header — stored at the beginning of each parameter slot._ 
* **struct** [**SYN\_ParamStore**](structSYN__ParamStore.md) _Wear-leveled flash parameter store instance._ 
* **struct** [**SYN\_PendingAck**](structSYN__PendingAck.md) _Pending ACK entry for reliable delivery tracking._ 
* **struct** [**SYN\_PingPong**](structSYN__PingPong.md) _Ping-pong (double) buffer — zero-copy DMA pattern._ 
* **struct** [**SYN\_Pool**](structSYN__Pool.md) _Fixed-size block memory pool._ 
* **struct** [**SYN\_Power**](structSYN__Power.md) _Power monitor instance — ADC + hysteresis + brownout state._ 
* **struct** [**SYN\_Power\_Config**](structSYN__Power__Config.md) _Power monitor configuration._ 
* **struct** [**SYN\_ProfileEntry**](structSYN__ProfileEntry.md) _Per-task profile entry — timing stats and CPU usage._ 
* **struct** [**SYN\_Profiler**](structSYN__Profiler.md) _Task profiler instance._ 
* **struct** [**SYN\_PubSubBroker**](structSYN__PubSubBroker.md) _PubSub broker instance — subscription array + count._ 
* **struct** [**SYN\_PubSubSub**](structSYN__PubSubSub.md) _A single subscription record._ 
* **struct** [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) _Calendar date and time representation._ 
* **struct** [**SYN\_Ramp**](structSYN__Ramp.md) _Ramp generator instance — current value, target, rate._ 
* **struct** [**SYN\_RateLimit**](structSYN__RateLimit.md) _Token bucket rate limiter instance._ 
* **struct** [**SYN\_RingBuf**](structSYN__RingBuf.md) _Ring buffer control structure._ 
* **struct** [**SYN\_Router**](structSYN__Router.md) _Router instance — node ID, transport, handler table, ACK tracking._ 
* **struct** [**SYN\_RouterHandler**](structSYN__RouterHandler.md) _Handler registration entry — message type + callback._ 
* **struct** [**SYN\_SCurve**](structSYN__SCurve.md) _S-curve trajectory generator state._ 
* **struct** [**SYN\_SD**](structSYN__SD.md) _SD card driver handle (caller-owned, zero heap allocation)._ 
* **struct** [**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md) _SHA-1 hashing algorithm context._ 
* **struct** [**SYN\_SHA256**](structSYN__SHA256.md) _SHA-256 hash context — caller-owned._ 
* **struct** [**SYN\_SPIDev**](structSYN__SPIDev.md) _SPI device descriptor — bus, CS, read-bit convention._ 
* **struct** [**SYN\_SPI\_Config**](structSYN__SPI__Config.md) _SPI bus configuration._ 
* **struct** [**SYN\_Sched**](structSYN__Sched.md) _Scheduler control block._ 
* **struct** [**SYN\_Sensor**](structSYN__Sensor.md) _Sensor descriptor — owns the full read→filter→threshold pipeline._ 
* **struct** [**SYN\_SeqStep**](structSYN__SeqStep.md) 
* **struct** [**SYN\_Sequencer**](structSYN__Sequencer.md) _Sequencer runtime context._ 
* **struct** [**SYN\_Servo**](structSYN__Servo.md) _Hobby servo instance — pulse range, position, smooth movement._ 
* **struct** [**SYN\_Settings**](structSYN__Settings.md) _Persistent settings instance — flash-backed with change detection._ 
* **struct** [**SYN\_Signal**](structSYN__Signal.md) _Signal statistics — sliding window with incremental min/max/mean._ 
* **struct** [**SYN\_Sleep**](structSYN__Sleep.md) _Sleep coordinator — tracks wake locks and sleep statistics._ 
* **struct** [**SYN\_SockAddr**](structSYN__SockAddr.md) 
* **struct** [**SYN\_SoftI2C**](structSYN__SoftI2C.md) _Software I2C instance._ 
* **struct** [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) _1-Wire bus handle. Caller allocates; zero heap._ 
* **struct** [**SYN\_SoftPWM**](structSYN__SoftPWM.md) _Software PWM channel descriptor._ 
* **struct** [**SYN\_SoftSPI**](structSYN__SoftSPI.md) _Software SPI instance._ 
* **struct** [**SYN\_Stepper**](structSYN__Stepper.md) _Stepper motor controller context._ 
* **struct** [**SYN\_Task**](structSYN__Task.md) _Task descriptor — binds a protothread to scheduler metadata._ 
* **struct** [**SYN\_Timeout**](structSYN__Timeout.md) _Non-blocking timeout — tick wrap-around safe._ 
* **struct** [**SYN\_Timer**](structSYN__Timer.md) _Software timer descriptor._ 
* **struct** [**SYN\_Trace**](structSYN__Trace.md) _Circular trace event buffer._ 
* **struct** [**SYN\_TraceEntry**](structSYN__TraceEntry.md) _Single trace event entry._ 
* **struct** [**SYN\_Transport**](structSYN__Transport.md) _Abstract transport interface (send/receive function pointers)._ 
* **struct** [**SYN\_TransportTcp**](structSYN__TransportTcp.md) _TCP bridge transport layer context._ 
* **struct** [**SYN\_UART**](structSYN__UART.md) _UART driver handle._ 
* **struct** [**SYN\_Version**](structSYN__Version.md) _Compile-time version information._ 
* **struct** [**SYN\_VfsDir**](structSYN__VfsDir.md) _Open directory descriptor._ 
* **struct** [**SYN\_VfsDirEnt**](structSYN__VfsDirEnt.md) _Directory entry returned by_ [_**syn\_vfs\_readdir()**_](syn__vfs_8c.md#function-syn_vfs_readdir) _._
* **struct** [**SYN\_VfsFile**](structSYN__VfsFile.md) _Open file descriptor._ 
* **struct** [**SYN\_VfsMount**](structSYN__VfsMount.md) _Mount point binding a path prefix to a filesystem._ 
* **struct** [**SYN\_VfsOps**](structSYN__VfsOps.md) _VFS operations structure. Implement these for your filesystem (e.g. LittleFS)._ 
* **struct** [**SYN\_WDT\_Entry**](structSYN__WDT__Entry.md) _Single watchdog entry for task monitoring._ 
* **struct** [**SYN\_Watchdog**](structSYN__Watchdog.md) _Software watchdog — monitors task deadlines._ 
* **struct** [**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) _WebSocket session context._ 
* **struct** [**SYN\_WorkItem**](structSYN__WorkItem.md) 
* **struct** [**SYN\_WorkQueue**](structSYN__WorkQueue.md) 

