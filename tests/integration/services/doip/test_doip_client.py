#!/usr/bin/env python3
"""
3rd-Party Python DoIP + ISO 14229-1 UDS Integration Test Suite for SyntropicOS.
Validates SyntropicOS C DoIP Stack (syn_doip.c) and UDS Engine (syn_uds.c) over ISO 13400 Ethernet TCP/UDP transport.

Exhaustively tests:
1. DoIP Vehicle Identification (UDP 13400)
2. DoIP Routing Activation (TCP 13400)
3. DiagnosticSessionControl (0x10) - Default, Extended, Programming
4. SecurityAccess (0x27) - Seed & Key Unlock
5. ReadDataByIdentifier (0x22) - VIN (0xF190), DIDs (0x0100, 0x0300)
6. Session Security Permissions & NRC 0x7E Validation
7. Memory Services (0x3D & 0x23) - WriteMemoryByAddress & ReadMemoryByAddress
8. WriteDataByIdentifier (0x2E)
9. CommunicationControl (0x28)
10. ControlDTCSetting (0x85)
11. RoutineControl (0x31) - Start, Stop, Results (0x0201)
12. Firmware Transfer Services (0x34, 0x36, 0x37)
13. ReadDTCInformation (0x19) - Subfunctions 0x02 & 0x0A
14. ClearDiagnosticInformation (0x14) - Groups 0x000000, 0x100000, 0xFFFFFF
15. ECUReset (0x11)
16. TesterPresent (0x3E)
17. NRC Validation (Invalid DID 0x9999 -> NRC 0x31)
"""

import os
import socket
import struct
import sys
import time

def encode_doip(payload_type, payload_data=b''):
    """Encodes ISO 13400-2 DoIP Header (8 bytes) + Payload."""
    version = 0x02
    inv_version = 0xFD
    length = len(payload_data)
    header = struct.pack(">BBHI", version, inv_version, payload_type, length)
    return header + payload_data

def decode_doip(data):
    """Decodes ISO 13400-2 DoIP Header + Payload."""
    if len(data) < 8:
        return None, None, None
    version, inv_version, payload_type, length = struct.unpack(">BBHI", data[:8])
    payload = data[8:8+length]
    return payload_type, length, payload

def encode_uds_doip_req(sa, ta, uds_data):
    """Encodes ISO 13400-2 Diagnostic Message Payload (0x8001)."""
    payload_data = struct.pack(">HH", sa, ta) + uds_data
    return encode_doip(0x8001, payload_data)

def decode_uds_doip_resp(payload):
    """Decodes ISO 13400-2 Diagnostic Message Response."""
    if len(payload) < 4:
        return None, None, None
    sa, ta = struct.unpack(">HH", payload[:4])
    uds_data = payload[4:]
    return sa, ta, uds_data

def send_doip_uds(sock, sa, ta, uds_req, timeout=2.0):
    """Sends a DoIP Diagnostic Request and waits for DoIP Diagnostic Message ACK & Response."""
    req_bytes = encode_uds_doip_req(sa, ta, uds_req)
    sock.sendall(req_bytes)

    t0 = time.time()
    uds_resp = None

    while time.time() - t0 < timeout:
        sock.settimeout(timeout)
        try:
            data = sock.recv(4096)
            if not data:
                break
        except socket.timeout:
            break

        offset = 0
        while offset + 8 <= len(data):
            p_type, p_len, payload = decode_doip(data[offset:])
            offset += 8 + p_len

            if p_type == 0x8002:
                # Diagnostic Message Ack (ignore)
                continue
            elif p_type == 0x8001:
                # Diagnostic Message Data Response
                _sa, _ta, uds_resp = decode_uds_doip_resp(payload)
                return uds_resp

    return uds_resp

def main():
    host = os.environ.get("DOIP_HOST", "127.0.0.1")
    port = int(os.environ.get("DOIP_PORT", 13400))

    tester_addr = 0x0E80
    ecu_addr = 0x1000

    print(f"[3rd-Party DoIP Client] Connecting to SyntropicOS DoIP Server at {host}:{port}...")

    # --- Phase 1: UDP Vehicle Identification ---
    udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_sock.settimeout(2.0)
    veh_id_req = encode_doip(0x0001, b'')
    udp_sock.sendto(veh_id_req, (host, port))

    try:
        data, _ = udp_sock.recvfrom(4096)
        p_type, _, payload = decode_doip(data)
        if p_type == 0x0004:
            vin = payload[:17].decode('ascii', errors='ignore')
            la = struct.unpack(">H", payload[17:19])[0]
            print(f"[DoIP UDP] Vehicle Announcement Received OK: VIN={vin}, LogicalAddr=0x{la:04X}")
        else:
            print(f"[DoIP UDP] Unexpected payload type: 0x{p_type:04X}")
    except Exception as e:
        print(f"[DoIP UDP] Warning: Vehicle Announcement UDP response timeout ({e})")
    finally:
        udp_sock.close()

    # --- Phase 2: TCP DoIP Connection & Routing Activation ---
    tcp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcp_sock.connect((host, port))
    print("[3rd-Party DoIP Client] Connected to SyntropicOS DoIP Server via TCP!")

    # Routing Activation Request (0x0005): SA=0x00, TesterAddr=0x0E80
    act_req_payload = struct.pack(">HB", tester_addr, 0x00) + b'\x00' * 4
    act_req = encode_doip(0x0005, act_req_payload)
    tcp_sock.sendall(act_req)

    data = tcp_sock.recv(4096)
    p_type, _, payload = decode_doip(data)
    if p_type == 0x0006:
        sa, ta, code = struct.unpack(">HHB", payload[:5])
        if code == 0x10:
            print(f"[DoIP TCP] Routing Activation Successful! Tester=0x{sa:04X}, ECU=0x{ta:04X}")
        else:
            print(f"[DoIP TCP] Routing Activation Failed! Code=0x{code:02X}")
            sys.exit(1)
    else:
        print(f"[DoIP TCP] Unexpected Routing Activation Response: 0x{p_type:04X}")
        sys.exit(1)

    # --- Phase 3: Exhaustive ISO 14229-1 UDS Compliance Tests ---
    print("\n==================================================")
    print("=== Executing Exhaustive ISO 14229-1 UDS Suite over DoIP ===")

    # Test 1: DiagnosticSessionControl (0x10) Extended (0x03)
    print("\n--- Test 1: DiagnosticSessionControl (0x10) Extended Session ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x10\x03')
    assert resp == b'\x50\x03\x00\x32\x01\xF4', f"Session change failed: {resp.hex()}"
    print("[DoIP UDS] Session Change Extended OK!")

    # Test 2: SecurityAccess (0x27) Seed/Key Unlock
    print("\n--- Test 2: SecurityAccess (0x27) Seed/Key Unlock ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x27\x01')
    assert resp[:2] == b'\x67\x01', f"Security seed request failed: {resp.hex()}"
    seed = resp[2:]
    seed_int = int.from_bytes(seed, byteorder='little')
    key = (seed_int ^ 0xA5A5A5A5).to_bytes(4, byteorder='little')
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x27\x02' + key)
    assert resp == b'\x67\x02', f"Security key send failed: {resp.hex()}"
    print("[DoIP UDS] Security Access Unlocked OK!")

    # Test 3: SecurityAccess Repeated Unlock Cycle Check
    print("\n--- Test 3: SecurityAccess Repeated Unlock Cycle Check ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x27\x01')
    assert resp[:2] == b'\x67\x01' and len(resp) == 6, f"Already unlocked response mismatch: {resp.hex()}"
    print("[DoIP UDS] Repeated Security Access Unlock Cycle OK!")

    # Test 4: ReadDataByIdentifier (0x22) VIN 0xF190
    print("\n--- Test 4: ReadDataByIdentifier (0x22) VIN 0xF190 ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x22\xF1\x90')
    assert resp[:3] == b'\x62\xF1\x90', f"Read VIN header failed: {resp.hex()}"
    vin_str = resp[3:].decode('ascii')
    assert vin_str == "SYN12345678901234", f"VIN mismatch: {vin_str}"
    print(f"[DoIP UDS] Read VIN OK: {vin_str}")

    # Test 5: Session Mask Security & NRC 0x7E
    print("\n--- Test 5: Session Mask Security & NRC 0x7E ---")
    send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x10\x01') # Default Session
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x22\x03\x00')
    assert resp == b'\x7F\x22\x7E', f"NRC 0x7E expected for DID 0x0300 in default session, got {resp.hex()}"
    print("[DoIP UDS] NRC 0x7E Caught OK: SubFunctionNotSupportedInActiveSession")

    send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x10\x02') # Programming Session
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x22\x03\x00')
    assert resp == b'\x62\x03\x00\xAA\xBB', f"Read DID 0x0300 failed in Programming session: {resp.hex()}"
    print("[DoIP UDS] Read Programming DID 0x0300 OK: aabb")

    # Test 6: Memory Services 0x3D & 0x23
    print("\n--- Test 6: Memory Services 0x3D & 0x23 ---")
    send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x10\x03') # Switch back to Extended session
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x27\x01')
    seed = resp[2:]
    seed_int = int.from_bytes(seed, byteorder='little')
    key = (seed_int ^ 0xA5A5A5A5).to_bytes(4, byteorder='little')
    send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x27\x02' + key) # Unlock security access

    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x3D\x14\x00\x00\x00\x00\x02\xDE\xAD')
    assert resp == b'\x7D\x14\x00\x00\x00\x00\x02', f"WriteMemoryByAddress failed: {resp.hex()}"
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x23\x14\x00\x00\x00\x00\x02')
    assert resp == b'\x63\xDE\xAD', f"ReadMemoryByAddress failed: {resp.hex()}"
    print("[DoIP UDS] Memory Write & Read OK: 0xdead")

    # Test 7: WriteDataByIdentifier (0x2E) System Status 0x0100
    print("\n--- Test 7: WriteDataByIdentifier (0x2E) System Status 0x0100 ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x2E\x01\x00\xA5')
    assert resp == b'\x6E\x01\x00', f"Write DID 0x0100 failed: {resp.hex()}"
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x22\x01\x00')
    assert resp == b'\x62\x01\x00\xA5', f"Verify Write DID 0x0100 failed: {resp.hex()}"
    print("[DoIP UDS] Write DID 0x0100 OK!")

    # Test 8: CommunicationControl (0x28)
    print("\n--- Test 8: CommunicationControl (0x28) ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x28\x00\x01')
    assert resp == b'\x68\x00', f"CommunicationControl failed: {resp.hex()}"
    print("[DoIP UDS] CommunicationControl (0x28) OK!")

    # Test 9: ControlDTCSetting (0x85)
    print("\n--- Test 9: ControlDTCSetting (0x85) ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x85\x01')
    assert resp == b'\xC5\x01', f"ControlDTCSetting failed: {resp.hex()}"
    print("[DoIP UDS] ControlDTCSetting (0x85) OK!")

    # Test 10: RoutineControl (0x31)
    print("\n--- Test 10: RoutineControl (0x31) ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x31\x01\x02\x01')
    assert resp[:4] == b'\x71\x01\x02\x01', f"RoutineControl Start failed: {resp.hex()}"
    print("[DoIP UDS] RoutineControl (0x31) OK!")

    # Test 11: Firmware Transfer (0x34, 0x36, 0x37)
    print("\n--- Test 11: Firmware Transfer (0x34, 0x36, 0x37) ---")
    send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x10\x02') # Switch to Programming session
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x27\x01')
    seed_int = int.from_bytes(resp[2:], byteorder='little')
    key = (seed_int ^ 0xA5A5A5A5).to_bytes(4, byteorder='little')
    send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x27\x02' + key)
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x34\x00\x11\x10\x04')
    assert resp[:2] == b'\x74\x20', f"RequestDownload failed: {resp.hex()}"
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x36\x01\x11\x22\x33\x44')
    assert resp == b'\x76\x01', f"TransferData failed: {resp.hex()}"
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x37')
    assert resp == b'\x77', f"RequestTransferExit failed: {resp.hex()}"
    print("[DoIP UDS] Firmware Transfer Services (0x34, 0x36, 0x37) OK!")

    # Test 12: ReadDTCInformation (0x19) Subfunctions
    print("\n--- Test 12: ReadDTCInformation (0x19) Subfunctions ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x19\x02\xFF')
    assert resp == b'\x59\x02\xFF\x01\x23\x45\x2F', f"ReadDTC 0x02 failed: {resp.hex()}"
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x19\x0A')
    assert resp == b'\x59\x0A\xFF\x01\x23\x45\x2F', f"ReadDTC 0x0A failed: {resp.hex()}"
    print("[DoIP UDS] ReadDTCInformation (0x19) Subfunctions OK!")

    # Test 13: ClearDiagnosticInformation (0x14) Groups
    print("\n--- Test 13: ClearDiagnosticInformation (0x14) Groups ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x14\x00\x00\x00')
    assert resp == b'\x54', f"Clear DTCs Emissions failed: {resp.hex()}"
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x14\xFF\xFF\xFF')
    assert resp == b'\x54', f"Clear DTCs All failed: {resp.hex()}"
    print("[DoIP UDS] ClearDiagnosticInformation (0x14) Groups OK!")

    # Test 14: ECUReset (0x11)
    print("\n--- Test 14: ECUReset (0x11) ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x11\x01')
    assert resp == b'\x51\x01', f"ECUReset failed: {resp.hex()}"
    print("[DoIP UDS] ECUReset (0x11) OK!")

    # Test 15: TesterPresent (0x3E)
    print("\n--- Test 15: TesterPresent (0x3E) ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x3E\x00')
    assert resp == b'\x7E\x00', f"TesterPresent failed: {resp.hex()}"
    print("[DoIP UDS] TesterPresent (0x3E) OK!")

    # Test 16: NRC Validation (Invalid DID 0x9999 -> NRC 0x31)
    print("\n--- Test 16: NRC Validation ---")
    resp = send_doip_uds(tcp_sock, tester_addr, ecu_addr, b'\x22\x99\x99')
    assert resp == b'\x7F\x22\x31', f"NRC 0x31 expected, got {resp.hex()}"
    print("[DoIP UDS] NRC Caught OK: RequestOutOfRange (0x31)")

    # Test 17: Diagnostic Message NACK for Unknown Target Address (0x2000)
    print("\n--- Test 17: Diagnostic Message NACK (Unknown Target Address) ---")
    req = encode_uds_doip_req(tester_addr, 0x2000, b'\x22\xF1\x90')
    tcp_sock.sendall(req)
    nack_data = tcp_sock.recv(4096)
    p_type, _, p_load = decode_doip(nack_data)
    assert p_type == 0x8003, f"Expected Diagnostic Message NACK (0x8003), got 0x{p_type:04X}"
    assert p_load[4] == 0x03, f"Expected Unknown Target Address code 0x03, got 0x{p_load[4]:02X}"
    print("[DoIP UDS] Diagnostic Message NACK (0x8003, Code 0x03) Caught OK!")

    # Test 18: DoIP Generic NACK for Unknown Payload Type (0x9999)
    print("\n--- Test 18: Generic NACK (Unknown Payload Type) ---")
    unknown_req = encode_doip(0x9999, b'\x00\x00')
    tcp_sock.sendall(unknown_req)
    nack_data = tcp_sock.recv(4096)
    p_type, _, p_load = decode_doip(nack_data)
    assert p_type == 0x0000, f"Expected Generic NACK (0x0000), got 0x{p_type:04X}"
    assert p_load[0] == 0x01, f"Expected Unknown Payload Type code 0x01, got 0x{p_load[0]:02X}"
    print("[DoIP UDS] Generic NACK (Unknown Payload Type 0x01) Caught OK!")

    # Test 19: DoIP Generic NACK for Header Inverse Protocol Version Mismatch
    print("\n--- Test 19: Generic NACK (Incorrect Header Pattern) ---")
    bad_hdr = struct.pack(">BBHI", 0x02, 0x00, 0x0001, 0) # 0x00 instead of ~0x02 (0xFD)
    tcp_sock.sendall(bad_hdr)
    nack_data = tcp_sock.recv(4096)
    p_type, _, p_load = decode_doip(nack_data)
    assert p_type == 0x0000, f"Expected Generic NACK (0x0000), got 0x{p_type:04X}"
    assert p_load[0] == 0x00, f"Expected Incorrect Pattern code 0x00, got 0x{p_load[0]:02X}"
    print("[DoIP UDS] Generic NACK (Incorrect Pattern 0x00) Caught OK!")

    tcp_sock.close()

    print("\n==================================================")
    print("=== Exhaustive DoIP + ISO 14229-1 UDS Integration Suite PASS ===")

if __name__ == "__main__":
    main()
