#!/usr/bin/env python3
import sys
import time

try:
    import serial
except ImportError:
    print("Error: 'pyserial' package is not installed. Please run: pip3 install pyserial")
    sys.exit(1)

PORT = "/dev/ttyACM0"
BAUD = 115200
TIMEOUT = 2.0

def send_command(ser, cmd):
    print(f"\nSending: {repr(cmd)}")
    ser.write((cmd + "\r").encode('utf-8'))
    time.sleep(0.5)
    
    # Read the response
    response = ""
    start_time = time.time()
    while (time.time() - start_time) < TIMEOUT:
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            response += data
            # Stop reading if we see the prompt again after sending cmd
            if "> " in response and len(response) > len(cmd) + 3:
                # Sleep a little to catch any trailing output
                time.sleep(0.1)
                if ser.in_waiting > 0:
                    response += ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                break
        time.sleep(0.05)
    
    print("Response:")
    print(response.strip())
    return response

def main():
    print(f"Connecting to Arduino Uno on {PORT} at {BAUD} baud...")
    try:
        ser = serial.Serial(PORT, BAUD, timeout=TIMEOUT)
    except Exception as e:
        print(f"Failed to open serial port {PORT}: {e}")
        sys.exit(1)

    # Give Arduino time to reset
    print("Waiting 3 seconds for Arduino to reset...")
    time.sleep(3.0)
    ser.reset_input_buffer()

    # Read startup logs / initial prompt
    startup = ""
    start_time = time.time()
    while (time.time() - start_time) < 3.0:
        if ser.in_waiting > 0:
            startup += ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        time.sleep(0.1)
    
    print("\nStartup output:")
    print(startup.strip())

    # Send help
    send_command(ser, "help")

    # Send version
    send_command(ser, "version")

    # Send uptime
    send_command(ser, "uptime")

    # Send tasks
    send_command(ser, "tasks")


    # 3. Test Signal Statistics & Physical ADCs
    print("\n--- Testing Physical ADC Readings & Signal Stats ---")
    send_command(ser, "adc")
    send_command(ser, "adc 0")
    send_command(ser, "adc 1")

    # 4. Test FSM Transitions
    print("\n--- Testing FSM (Finite State Machine) ---")
    send_command(ser, "fsm start")
    send_command(ser, "fsm stop")

    # 5. Test LED Patterns
    print("\n--- Testing LED Patterns ---")
    send_command(ser, "led blink")
    time.sleep(1.0)
    send_command(ser, "led flash")
    time.sleep(1.0)
    send_command(ser, "led sos")
    time.sleep(1.0)
    send_command(ser, "led off")

    # Read trailing data (check if we see a Software Timer tick log)
    print("\nWaiting 6 seconds to capture any periodic software timer logs...")
    time.sleep(6.0)
    if ser.in_waiting > 0:
        trailing = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        print("Captured trailing output:")
        print(trailing.strip())

    ser.close()
    print("\nVerification finished successfully!")

if __name__ == '__main__':
    main()
