#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(SYN_USE_COREDUMP) && SYN_USE_COREDUMP

/**
 * @file syn_coredump.c
 * @brief Persistent core dump implementation.
 *
 * Writes fault context + partial stack to a reserved flash sector.
 * Designed to be callable from a hard fault handler (no heap, no locks,
 * minimal stack usage).
 */

#include "syn_coredump.h"
#include "../util/syn_crc.h"
#include "../util/syn_assert.h"
#include "../port/syn_port_flash.h"
#include "../port/syn_port_system.h"

#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Compute CRC-32 over all fields preceding the crc member.
 * @param dump  Core dump to checksum.
 * @return CRC-32 value.
 */
static uint32_t coredump_compute_crc(const SYN_CoreDump *dump)
{
    /* CRC covers everything from magic through stack[] and uptime_ms,
     * i.e. everything before the crc field. */
    size_t len = offsetof(SYN_CoreDump, crc);
    return syn_crc32(dump, len);
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_coredump_save(const SYN_FaultContext *ctx)
{
    if (ctx == NULL) return;

    SYN_CoreDump dump;
    memset(&dump, 0, sizeof(dump));

    dump.magic = SYN_COREDUMP_MAGIC;
    dump.regs  = *ctx;

    /* Capture partial stack from SP upward.
     * On 32-bit MCU targets, ctx->sp is a valid stack pointer.
     * On 64-bit hosts (test builds), the uint32_t SP truncates the
     * pointer, so we skip the copy unless explicitly enabled. */
/** @cond INTERNAL */
#if !defined(SYN_COREDUMP_CAPTURE_STACK)
  #if UINTPTR_MAX == 0xFFFFFFFFu
    #define SYN_COREDUMP_CAPTURE_STACK 1
  #else
    #define SYN_COREDUMP_CAPTURE_STACK 0
  #endif
#endif
/** @endcond */

#if SYN_COREDUMP_CAPTURE_STACK
    {
        const uint8_t *sp = (const uint8_t *)(uintptr_t)ctx->sp;
        if (sp != NULL) {
            size_t to_copy = SYN_COREDUMP_STACK_SIZE;
            dump.stack_bytes = (uint32_t)to_copy;
            memcpy(dump.stack, sp, to_copy);
        } else {
            dump.stack_bytes = 0;
        }
    }
#else
    dump.stack_bytes = 0;
#endif

    dump.uptime_ms = syn_port_get_tick_ms();
    dump.crc = coredump_compute_crc(&dump);

    /* Write to flash — best-effort, ignore errors in fault context */
    syn_port_flash_erase(SYN_COREDUMP_FLASH_ADDR);
    syn_port_flash_write(SYN_COREDUMP_FLASH_ADDR, &dump, sizeof(dump));
}

bool syn_coredump_read(SYN_CoreDump *out)
{
    SYN_ASSERT(out != NULL);

    SYN_CoreDump dump;
    SYN_Status st = syn_port_flash_read(SYN_COREDUMP_FLASH_ADDR,
                                          &dump, sizeof(dump));
    if (st != SYN_OK) return false;

    /* Validate */
    if (dump.magic != SYN_COREDUMP_MAGIC) return false;

    uint32_t expected_crc = coredump_compute_crc(&dump);
    if (dump.crc != expected_crc) return false;

    *out = dump;
    return true;
}

void syn_coredump_clear(void)
{
    syn_port_flash_erase(SYN_COREDUMP_FLASH_ADDR);
}

#endif /* SYN_USE_COREDUMP */

/* Avoid empty translation unit when feature is disabled (-pedantic) */
/** @cond INTERNAL */
typedef int syn_coredump_unused_t;
/** @endcond */
