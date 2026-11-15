

# Struct SYN\_OCPP\_MeterValues



[**ClassList**](annotated.md) **>** [**SYN\_OCPP\_MeterValues**](structSYN__OCPP__MeterValues.md)



_Meter Values reading structure._ 

* `#include <syn_ocpp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**current\_a**](#variable-current_a)  <br> |
|  uint32\_t | [**energy\_wh**](#variable-energy_wh)  <br> |
|  uint16\_t | [**power\_kw**](#variable-power_kw)  <br> |
|  uint8\_t | [**soc\_percent**](#variable-soc_percent)  <br> |
|  uint16\_t | [**voltage\_v**](#variable-voltage_v)  <br> |












































## Public Attributes Documentation




### variable current\_a 

```C++
uint16_t SYN_OCPP_MeterValues::current_a;
```



RMS Current in A (deci-Amperes, 0.1A) 


        

<hr>



### variable energy\_wh 

```C++
uint32_t SYN_OCPP_MeterValues::energy_wh;
```



Total active energy in Wh 


        

<hr>



### variable power\_kw 

```C++
uint16_t SYN_OCPP_MeterValues::power_kw;
```



Active Power in W 


        

<hr>



### variable soc\_percent 

```C++
uint8_t SYN_OCPP_MeterValues::soc_percent;
```



State of Charge (0..100%) 


        

<hr>



### variable voltage\_v 

```C++
uint16_t SYN_OCPP_MeterValues::voltage_v;
```



RMS Voltage in V 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ocpp.h`

