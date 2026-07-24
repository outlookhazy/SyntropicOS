

# File syn\_port\_flash.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_flash.h**](syn__port__flash_8h.md)

[Go to the documentation of this file](syn__port__flash_8h.md)


```C++

#ifndef SYN_PORT_FLASH_H
#define SYN_PORT_FLASH_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_flash_erase(uint32_t addr);

SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len);

SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len);

uint32_t syn_port_flash_sector_size(uint32_t addr);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_FLASH_H */
```


