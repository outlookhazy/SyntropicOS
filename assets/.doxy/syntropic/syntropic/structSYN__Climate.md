

# Struct SYN\_Climate



[**ClassList**](annotated.md) **>** [**SYN\_Climate**](structSYN__Climate.md)



_Generic Climate Sensor Context._ 

* `#include <syn_climate.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**dew\_point\_c**](#variable-dew_point_c)  <br> |
|  float | [**humidity\_rh**](#variable-humidity_rh)  <br> |
|  [**SYN\_SoftI2C**](structSYN__SoftI2C.md) | [**i2c**](#variable-i2c)  <br> |
|  uint8\_t | [**i2c\_addr**](#variable-i2c_addr)  <br> |
|  float | [**pressure\_hpa**](#variable-pressure_hpa)  <br> |
|  float | [**temperature\_c**](#variable-temperature_c)  <br> |
|  [**SYN\_ClimateType**](syn__climate_8h.md#enum-syn_climatetype) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable dew\_point\_c 

```C++
float SYN_Climate::dew_point_c;
```



Calculated Dew Point in Degrees Celsius 


        

<hr>



### variable humidity\_rh 

```C++
float SYN_Climate::humidity_rh;
```



Relative Humidity % 


        

<hr>



### variable i2c 

```C++
SYN_SoftI2C SYN_Climate::i2c;
```



Software I2C bus context 


        

<hr>



### variable i2c\_addr 

```C++
uint8_t SYN_Climate::i2c_addr;
```



7-bit I2C device address 


        

<hr>



### variable pressure\_hpa 

```C++
float SYN_Climate::pressure_hpa;
```



Barometric Pressure in hPa / mbar 


        

<hr>



### variable temperature\_c 

```C++
float SYN_Climate::temperature_c;
```



Temperature in Degrees Celsius 


        

<hr>



### variable type 

```C++
SYN_ClimateType SYN_Climate::type;
```



Climate sensor model type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_climate.h`

