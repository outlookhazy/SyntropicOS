

# Struct SYN\_IOLink\_DirectParams



[**ClassList**](annotated.md) **>** [**SYN\_IOLink\_DirectParams**](structSYN__IOLink__DirectParams.md)



_Direct Parameter Page 1 structure._ 

* `#include <syn_iolink.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**device\_id**](#variable-device_id)  <br> |
|  uint8\_t | [**master\_cycle\_time**](#variable-master_cycle_time)  <br> |
|  uint8\_t | [**min\_cycle\_time**](#variable-min_cycle_time)  <br> |
|  uint8\_t | [**mseq\_capability**](#variable-mseq_capability)  <br> |
|  uint8\_t | [**pd\_in\_len**](#variable-pd_in_len)  <br> |
|  uint8\_t | [**pd\_out\_len**](#variable-pd_out_len)  <br> |
|  uint8\_t | [**revision\_id**](#variable-revision_id)  <br> |
|  uint16\_t | [**vendor\_id**](#variable-vendor_id)  <br> |












































## Public Attributes Documentation




### variable device\_id 

```C++
uint32_t SYN_IOLink_DirectParams::device_id;
```



24-bit Device Identifier 


        

<hr>



### variable master\_cycle\_time 

```C++
uint8_t SYN_IOLink_DirectParams::master_cycle_time;
```



Configured master cycle time 


        

<hr>



### variable min\_cycle\_time 

```C++
uint8_t SYN_IOLink_DirectParams::min_cycle_time;
```



Minimum supported device cycle time 


        

<hr>



### variable mseq\_capability 

```C++
uint8_t SYN_IOLink_DirectParams::mseq_capability;
```



Supported M-Sequence capability 


        

<hr>



### variable pd\_in\_len 

```C++
uint8_t SYN_IOLink_DirectParams::pd_in_len;
```



Process data input length in bytes 


        

<hr>



### variable pd\_out\_len 

```C++
uint8_t SYN_IOLink_DirectParams::pd_out_len;
```



Process data output length in bytes 


        

<hr>



### variable revision\_id 

```C++
uint8_t SYN_IOLink_DirectParams::revision_id;
```



IO-Link revision ID (0x10 or 0x11) 


        

<hr>



### variable vendor\_id 

```C++
uint16_t SYN_IOLink_DirectParams::vendor_id;
```



16-bit Vendor Identifier 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_iolink.h`

