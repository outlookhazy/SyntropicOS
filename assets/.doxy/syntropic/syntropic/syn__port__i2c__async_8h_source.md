

# File syn\_port\_i2c\_async.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_i2c\_async.h**](syn__port__i2c__async_8h.md)

[Go to the documentation of this file](syn__port__i2c__async_8h.md)


```C++

#ifndef SYN_PORT_I2C_ASYNC_H
#define SYN_PORT_I2C_ASYNC_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(SYN_USE_I2C_ASYNC) && SYN_USE_I2C_ASYNC

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Callback ──────────────────────────────────────────────────────────── */

typedef void (*SYN_I2C_Callback)(uint8_t bus, SYN_Status result, void *ctx);

/* ── Transfer descriptor ───────────────────────────────────────────────── */

typedef struct {
    uint8_t          bus;        
    uint8_t          addr;       
    const uint8_t   *tx_data;    
    size_t           tx_len;     
    uint8_t         *rx_data;    
    size_t           rx_len;     
    SYN_I2C_Callback callback;   
    void            *user_data;  
} SYN_I2C_Xfer;

/* ── Port functions (user implements) ──────────────────────────────────── */

SYN_Status syn_port_i2c_xfer_async(const SYN_I2C_Xfer *xfer);

SYN_Status syn_port_i2c_cancel(uint8_t bus);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_I2C_ASYNC */

#endif /* SYN_PORT_I2C_ASYNC_H */
```


