

# Struct SYN\_I2C\_Config



[**ClassList**](annotated.md) **>** [**SYN\_I2C\_Config**](structSYN__I2C__Config.md)



_I2C bus configuration._ 

* `#include <syn_port_i2c.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**bus**](#variable-bus)  <br> |
|  uint32\_t | [**clock\_hz**](#variable-clock_hz)  <br> |












































## Public Attributes Documentation




### variable bus 

```C++
uint8_t SYN_I2C_Config::bus;
```



I2C bus index (0, 1, ...) 
 


        

<hr>



### variable clock\_hz 

```C++
uint32_t SYN_I2C_Config::clock_hz;
```



I2C clock frequency (100k, 400k, etc) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_i2c.h`

