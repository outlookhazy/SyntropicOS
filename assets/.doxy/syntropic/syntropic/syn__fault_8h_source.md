

# File syn\_fault.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fault.h**](syn__fault_8h.md)

[Go to the documentation of this file](syn__fault_8h.md)


```C++

#ifndef SYN_FAULT_H
#define SYN_FAULT_H

#include "../common/syn_defs.h"
#include "syn_errlog.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_FAULT_SIGNATURE  0xFA17C0DE  
typedef struct {
    uint32_t pc;    
    uint32_t lr;    
    uint32_t sp;    
    uint32_t r0;    
    uint32_t r1;    
    uint32_t r2;    
    uint32_t r3;    
    uint32_t r12;   
    uint32_t xpsr;  
} SYN_FaultContext;

typedef struct {
    uint32_t         signature;  
    SYN_FaultContext context;    
} SYN_FaultDump;

void syn_fault_capture(const SYN_FaultContext *ctx);

bool syn_fault_check_and_log(SYN_ErrLog *errlog);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FAULT_H */
```


