

# Struct SYN\_MSP\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_MSP\_Frame**](structSYN__MSP__Frame.md)



[More...](#detailed-description)

* `#include <syn_msp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**cmd**](#variable-cmd)  <br> |
|  uint8\_t | [**dir\_char**](#variable-dir_char)  <br> |
|  uint8\_t | [**payload**](#variable-payload)  <br> |
|  uint8\_t | [**payload\_len**](#variable-payload_len)  <br> |












































## Detailed Description


Parsed MSP Frame. 


    
## Public Attributes Documentation




### variable cmd 

```C++
uint8_t SYN_MSP_Frame::cmd;
```



Command ID (0..255). 


        

<hr>



### variable dir\_char 

```C++
uint8_t SYN_MSP_Frame::dir_char;
```



'&lt;', '&gt;', or '!'. 


        

<hr>



### variable payload 

```C++
uint8_t SYN_MSP_Frame::payload[SYN_MSP_MAX_PAYLOAD];
```



Payload buffer. 


        

<hr>



### variable payload\_len 

```C++
uint8_t SYN_MSP_Frame::payload_len;
```



Payload byte length. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_msp.h`

