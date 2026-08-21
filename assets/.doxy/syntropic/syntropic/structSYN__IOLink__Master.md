

# Struct SYN\_IOLink\_Master



[**ClassList**](annotated.md) **>** [**SYN\_IOLink\_Master**](structSYN__IOLink__Master.md)



_IO-Link Master instance context._ 

* `#include <syn_iolink.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_IOLink\_MasterConfig**](structSYN__IOLink__MasterConfig.md) | [**cfg**](#variable-cfg)  <br> |
|  uint8\_t | [**current\_pd\_in**](#variable-current_pd_in)  <br> |
|  size\_t | [**current\_pd\_in\_len**](#variable-current_pd_in_len)  <br> |
|  uint8\_t | [**current\_pd\_out**](#variable-current_pd_out)  <br> |
|  size\_t | [**current\_pd\_out\_len**](#variable-current_pd_out_len)  <br> |
|  [**SYN\_IOLink\_DirectParams**](structSYN__IOLink__DirectParams.md) | [**dev\_params**](#variable-dev_params)  <br> |
|  uint32\_t | [**last\_cycle\_ms**](#variable-last_cycle_ms)  <br> |
|  [**SYN\_IOLink\_MSeqType**](syn__iolink_8h.md#enum-syn_iolink_mseqtype) | [**mseq\_type**](#variable-mseq_type)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**pt**](#variable-pt)  <br> |
|  [**SYN\_IOLink\_PortState**](syn__iolink_8h.md#enum-syn_iolink_portstate) | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_IOLink_MasterConfig SYN_IOLink_Master::cfg;
```



Configuration 


        

<hr>



### variable current\_pd\_in 

```C++
uint8_t SYN_IOLink_Master::current_pd_in[SYN_IOLINK_MAX_PD_LEN];
```



Latest input process data 


        

<hr>



### variable current\_pd\_in\_len 

```C++
size_t SYN_IOLink_Master::current_pd_in_len;
```



Input process data length 


        

<hr>



### variable current\_pd\_out 

```C++
uint8_t SYN_IOLink_Master::current_pd_out[SYN_IOLINK_MAX_PD_LEN];
```



Active output process data 


        

<hr>



### variable current\_pd\_out\_len 

```C++
size_t SYN_IOLink_Master::current_pd_out_len;
```



Output process data length 


        

<hr>



### variable dev\_params 

```C++
SYN_IOLink_DirectParams SYN_IOLink_Master::dev_params;
```



Discovered device parameters 


        

<hr>



### variable last\_cycle\_ms 

```C++
uint32_t SYN_IOLink_Master::last_cycle_ms;
```



Timestamp of last cycle execution 


        

<hr>



### variable mseq\_type 

```C++
SYN_IOLink_MSeqType SYN_IOLink_Master::mseq_type;
```



Active M-Sequence type 


        

<hr>



### variable pt 

```C++
SYN_PT SYN_IOLink_Master::pt;
```



Protothread state 


        

<hr>



### variable state 

```C++
SYN_IOLink_PortState SYN_IOLink_Master::state;
```



Active port state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_iolink.h`

