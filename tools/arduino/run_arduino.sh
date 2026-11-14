#!/usr/bin/env bash
set -euo pipefail

# Validate SyntropicOS Arduino library packaging with official arduino-cli
# and optionally flash + verify live serial execution on connected AVR hardware.

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

FQBN_EXPLICIT="${FQBN:-}"
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
    SKETCHES+=("tools/arduino/hil_suite")
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
    if [ "${base}" = "hil_suite" ] && [ "${sk_fqbn}" = "arduino:avr:uno" ]; then
        echo "--- Compiling ${sk} (arduino:avr:mega) ---"
        if ! arduino-cli compile --fqbn "arduino:avr:mega" --warnings all "${sk}"; then
            echo "FAILURE: ${sk} failed to compile for arduino:avr:mega"
            FAIL=1
        fi
    fi
done

if [ "${FAIL}" -ne 0 ]; then
    exit 1
fi
echo "=== ALL ${#SKETCHES[@]} AVR ARDUINO SKETCHES COMPILED CLEANLY ==="

if [ "${UPLOAD:-auto}" != "0" ] && [ -c "${PORT}" ]; then
    HIL_FQBN="${FQBN}"
    if [ -z "${FQBN_EXPLICIT}" ]; then
        USB_ID="${ARDUINO_USB_ID:-}"
        if [ -z "${USB_ID}" ]; then
            PORT_BASE="$(basename "${PORT}")"
            VID="$(cat "/sys/class/tty/${PORT_BASE}/device/../idVendor" 2>/dev/null || true)"
            PID="$(cat "/sys/class/tty/${PORT_BASE}/device/../idProduct" 2>/dev/null || true)"
            if [ -n "${VID}" ] && [ -n "${PID}" ]; then
                USB_ID="${VID}:${PID}"
            fi
        fi
        case "${USB_ID,,}" in
            2341:0058)
                HIL_FQBN="arduino:megaavr:nona4809:mode=off"
                ;;
            2341:0042|2341:0010|2a03:0042)
                HIL_FQBN="arduino:avr:mega"
                ;;
        esac
    fi

    HIL_CORE="$(echo "${HIL_FQBN}" | cut -d: -f1-2)"
    if ! arduino-cli core list | grep -q "^${HIL_CORE} "; then
        echo "=== Installing ${HIL_CORE} core for connected HIL board ==="
        arduino-cli core update-index
        arduino-cli core install "${HIL_CORE}"
    fi

    echo "=== 2. Hardware-in-the-Loop Verification on ${PORT} (${HIL_FQBN}, usb=${ARDUINO_USB_ID:-auto}) ==="
    if ! python3 -c "import serial" >/dev/null 2>&1; then
        python3 -m pip install --break-system-packages --root-user-action=ignore --quiet pyserial
    fi

    upload_with_timeout() {
        local sketch="$1"
        python3 -c '
import serial, time
with serial.Serial("'"${PORT}"'", 115200, timeout=0.1) as s:
    s.dtr = False; time.sleep(0.05); s.dtr = True; s.reset_input_buffer()
' 2>/dev/null || true
        if ! timeout -k 5 90s arduino-cli upload -p "${PORT}" --fqbn "${HIL_FQBN}" "${sketch}"; then
            sleep 2
            timeout -k 5 90s arduino-cli upload -p "${PORT}" --fqbn "${HIL_FQBN}" "${sketch}"
        fi
    }

    HIL_SKETCH="${ROOT_DIR}/tools/arduino/hil_suite"
    echo "--- Compiling & Flashing ${HIL_SKETCH} to ${PORT} (${HIL_FQBN}) ---"
    arduino-cli compile --fqbn "${HIL_FQBN}" --warnings all "${HIL_SKETCH}"
    upload_with_timeout "${HIL_SKETCH}"

    echo "--- Running 5-stage AVR Hardware-in-the-Loop Suite on ${PORT} (${HIL_FQBN}) ---"
    python3 - "${PORT}" "${HIL_FQBN}" <<'PY'
import serial, sys, time

port = sys.argv[1]
fqbn = sys.argv[2]
with serial.Serial(port, 115200, timeout=0.2) as ser:
    # Wait for DTR-triggered Optiboot reset (~1.6s) if applicable
    time.sleep(1.8)
    boot_out = b""
    deadline = time.monotonic() + 2.0
    while time.monotonic() < deadline:
        chunk = ser.read(512)
        if chunk:
            boot_out += chunk
            if b"Free SRAM at boot:" in boot_out and b"bytes" in boot_out:
                break

    # Non-DTR-reset bridges (e.g. Nano Every SAMD11 MuxTO) boot right after UPDI upload
    # before pyserial opens the port; request banner explicitly if missed.
    if b"HIL Test Suite Ready" not in boot_out:
        ser.write(b"banner\r\n")
        ser.flush()
        deadline = time.monotonic() + 2.0
        while time.monotonic() < deadline:
            chunk = ser.read(512)
            if chunk:
                boot_out += chunk
                if b"Free SRAM at boot:" in boot_out and b"bytes" in boot_out:
                    break

    print(boot_out.decode("utf-8", errors="replace"), end="")
    if b"HIL Test Suite Ready" not in boot_out:
        print("\nFAILURE: Did not observe HIL suite boot banner", file=sys.stderr)
        sys.exit(1)

    ser.write(b"run_hil\r\n")
    ser.flush()

    hil_out = b""
    t_end = time.monotonic() + 25.0
    while time.monotonic() < t_end:
        chunk = ser.read(512)
        if chunk:
            hil_out += chunk
            print(chunk.decode("utf-8", errors="replace"), end="", flush=True)
            if b"=== ALL 5 AVR HIL SUBSYSTEM TESTS PASSED ===" in hil_out:
                break
            if b"FAILURE:" in hil_out or b": FAIL" in hil_out:
                break

    exp_slices = "0" if fqbn == "arduino:avr:uno" else "510"
    txt = hil_out.decode("utf-8", errors="replace")
    for required in [
        "[HIL-1/5] SREG_CRITICAL_NESTING: PASS",
        "[HIL-2/5] TIMER1_ISR_AND_MUTEX_PI: PASS",
        "[HIL-3/5] EXTI_INT0_SELF_TRIGGER: PASS",
        f"[HIL-4/5] AVR16_CRYPTO_PROTO_KATS: PASS (crc16=0x29B1 x25519_slices={exp_slices}",
        "[HIL-4b]  EEPROM_DUAL_BANK_FAILOVER: PASS",
        "[HIL-5/5] WDT_REBOOT_RECOVERY: PASS (seq=42 crc=0x29B1",
        "=== ALL 5 AVR HIL SUBSYSTEM TESTS PASSED ===",
    ]:
        if required not in txt:
            print(f"\nFAILURE: Missing expected HIL result: {required}", file=sys.stderr)
            sys.exit(1)
PY

    HW_SKETCH="${HW_SKETCH:-examples/PID_TempControl}"
    echo "--- Flashing ${HW_SKETCH} to ${PORT} (${HIL_FQBN}) ---"
    if [ "${HIL_FQBN}" != "${FQBN}" ] || { [ "${SKETCH:-}" != "${HW_SKETCH}" ] && [ -n "${SKETCH:-}" ]; }; then
        arduino-cli compile --fqbn "${HIL_FQBN}" --warnings all "${HW_SKETCH}"
    fi
    upload_with_timeout "${HW_SKETCH}"

    echo "--- Verifying live Serial CLI & cooperative scheduler on ${PORT} (${HIL_FQBN}) ---"
    python3 - "${PORT}" <<'PY'
import re, serial, sys, time

port = sys.argv[1]
with serial.Serial(port, 115200, timeout=0.2) as ser:
    # Opening serial port toggles DTR and resets Optiboot (~1.6s) if applicable
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
    print(boot_text, end="", flush=True)
    if "SyntropicOS PID Temp Controller" not in boot_text and "SP=" not in boot_text:
        print("\nFAILURE: Did not observe live scheduler/PID boot output from AVR board", file=sys.stderr)
        sys.exit(1)

    all_output = boot_text

    def send_cmd(cmd_bytes, stop_condition=None, timeout=2.5):
        ser.write(cmd_bytes)
        ser.flush()
        accum = b""
        end_time = time.monotonic() + timeout
        while time.monotonic() < end_time:
            c = ser.read(512)
            if c:
                accum += c
                txt = accum.decode("utf-8", errors="replace")
                if stop_condition and stop_condition(txt):
                    time.sleep(0.05)
                    extra = ser.read(512)
                    if extra:
                        accum += extra
                    break
        text = accum.decode("utf-8", errors="replace")
        print(text, end="", flush=True)
        return text

    # 1. Exercise built-in version command (must match SyntropicOS v\d+\.\d+\.\d+)
    resp_version = send_cmd(b"version\r\n", stop_condition=lambda t: bool(re.search(r"SyntropicOS v\d+\.\d+\.\d+", t)))
    all_output += resp_version
    if not re.search(r"SyntropicOS v\d+\.\d+\.\d+", resp_version):
        print("\nFAILURE: 'version' did not match pattern 'SyntropicOS v\\d+\\.\\d+\\.\\d+'", file=sys.stderr)
        sys.exit(1)

    # 2. Exercise sp command (after 'sp 600' require exact line 'Setpoint updated to 60.0 degC')
    resp_sp = send_cmd(b"sp 600\r\n", stop_condition=lambda t: "Setpoint updated" in t)
    all_output += resp_sp
    if not any(line.strip() == "Setpoint updated to 60.0 degC" for line in resp_sp.splitlines()):
        print("\nFAILURE: 'sp 600' did not produce exact line 'Setpoint updated to 60.0 degC'", file=sys.stderr)
        sys.exit(1)

    # 3. Exercise status command (require 'Setpoint:  60.0 degC' and Heater value in [0, 100])
    resp_status = send_cmd(b"status\r\n", stop_condition=lambda t: "Heater:" in t)
    all_output += resp_status
    if "Setpoint:  60.0 degC" not in resp_status:
        print("\nFAILURE: 'status' missing required 'Setpoint:  60.0 degC'", file=sys.stderr)
        sys.exit(1)
    m_heater = re.search(r"Heater:\s*([+-]?\d+(?:\.\d+)?)%", resp_status)
    if not m_heater:
        print("\nFAILURE: 'status' missing 'Heater:' percentage value", file=sys.stderr)
        sys.exit(1)
    heater_val = float(m_heater.group(1))
    if not (0.0 <= heater_val <= 100.0):
        print(f"\nFAILURE: Heater value {heater_val}% outside valid range [0, 100]", file=sys.stderr)
        sys.exit(1)

    # 4. Exercise built-in uptime command
    resp_uptime = send_cmd(b"uptime\r\n", stop_condition=lambda t: "Uptime:" in t)
    all_output += resp_uptime
    if "Uptime:" not in resp_uptime:
        print("\nFAILURE: 'uptime' response missing 'Uptime:'", file=sys.stderr)
        sys.exit(1)

    # 5. Parse at least 3 periodic 'SP=x C | PV=y C | OUT=z%' lines with regex
    periodic_regex = re.compile(
        r"SP=([+-]?\d+(?:\.\d+)?)\s*C\s*\|\s*PV=([+-]?\d+(?:\.\d+)?)\s*C\s*\|\s*OUT=([+-]?\d+(?:\.\d+)?)\s*%"
    )
    extra_deadline = time.monotonic() + 1.5
    while len(periodic_regex.findall(all_output)) < 3 and time.monotonic() < extra_deadline:
        c = ser.read(512)
        if c:
            chunk_txt = c.decode("utf-8", errors="replace")
            print(chunk_txt, end="", flush=True)
            all_output += chunk_txt

    matches = periodic_regex.findall(all_output)
    if len(matches) < 3:
        print(f"\nFAILURE: Expected at least 3 periodic status lines, found {len(matches)}", file=sys.stderr)
        sys.exit(1)

    for sp_str, pv_str, out_str in matches:
        sp_f = float(sp_str)
        pv_f = float(pv_str)
        out_f = float(out_str)
        if not (0.0 <= sp_f <= 150.0):
            print(f"\nFAILURE: Periodic SP={sp_f} out of sane range [0, 150]", file=sys.stderr)
            sys.exit(1)
        if not (-50.0 <= pv_f <= 500.0):
            print(f"\nFAILURE: Periodic PV={pv_f} out of sane range [-50, 500]", file=sys.stderr)
            sys.exit(1)
        if not (0.0 <= out_f <= 100.0):
            print(f"\nFAILURE: Periodic OUT={out_f} out of sane range [0, 100]", file=sys.stderr)
            sys.exit(1)

print("\n=== AVR HARDWARE-IN-THE-LOOP TEST PASS ===")
PY
fi
