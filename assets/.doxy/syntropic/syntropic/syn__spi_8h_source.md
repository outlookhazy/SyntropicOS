

# File syn\_spi.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_spi.h**](syn__spi_8h.md)

[Go to the documentation of this file](syn__spi_8h.md)


```C++

#ifndef SYN_SPI_H
#define SYN_SPI_H

#include "../common/syn_defs.h"
#include "../port/syn_port_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_SPI_ROLE_MASTER = 0, 
    SYN_SPI_ROLE_SLAVE       
} SYN_SPI_Role;

typedef enum {
    SYN_SPI_MODE_0 = 0, 
    SYN_SPI_MODE_1 = 1, 
    SYN_SPI_MODE_2 = 2, 
    SYN_SPI_MODE_3 = 3  
} SYN_SPI_Mode;

typedef struct {
    uint8_t spi_id;       
    uint32_t baudrate_hz; 
    SYN_SPI_Mode mode;    
    SYN_SPI_Role role;    
    bool use_dma;         
} SYN_SPI_Config;

typedef struct {
    SYN_SPI_Config cfg; 
    bool initialized;   
} SYN_SPI;

SYN_Status syn_spi_init(SYN_SPI *spi, const SYN_SPI_Config *cfg);

SYN_Status syn_spi_deinit(SYN_SPI *spi);

SYN_Status syn_spi_transfer(SYN_SPI *spi, const uint8_t *tx, uint8_t *rx, size_t len);

SYN_Status syn_spi_write(SYN_SPI *spi, const uint8_t *tx, size_t len);

SYN_Status syn_spi_read(SYN_SPI *spi, uint8_t *rx, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SPI_H */
```


