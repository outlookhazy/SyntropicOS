#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

echo "=== SyntropicOS Examples Cross-Compilation Check ==="

JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

python3 -c "
import glob, subprocess, sys, os, concurrent.futures

c_files = glob.glob('${ROOT_DIR}/examples/**/*.c', recursive=True)
print(f'Checking {len(c_files)} example C files across ${JOBS} parallel workers...')

def check_file(c):
    if 'ch32v307' in c:
        riscv_gcc = 'riscv-none-embed-gcc' if subprocess.run('command -v riscv-none-embed-gcc', shell=True).returncode == 0 else \
                    ('riscv64-unknown-elf-gcc' if subprocess.run('command -v riscv64-unknown-elf-gcc', shell=True).returncode == 0 else None)
        if riscv_gcc:
            cmd = f'{riscv_gcc} -std=c99 -Wall -Wextra -Werror -I${ROOT_DIR} -I${ROOT_DIR}/src -I${ROOT_DIR}/examples/ch32v307_demo -march=rv32imafc -mabi=ilp32f -c {c} -o /dev/null'
        else:
            cmd = f'gcc -std=c99 -Wall -Wextra -Werror -I${ROOT_DIR} -I${ROOT_DIR}/src -I${ROOT_DIR}/examples/ch32v307_demo -c {c} -o /dev/null'
    else:
        cmd = f'arm-none-eabi-gcc -std=c99 -Wall -Wextra -Werror -I${ROOT_DIR} -I${ROOT_DIR}/src -I${ROOT_DIR}/src/port/stm32f4 -I${ROOT_DIR}/tests/unit/mocks/stm32 -I${ROOT_DIR}/tests/unit/mocks/esp32 -I${ROOT_DIR}/tests/unit/mocks/pico -mcpu=cortex-m4 -mthumb -c {c} -o /dev/null'
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if res.returncode != 0:
        return (c, res.stderr.strip())
    return None


failed = []
with concurrent.futures.ProcessPoolExecutor(max_workers=int('${JOBS}')) as executor:
    results = executor.map(check_file, sorted(c_files))
    for res in results:
        if res is not None:
            failed.append(res)

if not failed:
    print(f'=== ALL {len(c_files)} EXAMPLES COMPILE 100% CLEAN ===')
else:
    print(f'FAILURE: {len(failed)} example files failed to compile:')
    for f, err in failed:
        print(f'=== {f} ===\n{err}\n')
    sys.exit(1)

src_files = [
    s for s in sorted(glob.glob('${ROOT_DIR}/src/syntropic/**/*.c', recursive=True))
    if '/port_stubs/' not in s
]
lean_configs = [
    ('PicoBlink', '${ROOT_DIR}/examples/PicoBlink'),
    ('pico_dual_core', '${ROOT_DIR}/examples/pico_dual_core'),
]
print(f'Checking {len(src_files)} core sources against {len(lean_configs)} lean syn_config.h profiles...')

def check_lean(item):
    cfg_name, cfg_dir, src = item
    cmd = f'arm-none-eabi-gcc -std=c99 -Wall -Wextra -Werror -I{cfg_dir} -I${ROOT_DIR} -I${ROOT_DIR}/src -mcpu=cortex-m4 -mthumb -c {src} -o /dev/null'
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if res.returncode != 0:
        return (f'{cfg_name}: {src}', res.stderr.strip())
    return None

lean_jobs = [(name, d, s) for name, d in lean_configs for s in src_files]
lean_failed = []
with concurrent.futures.ProcessPoolExecutor(max_workers=int('${JOBS}')) as executor:
    for res in executor.map(check_lean, lean_jobs):
        if res is not None:
            lean_failed.append(res)

if not lean_failed:
    print(f'=== ALL {len(lean_jobs)} LEAN SYN_CONFIG UNITS COMPILE 100% CLEAN ===')
else:
    print(f'FAILURE: {len(lean_failed)} lean syn_config compilations failed:')
    for f, err in lean_failed:
        print(f'=== {f} ===\n{err}\n')
    sys.exit(1)

hdr_files = (
    ['${ROOT_DIR}/src/SyntropicOS.h']
    + [h for h in sorted(glob.glob('${ROOT_DIR}/src/syntropic/**/*.h', recursive=True))
       if not h.endswith('/syn_config_template.h')]
    + sorted(glob.glob('${ROOT_DIR}/src/port/*.h'))
)
print(f'Checking {len(hdr_files)} public headers under Arduino C++11 (-std=gnu++11)...')

def check_cpp_header(h):
    cmd = (
        f'arm-none-eabi-g++ -std=gnu++11 -Wall -Wextra -Werror -x c++ '
        f'-I${ROOT_DIR} -I${ROOT_DIR}/src -I${ROOT_DIR}/tests/unit/mocks '
        f'-DSYN_USE_MULTICORE=1 -DSYN_USE_METRICS=1 -DSYN_USE_DMA=1 '
        f'-DSYN_USE_I2C_ASYNC=1 -DSYN_USE_SPI_ASYNC=1 -DSYN_USE_TICKLESS=1 '
        f'-DSYN_USE_COREDUMP=1 -DSYN_COREDUMP_FLASH_ADDR=0 -fsyntax-only {h}'
    )
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if res.returncode != 0:
        return (h, res.stderr.strip())
    return None

cpp_failed = []
with concurrent.futures.ProcessPoolExecutor(max_workers=int('${JOBS}')) as executor:
    for res in executor.map(check_cpp_header, hdr_files):
        if res is not None:
            cpp_failed.append(res)

if not cpp_failed:
    print(f'=== ALL {len(hdr_files)} HEADERS COMPILE 100% CLEAN UNDER ARDUINO C++11 ===')
else:
    print(f'FAILURE: {len(cpp_failed)} headers failed Arduino C++11 check:')
    for f, err in cpp_failed:
        print(f'=== {f} ===\n{err}\n')
    sys.exit(1)

port_c_files = sorted(glob.glob('${ROOT_DIR}/src/port/**/*.c', recursive=True))
print(f'Checking {len(port_c_files)} non-Arduino port C files emit 0 symbols when -DARDUINO=10819...')

def check_arduino_port_guard(p):
    obj = f'/tmp/syn_arduino_guard_{os.getpid()}_{os.path.basename(p)}.o'
    try:
        cmd = (
            f'arm-none-eabi-gcc -std=c99 -Wall -Wextra -Werror '
            f'-DARDUINO=10819 -DSTM32F407xx=1 -DSTM32F4xx=1 -DCH32V307xx=1 '
            f'-DPICO_BOARD=1 -DESP_PLATFORM=1 -D_POSIX_VERSION=200809L -DSYN_USE_MULTICORE=1 '
            f'-I${ROOT_DIR} -I${ROOT_DIR}/src -c {p} -o {obj}'
        )
        res = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if res.returncode != 0:
            return (p, res.stderr.strip())
        nm = subprocess.run(f'arm-none-eabi-nm {obj}', shell=True, capture_output=True, text=True)
        if nm.stdout.strip():
            return (p, f'Leaked symbols under -DARDUINO:\n{nm.stdout.strip()}')
        return None
    finally:
        if os.path.exists(obj):
            os.remove(obj)

guard_failed = []
with concurrent.futures.ProcessPoolExecutor(max_workers=int('${JOBS}')) as executor:
    for res in executor.map(check_arduino_port_guard, port_c_files):
        if res is not None:
            guard_failed.append(res)

if not guard_failed:
    print(f'=== ALL {len(port_c_files)} NON-ARDUINO PORT FILES GUARDED CLEANLY ===')
else:
    print(f'FAILURE: {len(guard_failed)} non-Arduino port files leaked symbols under -DARDUINO:')
    for f, err in guard_failed:
        print(f'=== {f} ===\n{err}\n')
    sys.exit(1)
"
