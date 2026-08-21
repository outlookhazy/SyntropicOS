

# File syn\_port\_spi.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_spi.h**](syn__port__spi_8h.md)

[Go to the documentation of this file](syn__port__spi_8h.md)


```C++

#ifndef SYN_PORT_SPI_H
#define SYN_PORT_SPI_H

#include "../common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_spi_init(uint8_t spi_id, uint32_t baudrate_hz, uint8_t mode, uint8_t role);

SYN_Status syn_port_spi_deinit(uint8_t spi_id);

SYN_Status syn_port_spi_transfer(uint8_t spi_id, const uint8_t *tx, uint8_t *rx, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_SPI_H */
```


