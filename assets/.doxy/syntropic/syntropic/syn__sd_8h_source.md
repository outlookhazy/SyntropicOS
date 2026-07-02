

# File syn\_sd.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_sd.h**](syn__sd_8h.md)

[Go to the documentation of this file](syn__sd_8h.md)


```C++

#ifndef SYN_SD_H
#define SYN_SD_H

#include "../common/syn_defs.h"
#include "../port/syn_port_spi.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_SD_SECTOR_SIZE  512u

typedef enum {
    SYN_SD_UNKNOWN = 0, 
    SYN_SD_SDSC    = 1, 
    SYN_SD_SDHC    = 2, 
} SYN_SD_Type;

typedef struct {
    uint8_t       spi_bus;      
    SYN_GPIO_Pin  cs_pin;       
    SYN_SD_Type   type;         
    uint32_t      sector_count; 
    bool          initialized;  
} SYN_SD;

SYN_Status syn_sd_init(SYN_SD *sd, uint8_t spi_bus, SYN_GPIO_Pin cs);

SYN_Status syn_sd_read(const SYN_SD *sd, uint32_t sector, uint8_t *buf);

SYN_Status syn_sd_write(const SYN_SD *sd, uint32_t sector, const uint8_t *buf);

SYN_Status syn_sd_sync(const SYN_SD *sd);

uint32_t syn_sd_sectors(const SYN_SD *sd);

SYN_SD_Type syn_sd_type(const SYN_SD *sd);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SD_H */
```


