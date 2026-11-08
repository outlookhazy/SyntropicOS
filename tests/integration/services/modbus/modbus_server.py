#!/usr/bin/env python3
"""
Modbus TCP Daemon for SyntropicOS Integration Tests
"""
import socket
import struct
import sys

def run_modbus_server():
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_sock.bind(("0.0.0.0", 5020))
    server_sock.listen(5)
    print("[Modbus Server] Listening on 0.0.0.0:5020...", flush=True)

    # Pre-populated registers: Reg 0 = 0x1234, Reg 1 = 0x5678
    registers = [0x1234, 0x5678, 0x9ABC, 0xDEF0]
    coils = [True, False, True, False]

    while True:
        client, addr = server_sock.accept()
        print(f"[Modbus Server] Client connected from {addr}", flush=True)
        while True:
            try:
                mbap = client.recv(7)
                if not mbap or len(mbap) < 7:
                    break
                tx_id, proto_id, length, unit_id = struct.unpack(">HHHB", mbap)
                pdu_len = length - 1
                pdu = client.recv(pdu_len)
                if not pdu or len(pdu) < pdu_len:
                    break

                fc = pdu[0]
                if fc == 0x03: # Read Holding Registers
                    start_addr, count = struct.unpack(">HH", pdu[1:5])
                    data = b''
                    for i in range(count):
                        val = registers[(start_addr + i) % len(registers)]
                        data += struct.pack(">H", val)
                    res_pdu = bytes([fc, len(data)]) + data
                elif fc == 0x06: # Write Single Register
                    reg_addr, val = struct.unpack(">HH", pdu[1:5])
                    registers[reg_addr % len(registers)] = val
                    res_pdu = pdu # Echo request
                elif fc == 0x05: # Write Single Coil
                    coil_addr, val = struct.unpack(">HH", pdu[1:5])
                    coils[coil_addr % len(coils)] = (val == 0xFF00)
                    res_pdu = pdu # Echo request
                else:
                    res_pdu = bytes([fc | 0x80, 0x01]) # Exception Illegal Function

                res_mbap = struct.pack(">HHHB", tx_id, proto_id, len(res_pdu) + 1, unit_id)
                client.sendall(res_mbap + res_pdu)
                print(f"[Modbus Server] Handled FC 0x{fc:02X} request successfully", flush=True)
            except Exception as e:
                print(f"[Modbus Server] Error: {e}", flush=True)
                break
        client.close()

if __name__ == '__main__':
    run_modbus_server()
