

# File syn\_version.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_version.h**](syn__version_8h.md)

[Go to the documentation of this file](syn__version_8h.md)


```C++

#ifndef SYN_VERSION_H
#define SYN_VERSION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Defaults (override in build system or config) ──────────────────────── */

#ifndef SYN_VERSION_YEAR
#define SYN_VERSION_YEAR   2026
#endif

#ifndef SYN_VERSION_MONTH
#define SYN_VERSION_MONTH  7
#endif

#ifndef SYN_VERSION_RELEASE
#define SYN_VERSION_RELEASE 0
#endif

#define SYN_VERSION_CODE \
    (((uint32_t)SYN_VERSION_YEAR  << 16) | \
     ((uint32_t)SYN_VERSION_MONTH <<  8) | \
      (uint32_t)SYN_VERSION_RELEASE)

#ifndef SYN_BUILD_DATE
#define SYN_BUILD_DATE  __DATE__
#endif

#ifndef SYN_BUILD_TIME
#define SYN_BUILD_TIME  __TIME__
#endif

#ifndef SYN_GIT_HASH
#define SYN_GIT_HASH  "unknown"
#endif

#ifndef SYN_APP_NAME
#define SYN_APP_NAME  "SyntropicOS"
#endif

/* ── Version struct ─────────────────────────────────────────────────────── */

typedef struct {
    uint16_t     year;        
    uint8_t      month;       
    uint8_t      release;     
    uint32_t     code;        
    const char  *date;        
    const char  *time;        
    const char  *git_hash;    
    const char  *app_name;    
} SYN_Version;

static inline const SYN_Version *syn_version(void)
{
    static const SYN_Version v = {
        .year     = SYN_VERSION_YEAR,
        .month    = SYN_VERSION_MONTH,
        .release  = SYN_VERSION_RELEASE,
        .code     = SYN_VERSION_CODE,
        .date     = SYN_BUILD_DATE,
        .time     = SYN_BUILD_TIME,
        .git_hash = SYN_GIT_HASH,
        .app_name = SYN_APP_NAME,
    };
    return &v;
}

static inline int syn_version_at_least(uint16_t year, uint8_t month, uint8_t rel)
{
    uint32_t required = ((uint32_t)year  << 16) |
                        ((uint32_t)month << 8)  |
                         (uint32_t)rel;
    return SYN_VERSION_CODE >= required;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_VERSION_H */
```


