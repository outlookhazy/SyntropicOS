

# File syn\_port\_wdt.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_wdt.h**](syn__port__wdt_8h.md)

[Go to the documentation of this file](syn__port__wdt_8h.md)


```C++

#ifndef SYN_PORT_WDT_H
#define SYN_PORT_WDT_H

#include <stdint.h>
#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_wdt_init(uint32_t timeout_ms);

void syn_port_wdt_feed(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_WDT_H */
```


