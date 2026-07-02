

# File syn\_port\_spi\_async.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_spi\_async.h**](syn__port__spi__async_8h.md)

[Go to the documentation of this file](syn__port__spi__async_8h.md)


```C++

#ifndef SYN_PORT_SPI_ASYNC_H
#define SYN_PORT_SPI_ASYNC_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(SYN_USE_SPI_ASYNC) && SYN_USE_SPI_ASYNC

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Callback ──────────────────────────────────────────────────────────── */

typedef void (*SYN_SPI_Callback)(uint8_t bus, SYN_Status result, void *ctx);

/* ── Transfer descriptor ───────────────────────────────────────────────── */

typedef struct {
    uint8_t          bus;        
    const uint8_t   *tx_buf;     
    uint8_t         *rx_buf;     
    size_t           len;        
    SYN_SPI_Callback callback;   
    void            *user_data;  
} SYN_SPI_Xfer;

/* ── Port functions (user implements) ──────────────────────────────────── */

SYN_Status syn_port_spi_xfer_async(const SYN_SPI_Xfer *xfer);

SYN_Status syn_port_spi_cancel(uint8_t bus);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_SPI_ASYNC */

#endif /* SYN_PORT_SPI_ASYNC_H */
```


