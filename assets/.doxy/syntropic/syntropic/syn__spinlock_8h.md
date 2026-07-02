

# File syn\_spinlock.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_spinlock.h**](syn__spinlock_8h.md)

[Go to the source code of this file](syn__spinlock_8h_source.md)

_Scoped spinlock helper for shared resource protection._ [More...](#detailed-description)


































































## Detailed Description


Convenience wrapper around the spinlock port functions. Provides an RAII-style scoped lock that guarantees release on scope exit.




**
**


```C++
// Protect a shared UART across two cores
SYN_SPINLOCK_GUARD(SYN_SPINLOCK_UART) {
    syn_port_uart_transmit(0, data, len, 10);
}
```





**
**

Define your own in syn\_config.h or use the defaults:
* `SYN_SPINLOCK_UART` (0) — Shared UART access
* `SYN_SPINLOCK_FLASH` (1) — Shared flash access
* `SYN_SPINLOCK_USER0` (2) — Application use
* `SYN_SPINLOCK_USER1` (3) — Application use 







    

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_spinlock.h`

