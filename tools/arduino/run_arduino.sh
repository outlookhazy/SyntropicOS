#!/usr/bin/env bash
set -euo pipefail

# Validate SyntropicOS Arduino library packaging with official arduino-cli
# and optionally flash + verify live serial execution on connected AVR / ESP32 hardware.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

CACHE_ROOT="${ROOT_DIR}/build/arduino"
BIN_DIR="${CACHE_ROOT}/bin"
export ARDUINO_DIRECTORIES_DATA="${CACHE_ROOT}/data"
export ARDUINO_DIRECTORIES_DOWNLOADS="${CACHE_ROOT}/staging"
export ARDUINO_DIRECTORIES_USER="${CACHE_ROOT}/user"
export XDG_CACHE_HOME="${CACHE_ROOT}/cache"
export PATH="${BIN_DIR}:${PATH}"

ESP32_INDEX_URL="https://espressif.github.io/arduino-esp32/package_esp32_index.json"

ensure_core_installed() {
    local core="$1"
    if arduino-cli core list | grep -q "^${core} "; then
        return 0
    fi
    echo "=== Installing ${core} core ==="
    if [ "${core}" = "esp32:esp32" ]; then
        arduino-cli --additional-urls "${ESP32_INDEX_URL}" core update-index
        arduino-cli --additional-urls "${ESP32_INDEX_URL}" core install "${core}"
    else
        arduino-cli core update-index
        arduino-cli core install "${core}"
    fi
}

mkdir -p "${BIN_DIR}" "${ARDUINO_DIRECTORIES_DATA}" "${ARDUINO_DIRECTORIES_DOWNLOADS}" \
         "${ARDUINO_DIRECTORIES_USER}/libraries" "${XDG_CACHE_HOME}"

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

FQBN_EXPLICIT="${FQBN:-}"
FQBN="${FQBN:-arduino:avr:uno}"
PORT="${ARDUINO_PORT:-/dev/ttyACM0}"
if [ -z "${FQBN}" ]; then FQBN="arduino:avr:uno"; fi
if [ -z "${PORT}" ]; then PORT="/dev/ttyACM0"; fi

PRIMARY_CORE="$(echo "${FQBN}" | cut -d: -f1-2)"
ensure_core_installed "${PRIMARY_CORE}"

if ! arduino-cli lib list | grep -q "^Ethernet "; then
    echo "=== Installing Arduino Ethernet library for network sketch verification ==="
    arduino-cli lib install Ethernet
fi

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
echo "=== ALL ${#SKETCHES[@]} ARDUINO SKETCHES COMPILED CLEANLY (${FQBN}) ==="

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
            303a:*)
                ensure_core_installed "esp32:esp32"
                ESPTOOL_BIN="$(find "${ARDUINO_DIRECTORIES_DATA}/packages/esp32/tools/esptool_py" -maxdepth 2 -type f -name "esptool" 2>/dev/null | head -n 1 || true)"
                CHIP_INFO=""
                if [ -n "${ESPTOOL_BIN}" ] && [ -x "${ESPTOOL_BIN}" ]; then
                    CHIP_INFO="$(timeout 8s "${ESPTOOL_BIN}" --port "${PORT}" chip-id 2>&1 || true)"
                fi
                if echo "${CHIP_INFO}" | grep -qi "ESP32-S3"; then
                    HIL_FQBN="esp32:esp32:esp32s3:CDCOnBoot=cdc"
                elif echo "${CHIP_INFO}" | grep -qi "ESP32-C3"; then
                    HIL_FQBN="esp32:esp32:esp32c3:CDCOnBoot=cdc"
                elif echo "${CHIP_INFO}" | grep -qi "ESP32-H2"; then
                    HIL_FQBN="esp32:esp32:esp32h2:CDCOnBoot=cdc"
                else
                    HIL_FQBN="esp32:esp32:esp32c6:CDCOnBoot=cdc"
                fi
                ;;
        esac
    fi

    HIL_CORE="$(echo "${HIL_FQBN}" | cut -d: -f1-2)"
    ensure_core_installed "${HIL_CORE}"

    echo "=== 2. Hardware-in-the-Loop Verification on ${PORT} (${HIL_FQBN}, usb=${ARDUINO_USB_ID:-auto}) ==="
    if ! python3 -c "import serial" >/dev/null 2>&1; then
        python3 -m pip install --break-system-packages --root-user-action=ignore --quiet pyserial
    fi

    upload_with_timeout() {
        local sketch="$1"
        if [[ "${HIL_FQBN}" != esp32:* ]]; then
            python3 -c '
import serial, time
with serial.Serial("'"${PORT}"'", 115200, timeout=0.1) as s:
    s.dtr = False; time.sleep(0.05); s.dtr = True; s.reset_input_buffer()
' 2>/dev/null || true
        fi
        if ! timeout -k 5 90s arduino-cli upload -p "${PORT}" --fqbn "${HIL_FQBN}" "${sketch}"; then
            sleep 2
            timeout -k 5 90s arduino-cli upload -p "${PORT}" --fqbn "${HIL_FQBN}" "${sketch}"
        fi
    }

    HIL_SKETCH="${ROOT_DIR}/tools/arduino/hil_suite"
    echo "--- Compiling & Flashing ${HIL_SKETCH} to ${PORT} (${HIL_FQBN}) ---"
    arduino-cli compile --fqbn "${HIL_FQBN}" --warnings all "${HIL_SKETCH}"
    upload_with_timeout "${HIL_SKETCH}"

    echo "--- Running Hardware-in-the-Loop Suite on ${PORT} (${HIL_FQBN}) ---"
    python3 - "${PORT}" "${HIL_FQBN}" <<'PY'
import os, serial, sys, time

port = sys.argv[1]
fqbn = sys.argv[2]
is_esp32 = fqbn.startswith("esp32:")

ser = serial.Serial(port, 115200, timeout=0.2)
try:
    time.sleep(0.4 if is_esp32 else 1.8)
    boot_out = b""
    deadline = time.monotonic() + (1.0 if is_esp32 else 2.0)
    while time.monotonic() < deadline:
        chunk = ser.read(512)
        if chunk:
            boot_out += chunk
            if b"Free SRAM at boot:" in boot_out and b"bytes" in boot_out:
                break

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
    wdt_armed_at = None
    wdt_queried = False
    t_end = time.monotonic() + 30.0
    while time.monotonic() < t_end:
        try:
            chunk = ser.read(512)
        except (serial.SerialException, OSError):
            try:
                ser.close()
            except Exception:
                pass
            reopen_end = time.monotonic() + 3.0
            while time.monotonic() < reopen_end:
                if os.path.exists(port):
                    try:
                        ser = serial.Serial(port, 115200, timeout=0.2)
                        break
                    except (serial.SerialException, OSError):
                        pass
                time.sleep(0.05)
            continue

        if chunk:
            hil_out += chunk
            print(chunk.decode("utf-8", errors="replace"), end="", flush=True)
            if wdt_armed_at is None and b"[HIL-5/5] Arming Hardware Watchdog" in hil_out:
                wdt_armed_at = time.monotonic()
            if b"=== ALL 5 AVR HIL SUBSYSTEM TESTS PASSED ===" in hil_out:
                break
            if b"FAILURE:" in hil_out or b": FAIL" in hil_out:
                break

        if (
            is_esp32
            and wdt_armed_at is not None
            and not wdt_queried
            and (time.monotonic() - wdt_armed_at) > 1.2
            and b"WDT_REBOOT_RECOVERY: PASS" not in hil_out
        ):
            try:
                ser.write(b"wdt_check\r\n")
                ser.flush()
                wdt_queried = True
            except (serial.SerialException, OSError):
                pass

    exp_slices = "0" if fqbn == "arduino:avr:uno" else "510"
    txt = hil_out.decode("utf-8", errors="replace")
    required_checks = [
        "[HIL-1/5] SREG_CRITICAL_NESTING: PASS",
        "[HIL-2/5] TIMER1_ISR_AND_MUTEX_PI: PASS",
        "[HIL-3/5] EXTI_INT0_SELF_TRIGGER: PASS",
        f"[HIL-4/5] AVR16_CRYPTO_PROTO_KATS: PASS (crc16=0x29B1 x25519_slices={exp_slices}",
        "[HIL-4b]  EEPROM_DUAL_BANK_FAILOVER: PASS",
    ]
    if is_esp32:
        required_checks.append("[HIL-4c]  RISCV_COOP_PROTOCOL_BUDGET: PASS (")
    required_checks.extend([
        "[HIL-5/5] WDT_REBOOT_RECOVERY: PASS (seq=42 crc=0x29B1",
        "=== ALL 5 AVR HIL SUBSYSTEM TESTS PASSED ===",
    ])
    for required in required_checks:
        if required not in txt:
            print(f"\nFAILURE: Missing expected HIL result: {required}", file=sys.stderr)
            sys.exit(1)
finally:
    try:
        ser.close()
    except Exception:
        pass
PY

    HW_SKETCH="${HW_SKETCH:-examples/PID_TempControl}"
    echo "--- Flashing ${HW_SKETCH} to ${PORT} (${HIL_FQBN}) ---"
    if [ "${HIL_FQBN}" != "${FQBN}" ] || { [ "${SKETCH:-}" != "${HW_SKETCH}" ] && [ -n "${SKETCH:-}" ]; }; then
        arduino-cli compile --fqbn "${HIL_FQBN}" --warnings all "${HW_SKETCH}"
    fi
    upload_with_timeout "${HW_SKETCH}"

    echo "--- Verifying live Serial CLI & cooperative scheduler on ${PORT} (${HIL_FQBN}) ---"
    python3 - "${PORT}" "${HIL_FQBN}" <<'PY'
import re, serial, sys, time

port = sys.argv[1]
fqbn = sys.argv[2]
is_esp32 = fqbn.startswith("esp32:")

with serial.Serial(port, 115200, timeout=0.2) as ser:
    time.sleep(0.4 if is_esp32 else 1.8)
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
        print("\nFAILURE: Did not observe live scheduler/PID boot output from board", file=sys.stderr)
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

print("\n=== ARDUINO HARDWARE-IN-THE-LOOP TEST PASS ===")
PY
fi
