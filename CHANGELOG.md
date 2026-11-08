# Changelog

All notable changes to SyntropicOS will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Calendar Versioning](https://calver.org/) (`YYYY.M.MINOR`).

---

## [Unreleased]

### Changed (Breaking)
- **Cooperative Scheduler & Protothread Yield Intent (`syn_pt`, `syn_task`, `syn_sched`)**:
  - Replaced ambiguous yield macros (`PT_YIELD`, `PT_YIELD_UNTIL`, `PT_DEFER` removed) with explicit intent macros `PT_PASS(pt)` (`PT_IDLE`), `PT_CONTINUE(pt)` (`PT_MORE`), and `PT_CONTINUE_IF_NEEDED(pt, task)` (`syn_task_should_share`).
  - Renamed `SYN_PT_Status` values `PT_WAITING` -> `PT_IDLE` (`0`) and `PT_YIELDED` -> `PT_MORE` (`1`), and aligned exit statuses with macro names so `PT_EXIT` returns `PT_EXITED` (`2`) and `PT_END` returns `PT_ENDED` (`3`).
  - Fixed `PT_SPAWN` to propagate child `PT_IDLE` and `PT_MORE` statuses unchanged instead of collapsing to `0`.
  - Added `SYN_Prio` enum (`SYN_PRIO_CRITICAL = 0`, `SYN_PRIO_HIGH = 1`, `SYN_PRIO_NORMAL = 2`, `SYN_PRIO_LOW = 3`), changed `SYN_SCHED_PRIO_LEVELS` default from `8` to `4`, updated `syn_task_create()`, `syn_task_boost_priority()`, and `syn_task_set_base_priority()` priority parameters from `uint8_t` to `SYN_Prio`, and removed `SYN_TASK_DEFERRED` and `SYN_TASK_WAITING` from `SYN_TaskState`.
  - Replaced strict-priority single-pass deferral in `syn_sched_run()` with ranked dispatch (`Overdue` -> `Work` -> `Poll` -> round reset, ordered by priority then round-robin within every rank) and configurable per-priority starvation windows (`syn_sched_set_max_wait`, defaults `1 / 5 / 15 / 50 ms`).
- **Active Object Task Binding (`syn_ao`)**:
  - Updated `syn_ao_init()` to accept caller-owned `SYN_Task *task` and `SYN_Prio priority` parameters (`syn_ao_init(ao, task, name, priority, transitions, initial_state, mailbox_buf, mailbox_cap)`).
  - Replaced embedded `SYN_Task task` field in `SYN_ActiveObject` with `SYN_Task *task` pointer, removed unused `SYN_PT pt` field, and enforced `mailbox_cap >= 2` (usable mailbox depth `mailbox_cap - 1`).

### Fixed
- **WireGuard Transport Padding & Standalone Modularity (`syn_wg`, `syn_hkdf`, `syn_ethercat`)**: Trimmed 16-byte WireGuard transport padding on receive using inner IPv4/IPv6 header length (`syn_wg`), split `SYN_HKDF_SHA512_ENABLED` so HKDF-SHA256 builds standalone when `SYN_USE_SHA512=0`, and split `SYN_ECAT_COE_OD_ENABLED` so EtherCAT builds standalone when `SYN_USE_CANOPEN=0`.
- **UDS Service 0x19 (ReadDTCInformation)**: Fixed fallback response wire formatting for subfunctions `0x04`, `0x06`, `0x10`, `0x18`, and `0x19` to return authoritative `statusOfDTC` instead of `SYN_UDS_DTC_STATUS_AVAILABILITY_MASK`. Enforced NRC `0x31` (`RequestOutOfRange`) when requested DTC is not registered.
- **UDS DTC Lifecycle & State Machine**: Aligned `syn_uds_dtc_report_test_result()` and `syn_uds_dtc_start_operation_cycle()` with ISO 14229-1: set Fault Detection Counter to +127 on qualified failure and -128 on qualified pass; clear `pendingDTC` (bit 2) on operation cycle transition if preceding cycle completed without malfunction.
- **DTC Manager Demo (`examples/stm32_dtc_manager_demo`)**: Corrected wire format in callback handlers for subfunctions `0x04` and `0x06` to adhere to ISO 14229-1 Tables 258 and 260. Consolidated DTC status tracking to use `syn_uds_dtc_get_status()` as single source of truth.

---

## [2026.8.0] - 2026-08-10

### Added
- **Bluetooth Low Energy (BLE) Host Stack (`syn_ble`)**: Zero-heap Tier 2 BLE Host Stack including H:4 UART transport (`syn_ble_hci`), L2CAP channel demuxing & ACL reassembly (`syn_ble_l2cap`), ATT protocol encoders/decoders (`syn_ble_att`), GAP advertising/connection engine (`syn_ble_gap`), and static attribute GATT server (`syn_ble_gatt`).
- **Audio & Signal Processing Subsystem (`syn_audio`)**: Pure C zero-heap audio pipeline featuring 16-channel audio mixer (`syn_audio_mixer`), RIFF/WAV header parser (`syn_wav`), IMA-ADPCM codec (`syn_adpcm`), Bluetooth Subband Codec (`syn_sbc`), and MFCC speech feature extraction (`syn_mfcc`).
- **OCPP 2.1 Edition 1 & OCPP-J 1.6 Protocol Engine (`syn_ocpp`)**: Full dual-role EVSE Client & CSMS Server protocol stack supporting JSON RPC 2.0 framing, ISO 15118-20 V2G data structures, DisplayMessage payloads, and automated CSMS integration tests.
- **USB Device Drivers (`syn_usb_midi`, `syn_usb_msc`)**: USB 2.0 MIDI Class driver (jack parsing, event packets) and USB Mass Storage Class (MSC) Bulk-Only Transport (BOT) / SCSI command block wrapper decoder.
- **ISO 14230-3 KWP2000 Diagnostic Stack (`syn_kwp2000`)**: K-Line / CAN diagnostic protocol engine with Session Control, ReadDataByLocalIdentifier, SecurityAccess, RoutineControl, and RequestDownload/TransferData.
- **ISO 14229-1 UDS Bootloader & Enhancements**: Extended `syn_uds` with multi-DID reading (`0x22`), post-TX ECU reset handlers (`0x11`), CommunicationControl (`0x28`), ControlDTCSetting (`0x85`), and added STM32F767 production A/B dual-bank swap bootloader example (`examples/stm32f7_uds_bootloader`).
- **Sensored FOC & Motor Control (`syn_foc_encoder`)**: Sensored FOC speed estimation, quadrature encoder feedback integration, 6-sector SVPWM dead-time compensation, and inverse Clarke/Park transforms.
- **Data Utilities & Security (`syn_lz4`, `syn_protobuf`, `syn_ymodem`, `syn_goertzel`, `syn_ntp_server`)**: Zero-heap LZ4 frame compression/decompression, Google Protocol Buffers varint/wire format encoder, YMODEM / XMODEM-1K serial file receiver, Goertzel DTMF tone detector, and NTP time server.
- **PlatformIO & Arduino Packaging Audit**: Updated `library.json`, `library.properties`, `sources.mk`, `CMakeLists.txt`, and header manifests for cross-platform Arduino IDE and PlatformIO builds.

---

## [2026.7.4] - 2026-07-27

### Added
- **AUTOSAR CAN Network Management (`syn_cannm`)**: 5-state non-blocking FSM (`BUS_SLEEP`, `PREPARE_BUS_SLEEP`, `REPEAT_MESSAGE`, `NORMAL_OPERATION`, `READY_SLEEP`) with configurable timers and zero dynamic memory allocation.

---

## [2026.7.3] - 2026-07-27

### Added
- **CJ/T 188 Protocol Engine (`syn_cjt188`)**: Smart metering protocol driver for water, heat, and gas meters.
- **DL/T 645 Protocol Engine (`syn_dlt645`)**: Electricity meter protocol implementation (1997 and 2007 standards).
- **M-Bus Protocol Engine (`syn_mbus`)**: EN 13757-2/3 master and slave communication stack.
- **Enhanced Button Controller (`syn_button`)**: Support for multi-click timing windows, hold auto-repeat, and multi-button combination event handlers (`SYN_ButtonCombo`).
- **Static Analysis & Sanitizer Infrastructure**: Integrated GCC `-fanalyzer`, ThreadSanitizer (TSan), Flawfinder, and containerized Valgrind Memcheck.

### Fixed
- Enforced production NULL and parameter validation guards across 70+ core, driver, net, storage, and protocol modules.
- Resolved frame length validation and buffer overflow bounds checks in Modbus, DLT645, CBOR, and COBS parsers.

---

## [2026.7.2] - 2026-07-25

### Added
- **EtherCAT Protocol Engine (`syn_ethercat`)**: Zero-allocation EtherCAT slave protocol stack integrated with OpenEtherCATSociety SOES.
- **PlatformIO Integration**: Official `library.json` manifest, top-level wrapper header, and automated CI publishing pipeline.
- **AES-128 FIPS 197 Cipher (`syn_aes128`)**: Constant-time AES-128 encryption/decryption with CBC mode and PKCS#7 padding.
- **Dual-Bank NVS Settings Storage (`syn_settings`)**: Transactional dual-bank non-volatile settings storage with wear-leveling and VFS export/import.
- **3D Jerk-Limited S-Curve Generator (`syn_scurve3d`)**: Synchronized 3-axis motion trajectory planner.
- **MISRA C:2023 Compliance & Renode Emulation**: Containerized MISRA C:2023 scanner and Renode board-level hardware emulation suite.

---

## [2026.7.1] - 2026-07-24

### Added
- **ISO 15765-2 (ISO-TP) Transport Stack (`syn_isotp`)**: Multi-frame CAN transport layer with block size (BS) and separation time (STmin) flow control, plus opt-in CAN FD support.
- **LIN Bus Protocol Stack (`syn_lin`, `syn_lintp`)**: LIN 2.1 / 2.2a master/slave protocol stack and LIN diagnostic transport layer.
- **USITT DMX512-A Protocol Engine (`syn_dmx512`)**: 512-channel lighting control master and slave protocol stack.
- **High-Precision Clock & Time Sync (`syn_hpclock`, `syn_timesync`)**: 64-bit system-clock precision timestamping primitive and PPS-disciplined time service.
- **Geodetic Coordinate Library (`syn_geo`)**: WGS84 ellipsoid conversions, Haversine/Vincenty distance calculations, and 3D local ENU transformations.
- **DALI Lighting Protocol Stack (`syn_dali`)**: IEC 62386 control gear and controller implementation.
- **NMEA 2000 & SAE J1939 CAN Stacks (`syn_n2k`, `syn_j1939`)**: Marine and heavy-duty vehicle CAN protocol engines with full PGN decoding.
- **O(1) Hashed Timing Wheel & Packet Pool**: Zero-copy packet buffer pool (`syn_netbuf`) and O(1) hashed timing wheel timer scheduler.

---

## [2026.7.0] - 2026-07-23

### Added
- **WireGuard VPN Client (`syn_wg`)**: Bare-metal WireGuard protocol implementation using Noise_IK handshake, ChaCha20-Poly1305, Curve25519, and BLAKE2s.
- **CANopen DS301 & CiA 402 Servo Profile (`syn_canopen`, `syn_cia402`)**: Complete CANopen slave engine with SDO/PDO management, Heartbeat, LSS (CiA 305), and CiA 402 motor drive FSM.
- **Sensorless FOC Motor Control (`syn_foc`)**: Field-Oriented Control with Park/Clarke transforms, Space Vector PWM (SVPWM), and Sliding Mode Observer (SMO) speed estimation.
- **Dynamic Biquad Filter Generator (`syn_filter`)**: Fixed-point IIR filter design generator for low-pass, high-pass, band-pass, and notch filtering.
- **Mahony 6-DOF AHRS Sensor Fusion (`syn_sensor`)**: Quaternion IMU orientation estimation filter.
- **Multi-Axis Interpolator (`syn_interpolator`)**: Linear, circular, and Bezier path motion planning primitives.
- **Event-Driven Task Blocking (`syn_sched`)**: Added `PT_BLOCK_EVENT`, `PT_DEFER`, and per-priority round-robin scheduling primitives.

---

## [2026.6.2] - 2026-06-30

### Added
- **AMP Multicore Support (`syn_multicore`)**: Symmetric/Asymmetric multiprocessing support with hardware spinlocks, execution barriers, and cross-core mailbox messaging (RP2040 support).
- **Crypto & Security Manager (`syn_sha256`, `syn_hkdf`, `syn_fwupdate`)**: SHA-256/HMAC digest engine, HKDF key derivation, and signed firmware verification.
- **Memory Pool Allocator (`syn_pool`)**: Fixed-block zero-fragmentation memory pool manager.
- **Core Dump & Diagnostic Logger (`syn_coredump`, `syn_errlog`)**: Non-volatile panic state persistence and fault analysis system.
- **Tickless Power Management (`syn_tickless`, `syn_power`)**: Low-power sleep mode integration and dynamic hardware timer suppression.

---

## [2026.6.1] - 2026-06-29

### Added
- **Initial Release of SyntropicOS**: Zero-heap cooperative embedded operating system and protocol framework for microcontrollers.
- **Core Cooperative Scheduler (`syn_sched`)**: Lightweight protothread engine based on Duff's device state machines.
- **Peripheral Drivers & Utilities**: Modular drivers for GPIO, UART, ADC, I2C, SPI, CAN, PWM, Timers, Ring Buffers, and Finite State Machines.
- **Arduino Library Support**: Packaged structure for single-header inclusion in Arduino IDE and PlatformIO environments.
