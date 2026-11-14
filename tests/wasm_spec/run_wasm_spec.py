#!/usr/bin/env python3
"""
Official W3C WebAssembly Spec Test Suite driver for syn_wasm.

Converts official W3C .wast test scripts via wabt's `wast2json --no-check` and
executes every binary module load, validation check, instantiation, start
function, cross-module link, invoke, global get, NaN bit-pattern check, and trap
assertion against `spec_runner` (linked directly with `syn_wasm.c`).
"""

import argparse
import glob
import json
import os
import subprocess
import sys
from typing import Any, Dict, List, Set, Tuple


WG20_REPLACEMENTS = {
    # wg-1.0 used pre-2020 `(assert_return_canonical_nan ...)` syntax in these 5
    # float/conversion files; wg-2.0 updates their syntax and adds trunc_sat_*
    "f32.wast",
    "f64.wast",
    "float_exprs.wast",
    "float_misc.wast",
    "conversions.wast",
    # Superset files in wg-2.0 adding sign-ext and Wasm 2.0 br_table bottom rule
    "i32.wast",
    "i64.wast",
    "unreached-invalid.wast",
}

WG20_ADDITIONAL = [
    "global.wast",
    "memory_copy.wast",
    "memory_fill.wast",
    "memory_init.wast",
    "unreached-valid.wast",
]

WG20_EXCLUDED_EXTENSION_FILES = [
    "ref_null.wast",
    "ref_func.wast",
    "ref_is_null.wast",
    "table_get.wast",
    "table_set.wast",
    "table_size.wast",
    "table_grow.wast",
    "table_fill.wast",
    "table_copy.wast",
    "table_init.wast",
]


def encode_hex(text: str) -> str:
    b = text.encode("utf-8")
    return "-" if len(b) == 0 else b.hex()


def encode_val(vobj: Dict[str, Any]) -> int:
    vtype = vobj["type"]
    raw = str(vobj["value"])
    val = int(raw, 10)
    if vtype in ("i32", "f32"):
        return val & 0xFFFFFFFF
    if vtype in ("i64", "f64"):
        return val & 0xFFFFFFFFFFFFFFFF
    raise ValueError(f"Unsupported arg type: {vtype}")


def match_single_expected(actual: int, exp: Dict[str, Any]) -> bool:
    etype = exp["type"]
    if etype == "either":
        return any(match_single_expected(actual, opt) for opt in exp["values"])
    eval_str = str(exp["value"])
    if etype == "i32":
        return (actual & 0xFFFFFFFF) == (int(eval_str, 10) & 0xFFFFFFFF)
    if etype == "i64":
        return (actual & 0xFFFFFFFFFFFFFFFF) == (int(eval_str, 10) & 0xFFFFFFFFFFFFFFFF)
    if etype == "f32":
        v32 = actual & 0xFFFFFFFF
        if eval_str == "nan:canonical":
            return (v32 & 0x7FFFFFFF) == 0x7FC00000
        if eval_str == "nan:arithmetic":
            return (v32 & 0x7FC00000) == 0x7FC00000
        return v32 == (int(eval_str, 10) & 0xFFFFFFFF)
    if etype == "f64":
        v64 = actual & 0xFFFFFFFFFFFFFFFF
        if eval_str == "nan:canonical":
            return (v64 & 0x7FFFFFFFFFFFFFFF) == 0x7FF8000000000000
        if eval_str == "nan:arithmetic":
            return (v64 & 0x7FF8000000000000) == 0x7FF8000000000000
        return v64 == (int(eval_str, 10) & 0xFFFFFFFFFFFFFFFF)
    return False


class RunnerClient:
    def __init__(self, binary_path: str) -> None:
        self.proc = subprocess.Popen(
            [binary_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True,
            bufsize=1,
        )

    def cmd(self, line: str) -> str:
        assert self.proc.stdin is not None and self.proc.stdout is not None
        self.proc.stdin.write(line + "\n")
        self.proc.stdin.flush()
        resp = self.proc.stdout.readline()
        if not resp:
            rc = self.proc.poll()
            raise RuntimeError(f"spec_runner exited unexpectedly (rc={rc}) on: {line}")
        return resp.strip()

    def close(self) -> int:
        if self.proc.stdin:
            try:
                self.proc.stdin.write("QUIT\n")
                self.proc.stdin.flush()
            except BrokenPipeError:
                pass
        return self.proc.wait()


def run_action(
    runner: RunnerClient, act: Dict[str, Any], skipped_slots: Set[str]
) -> Tuple[str, bool]:
    mname = act.get("module", "-")
    if mname in skipped_slots:
        return "SKIPPED_SLOT", True
    atype = act["type"]
    fhex = encode_hex(act["field"])
    if atype == "invoke":
        args = act.get("args", [])
        # Skip invoke actions passing unsupported ref types (externref/funcref)
        for a in args:
            if a["type"] not in ("i32", "i64", "f32", "f64"):
                return "SKIPPED_REFTYPE_ARG", True
        enc_args = " ".join(str(encode_val(a)) for a in args)
        line = f"INVOKE {mname} {fhex} {len(args)}"
        if enc_args:
            line += " " + enc_args
        resp = runner.cmd(line)
        return resp, (resp == "SKIP_SHARED_TABLE")
    if atype == "get":
        return runner.cmd(f"GET {mname} {fhex}"), False
    raise ValueError(f"Unknown action type: {atype}")


def convert_wast(wast_path: str, out_dir: str, is_wg10: bool) -> str:
    os.makedirs(out_dir, exist_ok=True)
    base = os.path.splitext(os.path.basename(wast_path))[0]
    json_path = os.path.join(out_dir, f"{base}.json")
    cmd = ["wast2json", "--no-check"]
    if is_wg10:
        cmd.append("--disable-bulk-memory")
    cmd.extend([wast_path, "-o", json_path])
    res = subprocess.run(cmd, capture_output=True, text=True, check=False)
    if res.returncode != 0:
        raise RuntimeError(
            f"wast2json failed on {wast_path}:\n{res.stderr or res.stdout}"
        )
    return json_path


def run_positive_suite(
    runner: RunnerClient,
    wast_entries: List[Tuple[str, str, bool]],
    work_dir: str,
    max_failures_shown: int = 40,
) -> Tuple[int, int, int, List[str]]:
    passed = 0
    skipped = 0
    text_only = 0
    failures: List[str] = []

    for label, wast_path, is_wg10 in wast_entries:
        sub_dir = os.path.join(work_dir, label.replace("/", "__"))
        json_path = convert_wast(wast_path, sub_dir, is_wg10)
        with open(json_path, "r", encoding="utf-8") as fp:
            manifest = json.load(fp)

        resp = runner.cmd("RESET")
        if resp != "OK":
            failures.append(f"{label}: RESET returned {resp}")
            continue

        skipped_slots: Set[str] = set()

        for c in manifest.get("commands", []):
            ctype = c["type"]
            line_no = c.get("line", 0)

            if ctype == "module":
                sname = c.get("name", "-")
                wpath = os.path.join(sub_dir, c["filename"])
                r = runner.cmd(f"LOAD {sname} {wpath}")
                if r == "OK":
                    skipped_slots.discard(sname)
                    if sname != "-":
                        skipped_slots.discard("-")
                    passed += 1
                elif r in ("SKIP_MULTI_MOD", "SKIP_MEM_CAP") or r == "LOAD_ERR 5":
                    # Track skipped multi-module shared-table / 256MB+ / excluded-ext
                    # modules in wg-2.0 files (e.g. multi-memory or externref globals)
                    skipped += 1
                    skipped_slots.add(sname)
                    skipped_slots.add("-")
                else:
                    skipped_slots.add(sname)
                    skipped_slots.add("-")
                    failures.append(f"{label}:{line_no} module load failed: {r}")

            elif ctype == "register":
                sname = c.get("name", "-")
                if sname in skipped_slots:
                    skipped += 1
                    continue
                as_hex = encode_hex(c["as"])
                r = runner.cmd(f"REGISTER {as_hex} {sname}")
                if r == "OK":
                    passed += 1
                else:
                    failures.append(f"{label}:{line_no} register failed: {r}")

            elif ctype == "action":
                r, was_skip = run_action(runner, c["action"], skipped_slots)
                if was_skip:
                    skipped += 1
                elif r.startswith("OK "):
                    passed += 1
                else:
                    failures.append(f"{label}:{line_no} action failed: {r}")

            elif ctype == "assert_return":
                expected = c.get("expected", [])
                if any(
                    e["type"] not in ("i32", "i64", "f32", "f64", "either")
                    for e in expected
                ):
                    skipped += 1
                    continue
                if len(expected) > 1:
                    # Multivalue function returns are outside our single-result profile
                    skipped += 1
                    continue
                r, was_skip = run_action(runner, c["action"], skipped_slots)
                if was_skip:
                    skipped += 1
                    continue
                if not r.startswith("OK "):
                    failures.append(
                        f"{label}:{line_no} assert_return got '{r}', expected {expected}"
                    )
                    continue
                parts = r.split()
                rcnt = int(parts[1])
                vals = [int(x, 10) for x in parts[2:]]
                if rcnt != len(expected):
                    failures.append(
                        f"{label}:{line_no} result count {rcnt} != {len(expected)}"
                    )
                    continue
                if all(match_single_expected(vals[i], expected[i]) for i in range(rcnt)):
                    passed += 1
                else:
                    failures.append(
                        f"{label}:{line_no} value mismatch: got {vals}, expected {expected}"
                    )

            elif ctype in ("assert_trap", "assert_exhaustion"):
                if "action" in c:
                    r, was_skip = run_action(runner, c["action"], skipped_slots)
                    if was_skip:
                        skipped += 1
                    elif r.startswith("TRAP "):
                        passed += 1
                    else:
                        failures.append(
                            f"{label}:{line_no} {ctype} did not trap: got '{r}'"
                        )
                elif "filename" in c and c.get("module_type") == "binary":
                    wpath = os.path.join(sub_dir, c["filename"])
                    r = runner.cmd(f"LOAD - {wpath}")
                    if r.startswith(("INST_TRAP ", "START_TRAP ", "TRAP ")):
                        passed += 1
                    elif r in ("SKIP_MULTI_MOD", "SKIP_MEM_CAP"):
                        skipped += 1
                    else:
                        failures.append(
                            f"{label}:{line_no} module {ctype} did not trap: '{r}'"
                        )

            elif ctype in ("assert_invalid", "assert_malformed"):
                if c.get("module_type") != "binary" or "filename" not in c:
                    text_only += 1
                    continue
                wpath = os.path.join(sub_dir, c["filename"])
                r = runner.cmd(f"CHECK_INVALID {wpath}")
                if r.startswith("REJECTED "):
                    passed += 1
                else:
                    failures.append(
                        f"{label}:{line_no} {ctype} ({c.get('text', '')}) ACCEPTED as VALID!"
                    )

            elif ctype in ("assert_unlinkable", "assert_uninstantiable"):
                if c.get("module_type") != "binary" or "filename" not in c:
                    text_only += 1
                    continue
                wpath = os.path.join(sub_dir, c["filename"])
                r = runner.cmd(f"LOAD - {wpath}")
                if r == "UNLINKABLE" or r.startswith(("INST_TRAP ", "START_TRAP ")):
                    passed += 1
                elif r in ("SKIP_MULTI_MOD", "SKIP_MEM_CAP") or r == "LOAD_ERR 5":
                    skipped += 1
                else:
                    failures.append(
                        f"{label}:{line_no} {ctype} ({c.get('text', '')}) got '{r}'"
                    )

    if len(failures) > max_failures_shown:
        extra = len(failures) - max_failures_shown
        failures = failures[:max_failures_shown] + [f"... and {extra} more failures"]
    return passed, skipped, text_only, failures


def run_excluded_extension_rejection(
    runner: RunnerClient, spec20_core: str, work_dir: str
) -> Tuple[int, List[str]]:
    """Verify modules using excluded Wasm 2.0 extensions are rejected at load time."""
    rejected = 0
    failures: List[str] = []

    wast_files: List[str] = []
    for fname in WG20_EXCLUDED_EXTENSION_FILES:
        p = os.path.join(spec20_core, fname)
        if os.path.isfile(p):
            wast_files.append(p)
    simd_dir = os.path.join(spec20_core, "simd")
    if os.path.isdir(simd_dir):
        wast_files.extend(sorted(glob.glob(os.path.join(simd_dir, "*.wast"))))

    for wast_path in wast_files:
        rel = os.path.relpath(wast_path, spec20_core)
        sub_dir = os.path.join(work_dir, "excluded__" + rel.replace("/", "__"))
        json_path = convert_wast(wast_path, sub_dir, is_wg10=False)
        with open(json_path, "r", encoding="utf-8") as fp:
            manifest = json.load(fp)
        for c in manifest.get("commands", []):
            if c["type"] != "module":
                continue
            wpath = os.path.join(sub_dir, c["filename"])
            # Check if this module actually uses an excluded opcode/type/multi-table;
            # note that Wast files often include helper modules that are pure MVP!
            val = subprocess.run(
                [
                    "wasm-validate",
                    "--disable-simd",
                    "--disable-reference-types",
                    "--disable-multi-value",
                    wpath,
                ],
                capture_output=True,
                check=False,
            )
            if val.returncode == 0:
                # Pure MVP helper module inside the test script — allowed to load
                continue
            r = runner.cmd(f"CHECK_INVALID {wpath}")
            if r.startswith("REJECTED "):
                rejected += 1
            else:
                failures.append(
                    f"excluded/{rel}:{c.get('line', 0)} out-of-profile module was NOT rejected ({r})"
                )
    return rejected, failures


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--runner", required=True, help="Path to compiled spec_runner")
    ap.add_argument("--spec10", required=True, help="Path to WebAssembly/spec wg-1.0")
    ap.add_argument("--spec20", required=True, help="Path to WebAssembly/spec wg-2.0")
    ap.add_argument("--workdir", required=True, help="Scratch directory for wast2json")
    ap.add_argument("--filter", default="", help="Optional substring filter on .wast name")
    args = ap.parse_args()

    core10 = os.path.join(args.spec10, "test", "core")
    core20 = os.path.join(args.spec20, "test", "core")

    entries: List[Tuple[str, str, bool]] = []
    for fname in sorted(os.listdir(core10)):
        if not fname.endswith(".wast"):
            continue
        if fname in WG20_REPLACEMENTS:
            entries.append((f"wg-2.0/{fname}", os.path.join(core20, fname), False))
        else:
            entries.append((f"wg-1.0/{fname}", os.path.join(core10, fname), True))

    for fname in WG20_ADDITIONAL:
        entries.append((f"wg-2.0/{fname}", os.path.join(core20, fname), False))

    if args.filter:
        entries = [e for e in entries if args.filter in e[0]]

    runner = RunnerClient(args.runner)
    try:
        passed, skipped, text_only, failures = run_positive_suite(
            runner, entries, os.path.join(args.workdir, "pos")
        )
        excl_rejected = 0
        excl_failures: List[str] = []
        if not args.filter:
            excl_rejected, excl_failures = run_excluded_extension_rejection(
                runner, core20, os.path.join(args.workdir, "neg")
            )
    finally:
        rc = runner.close()

    print("================================================================")
    print("  SyntropicOS syn_wasm — Official W3C WebAssembly Spec Suite")
    print("================================================================")
    print(f"  Core & Extension Test Files : {len(entries)}")
    print(f"  Assertions Passed           : {passed}")
    print(f"  Excluded-Ext Modules Blocked: {excl_rejected}")
    print(f"  Out-of-Scope Skipped        : {skipped} (multi-mod tables / >1 ret / ref args)")
    print(f"  WAT Text-Parser Only (N/A)  : {text_only}")
    print(f"  Failures                    : {len(failures) + len(excl_failures)}")
    print("================================================================")

    all_failures = failures + excl_failures
    if rc != 0:
        all_failures.append(f"spec_runner exited with non-zero status {rc} (sanitizer error?)")

    if all_failures:
        for f in all_failures:
            print(f"  [FAIL] {f}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
