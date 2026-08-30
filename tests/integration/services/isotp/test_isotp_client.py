#!/usr/bin/env python3
"""
3rd-Party Python isotp Integration Client Test Suite for SyntropicOS ISO-TP Stack.
Validates SyntropicOS C ISO-TP Stack (syn_isotp.c) using official, independent isotp library.
Tests Single Frame (SF), Multi-Frame (FF+CF), Large Multi-Frame (512B), and Flow Control (FC) parameters.
"""

import os
import select
import socket
import struct
import sys
import time
import can
import isotp

class TcpCanBus(can.bus.BusABC):
    """Emulates a python-can Bus interface over TCP socket connection."""
    def __init__(self, sock, channel='vcan0', **kwargs):
        super().__init__(channel=channel, **kwargs)
        self.sock = sock
        self.sock.setblocking(False)
        self.rx_buf = bytearray()

    def _recv_internal(self, timeout):
        t_end = time.time() + (timeout if timeout is not None and timeout > 0 else 0.001)
        while len(self.rx_buf) < 13:
            rem = t_end - time.time()
            if rem <= 0:
                break
            r, _, _ = select.select([self.sock], [], [], min(rem, 0.05))
            if r:
                try:
                    chunk = self.sock.recv(1024)
                    if chunk:
                        self.rx_buf.extend(chunk)
                    else:
                        break
                except Exception:
                    break

        if len(self.rx_buf) >= 13:
            raw_frame = self.rx_buf[:13]
            self.rx_buf = self.rx_buf[13:]
            can_id, dlc = struct.unpack(">IB", raw_frame[:5])
            msg = can.Message(arbitration_id=can_id, dlc=dlc, data=raw_frame[5:5+dlc], is_extended_id=False, is_rx=True, timestamp=time.time())
            return msg, False
        return None, False

    def send(self, msg, timeout=None):
        can_id = msg.arbitration_id
        dlc = msg.dlc
        data_bytes = bytes(msg.data)
        if len(data_bytes) < 8:
            data_bytes = data_bytes + b'\x00' * (8 - len(data_bytes))

        frame_data = struct.pack(">IB", can_id, dlc) + data_bytes[:8]
        self.sock.sendall(frame_data)

    def _close_internal(self):
        try:
            self.sock.close()
        except Exception:
            pass

def main():
    host = os.environ.get("ISOTP_HOST", "isotp")
    port = 10887

    print(f"[3rd-Party isotp Client] Connecting to SyntropicOS ISO-TP C Server at {host}:{port}...")

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connected = False
    for attempt in range(10):
        try:
            sock.connect((host, port))
            connected = True
            break
        except Exception as e:
            print(f"[3rd-Party isotp Client] Waiting for ISO-TP C Server... ({e})")
            time.sleep(1)

    if not connected:
        print("[3rd-Party isotp Client] ERROR: Could not connect to SyntropicOS ISO-TP C Server!")
        sys.exit(1)

    print("[3rd-Party isotp Client] Connected to SyntropicOS ISO-TP C Server!")

    bus = TcpCanBus(sock)
    # Python client transmits on 0x7E0, receives on 0x7E8
    addr = isotp.Address(isotp.AddressingMode.Normal_11bits, txid=0x7E0, rxid=0x7E8)
    params = {
        'blocksize': 0,
        'stmin': 0,
        'tx_data_length': 8,
        'tx_padding': 0xCC,
        'rx_flowcontrol_timeout': 2000,
        'rx_consecutive_frame_timeout': 2000
    }

    def on_error(error):
        print(f"[isotp stack error] {error}")

    stack = isotp.CanStack(bus=bus, address=addr, params=params, error_handler=on_error)

    failures = 0

    # 1. Single Frame (SF) <= 7 bytes
    print("\n--- Test 1: Single Frame (SF) Transmission (5 bytes) ---")
    payload1 = b"HELLO"
    stack.send(payload1)

    t0 = time.time()
    rx_payload = None
    while time.time() - t0 < 3.0:
        stack.process()
        if stack.available():
            rx_payload = stack.recv()
            break
        time.sleep(0.001)

    if rx_payload == payload1:
        print(f"[isotp] SF Echo Received OK: {rx_payload.decode('ascii')}")
    else:
        print(f"[isotp] FAIL: SF Echo mismatch! Expected {payload1}, got {rx_payload}")
        failures += 1

    # 2. Multi-Frame (FF + CF) 64 bytes
    print("\n--- Test 2: Multi-Frame (FF + CF) Transmission (64 bytes) ---")
    payload2 = b"A" * 64
    stack.send(payload2)

    t0 = time.time()
    rx_payload = None
    while time.time() - t0 < 3.0:
        stack.process()
        if stack.available():
            rx_payload = stack.recv()
            break
        time.sleep(0.001)

    if rx_payload == payload2:
        print(f"[isotp] Multi-Frame (64B) Echo Received OK! Length={len(rx_payload)}")
    else:
        print(f"[isotp] FAIL: Multi-Frame (64B) Echo mismatch! Length={len(rx_payload) if rx_payload else 0}")
        failures += 1

    # 3. Large Multi-Frame (512 bytes)
    print("\n--- Test 3: Large Multi-Frame Transmission (512 bytes) ---")
    payload3 = b"B" * 512
    stack.send(payload3)

    t0 = time.time()
    rx_payload = None
    while time.time() - t0 < 5.0:
        stack.process()
        if stack.available():
            rx_payload = stack.recv()
            break
        time.sleep(0.005)

    if rx_payload == payload3:
        print(f"[isotp] Large Multi-Frame (512B) Echo Received OK! Length={len(rx_payload)}")
    else:
        print(f"[isotp] FAIL: Large Multi-Frame (512B) Echo mismatch! Length={len(rx_payload) if rx_payload else 0}")
        failures += 1

    # 4. Exact Max Single Frame (SF) Boundary (7 bytes)
    print("\n--- Test 4: Max Single Frame (SF) Boundary Transmission (7 bytes) ---")
    payload4 = b"1234567"
    stack.send(payload4)

    t0 = time.time()
    rx_payload = None
    while time.time() - t0 < 3.0:
        stack.process()
        if stack.available():
            rx_payload = stack.recv()
            break
        time.sleep(0.001)

    if rx_payload == payload4:
        print(f"[isotp] SF Boundary (7B) Echo Received OK: {rx_payload.decode('ascii')}")
    else:
        print(f"[isotp] FAIL: SF Boundary (7B) Echo mismatch! Expected {payload4}, got {rx_payload}")
        failures += 1

    # 5. Exact Min Multi-Frame (FF + CF) Boundary (8 bytes)
    print("\n--- Test 5: Min Multi-Frame Boundary Transmission (8 bytes) ---")
    payload5 = b"12345678"
    stack.send(payload5)

    t0 = time.time()
    rx_payload = None
    while time.time() - t0 < 3.0:
        stack.process()
        if stack.available():
            rx_payload = stack.recv()
            break
        time.sleep(0.001)

    if rx_payload == payload5:
        print(f"[isotp] Min Multi-Frame (8B) Echo Received OK: {rx_payload.decode('ascii')}")
    else:
        print(f"[isotp] FAIL: Min Multi-Frame (8B) Echo mismatch! Expected {payload5}, got {rx_payload}")
        failures += 1

    # 6. Intermediate Multi-Frame (128 bytes)
    print("\n--- Test 6: Intermediate Multi-Frame Transmission (128 bytes) ---")
    payload6 = bytes([(i % 256) for i in range(128)])
    stack.send(payload6)

    t0 = time.time()
    rx_payload = None
    while time.time() - t0 < 4.0:
        stack.process()
        if stack.available():
            rx_payload = stack.recv()
            break
        time.sleep(0.002)

    if rx_payload == payload6:
        print(f"[isotp] Intermediate Multi-Frame (128B) Echo Received OK! Length={len(rx_payload)}")
    else:
        print(f"[isotp] FAIL: Intermediate Multi-Frame (128B) Echo mismatch! Length={len(rx_payload) if rx_payload else 0}")
        failures += 1

    print("\n==================================================")
    if failures == 0:
        print("=== 3rd-Party isotp Python Client Integration PASS ===")
        sys.exit(0)
    else:
        print(f"=== 3rd-Party isotp Python Client Integration FAILED ({failures} failures) ===")
        sys.exit(1)

if __name__ == '__main__':
    main()
