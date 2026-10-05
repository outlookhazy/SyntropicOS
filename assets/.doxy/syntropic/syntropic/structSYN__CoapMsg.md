

# Struct SYN\_CoapMsg



[**ClassList**](annotated.md) **>** [**SYN\_CoapMsg**](structSYN__CoapMsg.md)



_Parsed or to-be-serialized CoAP message header._ 

* `#include <syn_coap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**code**](#variable-code)  <br> |
|  uint16\_t | [**msg\_id**](#variable-msg_id)  <br> |
|  const uint8\_t \* | [**payload**](#variable-payload)  <br> |
|  size\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  uint8\_t | [**token**](#variable-token)  <br> |
|  uint8\_t | [**token\_len**](#variable-token_len)  <br> |
|  uint8\_t | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable code 

```C++
uint8_t SYN_CoapMsg::code;
```



Request method or response code 
 


        

<hr>



### variable msg\_id 

```C++
uint16_t SYN_CoapMsg::msg_id;
```



Message ID for deduplication 
 


        

<hr>



### variable payload 

```C++
const uint8_t* SYN_CoapMsg::payload;
```



Pointer to payload (NULL if none) 
 


        

<hr>



### variable payload\_len 

```C++
size_t SYN_CoapMsg::payload_len;
```



Payload length in bytes 
 


        

<hr>



### variable token 

```C++
uint8_t SYN_CoapMsg::token[8];
```



Token bytes 
 


        

<hr>



### variable token\_len 

```C++
uint8_t SYN_CoapMsg::token_len;
```



Token length (0–8) 
 


        

<hr>



### variable type 

```C++
uint8_t SYN_CoapMsg::type;
```



Message type (CON/NON/ACK/RST) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.h`

