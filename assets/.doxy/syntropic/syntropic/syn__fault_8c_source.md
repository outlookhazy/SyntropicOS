

# File syn\_fault.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fault.c**](syn__fault_8c.md)

[Go to the documentation of this file](syn__fault_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_FAULT) || SYN_USE_FAULT

#include "syn_fault.h"
#include "../util/syn_assert.h"
#include <string.h>

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
  #define SYN_NOINIT __attribute__((section(".bss.noinit")))
#elif defined(__GNUC__)
  #define SYN_NOINIT __attribute__((section(".noinit")))
#else
  #define SYN_NOINIT
#endif

/* Global variable mapped to uninitialized RAM segment */
static SYN_FaultDump g_fault_dump SYN_NOINIT;

void syn_fault_capture(const SYN_FaultContext *ctx)
{
    if (ctx != NULL) {
        g_fault_dump.context = *ctx;
        g_fault_dump.signature = SYN_FAULT_SIGNATURE;
    }
}

bool syn_fault_check_and_log(SYN_ErrLog *errlog)
{
    if (g_fault_dump.signature == SYN_FAULT_SIGNATURE) {
        if (errlog != NULL) {
            /* Record as SYN_ERR_FATAL with pc as context */
            syn_errlog_record(errlog, 0xFA17, SYN_ERR_FATAL, g_fault_dump.context.pc);
        }
        /* Clear signature so it doesn't log on subsequent boots */
        g_fault_dump.signature = 0;
        return true;
    }
    return false;
}

#endif /* SYN_USE_FAULT */
```


