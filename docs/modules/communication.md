# Communication

## Framing & Fieldbus Protocols

| Module | Header | Config | Description |
|---|---|---|---|
| COBS | `proto/syn_cobs.h` | `SYN_USE_COBS` | Consistent Overhead Byte Stuffing — packet framing with a streaming byte-at-a-time decoder. Zero-delimited packets over any byte stream. |
| Modbus RTU Slave | `proto/syn_modbus.h` | `SYN_USE_MODBUS` | Complete 19-function-code Modbus RTU slave implementation: FC01 (Read Coils), FC02 (Read Discrete Inputs), FC03 (Read Holding), FC04 (Read Input), FC05 (Write Single Coil), FC06 (Write Single Register), FC07 (Read Exception Status), FC08 (Diagnostics Echo), FC0B (Get Comm Event Counter), FC0C (Get Comm Event Log), FC0F (Write Multiple Coils), FC10 (Write Multiple Registers), FC11 (Report Server ID), FC14 (Read File Record), FC15 (Write File Record), FC16 (Mask Write Register), FC17 (Read/Write Multiple), FC18 (Read FIFO Queue), FC2B/0E (Read Device Identification). Uses `syn_crc16_modbus()`. |
| Modbus RTU Master | `proto/syn_modbus_master.h` | `SYN_USE_MODBUS` | Non-blocking Modbus RTU Master / Client state machine supporting FC01, FC02, FC03, FC04, FC05, FC06, FC0F, FC10, FC11, FC16, and FC18 queries, stream parsing, and tick-based timeout tracking. |
| Modbus TCP ADU | `proto/syn_modbus_tcp.h` | `SYN_USE_MODBUS` | MBAP 7-byte header encapsulation over TCP sockets (`syn_transport_tcp`), enabling zero-overhead Modbus TCP Client and Server functionality. |
| NMEA 0183 Navigation | `proto/syn_nmea.h` | `SYN_USE_NMEA` | Streaming NMEA 0183 sentence parser and decoder for GNSS/GPS devices (GGA, RMC, VTG, GSA, ZDA), XOR checksum validation, and DDMM.MMMM coordinate conversion. |
| CANopen DS301 Slave | `proto/syn_canopen.h` | `SYN_USE_CANOPEN` | Zero-allocation CANopen DS301 slave protocol engine supporting Object Dictionary (OD) lookup, SDO Server (expedited download/upload, segmented SDO, SDO Aborts), up to 4 RPDOs & 4 TPDOs, Heartbeat Producer, Emergency (EMCY) alarms, and NMT state machine management. |
| CANopen CiA 402 Drive | `proto/syn_cia402.h` | `SYN_USE_CIA402` | CANopen CiA 402 Servo Drive & Motion Control profile engine supporting standard 8-state power drive state machine, Controlword (0x6040), Statusword (0x6041), Profile Position (PP), Profile Velocity (PV), Profile Torque (PT), and Cyclic Synchronous (CSP/CSV) modes with S-Curve trajectory generation. |
| CANopen CiA 305 LSS | `proto/syn_lss.h` | `SYN_USE_CANOPEN` | Layer Setting Services (LSS) master and slave protocol engines for dynamic Node-ID allocation, bit-timing baud rate selection, fastscan, and non-volatile configuration store. |
| CANopen CiA 401 I/O | `proto/syn_cia401.h` | `SYN_USE_CANOPEN` | Device Profile for Generic I/O Modules providing standardized Object Dictionary entries for 8-bit digital inputs/outputs and 16-bit analog inputs/outputs with interrupt delta thresholds. |
| CANopen CiA 302 Manager | `proto/syn_canopen_mgr.h` | `SYN_USE_CANOPEN` | Network Manager, NMT Master, non-blocking SDO Client (expedited read/write), and 128-node heartbeat monitoring engine. |

## Cooperative Network Stack

All network modules are cooperative — they yield between operations so other scheduler tasks continue to run. Zero-allocation throughout.

!!! note "Include individually"
    The higher-level network modules (HTTP, MQTT, WebSocket, DNS) are **not** auto-included by the umbrella header `syntropic.h` because they depend on platform socket support. Include them directly in files that need them, e.g. `#include "syntropic/net/syn_http.h"`.

### Application Protocols

| Module | Header | Config | Description |
|---|---|---|---|
| HTTP Client | `net/syn_http.h` | `SYN_USE_HTTP` | Cooperative HTTP/1.1 client with streaming response handling |
| HTTP Server | `net/syn_httpd.h` | `SYN_USE_HTTPD` | Route-based minimal HTTP/1.1 server. Yields between accepts so other tasks keep running. |
| WebSockets | `net/syn_websocket.h` | `SYN_USE_WEBSOCKET` | WebSocket protocol support on the HTTP server |
| MQTT Client | `net/syn_mqtt.h` | `SYN_USE_MQTT` | Cooperative MQTT 3.1.1 client with publish, subscribe, and QoS support |
| DNS Resolver | `net/syn_dns.h` | `SYN_USE_DNS` | UDP DNS resolver and mDNS responder |
| CoAP | `net/syn_coap.h` | `SYN_USE_COAP` | Constrained Application Protocol (CoAP) client/server helper |

### Transport & Routing

| Module | Header | Config | Description |
|---|---|---|---|
| Transport | `net/syn_transport.h` | Always available | Transport layer interface abstraction |
| TCP Transport | `net/syn_transport_tcp.h` | `SYN_USE_TRANSPORT_TCP` | TCP socket transport implementation |
| Router | `net/syn_router.h` | `SYN_USE_ROUTER` | Software packet router / transport dispatcher |
| Heartbeat | `net/syn_heartbeat.h` | `SYN_USE_HEARTBEAT` | Peer discovery and keep-alive monitor |

### Network Services

| Module | Header | Config | Description |
|---|---|---|---|
| SNTP | `net/syn_sntp.h` | `SYN_USE_SNTP` | SNTPv4 client — single-query time sync with retry and periodic re-sync |

The SNTP client is a cooperative protothread task that syncs to a configurable NTP server.
Provides Unix-epoch timestamps with nanosecond sub-second resolution.

```c
#include "syntropic/net/syn_sntp.h"

static SYN_SNTP sntp;
SYN_SockAddr ntp_server = { .ip = SYN_IP4(216,239,35,0), .port = 123 };

syn_sntp_init(&sntp, &ntp_server, 3600);  /* re-sync every hour */

/* Register as a scheduler task */
syn_task_create(&tasks[0], "sntp", syn_sntp_task, 0, &sntp);

/* Read current time */
uint32_t epoch = syn_sntp_get_epoch_s(&sntp);
uint32_t ns    = syn_sntp_get_epoch_ns(&sntp);
```

---

## CANopen Ecosystem (CiA 301, 302, 305, 401, 402)

SyntropicOS features a complete, zero-allocation, bare-metal CANopen suite suitable for motor drives, sensor blocks, battery management systems (BMS), and central motion controllers.

### CANopen Layer Setting Services (CiA 305 LSS)

Configures Node-ID allocation and CAN bitrate over the CAN bus without physical DIP switches.

```c
#include "syntropic/proto/syn_lss.h"

static SYN_LSSSlave lss;
static SYN_LSSAddress lss_addr = {
    .vendor_id = 0x000000A5,
    .product_code = 0x12345678,
    .revision_no = 0x00010000,
    .serial_no = 0x88776655
};

/* Initialize slave with default unconfigured node ID 0xFF */
syn_lss_slave_init(&lss, &lss_addr, 0xFF);

/* Process incoming CAN frames on COB-ID 0x7E5 */
SYN_CAN_Frame rx_frame, tx_resp;
if (syn_lss_slave_process(&lss, &rx_frame, &tx_resp)) {
    syn_can_send(&can, &tx_resp); /* Transmit response on 0x7E4 */
}
```

### CANopen Generic I/O Profile (CiA 401)

Binds digital and analog I/O lines to standard CANopen Object Dictionary indices (`0x6000`, `0x6200`, `0x6401`, `0x6411`).

```c
#include "syntropic/proto/syn_cia401.h"

static SYN_CiA401_Device io_dev;
static SYN_CANOpenODEntry od_table[32];

/* Initialize 2 digital input bytes, 2 digital output bytes, 4 analog inputs, 2 analog outputs */
syn_cia401_init(&io_dev, 2, 2, 4, 2);

/* Populate Object Dictionary table */
size_t od_count = syn_cia401_populate_od(&io_dev, od_table, 32);
```

### CANopen Network Manager & Master (CiA 302)

Acts as a central CANopen Master node commanding remote slaves via NMT, reading/writing Object Dictionaries via non-blocking SDO, and monitoring remote node heartbeats.

```c
#include "syntropic/proto/syn_canopen_mgr.h"

static SYN_CANOpenManager mgr;
syn_canopen_mgr_init(&mgr);

/* Broadcast NMT Start Node to all slave nodes */
SYN_CAN_Frame nmt_frame;
syn_canopen_mgr_build_nmt(&nmt_frame, 0x00, SYN_CANOPEN_NMT_CMD_START);
syn_can_send(&can, &nmt_frame);

/* Issue SDO Read request to Node 5, Index 0x1000, Subindex 0x00 */
SYN_CAN_Frame sdo_req;
syn_canopen_mgr_sdo_read_init(&mgr, &sdo_req, 5, 0x1000, 0x00);
syn_can_send(&can, &sdo_req);
```

Used as a prerequisite by the [WireGuard VPN client](crypto.md#wireguard-vpn-client) for TAI64N handshake timestamps.
