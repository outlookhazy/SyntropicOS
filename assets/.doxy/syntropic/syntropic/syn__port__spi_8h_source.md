

# File syn\_port\_spi.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_spi.h**](syn__port__spi_8h.md)

[Go to the documentation of this file](syn__port__spi_8h.md)


```C++

#ifndef SYN_PORT_SPI_H
#define SYN_PORT_SPI_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── SPI mode (CPOL/CPHA) ───────────────────────────────────────────────── */

typedef enum {
    SYN_SPI_MODE_0 = 0,  
    SYN_SPI_MODE_1 = 1,  
    SYN_SPI_MODE_2 = 2,  
    SYN_SPI_MODE_3 = 3,  
} SYN_SPI_Mode;

/* ── SPI configuration ──────────────────────────────────────────────────── */

typedef struct {
    uint8_t       bus;            
    uint32_t      clock_hz;       
    SYN_SPI_Mode mode;           
    uint8_t       bit_order;      
} SYN_SPI_Config;

/* ── Port functions (user implements) ───────────────────────────────────── */

SYN_Status syn_port_spi_init(const SYN_SPI_Config *cfg);

SYN_Status syn_port_spi_deinit(uint8_t bus);

SYN_Status syn_port_spi_transfer(uint8_t bus,
                                   const uint8_t *tx_buf,
                                   uint8_t *rx_buf,
                                   size_t len);

SYN_Status syn_port_spi_cs_assert(uint8_t bus, SYN_GPIO_Pin cs_pin);

SYN_Status syn_port_spi_cs_deassert(uint8_t bus, SYN_GPIO_Pin cs_pin);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_SPI_H */
```


