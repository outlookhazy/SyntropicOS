

# Struct SYN\_CoapTransportRequest



[**ClassList**](annotated.md) **>** [**SYN\_CoapTransportRequest**](structSYN__CoapTransportRequest.md)



_CoAP request context over generic_ [_**SYN\_Transport**_](structSYN__Transport.md) _(e.g. DTLS 1.3 / UDP / Serial)._

* `#include <syn_coap.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Backoff**](structSYN__Backoff.md) | [**backoff**](#variable-backoff)  <br> |
|  const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* | [**req\_msg**](#variable-req_msg)  <br> |
|  size\_t | [**req\_option\_count**](#variable-req_option_count)  <br> |
|  const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* | [**req\_options**](#variable-req_options)  <br> |
|  uint8\_t | [**resp\_buf**](#variable-resp_buf)  <br> |
|  size\_t | [**resp\_len**](#variable-resp_len)  <br> |
|  [**SYN\_CoapMsg**](structSYN__CoapMsg.md) | [**resp\_msg**](#variable-resp_msg)  <br> |
|  size\_t | [**resp\_option\_count**](#variable-resp_option_count)  <br> |
|  [**SYN\_CoapOption**](structSYN__CoapOption.md) | [**resp\_options**](#variable-resp_options)  <br> |
|  uint32\_t | [**start\_ms**](#variable-start_ms)  <br> |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**status**](#variable-status)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**transport**](#variable-transport)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_len**](#variable-tx_len)  <br> |












































## Public Attributes Documentation




### variable backoff 

```C++
SYN_Backoff SYN_CoapTransportRequest::backoff;
```



Retransmit backoff state 


        

<hr>



### variable req\_msg 

```C++
const SYN_CoapMsg* SYN_CoapTransportRequest::req_msg;
```



Request message to send 


        

<hr>



### variable req\_option\_count 

```C++
size_t SYN_CoapTransportRequest::req_option_count;
```



Number of request options 


        

<hr>



### variable req\_options 

```C++
const SYN_CoapOption* SYN_CoapTransportRequest::req_options;
```



Request options array 


        

<hr>



### variable resp\_buf 

```C++
uint8_t SYN_CoapTransportRequest::resp_buf[256];
```



Raw response packet buffer 


        

<hr>



### variable resp\_len 

```C++
size_t SYN_CoapTransportRequest::resp_len;
```



Raw response length in bytes 


        

<hr>



### variable resp\_msg 

```C++
SYN_CoapMsg SYN_CoapTransportRequest::resp_msg;
```



Parsed response message 


        

<hr>



### variable resp\_option\_count 

```C++
size_t SYN_CoapTransportRequest::resp_option_count;
```



Number of parsed response options 


        

<hr>



### variable resp\_options 

```C++
SYN_CoapOption SYN_CoapTransportRequest::resp_options[8];
```



Parsed response options 


        

<hr>



### variable start\_ms 

```C++
uint32_t SYN_CoapTransportRequest::start_ms;
```



Timestamp of current attempt 


        

<hr>



### variable status 

```C++
SYN_Status SYN_CoapTransportRequest::status;
```



Final request status 


        

<hr>



### variable transport 

```C++
SYN_Transport* SYN_CoapTransportRequest::transport;
```



Transport interface 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_CoapTransportRequest::tx_buf[256];
```



Serialized request buffer 


        

<hr>



### variable tx\_len 

```C++
size_t SYN_CoapTransportRequest::tx_len;
```



Serialized request length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.h`

