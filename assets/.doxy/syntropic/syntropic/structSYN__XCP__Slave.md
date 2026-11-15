

# Struct SYN\_XCP\_Slave



[**ClassList**](annotated.md) **>** [**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md)



_XCP Slave instance context._ 

* `#include <syn_xcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**connected**](#variable-connected)  <br> |
|  uint8\_t | [**current\_daq\_ptr\_idx**](#variable-current_daq_ptr_idx)  <br> |
|  uint8\_t | [**current\_daq\_ptr\_list**](#variable-current_daq_ptr_list)  <br> |
|  uint8\_t | [**current\_daq\_ptr\_odt**](#variable-current_daq_ptr_odt)  <br> |
|  [**SYN\_XCP\_DAQList**](structSYN__XCP__DAQList.md) | [**daq\_lists**](#variable-daq_lists)  <br> |
|  uint8\_t | [**daq\_tick\_ch**](#variable-daq_tick_ch)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**daq\_tick\_drained**](#variable-daq_tick_drained)  <br> |
|  [**SYN\_XCP\_MemRegion**](structSYN__XCP__MemRegion.md) | [**mem\_regions**](#variable-mem_regions)  <br> |
|  uintptr\_t | [**mta\_addr**](#variable-mta_addr)  <br> |
|  uint8\_t | [**mta\_ext**](#variable-mta_ext)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mta\_host\_valid**](#variable-mta_host_valid)  <br> |
|  uint8\_t | [**pending\_unlock\_resource**](#variable-pending_unlock_resource)  <br> |
|  uint8\_t | [**region\_count**](#variable-region_count)  <br> |
|  uint8\_t | [**rr\_daq\_idx**](#variable-rr_daq_idx)  <br> |
|  [**SYN\_XCP\_SeedFn**](syn__xcp_8h.md#typedef-syn_xcp_seedfn) | [**seed\_cb**](#variable-seed_cb)  <br> |
|  void \* | [**seed\_ctx**](#variable-seed_ctx)  <br> |
|  uint16\_t | [**station\_id**](#variable-station_id)  <br> |
|  [**SYN\_XCP\_UnlockFn**](syn__xcp_8h.md#typedef-syn_xcp_unlockfn) | [**unlock\_cb**](#variable-unlock_cb)  <br> |
|  void \* | [**unlock\_ctx**](#variable-unlock_ctx)  <br> |
|  uint8\_t | [**unlocked\_resources**](#variable-unlocked_resources)  <br> |












































## Public Attributes Documentation




### variable connected 

```C++
bool SYN_XCP_Slave::connected;
```



Connection state 


        

<hr>



### variable current\_daq\_ptr\_idx 

```C++
uint8_t SYN_XCP_Slave::current_daq_ptr_idx;
```



Active DAQ pointer entry index 


        

<hr>



### variable current\_daq\_ptr\_list 

```C++
uint8_t SYN_XCP_Slave::current_daq_ptr_list;
```



Active DAQ pointer list index 


        

<hr>



### variable current\_daq\_ptr\_odt 

```C++
uint8_t SYN_XCP_Slave::current_daq_ptr_odt;
```



Active DAQ pointer ODT index 


        

<hr>



### variable daq\_lists 

```C++
SYN_XCP_DAQList SYN_XCP_Slave::daq_lists[SYN_XCP_MAX_DAQ_LISTS];
```



Configured DAQ lists array 


        

<hr>



### variable daq\_tick\_ch 

```C++
uint8_t SYN_XCP_Slave::daq_tick_ch;
```



Event channel of completed tick drain 


        

<hr>



### variable daq\_tick\_drained 

```C++
bool SYN_XCP_Slave::daq_tick_drained;
```



True once current event tick ODTs drained 


        

<hr>



### variable mem\_regions 

```C++
SYN_XCP_MemRegion SYN_XCP_Slave::mem_regions[SYN_XCP_MAX_MEM_REGIONS];
```



Registered memory regions 


        

<hr>



### variable mta\_addr 

```C++
uintptr_t SYN_XCP_Slave::mta_addr;
```



Memory transfer target address 


        

<hr>



### variable mta\_ext 

```C++
uint8_t SYN_XCP_Slave::mta_ext;
```



Memory transfer address extension 


        

<hr>



### variable mta\_host\_valid 

```C++
bool SYN_XCP_Slave::mta_host_valid;
```



True if MTA holds host pointer 


        

<hr>



### variable pending\_unlock\_resource 

```C++
uint8_t SYN_XCP_Slave::pending_unlock_resource;
```



Resource requested in last GET\_SEED 


        

<hr>



### variable region\_count 

```C++
uint8_t SYN_XCP_Slave::region_count;
```



Active memory region count 


        

<hr>



### variable rr\_daq\_idx 

```C++
uint8_t SYN_XCP_Slave::rr_daq_idx;
```



Round-robin DAQ list cursor 


        

<hr>



### variable seed\_cb 

```C++
SYN_XCP_SeedFn SYN_XCP_Slave::seed_cb;
```



Optional seed generator callback 


        

<hr>



### variable seed\_ctx 

```C++
void* SYN_XCP_Slave::seed_ctx;
```



Seed callback context 


        

<hr>



### variable station\_id 

```C++
uint16_t SYN_XCP_Slave::station_id;
```



XCP station ID 


        

<hr>



### variable unlock\_cb 

```C++
SYN_XCP_UnlockFn SYN_XCP_Slave::unlock_cb;
```



Optional key verification callback 


        

<hr>



### variable unlock\_ctx 

```C++
void* SYN_XCP_Slave::unlock_ctx;
```



Unlock callback context 


        

<hr>



### variable unlocked\_resources 

```C++
uint8_t SYN_XCP_Slave::unlocked_resources;
```



Unlocked resources bitmask 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xcp.h`

