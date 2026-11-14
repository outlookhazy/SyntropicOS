#!/usr/bin/env python3
"""
SyntropicOS Protothread Cross-Yield Local Variable Linter

Parses C translation units containing protothreads (PT_BEGIN .. PT_END) with
Clang's JSON AST (-Xclang -ast-dump=json), constructs an intra-procedural
control-flow graph (CFG) modeling Duff's-device switch continuations across
every yield point (case __LINE__:), and performs forward dataflow analysis to
verify that no non-static stack local variable is read when its value may have
been lost or clobbered across a protothread yield.

Bug classes caught:
  1. Non-static local declared inside PT_BEGIN..PT_END whose declaration/write
     is bypassed or lost across a yield (PT_CONTINUE, PT_PASS, PT_WAIT_*,
     PT_SPAWN, PT_DELAY_*, PT_BLOCK_*, etc.) before a subsequent read.
  2. Non-static local declared before PT_BEGIN that is uninitialized or whose
     dummy initial value is overwritten inside PT_BEGIN..PT_END and read across
     a yield where prologue re-execution clobbers its stack value (including
     accidental retry/loop counters reset to a constant on every yield).
  3. Non-static loop induction variable (e.g. 'for (int i = 0; i < n; i++)')
     spanning a yield point.
  4. Non-static local array/struct populated before a yield and read after a
     yield without being repopulated in the resumed slice.
  5. Non-static 'SYN_PT' child protothread state declared on the stack inside
     a parent protothread.
"""

import argparse
import concurrent.futures
import json
import os
import re
import subprocess
import sys
import tempfile

PT_BEGIN_RE = re.compile(r"\bPT_BEGIN\s*\(")
MACRO_DEF_RE = re.compile(r"^\s*#\s*define\s+PT_BEGIN\b")

BASE_CFLAGS = [
    "-std=c99",
    "-D_DEFAULT_SOURCE",
    "-fsyntax-only",
    "-Xclang",
    "-ast-dump=json",
    "-I.",
    "-Isrc",
    "-Itests/unit",
    "-Itests/unit/mocks",
    "-DSYN_LOG_COLOR=1",
    "-DSYN_CLI_HISTORY_DEPTH=4",
    "-DSYN_USE_COREDUMP=1",
    "-DSYN_COREDUMP_FLASH_ADDR=0",
    "-DSYN_USE_TICKLESS=1",
    "-DSYN_USE_DMA=1",
    "-DSYN_USE_I2C_ASYNC=1",
    "-DSYN_USE_SPI_ASYNC=1",
    "-DSYN_FW_USE_HMAC=1",
    "-DSYN_FW_USE_ED25519=1",
    "-DSYN_FW_USE_AES_GCM=1",
    "-DSYN_USE_PORT_AES=1",
    "-DSYN_USE_MULTICORE=1",
    "-DUNITY_INCLUDE_DOUBLE",
    "-DSYN_USE_METRICS=1",
    "-DSYN_USE_ROUTER=1",
    "-DSYN_USE_LIN=1",
    "-DSYN_USE_LINTP=1",
    "-DSYN_USE_GBT27930=1",
    "-DSYN_USE_CANNM=1",
    "-DSYN_USE_IR=1",
    "-DSYN_USE_SMBUS=1",
    "-DSYN_USE_PMBUS=1",
    "-DSYN_USE_MBUS=1",
    "-DSYN_USE_DLT645=1",
    "-DSYN_USE_AT_PARSER=1",
    "-DSYN_USE_BACNET=1",
    "-DSYN_USE_DEVICENET=1",
    "-DSYN_USE_YMODEM=1",
    "-DSYN_USE_AUDIO=1",
    "-DSYN_USE_ADPCM=1",
    "-DSYN_USE_SBC=1",
    "-DSYN_USE_BLE=1",
    "-DSYN_USE_OTA=1",
    "-DSYN_USE_LWM2M_TASK=1",
    "-DSYN_USE_CAN_FD=1",
    "-DSYN_USE_RTC=1",
    "-DSYN_USE_WG=1",
]

COMPOUND_ASSIGN_OPS = {
    "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|="
}


def strip_comments(text):
    """Strip C block and line comments while preserving line structure."""
    text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return text


def count_pt_begins(filepath):
    """Return number of PT_BEGIN(...) invocations in filepath outside comments/macro defs."""
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        raw = f.read()
    if "PT_BEGIN" not in raw:
        return 0
    cleaned = strip_comments(raw)
    count = 0
    for line in cleaned.splitlines():
        if MACRO_DEF_RE.match(line):
            continue
        count += len(PT_BEGIN_RE.findall(line))
    return count


def extract_pt_func_filter(filepath):
    """
    If all C functions containing PT_BEGIN in filepath share a common name
    substring of length >= 3, return it for -ast-dump-filter fast path.
    """
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        cleaned = strip_comments(f.read())

    func_names = []
    header_re = re.compile(r"([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{")
    pos = 0
    n = len(cleaned)
    while pos < n:
        m = header_re.search(cleaned, pos)
        if not m:
            break
        fname = m.group(1)
        brace_start = m.end() - 1
        depth = 0
        idx = brace_start
        while idx < n:
            ch = cleaned[idx]
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    break
            idx += 1
        body = cleaned[brace_start : idx + 1]
        if fname not in ("if", "while", "for", "switch") and PT_BEGIN_RE.search(body):
            func_names.append(fname)
        pos = idx + 1

    if not func_names:
        return None
    if len(func_names) == 1:
        return func_names[0]

    shortest = min(func_names, key=len)
    best = ""
    for i in range(len(shortest)):
        for j in range(i + len(best) + 1, len(shortest) + 1):
            sub = shortest[i:j]
            if len(sub) >= 3 and all(sub in fn for fn in func_names):
                best = sub
    return best if len(best) >= 3 else None


def normalize_ast_locs(root):
    """
    Clang's JSONNodeDumper omits 'file' and 'line' when unchanged from the
    previously emitted SourceLocation in preorder traversal. Restore both on
    every location object so diagnostics always report exact source lines.
    """
    cur = {"file": "", "line": 1}

    def fix_bare_loc(d):
        if not isinstance(d, dict) or not d:
            return
        if "file" in d:
            cur["file"] = d["file"]
        elif cur["file"]:
            d["file"] = cur["file"]
        if "line" in d:
            cur["line"] = d["line"]
        else:
            d["line"] = cur["line"]

    def fix_full_loc(d):
        if not isinstance(d, dict) or not d:
            return
        if "spellingLoc" in d or "expansionLoc" in d:
            if "spellingLoc" in d:
                fix_bare_loc(d["spellingLoc"])
            if "expansionLoc" in d:
                fix_bare_loc(d["expansionLoc"])
            exp = d.get("expansionLoc") or d.get("spellingLoc") or {}
            if "file" in exp:
                d.setdefault("file", exp["file"])
            if "line" in exp:
                d.setdefault("line", exp["line"])
        else:
            fix_bare_loc(d)

    def walk(node):
        if not isinstance(node, dict):
            return
        if "loc" in node:
            fix_full_loc(node["loc"])
        rng = node.get("range")
        if isinstance(rng, dict):
            if "begin" in rng:
                fix_full_loc(rng["begin"])
            if "end" in rng:
                fix_full_loc(rng["end"])
        if node.get("kind") == "TranslationUnitDecl":
            for ch in node.get("inner", []):
                if not isinstance(ch, dict):
                    continue
                if ch.get("kind") == "FunctionDecl":
                    ch_inner = ch.get("inner", [])
                    if any(isinstance(x, dict) and x.get("kind") == "CompoundStmt" for x in ch_inner):
                        walk(ch)
                        continue
                if "loc" in ch:
                    fix_full_loc(ch["loc"])
                crng = ch.get("range")
                if isinstance(crng, dict):
                    if "begin" in crng:
                        fix_full_loc(crng["begin"])
                    if "end" in crng:
                        fix_full_loc(crng["end"])
            return
        for ch in node.get("inner", []):
            walk(ch)

    walk(root)


def get_loc(node, fallback=(1, 1)):
    """Extract (line, col) preferring macro expansionLoc in the user source file."""
    if not isinstance(node, dict):
        return fallback
    loc = node.get("loc")
    if isinstance(loc, dict):
        exp = loc.get("expansionLoc")
        if isinstance(exp, dict) and "line" in exp:
            return (exp["line"], exp.get("col", 1))
        if "line" in loc:
            return (loc["line"], loc.get("col", 1))
    rng = node.get("range")
    if isinstance(rng, dict):
        beg = rng.get("begin")
        if isinstance(beg, dict):
            exp = beg.get("expansionLoc")
            if isinstance(exp, dict) and "line" in exp:
                return (exp["line"], exp.get("col", 1))
            if "line" in beg:
                return (beg["line"], beg.get("col", 1))
    return fallback


def get_var_init_expr(vd):
    """Return the initializer AST node of a VarDecl, or None if uninitialized."""
    if "init" not in vd:
        return None
    exprs = [
        ch
        for ch in vd.get("inner", [])
        if isinstance(ch, dict) and ch.get("kind") and not ch["kind"].endswith("Attr")
    ]
    return exprs[-1] if exprs else None


def is_compile_time_const_expr(expr):
    """Return True if expr is a compile-time literal/cast expression with no VarDecl/CallExpr."""
    if not isinstance(expr, dict) or not expr.get("kind"):
        return False
    k = expr["kind"]
    if k in ("DeclRefExpr", "CallExpr", "MemberExpr", "ArraySubscriptExpr"):
        return False
    if k in ("IntegerLiteral", "FloatingLiteral", "CharacterLiteral", "InitListExpr"):
        return all(is_compile_time_const_expr(ch) for ch in expr.get("inner", []))
    inner = expr.get("inner", [])
    return bool(inner) and all(is_compile_time_const_expr(ch) for ch in inner)


def is_syn_pt_type(vd):
    """Return True if VarDecl has type SYN_PT (not pointer/array)."""
    t = vd.get("type", {})
    for key in ("qualType", "desugaredQualType"):
        qt = t.get(key, "")
        qt_clean = re.sub(r"\b(const|volatile)\b", "", qt).strip()
        if qt_clean in ("SYN_PT", "struct SYN_PT"):
            return True
    return False


def unwrap_parens(node):
    """Strip ParenExpr wrappers."""
    while isinstance(node, dict) and node.get("kind") == "ParenExpr":
        inner = node.get("inner", [])
        if not inner:
            break
        node = inner[0]
    return node


def unwrap_noop_casts(node):
    """Strip ParenExpr and non-rvalue ImplicitCastExpr/CStyleCastExpr wrappers."""
    while isinstance(node, dict):
        k = node.get("kind")
        if k == "ParenExpr":
            inner = node.get("inner", [])
            if not inner:
                break
            node = inner[0]
        elif k in ("ImplicitCastExpr", "CStyleCastExpr", "ConstantExpr"):
            if node.get("castKind") == "LValueToRValue":
                break
            inner = node.get("inner", [])
            if not inner:
                break
            node = inner[0]
        else:
            break
    return node


def extract_addr_or_array_var(arg_node):
    """
    If arg_node is '&var' or array-decayed 'var' (or '&var.field' / '&var[0]'),
    return (var_id, index_exprs_to_eval). Otherwise return (None, []).
    """
    cur = unwrap_parens(arg_node)
    saw_array_decay = False
    while isinstance(cur, dict) and cur.get("kind") in ("ImplicitCastExpr", "CStyleCastExpr", "ParenExpr"):
        ck = cur.get("castKind")
        if ck == "LValueToRValue":
            return (None, [])
        if ck == "ArrayToPointerDecay":
            saw_array_decay = True
        inner = cur.get("inner", [])
        if not inner:
            return (None, [])
        cur = inner[0]

    if not isinstance(cur, dict):
        return (None, [])

    if cur.get("kind") == "UnaryOperator" and cur.get("opcode") == "&":
        sub = cur.get("inner", [None])[0]
        return extract_lvalue_root_var(sub)

    if saw_array_decay:
        return extract_lvalue_root_var(cur)

    return (None, [])


def extract_lvalue_root_var(lhs_node):
    """
    Given an lvalue expression (e.g. 'x', 'pkt.field', 'buf[i]', 'pkt.arr[i]'),
    walk through direct '.' MemberExpr and ArraySubscriptExpr (only when base is
    ArrayToPointerDecay, NOT pointer LValueToRValue!) to find the root VarDecl id.
    Returns (var_id, extra_read_exprs).
    """
    extra_exprs = []
    cur = unwrap_parens(lhs_node)
    while isinstance(cur, dict):
        k = cur.get("kind")
        if k == "ParenExpr":
            cur = cur.get("inner", [None])[0]
        elif k == "DeclRefExpr":
            ref = cur.get("referencedDecl", {})
            if ref.get("kind") == "VarDecl":
                return (ref.get("id"), extra_exprs)
            return (None, extra_exprs)
        elif k == "MemberExpr" and not cur.get("isArrow", False):
            cur = cur.get("inner", [None])[0]
        elif k == "ArraySubscriptExpr":
            inner = cur.get("inner", [])
            if len(inner) != 2:
                return (None, extra_exprs)
            base, idx = inner[0], inner[1]
            b_unwrapped = unwrap_parens(base)
            if (
                isinstance(b_unwrapped, dict)
                and b_unwrapped.get("kind") == "ImplicitCastExpr"
                and b_unwrapped.get("castKind") == "ArrayToPointerDecay"
            ):
                extra_exprs.append(idx)
                cur = b_unwrapped.get("inner", [None])[0]
            else:
                extra_exprs.append(base)
                extra_exprs.append(idx)
                return (None, extra_exprs)
        elif k == "UnaryOperator" and cur.get("opcode") == "*":
            sub = unwrap_noop_casts(cur.get("inner", [None])[0])
            if isinstance(sub, dict) and sub.get("kind") == "UnaryOperator" and sub.get("opcode") == "&":
                cur = sub.get("inner", [None])[0]
            else:
                extra_exprs.append(cur.get("inner", [None])[0])
                return (None, extra_exprs)
        else:
            extra_exprs.append(cur)
            return (None, extra_exprs)
    return (None, extra_exprs)


def is_pointer_to_const(type_dict):
    """Return True if type_dict represents a pointer to a const-qualified pointee."""
    if not isinstance(type_dict, dict):
        return False
    qt = type_dict.get("desugaredQualType") or type_dict.get("qualType", "")
    star_idx = qt.rfind("*")
    if star_idx == -1:
        return False
    pointee = qt[:star_idx]
    return bool(re.search(r"\bconst\b", pointee))


def extract_callee_decl_id(callee_node):
    """Return referenced FunctionDecl id of a direct CallExpr callee, or None."""
    cur = unwrap_noop_casts(callee_node)
    if isinstance(cur, dict) and cur.get("kind") == "DeclRefExpr":
        ref = cur.get("referencedDecl", {})
        if ref.get("kind") == "FunctionDecl":
            return ref.get("id")
    return None


def build_addr_only_param_map(tu_node):
    """
    Identify pointer parameters of defined functions in the TU that are ONLY
    tested for NULL/equality or cast to void (never dereferenced, indexed,
    arrow-accessed, assigned out, returned, or forwarded to another call).
    Passing '&v' or 'buf' to such a parameter inspects only its address.
    Returns dict: func_decl_id -> set(param_index).
    """
    addr_only = {}
    if not isinstance(tu_node, dict):
        return addr_only

    for fn in tu_node.get("inner", []):
        if not isinstance(fn, dict) or fn.get("kind") != "FunctionDecl":
            continue
        fid = fn.get("id")
        inner = fn.get("inner", [])
        body = next(
            (ch for ch in reversed(inner) if isinstance(ch, dict) and ch.get("kind") == "CompoundStmt"),
            None,
        )
        if not fid or body is None:
            continue
        params = [ch for ch in inner if isinstance(ch, dict) and ch.get("kind") == "ParmVarDecl"]
        if not params:
            continue

        param_ids = {p["id"]: idx for idx, p in enumerate(params) if "id" in p}
        deref_or_escaped = set()

        def expr_refs_param(node):
            found = set()
            if not isinstance(node, dict):
                return found
            if node.get("kind") == "DeclRefExpr":
                rid = node.get("referencedDecl", {}).get("id")
                if rid in param_ids:
                    found.add(rid)
            for ch in node.get("inner", []):
                found |= expr_refs_param(ch)
            return found

        def walk(node):
            if not isinstance(node, dict):
                return
            k = node.get("kind")
            if k == "UnaryOperator" and node.get("opcode") == "*":
                deref_or_escaped.update(expr_refs_param(node))
            elif k == "MemberExpr" and node.get("isArrow", False):
                deref_or_escaped.update(expr_refs_param(node.get("inner", [None])[0]))
            elif k == "ArraySubscriptExpr":
                deref_or_escaped.update(expr_refs_param(node))
            elif k == "CallExpr":
                for arg in node.get("inner", [])[1:]:
                    deref_or_escaped.update(expr_refs_param(arg))
            elif k == "ReturnStmt":
                deref_or_escaped.update(expr_refs_param(node))
            elif k == "VarDecl":
                deref_or_escaped.update(expr_refs_param(node))
            elif k == "BinaryOperator" and node.get("opcode") not in ("==", "!=", "&&", "||"):
                deref_or_escaped.update(expr_refs_param(node))
            for ch in node.get("inner", []):
                walk(ch)

        walk(body)
        pure_indices = {
            idx
            for pid, idx in param_ids.items()
            if pid not in deref_or_escaped and "*" in params[idx].get("type", {}).get("qualType", "")
        }
        if pure_indices:
            addr_only[fid] = pure_indices

    return addr_only


def parse_callee_variadic_info(callee_node):
    """Return (is_variadic, num_fixed_params) for a CallExpr callee."""
    t = callee_node.get("type", {}) if isinstance(callee_node, dict) else {}
    qt = t.get("desugaredQualType") or t.get("qualType", "")
    close_idx = qt.rfind(")")
    if close_idx == -1:
        return (False, 999)
    depth = 0
    open_idx = -1
    for i in range(close_idx, -1, -1):
        if qt[i] == ")":
            depth += 1
        elif qt[i] == "(":
            depth -= 1
            if depth == 0:
                open_idx = i
                break
    if open_idx == -1:
        return (False, 999)
    params_str = qt[open_idx + 1 : close_idx].strip()
    if not params_str or params_str == "void":
        return (False, 0)
    parts = []
    cur = []
    d = 0
    for ch in params_str:
        if ch == "(":
            d += 1
        elif ch == ")":
            d -= 1
        elif ch == "," and d == 0:
            parts.append("".join(cur).strip())
            cur = []
            continue
        cur.append(ch)
    parts.append("".join(cur).strip())
    if parts and parts[-1] == "...":
        return (True, len(parts) - 1)
    return (False, len(parts))


def const_bool_val(expr):
    """Evaluate compile-time integer constant conditions ('0' or '1')."""
    cur = unwrap_noop_casts(expr)
    if not isinstance(cur, dict):
        return None
    k = cur.get("kind")
    if k == "IntegerLiteral":
        try:
            return int(cur.get("value", "0"), 0) != 0
        except ValueError:
            return None
    if k == "UnaryOperator" and cur.get("opcode") == "!":
        inner = cur.get("inner", [None])[0]
        sub = const_bool_val(inner)
        return (not sub) if sub is not None else None
    return None


def find_pt_blocks(func_body):
    """Find all PT_BEGIN..PT_END DoStmt nodes in func_body."""
    results = []

    def walk(node):
        if not isinstance(node, dict):
            return
        if node.get("kind") == "DoStmt":
            inner = node.get("inner", [])
            if inner and isinstance(inner[0], dict) and inner[0].get("kind") == "CompoundStmt":
                cstmt = inner[0]
                has_yield_flag = False
                sw_node = None
                for ch in cstmt.get("inner", []):
                    if not isinstance(ch, dict):
                        continue
                    if ch.get("kind") == "DeclStmt":
                        for decl in ch.get("inner", []):
                            if isinstance(decl, dict) and decl.get("name") == "_pt_yield_flag":
                                has_yield_flag = True
                    elif ch.get("kind") == "SwitchStmt":
                        sw_node = ch
                if has_yield_flag and sw_node is not None:
                    results.append((node, sw_node))
                    return
        for ch in node.get("inner", []):
            walk(ch)

    walk(func_body)
    return results


def collect_referenced_vars(root_node):
    """Return set of VarDecl ids referenced anywhere inside root_node."""
    refs = set()

    def walk(node):
        if not isinstance(node, dict):
            return
        if node.get("kind") == "DeclRefExpr":
            ref = node.get("referencedDecl", {})
            if ref.get("kind") == "VarDecl" and "id" in ref:
                refs.add(ref["id"])
        for ch in node.get("inner", []):
            walk(ch)

    walk(root_node)
    return refs


def collect_mutated_vars(root_node, addr_only_params):
    """Return set of VarDecl ids written/mutated anywhere inside root_node."""
    mutated = set()

    def walk(node):
        if not isinstance(node, dict):
            return
        k = node.get("kind")
        if k == "BinaryOperator":
            op = node.get("opcode")
            inner = node.get("inner", [])
            if len(inner) == 2 and (op == "=" or op in COMPOUND_ASSIGN_OPS):
                vid, _ = extract_lvalue_root_var(inner[0])
                if vid:
                    mutated.add(vid)
        elif k == "UnaryOperator" and node.get("opcode") in ("++", "--"):
            inner = node.get("inner", [])
            if inner:
                vid, _ = extract_lvalue_root_var(inner[0])
                if vid:
                    mutated.add(vid)
        elif k == "CallExpr":
            inner = node.get("inner", [])
            if inner:
                callee = inner[0]
                callee_id = extract_callee_decl_id(callee)
                addr_only_idxs = addr_only_params.get(callee_id, set())
                is_var, n_fixed = parse_callee_variadic_info(callee)
                for idx, arg in enumerate(inner[1:]):
                    if idx in addr_only_idxs:
                        continue
                    vid, _ = extract_addr_or_array_var(arg)
                    if vid:
                        if not (is_var and idx >= n_fixed) and not is_pointer_to_const(arg.get("type")):
                            mutated.add(vid)
        for ch in node.get("inner", []):
            walk(ch)

    walk(root_node)
    return mutated


class CFGBuilder:
    """Builds a statement/condition CFG for a PT_BEGIN SwitchStmt."""

    def __init__(self):
        self.nodes = []
        self.labels = {}
        self.case_zero_nodes = []
        self.resume_case_nodes = []

    def new_node(self, kind, **kwargs):
        nid = len(self.nodes)
        d = {"id": nid, "kind": kind}
        d.update(kwargs)
        self.nodes.append(d)
        return nid

    def prescan_labels(self, node):
        if not isinstance(node, dict):
            return
        if node.get("kind") == "LabelStmt" and "name" in node:
            name = node["name"]
            if name not in self.labels:
                self.labels[name] = self.new_node("nop", succ=None)
        for ch in node.get("inner", []):
            self.prescan_labels(ch)

    def build(self, switch_stmt):
        self.prescan_labels(switch_stmt)
        exit_id = self.new_node("nop", succ=None)
        inner = switch_stmt.get("inner", [])
        if len(inner) < 2:
            return exit_id
        sw_body = inner[-1]
        self.build_stmt(sw_body, next_id=exit_id, break_id=exit_id, cont_id=exit_id)
        return exit_id

    def build_stmt(self, stmt, next_id, break_id, cont_id):
        if not isinstance(stmt, dict) or not stmt.get("kind"):
            return next_id

        k = stmt["kind"]

        if k == "NullStmt":
            return next_id

        if k == "CompoundStmt":
            cur = next_id
            for ch in reversed(stmt.get("inner", [])):
                cur = self.build_stmt(ch, cur, break_id, cont_id)
            return cur

        if k == "CaseStmt":
            inner = stmt.get("inner", [])
            case_expr = inner[0] if inner else None
            sub_stmt = inner[-1] if len(inner) >= 2 else None
            is_zero = const_bool_val(case_expr) is False
            sub_entry = self.build_stmt(sub_stmt, next_id, break_id, cont_id)
            cid = self.new_node("case", is_zero=is_zero, succ=sub_entry, ast=stmt)
            if is_zero:
                self.case_zero_nodes.append(cid)
            else:
                self.resume_case_nodes.append(cid)
            return cid

        if k == "DefaultStmt":
            inner = stmt.get("inner", [])
            sub_stmt = inner[-1] if inner else None
            sub_entry = self.build_stmt(sub_stmt, next_id, break_id, cont_id)
            cid = self.new_node("case", is_zero=False, succ=sub_entry, ast=stmt)
            self.resume_case_nodes.append(cid)
            return cid

        if k == "LabelStmt":
            name = stmt.get("name")
            inner = stmt.get("inner", [])
            sub_stmt = inner[-1] if inner else None
            sub_entry = self.build_stmt(sub_stmt, next_id, break_id, cont_id)
            lid = self.labels.get(name)
            if lid is not None:
                self.nodes[lid]["succ"] = sub_entry
                return lid
            return sub_entry

        if k == "GotoStmt":
            for lname, lid in self.labels.items():
                if lname == stmt.get("name"):
                    return lid
            return next_id

        if k == "ReturnStmt":
            return self.new_node("stmt", ast=stmt, succ=None)

        if k == "BreakStmt":
            return break_id

        if k == "ContinueStmt":
            return cont_id

        if k == "IfStmt":
            inner = stmt.get("inner", [])
            has_else = stmt.get("hasElse", False)
            if has_else and len(inner) >= 3:
                cond, then_s, else_s = inner[-3], inner[-2], inner[-1]
            elif len(inner) >= 2:
                cond, then_s, else_s = inner[-2], inner[-1], None
            else:
                return next_id
            then_entry = self.build_stmt(then_s, next_id, break_id, cont_id)
            else_entry = self.build_stmt(else_s, next_id, break_id, cont_id) if else_s else next_id
            return self.new_node("cond", ast=cond, true_succ=then_entry, false_succ=else_entry)

        if k == "WhileStmt":
            inner = stmt.get("inner", [])
            if len(inner) < 2:
                return next_id
            cond, body = inner[-2], inner[-1]
            cond_id = self.new_node("cond", ast=cond, true_succ=None, false_succ=next_id)
            body_entry = self.build_stmt(body, next_id=cond_id, break_id=next_id, cont_id=cond_id)
            self.nodes[cond_id]["true_succ"] = body_entry
            return cond_id

        if k == "DoStmt":
            inner = stmt.get("inner", [])
            if len(inner) < 2:
                return next_id
            body, cond = inner[0], inner[1]
            cond_id = self.new_node("cond", ast=cond, true_succ=None, false_succ=next_id)
            body_entry = self.build_stmt(body, next_id=cond_id, break_id=next_id, cont_id=cond_id)
            self.nodes[cond_id]["true_succ"] = body_entry
            return body_entry

        if k == "ForStmt":
            inner = stmt.get("inner", [])
            if len(inner) == 5:
                init_s, _cond_var, cond_s, inc_s, body_s = inner
            else:
                return next_id
            has_cond = isinstance(cond_s, dict) and bool(cond_s.get("kind"))
            has_inc = isinstance(inc_s, dict) and bool(inc_s.get("kind"))
            has_init = isinstance(init_s, dict) and bool(init_s.get("kind"))

            if has_cond:
                cond_id = self.new_node("cond", ast=cond_s, true_succ=None, false_succ=next_id)
            else:
                cond_id = self.new_node("nop", succ=None)

            inc_entry = (
                self.new_node("stmt", ast=inc_s, succ=cond_id) if has_inc else cond_id
            )
            body_entry = self.build_stmt(
                body_s, next_id=inc_entry, break_id=next_id, cont_id=inc_entry
            )
            if has_cond:
                self.nodes[cond_id]["true_succ"] = body_entry
            else:
                self.nodes[cond_id]["succ"] = body_entry

            if has_init:
                return self.build_stmt(init_s, next_id=cond_id, break_id=next_id, cont_id=cont_id)
            return cond_id

        return self.new_node("stmt", ast=stmt, succ=next_id)


def cfg_intra_slice_succs(builder, nid):
    """Yield successor CFG node IDs of nid that do NOT enter a resume CaseStmt."""
    node = builder.nodes[nid]
    nk = node["kind"]
    cand = []
    if nk in ("nop", "case", "stmt"):
        if node.get("succ") is not None:
            cand.append(node["succ"])
    elif nk == "cond":
        cval = const_bool_val(node["ast"])
        if cval is True:
            if node.get("true_succ") is not None:
                cand.append(node["true_succ"])
        elif cval is False:
            if node.get("false_succ") is not None:
                cand.append(node["false_succ"])
        else:
            if node.get("true_succ") is not None:
                cand.append(node["true_succ"])
            if node.get("false_succ") is not None:
                cand.append(node["false_succ"])
    for s in cand:
        snode = builder.nodes[s]
        if snode["kind"] == "case" and not snode.get("is_zero", False):
            continue
        yield s


def analyze_pt_block(filepath, func_name, func_body, pt_do, pt_switch, addr_only_params):
    """Analyze a single PT_BEGIN..PT_END block inside func_body and return error strings."""
    errors = []
    referenced_in_pt = collect_referenced_vars(pt_switch)
    mutated_in_pt = collect_mutated_vars(pt_switch, addr_only_params)

    tracked_vars = {}
    prologue_candidates = {}

    def scan_decls(node, before_pt_state):
        if not isinstance(node, dict):
            return before_pt_state
        if node is pt_do:
            scan_inside_pt(pt_switch)
            return False

        if node.get("kind") == "VarDecl" and before_pt_state:
            sc = node.get("storageClass")
            vname = node.get("name", "<anon>")
            vid = node.get("id")
            if sc not in ("static", "extern") and vname != "_pt_yield_flag" and vid in referenced_in_pt:
                line, col = get_loc(node)
                if is_syn_pt_type(node):
                    errors.append(
                        f"{filepath}:{line}:{col}: error: non-static 'SYN_PT' local '{vname}' "
                        f"in protothread '{func_name}' loses continuation state across yields; "
                        f"declare 'static' or move to persistent context."
                    )
                else:
                    init_e = get_var_init_expr(node)
                    has_init = init_e is not None
                    if not has_init:
                        tracked_vars[vid] = {
                            "name": vname,
                            "line": line,
                            "col": col,
                            "cls": "prologue",
                            "init_safe_at_zero": False,
                        }
                    elif vid in mutated_in_pt:
                        prologue_candidates[vid] = {
                            "name": vname,
                            "line": line,
                            "col": col,
                            "const_init": is_compile_time_const_expr(init_e),
                        }
            return before_pt_state

        for ch in node.get("inner", []):
            before_pt_state = scan_decls(ch, before_pt_state)
        return before_pt_state

    def scan_inside_pt(node):
        if not isinstance(node, dict):
            return
        if node.get("kind") == "VarDecl":
            sc = node.get("storageClass")
            vname = node.get("name", "<anon>")
            vid = node.get("id")
            if sc not in ("static", "extern") and vname != "_pt_yield_flag" and vid:
                line, col = get_loc(node)
                if is_syn_pt_type(node) and vid in referenced_in_pt:
                    errors.append(
                        f"{filepath}:{line}:{col}: error: non-static 'SYN_PT' local '{vname}' "
                        f"in protothread '{func_name}' loses continuation state across yields; "
                        f"declare 'static' or move to persistent context."
                    )
                else:
                    tracked_vars[vid] = {
                        "name": vname,
                        "line": line,
                        "col": col,
                        "cls": "inside_pt",
                        "init_safe_at_zero": False,
                    }
        for ch in node.get("inner", []):
            scan_inside_pt(ch)

    scan_decls(func_body, True)

    if not tracked_vars and not prologue_candidates:
        return errors

    builder = CFGBuilder()
    builder.build(pt_switch)

    # Determine which initialized prologue variables have a LIVE prologue value at case 0
    # (e.g. per-slice budget like syn_ao's 'burst_budget') vs dummy initializers ('int rc = 0;')
    # that are always overwritten before being read on case 0.
    if prologue_candidates:
        cand_ids = frozenset(prologue_candidates.keys())
        node_reads = [set() for _ in builder.nodes]
        node_writes = [set() for _ in builder.nodes]
        node_rmw = [set() for _ in builder.nodes]
        node_write_locs = {}

        def summarize_node_rw(nid, expr):
            if not isinstance(expr, dict) or not expr.get("kind"):
                return
            k = expr["kind"]
            if k == "UnaryExprOrTypeTraitExpr":
                return
            if k == "ImplicitCastExpr" and expr.get("castKind") == "LValueToRValue":
                sub = expr.get("inner", [None])[0]
                vid, extra = extract_lvalue_root_var(sub)
                for ex in extra:
                    summarize_node_rw(nid, ex)
                if vid in cand_ids and vid not in node_writes[nid]:
                    node_reads[nid].add(vid)
                return
            if k == "BinaryOperator":
                op = expr.get("opcode")
                inner = expr.get("inner", [])
                if len(inner) == 2:
                    lhs, rhs = inner[0], inner[1]
                    if op == "=":
                        summarize_node_rw(nid, rhs)
                        vid, extra = extract_lvalue_root_var(lhs)
                        for ex in extra:
                            summarize_node_rw(nid, ex)
                        if vid in cand_ids:
                            node_writes[nid].add(vid)
                            node_write_locs.setdefault((nid, vid), get_loc(expr))
                        return
                    if op in COMPOUND_ASSIGN_OPS:
                        summarize_node_rw(nid, rhs)
                        vid, extra = extract_lvalue_root_var(lhs)
                        for ex in extra:
                            summarize_node_rw(nid, ex)
                        if vid in cand_ids:
                            if vid not in node_writes[nid]:
                                node_reads[nid].add(vid)
                            node_writes[nid].add(vid)
                            node_rmw[nid].add(vid)
                            node_write_locs.setdefault((nid, vid), get_loc(expr))
                        return
            if k == "UnaryOperator":
                op = expr.get("opcode")
                sub = expr.get("inner", [None])[0]
                if op in ("++", "--"):
                    vid, extra = extract_lvalue_root_var(sub)
                    for ex in extra:
                        summarize_node_rw(nid, ex)
                    if vid in cand_ids:
                        if vid not in node_writes[nid]:
                            node_reads[nid].add(vid)
                        node_writes[nid].add(vid)
                        node_rmw[nid].add(vid)
                        node_write_locs.setdefault((nid, vid), get_loc(expr))
                    return
                if op == "&":
                    _vid, extra = extract_lvalue_root_var(sub)
                    for ex in extra:
                        summarize_node_rw(nid, ex)
                    return
            if k == "CallExpr":
                inner = expr.get("inner", [])
                if inner:
                    callee = inner[0]
                    summarize_node_rw(nid, callee)
                    callee_id = extract_callee_decl_id(callee)
                    addr_only_idxs = addr_only_params.get(callee_id, set())
                    is_var, n_fixed = parse_callee_variadic_info(callee)
                    for idx, arg in enumerate(inner[1:]):
                        if idx in addr_only_idxs:
                            continue
                        vid, idx_exprs = extract_addr_or_array_var(arg)
                        if vid is not None:
                            for ie in idx_exprs:
                                summarize_node_rw(nid, ie)
                            if vid in cand_ids:
                                if is_var and idx >= n_fixed or is_pointer_to_const(arg.get("type")):
                                    if vid not in node_writes[nid]:
                                        node_reads[nid].add(vid)
                                else:
                                    node_writes[nid].add(vid)
                                    node_write_locs.setdefault((nid, vid), get_loc(arg))
                        else:
                            summarize_node_rw(nid, arg)
                    return
            for ch in expr.get("inner", []):
                summarize_node_rw(nid, ch)

        for nid, nd in enumerate(builder.nodes):
            if nd["kind"] in ("stmt", "cond"):
                summarize_node_rw(nid, nd["ast"])

        # Check reachability of an un-overwritten read of 'vid' from case 0 within the first slice
        live_from_prologue = set()
        for vid, cinfo in prologue_candidates.items():
            visited = set()
            stack = list(builder.case_zero_nodes)
            while stack:
                cur = stack.pop()
                if cur in visited:
                    continue
                visited.add(cur)
                if vid in node_reads[cur]:
                    live_from_prologue.add(vid)
                    break
                if vid in node_writes[cur]:
                    continue
                for s in cfg_intra_slice_succs(builder, cur):
                    stack.append(s)

            if vid not in live_from_prologue:
                # Dummy prologue init (e.g. 'int rc = 0;'): clobbered if relied upon across yield!
                tracked_vars[vid] = {
                    "name": cinfo["name"],
                    "line": cinfo["line"],
                    "col": cinfo["col"],
                    "cls": "prologue",
                    "init_safe_at_zero": True,
                }
            elif cinfo["const_init"]:
                # Check Rule P2: constant-initialized counter with RMW that has NO intra-slice cycle
                for wnid in range(len(builder.nodes)):
                    if vid not in node_rmw[wnid]:
                        continue
                    has_cycle = False
                    seen = set()
                    q = list(cfg_intra_slice_succs(builder, wnid))
                    while q:
                        cur = q.pop()
                        if cur == wnid:
                            has_cycle = True
                            break
                        if cur in seen:
                            continue
                        seen.add(cur)
                        q.extend(cfg_intra_slice_succs(builder, cur))
                    if not has_cycle:
                        wline, wcol = node_write_locs.get((wnid, vid), (cinfo["line"], cinfo["col"]))
                        errors.append(
                            f"{filepath}:{wline}:{wcol}: error: non-static local '{cinfo['name']}' "
                            f"(declared before PT_BEGIN at line {cinfo['line']}) in protothread "
                            f"'{func_name}' is updated across a yield without an intra-slice loop, "
                            f"so prologue re-initialization resets it on every resume. "
                            f"Declare 'static' or move to persistent context."
                        )

    if not tracked_vars:
        return errors

    all_unsafe = frozenset(tracked_vars.keys())
    zero_unsafe = frozenset(
        vid for vid, info in tracked_vars.items() if not info["init_safe_at_zero"]
    )

    in_states = [None] * len(builder.nodes)
    worklist = []

    def push_edge(target_id, state):
        if target_id is None:
            return
        prev = in_states[target_id]
        merged = state if prev is None else (prev | state)
        if merged != prev:
            in_states[target_id] = merged
            worklist.append(target_id)

    for cid in builder.case_zero_nodes:
        push_edge(cid, zero_unsafe)
    for cid in builder.resume_case_nodes:
        push_edge(cid, all_unsafe)

    violations = {}

    def record_read_if_unsafe(vid, use_node, state):
        if vid in tracked_vars and vid in state:
            info = tracked_vars[vid]
            uline, ucol = get_loc(use_node, (info["line"], info["col"]))
            key = (uline, ucol, vid)
            if key not in violations:
                if info["cls"] == "prologue":
                    reason = (
                        f"non-static local '{info['name']}' (declared before PT_BEGIN at line "
                        f"{info['line']}) is mutated in '{func_name}' and read across/after a "
                        f"protothread yield where prologue re-execution clobbers its value"
                    )
                else:
                    reason = (
                        f"non-static local '{info['name']}' (declared at line {info['line']}) "
                        f"in protothread '{func_name}' may be read after a yield point "
                        f"without being re-assigned in the resumed slice"
                    )
                violations[key] = (
                    f"{filepath}:{uline}:{ucol}: error: {reason}. "
                    f"Declare 'static', move to persistent task/session context, or recompute after yield."
                )

    def eval_expr(expr, state):
        if not isinstance(expr, dict) or not expr.get("kind"):
            return state
        k = expr["kind"]

        if k == "UnaryExprOrTypeTraitExpr":
            return state

        if k == "DeclStmt":
            cur_s = state
            for decl in expr.get("inner", []):
                if not isinstance(decl, dict) or decl.get("kind") != "VarDecl":
                    continue
                vid = decl.get("id")
                init_e = get_var_init_expr(decl)
                if init_e is not None:
                    cur_s = eval_expr(init_e, cur_s)
                    if vid in tracked_vars:
                        cur_s = cur_s - {vid}
                else:
                    if vid in tracked_vars:
                        cur_s = cur_s | {vid}
            return cur_s

        if k == "ImplicitCastExpr" and expr.get("castKind") == "LValueToRValue":
            sub = expr.get("inner", [None])[0]
            vid, extra_exprs = extract_lvalue_root_var(sub)
            cur_s = state
            for ex in extra_exprs:
                cur_s = eval_expr(ex, cur_s)
            if vid is not None:
                record_read_if_unsafe(vid, expr, cur_s)
            return cur_s

        if k == "BinaryOperator":
            op = expr.get("opcode")
            inner = expr.get("inner", [])
            if len(inner) != 2:
                return state
            lhs, rhs = inner[0], inner[1]

            if op == "=":
                cur_s = eval_expr(rhs, state)
                vid, extra_exprs = extract_lvalue_root_var(lhs)
                for ex in extra_exprs:
                    cur_s = eval_expr(ex, cur_s)
                if vid in tracked_vars:
                    cur_s = cur_s - {vid}
                return cur_s

            if op in COMPOUND_ASSIGN_OPS:
                cur_s = eval_expr(rhs, state)
                vid, extra_exprs = extract_lvalue_root_var(lhs)
                for ex in extra_exprs:
                    cur_s = eval_expr(ex, cur_s)
                if vid is not None:
                    record_read_if_unsafe(vid, lhs, cur_s)
                    if vid in tracked_vars:
                        cur_s = cur_s - {vid}
                return cur_s

            if op in ("&&", "||"):
                t_s, f_s = eval_cond(expr, state)
                return t_s | f_s

            cur_s = eval_expr(lhs, state)
            return eval_expr(rhs, cur_s)

        if k == "UnaryOperator":
            op = expr.get("opcode")
            inner = expr.get("inner", [])
            sub = inner[0] if inner else None
            if op in ("++", "--"):
                vid, extra_exprs = extract_lvalue_root_var(sub)
                cur_s = state
                for ex in extra_exprs:
                    cur_s = eval_expr(ex, cur_s)
                if vid is not None:
                    record_read_if_unsafe(vid, expr, cur_s)
                    if vid in tracked_vars:
                        cur_s = cur_s - {vid}
                return cur_s
            if op == "&":
                _vid, extra_exprs = extract_lvalue_root_var(sub)
                cur_s = state
                for ex in extra_exprs:
                    cur_s = eval_expr(ex, cur_s)
                return cur_s
            return eval_expr(sub, state)

        if k == "ConditionalOperator":
            inner = expr.get("inner", [])
            if len(inner) == 3:
                t_s, f_s = eval_cond(inner[0], state)
                s1 = eval_expr(inner[1], t_s)
                s2 = eval_expr(inner[2], f_s)
                return s1 | s2

        if k == "CallExpr":
            inner = expr.get("inner", [])
            if not inner:
                return state
            callee = inner[0]
            cur_s = eval_expr(callee, state)
            callee_id = extract_callee_decl_id(callee)
            addr_only_idxs = addr_only_params.get(callee_id, set())
            is_var, n_fixed = parse_callee_variadic_info(callee)
            out_written = set()
            for idx, arg in enumerate(inner[1:]):
                vid, idx_exprs = extract_addr_or_array_var(arg)
                if vid is not None:
                    for ie in idx_exprs:
                        cur_s = eval_expr(ie, cur_s)
                    if idx in addr_only_idxs:
                        continue
                    is_variadic_arg = is_var and idx >= n_fixed
                    if is_variadic_arg or is_pointer_to_const(arg.get("type")):
                        record_read_if_unsafe(vid, arg, cur_s)
                    else:
                        if vid in tracked_vars:
                            out_written.add(vid)
                else:
                    cur_s = eval_expr(arg, cur_s)
            if out_written:
                cur_s = cur_s - out_written
            return cur_s

        cur_s = state
        for ch in expr.get("inner", []):
            cur_s = eval_expr(ch, cur_s)
        return cur_s

    def eval_cond(expr, state):
        cur = unwrap_parens(expr)
        if not isinstance(cur, dict) or not cur.get("kind"):
            return (state, state)
        k = cur["kind"]
        if k == "ImplicitCastExpr" and cur.get("castKind") != "LValueToRValue":
            inner = cur.get("inner", [])
            if inner:
                return eval_cond(inner[0], state)
        if k == "UnaryOperator" and cur.get("opcode") == "!":
            inner = cur.get("inner", [])
            if inner:
                t_s, f_s = eval_cond(inner[0], state)
                return (f_s, t_s)
        if k == "BinaryOperator":
            op = cur.get("opcode")
            inner = cur.get("inner", [])
            if len(inner) == 2 and op == "&&":
                a_t, a_f = eval_cond(inner[0], state)
                b_t, b_f = eval_cond(inner[1], a_t)
                return (b_t, a_f | b_f)
            if len(inner) == 2 and op == "||":
                a_t, a_f = eval_cond(inner[0], state)
                b_t, b_f = eval_cond(inner[1], a_f)
                return (a_t | b_t, b_f)
        s = eval_expr(cur, state)
        return (s, s)

    while worklist:
        nid = worklist.pop()
        node = builder.nodes[nid]
        state = in_states[nid]
        if state is None:
            continue
        nk = node["kind"]
        if nk in ("nop", "case"):
            push_edge(node.get("succ"), state)
        elif nk == "stmt":
            out_s = eval_expr(node["ast"], state)
            push_edge(node.get("succ"), out_s)
        elif nk == "cond":
            cval = const_bool_val(node["ast"])
            t_s, f_s = eval_cond(node["ast"], state)
            if cval is True:
                push_edge(node.get("true_succ"), t_s)
            elif cval is False:
                push_edge(node.get("false_succ"), f_s)
            else:
                push_edge(node.get("true_succ"), t_s)
                push_edge(node.get("false_succ"), f_s)

    for key in sorted(violations.keys()):
        errors.append(violations[key])
    return errors


def cflags_for_file(filepath, extra_include_dirs):
    norm = filepath.replace("\\", "/")
    extra = [f"-I{d}" for d in extra_include_dirs]
    if norm.startswith("examples/") or "/examples/" in norm:
        if "ch32v307" in norm:
            return [
                "-std=c99",
                "-fsyntax-only",
                "-Xclang",
                "-ast-dump=json",
                "-I.",
                "-Isrc",
                "-Iexamples/ch32v307_demo",
                *extra,
            ]
        return [
            "-std=c99",
            "-fsyntax-only",
            "-Xclang",
            "-ast-dump=json",
            "-I.",
            "-Isrc",
            "-Isrc/port/stm32f4",
            "-Itests/unit/mocks/stm32",
            "-Itests/unit/mocks/esp32",
            "-Itests/unit/mocks/pico",
            *extra,
        ]
    return [*BASE_CFLAGS, *extra]


def iter_filtered_functions(raw_bytes):
    """Parse one or more FunctionDecl JSON objects emitted by -ast-dump-filter."""
    text = raw_bytes.decode("utf-8", errors="replace")
    decoder = json.JSONDecoder()
    idx = 0
    n = len(text)
    while idx < n:
        while idx < n and text[idx].isspace():
            idx += 1
        if idx >= n:
            break
        if text[idx] != "{":
            nl = text.find("\n", idx)
            if nl == -1:
                break
            idx = nl + 1
            continue
        obj, end_idx = decoder.raw_decode(text, idx)
        idx = end_idx
        if isinstance(obj, dict) and obj.get("kind") == "FunctionDecl":
            normalize_ast_locs(obj)
            yield obj


def check_file_worker(args):
    filepath, extra_include_dirs, expected_pts = args
    flags = cflags_for_file(filepath, extra_include_dirs)
    target_abs = os.path.abspath(filepath)

    # Fast path: filter Clang JSON AST dump to matching PT functions in this file.
    # Because addr_only_params={} is strictly more conservative than full-TU analysis,
    # 0 violations with all PT blocks accounted for is guaranteed sound; any candidate
    # violation or missing block automatically falls back to full-TU analysis below.
    filt = extract_pt_func_filter(filepath)
    if filt and expected_pts > 0:
        fast_res = subprocess.run(
            ["clang", *flags, "-Xclang", f"-ast-dump-filter={filt}", filepath],
            capture_output=True,
        )
        if fast_res.returncode != 0:
            err_txt = fast_res.stderr.decode("utf-8", errors="replace").strip()
            return [
                f"{filepath}:1:1: error: clang AST parse failed (exit {fast_res.returncode}):\n{err_txt}"
            ]
        fast_errors = []
        fast_checked = 0
        for fn in iter_filtered_functions(fast_res.stdout):
            fn_file = fn.get("loc", {}).get("file", "")
            if fn_file and os.path.abspath(fn_file) != target_abs:
                continue
            inner = fn.get("inner", [])
            body = next(
                (ch for ch in reversed(inner) if isinstance(ch, dict) and ch.get("kind") == "CompoundStmt"),
                None,
            )
            if body is None:
                continue
            fname = fn.get("name", "<unknown>")
            for pt_do, pt_sw in find_pt_blocks(body):
                fast_checked += 1
                fast_errors.extend(analyze_pt_block(filepath, fname, body, pt_do, pt_sw, {}))
        if fast_checked >= expected_pts and not fast_errors:
            return []

    res = subprocess.run(["clang", *flags, filepath], capture_output=True)
    if res.returncode != 0:
        err_txt = res.stderr.decode("utf-8", errors="replace").strip()
        return [
            f"{filepath}:1:1: error: clang AST parse failed (exit {res.returncode}):\n{err_txt}"
        ]

    tu = json.loads(res.stdout)
    normalize_ast_locs(tu)
    addr_only_params = build_addr_only_param_map(tu)

    errors = []
    for fn in tu.get("inner", []):
        if not isinstance(fn, dict) or fn.get("kind") != "FunctionDecl":
            continue
        fn_file = fn.get("loc", {}).get("file", "")
        if fn_file and os.path.abspath(fn_file) != target_abs:
            continue
        inner = fn.get("inner", [])
        body = next(
            (ch for ch in reversed(inner) if isinstance(ch, dict) and ch.get("kind") == "CompoundStmt"),
            None,
        )
        if body is None:
            continue
        fname = fn.get("name", "<unknown>")
        for pt_do, pt_sw in find_pt_blocks(body):
            errors.extend(
                analyze_pt_block(filepath, fname, body, pt_do, pt_sw, addr_only_params)
            )

    return errors


def prepare_wasm_stub_headers(stub_root):
    """Create lightweight xxd stub headers for tests/wasm/*.c so test_wasm.c parses standalone."""
    wasm_src_dir = os.path.join("tests", "wasm")
    if not os.path.isdir(wasm_src_dir):
        return
    out_dir = os.path.join(stub_root, "wasm")
    os.makedirs(out_dir, exist_ok=True)
    for fname in os.listdir(wasm_src_dir):
        if fname.endswith(".c"):
            stem = fname[:-2]
            sym = re.sub(r"[^A-Za-z0-9_]", "_", f"{stem}.wasm")
            hdr_path = os.path.join(out_dir, f"{stem}.wasm.h")
            with open(hdr_path, "w", encoding="utf-8") as f:
                f.write(
                    f"static unsigned char {sym}[] = {{ 0x00 }};\n"
                    f"static unsigned int {sym}_len = 1U;\n"
                )


def run_self_test():
    """Verify soundness on negative (must-fail) and positive (must-pass) synthetic protothreads."""
    cases = [
        (
            "reject_scalar_across_continue",
            False,
            """
            #include <stddef.h>
            #include "syntropic/pt/syn_pt.h"
            extern size_t get_len(void);
            extern void consume(size_t n);
            SYN_PT_Status bad_pt(SYN_PT *pt) {
                PT_BEGIN(pt);
                size_t klen = get_len();
                PT_CONTINUE(pt);
                consume(klen);
                PT_END(pt);
            }
            """,
        ),
        (
            "reject_prologue_clobbered_across_yield_even_with_pre_read",
            False,
            """
            #include "syntropic/pt/syn_pt.h"
            extern int step1(void);
            extern void log_rc(int rc);
            extern void consume_rc(int rc);
            SYN_PT_Status bad_prologue_pt(SYN_PT *pt) {
                int rc = 0;
                PT_BEGIN(pt);
                rc = step1();
                log_rc(rc);
                PT_CONTINUE(pt);
                if (rc != 0) {
                    consume_rc(rc);
                }
                PT_END(pt);
            }
            """,
        ),
        (
            "reject_prologue_retry_counter_reset_across_yield",
            False,
            """
            #include <stdbool.h>
            #include "syntropic/pt/syn_pt.h"
            extern bool try_send(void);
            SYN_PT_Status bad_retry_pt(SYN_PT *pt) {
                int retries = 0;
                PT_BEGIN(pt);
                while (!try_send()) {
                    if (++retries > 3) {
                        PT_EXIT(pt);
                    }
                    PT_PASS(pt);
                }
                PT_END(pt);
            }
            """,
        ),
        (
            "reject_for_loop_counter_across_yield",
            False,
            """
            #include "syntropic/pt/syn_pt.h"
            extern void work_iter(int idx);
            SYN_PT_Status bad_loop_pt(SYN_PT *pt) {
                PT_BEGIN(pt);
                for (int i = 0; i < 4; i++) {
                    work_iter(i);
                    PT_CONTINUE(pt);
                }
                PT_END(pt);
            }
            """,
        ),
        (
            "reject_stack_buffer_across_yield",
            False,
            """
            #include <stdint.h>
            #include <stddef.h>
            #include "syntropic/pt/syn_pt.h"
            extern void fill_buf(uint8_t *out);
            extern void send_buf(const uint8_t *in, size_t len);
            SYN_PT_Status bad_buf_pt(SYN_PT *pt) {
                PT_BEGIN(pt);
                uint8_t buf[16];
                fill_buf(buf);
                PT_PASS(pt);
                send_buf(buf, sizeof(buf));
                PT_END(pt);
            }
            """,
        ),
        (
            "reject_nonstatic_child_syn_pt",
            False,
            """
            #include "syntropic/pt/syn_pt.h"
            extern SYN_PT_Status child_fn(SYN_PT *pt);
            SYN_PT_Status bad_spawn_pt(SYN_PT *pt) {
                SYN_PT child;
                PT_BEGIN(pt);
                PT_SPAWN(pt, &child, child_fn(&child));
                PT_END(pt);
            }
            """,
        ),
        (
            "accept_valid_static_prologue_outparam_budget_and_intraslice",
            True,
            """
            #include <stdbool.h>
            #include <stddef.h>
            #include <stdint.h>
            #include "syntropic/pt/syn_pt.h"
            extern bool poll_rx(void *ctx, size_t *out_len);
            extern bool more_steps(void *ctx);
            extern void one_step(void *ctx);
            extern void process_rx(void *ctx, const uint8_t *hdr, size_t len);
            extern SYN_PT_Status child_fn(SYN_PT *pt);
            SYN_PT_Status good_pt(SYN_PT *pt, void *user_data) {
                void *ctx = user_data;
                uint32_t slice_ops = 0U;
                static int iter;
                static SYN_PT child;
                size_t rx_len = 0U;
                PT_BEGIN(pt);
                while (more_steps(ctx)) {
                    one_step(ctx);
                    if (++slice_ops >= 8U) {
                        PT_CONTINUE(pt);
                    }
                }
                for (iter = 0; iter < 3; iter++) {
                    while (!poll_rx(ctx, &rx_len)) {
                        PT_PASS(pt);
                    }
                    uint8_t hdr[2];
                    hdr[0] = 0xAAU;
                    hdr[1] = (uint8_t)iter;
                    process_rx(ctx, hdr, rx_len);
                    PT_SPAWN(pt, &child, child_fn(&child));
                }
                PT_END(pt);
            }
            """,
        ),
    ]

    with tempfile.TemporaryDirectory() as tmpdir:
        for name, should_pass, src in cases:
            cpath = os.path.join(tmpdir, f"{name}.c")
            with open(cpath, "w", encoding="utf-8") as f:
                f.write(src)
            errs = check_file_worker((cpath, [], 1))
            passed = len(errs) == 0
            if passed != should_pass:
                expected_str = "PASS" if should_pass else "FAIL"
                actual_str = "PASS" if passed else f"FAIL ({errs})"
                print(
                    f"Self-test '{name}' FAILED: expected {expected_str}, got {actual_str}",
                    file=sys.stderr,
                )
                return False
    print(f"Protothread Local Variable Linter Self-Test: PASS ({len(cases)}/{len(cases)} cases)")
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "targets",
        nargs="*",
        default=["src", "examples", "tests"],
        help="Directories or .c files to scan (default: src examples tests)",
    )
    parser.add_argument(
        "--self-test",
        action="store_true",
        help="Run built-in negative/positive soundness self-tests before scanning",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=os.cpu_count() or 4,
        help="Parallel worker count",
    )
    args = parser.parse_args()

    if subprocess.run(["clang", "--version"], capture_output=True).returncode != 0:
        print("ERROR: clang not found. Run via container: make container-lint", file=sys.stderr)
        sys.exit(1)

    if args.self_test and not run_self_test():
        sys.exit(1)

    c_files = []
    for target in args.targets:
        if os.path.isfile(target):
            if target.endswith(".c"):
                cnt = count_pt_begins(target)
                if cnt > 0:
                    c_files.append((target, cnt))
        elif os.path.isdir(target):
            for root, _, files in os.walk(target):
                if "port_stubs" in root.replace("\\", "/").split("/"):
                    continue
                for fname in files:
                    if fname.endswith(".c"):
                        fpath = os.path.join(root, fname)
                        cnt = count_pt_begins(fpath)
                        if cnt > 0:
                            c_files.append((fpath, cnt))
    c_files.sort(key=lambda item: item[0])

    all_errors = []
    with tempfile.TemporaryDirectory() as stub_dir:
        prepare_wasm_stub_headers(stub_dir)
        work_items = [(f, [stub_dir], cnt) for f, cnt in c_files]
        with concurrent.futures.ProcessPoolExecutor(max_workers=max(1, args.jobs)) as ex:
            for errs in ex.map(check_file_worker, work_items):
                all_errors.extend(errs)

    if all_errors:
        print("=== Protothread Local Variable Linter Errors ===")
        for err in all_errors:
            print(err)
        print(f"Total violations: {len(all_errors)}")
        sys.exit(1)

    print(
        f"Protothread Local Variable Linter: PASS (0 violations across {len(c_files)} files)"
    )
    sys.exit(0)


if __name__ == "__main__":
    main()
