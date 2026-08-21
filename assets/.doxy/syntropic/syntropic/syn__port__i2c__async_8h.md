

# File syn\_port\_i2c\_async.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_i2c\_async.h**](syn__port__i2c__async_8h.md)

[Go to the source code of this file](syn__port__i2c__async_8h_source.md)

_Async I2C port interface — implement these for your platform._ [More...](#detailed-description)


































































## Detailed Description


Provides non-blocking, callback-based I2C transactions alongside the existing blocking API in [**syn\_port\_i2c.h**](syn__port__i2c_8h.md). The callback fires from ISR context.




**
**


```C++
static void on_i2c_done(uint8_t bus, SYN_Status result, void *ctx) {
    // Access rx_buf — data is ready
}

uint8_t reg = 0xD0;
uint8_t val;
SYN_I2C_Xfer xfer = {
    .bus       = 0,
    .addr      = 0x76,
    .tx_data   = &reg,
    .tx_len    = 1,
    .rx_data   = &val,
    .rx_len    = 1,
    .callback  = on_i2c_done,
    .user_data = NULL,
};
syn_port_i2c_xfer_async(&xfer);
```
 





    

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_i2c_async.h`

