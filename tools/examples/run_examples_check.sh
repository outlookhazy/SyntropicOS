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
"
