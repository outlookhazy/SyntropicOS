

# File syn\_errlog.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_errlog.h**](syn__errlog_8h.md)

[Go to the documentation of this file](syn__errlog_8h.md)


```C++

#ifndef SYN_ERRLOG_H
#define SYN_ERRLOG_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Error severity ─────────────────────────────────────────────────────── */

typedef enum {
    SYN_ERR_INFO    = 0,   
    SYN_ERR_WARNING = 1,   
    SYN_ERR_ERROR   = 2,   
    SYN_ERR_FATAL   = 3,   
} SYN_ErrSeverity;

/* ── Error entry ────────────────────────────────────────────────────────── */

typedef struct {
    uint16_t  code;         
    uint8_t   severity;     
    uint8_t   _pad;         
    uint32_t  context;      
    uint32_t  timestamp;    
    uint32_t  boot_count;   
} SYN_ErrEntry;

/* ── Error log instance ─────────────────────────────────────────────────── */

typedef struct SYN_ErrLog {
    SYN_ErrEntry  *entries;     
    size_t          capacity;    
    size_t          head;        
    size_t          total_count; 
    uint32_t        boot_count;  
    bool            enabled;     
} SYN_ErrLog;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_errlog_init(SYN_ErrLog *log, SYN_ErrEntry *entries,
                      size_t capacity, uint32_t boot_count);

void syn_errlog_record(SYN_ErrLog *log, uint16_t code,
                        SYN_ErrSeverity severity, uint32_t context);

bool syn_errlog_read(const SYN_ErrLog *log, size_t index,
                      SYN_ErrEntry *out);

static inline size_t syn_errlog_count(const SYN_ErrLog *log)
{
    return log->total_count;
}

static inline size_t syn_errlog_available(const SYN_ErrLog *log)
{
    return (log->total_count < log->capacity)
         ? log->total_count
         : log->capacity;
}

static inline void syn_errlog_enable(SYN_ErrLog *log, bool en)
{
    log->enabled = en;
}

void syn_errlog_clear(SYN_ErrLog *log);

const SYN_ErrEntry *syn_errlog_latest(const SYN_ErrLog *log);

size_t syn_errlog_count_severity(const SYN_ErrLog *log,
                                  SYN_ErrSeverity severity);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ERRLOG_H */
```


