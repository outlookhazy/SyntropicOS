

# Struct SYN\_BootData



[**ClassList**](annotated.md) **>** [**SYN\_BootData**](structSYN__BootData.md)



_Boot data persisted to flash across resets._ 

* `#include <syn_boot.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**boot\_count**](#variable-boot_count)  <br> |
|  uint16\_t | [**fail\_count**](#variable-fail_count)  <br> |
|  uint8\_t | [**last\_reset**](#variable-last_reset)  <br> |
|  uint8\_t | [**was\_healthy**](#variable-was_healthy)  <br> |












































## Public Attributes Documentation




### variable boot\_count 

```C++
uint32_t SYN_BootData::boot_count;
```



Total boot count (monotonic) 
 


        

<hr>



### variable fail\_count 

```C++
uint16_t SYN_BootData::fail_count;
```



Consecutive boots without healthy 
 


        

<hr>



### variable last\_reset 

```C++
uint8_t SYN_BootData::last_reset;
```



Last reset reason (application-set) 
 


        

<hr>



### variable was\_healthy 

```C++
uint8_t SYN_BootData::was_healthy;
```



Did last boot complete successfully? 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_boot.h`

