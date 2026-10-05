

# Struct SYN\_MSP\_Parser



[**ClassList**](annotated.md) **>** [**SYN\_MSP\_Parser**](structSYN__MSP__Parser.md)



[More...](#detailed-description)

* `#include <syn_msp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**checksum**](#variable-checksum)  <br> |
|  uint32\_t | [**checksum\_errors**](#variable-checksum_errors)  <br> |
|  uint8\_t | [**cmd**](#variable-cmd)  <br> |
|  uint8\_t | [**dir\_char**](#variable-dir_char)  <br> |
|  uint32\_t | [**frames\_received**](#variable-frames_received)  <br> |
|  [**SYN\_MSP\_Frame**](structSYN__MSP__Frame.md) | [**last\_frame**](#variable-last_frame)  <br> |
|  uint8\_t | [**payload**](#variable-payload)  <br> |
|  uint8\_t | [**payload\_idx**](#variable-payload_idx)  <br> |
|  uint8\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  uint8\_t | [**state**](#variable-state)  <br> |












































## Detailed Description


MSP Streaming Parser Instance. 


    
## Public Attributes Documentation




### variable checksum 

```C++
uint8_t SYN_MSP_Parser::checksum;
```



Running XOR checksum accumulator 


        

<hr>



### variable checksum\_errors 

```C++
uint32_t SYN_MSP_Parser::checksum_errors;
```



Count of checksum mismatch errors 


        

<hr>



### variable cmd 

```C++
uint8_t SYN_MSP_Parser::cmd;
```



Received command ID 


        

<hr>



### variable dir\_char 

```C++
uint8_t SYN_MSP_Parser::dir_char;
```



Received direction character 


        

<hr>



### variable frames\_received 

```C++
uint32_t SYN_MSP_Parser::frames_received;
```



Total valid frames decoded 


        

<hr>



### variable last\_frame 

```C++
SYN_MSP_Frame SYN_MSP_Parser::last_frame;
```



Last decoded frame 


        

<hr>



### variable payload 

```C++
uint8_t SYN_MSP_Parser::payload[SYN_MSP_MAX_PAYLOAD];
```



Payload assembly buffer 


        

<hr>



### variable payload\_idx 

```C++
uint8_t SYN_MSP_Parser::payload_idx;
```



Payload write index 


        

<hr>



### variable payload\_len 

```C++
uint8_t SYN_MSP_Parser::payload_len;
```



Expected payload length 


        

<hr>



### variable state 

```C++
uint8_t SYN_MSP_Parser::state;
```



Parser state machine step 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_msp.h`

