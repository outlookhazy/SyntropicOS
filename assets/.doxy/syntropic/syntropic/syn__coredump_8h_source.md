

# File syn\_coredump.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_coredump.h**](syn__coredump_8h.md)

[Go to the documentation of this file](syn__coredump_8h.md)


```C++

#ifndef SYN_COREDUMP_H
#define SYN_COREDUMP_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(SYN_USE_COREDUMP) && SYN_USE_COREDUMP

#include "../common/syn_defs.h"
#include "syn_fault.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration defaults ─────────────────────────────────────────────── */

#ifndef SYN_COREDUMP_STACK_SIZE
  #define SYN_COREDUMP_STACK_SIZE  128  
#endif

#ifndef SYN_COREDUMP_FLASH_ADDR
  #error "SYN_COREDUMP_FLASH_ADDR must be defined to the reserved flash sector address"
#endif

/* ── Magic ──────────────────────────────────────────────────────────────── */

#define SYN_COREDUMP_MAGIC  0x43444D50u

/* ── Core dump structure ────────────────────────────────────────────────── */

typedef struct {
    uint32_t         magic;                              
    SYN_FaultContext regs;                               
    uint32_t         stack_bytes;                        
    uint8_t          stack[SYN_COREDUMP_STACK_SIZE];     
    uint32_t         uptime_ms;                          
    uint32_t         crc;                                
} SYN_CoreDump;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_coredump_save(const SYN_FaultContext *ctx);

bool syn_coredump_read(SYN_CoreDump *out);

void syn_coredump_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_COREDUMP */

#endif /* SYN_COREDUMP_H */
```


