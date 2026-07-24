

# File syn\_port\_spinlock.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_spinlock.h**](syn__port__spinlock_8h.md)

[Go to the source code of this file](syn__port__spinlock_8h_source.md)

_Multicore spinlock and core-identity port functions._ [More...](#detailed-description)


































































## Detailed Description


Implement these for your platform to enable AMP multicore support. On RP2040, map to the hardware spinlock peripheral. On other MCUs, implement using compare-and-swap (CAS) or test-and-set.




**
**





* `acquire()` MUST disable interrupts on the calling core before spinning. This prevents a deadlock where an ISR on the same core tries to acquire a lock held by the interrupted main context.
* `release()` MUST restore the interrupt state to what it was before the matching `acquire()`.
* Spinlocks are NOT recursive. Acquiring a lock you already hold is undefined behavior (deadlock on most implementations).
* Spinlocks should be held for the shortest possible time. They are intended for protecting hardware registers and short critical sections — NOT for guarding long computations.






**
**


```C++
// Protect shared UART across two cores
syn_port_spinlock_acquire(SYN_SPINLOCK_UART);
syn_port_uart_transmit(0, data, len, 10);
syn_port_spinlock_release(SYN_SPINLOCK_UART);
```
 





    

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_spinlock.h`

