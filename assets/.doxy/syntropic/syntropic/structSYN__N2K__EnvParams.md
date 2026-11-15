

# Struct SYN\_N2K\_EnvParams



[**ClassList**](annotated.md) **>** [**SYN\_N2K\_EnvParams**](structSYN__N2K__EnvParams.md)



_Environmental Parameters (PGN 130310)._ 

* `#include <syn_n2k.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**air\_temp\_1e2**](#variable-air_temp_1e2)  <br> |
|  uint16\_t | [**pressure\_pa\_1e2**](#variable-pressure_pa_1e2)  <br> |
|  uint8\_t | [**sid**](#variable-sid)  <br> |
|  uint16\_t | [**water\_temp\_1e2**](#variable-water_temp_1e2)  <br> |












































## Public Attributes Documentation




### variable air\_temp\_1e2 

```C++
uint16_t SYN_N2K_EnvParams::air_temp_1e2;
```



Outside Air Temperature in 0.01 Kelvin 


        

<hr>



### variable pressure\_pa\_1e2 

```C++
uint16_t SYN_N2K_EnvParams::pressure_pa_1e2;
```



Atmospheric Pressure in 100 Pa (hPa / mbar) 


        

<hr>



### variable sid 

```C++
uint8_t SYN_N2K_EnvParams::sid;
```



Sequence ID 


        

<hr>



### variable water\_temp\_1e2 

```C++
uint16_t SYN_N2K_EnvParams::water_temp_1e2;
```



Water Temperature in 0.01 Kelvin 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_n2k.h`

