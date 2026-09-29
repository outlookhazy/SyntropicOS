

# Struct LwM2M\_FwCtx



[**ClassList**](annotated.md) **>** [**LwM2M\_FwCtx**](structLwM2M__FwCtx.md)



_LwM2M Firmware object internal layout for state synchronization._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  char | [**package\_uri**](#variable-package_uri)  <br> |
|  char | [**pkg\_name**](#variable-pkg_name)  <br> |
|  char | [**pkg\_version**](#variable-pkg_version)  <br> |
|  [**LwM2M\_FwResult**](syn__ota_8c.md#enum-lwm2m_fwresult) | [**result**](#variable-result)  <br> |
|  [**LwM2M\_FwState**](syn__ota_8c.md#enum-lwm2m_fwstate) | [**state**](#variable-state)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**update\_requested**](#variable-update_requested)  <br> |












































## Public Attributes Documentation




### variable package\_uri 

```C++
char LwM2M_FwCtx::package_uri[128];
```



Package download URI. 


        

<hr>



### variable pkg\_name 

```C++
char LwM2M_FwCtx::pkg_name[32];
```



Package name. 


        

<hr>



### variable pkg\_version 

```C++
char LwM2M_FwCtx::pkg_version[32];
```



Package version string. 


        

<hr>



### variable result 

```C++
LwM2M_FwResult LwM2M_FwCtx::result;
```



Firmware update result code. 


        

<hr>



### variable state 

```C++
LwM2M_FwState LwM2M_FwCtx::state;
```



Firmware object state. 


        

<hr>



### variable update\_requested 

```C++
bool LwM2M_FwCtx::update_requested;
```



Update execution flag. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_ota.c`

