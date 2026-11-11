#!/usr/bin/env bash
set -euo pipefail

# Validate SyntropicOS Arduino library packaging with official arduino-cli (arduino:avr:uno)
# and optionally flash + verify live serial execution on connected Arduino Uno hardware.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

CACHE_ROOT="${ROOT_DIR}/build/arduino"
BIN_DIR="${CACHE_ROOT}/bin"
export ARDUINO_DIRECTORIES_DATA="${CACHE_ROOT}/data"
export ARDUINO_DIRECTORIES_DOWNLOADS="${CACHE_ROOT}/staging"
export ARDUINO_DIRECTORIES_USER="${CACHE_ROOT}/user"
export PATH="${BIN_DIR}:${PATH}"

mkdir -p "${BIN_DIR}" "${ARDUINO_DIRECTORIES_DATA}" "${ARDUINO_DIRECTORIES_DOWNLOADS}" \
         "${ARDUINO_DIRECTORIES_USER}/libraries"

if [ ! -x "${BIN_DIR}/arduino-cli" ]; then
    echo "=== Installing arduino-cli into build cache ==="
    python3 -c '
import tarfile, urllib.request, io, sys
url = "https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Linux_64bit.tar.gz"
data = urllib.request.urlopen(url, timeout=60).read()
with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz") as tf:
    if sys.version_info >= (3, 12):
        tf.extract("arduino-cli", path="'"${BIN_DIR}"'", filter="data")
    else:
        tf.extract("arduino-cli", path="'"${BIN_DIR}"'")
'
    chmod +x "${BIN_DIR}/arduino-cli"
fi

ln -sfn "${ROOT_DIR}" "${ARDUINO_DIRECTORIES_USER}/libraries/SyntropicOS"

if ! arduino-cli core list | grep -q "arduino:avr"; then
    echo "=== Installing arduino:avr core ==="
    arduino-cli core update-index
    arduino-cli core install arduino:avr
fi

if ! arduino-cli lib list | grep -q "^Ethernet "; then
    echo "=== Installing Arduino Ethernet library for network sketch verification ==="
    arduino-cli lib install Ethernet
fi

FQBN="${FQBN:-arduino:avr:uno}"
PORT="${ARDUINO_PORT:-/dev/ttyACM0}"
if [ -z "${FQBN}" ]; then FQBN="arduino:avr:uno"; fi
if [ -z "${PORT}" ]; then PORT="/dev/ttyACM0"; fi

echo "=== 1. Compiling Arduino Sketches (${FQBN}) ==="
if [ -n "${SKETCH:-}" ]; then
    SKETCHES=("${SKETCH}")
else
    SKETCHES=()
    for ino in examples/*/*.ino; do
        dir="$(dirname "${ino}")"
        base="$(basename "${dir}")"
        if [ "${base}" = "PicoBlink" ] || [ "${base}" = "PicoDualCore" ]; then
            continue
        fi
        SKETCHES+=("${dir}")
    done
fi

FAIL=0
for sk in "${SKETCHES[@]}"; do
    base="$(basename "${sk}")"
    sk_fqbn="${FQBN}"
    if [ "${sk_fqbn}" = "arduino:avr:uno" ]; then
        case "${base}" in
            BiquadFilter|EthernetWebServer|FftSpectrumAnalyzer|TaskMailbox)
                sk_fqbn="arduino:avr:mega"
                ;;
        esac
    fi
    echo "--- Compiling ${sk} (${sk_fqbn}) ---"
    if ! arduino-cli compile --fqbn "${sk_fqbn}" --warnings all "${sk}"; then
        echo "FAILURE: ${sk} failed to compile for ${sk_fqbn}"
        FAIL=1
    fi
done

if [ "${FAIL}" -ne 0 ]; then
    exit 1
fi
echo "=== ALL ${#SKETCHES[@]} AVR ARDUINO SKETCHES COMPILED CLEANLY ==="

if [ "${UPLOAD:-auto}" != "0" ] && [ -c "${PORT}" ]; then
    echo "=== 2. Hardware-in-the-Loop Verification on ${PORT} (${FQBN}) ==="
    if ! python3 -c "import serial" >/dev/null 2>&1; then
        python3 -m pip install --break-system-packages --root-user-action=ignore --quiet pyserial
    fi

    HIL_SKETCH="${ROOT_DIR}/tools/arduino/hil_suite"
    echo "--- Compiling & Flashing ${HIL_SKETCH} to ${PORT} ---"
    arduino-cli compile --fqbn "${FQBN}" --warnings all "${HIL_SKETCH}"
    arduino-cli upload -p "${PORT}" --fqbn "${FQBN}" --verify "${HIL_SKETCH}" 2>/dev/null || \
        arduino-cli upload -p "${PORT}" --fqbn "${FQBN}" "${HIL_SKETCH}"

    echo "--- Running 5-stage AVR Hardware-in-the-Loop Suite on ${PORT} ---"
    python3 - "${PORT}" <<'PY'
import serial, sys, time

port = sys.argv[1]
with serial.Serial(port, 115200, timeout=0.2) as ser:
    # Wait for DTR-triggered Optiboot reset (~1.6s) to complete
    time.sleep(1.8)
    boot_out = b""
    deadline = time.monotonic() + 3.5
    while time.monotonic() < deadline:
        chunk = ser.read(512)
        if chunk:
            boot_out += chunk
            if b"Free SRAM at boot:" in boot_out and b"bytes" in boot_out:
                break

    print(boot_out.decode("utf-8", errors="replace"), end="")
    if b"HIL Test Suite Ready" not in boot_out:
        print("\nFAILURE: Did not observe HIL suite boot banner on Uno", file=sys.stderr)
        sys.exit(1)

    ser.write(b"run_hil\r\n")
    ser.flush()

    hil_out = b""
    t_end = time.monotonic() + 8.0
    while time.monotonic() < t_end:
        chunk = ser.read(512)
        if chunk:
            hil_out += chunk
            print(chunk.decode("utf-8", errors="replace"), end="", flush=True)
            if b"=== ALL 5 ARDUINO UNO HIL SUBSYSTEM TESTS PASSED ===" in hil_out:
                break
            if b"FAILURE:" in hil_out or b": FAIL" in hil_out:
                break

    txt = hil_out.decode("utf-8", errors="replace")
    for required in [
        "[HIL-1/5] SREG_CRITICAL_NESTING: PASS",
        "[HIL-2/5] TIMER1_ISR_AND_MUTEX_PI: PASS",
        "[HIL-3/5] EXTI_INT0_SELF_TRIGGER: PASS",
        "[HIL-4/5] AVR16_CRYPTO_PROTO_KATS: PASS",
        "[HIL-4b]  EEPROM_DUAL_BANK_FAILOVER: PASS",
        "[HIL-5/5] WDT_REBOOT_RECOVERY: PASS (seq=42 crc=0x29B1",
        "=== ALL 5 ARDUINO UNO HIL SUBSYSTEM TESTS PASSED ===",
    ]:
        if required not in txt:
            print(f"\nFAILURE: Missing expected HIL result: {required}", file=sys.stderr)
            sys.exit(1)
PY

    HW_SKETCH="${HW_SKETCH:-examples/PID_TempControl}"
    echo "--- Flashing ${HW_SKETCH} to ${PORT} ---"
    if [ "${SKETCH:-}" != "${HW_SKETCH}" ] && [ -n "${SKETCH:-}" ]; then
        arduino-cli compile --fqbn "${FQBN}" --warnings all "${HW_SKETCH}"
    fi
    arduino-cli upload -p "${PORT}" --fqbn "${FQBN}" --verify "${HW_SKETCH}" 2>/dev/null || \
        arduino-cli upload -p "${PORT}" --fqbn "${FQBN}" "${HW_SKETCH}"

    echo "--- Verifying live Serial CLI & cooperative scheduler on Arduino Uno ---"
    python3 - "${PORT}" <<'PY'
import serial, sys, time

port = sys.argv[1]
with serial.Serial(port, 115200, timeout=0.2) as ser:
    # Opening /dev/ttyACM0 toggles DTR and resets Uno optiboot (~1.6s)
    time.sleep(1.8)
    boot_out = b""
    deadline = time.monotonic() + 3.5
    while time.monotonic() < deadline:
        chunk = ser.read(512)
        if chunk:
            boot_out += chunk
            if b"SP=" in boot_out:
                break

    boot_text = boot_out.decode("utf-8", errors="replace")
    print(boot_text, end="")
    if "SyntropicOS PID Temp Controller" not in boot_text and "SP=" not in boot_text:
        print("\nFAILURE: Did not observe live scheduler/PID boot output from Uno", file=sys.stderr)
        sys.exit(1)

    # Exercise built-in version/uptime commands + custom sp + status commands
    for cmd, expected in [
        (b"version\r\n", "2026.11.0"),
        (b"sp 600\r\n", "60.0"),
        (b"status\r\n", "Heater:"),
        (b"uptime\r\n", "Uptime:"),
    ]:
        ser.write(cmd)
        ser.flush()
        resp = b""
        t_end = time.monotonic() + 2.0
        while time.monotonic() < t_end:
            c = ser.read(512)
            if c:
                resp += c
                if expected.encode() in resp:
                    break
        txt = resp.decode("utf-8", errors="replace")
        print(txt, end="")
        if expected not in txt:
            print(f"\nFAILURE: Expected '{expected}' in response to {cmd!r}", file=sys.stderr)
            sys.exit(1)

print("\n=== ARDUINO UNO HARDWARE-IN-THE-LOOP TEST PASS ===")
PY
fi
