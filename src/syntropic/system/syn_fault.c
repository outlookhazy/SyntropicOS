#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_FAULT) || SYN_USE_FAULT

/**
 * @file syn_fault.c
 * @brief CPU Hard Fault / Post-Mortem Diagnostics implementation.
 */

#include "../common/syn_compiler.h"
#include "../util/syn_assert.h"
#include "syn_fault.h"

#include <string.h>

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define SYN_NOINIT __attribute__((section(".bss.noinit")))
#elif defined(__GNUC__)
#define SYN_NOINIT __attribute__((section(".noinit")))
#else
#define SYN_NOINIT
#endif

/* Global variable mapped to uninitialized RAM segment */
/** @brief Global fault dump stored in uninitialized RAM. */
static SYN_FaultDump g_fault_dump SYN_NOINIT;

void syn_fault_capture(const SYN_FaultContext *ctx)
{
    if (ctx != NULL) {
        g_fault_dump.context = *ctx;
        SYN_COMPILER_BARRIER();
        g_fault_dump.signature = SYN_FAULT_SIGNATURE;
        SYN_COMPILER_BARRIER();
    }
}

bool syn_fault_check_and_log(SYN_ErrLog *errlog)
{
    if (g_fault_dump.signature == SYN_FAULT_SIGNATURE) {
        if (errlog != NULL) {
            /* Record as SYN_ERR_FATAL with pc as context */
            syn_errlog_record(errlog, 0xFA17U, SYN_ERR_FATAL, g_fault_dump.context.pc);
        }
        /* Clear signature so it doesn't log on subsequent boots */
        g_fault_dump.signature = 0U;
        SYN_COMPILER_BARRIER();
        return true;
    }
    return false;
}

#endif /* SYN_USE_FAULT */
