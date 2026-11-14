

# File port\_rp2040\_multicore.c



[**FileList**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**rp2040**](dir_d9ab0060999f493ad92c011306b81fae.md) **>** [**port\_rp2040\_multicore.c**](port__rp2040__multicore_8c.md)

[Go to the source code of this file](port__rp2040__multicore_8c_source.md)

_SyntropicOS AMP multicore port for RP2040/RP2350 (Pico SDK)._ [More...](#detailed-description)


































































## Detailed Description


Implements the spinlock, memory barrier, core identity, and IPC port functions using the RP2040 hardware spinlock peripheral and Cortex-M0+ barrier instructions.


Hardware spinlocks are dynamically allocated via spin\_lock\_claim\_unused() to avoid conflicts with SDK-internal usage.


Compiled when: PICO\_BOARD && !ARDUINO && SYN\_USE\_MULTICORE 


    

------------------------------
The documentation for this class was generated from the following file `src/port/rp2040/port_rp2040_multicore.c`

