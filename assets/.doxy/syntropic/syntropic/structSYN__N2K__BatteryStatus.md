

# Struct SYN\_N2K\_BatteryStatus



[**ClassList**](annotated.md) **>** [**SYN\_N2K\_BatteryStatus**](structSYN__N2K__BatteryStatus.md)



_Battery Status (PGN 127508)._ 

* `#include <syn_n2k.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t | [**current\_1e1**](#variable-current_1e1)  <br> |
|  uint8\_t | [**instance**](#variable-instance)  <br> |
|  uint8\_t | [**sid**](#variable-sid)  <br> |
|  uint16\_t | [**temperature\_1e1**](#variable-temperature_1e1)  <br> |
|  uint16\_t | [**voltage\_1e2**](#variable-voltage_1e2)  <br> |












































## Public Attributes Documentation




### variable current\_1e1 

```C++
int16_t SYN_N2K_BatteryStatus::current_1e1;
```



Battery Current in 0.1 Amperes (e.g. 150 = 15.0A) 


        

<hr>



### variable instance 

```C++
uint8_t SYN_N2K_BatteryStatus::instance;
```



Battery Instance ID (0-254) 


        

<hr>



### variable sid 

```C++
uint8_t SYN_N2K_BatteryStatus::sid;
```



Sequence ID 


        

<hr>



### variable temperature\_1e1 

```C++
uint16_t SYN_N2K_BatteryStatus::temperature_1e1;
```



Battery Temperature in 0.1 Kelvin (e.g. 2982 = 298.2K / 25C) 


        

<hr>



### variable voltage\_1e2 

```C++
uint16_t SYN_N2K_BatteryStatus::voltage_1e2;
```



Battery Voltage in 0.01 Volts (e.g. 1250 = 12.50V) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_n2k.h`

