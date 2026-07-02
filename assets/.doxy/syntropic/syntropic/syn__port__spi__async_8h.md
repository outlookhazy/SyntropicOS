

# File syn\_port\_spi\_async.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_spi\_async.h**](syn__port__spi__async_8h.md)

[Go to the source code of this file](syn__port__spi__async_8h_source.md)

_Async SPI port interface — implement these for your platform._ [More...](#detailed-description)


































































## Detailed Description


Provides non-blocking, callback-based SPI transfers alongside the existing blocking API in [**syn\_port\_spi.h**](syn__port__spi_8h.md). The callback fires from ISR context.




**
**


```C++
static void on_spi_done(uint8_t bus, SYN_Status result, void *ctx) {
    syn_port_spi_cs_deassert(0, cs_pin);
    // Process rx_buf
}

syn_port_spi_cs_assert(0, cs_pin);
SYN_SPI_Xfer xfer = {
    .bus       = 0,
    .tx_buf    = tx_data,
    .rx_buf    = rx_data,
    .len       = 256,
    .callback  = on_spi_done,
    .user_data = NULL,
};
syn_port_spi_xfer_async(&xfer);
```
 





    

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_spi_async.h`

