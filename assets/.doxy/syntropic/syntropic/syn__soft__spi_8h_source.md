

# File syn\_soft\_spi.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_soft\_spi.h**](syn__soft__spi_8h.md)

[Go to the documentation of this file](syn__soft__spi_8h.md)


```C++

#ifndef SYN_SOFT_SPI_H
#define SYN_SOFT_SPI_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_SPI_MODE_0 = 0, 
    SYN_SPI_MODE_1 = 1, 
    SYN_SPI_MODE_2 = 2, 
    SYN_SPI_MODE_3 = 3  
} SYN_SPIMode;

typedef struct {
    SYN_GPIO_Pin sck;              
    SYN_GPIO_Pin mosi;             
    SYN_GPIO_Pin miso;             
    SYN_SPIMode mode;              
    uint32_t delay_loops;          
    // Cached states for faster transfer
    bool cpha;                     
    SYN_GPIO_State idle_state;     
    SYN_GPIO_State active_state;   
    // Optional chip select (set to (SYN_GPIO_Pin)-1 if unused)
    SYN_GPIO_Pin cs_pin;           
    bool cs_active_low;            
} SYN_SoftSPI;

void syn_soft_spi_init(SYN_SoftSPI *spi, SYN_GPIO_Pin sck, SYN_GPIO_Pin mosi, SYN_GPIO_Pin miso, SYN_SPIMode mode, uint32_t delay_loops);

uint8_t syn_soft_spi_transfer(const SYN_SoftSPI *spi, uint8_t data);

void syn_soft_spi_transfer_bulk(SYN_SoftSPI *spi, const uint8_t *tx, uint8_t *rx, size_t len);

void syn_soft_spi_set_cs(SYN_SoftSPI *spi, SYN_GPIO_Pin cs_pin, bool active_low);

void syn_soft_spi_select(SYN_SoftSPI *spi);

void syn_soft_spi_deselect(SYN_SoftSPI *spi);

#ifdef __cplusplus
}
#endif
#endif // SYN_SOFT_SPI_H
```


