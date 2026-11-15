

# Struct SYN\_Lux



[**ClassList**](annotated.md) **>** [**SYN\_Lux**](structSYN__Lux.md)



_Generic Light & Color Sensor Context._ 

* `#include <syn_lux.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**color\_b**](#variable-color_b)  <br> |
|  uint16\_t | [**color\_c**](#variable-color_c)  <br> |
|  uint16\_t | [**color\_g**](#variable-color_g)  <br> |
|  uint16\_t | [**color\_r**](#variable-color_r)  <br> |
|  uint16\_t | [**color\_temp\_k**](#variable-color_temp_k)  <br> |
|  [**SYN\_SoftI2C**](structSYN__SoftI2C.md) | [**i2c**](#variable-i2c)  <br> |
|  uint8\_t | [**i2c\_addr**](#variable-i2c_addr)  <br> |
|  float | [**illuminance\_lux**](#variable-illuminance_lux)  <br> |
|  [**SYN\_LuxType**](syn__lux_8h.md#enum-syn_luxtype) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable color\_b 

```C++
uint16_t SYN_Lux::color_b;
```



Raw Blue channel (TCS34725) 


        

<hr>



### variable color\_c 

```C++
uint16_t SYN_Lux::color_c;
```



Raw Clear channel (TCS34725) 


        

<hr>



### variable color\_g 

```C++
uint16_t SYN_Lux::color_g;
```



Raw Green channel (TCS34725) 


        

<hr>



### variable color\_r 

```C++
uint16_t SYN_Lux::color_r;
```



Raw Red channel (TCS34725) 


        

<hr>



### variable color\_temp\_k 

```C++
uint16_t SYN_Lux::color_temp_k;
```



Calculated Correlated Color Temp in Kelvin 


        

<hr>



### variable i2c 

```C++
SYN_SoftI2C SYN_Lux::i2c;
```



Software I2C bus context 


        

<hr>



### variable i2c\_addr 

```C++
uint8_t SYN_Lux::i2c_addr;
```



7-bit I2C device address 


        

<hr>



### variable illuminance\_lux 

```C++
float SYN_Lux::illuminance_lux;
```



Ambient Light in Lux 


        

<hr>



### variable type 

```C++
SYN_LuxType SYN_Lux::type;
```



Light sensor model type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_lux.h`

