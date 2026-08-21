

# Struct SYN\_LwM2M\_FirmwareContext



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_FirmwareContext**](structSYN__LwM2M__FirmwareContext.md)



_Firmware Update Object (Object ID 5) context._ 

* `#include <syn_lwm2m.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  char | [**package\_uri**](#variable-package_uri)  <br> |
|  char | [**pkg\_name**](#variable-pkg_name)  <br> |
|  char | [**pkg\_version**](#variable-pkg_version)  <br> |
|  [**SYN\_LwM2M\_FirmwareResult**](syn__lwm2m_8h.md#enum-syn_lwm2m_firmwareresult) | [**result**](#variable-result)  <br> |
|  [**SYN\_LwM2M\_FirmwareState**](syn__lwm2m_8h.md#enum-syn_lwm2m_firmwarestate) | [**state**](#variable-state)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**update\_requested**](#variable-update_requested)  <br> |












































## Public Attributes Documentation




### variable package\_uri 

```C++
char SYN_LwM2M_FirmwareContext::package_uri[128];
```



Res 1: Package URI 


        

<hr>



### variable pkg\_name 

```C++
char SYN_LwM2M_FirmwareContext::pkg_name[32];
```



Res 6: Package Name 


        

<hr>



### variable pkg\_version 

```C++
char SYN_LwM2M_FirmwareContext::pkg_version[32];
```



Res 7: Package Version 


        

<hr>



### variable result 

```C++
SYN_LwM2M_FirmwareResult SYN_LwM2M_FirmwareContext::result;
```



Res 5: Update Result 


        

<hr>



### variable state 

```C++
SYN_LwM2M_FirmwareState SYN_LwM2M_FirmwareContext::state;
```



Res 3: State 


        

<hr>



### variable update\_requested 

```C++
bool SYN_LwM2M_FirmwareContext::update_requested;
```



Flag set when Res 2 (Update) executed 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.h`

