

# File syn\_config\_template.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**syn\_config\_template.h**](syn__config__template_8h.md)

[Go to the source code of this file](syn__config__template_8h_source.md)

_SyntropicOS configuration template._ [More...](#detailed-description)

































































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_AES\_GCM\_TABLE**](syn__config__template_8h.md#define-syn_aes_gcm_table)  `0`<br> |
| define  | [**SYN\_AES\_MAX\_KEY\_BITS**](syn__config__template_8h.md#define-syn_aes_max_key_bits)  `256`<br> |
| define  | [**SYN\_AUDIO\_MIXER\_MAX\_CHANNELS**](syn__config__template_8h.md#define-syn_audio_mixer_max_channels)  `4U`<br> |
| define  | [**SYN\_BLE\_HCI\_RX\_BUF\_SIZE**](syn__config__template_8h.md#define-syn_ble_hci_rx_buf_size)  `260U`<br> |
| define  | [**SYN\_BLE\_HCI\_TX\_BUF\_SIZE**](syn__config__template_8h.md#define-syn_ble_hci_tx_buf_size)  `261U`<br> |
| define  | [**SYN\_BLE\_MAX\_ATTRIBUTES**](syn__config__template_8h.md#define-syn_ble_max_attributes)  `32U`<br> |
| define  | [**SYN\_BLE\_MAX\_CONNECTIONS**](syn__config__template_8h.md#define-syn_ble_max_connections)  `2U`<br> |
| define  | [**SYN\_CLI\_HISTORY\_DEPTH**](syn__config__template_8h.md#define-syn_cli_history_depth)  `0`<br> |
| define  | [**SYN\_CLI\_LINE\_BUF\_SIZE**](syn__config__template_8h.md#define-syn_cli_line_buf_size)  `128`<br> |
| define  | [**SYN\_CLI\_MAX\_ARGS**](syn__config__template_8h.md#define-syn_cli_max_args)  `16`<br> |
| define  | [**SYN\_COREDUMP\_STACK\_SIZE**](syn__config__template_8h.md#define-syn_coredump_stack_size)  `128`<br> |
| define  | [**SYN\_CRC\_USE\_TABLE**](syn__config__template_8h.md#define-syn_crc_use_table)  `1`<br> |
| define  | [**SYN\_ETH\_ARP\_CACHE\_SIZE**](syn__config__template_8h.md#define-syn_eth_arp_cache_size)  `8`<br> |
| define  | [**SYN\_FILTER\_MAX\_WINDOW**](syn__config__template_8h.md#define-syn_filter_max_window)  `32`<br> |
| define  | [**SYN\_FW\_USE\_ED25519**](syn__config__template_8h.md#define-syn_fw_use_ed25519)  `1`<br> |
| define  | [**SYN\_FW\_USE\_HMAC**](syn__config__template_8h.md#define-syn_fw_use_hmac)  `0`<br> |
| define  | [**SYN\_GFX\_BACKEND**](syn__config__template_8h.md#define-syn_gfx_backend)  `[**SYN\_GFX\_BACKEND\_CANVAS**](syn__config__template_8h.md#define-syn_gfx_backend_canvas)`<br> |
| define  | [**SYN\_GFX\_BACKEND\_CANVAS**](syn__config__template_8h.md#define-syn_gfx_backend_canvas)  `0`<br> |
| define  | [**SYN\_GFX\_BACKEND\_DIRECT**](syn__config__template_8h.md#define-syn_gfx_backend_direct)  `1`<br> |
| define  | [**SYN\_GNSS\_USE\_FIXED\_POINT**](syn__config__template_8h.md#define-syn_gnss_use_fixed_point)  `0`<br> |
| define  | [**SYN\_LOG\_BUF\_SIZE**](syn__config__template_8h.md#define-syn_log_buf_size)  `192`<br> |
| define  | [**SYN\_LOG\_COLOR**](syn__config__template_8h.md#define-syn_log_color)  `0`<br> |
| define  | [**SYN\_LOG\_LEVEL**](syn__config__template_8h.md#define-syn_log_level)  `1`<br> |
| define  | [**SYN\_LOG\_TIMESTAMP**](syn__config__template_8h.md#define-syn_log_timestamp)  `1`<br> |
| define  | [**SYN\_LZ4\_HASH\_SIZE**](syn__config__template_8h.md#define-syn_lz4_hash_size)  `4096U`<br> |
| define  | [**SYN\_MUTEX\_MAX\_WAITERS**](syn__config__template_8h.md#define-syn_mutex_max_waiters)  `4U`<br> |
| define  | [**SYN\_P256\_USE\_COMB\_TABLE**](syn__config__template_8h.md#define-syn_p256_use_comb_table)  `0`<br> |
| define  | [**SYN\_SCHED\_MAX\_WAIT\_CRITICAL\_MS**](syn__config__template_8h.md#define-syn_sched_max_wait_critical_ms)  `1U`<br> |
| define  | [**SYN\_SCHED\_MAX\_WAIT\_HIGH\_MS**](syn__config__template_8h.md#define-syn_sched_max_wait_high_ms)  `5U`<br> |
| define  | [**SYN\_SCHED\_MAX\_WAIT\_LOW\_MS**](syn__config__template_8h.md#define-syn_sched_max_wait_low_ms)  `50U`<br> |
| define  | [**SYN\_SCHED\_MAX\_WAIT\_NORMAL\_MS**](syn__config__template_8h.md#define-syn_sched_max_wait_normal_ms)  `15U`<br> |
| define  | [**SYN\_SCHED\_PRIO\_LEVELS**](syn__config__template_8h.md#define-syn_sched_prio_levels)  `4`<br> |
| define  | [**SYN\_SPINLOCK\_COUNT**](syn__config__template_8h.md#define-syn_spinlock_count)  `4`<br> |
| define  | [**SYN\_UART\_MAX\_INSTANCES**](syn__config__template_8h.md#define-syn_uart_max_instances)  `2`<br> |
| define  | [**SYN\_UART\_RX\_BUF\_SIZE**](syn__config__template_8h.md#define-syn_uart_rx_buf_size)  `128`<br> |
| define  | [**SYN\_UART\_TX\_BUF\_SIZE**](syn__config__template_8h.md#define-syn_uart_tx_buf_size)  `128`<br> |
| define  | [**SYN\_USE\_ACTUATOR**](syn__config__template_8h.md#define-syn_use_actuator)  `1`<br> |
| define  | [**SYN\_USE\_ADC**](syn__config__template_8h.md#define-syn_use_adc)  `1`<br> |
| define  | [**SYN\_USE\_ADPCM**](syn__config__template_8h.md#define-syn_use_adpcm)  `1`<br> |
| define  | [**SYN\_USE\_AES**](syn__config__template_8h.md#define-syn_use_aes)  `1`<br> |
| define  | [**SYN\_USE\_AES\_CBC**](syn__config__template_8h.md#define-syn_use_aes_cbc)  `1`<br> |
| define  | [**SYN\_USE\_AES\_CCM**](syn__config__template_8h.md#define-syn_use_aes_ccm)  `1`<br> |
| define  | [**SYN\_USE\_AES\_CMAC**](syn__config__template_8h.md#define-syn_use_aes_cmac)  `1`<br> |
| define  | [**SYN\_USE\_AES\_CTR**](syn__config__template_8h.md#define-syn_use_aes_ctr)  `1`<br> |
| define  | [**SYN\_USE\_AES\_DECRYPT**](syn__config__template_8h.md#define-syn_use_aes_decrypt)  `1`<br> |
| define  | [**SYN\_USE\_AES\_GCM**](syn__config__template_8h.md#define-syn_use_aes_gcm)  `1`<br> |
| define  | [**SYN\_USE\_AO**](syn__config__template_8h.md#define-syn_use_ao)  `1`<br> |
| define  | [**SYN\_USE\_ASN1**](syn__config__template_8h.md#define-syn_use_asn1)  `1`<br> |
| define  | [**SYN\_USE\_AT\_PARSER**](syn__config__template_8h.md#define-syn_use_at_parser)  `1`<br> |
| define  | [**SYN\_USE\_AUDIO**](syn__config__template_8h.md#define-syn_use_audio)  `1`<br> |
| define  | [**SYN\_USE\_AUTOIP**](syn__config__template_8h.md#define-syn_use_autoip)  `1`<br> |
| define  | [**SYN\_USE\_AUTOTUNE**](syn__config__template_8h.md#define-syn_use_autotune)  `1`<br> |
| define  | [**SYN\_USE\_BACNET**](syn__config__template_8h.md#define-syn_use_bacnet)  `1`<br> |
| define  | [**SYN\_USE\_BASE64**](syn__config__template_8h.md#define-syn_use_base64)  `1`<br> |
| define  | [**SYN\_USE\_BIOMETRIC**](syn__config__template_8h.md#define-syn_use_biometric)  `1`<br> |
| define  | [**SYN\_USE\_BIQUAD**](syn__config__template_8h.md#define-syn_use_biquad)  `1`<br> |
| define  | [**SYN\_USE\_BLACKBOX**](syn__config__template_8h.md#define-syn_use_blackbox)  `1`<br> |
| define  | [**SYN\_USE\_BLAKE2S**](syn__config__template_8h.md#define-syn_use_blake2s)  `1`<br> |
| define  | [**SYN\_USE\_BLDC\_6STEP**](syn__config__template_8h.md#define-syn_use_bldc_6step)  `1`<br> |
| define  | [**SYN\_USE\_BLE**](syn__config__template_8h.md#define-syn_use_ble)  `1`<br> |
| define  | [**SYN\_USE\_BOOT**](syn__config__template_8h.md#define-syn_use_boot)  `1`<br> |
| define  | [**SYN\_USE\_BUTTON**](syn__config__template_8h.md#define-syn_use_button)  `1`<br> |
| define  | [**SYN\_USE\_BUZZER**](syn__config__template_8h.md#define-syn_use_buzzer)  `1`<br> |
| define  | [**SYN\_USE\_CAN**](syn__config__template_8h.md#define-syn_use_can)  `1`<br> |
| define  | [**SYN\_USE\_CANNM**](syn__config__template_8h.md#define-syn_use_cannm)  `1`<br> |
| define  | [**SYN\_USE\_CANOPEN**](syn__config__template_8h.md#define-syn_use_canopen)  `1`<br> |
| define  | [**SYN\_USE\_CANVAS**](syn__config__template_8h.md#define-syn_use_canvas)  `1`<br> |
| define  | [**SYN\_USE\_CAN\_FD**](syn__config__template_8h.md#define-syn_use_can_fd)  `0`<br> |
| define  | [**SYN\_USE\_CBOR**](syn__config__template_8h.md#define-syn_use_cbor)  `1`<br> |
| define  | [**SYN\_USE\_CCP**](syn__config__template_8h.md#define-syn_use_ccp)  `1`<br> |
| define  | [**SYN\_USE\_CHACHA20POLY1305**](syn__config__template_8h.md#define-syn_use_chacha20poly1305)  `1`<br> |
| define  | [**SYN\_USE\_CHARLCD**](syn__config__template_8h.md#define-syn_use_charlcd)  `1`<br> |
| define  | [**SYN\_USE\_CIA402**](syn__config__template_8h.md#define-syn_use_cia402)  `1`<br> |
| define  | [**SYN\_USE\_CJT188**](syn__config__template_8h.md#define-syn_use_cjt188)  `1`<br> |
| define  | [**SYN\_USE\_CLI**](syn__config__template_8h.md#define-syn_use_cli)  `1`<br> |
| define  | [**SYN\_USE\_CLIMATE**](syn__config__template_8h.md#define-syn_use_climate)  `1`<br> |
| define  | [**SYN\_USE\_COAP**](syn__config__template_8h.md#define-syn_use_coap)  `1`<br> |
| define  | [**SYN\_USE\_COBS**](syn__config__template_8h.md#define-syn_use_cobs)  `1`<br> |
| define  | [**SYN\_USE\_COMP**](syn__config__template_8h.md#define-syn_use_comp)  `1`<br> |
| define  | [**SYN\_USE\_COREDUMP**](syn__config__template_8h.md#define-syn_use_coredump)  `0`<br> |
| define  | [**SYN\_USE\_COSE**](syn__config__template_8h.md#define-syn_use_cose)  `1`<br> |
| define  | [**SYN\_USE\_CRSF**](syn__config__template_8h.md#define-syn_use_crsf)  `1`<br> |
| define  | [**SYN\_USE\_DAC**](syn__config__template_8h.md#define-syn_use_dac)  `1`<br> |
| define  | [**SYN\_USE\_DALI**](syn__config__template_8h.md#define-syn_use_dali)  `1`<br> |
| define  | [**SYN\_USE\_DATALOG**](syn__config__template_8h.md#define-syn_use_datalog)  `1`<br> |
| define  | [**SYN\_USE\_DC\_MOTOR**](syn__config__template_8h.md#define-syn_use_dc_motor)  `1`<br> |
| define  | [**SYN\_USE\_DDS**](syn__config__template_8h.md#define-syn_use_dds)  `1`<br> |
| define  | [**SYN\_USE\_DEVICENET**](syn__config__template_8h.md#define-syn_use_devicenet)  `1`<br> |
| define  | [**SYN\_USE\_DHCP**](syn__config__template_8h.md#define-syn_use_dhcp)  `1`<br> |
| define  | [**SYN\_USE\_DIPSWITCH**](syn__config__template_8h.md#define-syn_use_dipswitch)  `1`<br> |
| define  | [**SYN\_USE\_DISTANCE**](syn__config__template_8h.md#define-syn_use_distance)  `1`<br> |
| define  | [**SYN\_USE\_DMA**](syn__config__template_8h.md#define-syn_use_dma)  `0`<br> |
| define  | [**SYN\_USE\_DMX512**](syn__config__template_8h.md#define-syn_use_dmx512)  `1`<br> |
| define  | [**SYN\_USE\_DNS**](syn__config__template_8h.md#define-syn_use_dns)  `1`<br> |
| define  | [**SYN\_USE\_DOIP**](syn__config__template_8h.md#define-syn_use_doip)  `1`<br> |
| define  | [**SYN\_USE\_DSHOT**](syn__config__template_8h.md#define-syn_use_dshot)  `1`<br> |
| define  | [**SYN\_USE\_DSHOT\_TELEMETRY**](syn__config__template_8h.md#define-syn_use_dshot_telemetry)  `1`<br> |
| define  | [**SYN\_USE\_DTLS**](syn__config__template_8h.md#define-syn_use_dtls)  `1`<br> |
| define  | [**SYN\_USE\_ED25519**](syn__config__template_8h.md#define-syn_use_ed25519)  `1`<br> |
| define  | [**SYN\_USE\_ENCODER**](syn__config__template_8h.md#define-syn_use_encoder)  `1`<br> |
| define  | [**SYN\_USE\_ERRLOG**](syn__config__template_8h.md#define-syn_use_errlog)  `1`<br> |
| define  | [**SYN\_USE\_ETH**](syn__config__template_8h.md#define-syn_use_eth)  `1`<br> |
| define  | [**SYN\_USE\_ETHERCAT**](syn__config__template_8h.md#define-syn_use_ethercat)  `1`<br> |
| define  | [**SYN\_USE\_EVENT**](syn__config__template_8h.md#define-syn_use_event)  `1`<br> |
| define  | [**SYN\_USE\_EXTI**](syn__config__template_8h.md#define-syn_use_exti)  `1`<br> |
| define  | [**SYN\_USE\_FAT**](syn__config__template_8h.md#define-syn_use_fat)  `1`<br> |
| define  | [**SYN\_USE\_FAULT**](syn__config__template_8h.md#define-syn_use_fault)  `1`<br> |
| define  | [**SYN\_USE\_FFT**](syn__config__template_8h.md#define-syn_use_fft)  `1`<br> |
| define  | [**SYN\_USE\_FILTER**](syn__config__template_8h.md#define-syn_use_filter)  `1`<br> |
| define  | [**SYN\_USE\_FLIGHT\_PID**](syn__config__template_8h.md#define-syn_use_flight_pid)  `1`<br> |
| define  | [**SYN\_USE\_FMT**](syn__config__template_8h.md#define-syn_use_fmt)  `1`<br> |
| define  | [**SYN\_USE\_FOC**](syn__config__template_8h.md#define-syn_use_foc)  `1`<br> |
| define  | [**SYN\_USE\_FSM**](syn__config__template_8h.md#define-syn_use_fsm)  `1`<br> |
| define  | [**SYN\_USE\_GBT27930**](syn__config__template_8h.md#define-syn_use_gbt27930)  `1`<br> |
| define  | [**SYN\_USE\_GCODE**](syn__config__template_8h.md#define-syn_use_gcode)  `1`<br> |
| define  | [**SYN\_USE\_GEO**](syn__config__template_8h.md#define-syn_use_geo)  `1`<br> |
| define  | [**SYN\_USE\_GOERTZEL**](syn__config__template_8h.md#define-syn_use_goertzel)  `1`<br> |
| define  | [**SYN\_USE\_GPIO**](syn__config__template_8h.md#define-syn_use_gpio)  `1`<br> |
| define  | [**SYN\_USE\_HEARTBEAT**](syn__config__template_8h.md#define-syn_use_heartbeat)  `1`<br> |
| define  | [**SYN\_USE\_HKDF**](syn__config__template_8h.md#define-syn_use_hkdf)  `1`<br> |
| define  | [**SYN\_USE\_HMAC\_DRBG**](syn__config__template_8h.md#define-syn_use_hmac_drbg)  `1`<br> |
| define  | [**SYN\_USE\_HPCLOCK**](syn__config__template_8h.md#define-syn_use_hpclock)  `1`<br> |
| define  | [**SYN\_USE\_HRPWM**](syn__config__template_8h.md#define-syn_use_hrpwm)  `1`<br> |
| define  | [**SYN\_USE\_HTTP**](syn__config__template_8h.md#define-syn_use_http)  `1`<br> |
| define  | [**SYN\_USE\_HTTPD**](syn__config__template_8h.md#define-syn_use_httpd)  `1`<br> |
| define  | [**SYN\_USE\_HWWDT**](syn__config__template_8h.md#define-syn_use_hwwdt)  `1`<br> |
| define  | [**SYN\_USE\_I2C**](syn__config__template_8h.md#define-syn_use_i2c)  `1`<br> |
| define  | [**SYN\_USE\_I2C\_ASYNC**](syn__config__template_8h.md#define-syn_use_i2c_async)  `0`<br> |
| define  | [**SYN\_USE\_IBUS**](syn__config__template_8h.md#define-syn_use_ibus)  `1`<br> |
| define  | [**SYN\_USE\_ICMP**](syn__config__template_8h.md#define-syn_use_icmp)  `1`<br> |
| define  | [**SYN\_USE\_IGMP**](syn__config__template_8h.md#define-syn_use_igmp)  `1`<br> |
| define  | [**SYN\_USE\_IMGUI**](syn__config__template_8h.md#define-syn_use_imgui)  `1`<br> |
| define  | [**SYN\_USE\_INTERPOLATOR**](syn__config__template_8h.md#define-syn_use_interpolator)  `1`<br> |
| define  | [**SYN\_USE\_IOEXP**](syn__config__template_8h.md#define-syn_use_ioexp)  `1`<br> |
| define  | [**SYN\_USE\_IOLINK**](syn__config__template_8h.md#define-syn_use_iolink)  `1`<br> |
| define  | [**SYN\_USE\_IR**](syn__config__template_8h.md#define-syn_use_ir)  `1`<br> |
| define  | [**SYN\_USE\_ISOTP**](syn__config__template_8h.md#define-syn_use_isotp)  `1`<br> |
| define  | [**SYN\_USE\_J1939**](syn__config__template_8h.md#define-syn_use_j1939)  `1`<br> |
| define  | [**SYN\_USE\_JOYSTICK**](syn__config__template_8h.md#define-syn_use_joystick)  `1`<br> |
| define  | [**SYN\_USE\_JSON**](syn__config__template_8h.md#define-syn_use_json)  `1`<br> |
| define  | [**SYN\_USE\_KALMAN**](syn__config__template_8h.md#define-syn_use_kalman)  `1`<br> |
| define  | [**SYN\_USE\_KEYPAD**](syn__config__template_8h.md#define-syn_use_keypad)  `1`<br> |
| define  | [**SYN\_USE\_KINEMATICS**](syn__config__template_8h.md#define-syn_use_kinematics)  `1`<br> |
| define  | [**SYN\_USE\_LED**](syn__config__template_8h.md#define-syn_use_led)  `1`<br> |
| define  | [**SYN\_USE\_LFS**](syn__config__template_8h.md#define-syn_use_lfs)  `1`<br> |
| define  | [**SYN\_USE\_LIN**](syn__config__template_8h.md#define-syn_use_lin)  `1`<br> |
| define  | [**SYN\_USE\_LINTP**](syn__config__template_8h.md#define-syn_use_lintp)  `1`<br> |
| define  | [**SYN\_USE\_LOG**](syn__config__template_8h.md#define-syn_use_log)  `1`<br> |
| define  | [**SYN\_USE\_LUX**](syn__config__template_8h.md#define-syn_use_lux)  `1`<br> |
| define  | [**SYN\_USE\_LWM2M**](syn__config__template_8h.md#define-syn_use_lwm2m)  `1`<br> |
| define  | [**SYN\_USE\_LWM2M\_TASK**](syn__config__template_8h.md#define-syn_use_lwm2m_task)  `1`<br> |
| define  | [**SYN\_USE\_LZ4**](syn__config__template_8h.md#define-syn_use_lz4)  `1`<br> |
| define  | [**SYN\_USE\_MATRIX**](syn__config__template_8h.md#define-syn_use_matrix)  `1`<br> |
| define  | [**SYN\_USE\_MAVLINK**](syn__config__template_8h.md#define-syn_use_mavlink)  `1`<br> |
| define  | [**SYN\_USE\_MBUS**](syn__config__template_8h.md#define-syn_use_mbus)  `1`<br> |
| define  | [**SYN\_USE\_MENU**](syn__config__template_8h.md#define-syn_use_menu)  `1`<br> |
| define  | [**SYN\_USE\_METRICS**](syn__config__template_8h.md#define-syn_use_metrics)  `0`<br> |
| define  | [**SYN\_USE\_MFCC**](syn__config__template_8h.md#define-syn_use_mfcc)  `1`<br> |
| define  | [**SYN\_USE\_MODBUS**](syn__config__template_8h.md#define-syn_use_modbus)  `1`<br> |
| define  | [**SYN\_USE\_MODBUS\_MASTER**](syn__config__template_8h.md#define-syn_use_modbus_master)  `1`<br> |
| define  | [**SYN\_USE\_MOTOR\_CTRL**](syn__config__template_8h.md#define-syn_use_motor_ctrl)  `1`<br> |
| define  | [**SYN\_USE\_MQTT**](syn__config__template_8h.md#define-syn_use_mqtt)  `1`<br> |
| define  | [**SYN\_USE\_MQTTSN**](syn__config__template_8h.md#define-syn_use_mqttsn)  `1`<br> |
| define  | [**SYN\_USE\_MSP**](syn__config__template_8h.md#define-syn_use_msp)  `1`<br> |
| define  | [**SYN\_USE\_MULTICORE**](syn__config__template_8h.md#define-syn_use_multicore)  `0`<br> |
| define  | [**SYN\_USE\_MUTEX**](syn__config__template_8h.md#define-syn_use_mutex)  `1`<br> |
| define  | [**SYN\_USE\_N2K**](syn__config__template_8h.md#define-syn_use_n2k)  `1`<br> |
| define  | [**SYN\_USE\_NETCFG**](syn__config__template_8h.md#define-syn_use_netcfg)  `1`<br> |
| define  | [**SYN\_USE\_NMEA**](syn__config__template_8h.md#define-syn_use_nmea)  `1`<br> |
| define  | [**SYN\_USE\_NTP\_SERVER**](syn__config__template_8h.md#define-syn_use_ntp_server)  `1`<br> |
| define  | [**SYN\_USE\_OLED**](syn__config__template_8h.md#define-syn_use_oled)  `1`<br> |
| define  | [**SYN\_USE\_ONEWIRE**](syn__config__template_8h.md#define-syn_use_onewire)  `1`<br> |
| define  | [**SYN\_USE\_OPCUA**](syn__config__template_8h.md#define-syn_use_opcua)  `1`<br> |
| define  | [**SYN\_USE\_OTA**](syn__config__template_8h.md#define-syn_use_ota)  `1`<br> |
| define  | [**SYN\_USE\_P256**](syn__config__template_8h.md#define-syn_use_p256)  `1`<br> |
| define  | [**SYN\_USE\_PARAM**](syn__config__template_8h.md#define-syn_use_param)  `1`<br> |
| define  | [**SYN\_USE\_PID**](syn__config__template_8h.md#define-syn_use_pid)  `1`<br> |
| define  | [**SYN\_USE\_PMBUS**](syn__config__template_8h.md#define-syn_use_pmbus)  `1`<br> |
| define  | [**SYN\_USE\_POOL**](syn__config__template_8h.md#define-syn_use_pool)  `1`<br> |
| define  | [**SYN\_USE\_PORT\_AES**](syn__config__template_8h.md#define-syn_use_port_aes)  `0`<br> |
| define  | [**SYN\_USE\_POWER**](syn__config__template_8h.md#define-syn_use_power)  `1`<br> |
| define  | [**SYN\_USE\_POWERMON**](syn__config__template_8h.md#define-syn_use_powermon)  `1`<br> |
| define  | [**SYN\_USE\_PPM**](syn__config__template_8h.md#define-syn_use_ppm)  `1`<br> |
| define  | [**SYN\_USE\_PROFILER**](syn__config__template_8h.md#define-syn_use_profiler)  `1`<br> |
| define  | [**SYN\_USE\_PROTOBUF**](syn__config__template_8h.md#define-syn_use_protobuf)  `1`<br> |
| define  | [**SYN\_USE\_PT**](syn__config__template_8h.md#define-syn_use_pt)  `1`<br> |
| define  | [**SYN\_USE\_PUBSUB**](syn__config__template_8h.md#define-syn_use_pubsub)  `1`<br> |
| define  | [**SYN\_USE\_PWM**](syn__config__template_8h.md#define-syn_use_pwm)  `1`<br> |
| define  | [**SYN\_USE\_QMATH**](syn__config__template_8h.md#define-syn_use_qmath)  `1`<br> |
| define  | [**SYN\_USE\_RAMP**](syn__config__template_8h.md#define-syn_use_ramp)  `1`<br> |
| define  | [**SYN\_USE\_RC\_CURVE**](syn__config__template_8h.md#define-syn_use_rc_curve)  `1`<br> |
| define  | [**SYN\_USE\_RC\_FAILSAFE**](syn__config__template_8h.md#define-syn_use_rc_failsafe)  `1`<br> |
| define  | [**SYN\_USE\_RFID**](syn__config__template_8h.md#define-syn_use_rfid)  `1`<br> |
| define  | [**SYN\_USE\_ROUTER**](syn__config__template_8h.md#define-syn_use_router)  `1`<br> |
| define  | [**SYN\_USE\_RTC**](syn__config__template_8h.md#define-syn_use_rtc)  `1`<br> |
| define  | [**SYN\_USE\_SBC**](syn__config__template_8h.md#define-syn_use_sbc)  `1`<br> |
| define  | [**SYN\_USE\_SBUS**](syn__config__template_8h.md#define-syn_use_sbus)  `1`<br> |
| define  | [**SYN\_USE\_SCALE**](syn__config__template_8h.md#define-syn_use_scale)  `1`<br> |
| define  | [**SYN\_USE\_SCHED**](syn__config__template_8h.md#define-syn_use_sched)  `1`<br> |
| define  | [**SYN\_USE\_SCURVE**](syn__config__template_8h.md#define-syn_use_scurve)  `1`<br> |
| define  | [**SYN\_USE\_SD**](syn__config__template_8h.md#define-syn_use_sd)  `1`<br> |
| define  | [**SYN\_USE\_SEG7**](syn__config__template_8h.md#define-syn_use_seg7)  `1`<br> |
| define  | [**SYN\_USE\_SENSOR**](syn__config__template_8h.md#define-syn_use_sensor)  `1`<br> |
| define  | [**SYN\_USE\_SEQUENCER**](syn__config__template_8h.md#define-syn_use_sequencer)  `1`<br> |
| define  | [**SYN\_USE\_SERVO**](syn__config__template_8h.md#define-syn_use_servo)  `1`<br> |
| define  | [**SYN\_USE\_SETTINGS**](syn__config__template_8h.md#define-syn_use_settings)  `1`<br> |
| define  | [**SYN\_USE\_SHA256**](syn__config__template_8h.md#define-syn_use_sha256)  `1`<br> |
| define  | [**SYN\_USE\_SHA512**](syn__config__template_8h.md#define-syn_use_sha512)  `1`<br> |
| define  | [**SYN\_USE\_SHIFTREG**](syn__config__template_8h.md#define-syn_use_shiftreg)  `1`<br> |
| define  | [**SYN\_USE\_SIGNAL**](syn__config__template_8h.md#define-syn_use_signal)  `1`<br> |
| define  | [**SYN\_USE\_SMARTLED**](syn__config__template_8h.md#define-syn_use_smartled)  `1`<br> |
| define  | [**SYN\_USE\_SMBUS**](syn__config__template_8h.md#define-syn_use_smbus)  `1`<br> |
| define  | [**SYN\_USE\_SNTP**](syn__config__template_8h.md#define-syn_use_sntp)  `1`<br> |
| define  | [**SYN\_USE\_SOFT\_I2C**](syn__config__template_8h.md#define-syn_use_soft_i2c)  `1`<br> |
| define  | [**SYN\_USE\_SOFT\_PWM**](syn__config__template_8h.md#define-syn_use_soft_pwm)  `1`<br> |
| define  | [**SYN\_USE\_SOFT\_SPI**](syn__config__template_8h.md#define-syn_use_soft_spi)  `1`<br> |
| define  | [**SYN\_USE\_SPI**](syn__config__template_8h.md#define-syn_use_spi)  `1`<br> |
| define  | [**SYN\_USE\_SPI\_ASYNC**](syn__config__template_8h.md#define-syn_use_spi_async)  `0`<br> |
| define  | [**SYN\_USE\_STEPPER**](syn__config__template_8h.md#define-syn_use_stepper)  `1`<br> |
| define  | [**SYN\_USE\_TICKLESS**](syn__config__template_8h.md#define-syn_use_tickless)  `0`<br> |
| define  | [**SYN\_USE\_TIMER**](syn__config__template_8h.md#define-syn_use_timer)  `1`<br> |
| define  | [**SYN\_USE\_TIMESYNC**](syn__config__template_8h.md#define-syn_use_timesync)  `1`<br> |
| define  | [**SYN\_USE\_TLS**](syn__config__template_8h.md#define-syn_use_tls)  `1`<br> |
| define  | [**SYN\_USE\_TOUCH**](syn__config__template_8h.md#define-syn_use_touch)  `1`<br> |
| define  | [**SYN\_USE\_TPM2**](syn__config__template_8h.md#define-syn_use_tpm2)  `1`<br> |
| define  | [**SYN\_USE\_TRACE**](syn__config__template_8h.md#define-syn_use_trace)  `1`<br> |
| define  | [**SYN\_USE\_TRANSPORT\_TCP**](syn__config__template_8h.md#define-syn_use_transport_tcp)  `1`<br> |
| define  | [**SYN\_USE\_TRANSPORT\_UDP**](syn__config__template_8h.md#define-syn_use_transport_udp)  `1`<br> |
| define  | [**SYN\_USE\_UART**](syn__config__template_8h.md#define-syn_use_uart)  `1`<br> |
| define  | [**SYN\_USE\_UDS**](syn__config__template_8h.md#define-syn_use_uds)  `1`<br> |
| define  | [**SYN\_USE\_USB\_CDC**](syn__config__template_8h.md#define-syn_use_usb_cdc)  `1`<br> |
| define  | [**SYN\_USE\_USB\_MIDI**](syn__config__template_8h.md#define-syn_use_usb_midi)  `1`<br> |
| define  | [**SYN\_USE\_USB\_MSC**](syn__config__template_8h.md#define-syn_use_usb_msc)  `1`<br> |
| define  | [**SYN\_USE\_VAD**](syn__config__template_8h.md#define-syn_use_vad)  `1`<br> |
| define  | [**SYN\_USE\_VFS**](syn__config__template_8h.md#define-syn_use_vfs)  `1`<br> |
| define  | [**SYN\_USE\_WASM**](syn__config__template_8h.md#define-syn_use_wasm)  `1`<br> |
| define  | [**SYN\_USE\_WATCHDOG**](syn__config__template_8h.md#define-syn_use_watchdog)  `1`<br> |
| define  | [**SYN\_USE\_WEBSOCKET**](syn__config__template_8h.md#define-syn_use_websocket)  `1`<br> |
| define  | [**SYN\_USE\_WG**](syn__config__template_8h.md#define-syn_use_wg)  `1`<br> |
| define  | [**SYN\_USE\_WORKQUEUE**](syn__config__template_8h.md#define-syn_use_workqueue)  `1`<br> |
| define  | [**SYN\_USE\_X25519**](syn__config__template_8h.md#define-syn_use_x25519)  `1`<br> |
| define  | [**SYN\_USE\_X509**](syn__config__template_8h.md#define-syn_use_x509)  `1`<br> |
| define  | [**SYN\_USE\_XCP**](syn__config__template_8h.md#define-syn_use_xcp)  `1`<br> |
| define  | [**SYN\_USE\_XRCE\_DDS**](syn__config__template_8h.md#define-syn_use_xrce_dds)  `1`<br> |
| define  | [**SYN\_USE\_YMODEM**](syn__config__template_8h.md#define-syn_use_ymodem)  `1`<br> |
| define  | [**SYN\_WASM\_USE\_FIXED**](syn__config__template_8h.md#define-syn_wasm_use_fixed)  `0`<br> |
| define  | [**SYN\_WG\_MTU**](syn__config__template_8h.md#define-syn_wg_mtu)  `1420`<br> |

## Detailed Description


HOW TO USE:
* Copy this file to your project's include path.
* Rename the copy to "syn\_config.h".
* Edit the copy to enable/disable modules and set buffer sizes.




SyntropicOS headers look for "syn\_config.h" on the include path. If the file is not found, sensible defaults are used. 


    
## Macro Definition Documentation





### define SYN\_AES\_GCM\_TABLE 

```C++
#define SYN_AES_GCM_TABLE `0`
```



GCM GHASH table: 0=none, 4=256B, 8=4KB 


        

<hr>



### define SYN\_AES\_MAX\_KEY\_BITS 

```C++
#define SYN_AES_MAX_KEY_BITS `256`
```



Max AES key bits: 128, 192, or 256 


        

<hr>



### define SYN\_AUDIO\_MIXER\_MAX\_CHANNELS 

```C++
#define SYN_AUDIO_MIXER_MAX_CHANNELS `4U`
```



Maximum concurrent mixer channels 


        

<hr>



### define SYN\_BLE\_HCI\_RX\_BUF\_SIZE 

```C++
#define SYN_BLE_HCI_RX_BUF_SIZE `260U`
```



HCI RX packet capacity 


        

<hr>



### define SYN\_BLE\_HCI\_TX\_BUF\_SIZE 

```C++
#define SYN_BLE_HCI_TX_BUF_SIZE `261U`
```



HCI TX packet capacity 


        

<hr>



### define SYN\_BLE\_MAX\_ATTRIBUTES 

```C++
#define SYN_BLE_MAX_ATTRIBUTES `32U`
```



Max GATT attribute table capacity 


        

<hr>



### define SYN\_BLE\_MAX\_CONNECTIONS 

```C++
#define SYN_BLE_MAX_CONNECTIONS `2U`
```



Max simultaneous BLE connections 


        

<hr>



### define SYN\_CLI\_HISTORY\_DEPTH 

```C++
#define SYN_CLI_HISTORY_DEPTH `0`
```



Command history depth (0=disabled) 


        

<hr>



### define SYN\_CLI\_LINE\_BUF\_SIZE 

```C++
#define SYN_CLI_LINE_BUF_SIZE `128`
```



Max command line length 


        

<hr>



### define SYN\_CLI\_MAX\_ARGS 

```C++
#define SYN_CLI_MAX_ARGS `16`
```



Max argc (including command name) 


        

<hr>



### define SYN\_COREDUMP\_STACK\_SIZE 

```C++
#define SYN_COREDUMP_STACK_SIZE `128`
```



&lt; Flash address for core dump sector Bytes of stack to capture 


        

<hr>



### define SYN\_CRC\_USE\_TABLE 

```C++
#define SYN_CRC_USE_TABLE `1`
```



1=lookup table (fast), 0=bitwise 


        

<hr>



### define SYN\_ETH\_ARP\_CACHE\_SIZE 

```C++
#define SYN_ETH_ARP_CACHE_SIZE `8`
```



ARP table cache size (entries) 


        

<hr>



### define SYN\_FILTER\_MAX\_WINDOW 

```C++
#define SYN_FILTER_MAX_WINDOW `32`
```



Max filter window size 


        

<hr>



### define SYN\_FW\_USE\_ED25519 

```C++
#define SYN_FW_USE_ED25519 `1`
```



Ed25519-signed firmware images (requires: ED25519, BOOT) 


        

<hr>



### define SYN\_FW\_USE\_HMAC 

```C++
#define SYN_FW_USE_HMAC `0`
```



HMAC-signed firmware images (requires: SHA256, BOOT) 


        

<hr>



### define SYN\_GFX\_BACKEND 

```C++
#define SYN_GFX_BACKEND `SYN_GFX_BACKEND_CANVAS`
```



Active renderer 


        

<hr>



### define SYN\_GFX\_BACKEND\_CANVAS 

```C++
#define SYN_GFX_BACKEND_CANVAS `0`
```



Framebuffer canvas (default) 


        

<hr>



### define SYN\_GFX\_BACKEND\_DIRECT 

```C++
#define SYN_GFX_BACKEND_DIRECT `1`
```



Direct-draw (no framebuffer) 


        

<hr>



### define SYN\_GNSS\_USE\_FIXED\_POINT 

```C++
#define SYN_GNSS_USE_FIXED_POINT `0`
```



0 = 64-bit double, 1 = 32-bit Micro-Degree (1e-7 deg) integer 


        

<hr>



### define SYN\_LOG\_BUF\_SIZE 

```C++
#define SYN_LOG_BUF_SIZE `192`
```



Log output buffer size (bytes) 


        

<hr>



### define SYN\_LOG\_COLOR 

```C++
#define SYN_LOG_COLOR `0`
```



Enable ANSI color codes 


        

<hr>



### define SYN\_LOG\_LEVEL 

```C++
#define SYN_LOG_LEVEL `1`
```



Compile-time min level (0=TRACE..5=FATAL, 6=NONE) 


        

<hr>



### define SYN\_LOG\_TIMESTAMP 

```C++
#define SYN_LOG_TIMESTAMP `1`
```



Include [tick] timestamp in output 


        

<hr>



### define SYN\_LZ4\_HASH\_SIZE 

```C++
#define SYN_LZ4_HASH_SIZE `4096U`
```



4096 entries (8 KB) 


        

<hr>



### define SYN\_MUTEX\_MAX\_WAITERS 

```C++
#define SYN_MUTEX_MAX_WAITERS `4U`
```



Tracked contended waiters per mutex 


        

<hr>



### define SYN\_P256\_USE\_COMB\_TABLE 

```C++
#define SYN_P256_USE_COMB_TABLE `0`
```



Precomputed G comb table (opt-in) 


        

<hr>



### define SYN\_SCHED\_MAX\_WAIT\_CRITICAL\_MS 

```C++
#define SYN_SCHED_MAX_WAIT_CRITICAL_MS `1U`
```



Starvation window for CRITICAL 


        

<hr>



### define SYN\_SCHED\_MAX\_WAIT\_HIGH\_MS 

```C++
#define SYN_SCHED_MAX_WAIT_HIGH_MS `5U`
```



Starvation window for HIGH 


        

<hr>



### define SYN\_SCHED\_MAX\_WAIT\_LOW\_MS 

```C++
#define SYN_SCHED_MAX_WAIT_LOW_MS `50U`
```



Starvation window for LOW 


        

<hr>



### define SYN\_SCHED\_MAX\_WAIT\_NORMAL\_MS 

```C++
#define SYN_SCHED_MAX_WAIT_NORMAL_MS `15U`
```



Starvation window for NORMAL 


        

<hr>



### define SYN\_SCHED\_PRIO\_LEVELS 

```C++
#define SYN_SCHED_PRIO_LEVELS `4`
```



Priority levels (0..N-1) 


        

<hr>



### define SYN\_SPINLOCK\_COUNT 

```C++
#define SYN_SPINLOCK_COUNT `4`
```



Number of spinlock IDs 


        

<hr>



### define SYN\_UART\_MAX\_INSTANCES 

```C++
#define SYN_UART_MAX_INSTANCES `2`
```



Max simultaneous UART handles 


        

<hr>



### define SYN\_UART\_RX\_BUF\_SIZE 

```C++
#define SYN_UART_RX_BUF_SIZE `128`
```



RX ring buffer size (bytes) 


        

<hr>



### define SYN\_UART\_TX\_BUF\_SIZE 

```C++
#define SYN_UART_TX_BUF_SIZE `128`
```



TX ring buffer size (bytes) 


        

<hr>



### define SYN\_USE\_ACTUATOR 

```C++
#define SYN_USE_ACTUATOR `1`
```



Linear actuator (requires: PID) 


        

<hr>



### define SYN\_USE\_ADC 

```C++
#define SYN_USE_ADC `1`
```



ADC with oversampling + calibration 


        

<hr>



### define SYN\_USE\_ADPCM 

```C++
#define SYN_USE_ADPCM `1`
```



IMA/DVI ADPCM speech & audio codec 


        

<hr>



### define SYN\_USE\_AES 

```C++
#define SYN_USE_AES `1`
```



Unified AES block cipher & AEAD 


        

<hr>



### define SYN\_USE\_AES\_CBC 

```C++
#define SYN_USE_AES_CBC `1`
```



AES-CBC mode with PKCS#7 


        

<hr>



### define SYN\_USE\_AES\_CCM 

```C++
#define SYN_USE_AES_CCM `1`
```



AES-CCM AEAD (NIST SP 800-38C / BLE) 


        

<hr>



### define SYN\_USE\_AES\_CMAC 

```C++
#define SYN_USE_AES_CMAC `1`
```



AES-CMAC message authentication 


        

<hr>



### define SYN\_USE\_AES\_CTR 

```C++
#define SYN_USE_AES_CTR `1`
```



AES-CTR stream cipher mode 


        

<hr>



### define SYN\_USE\_AES\_DECRYPT 

```C++
#define SYN_USE_AES_DECRYPT `1`
```



AES ECB/CBC decryption (+256B Flash) 


        

<hr>



### define SYN\_USE\_AES\_GCM 

```C++
#define SYN_USE_AES_GCM `1`
```



AES-GCM AEAD authenticated cipher 


        

<hr>



### define SYN\_USE\_AO 

```C++
#define SYN_USE_AO `1`
```



Active Object (requires: FSM, SCHED) 


        

<hr>



### define SYN\_USE\_ASN1 

```C++
#define SYN_USE_ASN1 `1`
```



ASN.1 DER TLV parser 


        

<hr>



### define SYN\_USE\_AT\_PARSER 

```C++
#define SYN_USE_AT_PARSER `1`
```



Stream-oriented AT command parser 


        

<hr>



### define SYN\_USE\_AUDIO 

```C++
#define SYN_USE_AUDIO `1`
```



Double-buffered PCM audio engine 


        

<hr>



### define SYN\_USE\_AUTOIP 

```C++
#define SYN_USE_AUTOIP `1`
```



RFC 3927 AutoIP Link-Local engine 


        

<hr>



### define SYN\_USE\_AUTOTUNE 

```C++
#define SYN_USE_AUTOTUNE `1`
```



Motor auto-tuner (requires: MOTOR\_CTRL) 


        

<hr>



### define SYN\_USE\_BACNET 

```C++
#define SYN_USE_BACNET `1`
```



BACnet MS/TP & APDU (ISO 16484-5) 


        

<hr>



### define SYN\_USE\_BASE64 

```C++
#define SYN_USE_BASE64 `1`
```



RFC 4648 Base64 & Base64URL codec 


        

<hr>



### define SYN\_USE\_BIOMETRIC 

```C++
#define SYN_USE_BIOMETRIC `1`
```



Pulse oximeter & heart rate sensor 


        

<hr>



### define SYN\_USE\_BIQUAD 

```C++
#define SYN_USE_BIQUAD `1`
```



Biquad digital filter 


        

<hr>



### define SYN\_USE\_BLACKBOX 

```C++
#define SYN_USE_BLACKBOX `1`
```



Flight telemetry blackbox recorder 


        

<hr>



### define SYN\_USE\_BLAKE2S 

```C++
#define SYN_USE_BLAKE2S `1`
```



BLAKE2s hash + keyed MAC (RFC 7693) 


        

<hr>



### define SYN\_USE\_BLDC\_6STEP 

```C++
#define SYN_USE_BLDC_6STEP `1`
```



6-step trapezoidal BLDC commutation 


        

<hr>



### define SYN\_USE\_BLE 

```C++
#define SYN_USE_BLE `1`
```



BLE Host Stack (HCI/GAP/GATT) 


        

<hr>



### define SYN\_USE\_BOOT 

```C++
#define SYN_USE_BOOT `1`
```



Boot manager (crash recovery) 


        

<hr>



### define SYN\_USE\_BUTTON 

```C++
#define SYN_USE_BUTTON `1`
```



Button debouncer (requires: FSM) 


        

<hr>



### define SYN\_USE\_BUZZER 

```C++
#define SYN_USE_BUZZER `1`
```



Piezo buzzer & tone generator 


        

<hr>



### define SYN\_USE\_CAN 

```C++
#define SYN_USE_CAN `1`
```



CAN bus driver abstraction 


        

<hr>



### define SYN\_USE\_CANNM 

```C++
#define SYN_USE_CANNM `1`
```



AUTOSAR CAN Network Management 


        

<hr>



### define SYN\_USE\_CANOPEN 

```C++
#define SYN_USE_CANOPEN `1`
```



CANopen DS301 slave protocol engine 


        

<hr>



### define SYN\_USE\_CANVAS 

```C++
#define SYN_USE_CANVAS `1`
```



Display canvas / framebuffer 


        

<hr>



### define SYN\_USE\_CAN\_FD 

```C++
#define SYN_USE_CAN_FD `0`
```



CAN FD 64-byte frame support 


        

<hr>



### define SYN\_USE\_CBOR 

```C++
#define SYN_USE_CBOR `1`
```



CBOR binary serializer 


        

<hr>



### define SYN\_USE\_CCP 

```C++
#define SYN_USE_CCP `1`
```



ASAM CCP v2.1 calibration slave 


        

<hr>



### define SYN\_USE\_CHACHA20POLY1305 

```C++
#define SYN_USE_CHACHA20POLY1305 `1`
```



ChaCha20-Poly1305 AEAD (RFC 8439) 


        

<hr>



### define SYN\_USE\_CHARLCD 

```C++
#define SYN_USE_CHARLCD `1`
```



Character LCD (HD44780) driver 


        

<hr>



### define SYN\_USE\_CIA402 

```C++
#define SYN_USE_CIA402 `1`
```



CiA 402 Drive & Motion profile engine 


        

<hr>



### define SYN\_USE\_CJT188 

```C++
#define SYN_USE_CJT188 `1`
```



CJ/T 188 smart metering protocol 


        

<hr>



### define SYN\_USE\_CLI 

```C++
#define SYN_USE_CLI `1`
```



Command-line interface (requires: FMT) 


        

<hr>



### define SYN\_USE\_CLIMATE 

```C++
#define SYN_USE_CLIMATE `1`
```



Temperature, humidity & pressure 


        

<hr>



### define SYN\_USE\_COAP 

```C++
#define SYN_USE_COAP `1`
```



CoAP protocol client 


        

<hr>



### define SYN\_USE\_COBS 

```C++
#define SYN_USE_COBS `1`
```



COBS packet framing 


        

<hr>



### define SYN\_USE\_COMP 

```C++
#define SYN_USE_COMP `1`
```



Analog comparator driver 


        

<hr>



### define SYN\_USE\_COREDUMP 

```C++
#define SYN_USE_COREDUMP `0`
```



Persistent core dump to flash (requires: FAULT, CRC) 


        

<hr>



### define SYN\_USE\_COSE 

```C++
#define SYN_USE_COSE `1`
```



COSE (RFC 9052) signing & encryption 


        

<hr>



### define SYN\_USE\_CRSF 

```C++
#define SYN_USE_CRSF `1`
```



CRSF RC & telemetry parser 


        

<hr>



### define SYN\_USE\_DAC 

```C++
#define SYN_USE_DAC `1`
```



Digital-to-Analog Converter driver 


        

<hr>



### define SYN\_USE\_DALI 

```C++
#define SYN_USE_DALI `1`
```



DALI / IEC 62386 lighting stack 


        

<hr>



### define SYN\_USE\_DATALOG 

```C++
#define SYN_USE_DATALOG `1`
```



Data logger (ring buffer to flash) 


        

<hr>



### define SYN\_USE\_DC\_MOTOR 

```C++
#define SYN_USE_DC_MOTOR `1`
```



DC motor (H-bridge + ramp) 


        

<hr>



### define SYN\_USE\_DDS 

```C++
#define SYN_USE_DDS `1`
```



Direct Digital Synthesis waveform 


        

<hr>



### define SYN\_USE\_DEVICENET 

```C++
#define SYN_USE_DEVICENET `1`
```



ODVA DeviceNet (CIP over CAN) stack 


        

<hr>



### define SYN\_USE\_DHCP 

```C++
#define SYN_USE_DHCP `1`
```



DHCP client protocol engine 


        

<hr>



### define SYN\_USE\_DIPSWITCH 

```C++
#define SYN_USE_DIPSWITCH `1`
```



DIP switch & rotary selector 


        

<hr>



### define SYN\_USE\_DISTANCE 

```C++
#define SYN_USE_DISTANCE `1`
```



Ultrasonic, ToF & IR distance 


        

<hr>



### define SYN\_USE\_DMA 

```C++
#define SYN_USE_DMA `0`
```



DMA port abstraction 


        

<hr>



### define SYN\_USE\_DMX512 

```C++
#define SYN_USE_DMX512 `1`
```



USITT DMX512-A lighting stack 


        

<hr>



### define SYN\_USE\_DNS 

```C++
#define SYN_USE_DNS `1`
```



DNS resolver 


        

<hr>



### define SYN\_USE\_DOIP 

```C++
#define SYN_USE_DOIP `1`
```



ISO 13400-2 Diagnostics over IP 


        

<hr>



### define SYN\_USE\_DSHOT 

```C++
#define SYN_USE_DSHOT `1`
```



DShot150/300/600 digital ESC output 


        

<hr>



### define SYN\_USE\_DSHOT\_TELEMETRY 

```C++
#define SYN_USE_DSHOT_TELEMETRY `1`
```



Bidirectional DShot telemetry 


        

<hr>



### define SYN\_USE\_DTLS 

```C++
#define SYN_USE_DTLS `1`
```



DTLS 1.3 datagram security (RFC 9147) 


        

<hr>



### define SYN\_USE\_ED25519 

```C++
#define SYN_USE_ED25519 `1`
```



Ed25519 signatures (RFC 8032) 


        

<hr>



### define SYN\_USE\_ENCODER 

```C++
#define SYN_USE_ENCODER `1`
```



Rotary encoder (quadrature) 


        

<hr>



### define SYN\_USE\_ERRLOG 

```C++
#define SYN_USE_ERRLOG `1`
```



Persistent error registry 


        

<hr>



### define SYN\_USE\_ETH 

```C++
#define SYN_USE_ETH `1`
```



Zero-heap Ethernet II & ARP engine 


        

<hr>



### define SYN\_USE\_ETHERCAT 

```C++
#define SYN_USE_ETHERCAT `1`
```



EtherCAT bare-metal protocol stack 


        

<hr>



### define SYN\_USE\_EVENT 

```C++
#define SYN_USE_EVENT `1`
```



Event flag groups (32-bit bitmask) 


        

<hr>



### define SYN\_USE\_EXTI 

```C++
#define SYN_USE_EXTI `1`
```



GPIO interrupt dispatcher 


        

<hr>



### define SYN\_USE\_FAT 

```C++
#define SYN_USE_FAT `1`
```



FAT filesystem adapter (requires: VFS) 


        

<hr>



### define SYN\_USE\_FAULT 

```C++
#define SYN_USE_FAULT `1`
```



Hard Fault diagnostics 


        

<hr>



### define SYN\_USE\_FFT 

```C++
#define SYN_USE_FFT `1`
```



Fast Fourier Transform (FFT) 


        

<hr>



### define SYN\_USE\_FILTER 

```C++
#define SYN_USE_FILTER `1`
```



Digital filters (MA, EMA, Median) 


        

<hr>



### define SYN\_USE\_FLIGHT\_PID 

```C++
#define SYN_USE_FLIGHT_PID `1`
```



3-axis flight PID & motor mixer 


        

<hr>



### define SYN\_USE\_FMT 

```C++
#define SYN_USE_FMT `1`
```



Lightweight printf alternative 


        

<hr>



### define SYN\_USE\_FOC 

```C++
#define SYN_USE_FOC `1`
```



Field-Oriented Control (Clarke/Park) 


        

<hr>



### define SYN\_USE\_FSM 

```C++
#define SYN_USE_FSM `1`
```



Table-driven finite state machine 


        

<hr>



### define SYN\_USE\_GBT27930 

```C++
#define SYN_USE_GBT27930 `1`
```



GB/T 27930 EV DC fast charging stack 


        

<hr>



### define SYN\_USE\_GCODE 

```C++
#define SYN_USE_GCODE `1`
```



G-Code RS-274 motion interpreter (requires: INTERPOLATOR) 


        

<hr>



### define SYN\_USE\_GEO 

```C++
#define SYN_USE_GEO `1`
```



WGS84 & 3D local ENU geodetic math 


        

<hr>



### define SYN\_USE\_GOERTZEL 

```C++
#define SYN_USE_GOERTZEL `1`
```



Goertzel tone detector 


        

<hr>



### define SYN\_USE\_GPIO 

```C++
#define SYN_USE_GPIO `1`
```



GPIO pin control 


        

<hr>



### define SYN\_USE\_HEARTBEAT 

```C++
#define SYN_USE_HEARTBEAT `1`
```



Heartbeat / keep-alive service 


        

<hr>



### define SYN\_USE\_HKDF 

```C++
#define SYN_USE_HKDF `1`
```



HKDF key derivation (RFC 5869) 


        

<hr>



### define SYN\_USE\_HMAC\_DRBG 

```C++
#define SYN_USE_HMAC_DRBG `1`
```



NIST SP 800-90A HMAC-DRBG (SHA-256) 


        

<hr>



### define SYN\_USE\_HPCLOCK 

```C++
#define SYN_USE_HPCLOCK `1`
```



High-precision 64-bit system clock 


        

<hr>



### define SYN\_USE\_HRPWM 

```C++
#define SYN_USE_HRPWM `1`
```



High-resolution PWM driver 


        

<hr>



### define SYN\_USE\_HTTP 

```C++
#define SYN_USE_HTTP `1`
```



HTTP client 


        

<hr>



### define SYN\_USE\_HTTPD 

```C++
#define SYN_USE_HTTPD `1`
```



HTTP server (embedded web server) 


        

<hr>



### define SYN\_USE\_HWWDT 

```C++
#define SYN_USE_HWWDT `1`
```



Hardware watchdog timer 


        

<hr>



### define SYN\_USE\_I2C 

```C++
#define SYN_USE_I2C `1`
```



I2C master/slave driver 


        

<hr>



### define SYN\_USE\_I2C\_ASYNC 

```C++
#define SYN_USE_I2C_ASYNC `0`
```



Async I2C transactions 


        

<hr>



### define SYN\_USE\_IBUS 

```C++
#define SYN_USE_IBUS `1`
```



IBUS 14-channel RC receiver decoder 


        

<hr>



### define SYN\_USE\_ICMP 

```C++
#define SYN_USE_ICMP `1`
```



ICMP Echo Ping protocol engine 


        

<hr>



### define SYN\_USE\_IGMP 

```C++
#define SYN_USE_IGMP `1`
```



IGMPv2 Multicast protocol engine 


        

<hr>



### define SYN\_USE\_IMGUI 

```C++
#define SYN_USE_IMGUI `1`
```



Immediate-mode GUI 


        

<hr>



### define SYN\_USE\_INTERPOLATOR 

```C++
#define SYN_USE_INTERPOLATOR `1`
```



Multi-axis linear/arc interpolator 


        

<hr>



### define SYN\_USE\_IOEXP 

```C++
#define SYN_USE_IOEXP `1`
```



I2C GPIO expander driver 


        

<hr>



### define SYN\_USE\_IOLINK 

```C++
#define SYN_USE_IOLINK `1`
```



IO-Link (IEC 61131-9) Master & Device protocol 


        

<hr>



### define SYN\_USE\_IR 

```C++
#define SYN_USE_IR `1`
```



Infrared remote protocol codec 


        

<hr>



### define SYN\_USE\_ISOTP 

```C++
#define SYN_USE_ISOTP `1`
```



ISO 15765-2 (ISO-TP) CAN transport 


        

<hr>



### define SYN\_USE\_J1939 

```C++
#define SYN_USE_J1939 `1`
```



SAE J1939 heavy-duty CAN stack 


        

<hr>



### define SYN\_USE\_JOYSTICK 

```C++
#define SYN_USE_JOYSTICK `1`
```



Analog joystick & potentiometer 


        

<hr>



### define SYN\_USE\_JSON 

```C++
#define SYN_USE_JSON `1`
```



JSON reader/writer 


        

<hr>



### define SYN\_USE\_KALMAN 

```C++
#define SYN_USE_KALMAN `1`
```



Fixed-point Kalman filter 


        

<hr>



### define SYN\_USE\_KEYPAD 

```C++
#define SYN_USE_KEYPAD `1`
```



Matrix keypad scanner 


        

<hr>



### define SYN\_USE\_KINEMATICS 

```C++
#define SYN_USE_KINEMATICS `1`
```



Multi-axis robot forward & inverse kinematics 


        

<hr>



### define SYN\_USE\_LED 

```C++
#define SYN_USE_LED `1`
```



LED controller (blink/flash/pattern) 


        

<hr>



### define SYN\_USE\_LFS 

```C++
#define SYN_USE_LFS `1`
```



LittleFS adapter (requires: VFS) 


        

<hr>



### define SYN\_USE\_LIN 

```C++
#define SYN_USE_LIN `1`
```



LIN bus 2.1/2.2a & ISO 17987 stack 


        

<hr>



### define SYN\_USE\_LINTP 

```C++
#define SYN_USE_LINTP `1`
```



ISO 17987-2 LIN transport layer 


        

<hr>



### define SYN\_USE\_LOG 

```C++
#define SYN_USE_LOG `1`
```



Logging system (requires: FMT) 


        

<hr>



### define SYN\_USE\_LUX 

```C++
#define SYN_USE_LUX `1`
```



Ambient light & RGB color sensor 


        

<hr>



### define SYN\_USE\_LWM2M 

```C++
#define SYN_USE_LWM2M `1`
```



OMA Lightweight M2M client engine 


        

<hr>



### define SYN\_USE\_LWM2M\_TASK 

```C++
#define SYN_USE_LWM2M_TASK `1`
```



LwM2M Autonomous Client Task engine 


        

<hr>



### define SYN\_USE\_LZ4 

```C++
#define SYN_USE_LZ4 `1`
```



Zero-heap LZ4 compression 


        

<hr>



### define SYN\_USE\_MATRIX 

```C++
#define SYN_USE_MATRIX `1`
```



Q16.16 fixed-point matrix math 


        

<hr>



### define SYN\_USE\_MAVLINK 

```C++
#define SYN_USE_MAVLINK `1`
```



MAVLink v2 encoder & decoder 


        

<hr>



### define SYN\_USE\_MBUS 

```C++
#define SYN_USE_MBUS `1`
```



M-Bus (EN 13757-2/3) protocol engine 


        

<hr>



### define SYN\_USE\_MENU 

```C++
#define SYN_USE_MENU `1`
```



UI menu system 


        

<hr>



### define SYN\_USE\_METRICS 

```C++
#define SYN_USE_METRICS `0`
```



Telemetry counters/gauges (opt-in) 


        

<hr>



### define SYN\_USE\_MFCC 

```C++
#define SYN_USE_MFCC `1`
```



Fixed-point MFCC feature extractor 


        

<hr>



### define SYN\_USE\_MODBUS 

```C++
#define SYN_USE_MODBUS `1`
```



Modbus RTU slave (requires: CRC) 


        

<hr>



### define SYN\_USE\_MODBUS\_MASTER 

```C++
#define SYN_USE_MODBUS_MASTER `1`
```



Modbus RTU master / client 


        

<hr>



### define SYN\_USE\_MOTOR\_CTRL 

```C++
#define SYN_USE_MOTOR_CTRL `1`
```



Closed-loop motor controller (requires: PID) 


        

<hr>



### define SYN\_USE\_MQTT 

```C++
#define SYN_USE_MQTT `1`
```



MQTT 3.1.1 client 


        

<hr>



### define SYN\_USE\_MQTTSN 

```C++
#define SYN_USE_MQTTSN `1`
```



MQTT-SN v1.2 datagram client engine 


        

<hr>



### define SYN\_USE\_MSP 

```C++
#define SYN_USE_MSP `1`
```



MultiWii Serial Protocol v1 codec 


        

<hr>



### define SYN\_USE\_MULTICORE 

```C++
#define SYN_USE_MULTICORE `0`
```



AMP multicore support 


        

<hr>



### define SYN\_USE\_MUTEX 

```C++
#define SYN_USE_MUTEX `1`
```



Priority-inheriting mutex 


        

<hr>



### define SYN\_USE\_N2K 

```C++
#define SYN_USE_N2K `1`
```



NMEA 2000 marine CAN protocol stack 


        

<hr>



### define SYN\_USE\_NETCFG 

```C++
#define SYN_USE_NETCFG `1`
```



Network IP Manager & Link Events 


        

<hr>



### define SYN\_USE\_NMEA 

```C++
#define SYN_USE_NMEA `1`
```



NMEA 0183 GNSS parser & encoder 


        

<hr>



### define SYN\_USE\_NTP\_SERVER 

```C++
#define SYN_USE_NTP_SERVER `1`
```



NTP v4 time server 


        

<hr>



### define SYN\_USE\_OLED 

```C++
#define SYN_USE_OLED `1`
```



Monochrome I2C OLED display driver 


        

<hr>



### define SYN\_USE\_ONEWIRE 

```C++
#define SYN_USE_ONEWIRE `1`
```



Software 1-Wire master driver 


        

<hr>



### define SYN\_USE\_OPCUA 

```C++
#define SYN_USE_OPCUA `1`
```



Micro OPC UA Binary server engine (IEC 62541) 


        

<hr>



### define SYN\_USE\_OTA 

```C++
#define SYN_USE_OTA `1`
```



Secure Streaming OTA Orchestrator 


        

<hr>



### define SYN\_USE\_P256 

```C++
#define SYN_USE_P256 `1`
```



NIST P-256 / secp256r1 ECC 


        

<hr>



### define SYN\_USE\_PARAM 

```C++
#define SYN_USE_PARAM `1`
```



Wear-leveled parameter store (requires: CRC) 


        

<hr>



### define SYN\_USE\_PID 

```C++
#define SYN_USE_PID `1`
```



PID controller 


        

<hr>



### define SYN\_USE\_PMBUS 

```C++
#define SYN_USE_PMBUS `1`
```



PMBus 1.2/1.3 protocol & linear fmt 


        

<hr>



### define SYN\_USE\_POOL 

```C++
#define SYN_USE_POOL `1`
```



Fixed-size block memory pool 


        

<hr>



### define SYN\_USE\_PORT\_AES 

```C++
#define SYN_USE_PORT_AES `0`
```



Hardware AES/crypto port offload 


        

<hr>



### define SYN\_USE\_POWER 

```C++
#define SYN_USE_POWER `1`
```



Power management (sleep/wake) 


        

<hr>



### define SYN\_USE\_POWERMON 

```C++
#define SYN_USE_POWERMON `1`
```



Power, voltage & current monitor 


        

<hr>



### define SYN\_USE\_PPM 

```C++
#define SYN_USE_PPM `1`
```



PPM multi-channel RC decoder 


        

<hr>



### define SYN\_USE\_PROFILER 

```C++
#define SYN_USE_PROFILER `1`
```



Task CPU profiler 


        

<hr>



### define SYN\_USE\_PROTOBUF 

```C++
#define SYN_USE_PROTOBUF `1`
```



Protocol Buffers wire format 


        

<hr>



### define SYN\_USE\_PT 

```C++
#define SYN_USE_PT `1`
```



Protothreads (cooperative coroutines) 


        

<hr>



### define SYN\_USE\_PUBSUB 

```C++
#define SYN_USE_PUBSUB `1`
```



Publish-subscribe message bus 


        

<hr>



### define SYN\_USE\_PWM 

```C++
#define SYN_USE_PWM `1`
```



Hardware PWM driver 


        

<hr>



### define SYN\_USE\_QMATH 

```C++
#define SYN_USE_QMATH `1`
```



Q16.16 transcendental math & I/O 


        

<hr>



### define SYN\_USE\_RAMP 

```C++
#define SYN_USE_RAMP `1`
```



Ramp / slew rate generator 


        

<hr>



### define SYN\_USE\_RC\_CURVE 

```C++
#define SYN_USE_RC_CURVE `1`
```



RC expo & deadband curve mapper 


        

<hr>



### define SYN\_USE\_RC\_FAILSAFE 

```C++
#define SYN_USE_RC_FAILSAFE `1`
```



RC link failsafe monitor 


        

<hr>



### define SYN\_USE\_RFID 

```C++
#define SYN_USE_RFID `1`
```



RFID / NFC card reader driver 


        

<hr>



### define SYN\_USE\_ROUTER 

```C++
#define SYN_USE_ROUTER `1`
```



Network router / dispatcher 


        

<hr>



### define SYN\_USE\_RTC 

```C++
#define SYN_USE_RTC `1`
```



Real-Time Clock (RTC) driver 


        

<hr>



### define SYN\_USE\_SBC 

```C++
#define SYN_USE_SBC `1`
```



Bluetooth A2DP SBC audio decoder 


        

<hr>



### define SYN\_USE\_SBUS 

```C++
#define SYN_USE_SBUS `1`
```



SBUS 16-channel RC receiver decoder 


        

<hr>



### define SYN\_USE\_SCALE 

```C++
#define SYN_USE_SCALE `1`
```



Load cell & weight scale ADC 


        

<hr>



### define SYN\_USE\_SCHED 

```C++
#define SYN_USE_SCHED `1`
```



Cooperative task scheduler 


        

<hr>



### define SYN\_USE\_SCURVE 

```C++
#define SYN_USE_SCURVE `1`
```



S-curve motion profile 


        

<hr>



### define SYN\_USE\_SD 

```C++
#define SYN_USE_SD `1`
```



SD card SPI block driver 


        

<hr>



### define SYN\_USE\_SEG7 

```C++
#define SYN_USE_SEG7 `1`
```



7-segment LED display driver 


        

<hr>



### define SYN\_USE\_SENSOR 

```C++
#define SYN_USE_SENSOR `1`
```



Sensor polling framework 


        

<hr>



### define SYN\_USE\_SEQUENCER 

```C++
#define SYN_USE_SEQUENCER `1`
```



Timed action sequencer 


        

<hr>



### define SYN\_USE\_SERVO 

```C++
#define SYN_USE_SERVO `1`
```



Hobby servo (pulse-width control) 


        

<hr>



### define SYN\_USE\_SETTINGS 

```C++
#define SYN_USE_SETTINGS `1`
```



Settings manager (requires: PARAM, CRC) 


        

<hr>



### define SYN\_USE\_SHA256 

```C++
#define SYN_USE_SHA256 `1`
```



SHA-256 hash + HMAC-SHA256 


        

<hr>



### define SYN\_USE\_SHA512 

```C++
#define SYN_USE_SHA512 `1`
```



SHA-512 / SHA-384 / HMAC-SHA512/384 


        

<hr>



### define SYN\_USE\_SHIFTREG 

```C++
#define SYN_USE_SHIFTREG `1`
```



Shift register I/O expander driver 


        

<hr>



### define SYN\_USE\_SIGNAL 

```C++
#define SYN_USE_SIGNAL `1`
```



Signal statistics (sliding window) 


        

<hr>



### define SYN\_USE\_SMARTLED 

```C++
#define SYN_USE_SMARTLED `1`
```



Addressable RGB/RGBW LED strip 


        

<hr>



### define SYN\_USE\_SMBUS 

```C++
#define SYN_USE_SMBUS `1`
```



SMBus 1.1/2.0/3.0 protocol engine 


        

<hr>



### define SYN\_USE\_SNTP 

```C++
#define SYN_USE_SNTP `1`
```



SNTP time synchronization client 


        

<hr>



### define SYN\_USE\_SOFT\_I2C 

```C++
#define SYN_USE_SOFT_I2C `1`
```



Software bit-bang I2C driver 


        

<hr>



### define SYN\_USE\_SOFT\_PWM 

```C++
#define SYN_USE_SOFT_PWM `1`
```



Software PWM on any GPIO 


        

<hr>



### define SYN\_USE\_SOFT\_SPI 

```C++
#define SYN_USE_SOFT_SPI `1`
```



Software bit-bang SPI driver 


        

<hr>



### define SYN\_USE\_SPI 

```C++
#define SYN_USE_SPI `1`
```



SPI master/slave driver 


        

<hr>



### define SYN\_USE\_SPI\_ASYNC 

```C++
#define SYN_USE_SPI_ASYNC `0`
```



Async SPI transactions 


        

<hr>



### define SYN\_USE\_STEPPER 

```C++
#define SYN_USE_STEPPER `1`
```



Stepper motor (step/dir + accel) 


        

<hr>



### define SYN\_USE\_TICKLESS 

```C++
#define SYN_USE_TICKLESS `0`
```



Tickless idle scheduler (requires: SCHED, port sleep\_until) 


        

<hr>



### define SYN\_USE\_TIMER 

```C++
#define SYN_USE_TIMER `1`
```



Software timers (one-shot + periodic) 


        

<hr>



### define SYN\_USE\_TIMESYNC 

```C++
#define SYN_USE_TIMESYNC `1`
```



GPS/RTC time discipline service 


        

<hr>



### define SYN\_USE\_TLS 

```C++
#define SYN_USE_TLS `1`
```



TLS 1.3 engine & transport (RFC 8446) 


        

<hr>



### define SYN\_USE\_TOUCH 

```C++
#define SYN_USE_TOUCH `1`
```



Capacitive touch sensor 


        

<hr>



### define SYN\_USE\_TPM2 

```C++
#define SYN_USE_TPM2 `1`
```



TCG TPM 2.0 command & measured boot engine 


        

<hr>



### define SYN\_USE\_TRACE 

```C++
#define SYN_USE_TRACE `1`
```



Event trace buffer 


        

<hr>



### define SYN\_USE\_TRANSPORT\_TCP 

```C++
#define SYN_USE_TRANSPORT_TCP `1`
```



TCP transport layer 


        

<hr>



### define SYN\_USE\_TRANSPORT\_UDP 

```C++
#define SYN_USE_TRANSPORT_UDP `1`
```



UDP port-to-native stack bridge 


        

<hr>



### define SYN\_USE\_UART 

```C++
#define SYN_USE_UART `1`
```



Peripheral UARTs (Modbus, GPS, etc) 


        

<hr>



### define SYN\_USE\_UDS 

```C++
#define SYN_USE_UDS `1`
```



ISO 14229 Unified Diagnostic Server 


        

<hr>



### define SYN\_USE\_USB\_CDC 

```C++
#define SYN_USE_USB_CDC `1`
```



USB 2.0 CDC ACM virtual COM port 


        

<hr>



### define SYN\_USE\_USB\_MIDI 

```C++
#define SYN_USE_USB_MIDI `1`
```



USB MIDI Class 1.0 device 


        

<hr>



### define SYN\_USE\_USB\_MSC 

```C++
#define SYN_USE_USB_MSC `1`
```



USB Mass Storage Class BOT device 


        

<hr>



### define SYN\_USE\_VAD 

```C++
#define SYN_USE_VAD `1`
```



Zero-heap Voice Activity Detector 


        

<hr>



### define SYN\_USE\_VFS 

```C++
#define SYN_USE_VFS `1`
```



Virtual File System (VFS) 


        

<hr>



### define SYN\_USE\_WASM 

```C++
#define SYN_USE_WASM `1`
```



[EXPERIMENTAL] WebAssembly MVP interpreter 


        

<hr>



### define SYN\_USE\_WATCHDOG 

```C++
#define SYN_USE_WATCHDOG `1`
```



Task-level watchdog monitor 


        

<hr>



### define SYN\_USE\_WEBSOCKET 

```C++
#define SYN_USE_WEBSOCKET `1`
```



WebSocket client 


        

<hr>



### define SYN\_USE\_WG 

```C++
#define SYN_USE_WG `1`
```



WireGuard VPN client (requires: BLAKE2S, CHACHA20POLY1305, X25519, SNTP) 


        

<hr>



### define SYN\_USE\_WORKQUEUE 

```C++
#define SYN_USE_WORKQUEUE `1`
```



Deferred work queue (ISR→main) 


        

<hr>



### define SYN\_USE\_X25519 

```C++
#define SYN_USE_X25519 `1`
```



X25519 Diffie-Hellman (RFC 7748) 


        

<hr>



### define SYN\_USE\_X509 

```C++
#define SYN_USE_X509 `1`
```



X.509 v3 certificate parser 


        

<hr>



### define SYN\_USE\_XCP 

```C++
#define SYN_USE_XCP `1`
```



ASAM XCP v1.x calibration slave 


        

<hr>



### define SYN\_USE\_XRCE\_DDS 

```C++
#define SYN_USE_XRCE_DDS `1`
```



Micro-ROS / XRCE-DDS v1.2 client engine 


        

<hr>



### define SYN\_USE\_YMODEM 

```C++
#define SYN_USE_YMODEM `1`
```



YMODEM / XMODEM-1K file receiver 


        

<hr>



### define SYN\_WASM\_USE\_FIXED 

```C++
#define SYN_WASM_USE_FIXED `0`
```



f32 opcodes use Q16.16 fixed-point 


        

<hr>



### define SYN\_WG\_MTU 

```C++
#define SYN_WG_MTU `1420`
```



WireGuard tunnel MTU (inner payload) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/syn_config_template.h`

