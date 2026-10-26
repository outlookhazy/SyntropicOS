

# Struct SYN\_DALI\_ForwardFrame



[**ClassList**](annotated.md) **>** [**SYN\_DALI\_ForwardFrame**](structSYN__DALI__ForwardFrame.md)



_DALI Forward Frame (Master -&gt; Control Gear, 16-bit payload)_ 

* `#include <syn_dali.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_DALI\_AddressType**](syn__dali_8h.md#enum-syn_dali_addresstype) | [**addr\_type**](#variable-addr_type)  <br> |
|  uint8\_t | [**address**](#variable-address)  <br> |
|  uint8\_t | [**data\_cmd**](#variable-data_cmd)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_direct**](#variable-is_direct)  <br> |
|  uint16\_t | [**raw**](#variable-raw)  <br> |












































## Public Attributes Documentation




### variable addr\_type 

```C++
SYN_DALI_AddressType SYN_DALI_ForwardFrame::addr_type;
```



Decoded address type 
 


        

<hr>



### variable address 

```C++
uint8_t SYN_DALI_ForwardFrame::address;
```



Target address value 
 


        

<hr>



### variable data\_cmd 

```C++
uint8_t SYN_DALI_ForwardFrame::data_cmd;
```



Direct Arc Level (0..254) or Cmd 


        

<hr>



### variable is\_direct 

```C++
bool SYN_DALI_ForwardFrame::is_direct;
```



true = Direct Arc Level, false = Cmd 


        

<hr>



### variable raw 

```C++
uint16_t SYN_DALI_ForwardFrame::raw;
```



Raw 16-bit payload 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dali.h`

