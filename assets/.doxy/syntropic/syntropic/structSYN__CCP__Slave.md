

# Struct SYN\_CCP\_Slave



[**ClassList**](annotated.md) **>** [**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md)



_CCP Slave Instance Handle._ 

* `#include <syn_ccp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**active\_cal\_page**](#variable-active_cal_page)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**connected**](#variable-connected)  <br> |
|  [**SYN\_CCP\_DAQList**](structSYN__CCP__DAQList.md) | [**daq\_lists**](#variable-daq_lists)  <br> |
|  [**SYN\_CCP\_EraseFn**](syn__ccp_8h.md#typedef-syn_ccp_erasefn) | [**erase\_cb**](#variable-erase_cb)  <br> |
|  void \* | [**erase\_ctx**](#variable-erase_ctx)  <br> |
|  uintptr\_t | [**mta0\_addr**](#variable-mta0_addr)  <br> |
|  uint8\_t | [**mta0\_ext**](#variable-mta0_ext)  <br> |
|  uintptr\_t | [**mta1\_addr**](#variable-mta1_addr)  <br> |
|  uint8\_t | [**mta1\_ext**](#variable-mta1_ext)  <br> |
|  [**SYN\_CCP\_SeedFn**](syn__ccp_8h.md#typedef-syn_ccp_seedfn) | [**seed\_cb**](#variable-seed_cb)  <br> |
|  void \* | [**seed\_ctx**](#variable-seed_ctx)  <br> |
|  uint8\_t | [**selected\_daq**](#variable-selected_daq)  <br> |
|  uint8\_t | [**selected\_odt**](#variable-selected_odt)  <br> |
|  uint16\_t | [**station\_addr**](#variable-station_addr)  <br> |
|  [**SYN\_CCP\_UnlockFn**](syn__ccp_8h.md#typedef-syn_ccp_unlockfn) | [**unlock\_cb**](#variable-unlock_cb)  <br> |
|  void \* | [**unlock\_ctx**](#variable-unlock_ctx)  <br> |
|  uint8\_t | [**unlocked\_resources**](#variable-unlocked_resources)  <br> |












































## Public Attributes Documentation




### variable active\_cal\_page 

```C++
uint8_t SYN_CCP_Slave::active_cal_page;
```



Currently active calibration page 


        

<hr>



### variable connected 

```C++
bool SYN_CCP_Slave::connected;
```



Session connection state 


        

<hr>



### variable daq\_lists 

```C++
SYN_CCP_DAQList SYN_CCP_Slave::daq_lists[SYN_CCP_MAX_DAQ_LISTS];
```



Configured DAQ lists 


        

<hr>



### variable erase\_cb 

```C++
SYN_CCP_EraseFn SYN_CCP_Slave::erase_cb;
```



Flash/RAM memory erase callback 


        

<hr>



### variable erase\_ctx 

```C++
void* SYN_CCP_Slave::erase_ctx;
```



Context for erase callback 


        

<hr>



### variable mta0\_addr 

```C++
uintptr_t SYN_CCP_Slave::mta0_addr;
```



MTA0 target address 


        

<hr>



### variable mta0\_ext 

```C++
uint8_t SYN_CCP_Slave::mta0_ext;
```



MTA0 extension address 


        

<hr>



### variable mta1\_addr 

```C++
uintptr_t SYN_CCP_Slave::mta1_addr;
```



MTA1 target address 


        

<hr>



### variable mta1\_ext 

```C++
uint8_t SYN_CCP_Slave::mta1_ext;
```



MTA1 extension address 


        

<hr>



### variable seed\_cb 

```C++
SYN_CCP_SeedFn SYN_CCP_Slave::seed_cb;
```



Security seed generator callback 


        

<hr>



### variable seed\_ctx 

```C++
void* SYN_CCP_Slave::seed_ctx;
```



Context for seed callback 


        

<hr>



### variable selected\_daq 

```C++
uint8_t SYN_CCP_Slave::selected_daq;
```



Currently selected DAQ list index 


        

<hr>



### variable selected\_odt 

```C++
uint8_t SYN_CCP_Slave::selected_odt;
```



Currently selected ODT index 


        

<hr>



### variable station\_addr 

```C++
uint16_t SYN_CCP_Slave::station_addr;
```



CCP station address 


        

<hr>



### variable unlock\_cb 

```C++
SYN_CCP_UnlockFn SYN_CCP_Slave::unlock_cb;
```



Security unlock key verification callback 


        

<hr>



### variable unlock\_ctx 

```C++
void* SYN_CCP_Slave::unlock_ctx;
```



Context for unlock callback 


        

<hr>



### variable unlocked\_resources 

```C++
uint8_t SYN_CCP_Slave::unlocked_resources;
```



Bitmask of unlocked resources 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ccp.h`

