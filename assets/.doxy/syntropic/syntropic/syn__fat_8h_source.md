

# File syn\_fat.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_fat.h**](syn__fat_8h.md)

[Go to the documentation of this file](syn__fat_8h.md)


```C++

#ifndef SYN_FAT_H
#define SYN_FAT_H

#include "syn_vfs.h"
#include "../drivers/syn_sd.h"

#ifdef __cplusplus
extern "C" {
#endif

const SYN_VfsOps *syn_fat_get_ops(void);

SYN_Status syn_fat_init(uint8_t spi_bus, SYN_GPIO_Pin cs);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FAT_H */
```


