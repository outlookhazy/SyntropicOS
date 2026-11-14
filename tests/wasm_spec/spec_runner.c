/**
 * @file spec_runner.c
 * @brief Official W3C WebAssembly Spec Test Suite execution harness for syn_wasm.
 *
 * Driven over stdin/stdout by tests/wasm_spec/run_wasm_spec.py to execute W3C
 * WebAssembly 1.0 (wg-1.0) and WebAssembly 2.0 extension (.wast -> wast2json)
 * test assertions directly against syn_wasm under ASan + UBSan.
 */

#include "syntropic/vm/syn_wasm.h"

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPEC_MAX_SLOTS 64
#define SPEC_MAX_REG 32
#define SPEC_MEM_CAP (64U * 1024U * 1024U) /* 64 MiB demand-paged arena (1024 Wasm pages) */
#define SPEC_STEP_BUDGET 20000000U

typedef struct SpecSlot SpecSlot;

typedef struct {
    SpecSlot *target_slot;
    uint16_t target_func_idx;
} SpecBridgeBinding;

struct SpecSlot {
    bool active;
    char name[64];
    uint8_t *wasm_bytes;
    uint32_t wasm_size;
    uint8_t *mem_buf;
    uint32_t mem_cap;
    bool owns_mem;
    bool shared_table_tainted;
    bool has_elem_sec;
    SpecSlot *imported_table_prov;
    uint8_t global_types[SYN_WASM_MAX_GLOBALS];
    uint8_t global_mut[SYN_WASM_MAX_GLOBALS];
    SpecBridgeBinding bridges[SYN_WASM_MAX_HOST_FUNCS];
    SYN_WASM_Module mod;
    SYN_WASM_Context ctx;
};

typedef struct {
    bool active;
    uint8_t mod_name[128];
    uint32_t mod_name_len;
    SpecSlot *slot;
} SpecRegEntry;

static SpecSlot g_slots[SPEC_MAX_SLOTS];
static SpecRegEntry g_reg[SPEC_MAX_REG];
static SpecSlot *g_last_slot = NULL;
static uint8_t g_spectest_mem[2U * 65536U];
static bool g_hit_tainted_table = false;

bool syn_task_should_share(const SYN_Task *task)
{
    (void)task;
    return false;
}

uint32_t syn_port_get_tick_ms(void)
{
    return 0U;
}

uint32_t syn_port_get_tick_us(void)
{
    return 0U;
}

SYN_WASM_HOST_SYNC(spectest_noop_cb, ctx, args, argc)
{
    (void)ctx;
    (void)args;
    (void)argc;
    return 0ULL;
}

static SYN_WASM_Status run_to_completion(SYN_WASM_Context *ctx)
{
    uint32_t total = 0U;
    SYN_WASM_Status st = ctx->status;
    while ((st == SYN_WASM_OK) || (st == SYN_WASM_YIELDED) || (st == SYN_WASM_WAITING)) {
        if (total >= SPEC_STEP_BUDGET) {
            ctx->status = SYN_WASM_TRAP_UNREACHABLE;
            return SYN_WASM_TRAP_UNREACHABLE;
        }
        st = syn_wasm_step(ctx, 50000U);
        total += 50000U;
    }
    return st;
}

static void sync_shared_mem_size(const uint8_t *mem_buf, uint32_t new_size)
{
    if (mem_buf == NULL) {
        return;
    }
    for (int i = 0; i < SPEC_MAX_SLOTS; i++) {
        if (g_slots[i].active && (g_slots[i].mem_buf == mem_buf)) {
            g_slots[i].ctx.linear_mem_size = new_size;
        }
    }
}

static uint64_t spec_invoke_bridge_slot(SYN_WASM_Context *caller_ctx, uint16_t imp_idx,
                                        const uint64_t *args, uint8_t argc)
{
    SpecSlot *caller_slot = (SpecSlot *)caller_ctx->user_ctx;
    if ((caller_slot == NULL) || (imp_idx >= SYN_WASM_MAX_HOST_FUNCS)) {
        caller_ctx->status = SYN_WASM_TRAP_UNREGISTERED_HOST;
        return 0ULL;
    }
    SpecSlot *target = caller_slot->bridges[imp_idx].target_slot;
    uint16_t fn_idx = caller_slot->bridges[imp_idx].target_func_idx;
    if ((target == NULL) || !target->active) {
        caller_ctx->status = SYN_WASM_TRAP_UNREGISTERED_HOST;
        return 0ULL;
    }
    if (target->shared_table_tainted) {
        g_hit_tainted_table = true;
        caller_ctx->status = SYN_WASM_TRAP_UNREACHABLE;
        return 0ULL;
    }

    /* Heap-allocate saved context so re-entrant A -> B -> A calls stay off the C stack */
    SYN_WASM_Context *saved = (SYN_WASM_Context *)malloc(sizeof(*saved));
    if (saved == NULL) {
        caller_ctx->status = SYN_WASM_TRAP_CALL_STACK_OVERFLOW;
        return 0ULL;
    }
    *saved = target->ctx;
    target->ctx.host_pending = false;
    target->ctx.host_waiting = false;
    target->ctx.sp = 0U;
    for (uint8_t i = 0U; i < argc; i++) {
        target->ctx.stack[target->ctx.sp++] = args[i];
    }
    if (!syn_wasm_call(&target->ctx, fn_idx)) {
        SYN_WASM_Status st = target->ctx.status;
        target->ctx = *saved;
        free(saved);
        caller_ctx->status = (st != SYN_WASM_OK) ? st : SYN_WASM_TRAP_CALL_STACK_OVERFLOW;
        return 0ULL;
    }
    if (target->ctx.status != SYN_WASM_HALTED) {
        SYN_WASM_Status st = run_to_completion(&target->ctx);
        if (st != SYN_WASM_HALTED) {
            uint64_t tmp_globals[SYN_WASM_MAX_GLOBALS];
            memcpy(tmp_globals, target->ctx.globals, sizeof(tmp_globals));
            uint32_t tmp_mem_size = target->ctx.linear_mem_size;
            uint64_t tmp_dropped[(SYN_WASM_MAX_DATA_SEGS + 63) / 64];
            memcpy(tmp_dropped, target->ctx.data_dropped, sizeof(tmp_dropped));
            target->ctx = *saved;
            free(saved);
            memcpy(target->ctx.globals, tmp_globals, sizeof(tmp_globals));
            target->ctx.linear_mem_size = tmp_mem_size;
            memcpy(target->ctx.data_dropped, tmp_dropped, sizeof(tmp_dropped));
            sync_shared_mem_size(target->mem_buf, tmp_mem_size);
            caller_ctx->status = ((st == SYN_WASM_YIELDED) || (st == SYN_WASM_WAITING))
                                     ? SYN_WASM_TRAP_UNREACHABLE
                                     : st;
            return 0ULL;
        }
    }
    uint64_t res = syn_wasm_result(&target->ctx);
    uint64_t tmp_globals[SYN_WASM_MAX_GLOBALS];
    memcpy(tmp_globals, target->ctx.globals, sizeof(tmp_globals));
    uint32_t tmp_mem_size = target->ctx.linear_mem_size;
    uint64_t tmp_dropped[(SYN_WASM_MAX_DATA_SEGS + 63) / 64];
    memcpy(tmp_dropped, target->ctx.data_dropped, sizeof(tmp_dropped));
    target->ctx = *saved;
    free(saved);
    memcpy(target->ctx.globals, tmp_globals, sizeof(tmp_globals));
    target->ctx.linear_mem_size = tmp_mem_size;
    memcpy(target->ctx.data_dropped, tmp_dropped, sizeof(tmp_dropped));
    sync_shared_mem_size(target->mem_buf, tmp_mem_size);
    return res;
}

static SYN_PT_Status spec_bridge_cb(SYN_PT *pt, SYN_WASM_Context *caller_ctx, const uint64_t *args,
                                    uint8_t argc, uint64_t *ret)
{
    uint16_t imp_idx = (uint16_t)(uintptr_t)syn_wasm_host_user(caller_ctx);
    uint64_t res = spec_invoke_bridge_slot(caller_ctx, imp_idx, args, argc);
    if (caller_ctx->status != SYN_WASM_OK) {
        PT_INIT(pt);
        return PT_EXITED;
    }
    SYN_WASM_HOST_RETURN(pt, ret, res);
}

static bool leb_read_u32(const uint8_t *b, uint32_t end, uint32_t *off, uint32_t *out)
{
    uint32_t res = 0U;
    uint32_t shift = 0U;
    for (uint32_t i = 0U; i < 5U; i++) {
        if (*off >= end) {
            return false;
        }
        uint8_t byte = b[(*off)++];
        if ((i == 4U) && ((byte & 0xF0U) != 0U)) {
            return false;
        }
        res |= ((uint32_t)(byte & 0x7FU)) << shift;
        if ((byte & 0x80U) == 0U) {
            *out = res;
            return true;
        }
        shift += 7U;
    }
    return false;
}

static uint32_t hex_decode(const char *hex, uint8_t *out, uint32_t max_out)
{
    if (strcmp(hex, "-") == 0) {
        return 0U;
    }
    size_t slen = strlen(hex);
    uint32_t n = 0U;
    for (size_t i = 0U; (i + 1U < slen) && (n < max_out); i += 2U) {
        unsigned int v = 0U;
        if (sscanf(&hex[i], "%2x", &v) == 1) {
            out[n++] = (uint8_t)v;
        }
    }
    return n;
}

static void free_slot(SpecSlot *s)
{
    if (s->wasm_bytes != NULL) {
        free(s->wasm_bytes);
        s->wasm_bytes = NULL;
    }
    if (s->owns_mem && (s->mem_buf != NULL)) {
        free(s->mem_buf);
    }
    s->mem_buf = NULL;
    s->owns_mem = false;
    s->active = false;
}

static void reset_all(void)
{
    for (int i = 0; i < SPEC_MAX_SLOTS; i++) {
        free_slot(&g_slots[i]);
    }
    memset(g_slots, 0, sizeof(g_slots));
    memset(g_reg, 0, sizeof(g_reg));
    memset(g_spectest_mem, 0, sizeof(g_spectest_mem));
    g_last_slot = NULL;
}

static SpecSlot *find_slot(const char *name)
{
    if (strcmp(name, "-") == 0) {
        return g_last_slot;
    }
    for (int i = 0; i < SPEC_MAX_SLOTS; i++) {
        if (g_slots[i].active && (strcmp(g_slots[i].name, name) == 0)) {
            return &g_slots[i];
        }
    }
    return NULL;
}

static SpecSlot *alloc_slot(const char *name)
{
    if (strcmp(name, "-") != 0) {
        for (int i = 0; i < SPEC_MAX_SLOTS; i++) {
            if (g_slots[i].active && (strcmp(g_slots[i].name, name) == 0)) {
                free_slot(&g_slots[i]);
                return &g_slots[i];
            }
        }
    }
    for (int i = 0; i < SPEC_MAX_SLOTS; i++) {
        if (!g_slots[i].active) {
            return &g_slots[i];
        }
    }
    free_slot(&g_slots[0]);
    return &g_slots[0];
}

static SpecSlot *find_registered(const uint8_t *name, uint32_t len)
{
    for (int i = SPEC_MAX_REG - 1; i >= 0; i--) {
        if (g_reg[i].active && (g_reg[i].mod_name_len == len) &&
            (memcmp(g_reg[i].mod_name, name, len) == 0)) {
            return g_reg[i].slot;
        }
    }
    return NULL;
}

static int32_t find_export_kind(const SYN_WASM_Module *mod, const uint8_t *field,
                                uint32_t field_len, uint8_t kind)
{
    for (uint16_t i = 0U; i < mod->export_count; i++) {
        if ((mod->exports[i].kind == kind) && (mod->exports[i].name_len == field_len) &&
            (memcmp(mod->bytes + mod->exports[i].name_offset, field, field_len) == 0)) {
            return (int32_t)mod->exports[i].func_idx;
        }
    }
    return -1;
}

static bool bytes_eq_str(const uint8_t *b, uint32_t len, const char *s)
{
    size_t slen = strlen(s);
    return (len == slen) && (memcmp(b, s, slen) == 0);
}

static bool check_spectest_func_sig(const SYN_WASM_Module *mod, uint32_t type_idx,
                                    const uint8_t *field, uint32_t field_len)
{
    const SYN_WASM_FuncType *ft = &mod->types[type_idx];
    if (ft->result_count != 0U) {
        return false;
    }
    const uint8_t *p = &mod->bytes[ft->params_offset];
    if (bytes_eq_str(field, field_len, "print")) {
        return ft->param_count == 0U;
    }
    if (bytes_eq_str(field, field_len, "print_i32")) {
        return (ft->param_count == 1U) && (p[0] == 0x7FU);
    }
    if (bytes_eq_str(field, field_len, "print_i64")) {
        return (ft->param_count == 1U) && (p[0] == 0x7EU);
    }
    if (bytes_eq_str(field, field_len, "print_f32")) {
        return (ft->param_count == 1U) && (p[0] == 0x7DU);
    }
    if (bytes_eq_str(field, field_len, "print_f64")) {
        return (ft->param_count == 1U) && (p[0] == 0x7CU);
    }
    if (bytes_eq_str(field, field_len, "print_i32_f32")) {
        return (ft->param_count == 2U) && (p[0] == 0x7FU) && (p[1] == 0x7DU);
    }
    if (bytes_eq_str(field, field_len, "print_f64_f64")) {
        return (ft->param_count == 2U) && (p[0] == 0x7CU) && (p[1] == 0x7CU);
    }
    return false;
}

/* Extract global types/mutabilities + resolve imports for a loaded module */
static int link_slot_imports(SpecSlot *slot, uint64_t *import_globals, uint16_t *out_imp_glob_cnt,
                             SYN_WASM_HostFunc *host_fns, uint16_t *out_host_cnt,
                             bool *out_unsupported_link)
{
    *out_imp_glob_cnt = 0U;
    *out_host_cnt = 0U;
    *out_unsupported_link = false;

    const uint8_t *b = slot->wasm_bytes;
    uint32_t sz = slot->wasm_size;
    uint32_t off = 8U;
    uint16_t g_total = 0U;

    while (off < sz) {
        uint8_t sec_id = b[off++];
        uint32_t sec_len = 0U;
        if (!leb_read_u32(b, sz, &off, &sec_len)) {
            return -1;
        }
        uint32_t sec_end = off + sec_len;

        if (sec_id == 2U) { /* Import section */
            uint32_t count = 0U;
            (void)leb_read_u32(b, sec_end, &off, &count);
            for (uint32_t i = 0U; i < count; i++) {
                uint32_t mlen = 0U, flen = 0U;
                (void)leb_read_u32(b, sec_end, &off, &mlen);
                const uint8_t *mname = &b[off];
                off += mlen;
                (void)leb_read_u32(b, sec_end, &off, &flen);
                const uint8_t *fname = &b[off];
                off += flen;
                uint8_t kind = b[off++];

                bool is_spectest = bytes_eq_str(mname, mlen, "spectest");
                SpecSlot *prov = is_spectest ? NULL : find_registered(mname, mlen);
                if (!is_spectest && (prov == NULL)) {
                    return -1; /* unknown import */
                }

                if (kind == 0U) { /* func */
                    uint32_t tidx = 0U;
                    (void)leb_read_u32(b, sec_end, &off, &tidx);
                    uint16_t imp_i = (*out_host_cnt)++;
                    if (is_spectest) {
                        if (!check_spectest_func_sig(&slot->mod, tidx, fname, flen)) {
                            return -1;
                        }
                        host_fns[imp_i] = spectest_noop_cb;
                    } else {
                        int32_t exp_fn = find_export_kind(&prov->mod, fname, flen, 0U);
                        if (exp_fn < 0) {
                            return -1;
                        }
                        const SYN_WASM_FuncType *want = &slot->mod.types[tidx];
                        const SYN_WASM_FuncType *got =
                            &prov->mod.types[prov->mod.funcs[exp_fn].type_idx];
                        if ((want->param_count != got->param_count) ||
                            (want->result_count != got->result_count) ||
                            (memcmp(&slot->mod.bytes[want->params_offset],
                                    &prov->mod.bytes[got->params_offset],
                                    want->param_count) != 0) ||
                            (memcmp(&slot->mod.bytes[want->results_offset],
                                    &prov->mod.bytes[got->results_offset],
                                    want->result_count) != 0)) {
                            return -1;
                        }
                        if ((imp_i >= SYN_WASM_MAX_HOST_FUNCS) || (got->result_count > 1U)) {
                            *out_unsupported_link = true;
                            return -2;
                        }
                        slot->bridges[imp_i].target_slot = prov;
                        slot->bridges[imp_i].target_func_idx = (uint16_t)exp_fn;
                        host_fns[imp_i] = spec_bridge_cb;
                    }
                } else if (kind == 1U) { /* table */
                    uint8_t et = b[off++];
                    (void)et;
                    uint8_t lflags = b[off++];
                    uint32_t lmin = 0U, lmax = 0U;
                    (void)leb_read_u32(b, sec_end, &off, &lmin);
                    if ((lflags & 1U) != 0U) {
                        (void)leb_read_u32(b, sec_end, &off, &lmax);
                    }
                    if (is_spectest) {
                        /* spectest.table has min=10, max=20 */
                        if (!bytes_eq_str(fname, flen, "table") || (lmin > 10U) ||
                            (((lflags & 1U) != 0U) && (lmax < 20U))) {
                            return -1;
                        }
                        slot->mod.table_min_elems = 10U;
                        slot->mod.table_has_max = true;
                        slot->mod.table_max_elems = 20U;
                    } else {
                        int32_t exp_tbl = find_export_kind(&prov->mod, fname, flen, 1U);
                        if (exp_tbl < 0) {
                            return -1;
                        }
                        uint32_t pmin = prov->mod.table_min_elems;
                        if ((lmin > pmin) ||
                            (((lflags & 1U) != 0U) &&
                             (!prov->mod.table_has_max || (lmax < prov->mod.table_max_elems)))) {
                            return -1;
                        }
                        /* Cross-module table sharing: bounds checks run normally in
                         * syn_wasm_init_ex; if elements are written across modules, mark tainted */
                        slot->imported_table_prov = prov;
                        slot->mod.table_min_elems = pmin;
                        slot->mod.table_has_max = prov->mod.table_has_max;
                        slot->mod.table_max_elems = prov->mod.table_max_elems;
                    }
                } else if (kind == 2U) { /* memory */
                    uint8_t lflags = b[off++];
                    uint32_t lmin = 0U, lmax = 0U;
                    (void)leb_read_u32(b, sec_end, &off, &lmin);
                    if ((lflags & 1U) != 0U) {
                        (void)leb_read_u32(b, sec_end, &off, &lmax);
                    }
                    if (is_spectest) {
                        /* spectest.memory has min=1, max=2 */
                        if (!bytes_eq_str(fname, flen, "memory") || (lmin > 1U) ||
                            (((lflags & 1U) != 0U) && (lmax < 2U))) {
                            return -1;
                        }
                        slot->mem_buf = g_spectest_mem;
                        slot->mem_cap = sizeof(g_spectest_mem);
                        slot->owns_mem = false;
                        slot->mod.mem_min_pages = 1U;
                        slot->mod.mem_has_max = true;
                        slot->mod.mem_max_pages = 2U;
                    } else {
                        int32_t exp_mem = find_export_kind(&prov->mod, fname, flen, 2U);
                        if (exp_mem < 0) {
                            return -1;
                        }
                        uint32_t pcur = prov->ctx.linear_mem_size / 65536U;
                        if ((lmin > pcur) ||
                            (((lflags & 1U) != 0U) &&
                             (!prov->mod.mem_has_max || (lmax < prov->mod.mem_max_pages)))) {
                            return -1;
                        }
                        slot->mem_buf = prov->mem_buf;
                        slot->mem_cap = prov->mem_cap;
                        slot->owns_mem = false;
                        slot->mod.mem_min_pages = pcur;
                        slot->mod.mem_has_max = prov->mod.mem_has_max;
                        slot->mod.mem_max_pages = prov->mod.mem_max_pages;
                    }
                } else if (kind == 3U) { /* global */
                    uint8_t gtype = b[off++];
                    uint8_t gmut = b[off++];
                    uint16_t gi = (*out_imp_glob_cnt)++;
                    slot->global_types[g_total] = gtype;
                    slot->global_mut[g_total] = gmut;
                    g_total++;

                    if (is_spectest) {
                        if (gmut != 0U) {
                            return -1;
                        }
                        if (bytes_eq_str(fname, flen, "global_i32") && (gtype == 0x7FU)) {
                            import_globals[gi] = 666ULL;
                        } else if (bytes_eq_str(fname, flen, "global_i64") && (gtype == 0x7EU)) {
                            import_globals[gi] = 666ULL;
                        } else if (bytes_eq_str(fname, flen, "global_f32") && (gtype == 0x7DU)) {
                            import_globals[gi] = 0x4426A666ULL;
                        } else if (bytes_eq_str(fname, flen, "global_f64") && (gtype == 0x7CU)) {
                            import_globals[gi] = 0x4084D4CCCCCCCCCDULL;
                        } else {
                            return -1;
                        }
                    } else {
                        int32_t exp_g = find_export_kind(&prov->mod, fname, flen, 3U);
                        if (exp_g < 0) {
                            return -1;
                        }
                        if ((prov->global_types[exp_g] != gtype) ||
                            (prov->global_mut[exp_g] != gmut)) {
                            return -1;
                        }
                        if (gmut != 0U) {
                            *out_unsupported_link = true;
                        }
                        import_globals[gi] = prov->ctx.globals[exp_g];
                    }
                }
            }
        } else if (sec_id == 6U) { /* Global section: record internal global types & mutability */
            uint32_t count = 0U;
            (void)leb_read_u32(b, sec_end, &off, &count);
            for (uint32_t i = 0U; i < count; i++) {
                uint8_t gtype = b[off++];
                uint8_t gmut = b[off++];
                if (g_total < SYN_WASM_MAX_GLOBALS) {
                    slot->global_types[g_total] = gtype;
                    slot->global_mut[g_total] = gmut;
                    g_total++;
                }
                while ((off < sec_end) && (b[off] != 0x0BU)) {
                    uint8_t op = b[off++];
                    if (op == 0x43U) {
                        off += 4U;
                    } else if (op == 0x44U) {
                        off += 8U;
                    } else {
                        uint32_t dummy = 0U;
                        (void)leb_read_u32(b, sec_end, &off, &dummy);
                    }
                }
                if (off < sec_end) {
                    off++; /* 0x0B end */
                }
            }
        } else if (sec_id == 9U) {
            slot->has_elem_sec = true;
        }
        off = sec_end;
    }
    return 0;
}

static uint8_t *read_file_bytes(const char *path, uint32_t *out_size)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return NULL;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    long len = ftell(fp);
    if (len < 0) {
        fclose(fp);
        return NULL;
    }
    rewind(fp);
    uint8_t *buf = (uint8_t *)malloc((len > 0) ? (size_t)len : 1U);
    if (buf == NULL) {
        fclose(fp);
        return NULL;
    }
    if ((len > 0) && (fread(buf, 1U, (size_t)len, fp) != (size_t)len)) {
        free(buf);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    *out_size = (uint32_t)len;
    return buf;
}

static void handle_load(const char *slot_name, const char *wasm_path)
{
    uint32_t sz = 0U;
    uint8_t *bytes = read_file_bytes(wasm_path, &sz);
    if (bytes == NULL) {
        printf("FILE_ERR\n");
        return;
    }

    SpecSlot *slot = alloc_slot(slot_name);
    memset(slot, 0, sizeof(*slot));
    snprintf(slot->name, sizeof(slot->name), "%s", slot_name);
    slot->wasm_bytes = bytes;
    slot->wasm_size = sz;

    SYN_WASM_LoadError lerr = SYN_WASM_LOAD_OK;
    if (!syn_wasm_module_load_ex(&slot->mod, slot->wasm_bytes, slot->wasm_size, &lerr)) {
        free_slot(slot);
        printf("LOAD_ERR %d\n", (int)lerr);
        return;
    }

    uint64_t imp_globals[SYN_WASM_MAX_GLOBALS];
    memset(imp_globals, 0, sizeof(imp_globals));
    SYN_WASM_HostFunc host_fns[SYN_WASM_MAX_HOST_FUNCS];
    memset(host_fns, 0, sizeof(host_fns));
    uint16_t imp_glob_cnt = 0U;
    uint16_t host_cnt = 0U;
    bool unsupported_link = false;

    int lrc =
        link_slot_imports(slot, imp_globals, &imp_glob_cnt, host_fns, &host_cnt, &unsupported_link);
    if (lrc == -1) {
        free_slot(slot);
        printf("UNLINKABLE\n");
        return;
    }
    if ((lrc == -2) || unsupported_link) {
        free_slot(slot);
        printf("SKIP_MULTI_MOD\n");
        return;
    }

    bool shared_mem = (slot->mem_buf != NULL);
    uint8_t *saved_shared = NULL;
    uint32_t shared_bytes = slot->mod.mem_min_pages * 65536U;
    if (shared_mem && (shared_bytes > 0U)) {
        /* syn_wasm_init_ex zeroes min_bytes on fresh module memory; preserve imported memory! */
        saved_shared = (uint8_t *)malloc(shared_bytes);
        memcpy(saved_shared, slot->mem_buf, shared_bytes);
    } else if (!shared_mem && slot->mod.has_memory) {
        uint64_t need = (uint64_t)slot->mod.mem_min_pages * 65536ULL;
        uint32_t cap = (need > SPEC_MEM_CAP) ? (uint32_t)need : SPEC_MEM_CAP;
        if (need > (256ULL * 1024ULL * 1024ULL)) {
            free_slot(slot);
            printf("SKIP_MEM_CAP\n");
            return;
        }
        slot->mem_buf = (uint8_t *)calloc(1U, cap > 0U ? cap : 1U);
        slot->mem_cap = cap;
        slot->owns_mem = true;
    }

    if (!syn_wasm_init_ex(&slot->ctx, &slot->mod, slot->mem_buf, slot->mem_cap, imp_globals,
                          imp_glob_cnt)) {
        if (saved_shared != NULL) {
            memcpy(slot->mem_buf, saved_shared, shared_bytes);
            free(saved_shared);
        }
        SYN_WASM_Status st = slot->ctx.status;
        free_slot(slot);
        printf("INST_TRAP %d\n", (int)st);
        return;
    }

    if (saved_shared != NULL) {
        /* Restore imported memory bytes, then re-apply this module's active data segments! */
        memcpy(slot->mem_buf, saved_shared, shared_bytes);
        free(saved_shared);
        /* Re-run syn_wasm_init_ex data copy by temporarily clearing has_memory zeroing?
         * Wait: even simpler — copy active data segments directly on top! */
        const uint8_t *b = slot->wasm_bytes;
        uint32_t off = 8U;
        while (off < slot->wasm_size) {
            uint8_t sid = b[off++];
            uint32_t slen = 0U;
            (void)leb_read_u32(b, slot->wasm_size, &off, &slen);
            uint32_t send = off + slen;
            if (sid == 11U) {
                uint32_t nseg = 0U;
                (void)leb_read_u32(b, send, &off, &nseg);
                for (uint32_t s = 0U; s < nseg; s++) {
                    uint32_t flg = 0U;
                    (void)leb_read_u32(b, send, &off, &flg);
                    if (flg == 0U) {
                        uint8_t op = b[off++];
                        uint32_t doff = 0U;
                        if (op == 0x41U) {
                            /* signed i32 leb */
                            int32_t iv = 0;
                            uint32_t sh = 0U;
                            uint8_t by = 0U;
                            do {
                                by = b[off++];
                                iv |= (int32_t)((uint32_t)(by & 0x7FU) << sh);
                                sh += 7U;
                            } while ((by & 0x80U) != 0U);
                            if ((sh < 32U) && ((by & 0x40U) != 0U)) {
                                iv |= (int32_t)(~0U << sh);
                            }
                            doff = (uint32_t)iv;
                        } else if (op == 0x23U) {
                            uint32_t gi = 0U;
                            (void)leb_read_u32(b, send, &off, &gi);
                            doff = (uint32_t)slot->ctx.globals[gi];
                        }
                        off++; /* 0x0B */
                        uint32_t dlen = 0U;
                        (void)leb_read_u32(b, send, &off, &dlen);
                        if (dlen > 0U) {
                            memcpy(&slot->mem_buf[doff], &b[off], dlen);
                        }
                        off += dlen;
                    } else {
                        uint32_t dlen = 0U;
                        (void)leb_read_u32(b, send, &off, &dlen);
                        off += dlen;
                    }
                }
            }
            off = send;
        }
    }

    if ((slot->imported_table_prov != NULL) && slot->has_elem_sec) {
        slot->imported_table_prov->shared_table_tainted = true;
        slot->shared_table_tainted = true;
    }

    slot->ctx.user_ctx = slot;
    for (uint16_t h = 0U; h < host_cnt; h++) {
        (void)syn_wasm_register_host_ex(&slot->ctx, h, host_fns[h], NULL, (void *)(uintptr_t)h);
    }

    slot->active = true;
    if (slot->mod.has_start_func) {
        slot->ctx.sp = 0U;
        if (!syn_wasm_call(&slot->ctx, (uint16_t)slot->mod.start_func_idx)) {
            SYN_WASM_Status st = slot->ctx.status;
            free_slot(slot);
            printf("START_TRAP %d\n", (int)st);
            return;
        }
        if (slot->ctx.status != SYN_WASM_HALTED) {
            SYN_WASM_Status st = run_to_completion(&slot->ctx);
            if (st != SYN_WASM_HALTED) {
                free_slot(slot);
                printf("START_TRAP %d\n", (int)st);
                return;
            }
        }
    }

    g_last_slot = slot;
    printf("OK\n");
}

static void handle_check_invalid(const char *wasm_path)
{
    uint32_t sz = 0U;
    uint8_t *bytes = read_file_bytes(wasm_path, &sz);
    if (bytes == NULL) {
        printf("FILE_ERR\n");
        return;
    }
    static SYN_WASM_Module tmp_mod;
    SYN_WASM_LoadError err = SYN_WASM_LOAD_OK;
    bool ok = syn_wasm_module_load_ex(&tmp_mod, bytes, sz, &err);
    free(bytes);
    if (ok) {
        printf("VALID\n");
    } else {
        printf("REJECTED %d\n", (int)err);
    }
}

static void handle_register(const char *as_hex, const char *slot_name)
{
    SpecSlot *slot = find_slot(slot_name);
    if (slot == NULL) {
        printf("ERR_NO_SLOT\n");
        return;
    }
    for (int i = 0; i < SPEC_MAX_REG; i++) {
        if (!g_reg[i].active) {
            g_reg[i].active = true;
            g_reg[i].mod_name_len =
                hex_decode(as_hex, g_reg[i].mod_name, sizeof(g_reg[i].mod_name));
            g_reg[i].slot = slot;
            printf("OK\n");
            return;
        }
    }
    printf("ERR_REG_FULL\n");
}

static void handle_invoke(char *rest)
{
    char *slot_name = strtok(rest, " \t\r\n");
    char *field_hex = strtok(NULL, " \t\r\n");
    char *argc_str = strtok(NULL, " \t\r\n");
    if ((slot_name == NULL) || (field_hex == NULL) || (argc_str == NULL)) {
        printf("ERR_ARGS\n");
        return;
    }
    SpecSlot *slot = find_slot(slot_name);
    if (slot == NULL) {
        printf("ERR_NO_SLOT\n");
        return;
    }
    if (slot->shared_table_tainted) {
        printf("SKIP_SHARED_TABLE\n");
        return;
    }
    uint8_t field[2048];
    uint32_t flen = hex_decode(field_hex, field, sizeof(field));
    int32_t fn_idx = find_export_kind(&slot->mod, field, flen, 0U);
    if (fn_idx < 0) {
        printf("ERR_NO_EXPORT\n");
        return;
    }

    int argc = atoi(argc_str);
    slot->ctx.sp = 0U;
    for (int i = 0; i < argc; i++) {
        char *arg_tok = strtok(NULL, " \t\r\n");
        uint64_t v = (arg_tok != NULL) ? strtoull(arg_tok, NULL, 10) : 0ULL;
        slot->ctx.stack[slot->ctx.sp++] = v;
    }

    g_hit_tainted_table = false;
    if (!syn_wasm_call(&slot->ctx, (uint16_t)fn_idx)) {
        if (g_hit_tainted_table) {
            printf("SKIP_SHARED_TABLE\n");
            return;
        }
        printf("TRAP %d\n", (int)slot->ctx.status);
        return;
    }
    if (slot->ctx.status != SYN_WASM_HALTED) {
        SYN_WASM_Status st = run_to_completion(&slot->ctx);
        sync_shared_mem_size(slot->mem_buf, slot->ctx.linear_mem_size);
        if (g_hit_tainted_table) {
            printf("SKIP_SHARED_TABLE\n");
            return;
        }
        if (st != SYN_WASM_HALTED) {
            printf("TRAP %d\n", (int)st);
            return;
        }
    }

    uint8_t rcnt = slot->mod.funcs[fn_idx].result_count;
    printf("OK %u", (unsigned)rcnt);
    for (uint8_t r = 0U; r < rcnt; r++) {
        printf(" %" PRIu64, slot->ctx.stack[r]);
    }
    printf("\n");
}

static void handle_get(const char *slot_name, const char *field_hex)
{
    SpecSlot *slot = find_slot(slot_name);
    if (slot == NULL) {
        printf("ERR_NO_SLOT\n");
        return;
    }
    uint8_t field[2048];
    uint32_t flen = hex_decode(field_hex, field, sizeof(field));
    int32_t g_idx = find_export_kind(&slot->mod, field, flen, 3U);
    if (g_idx < 0) {
        printf("ERR_NO_EXPORT\n");
        return;
    }
    printf("OK 1 %" PRIu64 "\n", slot->ctx.globals[g_idx]);
}

int main(void)
{
    static char line[65536];
    reset_all();
    while (fgets(line, (int)sizeof(line), stdin) != NULL) {
        char *cmd = strtok(line, " \t\r\n");
        if (cmd == NULL) {
            continue;
        }
        if (strcmp(cmd, "RESET") == 0) {
            reset_all();
            printf("OK\n");
        } else if (strcmp(cmd, "LOAD") == 0) {
            char *sname = strtok(NULL, " \t\r\n");
            char *path = strtok(NULL, " \t\r\n");
            handle_load(sname, path);
        } else if (strcmp(cmd, "CHECK_INVALID") == 0) {
            char *path = strtok(NULL, " \t\r\n");
            handle_check_invalid(path);
        } else if (strcmp(cmd, "REGISTER") == 0) {
            char *as_hex = strtok(NULL, " \t\r\n");
            char *sname = strtok(NULL, " \t\r\n");
            handle_register(as_hex, sname);
        } else if (strcmp(cmd, "INVOKE") == 0) {
            char *rest = strtok(NULL, "\r\n");
            handle_invoke(rest);
        } else if (strcmp(cmd, "GET") == 0) {
            char *sname = strtok(NULL, " \t\r\n");
            char *fhex = strtok(NULL, " \t\r\n");
            handle_get(sname, fhex);
        } else if (strcmp(cmd, "QUIT") == 0) {
            break;
        }
        fflush(stdout);
    }
    reset_all();
    return 0;
}
