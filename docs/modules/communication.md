# Communication & Networking Modules

SyntropicOS provides a comprehensive suite of communication protocols ranging from zero-overhead byte stuffing and point-to-point packet routing up to industrial fieldbuses and automotive networks.

---

## Protocol Overview

| Layer | Module | Header | Description |
|---|---|---|---|
| **Ethernet** | Ethernet II & ARP | `net/syn_eth.h` | Zero-heap Ethernet II framing, MAC filtering, & configurable ARP table cache (`SYN_ETH_ARP_CACHE_SIZE`, default 8) |
| **Transport** | Native TCP Engine | `net/syn_tcp.h` | Zero-alloc IPv4 TCP state machine, 3-way handshake (`SYN`, `SYN-ACK`, `ACK`), & `PT_TCP_BLOCK_READ` macro |
| **Transport** | Native UDP Engine | `net/syn_udp.h` | Zero-alloc IPv4 UDP demuxing engine, targeted port waking, & `PT_UDP_BLOCK_READ` macro |
| **Transport** | UDP Transport Bridge | `net/syn_transport_udp.h` | Dual-stack socket bridge connecting `syn_port_udp_*` platform abstraction to native `syn_udp` |
| **Ethernet** | HAL Contract | `port/syn_port_eth.h` | Hardware HAL contract driving STM32 RMII, W5500 SPI, or ESP32 ETH |
| **IP Address** | DHCP Client | `net/syn_dhcp.h` | RFC 2131 BOOTP/DHCP state machine (`DISCOVER` → `ACK`) & option parser |
| **IP Address** | AutoIP (RFC 3927) | `net/syn_autoip.h` | Link-Local (`169.254.x.x`) IP selection, ARP probing, & collision recovery |
| **IP Manager** | Netcfg Manager | `net/syn_netcfg.h` | Unified Static / DHCP / AutoIP fallback & Link Up/Down state machine |
| **ICMP** | ICMP Engine | `net/syn_icmp.h` | RFC 792 ICMP Echo Request/Reply (Ping) & Ones-Complement Checksum |
| **Framing** | COBS | `proto/syn_cobs.h` | Consistent Overhead Byte Stuffing (`0x00` packet delimiter) |
| **Routing** | Router | `net/syn_router.h` | Addressed packet dispatch (Node ID), type routing, & ACKs |
| **Industrial** | Modbus | `proto/syn_modbus.h` | Modbus RTU & Modbus TCP Master/Slave support |
| **Building** | BACnet MS/TP | `proto/syn_bacnet.h` | ANSI/ASHRAE 135 BACnet MS/TP framing, APDU codec, & Object DB |
| **Building** | DALI | `proto/syn_dali.h` | IEC 62386-101/102 DALI Master/Slave Manchester encoding |
| **Metering** | M-Bus | `proto/syn_mbus.h` | EN 13757-2 / EN 13757-3 European Meter Bus protocol |
| **Automotive** | ISO-TP | `proto/syn_isotp.h` | ISO 15765-2 multi-frame CAN transport layer |
| **Automotive** | J1939 | `proto/syn_j1939.h` | SAE J1939 heavy vehicle network protocol (PGN / SPN) |
| **Marine** | NMEA 2000 | `proto/syn_n2k.h` | NMEA 2000 marine CAN bus protocol decoder |
| **IoT / Edge** | OMA LwM2M Supervisor | `proto/syn_lwm2m_task.h` | Zero-heap autonomous LwM2M v1.1/v1.2 client task, DTLS 1.3, & Observe scheduler |
| **Robotics** | Micro XRCE-DDS | `proto/syn_xrce_dds.h` | Micro-ROS & XRCE-DDS v1.2 client with CDR serialization & entity tree |
| **Industrial** | Micro OPC UA Server | `proto/syn_opcua.h` | IEC 62541 binary server (UACP/UASC, Read, Write, Browse, Session management) |
| **Fieldbus** | IO-Link Master & Device | `proto/syn_iolink.h` | IEC 61131-9 single-drop digital point-to-point engine with CRC-6 & ISDU |
| **Wireless IoT** | MQTT-SN Client | `proto/syn_mqttsn.h` | MQTT for Sensor Networks v1.2 client engine over UDP/Zigbee/BLE/Sub-GHz |

---

## 1. COBS & Packet Router Pipeline (`syn_cobs` + `syn_router`)

For MCU-to-MCU serial communication over UART TTL or RS232/RS485, SyntropicOS pairs **COBS Framing** with the **Addressed Router**.

### Pipeline Data Flow

```mermaid
flowchart LR
    UART["Single-Byte UART RX Interrupt"] --> Decoder["syn_cobs_decoder_feed"]
    Decoder -->|0x00 Delimiter Found| Assembly["Decoded Frame Buffer"]
    Assembly --> Router["syn_router_poll"]
    Router -->|Match Node ID & Msg Type| Callback["Handler Callback (e.g. on_led_command)"]
```

### Complete STM32 HAL UART Single-Byte Interrupt Example

```c
#include <string.h>
#include <syntropic/proto/syn_cobs.h>
#include <syntropic/net/syn_router.h>

#define MASTER_NODE_ID 0x01
#define SLAVE_NODE_ID  0x02
#define MSG_TYPE_LED   0x10

static uint8_t rx_byte;
static SYN_COBS_Decoder cobs_dec;
static uint8_t cobs_buf[128];

static uint8_t rx_pkt_buf[128];
static volatile size_t rx_pkt_len;
static volatile bool rx_pkt_ready;

static SYN_Router router;
static SYN_RouterHandler handlers[4];

// Custom Transport: Send framed COBS packet over UART
static bool uart_send(const uint8_t *data, size_t len, void *ctx) {
    (void)ctx;
    uint8_t enc[140];
    size_t enc_len = syn_cobs_encode(data, len, enc);
    enc[enc_len++] = 0x00; // Append 0x00 frame delimiter

    return HAL_UART_Transmit(&huart2, enc, (uint16_t)enc_len, 100) == HAL_OK;
}

// Custom Transport: Poll decoded COBS frame buffered from ISR
static bool uart_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx) {
    (void)ctx;
    if (!rx_pkt_ready) {
        return false;
    }
    size_t n = rx_pkt_len;
    if (n > max_len) {
        rx_pkt_ready = false;
        return false;
    }
    memcpy(data, rx_pkt_buf, n);
    *out_len = n;
    rx_pkt_ready = false;
    return true;
}

static SYN_Transport transport = { .send = uart_send, .recv = uart_recv, .ctx = NULL };

// COBS Decoder Callback when a complete frame arrives
static void on_cobs_frame(const uint8_t *data, size_t len, void *ctx) {
    (void)ctx;
    if (!rx_pkt_ready && len <= sizeof(rx_pkt_buf)) {
        memcpy(rx_pkt_buf, data, len);
        rx_pkt_len = len;
        rx_pkt_ready = true;
    }
}

// Single-Byte UART Interrupt Callback (No DMA)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        syn_cobs_decoder_feed(&cobs_dec, rx_byte);
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1); // Re-arm interrupt
    }
}

// Message Handler Callback
static void on_led_command(const SYN_Packet *pkt, void *ctx) {
    (void)ctx;
    if (pkt->len > 0 && pkt->payload[0] == 0x01) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // LED ON
    }
}

void app_init(void) {
    syn_cobs_decoder_init(&cobs_dec, cobs_buf, sizeof(cobs_buf), on_cobs_frame, NULL);

    syn_router_init(&router, SLAVE_NODE_ID, &transport, handlers, 4);
    syn_router_register(&router, MSG_TYPE_LED, on_led_command, NULL);

    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

void app_loop(void) {
    syn_router_poll(&router);
}
```

---

## 2. M-Bus Protocol (`syn_mbus.h`)

The M-Bus (Meter-Bus) driver supports utility metering devices (water, gas, electricity, heat meters) compliant with EN 13757-2 / EN 13757-3.

### Features
- Single-byte ACK (`0xE5`) parsing.
- Short frame (`0x10`) and Long frame (`0x68`) header validation.
- Arithmetic checksum calculation and byte-at-a-time streaming parser.

```c
#include <syntropic/proto/syn_mbus.h>

void parse_mbus_stream(const uint8_t *buffer, size_t len) {
    SYN_MBUS_Frame frame;
    if (syn_mbus_decode_frame(buffer, len, &frame) == SYN_OK) {
        printf("M-Bus Frame Received! C-Field: 0x%02X, Address: 0x%02X\n",
               frame.c_field, frame.a_field);
    }
}
```

---

## 3. BACnet MS/TP Protocol (`syn_bacnet.h`)

The BACnet MS/TP protocol engine implements ANSI/ASHRAE 135 / ISO 16484-5 token-passing serial communication over RS485 without dynamic memory allocation (`malloc`).

### Features
- **MS/TP Framing & CRC**: Encodes/decodes Preamble (`0x55 0xFF`), Header CRC-8, and Data CRC-16 (ANSI X3.28 polynomial).
- **APDU Services**: Supports `Who-Is` / `I-Am` unconfirmed services and `ReadProperty` / `WriteProperty` confirmed services.
- **Static Object Database**: Manages Device, Analog Input (AI), Analog Output (AO), Binary Input (BI), and Binary Output (BO) objects.

```c
#include <syntropic/proto/syn_bacnet.h>

static SYN_BACnet_Node node;

void setup_bacnet(void) {
    syn_bacnet_node_init(&node, 12, 123456); // MAC 12, Device ID 123456
    syn_bacnet_add_object(&node, SYN_BACNET_OBJ_ANALOG_INPUT, 1, 23.5f, "Temperature");
}
```

---

## 4. DALI Protocol (`syn_dali.h`)

The DALI (Digital Addressable Lighting Interface / IEC 62386-101/102) engine implements Master and Control Gear (Slave) lighting nodes.

### Features
- **Manchester Encoding/Decoding**: Bi-phase bit encoding/decoding helper functions.
- **Forward & Backward Frames**: 16-bit Master Forward Frame decoding (`syn_dali_decode_forward`) and 8-bit Slave Backward Frame encoding (`syn_dali_encode_backward`).
- **Direct Arc Power Control (DAPC)**: Manages logarithmic/linear arc dimming levels (0..254), min/max bounds, power-on levels, and system failure levels.

---

## 5. Modbus RTU Master & Slave (`syn_modbus.h`, `syn_modbus_master.h`)

SyntropicOS provides non-blocking state-machine drivers for Modbus RTU Slave (`proto/syn_modbus.h`), Modbus RTU Master (`proto/syn_modbus_master.h`), and Modbus TCP MBAP ADU framing (`proto/syn_modbus_tcp.h`).

### Features
- **Non-blocking State Machine**: Processing occurs without thread blocking via periodic polling (`syn_modbus_process` / `syn_modbus_master_process`).
- **Inter-Frame Silence (t3.5)**: Timing gap detection handles partial frame arrival cleanly over UART DMA / ring buffers.
- **Modbus TCP (`proto/syn_modbus_tcp.h`)**: Encodes and decodes 7-byte MBAP headers (`SYN_MBAP_Header`, `syn_mbap_encode_header`, `syn_mbap_decode_header`) and wraps Slave/Client ADUs (`syn_modbus_tcp_process_slave`, `syn_modbus_tcp_build_client_adu`).

---

## 6. Zero-Heap Ethernet & IP Protocol Suite (`syn_eth`, `syn_dhcp`, `syn_icmp`, `syn_autoip`, `syn_netcfg`)

SyntropicOS provides a standalone, zero-heap Ethernet II, TCP/UDP/IPv4, TLS/DTLS 1.3, and application networking stack engineered for embedded microcontrollers.

### Core Link, IP & Transport Sub-Modules
- **Zero-Copy Packet Buffer Pool (`net/syn_netbuf.h`)**: Reference-counted packet buffers (`SYN_NetBuf`, `SYN_NetBufPool`, `syn_netbuf_alloc`, `syn_netbuf_push`, `syn_netbuf_free`) avoiding per-layer payload copies.
- **Ethernet II & ARP (`net/syn_eth.h`)**: Raw Ethernet II framing, MAC address filtering, and configurable static ARP table cache (`SYN_ETH_ARP_CACHE_SIZE`, defaults to 8 entries).
- **Native TCP Engine (`net/syn_tcp.h`)**: Zero-alloc IPv4 TCP state machine (`SYN_TCP_LISTEN`, `SYN_TCP_SYN_RCVD`, `SYN_TCP_ESTABLISHED`, `SYN_TCP_FIN_WAIT`), 3-way handshake (`SYN` → `SYN-ACK` → `ACK`), sequence/ACK tracking, and non-blocking `PT_TCP_BLOCK_READ` protothread task macro.
- **Native UDP Engine (`net/syn_udp.h`)**: Zero-alloc IPv4 UDP demuxing stack, 8-byte header construction, pseudo-header checksum (`syn_udp_checksum`), targeted destination port waking (`syn_task_resume`), and `PT_UDP_BLOCK_READ` protothread macro.
- **Pluggable Transport & Socket Bridges (`net/syn_transport.h`, `net/syn_transport_tcp.h`, `net/syn_transport_udp.h`)**: Generic `SYN_Transport` byte-stream/datagram vtable (`syn_transport_send`, `syn_transport_recv`), TCP socket adapter (`SYN_TransportTcp`, `syn_transport_tcp_init`), and UDP dual-stack bridge connecting `syn_port_udp_*` to `syn_udp`.
- **DHCP Client (`net/syn_dhcp.h`)**: RFC 2131 BOOTP/DHCP client state machine (`DISCOVER` → `OFFER` → `REQUEST` → `ACK`) over UDP ports 67/68.
- **ICMP & IGMPv2 (`net/syn_icmp.h`, `net/syn_igmp.h`)**: RFC 792 ICMP Echo Request / Reply (Ping) engine (`SYN_USE_ICMP`) and RFC 2236 IGMPv2 IPv4 multicast group join/leave/report engine (`SYN_USE_IGMP`, `syn_igmp_join_group`, `syn_igmp_leave_group`).
- **RFC 3927 AutoIP (`net/syn_autoip.h`)**: Link-Local `169.254.x.x` address selection, ARP probing, and collision recovery.
- **Network IP Manager (`net/syn_netcfg.h`)**: Unified IP configuration manager supporting Static IP, DHCP, automatic AutoIP fallback, and physical Link Up / Link Down state transitions.

### Security, Naming, Time & Application Sub-Modules (`net/*.h`)

| Header | Config Switch | Description |
|---|---|---|
| `net/syn_tls.h` | `SYN_USE_TLS` | Zero-heap RFC 8446 TLS 1.3 engine & `SYN_Transport` adapter (`SYN_TLS_Config`, `syn_tls_handshake`, `syn_tls_task`, `syn_tls_send`, `syn_tls_recv`) |
| `net/syn_dtls.h` | `SYN_USE_DTLS` | Zero-heap RFC 9147 DTLS 1.3 datagram security engine with anti-replay sliding window (`syn_dtls_init`, `syn_dtls_handshake`, `syn_dtls_send`) |
| `net/syn_wg.h` | `SYN_USE_WG` | WireGuard VPN client implementing Noise_IKpsk2 over X25519, ChaCha20-Poly1305, and BLAKE2s (`SYN_WG`, `syn_wg_init`, `syn_wg_task`, `syn_wg_send`) |
| `net/syn_dns.h` | `SYN_USE_DNS` | Cooperative UDP DNS A-record resolver and `.local` mDNS responder (`SYN_DnsResolver`, `syn_dns_resolve_task`, `SYN_Mdns`, `syn_mdns_task`) |
| `net/syn_sntp.h` | `SYN_USE_SNTP` | SNTPv4 UDP client resolving Unix epoch seconds and nanoseconds (`SYN_SNTP`, `syn_sntp_init`, `syn_sntp_task`, `syn_sntp_get_epoch_ns`) |
| `net/syn_ntp_server.h` | `SYN_USE_NTP_SERVER` | Stratum-configurable NTPv4 server over UDP port 123 (`SYN_NTPServer`, `syn_ntp_server_init`, `syn_ntp_server_process_packet`) |
| `net/syn_http.h` | `SYN_USE_HTTP` | Cooperative streaming HTTP/1.1 client (`SYN_HttpClient`, `syn_http_client_init`, `syn_http_client_task`) |
| `net/syn_httpd.h` | `SYN_USE_HTTPD` | Zero-alloc route-table HTTP/1.1 embedded server (`SYN_HttpdRoute`, `syn_httpd_init`, `syn_httpd_task`, `syn_httpd_step`) |
| `net/syn_websocket.h` | `SYN_USE_WEBSOCKET` | RFC 6455 WebSocket upgrade, masking, and frame coroutine over `syn_httpd` (`SYN_WebsocketSession`, `syn_websocket_upgrade`, `syn_websocket_task`) |
| `net/syn_mqtt.h` | `SYN_USE_MQTT` | Cooperative MQTT 3.1.1 / 5.0 client supporting publish, subscribe, keep-alive ping, and user properties (`syn_mqtt_init`, `syn_mqtt_publish`, `syn_mqtt_subscribe`) |
| `net/syn_coap.h` | `SYN_USE_COAP` | RFC 7252 CoAP parser, serializer, Block-Wise transfer (`RFC 7959`), and request coroutine (`SYN_CoapMsg`, `syn_coap_parse`, `syn_coap_request_task`) |
| `net/syn_heartbeat.h` | `SYN_USE_HEARTBEAT` | Multi-peer liveness monitor firing peer-found and peer-lost timeout callbacks (`SYN_Heartbeat`, `syn_heartbeat_add_peer`, `syn_heartbeat_update`) |

```c
#include <syntropic/syntropic.h>

static SYN_ETH eth;
static SYN_NETCFG netcfg;

void on_link_change(SYN_NETCFG *cfg, SYN_NETCFG_LinkState state, void *user_data) {
    if (state == SYN_NETCFG_LINK_UP) {
        printf("Ethernet Cable Plugged In!\n");
    } else {
        printf("Ethernet Cable Unplugged!\n");
    }
}

void app_init(const uint8_t mac[6]) {
    syn_eth_init(&eth, mac, 0);
    syn_netcfg_init(&netcfg, SYN_NETCFG_MODE_AUTO, mac); // Auto mode: DHCP with AutoIP fallback
    syn_netcfg_set_link_callback(&netcfg, on_link_change, NULL);
}
```

---

## 7. Unified Diagnostic Services (UDS / ISO 14229 & ISO 15765-2) (`syn_uds.h`, `syn_isotp.h`)

SyntropicOS includes a zero-heap UDS ISO 14229 diagnostic server engine operating over CAN / ISO 15765-2 transport.

### 0x11 ECU Reset Deferred Integration

When handling `0x11 ECUReset` requests, the ECU **must** transmit the positive ACK payload (`0x51 <sub-function>`) over CAN/ISO-TP **before** resetting the MCU hardware.

SyntropicOS provides native deferred post-TX reset callbacks with a configurable delay window (50 ms default):

```c
#include <syntropic/proto/syn_uds.h>

static SYN_UDS_Server g_uds;

static void on_ecu_reset(uint8_t reset_type, void *ctx) {
    (void)ctx;
    switch (reset_type) {
    case SYN_UDS_RESET_HARD:
    case SYN_UDS_RESET_KEY_OFF_ON:
    case SYN_UDS_RESET_SOFT:
    default:
        syn_port_system_reset(); /* Trigger hardware system reset 50 ms after 0x51 response */
        break;
    }
}

void uds_init(void) {
    syn_uds_init(&g_uds);
    syn_uds_set_reset_handler(&g_uds, on_ecu_reset, NULL);
    syn_uds_set_reset_wait_ms(&g_uds, 50); /* 50 ms post-TX delay window */
}

void uds_task_10ms(uint32_t dt_ms) {
    syn_uds_tick(&g_uds, dt_ms); /* Automatically triggers on_ecu_reset callback after 50 ms */
}
```

### Per-DID Session & Security Authorization Filters

DIDs can be registered with explicit session and security level bitmask filters using `syn_uds_register_did_ext`:

```c
uint8_t engine_speed[2];
/* Register DID 0xF190 accessible in DEFAULT/EXTENDED sessions and SECURITY LEVEL 1 */
syn_uds_register_did_ext(&g_uds, 0xF190, engine_speed, sizeof(engine_speed), true,
                         SYN_UDS_SESSION_MASK_DEFAULT | SYN_UDS_SESSION_MASK_EXTENDED,
                         SYN_UDS_SECURITY_MASK_LEVEL_1);
```

### Addressing Mode Support (Physical vs. Functional)

ISO 14229-1 mandates differentiation between **Physical** (1:1 point-to-point) and **Functional** (1:N broadcast) CAN addressing modes:

- `SYN_UDS_ADDR_PHYSICAL`: Point-to-point requests. Positively or negatively responded to as per protocol rules.
- `SYN_UDS_ADDR_FUNCTIONAL`: Broadcast requests. Functional-supported services (`0x10`, `0x11`, `0x14`, `0x19`, `0x22`, `0x28`, `0x29`, `0x31`, `0x3E`, `0x83`, `0x85`, `0x87`) are processed; physical-only services (`0x23`, `0x24`, `0x27`, `0x2A`, `0x2C`, `0x2E`, `0x2F`, `0x34`, `0x35`, `0x36`, `0x37`, `0x38`, `0x3D`, `0x84`, `0x86`) are silently dropped (`resp_len = 0`). Standard negative responses (`0x11`, `0x12`, `0x7E`) on functional requests are suppressed to prevent bus flooding.

```c
/* Processing a physical UDS request */
syn_uds_process_request(&g_uds, req_buf, req_len, resp_buf, sizeof(resp_buf), &resp_len, SYN_UDS_ADDR_PHYSICAL);

/* Processing a functional broadcast request */
syn_uds_process_request(&g_uds, req_buf, req_len, resp_buf, sizeof(resp_buf), &resp_len, SYN_UDS_ADDR_FUNCTIONAL);
```

### Per-Service Session & Security Policy Overrides

Configure custom session permission masks or required security levels for top-level Service Identifiers (SIDs):

```c
/* Restrict ReadDataByIdentifier (0x22) to EXTENDED session */
syn_uds_set_service_session_mask(&g_uds, 0x22, SYN_UDS_SESSION_MASK_EXTENDED);

/* Require Security Level 1 before ReadDataByIdentifier (0x22) can be executed */
syn_uds_set_service_security_mask(&g_uds, 0x22, SYN_UDS_SECURITY_MASK_LEVEL_1);
```

### AccessTimingParameter (0x83) Callback Handler

Register a callback handler to monitor or override P2Server_max and P2*Server_max timing parameters for Service `0x83` subfunctions (`0x01` Read Extended, `0x02` Set Default, `0x03` Read Active, `0x04` Set Given):

```c
static bool on_access_timing(SYN_UDS_AccessTimingType timing_type, uint16_t *p2_max_ms,
                             uint16_t *p2_star_max_10ms, void *ctx)
{
    (void)ctx;
    switch (timing_type) {
    case SYN_UDS_TIMING_READ_EXTENDED:
    case SYN_UDS_TIMING_READ_ACTIVE:
        if (p2_max_ms != NULL) *p2_max_ms = 50U;         /* 50 ms */
        if (p2_star_max_10ms != NULL) *p2_star_max_10ms = 500U; /* 5000 ms */
        return true;
    case SYN_UDS_TIMING_SET_TO_DEFAULT:
        if (p2_max_ms != NULL) *p2_max_ms = 50U;
        if (p2_star_max_10ms != NULL) *p2_star_max_10ms = 500U;
        return true;
    case SYN_UDS_TIMING_SET_TO_GIVEN:
        return (p2_max_ms != NULL && *p2_max_ms >= 10U);
    default:
        return false;
    }
}

/* Register handler on UDS server context */
syn_uds_register_access_timing(&g_uds, on_access_timing, NULL);
```

### Diagnostic Trouble Code (DTC) Management & ISO 14229-1 Lifecycle

SyntropicOS implements normative ISO 14229-1 DTC status bit management, fault detection counters, and operation cycle boundaries directly in the core stack:

- **Normative State Machine**: Tracks `DTCStatusByte` bits (`testFailed`, `testFailedThisOperationCycle`, `pendingDTC`, `confirmedDTC`, `testNotCompletedSinceLastClear`, `testFailedSinceLastClear`, `testNotCompletedThisOperationCycle`).
- **Single Source of Truth**: The core `SYN_UDS_Server` context manages status bits and fault detection counters (-128 to +127). Applications should not duplicate status byte tracking.
- **OEM Extension Hook (`dtc_cb`)**: Snapshot DIDs (subfunction `0x04`) and Extended Data records (subfunction `0x06`) are defined by vehicle manufacturers and delegated cleanly via `syn_uds_register_dtc_handler()`.

```c
/* 1. Registration at initialization */
syn_uds_register_dtc(&g_uds, 0x00B10016U, 0x00, SYN_UDS_DTC_SEVERITY_CHECK_IMMEDIATELY);

/* 2. Diagnostic monitors report test results (pass / fail) */
syn_uds_dtc_report_test_result(&g_uds, 0x00B10016U, true);  /* Sets testFailed, pending, confirmed; FDC = +127 */
syn_uds_dtc_report_test_result(&g_uds, 0x00B10016U, false); /* Clears testFailed; FDC = -128 */

/* 3. Driving / Operation cycle boundary */
syn_uds_dtc_start_operation_cycle(&g_uds); /* Resets testFailedThisOpCycle; clears pendingDTC if cycle completed clean */

/* 4. Querying authoritative status & SAE J2012 string formatting (proto/syn_uds_util.h) */
uint8_t dtc_status = 0;
char dtc_str[6];
syn_uds_dtc_get_status(&g_uds, 0x00B10016U, &dtc_status);
syn_uds_dtc_to_str(0x00B10016U, dtc_str); /* Formats "B1001" / parses via syn_uds_dtc_from_str() */
```

---

## 8. DoIP Protocol Stack (`syn_doip`)

SyntropicOS provides a zero-malloc ISO 13400-2 **Diagnostic over IP (DoIP)** transport layer for automotive Ethernet diagnostics.

### DoIP Features & Characteristics
- **8-Byte Header Serialization**: Big-endian packing for `Protocol Version` (`0x02`), `Inverse Protocol Version` (`0xFD`), 2-byte Payload Type, and 4-byte Payload Length.
- **Vehicle Discovery & Identification**: Supports UDP Vehicle Identification Requests (`0x0001`/`0x0002`/`0x0003`) and responds with Vehicle Announcement Messages (`0x0004`) containing VIN, EID (MAC), GID, and logical address.
- **Routing Activation**: Handles TCP Routing Activation Requests (`0x0005`) and returns Routing Activation Responses (`0x0006`).
- **UDS Message Dispatch**: Unpacks TCP Diagnostic Messages (`0x8001`), validates target logical address, passes UDS payload directly to `syn_uds_process_request()`, and frames positive response / NACK frames (`0x8002`/`0x8003`).

### Example Integration

```c
#include <syntropic/proto/syn_doip.h>
#include <syntropic/proto/syn_uds.h>

static SYN_DoIP_Server g_doip;
static SYN_UDS_Server g_uds;

void doip_setup(void)
{
    syn_uds_init(&g_uds);
    syn_doip_init(&g_doip, 0x1001); /* Logical Address 0x1001 */
    
    uint8_t vin[17] = "SYNTROPICOS123456";
    uint8_t eid[6]  = {0x00, 0x80, 0xE1, 0x01, 0x02, 0x03};
    uint8_t gid[6]  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    syn_doip_set_identifiers(&g_doip, vin, eid, gid);
}

void on_socket_rx(const uint8_t *rx_buf, uint16_t rx_len, uint8_t *tx_buf, uint16_t max_tx, uint16_t *tx_len)
{
    syn_doip_process_msg(&g_doip, &g_uds, rx_buf, rx_len, tx_buf, max_tx, tx_len);
}
```

---

## 9. DNS-Based Service Discovery (`syn_dnssd`)

Cleanroom multicast DNS-SD responder (RFC 6763 / RFC 6762) on UDP port 5353 (`224.0.0.251`) providing zero-allocation service advertisement.

### DNS-SD Features
- **Resource Records**: Automatic encoding of PTR (service type), SRV (port + hostname), TXT (attributes), and A (IPv4) records.
- **Gratuitous Announcement**: `syn_dnssd_announce()` broadcasts multicast announcements on service registration.
- **Query Demuxing**: `syn_dnssd_process_query()` matches incoming queries and replies with authoritative response packets.
- **Active Service Browsing**: `syn_dnssd_browser_init()` and cooperative protothread coroutine `syn_dnssd_browse_task()` query and stream discovered nodes to application callbacks.

```c
#include <syntropic/net/syn_dnssd.h>

static SYN_DnsSd g_dnssd;
static SYN_DnsSd_Browser g_browser;

static void on_node_found(const SYN_DnsSd_Discovered *svc, void *user_data) {
    printf("Discovered %s on %d.%d.%d.%d:%u (host: %s, txt: %s)\n",
           svc->instance_name, svc->ip[0], svc->ip[1], svc->ip[2], svc->ip[3],
           svc->port, svc->hostname, svc->txt);
}

void dnssd_setup(void) {
    syn_dnssd_init(&g_dnssd);

    /* 1. Register and announce local service */
    SYN_DnsSd_Service svc = {
        .instance_name = "Syntropic Sensor Node",
        .service_type = "_http._tcp",
        .hostname = "sensor-node",
        .port = 80,
        .ip = {192, 168, 1, 150},
        .txt_records = {"version=1.0", "vendor=Syntropic"},
        .txt_count = 2
    };
    syn_dnssd_register(&g_dnssd, &svc);

    uint8_t buf[1024];
    size_t len = 0;
    syn_dnssd_announce(&g_dnssd, 0, buf, sizeof(buf), &len);

    /* 2. Start cooperative service discovery browser (5-second window) */
    syn_dnssd_browser_init(&g_browser, &g_dnssd, "_coap._udp", on_node_found, NULL, 5000);
}
```

---

## 10. OMA LwM2M Client & Autonomous Supervisor (`proto/syn_lwm2m.h`, `proto/syn_lwm2m_task.h`)

`proto/syn_lwm2m.h` (`SYN_USE_LWM2M`) implements the zero-heap OMA LwM2M v1.1/v1.2 client object tree (`SYN_LwM2M_Client`, `SYN_LwM2M_Object`), TLV and Core Link-Format encoders (`syn_lwm2m_tlv_encode_*`), and IPSO Device/Sensor object builders. `proto/syn_lwm2m_task.h` (`SYN_USE_LWM2M_TASK`) wraps the client in an event-driven supervisor task managing the complete connection lifecycle:
- **Autonomous Registration**: Initial registration with core Link-Format serialization (`</3/0>`, `</5/0>`, `</3303/0>`).
- **Autonomous Lifetime Renewal**: Automatic update requests dispatched when reaching 80% of configured `lifetime_s`.
- **RFC 7641 Observe & Notify**: Periodic evaluation of resource changes with `pmin` suppression and `pmax` expiry.
- **DTLS 1.3 Integration**: Cooperative TLS/DTLS handshake advancement via `SYN_CoapsClient`.
- **System Hook Routing**: Automatic invocation of reboot, factory reset, and OTA firmware update handlers.

```c
#include <syntropic/proto/syn_lwm2m_task.h>

static SYN_LwM2M_Client g_client;
static SYN_LwM2M_Task g_task;
static SYN_LwM2M_DeviceContext g_dev_ctx;
static SYN_LwM2M_SensorContext g_temp_ctx;
static SYN_LwM2M_Object g_dev_obj;
static SYN_LwM2M_Object g_temp_obj;

static uint8_t g_rx_scratch[512];
static uint8_t g_tx_scratch[512];

static void on_reboot(void *user_data) {
    (void)user_data;
    syn_port_system_reset();
}

void lwm2m_app_init(SYN_Transport *transport) {
    syn_lwm2m_client_init(&g_client, "syn-edge-01", 300U, transport);

    g_dev_obj = syn_lwm2m_make_device_object(&g_dev_ctx);
    g_temp_obj = syn_lwm2m_make_temperature_object(&g_temp_ctx);
    syn_lwm2m_register_object(&g_client, &g_dev_obj);
    syn_lwm2m_register_object(&g_client, &g_temp_obj);

    SYN_LwM2M_TaskConfig cfg = {
        .client = &g_client,
        .transport = transport,
        .on_reboot = on_reboot,
        .rx_buf = g_rx_scratch,
        .rx_buf_size = sizeof(g_rx_scratch),
        .tx_buf = g_tx_scratch,
        .tx_buf_size = sizeof(g_tx_scratch),
    };
    syn_lwm2m_task_init(&g_task, &cfg);
}
```

---

## 11. Micro XRCE-DDS Client (`proto/syn_xrce_dds.h`)

Zero-heap, deterministic client engine for Micro-ROS & eProsima Micro XRCE-DDS v1.2:
- **CDR Serialization**: Standard Little-Endian CDR serializer/deserializer with strict alignment rules.
- **Entity Tree**: Complete ROS 2 hierarchy (`DomainParticipant`, `Topic`, `Publisher`, `Subscriber`, `DataWriter`, `DataReader`).
- **Framing & Transports**: XRCE submessage framing over any `SYN_Transport` (UART, UDP, CAN).

```c
#include <syntropic/proto/syn_xrce_dds.h>

static SYN_XRCE_Client g_xrce_client;
static uint8_t g_xrce_rx[256];
static uint8_t g_xrce_tx[256];

void xrce_setup(SYN_Transport *transport) {
    SYN_XRCE_Config cfg = {
        .transport = transport,
        .client_key = 0xCAFEBABEU,
        .session_id = 0x81U,
        .rx_buf = g_xrce_rx,
        .rx_buf_size = sizeof(g_xrce_rx),
        .tx_buf = g_xrce_tx,
        .tx_buf_size = sizeof(g_xrce_tx),
    };
    syn_xrce_client_init(&g_xrce_client, &cfg);
    syn_xrce_client_create_session(&g_xrce_client);
    syn_xrce_client_create_participant(&g_xrce_client, 1U);
    syn_xrce_client_create_topic(&g_xrce_client, 2U, 1U, "rt/chatter", "std_msgs::msg::dds_::String_");
    syn_xrce_client_create_publisher(&g_xrce_client, 3U, 1U);
    syn_xrce_client_create_datawriter(&g_xrce_client, 4U, 3U, 2U);
}
```

---

## 12. Micro OPC UA Embedded Server (`proto/syn_opcua.h`)

Zero-heap, deterministic IEC 62541 OPC UA binary server:
- **Protocol Framing**: UACP (`HEL`/`ACK`/`ERR`) and UASC (`OPN`/`CLO`/`MSG`) framing.
- **Core Services**: Read, Write, Browse, CreateSession, ActivateSession, CloseSession.
- **Address Space**: Static in-memory address space with Object and Variable nodes, custom DataValue callbacks.

```c
#include <syntropic/proto/syn_opcua.h>

static SYN_OPCUA_Server g_opcua_srv;
static uint8_t g_opcua_rx[512];
static uint8_t g_opcua_tx[512];

void opcua_setup(SYN_Transport *transport) {
    SYN_OPCUA_Config cfg = {
        .transport = transport,
        .endpoint_url = "opc.tcp://0.0.0.0:4840",
        .server_name = "SyntropicOS Edge Server",
        .rx_buf = g_opcua_rx,
        .rx_buf_size = sizeof(g_opcua_rx),
        .tx_buf = g_opcua_tx,
        .tx_buf_size = sizeof(g_opcua_tx),
    };
    syn_opcua_server_init(&g_opcua_srv, &cfg);

    SYN_OPCUA_Node var_node = {
        .node_id = {.ns_index = 1U, .id_type = SYN_OPCUA_NODEID_NUMERIC, .id = {.num = 1001U}},
        .browse_name = "SensorTemperature",
        .display_name = "SensorTemperature",
        .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
        .data_type = SYN_OPCUA_TYPE_FLOAT,
        .value = {.value = {.type = SYN_OPCUA_TYPE_FLOAT, .val = {.float_val = 23.5f}},
                  .status_code = SYN_OPCUA_STATUS_GOOD},
        .access_level = 0x01U, /* Read-Only */
    };
    syn_opcua_server_register_node(&g_opcua_srv, &var_node);
}
```

---

## 13. IO-Link Master & Device Protocol Engine (`proto/syn_iolink.h`)

Zero-heap IEC 61131-9 point-to-point digital protocol engine:
- **Physical Rates**: COM1 (4.8k), COM2 (38.4k), COM3 (230.4k) baud.
- **Framing & Checksum**: Standard 6-bit CRC with parity for M-Sequence Types 0, 1_1, 1_2, 2_1, 2_2, 2_V.
- **ISDU & Process Data**: Cyclic process data exchange and acyclic Direct Parameter Page 1 and ISDU parameter read/write.

```c
#include <syntropic/proto/syn_iolink.h>

static SYN_IOLink_Master g_iolink_master;
static uint8_t g_iolink_rx[128];
static uint8_t g_iolink_tx[128];

void iolink_setup(SYN_Transport *transport) {
    SYN_IOLink_MasterConfig cfg = {
        .transport = transport,
        .cycle_time_ms = 5U,
        .rx_buf = g_iolink_rx,
        .rx_buf_size = sizeof(g_iolink_rx),
        .tx_buf = g_iolink_tx,
        .tx_buf_size = sizeof(g_iolink_tx),
    };
    syn_iolink_master_init(&g_iolink_master, &cfg);
    syn_iolink_master_start(&g_iolink_master);
}
```

---

## 14. MQTT-SN Protocol Client Engine (`proto/syn_mqttsn.h`)

Zero-heap MQTT for Sensor Networks (MQTT-SN v1.2) client engine for constrained datagram transports (UDP, Zigbee, Thread, BLE, Sub-GHz):
- **Framing**: 1-byte and 3-byte variable header lengths (payloads up to 64 KB).
- **Gateway Discovery**: `SEARCHGW`, `GWINFO`, `ADVERTISE`.
- **Topic Addressing**: Normal registered topics (16-bit IDs via `REGISTER`/`REGACK`), Predefined topic IDs, and Short 2-character topic IDs without registration handshake.
- **QoS Support**: QoS 0, QoS 1 (with msg_id ack), and transparent QoS -1 (fire-and-forget without connect).
- **Power Optimization**: Low-power `SLEEP` state and `AWAKE` ping flush.
- **Protothread Integration**: `syn_mqttsn_client_pt()` coroutine for non-blocking task loops.

```c
#include <syntropic/proto/syn_mqttsn.h>

static SYN_MQTTSN_Client g_sn_client;
static uint8_t g_sn_rx[256], g_sn_tx[256];

void on_sn_message(SYN_MQTTSN_Client *client, uint16_t topic_id, uint8_t topic_type,
                   const uint8_t *payload, size_t len, void *user_data) {
    (void)client;
    (void)topic_id;
    (void)topic_type;
    (void)payload;
    (void)len;
    (void)user_data;
    /* Process incoming sensor command */
}

void mqttsn_setup(SYN_Transport *datagram_transport) {
    SYN_MQTTSN_Config cfg = {
        .transport = datagram_transport,
        .client_id = "sensor_node_01",
        .duration_s = 60U,
        .clean_session = true,
        .on_message = on_sn_message,
        .rx_buf = g_sn_rx,
        .rx_buf_size = sizeof(g_sn_rx),
        .tx_buf = g_sn_tx,
        .tx_buf_size = sizeof(g_sn_tx),
    };
    syn_mqttsn_client_init(&g_sn_client, &cfg);
    syn_mqttsn_client_searchgw(&g_sn_client, 1U);
}

void publish_telemetry(int16_t temperature_c) {
    /* Publish to short 2-char topic 'te' at QoS 0 */
    syn_mqttsn_client_publish_short(&g_sn_client, "te", 0, false, (const uint8_t *)&temperature_c, sizeof(temperature_c));
}
```

---

## 15. Industrial Fieldbuses: CANopen, DeviceNet & EtherCAT

| Header | Config Switch | Specification & Key API |
|---|---|---|
| `proto/syn_canopen.h` | `SYN_USE_CANOPEN` | CiA 301 CANopen Slave engine: Object Dictionary, Expedited/Segmented SDO Server, TPDO/RPDO mapping, NMT Slave, Heartbeat, and EMCY (`syn_canopen_init`, `syn_canopen_process_rx`, `syn_canopen_update`) |
| `proto/syn_canopen_mgr.h` | Always available | CiA 302 Network Manager, NMT Master, Heartbeat Consumer, and SDO Client (`SYN_CANOpenManager`, `syn_canopen_mgr_build_nmt`, `syn_canopen_mgr_sdo_read_init`) |
| `proto/syn_lss.h` | Always available | CiA 305 Layer Setting Services Master/Slave for dynamic node-ID and bit-timing assignment (`SYN_LSSSlave`, `syn_lss_slave_process`, `syn_lss_master_build_config_node_id`) |
| `proto/syn_cia303.h` | `SYN_USE_CANOPEN` | CiA 303-3 RUN/ERR LED blink-pattern generator driven by NMT and error state (`SYN_CiA303_Indicator`, `syn_cia303_step`) |
| `proto/syn_cia401.h` | Always available | CiA 401 Generic Digital/Analog I/O Module profile & delta-trigger helper (`SYN_CiA401_Device`, `syn_cia401_populate_od`, `syn_cia401_check_analog_delta`) |
| `proto/syn_cia402.h` | `SYN_USE_CIA402` | CiA 402 Drives and Motion Control FSA state machine (`Not Ready` → `Operation Enabled` / `Quick Stop`) and Controlword/Statusword codec (`syn_cia402_init`, `syn_cia402_set_controlword`) |
| `proto/syn_cia418.h` | Always available | CiA 418 Battery Module Object Dictionary index and sub-index definitions (`0x6000`–`0x60FF`) |
| `proto/syn_devicenet.h` | `SYN_USE_DEVICENET` | ODVA DeviceNet (CIP over CAN) Group 2 Only Server, Duplicate MAC ID check, Explicit/Polled I/O messaging (`SYN_CIP_DeviceNet`, `syn_devicenet_init`, `syn_devicenet_on_can_rx`) |
| `proto/syn_ethercat.h` | `SYN_USE_ETHERCAT` | Bare-metal IEEE 802.3 EtherType `0x88A4` EtherCAT Slave frame processor, AL state machine (`INIT` → `PREOP` → `SAFEOP` → `OP`), and CoE SDO mailbox (`syn_ecat_init`, `syn_ecat_slave_process_frame`) |

---

## 16. Automotive Calibration, Bus Management, LIN & EV Charging

| Header | Config Switch | Specification & Key API |
|---|---|---|
| `proto/syn_j1939.h` | `SYN_USE_J1939` | SAE J1939 29-bit CAN ID/PGN packing, 64-bit NAME address claiming, Transport Protocol (BAM / CMDT), and DM1 DTC encoding (`syn_j1939_id_pack`, `syn_j1939_name_encode`) |
| `proto/syn_n2k.h` | `SYN_USE_N2K` | NMEA 2000 marine CAN PGN encoders/decoders for rapid position, COG/SOG, vessel heading, and battery status (`syn_n2k_encode_position_rapid`, `syn_n2k_decode_position_rapid`) |
| `proto/syn_cannm.h` | `SYN_USE_CANNM` | AUTOSAR CAN Network Management state machine (`Bus-Sleep`, `Prepare Bus-Sleep`, `Repeat Message`, `Normal Operation`, `Ready Sleep`) (`SYN_CanNM_Session`, `syn_cannm_init`) |
| `proto/syn_ccp.h` | `SYN_USE_CCP` | ASAM CAN Calibration Protocol v2.1 Slave supporting CRO/DTO commands, MTA memory access, and synchronous DAQ lists (`syn_ccp_init`, `syn_ccp_process_cro`, `syn_ccp_daq_event`) |
| `proto/syn_xcp.h` | `SYN_USE_XCP` | Transport-agnostic ASAM XCP v1.x Slave (XCP-on-CAN / XCP-on-Ethernet / XCP-on-UART) with CTO command processor and ODT DAQ engine (`syn_xcp_init`, `syn_xcp_process_cto`) |
| `proto/syn_kwp2000.h` | Always available | ISO 14230-3 KWP2000 diagnostic server supporting LID/CID tables, diagnostic sessions, and ECU reset handlers (`SYN_KWP2000_Server`, `syn_kwp2000_init`) |
| `proto/syn_lin.h` | `SYN_USE_LIN` | LIN 2.1 / 2.2A & ISO 17987 schedule-table Master and Slave frame engine with protected ID parity and classic/enhanced checksums (`syn_lin_master_init`, `syn_lin_master_step`) |
| `proto/syn_lintp.h` | `SYN_USE_LINTP` | ISO 17987-2 LIN Transport Layer (`0x3C` MasterReq / `0x3D` SlaveResp) multi-frame segmentation for UDS-on-LIN (`SYN_LINTP_Link`, `syn_lintp_init`, `syn_lintp_send`) |
| `proto/syn_gbt27930.h` | `SYN_USE_GBT27930` | GB/T 27930 DC Fast Charging BMS & Charger handshake/parameter/charging state machine over J1939 CAN (`syn_gbt27930_init`, `syn_gbt27930_step`) |
| `proto/syn_ocpp.h` | Always available | OCPP-J 1.6 / 2.0.1 EV Charge Point JSON-RPC formatter and parser (`BootNotification`, `Heartbeat`, `StatusNotification`, `StartTransaction`, V2G modes) (`syn_ocpp_init`) |

---

## 17. Smart Metering, Stage Lighting & Power Management Buses

| Header | Config Switch | Specification & Key API |
|---|---|---|
| `proto/syn_cjt188.h` | `SYN_USE_CJT188` | CJ/T 188-2004 & 2018 water/gas/heat utility meter framing, BCD data decoding, and valve control (`SYN_CJT188_Decoder`, `syn_cjt188_encode_read_req`, `syn_cjt188_parse_frame`) |
| `proto/syn_dlt645.h` | Always available | DL/T 645-1997 & 2007 electricity meter `0x68` framing, `+0x33` byte scrambling, and streaming decoder (`SYN_DLT645_Decoder`, `syn_dlt645_encode`, `syn_dlt645_decoder_feed`) |
| `proto/syn_dmx512.h` | `SYN_USE_DMX512` | USITT DMX512-A 512-channel lighting universe Master frame builder and Slave slot receiver (`SYN_DMX512_Master`, `SYN_DMX512_Slave`, `syn_dmx512_master_set_channel`) |
| `proto/syn_smbus.h` | `SYN_USE_SMBUS` | System Management Bus v1.1–3.0 packet codec with CRC-8 Packet Error Checking (PEC) (`syn_smbus_calc_pec`, `syn_smbus_encode_packet`, `syn_smbus_decode_packet`) |
| `proto/syn_pmbus.h` | `SYN_USE_PMBUS` | Power Management Bus v1.2/1.3 command framing plus `LINEAR11` and `ULINEAR16` voltage/current float converters (`syn_pmbus_linear11_to_float`, `syn_pmbus_linear16_to_float`) |

---

## 18. UAV / RC Links, GNSS, Serial Modems & CBOR Object Security

| Header | Config Switch | Specification & Key API |
|---|---|---|
| `proto/syn_mavlink.h` | `SYN_USE_MAVLINK` | MAVLink v2 packet encoder and byte-streaming decoder with `CRC_EXTRA` validation (`SYN_MAVLINK_Parser`, `syn_mavlink_parse_byte`, `syn_mavlink_encode_msg`) |
| `proto/syn_msp.h` | `SYN_USE_MSP` | MultiWii / Betaflight Serial Protocol (MSP v1) telemetry & FC command codec (`SYN_MSP_Parser`, `syn_msp_parse_byte`, `syn_msp_encode_response`) |
| `proto/syn_crsf.h` | `SYN_USE_CRSF` | TBS Crossfire / ExpressLRS 420 kbaud 16-channel 11-bit RC & link-statistics parser (`SYN_CRSF_Parser`, `syn_crsf_parse_byte`, `syn_crsf_raw_to_us`) |
| `proto/syn_sbus.h` | `SYN_USE_SBUS` | Futaba / FrSky SBUS 25-byte 16-channel RC receiver decoder with failsafe/frame-lost flags (`SYN_SBUS_Parser`, `syn_sbus_parse_byte`, `syn_sbus_raw_to_us`) |
| `proto/syn_ibus.h` | `SYN_USE_IBUS` | FlySky IBUS 32-byte 14-channel serial receiver parser with 16-bit checksum verification (`SYN_IBUS_Parser`, `syn_ibus_parse_byte`) |
| `proto/syn_nmea.h` | `SYN_USE_NMEA` | NMEA 0183 GNSS streaming sentence parser (`GGA`, `RMC`, `GSA`, `VTG`) with integer micro-degree (`1e-6 deg`) coordinates (`syn_nmea_parser_feed`, `syn_udeg_to_deg`) |
| `proto/syn_at_parser.h` | `SYN_USE_AT_PARSER` | Stream-oriented cellular/Wi-Fi AT command response & URC parser with `+CME ERROR` extraction (`SYN_AtParser`, `syn_at_parser_feed_stream`) |
| `proto/syn_ymodem.h` | `SYN_USE_YMODEM` | YMODEM / XMODEM-1K CRC-16 batch serial file receiver for bootloader & VFS transfers (`SYN_YMODEM_Receiver`, `syn_ymodem_receiver_init`, `syn_ymodem_receive`) |
| `proto/syn_ir.h` | `SYN_USE_IR` | Infrared pulse-distance/bi-phase remote encoder and decoder supporting NEC, Sony SIRC, Philips RC5/RC6, Samsung, and Panasonic (`syn_ir_decode_pulse`, `syn_ir_encode_frame`) |
| `proto/syn_cose.h` | `SYN_USE_COSE` | RFC 9052 / RFC 8152 CBOR Object Signing and Encryption (`COSE_Sign1`, `COSE_Encrypt0`, `COSE_Mac0`) backed by SyntropicOS Ed25519/P-256/ChaCha20-Poly1305/AES-GCM (`syn_cose_sign1_create`, `syn_cose_sign1_verify`) |

