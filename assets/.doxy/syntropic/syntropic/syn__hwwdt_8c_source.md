

# File syn\_hwwdt.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_hwwdt.c**](syn__hwwdt_8c.md)

[Go to the documentation of this file](syn__hwwdt_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_HWWDT) || SYN_USE_HWWDT

#include "syn_hwwdt.h"
#include "../port/syn_port_wdt.h"
#include "../util/syn_assert.h"

SYN_Status syn_hwwdt_init(uint32_t timeout_ms)
{
    SYN_ASSERT(timeout_ms > 0u);
    return syn_port_wdt_init(timeout_ms);
}

void syn_hwwdt_feed(void)
{
    syn_port_wdt_feed();
}

#endif /* SYN_USE_HWWDT */
```


