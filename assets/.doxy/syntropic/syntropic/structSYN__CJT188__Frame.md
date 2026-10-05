

# Struct SYN\_CJT188\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_CJT188\_Frame**](structSYN__CJT188__Frame.md)



_Decoded CJ/T 188 Protocol Frame._ 

* `#include <syn_cjt188.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**ctrl**](#variable-ctrl)  <br> |
|  uint16\_t | [**data\_id**](#variable-data_id)  <br> |
|  uint8\_t | [**len**](#variable-len)  <br> |
|  uint8\_t | [**meter\_id**](#variable-meter_id)  <br> |
|  uint8\_t | [**meter\_type**](#variable-meter_type)  <br> |
|  const uint8\_t \* | [**payload**](#variable-payload)  <br> |
|  size\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  uint8\_t | [**seq**](#variable-seq)  <br> |
|  uint8\_t | [**vendor\_id**](#variable-vendor_id)  <br> |












































## Public Attributes Documentation




### variable ctrl 

```C++
uint8_t SYN_CJT188_Frame::ctrl;
```



Control code (0x01, 0x81, 0x04, etc.) 
 


        

<hr>



### variable data\_id 

```C++
uint16_t SYN_CJT188_Frame::data_id;
```



16-bit Data ID (e.g. 0x901F) 
 


        

<hr>



### variable len 

```C++
uint8_t SYN_CJT188_Frame::len;
```



Data field length 
 


        

<hr>



### variable meter\_id 

```C++
uint8_t SYN_CJT188_Frame::meter_id[5];
```



5-byte BCD meter ID (reverse order) 
 


        

<hr>



### variable meter\_type 

```C++
uint8_t SYN_CJT188_Frame::meter_type;
```



Meter type (0x10 cold water, etc.) 
 


        

<hr>



### variable payload 

```C++
const uint8_t* SYN_CJT188_Frame::payload;
```



Pointer to data payload inside frame 
 


        

<hr>



### variable payload\_len 

```C++
size_t SYN_CJT188_Frame::payload_len;
```



Length of payload (len - 3) 
 


        

<hr>



### variable seq 

```C++
uint8_t SYN_CJT188_Frame::seq;
```



Sequence number 
 


        

<hr>



### variable vendor\_id 

```C++
uint8_t SYN_CJT188_Frame::vendor_id[2];
```



2-byte manufacturer code 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cjt188.h`

