

# Struct SYN\_IOExp



[**ClassList**](annotated.md) **>** [**SYN\_IOExp**](structSYN__IOExp.md)



_Generic I2C GPIO Expander Instance Context._ 

* `#include <syn_ioexp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**dir\_mask**](#variable-dir_mask)  <br> |
|  [**SYN\_SoftI2C**](structSYN__SoftI2C.md) | [**i2c**](#variable-i2c)  <br> |
|  uint8\_t | [**i2c\_addr**](#variable-i2c_addr)  <br> |
|  uint8\_t | [**num\_pins**](#variable-num_pins)  <br> |
|  uint16\_t | [**out\_mask**](#variable-out_mask)  <br> |
|  uint16\_t | [**pullup\_mask**](#variable-pullup_mask)  <br> |
|  [**SYN\_IOExpType**](syn__ioexp_8h.md#enum-syn_ioexptype) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable dir\_mask 

```C++
uint16_t SYN_IOExp::dir_mask;
```



Direction mask (1 = input, 0 = output) 


        

<hr>



### variable i2c 

```C++
SYN_SoftI2C SYN_IOExp::i2c;
```



Software I2C bus context 


        

<hr>



### variable i2c\_addr 

```C++
uint8_t SYN_IOExp::i2c_addr;
```



7-bit I2C device address 


        

<hr>



### variable num\_pins 

```C++
uint8_t SYN_IOExp::num_pins;
```



Total pins (8 or 16) 


        

<hr>



### variable out\_mask 

```C++
uint16_t SYN_IOExp::out_mask;
```



Output state cache 


        

<hr>



### variable pullup\_mask 

```C++
uint16_t SYN_IOExp::pullup_mask;
```



Pull-up resistor mask 


        

<hr>



### variable type 

```C++
SYN_IOExpType SYN_IOExp::type;
```



GPIO expander model type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_ioexp.h`

