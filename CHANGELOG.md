# Changelog

All notable changes to SyntropicOS will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Calendar Versioning](https://calver.org/) (`YYYY.M.MINOR`).

---

## [Unreleased]

---

## [2026.11.0] - 2026-11-11

### Added
- **Priority-Inheriting Mutex (`syn_mutex`)**: Recursive zero-heap mutex primitive (`PT_MUTEX_LOCK`, `PT_MUTEX_TRYLOCK`, `syn_mutex_unlock`) with bounded transitive priority inheritance, cycle-safe demotion, automatic release on task exit or restart, and standalone/cross-scheduler owner support.
- **Unified AES, SHA-512/384, NIST P-256, Ed25519 Signing, HMAC-DRBG & TPM 2.0 (`syn_aes`, `syn_aes_cmac`, `syn_sha512`, `syn_p256`, `syn_ed25519`, `syn_hmac_drbg`, `syn_tpm2`, `syn_port_aes`)**:
  - Unified AES-128/192/256 engine supporting ECB, CBC (PKCS#7), CTR, GCM (AEAD), CCM/CCM-8, RFC 4493 AES-CMAC, and hardware acceleration port hooks (`syn_port_aes.h`, STM32 and ESP32).
  - FIPS 180-4 SHA-512 / SHA-384, RFC 4231 HMAC-SHA512 / HMAC-SHA384, and RFC 5869 / TLS 1.3 HKDF-SHA384 / HKDF-SHA512 key derivation (`syn_hkdf`).
  - Constant-time NIST P-256 (`secp256r1`) ECDH and ECDSA (including RFC 6979 deterministic signing), RFC 8032 Ed25519 key generation and signing, X.509 P-256/Ed25519 certificate chain verification (`syn_x509`), and NIST SP 800-90A HMAC-DRBG (`syn_hmac_drbg`).
  - Zero-heap TPM 2.0 command/response driver (`syn_tpm2`) for startup, self-test, TRNG, PCR read/extend, `TPM2_Quote` attestation, and NVRAM operations.
- **DTLS 1.3, CoAPS, DNS-SD, MQTT 5.0 / MQTT-SN & TLS 1.3 Cipher Suites (`syn_dtls`, `syn_coap`, `syn_dnssd`, `syn_mqtt`, `syn_mqttsn`, `syn_tls`)**:
  - RFC 9147 DTLS 1.3 datagram engine with 64-packet anti-replay sliding window, stateless `HelloRetryRequest` cookie verification, X.509 and PSK authentication, and CoAP-over-DTLS (`coaps://`) transport adapter.
  - CoAP RFC 7959 Block1/Block2 transfers and RFC 7641 Observe subscriptions (`syn_coap`), RFC 6763 / RFC 6762 mDNS/DNS-SD service responder and browser (`syn_dnssd`), MQTT v5.0 variable-byte integers and user properties (`syn_mqtt`), and OASIS MQTT-SN v1.2 client (`syn_mqttsn`).
  - Added TLS 1.3 X.509 authentication and AES-128/256-GCM, AES-CCM, and AES-CCM-8 cipher suites (`syn_tls`).
- **OMA LwM2M, RFC 9052 COSE & Streaming OTA (`syn_lwm2m`, `syn_lwm2m_task`, `syn_cose`, `syn_ota`, `syn_fwupdate`)**:
  - OMA LwM2M v1.1/v1.2 client engine (`syn_lwm2m`) and cooperative supervisor task (`syn_lwm2m_task`) supporting bootstrap, registration, IPSO object access, and Object 5 firmware updates.
  - RFC 9052 / RFC 9053 COSE signing, MAC, encryption, and key serialization (`COSE_Sign1`, `COSE_Mac0`, `COSE_Encrypt0`, `COSE_Key` across EdDSA, ES256, ChaCha20-Poly1305, AES-GCM, and HMAC-SHA256).
  - Zero-heap streaming OTA orchestrator (`syn_ota`) and Ed25519 / streaming AES-GCM firmware image verification (`syn_fwupdate`).
- **Industrial, Robotics, Audio & Encoding Subsystems (`syn_xrce_dds`, `syn_opcua`, `syn_iolink`, `syn_ethercat`, `syn_cia402`, `syn_gcode`, `syn_kinematics`, `syn_vad`, `syn_base64`)**:
  - Micro XRCE-DDS client with CDR codec (`syn_xrce_dds`), Micro OPC UA binary server (`syn_opcua`), IEC 61131-9 IO-Link master/device engine (`syn_iolink`), non-blocking EtherCAT master protothread engine (`syn_ethercat`), and expanded CiA 402 axis motion profiler (`syn_cia402`).
  - RS-274 G-code parser (`syn_gcode`), forward/inverse robot kinematics (`syn_kinematics`), Voice Activity Detection (`syn_vad`), and RFC 4648 Base64 / Base64URL codec (`syn_base64`).
- **Platform Flash Port & Configuration Manifest (`port_stm32_hal`, `syn_config_template.h`)**:
  - Added STM32F4 HAL flash sector erase and write drivers for `syn_settings` (`src/port/stm32_hal/`), and populated all 209 `SYN_USE_*` compile-time feature gates in `src/syntropic/syn_config_template.h`.

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
- **Module Header Relocations & Unified AES (`syn_aes`, `syn_sha256`, `syn_hmac`, `syn_dsp`, `syn_netbuf`)**:
  - Removed `syntropic/util/syn_aes128.h` (`SYN_AES128_Context`, `syn_aes128_*`) in favor of `syntropic/crypto/syn_aes.h` (`SYN_AES_Context`, `syn_aes_*`).
  - Relocated `syn_hmac.h` and `syn_sha256.h` from `syntropic/util/` to `syntropic/crypto/`, `syn_dsp.h` from `syntropic/util/` to `syntropic/dsp/`, and `syn_netbuf.h` from `syntropic/util/` to `syntropic/net/`.
- **USB EP0 Setup Capacity Parameter (`syn_usb`)**:
  - Added `uint16_t resp_cap` parameter to `syn_usb_process_setup()` and `SYN_USB_ClassDriver.setup` to bound EP0 control-stage responses, and increased `SYN_USB_EP0_BUF_SIZE` from `64` to `SYN_USB_MAX_CONFIG_DESC` (`256`).
- **Firmware Image Cryptographic Metadata Binding (`syn_fwimage`, `syn_fwupdate`)**:
  - Bound canonical 12-byte metadata (`SYN_FW_MAGIC || version_code || image_size` via `syn_fwimage_encode_auth_meta()`) into `SYN_FwImageHeader` HMAC-SHA256 tags, Ed25519 signatures, and AES-GCM AAD; firmware images signed or MAC'd under prior formats must be re-generated.
- **ISO-TP Default Padding & Mutable Buffer Signatures (`syn_isotp`, `syn_kwp2000`, `syn_httpd`, `syn_button`)**:
  - Changed default ISO 15765-2 TX CAN frame padding in `syn_isotp` to `0xCC` (`SYN_ISOTP_DEFAULT_PADDING`, configurable via `syn_isotp_set_padding()`).
  - Updated `syn_kwp2000_register_lid()` / `syn_kwp2000_register_cid()` `data` parameters from `const void *` to `void *`, `syn_httpd_read_body()` `resp` parameter to mutable `SYN_HttpdResponse *`, and `syn_button_combo_init()` `buttons` parameter to `SYN_Button *const *`.

### Fixed
- **Scheduler, Transitive Mutex PI, Timers & Sleep Coordinator (`syn_sched`, `syn_mutex`, `syn_ao`, `syn_sequencer`, `syn_timer`, `syn_timer_wheel`, `syn_watchdog`, `syn_mailbox`, `syn_workqueue`, `syn_sleep`)**:
  - Fixed transitive mutex priority inheritance, cycle-safe owner demotion, single-scheduler AMP mutex isolation (`SYN_INVALID_PARAM` on cross-scheduler contention), stale waiter pruning on task exit/restart/suspend/switch, self-restart + self-suspend state preservation, and preserved `SYN_SCHED_FLAG_MUTEX_WAIT` across recursive re-locks.
  - Added critical-section guards to `syn_task_create()`, `syn_sleep_enter()`, and tickless wakeup clamping; anchored expired `delay_until` timestamps across >24.85-day `SYN_TASK_SUSPENDED` intervals; and detected mid-slice `SYN_TASK_DEAD` slot respawns for `syn_task_should_share()`.
  - Capped single-dispatch event bursts in `syn_ao` to prevent self-post starvation, guarded step-0 re-initialization and `on_complete` loop toggles in `syn_sequencer`, rejected cross-wheel node corruption and callback re-initialization races in `syn_timer_wheel`, and prevented `capacity * size` multiplication overflow in `syn_mailbox` and `syn_workqueue`.
- **MCU Port Timers, UART, Flash & Critical Sections (`port_stm32f4`, `port_stm32_hal`, `port_ch32v307`, `port_rp2040`, `port_esp32`, `port_arduino`)**:
  - Fixed Cortex-M `syn_port_get_tick_us()` `+999 us` jump at `SYSTICK_VAL == 0` and integer-MHz truncation drift on `HCLK/8` clocks; flushed STM32F4 ART and STM32 HAL instruction/data caches across flash erase/write success and error paths; and added bounded `BSY` timeout and `PGERR`/`WRPRTERR` status checks to CH32V307 flash operations.
  - Unified `syn_port_uart_receive()` and `syn_port_uart_receive_byte()` `timeout_ms == 0` non-blocking poll semantics across RP2040, ESP32, and STM32 HAL (and fixed premature 1 ms loop termination on `timeout_ms > 0` in `port_stm32_hal`).
  - Restored CH32V307 ADC/DMA register layouts, masked `mstatus.MIE` across SysTick increments, and guarded `port_ch32v307.c` / `startup_ch32v307.c` with `!defined(ARDUINO)` so recursive Arduino IDE builds on WCH RISC-V cores do not collide with core startup symbols; added `SEV` on wake in RP2040; and saved/restored caller interrupt state, guarded W5100 Ethernet headers, prevented `syn_port_exti_configure()` trampoline slot exhaustion on non-interrupt pins, added `E2END` bounds validation to AVR EEPROM `syn_port_flash_*` helpers, and returned `SYN_NOT_IMPLEMENTED` (`SYN_WEAK`) on non-AVR Arduino flash stubs.
- **Motion Profiling, Rate Limiting, Time Discipline, RNG & Encoder Arithmetic (`syn_scurve`, `syn_ramp`, `syn_rate_limit`, `syn_random`, `syn_timesync`, `syn_encoder`)**:
  - Widened `syn_scurve_set_target()` distance and velocity-jerk intermediate math to 64-bit (`int64_t`/`uint64_t`) and replaced linear short-profile decrement loops with $O(\log N)$ binary search; replaced negative signed shifts in `syn_ramp` fixed-point accumulation with symmetric integer division so downward trapezoid ramps match upward deceleration profiles.
  - Guarded `syn_random_range(0, UINT32_MAX)` against `span + 1U` wraparound divide-by-zero, guarded `syn_rate_limit_allow()` against `interval_ms == 0` divide-by-zero and `max_tokens * elapsed` 32-bit overflow, widened `SYN_Encoder.sub_count` to `int16_t` for `steps_per_detent >= 128`, and hardened `syn_timesync` against skipped/glitched PPS drift spikes, `abs(INT32_MIN)` / 16-bit `int` truncation, and `uncertainty_ns` 32-bit wraparound.
- **16-Bit AVR Portability, Harvard PROGMEM Tables, Header-Only Device Helpers & CLI Stack Footprint (`syn_i2c_dev`, `syn_spi_dev`, `syn_cli`, `syn_crc`, `syn_aes`, `syn_sha256`, `syn_lwm2m`, `syn_pmbus`, `syn_tpm2`, `syn_canopen`, `syn_hmac_drbg`, `syn_coap`)**:
  - Updated `syn_i2c_dev.h` and `syn_spi_dev.h` inline register helpers to use `syn_port_i2c_transfer()` and `syn_port_gpio_write()` with `SYN_Status` error propagation and 16-bit-safe shifts.
  - Defaulted `SYN_CLI_LINE_BUF_SIZE` to `80` bytes and `SYN_CRC_USE_TABLE` to `0` on `__AVR__`, and moved `syn_aes` (`sbox`, `rsbox`, `rcon`) and `syn_sha256` (`K`) constant tables into AVR `PROGMEM` to save 2,315 bytes of SRAM on 2 KB ATmega328P targets.
  - Replaced `strtoll()` (absent in `avr-libc`) with overflow-checked `lwm2m_parse_i64()`, fixed `%lu` formatting and 32-bit TLV length shifts in `syn_lwm2m`, fixed `(uint32_t)1U << 16` shift overflow at `exponent == -16` in `syn_pmbus`, and guarded 32-bit shifts on 16-bit `size_t` targets across `syn_tpm2`, `syn_aes`, `syn_canopen`, `syn_hmac_drbg`, and `syn_coap`.
- **IEEE 802.3 CRC-32 Lookup Table (`syn_crc`)**: Corrected 12 corrupted entries in the standard IEEE 802.3 CRC-32 lookup table.
- **Storage, File Systems & Async Peripheral Queues (`syn_fat`, `syn_lfs`, `syn_vfs`, `syn_settings`, `syn_flash_wl`, `syn_i2c_queue`, `syn_spi_queue`)**:
  - Fixed FAT16/32 BPB geometry validation, cluster-chain cycle/premature-EOF checks, and seek bounds; preserved monotonic `next_seq` across dual-bank `syn_settings` fallback; rejected negative VFS seeks; and copied active transactions onto the stack in `syn_i2c_queue` and `syn_spi_queue` to prevent pointer invalidation on reentrant callbacks.
- **Network, Automotive & Industrial Protocol Engines (`syn_tcp`, `syn_udp`, `syn_eth`, `syn_dhcp`, `syn_dns`, `syn_dnssd`, `syn_isotp`, `syn_j1939`, `syn_n2k`, `syn_modbus`, `syn_modbus_tcp`, `syn_bacnet`, `syn_devicenet`, `syn_dlt645`, `syn_cjt188`, `syn_mbus`, `syn_ocpp`, `syn_ethercat`)**:
  - Fixed TCP sequence/RST/FIN window validation and partial-send socket cleanup, UDP IPv4 IHL options offset, DHCP `BOOTREQUEST` rejection, Ethernet multicast/IGMP dispatch, mDNS/DNS-SD `QCLASS`/`RCLASS`/`RCODE`/Goodbye `TTL == 0` filtering and case-insensitive `.local` service matching, ISO-TP CAN FD SF/FF bounds, J1939 TP RTS collision handling, NMEA 2000 Fast-Packet cross-PGN isolation, Modbus coil/TCP frame bounds, BACnet instance lookup, DL/T 645 / CJ/T 188 / M-Bus streaming resynchronization, and EtherCAT pure-input cyclic LRW framing.
- **WireGuard Transport Padding & Standalone Modularity (`syn_wg`, `syn_hkdf`, `syn_ethercat`)**: Trimmed 16-byte WireGuard transport padding on receive using inner IPv4/IPv6 header length (`syn_wg`), split `SYN_HKDF_SHA512_ENABLED` so HKDF-SHA256 builds standalone when `SYN_USE_SHA512=0`, and split `SYN_ECAT_COE_OD_ENABLED` so EtherCAT builds standalone when `SYN_USE_CANOPEN=0`.
- **UDS Service 0x19 (ReadDTCInformation)**: Fixed fallback response wire formatting for subfunctions `0x04`, `0x06`, `0x10`, `0x18`, and `0x19` to return authoritative `statusOfDTC` instead of `SYN_UDS_DTC_STATUS_AVAILABILITY_MASK`. Enforced NRC `0x31` (`RequestOutOfRange`) when requested DTC is not registered.
- **UDS DTC Lifecycle & State Machine**: Aligned `syn_uds_dtc_report_test_result()` and `syn_uds_dtc_start_operation_cycle()` with ISO 14229-1: set Fault Detection Counter to +127 on qualified failure and -128 on qualified pass; clear `pendingDTC` (bit 2) on operation cycle transition if preceding cycle completed without malfunction.
- **DTC Manager Demo (`examples/stm32_dtc_manager_demo`)**: Corrected wire format in callback handlers for subfunctions `0x04` and `0x06` to adhere to ISO 14229-1 Tables 258 and 260. Consolidated DTC status tracking to use `syn_uds_dtc_get_status()` as single source of truth.

### Security
- **Constant-Time ECC, AEAD Bounds, Zeroization & X.509 Signature Binding (`syn_p256`, `syn_ed25519`, `syn_x25519`, `syn_x509`, `syn_aes`, `syn_chacha20poly1305`)**:
  - Enforced constant-time scalar multiplication, RFC 8032 canonical `S < L` and point encoding checks in Ed25519, RFC 7748 all-zero shared-secret rejection across X25519 callers, 64-bit AEAD length guards in AES-GCM and ChaCha20-Poly1305, volatile-safe secret scrubbing (`syn_hmac_secure_zero`, `syn_blake2s_secure_zero`), and X.509 verification binding against `cert->sig_algo` (rejecting `SYN_X509_ALGO_UNKNOWN` mismatches).
- **TLS 1.3, DTLS 1.3, WireGuard & OTA Fail-Closed Guards (`syn_tls`, `syn_dtls`, `syn_wg`, `syn_ota`, `syn_fwupdate`)**:
  - Rejected `NULL` or zero-length PSK secrets in TLS 1.3 / DTLS 1.3, enforced handshake transcript reflection checks and AEAD sequence-number exhaustion limits, gated WireGuard response processing on `hs_valid` while scrubbing ephemeral handshake state on timeout/disconnect, verified `syn_aes_gcm_decrypt()` tag completion in `syn_fwupdate`, and failed closed with `SYN_OTA_ERR_NO_FLASH_SLOT` when `SYN_OTA_SLOT_AUTO` encounters an uninitialized active slot.
- **Diagnostic & Fieldbus Access Controls (`syn_doip`, `syn_kwp2000`, `syn_ccp`, `syn_canopen`)**:
  - Enforced ISO 13400 DoIP routing activation prior to diagnostic message forwarding, KWP2000 session and SecurityAccess checks on `WriteData` and download/transfer services, ASAM CCP privilege masks on memory/calibration commands, and CANopen read-only/write-only Object Dictionary SDO enforcement.
- **WebAssembly VM & Untrusted Frame Bounds (`syn_wasm`, `syn_ble`, `syn_usb`, `syn_dns`, `syn_dnssd`, `syn_http`, `syn_httpd`)**:
  - Hardened Wasm binary loader against malformed LEB128 varints, section/table/memory/data out-of-bounds indices, and call/value stack overflows; sized BLE HCI RX/TX buffers (`260B` / `261B`) for full 255-byte HCI events and 251-byte L2CAP ATT MTU payloads; and enforced DNS/DNS-SD backward-only compression-pointer hop/offset bounds, label validation, and HTTP/HTTPd header/body limits.

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
