

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
|  uintptr\_t | [**mta\_addr**](#variable-mta_addr)  <br> |
|  uint8\_t | [**mta\_ext**](#variable-mta_ext)  <br> |
|  uint16\_t | [**station\_id**](#variable-station_id)  <br> |
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



### variable station\_id 

```C++
uint16_t SYN_XCP_Slave::station_id;
```



XCP station ID 


        

<hr>



### variable unlocked\_resources 

```C++
uint8_t SYN_XCP_Slave::unlocked_resources;
```



Unlocked resources bitmask 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xcp.h`

