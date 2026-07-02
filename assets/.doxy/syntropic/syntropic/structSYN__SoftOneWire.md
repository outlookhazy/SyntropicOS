

# Struct SYN\_SoftOneWire



[**ClassList**](annotated.md) **>** [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md)



_1-Wire bus handle. Caller allocates; zero heap._ 

* `#include <syn_soft_onewire.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**delay\_loops**](#variable-delay_loops)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin**](#variable-pin)  <br> |












































## Public Attributes Documentation




### variable delay\_loops 

```C++
uint32_t SYN_SoftOneWire::delay_loops;
```



NOP iterations per µs — use [**SYN\_SOFT\_ONEWIRE\_LOOPS\_PER\_US(F\_CPU)**](syn__soft__onewire_8h.md#define-syn_soft_onewire_loops_per_us) 
 


        

<hr>



### variable pin 

```C++
SYN_GPIO_Pin SYN_SoftOneWire::pin;
```



GPIO pin for the data line 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_soft_onewire.h`

