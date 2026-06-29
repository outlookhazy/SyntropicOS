# Communication

## Framing & Fieldbus Protocols

| Module | Header | Config | Description |
|---|---|---|---|
| COBS | `proto/syn_cobs.h` | `SYN_USE_COBS` | Consistent Overhead Byte Stuffing — packet framing with a streaming byte-at-a-time decoder. Zero-delimited packets over any byte stream. |
| Modbus RTU | `proto/syn_modbus.h` | `SYN_USE_MODBUS` | Modbus RTU slave implementation: function codes FC01–FC04, FC06, FC16. Uses `syn_crc16_modbus()` for frame integrity. |

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
