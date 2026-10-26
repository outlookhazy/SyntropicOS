

# Struct SYN\_SoftI2C



[**ClassList**](annotated.md) **>** [**SYN\_SoftI2C**](structSYN__SoftI2C.md)



_Software I2C instance._ 

* `#include <syn_soft_i2c.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**delay\_loops**](#variable-delay_loops)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**scl**](#variable-scl)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**sda**](#variable-sda)  <br> |












































## Public Attributes Documentation




### variable delay\_loops 

```C++
uint32_t SYN_SoftI2C::delay_loops;
```



Iteration count for half-clock software delay 


        

<hr>



### variable scl 

```C++
SYN_GPIO_Pin SYN_SoftI2C::scl;
```



SCL GPIO pin identifier 


        

<hr>



### variable sda 

```C++
SYN_GPIO_Pin SYN_SoftI2C::sda;
```



SDA GPIO pin identifier 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_soft_i2c.h`

