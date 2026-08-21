

# Struct SYN\_CiA401\_Device



[**ClassList**](annotated.md) **>** [**SYN\_CiA401\_Device**](structSYN__CiA401__Device.md)



_CiA 401 Generic I/O Device State Instance._ 

* `#include <syn_cia401.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t | [**analog\_delta**](#variable-analog_delta)  <br> |
|  int16\_t | [**analog\_in**](#variable-analog_in)  <br> |
|  uint8\_t | [**analog\_in\_count**](#variable-analog_in_count)  <br> |
|  int16\_t | [**analog\_out**](#variable-analog_out)  <br> |
|  uint8\_t | [**analog\_out\_count**](#variable-analog_out_count)  <br> |
|  uint8\_t | [**digital\_in**](#variable-digital_in)  <br> |
|  uint8\_t | [**digital\_in\_count**](#variable-digital_in_count)  <br> |
|  uint8\_t | [**digital\_out**](#variable-digital_out)  <br> |
|  uint8\_t | [**digital\_out\_count**](#variable-digital_out_count)  <br> |
|  int16\_t | [**prev\_analog\_in**](#variable-prev_analog_in)  <br> |












































## Public Attributes Documentation




### variable analog\_delta 

```C++
int16_t SYN_CiA401_Device::analog_delta[SYN_CIA401_MAX_ANALOG_16BIT];
```



Change threshold for event PDO 


        

<hr>



### variable analog\_in 

```C++
int16_t SYN_CiA401_Device::analog_in[SYN_CIA401_MAX_ANALOG_16BIT];
```



Analog input 16-bit values 


        

<hr>



### variable analog\_in\_count 

```C++
uint8_t SYN_CiA401_Device::analog_in_count;
```



Configured analog input channel count 


        

<hr>



### variable analog\_out 

```C++
int16_t SYN_CiA401_Device::analog_out[SYN_CIA401_MAX_ANALOG_16BIT];
```



Analog output 16-bit values 


        

<hr>



### variable analog\_out\_count 

```C++
uint8_t SYN_CiA401_Device::analog_out_count;
```



Configured analog output channel count 


        

<hr>



### variable digital\_in 

```C++
uint8_t SYN_CiA401_Device::digital_in[SYN_CIA401_MAX_DIGITAL_8BIT];
```



Digital input bytes (channels 1..4) 


        

<hr>



### variable digital\_in\_count 

```C++
uint8_t SYN_CiA401_Device::digital_in_count;
```



Configured digital input byte count 


        

<hr>



### variable digital\_out 

```C++
uint8_t SYN_CiA401_Device::digital_out[SYN_CIA401_MAX_DIGITAL_8BIT];
```



Digital output bytes (channels 1..4) 


        

<hr>



### variable digital\_out\_count 

```C++
uint8_t SYN_CiA401_Device::digital_out_count;
```



Configured digital output byte count 


        

<hr>



### variable prev\_analog\_in 

```C++
int16_t SYN_CiA401_Device::prev_analog_in[SYN_CIA401_MAX_ANALOG_16BIT];
```



Saved previous analog values 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia401.h`

