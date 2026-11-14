

# Struct SYN\_I2C\_Config



[**ClassList**](annotated.md) **>** [**SYN\_I2C\_Config**](structSYN__I2C__Config.md)



_I2C driver instance configuration parameters._ 

* `#include <syn_i2c.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**clock\_speed\_hz**](#variable-clock_speed_hz)  <br> |
|  uint8\_t | [**i2c\_id**](#variable-i2c_id)  <br> |
|  uint16\_t | [**own\_address**](#variable-own_address)  <br> |
|  [**SYN\_I2C\_Role**](syn__i2c_8h.md#enum-syn_i2c_role) | [**role**](#variable-role)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**use\_dma**](#variable-use_dma)  <br> |












































## Public Attributes Documentation




### variable clock\_speed\_hz 

```C++
uint32_t SYN_I2C_Config::clock_speed_hz;
```



Bus speed in Hz (e.g. 100000, 400000) 


        

<hr>



### variable i2c\_id 

```C++
uint8_t SYN_I2C_Config::i2c_id;
```



Hardware I2C instance (0 = I2C1) 


        

<hr>



### variable own\_address 

```C++
uint16_t SYN_I2C_Config::own_address;
```



7-bit own slave address 


        

<hr>



### variable role 

```C++
SYN_I2C_Role SYN_I2C_Config::role;
```



Master or Slave mode 


        

<hr>



### variable use\_dma 

```C++
bool SYN_I2C_Config::use_dma;
```



Enable DMA transfers 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_i2c.h`

