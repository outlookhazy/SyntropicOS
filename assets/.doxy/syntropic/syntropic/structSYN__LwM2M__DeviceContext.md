

# Struct SYN\_LwM2M\_DeviceContext



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_DeviceContext**](structSYN__LwM2M__DeviceContext.md)



_Device Object (Object ID 3) state and context._ 

* `#include <syn_lwm2m.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int64\_t | [**battery\_level**](#variable-battery_level)  <br> |
|  int64\_t | [**current\_time**](#variable-current_time)  <br> |
|  int64\_t | [**error\_code**](#variable-error_code)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**factory\_reset\_requested**](#variable-factory_reset_requested)  <br> |
|  const char \* | [**firmware\_ver**](#variable-firmware_ver)  <br> |
|  const char \* | [**manufacturer**](#variable-manufacturer)  <br> |
|  int64\_t | [**memory\_free\_kb**](#variable-memory_free_kb)  <br> |
|  const char \* | [**model\_number**](#variable-model_number)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**reboot\_requested**](#variable-reboot_requested)  <br> |
|  const char \* | [**serial\_number**](#variable-serial_number)  <br> |
|  const char \* | [**timezone**](#variable-timezone)  <br> |
|  const char \* | [**utc\_offset**](#variable-utc_offset)  <br> |












































## Public Attributes Documentation




### variable battery\_level 

```C++
int64_t SYN_LwM2M_DeviceContext::battery_level;
```



Res 9: Battery Level (0..100) 


        

<hr>



### variable current\_time 

```C++
int64_t SYN_LwM2M_DeviceContext::current_time;
```



Res 13: Current Time (Unix Epoch) 


        

<hr>



### variable error\_code 

```C++
int64_t SYN_LwM2M_DeviceContext::error_code;
```



Res 11: Error Code 


        

<hr>



### variable factory\_reset\_requested 

```C++
bool SYN_LwM2M_DeviceContext::factory_reset_requested;
```



Flag set when Res 5 (Factory Reset) executed 


        

<hr>



### variable firmware\_ver 

```C++
const char* SYN_LwM2M_DeviceContext::firmware_ver;
```



Res 3: Firmware Version 


        

<hr>



### variable manufacturer 

```C++
const char* SYN_LwM2M_DeviceContext::manufacturer;
```



Res 0: Manufacturer 


        

<hr>



### variable memory\_free\_kb 

```C++
int64_t SYN_LwM2M_DeviceContext::memory_free_kb;
```



Res 10: Free RAM in KB 


        

<hr>



### variable model\_number 

```C++
const char* SYN_LwM2M_DeviceContext::model_number;
```



Res 1: Model Number 


        

<hr>



### variable reboot\_requested 

```C++
bool SYN_LwM2M_DeviceContext::reboot_requested;
```



Flag set when Res 4 (Reboot) executed 


        

<hr>



### variable serial\_number 

```C++
const char* SYN_LwM2M_DeviceContext::serial_number;
```



Res 2: Serial Number 


        

<hr>



### variable timezone 

```C++
const char* SYN_LwM2M_DeviceContext::timezone;
```



Res 15: Timezone 


        

<hr>



### variable utc\_offset 

```C++
const char* SYN_LwM2M_DeviceContext::utc_offset;
```



Res 14: UTC Offset 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.h`

