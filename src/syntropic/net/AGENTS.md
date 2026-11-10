# Networking & Secure Transport Invariants (`src/syntropic/net/`)

Applies to Ethernet/IP (`syn_eth`, `syn_icmp`, `syn_igmp`, `syn_autoip`, `syn_dhcp`, `syn_udp`, `syn_tcp`, `syn_netbuf`, `syn_netcfg`, `syn_router`), secure transports (`syn_tls`, `syn_dtls`, `syn_wg`), and application protocols (`syn_dns`, `syn_dnssd`, `syn_sntp`, `syn_ntp_server`, `syn_http`, `syn_httpd`, `syn_websocket`, `syn_mqtt`, `syn_coap`, `syn_heartbeat`).

## Cross-Directory Touchpoints

- **Crypto & Sched**: `src/syntropic/crypto/*` (TLS 1.3, DTLS 1.3, WireGuard), `src/syntropic/pt/syn_pt.h`, and `src/syntropic/sched/`.
- **Transports & Ports**: `syn_transport.h` (`syn_transport_tcp`, `syn_transport_udp`), `src/syntropic/port/syn_port_socket.h`, `src/port/syn_port_eth.h`, and `src/port/posix/port_posix_socket.c`.

## Protocol & Security Invariants

- **Untrusted Wire Bounds Before Pointer Arithmetic**: Validate minimum frame sizes and variable header offsets against actual received length *before* indexing payloads — specifically IPv4 `IHL * 4` options offsets, TCP data offset, UDP length vs. packet length, and HTTP/WebSocket header/body/chunk limits.
- **Bounded Pointer/Label Decompression (`syn_dns`, `syn_dnssd`)**: Enforce strict hop/offset bounds and backward-only/loop guards on RFC 1035 compression pointers so crafted cyclic packets cannot loop infinitely or read out of bounds.
- **Fail-Closed Handshake & Record Guards (`syn_tls`, `syn_dtls`, `syn_wg`)**: Reject `NULL` or zero-length PSK/key material; bind peer authentication to verified handshake state (`hs_valid`) and transcript hashes (rejecting reflection); enforce 64-bit AEAD sequence-number exhaustion limits and anti-replay windows; scrub ephemeral keys on timeout, alert, or disconnect.
- **Inner-Packet Length vs. Transport Padding (`syn_wg`, `syn_eth`)**: Trim outer transport/link padding using validated inner IPv4/IPv6 payload length headers while rejecting inner lengths that exceed decrypted frame length or fall below header minimums.
- **TCP/Socket Lifecycle & Role Validation**: Validate sequence/ACK/RST/FIN numbers against active receive windows; release/reset socket buffers on partial-send errors, RST, or timeout; verify client vs. server message roles (e.g., DHCP client rejecting stray `BOOTREQUEST` frames).
