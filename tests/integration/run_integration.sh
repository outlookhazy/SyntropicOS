#!/usr/bin/env bash
set -euo pipefail

if [ -d "/workspace/src/syntropic" ]; then
    ROOT_DIR="/workspace"
else
    ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
fi

cd "${ROOT_DIR}"

echo "=== SyntropicOS 3rd-Party Integration Test Suite ==="

CFLAGS="-std=c99 -D_DEFAULT_SOURCE -DSYN_POSIX_SOCKET_OVERRIDE=1 -pedantic -Wall -Wextra -I. -Isrc -Itests/unit -Itests/unit/mocks -DSYN_LOG_COLOR=1 -DSYN_USE_COREDUMP=1 -DSYN_COREDUMP_FLASH_ADDR=0 -DSYN_USE_TICKLESS=1 -DSYN_USE_DMA=1 -DSYN_USE_I2C_ASYNC=1 -DSYN_USE_SPI_ASYNC=1 -DSYN_FW_USE_HMAC=1 -DSYN_FW_USE_ED25519=1 -DSYN_FW_USE_AES_GCM=1 -DSYN_USE_PORT_AES=1 -DSYN_USE_MULTICORE=1 -DSYN_USE_CAN_FD=1 -DSYN_USE_RTC=1 -DUNITY_INCLUDE_DOUBLE -DSYN_USE_METRICS=1 -DSYN_USE_ROUTER=1 -DSYN_USE_LIN=1 -DSYN_USE_IR=1 -DSYN_USE_SMBUS=1 -DSYN_USE_PMBUS=1 -DSYN_USE_WG=1 -DSYN_USE_WEBSOCKET=1"

CORE_PORT="src/syntropic/sched/syn_sched.c src/syntropic/sched/syn_mutex.c src/syntropic/sched/syn_timer.c src/syntropic/sched/syn_event_flags.c src/syntropic/util/syn_metrics.c src/syntropic/net/syn_router.c src/syntropic/system/syn_fault.c src/syntropic/system/syn_errlog.c tests/unit/mocks/mock_port.c src/port/posix/port_posix_socket.c tests/unit/unity/unity.c"

mkdir -p build/tests
rm -rf build/tests/obj build/tests/test_*_integration build/tests/wg_daemon

echo "=== Building & Compiling Integration Test Drivers ==="
PIDS=()

gcc ${CFLAGS} -DSYN_USE_SHA512=0 src/syntropic/net/syn_mqtt.c src/syntropic/util/syn_fmt.c src/syntropic/crypto/syn_hkdf.c src/syntropic/crypto/syn_sha256.c ${CORE_PORT} tests/integration/test_mqtt_integration.c -o build/tests/test_mqtt_integration -lm &
PIDS+=($!)

gcc ${CFLAGS} src/syntropic/net/syn_sntp.c src/syntropic/util/syn_backoff.c src/syntropic/dsp/syn_filter.c src/syntropic/util/syn_random.c ${CORE_PORT} tests/integration/test_sntp_integration.c -o build/tests/test_sntp_integration -lm &
PIDS+=($!)

gcc ${CFLAGS} src/syntropic/net/syn_http.c src/syntropic/util/syn_fmt.c ${CORE_PORT} tests/integration/test_http_integration.c -o build/tests/test_http_integration -lm &
PIDS+=($!)

gcc ${CFLAGS} src/syntropic/net/syn_websocket.c src/syntropic/util/syn_base64.c ${CORE_PORT} tests/integration/test_ws_integration.c -o build/tests/test_ws_integration -lm &
PIDS+=($!)

gcc ${CFLAGS} src/syntropic/net/syn_dns.c ${CORE_PORT} tests/integration/test_dns_integration.c -o build/tests/test_dns_integration -lm &
PIDS+=($!)

gcc ${CFLAGS} src/syntropic/proto/syn_cia402.c src/syntropic/proto/syn_canopen.c src/syntropic/util/syn_scurve.c src/syntropic/util/syn_qmath.c ${CORE_PORT} tests/integration/test_can_integration.c -o build/tests/test_can_integration -lm &
PIDS+=($!)

gcc ${CFLAGS} src/syntropic/net/syn_wg.c src/syntropic/crypto/syn_blake2s.c src/syntropic/crypto/syn_chacha20poly1305.c src/syntropic/crypto/syn_x25519.c src/syntropic/net/syn_sntp.c src/syntropic/util/syn_backoff.c src/syntropic/dsp/syn_filter.c src/syntropic/util/syn_random.c ${CORE_PORT} tests/integration/test_wg_integration.c -o build/tests/test_wg_integration -lm &
PIDS+=($!)

(gcc -std=c99 -D_DEFAULT_SOURCE -O2 -pedantic -Wall -Wextra -I. -Isrc -DWG_DAEMON_MAIN=1 src/syntropic/crypto/syn_blake2s.c src/syntropic/crypto/syn_chacha20poly1305.c src/syntropic/crypto/syn_x25519.c tests/integration/test_wg_integration.c -static -o build/tests/wg_daemon.tmp 2>/dev/null || gcc -std=c99 -D_DEFAULT_SOURCE -O2 -pedantic -Wall -Wextra -I. -Isrc -DWG_DAEMON_MAIN=1 src/syntropic/crypto/syn_blake2s.c src/syntropic/crypto/syn_chacha20poly1305.c src/syntropic/crypto/syn_x25519.c tests/integration/test_wg_integration.c -o build/tests/wg_daemon.tmp) && chmod +x build/tests/wg_daemon.tmp && mv build/tests/wg_daemon.tmp build/tests/wg_daemon &
PIDS+=($!)

gcc ${CFLAGS} src/syntropic/proto/syn_modbus.c src/syntropic/util/syn_crc.c ${CORE_PORT} tests/integration/test_modbus_integration.c -o build/tests/test_modbus_integration -lm &
PIDS+=($!)

gcc ${CFLAGS} -DSYN_USE_CANOPEN=0 src/syntropic/proto/syn_ethercat.c ${CORE_PORT} tests/integration/test_ecat_integration.c -o build/tests/test_ecat_integration -lm &
PIDS+=($!)

gcc ${CFLAGS} -DSYN_USE_CANOPEN=0 src/syntropic/proto/syn_ethercat.c ${CORE_PORT} tests/integration/test_soes_integration.c -o build/tests/test_soes_integration -lm &
PIDS+=($!)

gcc ${CFLAGS} src/syntropic/proto/syn_ocpp.c src/syntropic/net/syn_websocket.c src/syntropic/util/syn_base64.c ${CORE_PORT} tests/integration/test_ocpp_integration.c -o build/tests/test_ocpp_integration -lm &
PIDS+=($!)

for pid in "${PIDS[@]}"; do
    wait "${pid}"
done

echo "=== Awaiting 3rd-Party Daemon Service Readiness ==="
sleep 2

echo "=== Executing Official 3rd-Party Python ocpp CSMS Integration Test Driver ==="
pip install --break-system-packages --quiet ocpp websockets jsonschema || true
python3 "${ROOT_DIR}/tests/integration/services/ocpp/test_ocpp_csms_server.py" &
CSMS_PID=$!
sleep 2

echo "=== Executing 3rd-Party Integration Test Drivers ==="
FAILURES=0

./build/tests/test_ocpp_integration || FAILURES=$((FAILURES + 1))
kill "${CSMS_PID}" 2>/dev/null || true

./build/tests/test_mqtt_integration || FAILURES=$((FAILURES + 1))
./build/tests/test_sntp_integration || FAILURES=$((FAILURES + 1))
./build/tests/test_http_integration || FAILURES=$((FAILURES + 1))
./build/tests/test_ws_integration || FAILURES=$((FAILURES + 1))
./build/tests/test_dns_integration || FAILURES=$((FAILURES + 1))
./build/tests/test_can_integration || FAILURES=$((FAILURES + 1))
./build/tests/test_wg_integration || FAILURES=$((FAILURES + 1))
./build/tests/test_modbus_integration || FAILURES=$((FAILURES + 1))
./build/tests/test_ecat_integration || FAILURES=$((FAILURES + 1))
./build/tests/test_soes_integration || FAILURES=$((FAILURES + 1))

echo "=== Executing Official 3rd-Party Python udsoncan Client Driver ==="
pip install --break-system-packages --quiet udsoncan can-isotp python-can || true
UDS_HOST="${UDS_HOST:-127.0.0.1}" python3 "${ROOT_DIR}/tests/integration/services/uds/test_udsoncan_client.py" || FAILURES=$((FAILURES + 1))

echo "=== Executing Official 3rd-Party Python can-isotp Client Driver ==="
ISOTP_HOST="${ISOTP_HOST:-127.0.0.1}" python3 "${ROOT_DIR}/tests/integration/services/isotp/test_isotp_client.py" || FAILURES=$((FAILURES + 1))

echo "=== Executing Official 3rd-Party Python DoIP + UDS Client Driver ==="
DOIP_HOST="${DOIP_HOST:-127.0.0.1}" python3 "${ROOT_DIR}/tests/integration/services/doip/test_doip_client.py" || FAILURES=$((FAILURES + 1))

rm -rf build/tests/obj build/tests/test_*_integration build/tests/wg_daemon build/tests/wg_daemon.tmp

if [ "${FAILURES}" -ne 0 ]; then
    echo "=== 3rd-Party Integration Test Suite FAILED (${FAILURES} drivers failed) ==="
    exit 1
fi

echo "=== 3rd-Party Integration Test Suite PASS ==="
exit 0
