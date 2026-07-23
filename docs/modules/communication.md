# Communication

## Framing & Fieldbus Protocols

| Module | Header | Config | Description |
|---|---|---|---|
| COBS | `proto/syn_cobs.h` | `SYN_USE_COBS` | Consistent Overhead Byte Stuffing — packet framing with a streaming byte-at-a-time decoder. Zero-delimited packets over any byte stream. |
| Modbus RTU Slave | `proto/syn_modbus.h` | `SYN_USE_MODBUS` | Complete 18-function-code Modbus RTU slave implementation: FC01 (Read Coils), FC02 (Read Discrete Inputs), FC03 (Read Holding), FC04 (Read Input), FC05 (Write Single Coil), FC06 (Write Single Register), FC07 (Read Exception Status), FC08 (Diagnostics Echo), FC0B (Get Comm Event Counter), FC0C (Get Comm Event Log), FC0F (Write Multiple Coils), FC10 (Write Multiple Registers), FC14 (Read File Record), FC15 (Write File Record), FC16 (Mask Write Register), FC17 (Read/Write Multiple), FC18 (Read FIFO Queue), FC2B/0E (Read Device Identification). Uses `syn_crc16_modbus()`. |
| Modbus RTU Master | `proto/syn_modbus_master.h` | `SYN_USE_MODBUS` | Non-blocking Modbus RTU Master / Client state machine supporting FC01, FC02, FC03, FC04, FC05, FC06, FC0F, FC10, FC16, and FC18 queries, stream parsing, and tick-based timeout tracking. |
| CANopen DS301 Slave | `proto/syn_canopen.h` | `SYN_USE_CANOPEN` | Zero-allocation CANopen DS301 slave protocol engine supporting Object Dictionary (OD) lookup, SDO Server (expedited download/upload, segmented SDO, SDO Aborts), up to 4 RPDOs & 4 TPDOs, Heartbeat Producer, Emergency (EMCY) alarms, and NMT state machine management. |
| CANopen CiA 402 Drive | `proto/syn_cia402.h` | `SYN_USE_CIA402` | CANopen CiA 402 Servo Drive & Motion Control profile engine supporting standard 8-state power drive state machine, Controlword (0x6040), Statusword (0x6041), Profile Position (PP), Profile Velocity (PV), Profile Torque (PT), and Cyclic Synchronous (CSP/CSV) modes with S-Curve trajectory generation. |

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

Used as a prerequisite by the [WireGuard VPN client](crypto.md#wireguard-vpn-client) for TAI64N handshake timestamps.
