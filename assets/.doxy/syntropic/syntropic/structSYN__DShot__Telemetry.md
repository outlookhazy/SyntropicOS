

# Struct SYN\_DShot\_Telemetry



[**ClassList**](annotated.md) **>** [**SYN\_DShot\_Telemetry**](structSYN__DShot__Telemetry.md)



[More...](#detailed-description)

* `#include <syn_dshot_telemetry.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**erpm**](#variable-erpm)  <br> |
|  uint32\_t | [**period\_us**](#variable-period_us)  <br> |
|  uint32\_t | [**rpm**](#variable-rpm)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid**](#variable-valid)  <br> |












































## Detailed Description


Decoded BDShot Telemetry Packet. 


    
## Public Attributes Documentation




### variable erpm 

```C++
uint32_t SYN_DShot_Telemetry::erpm;
```



Electrical RPM (eRPM). 


        

<hr>



### variable period\_us 

```C++
uint32_t SYN_DShot_Telemetry::period_us;
```



Motor commutation period in microseconds. 


        

<hr>



### variable rpm 

```C++
uint32_t SYN_DShot_Telemetry::rpm;
```



Mechanical RPM (for specified pole pairs). 


        

<hr>



### variable valid 

```C++
bool SYN_DShot_Telemetry::valid;
```



True if GCR decoding and CRC check succeeded. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_dshot_telemetry.h`

