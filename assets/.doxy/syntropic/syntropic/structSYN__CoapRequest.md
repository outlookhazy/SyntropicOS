

# Struct SYN\_CoapRequest



[**ClassList**](annotated.md) **>** [**SYN\_CoapRequest**](structSYN__CoapRequest.md)



_CoAP client request context._ [More...](#detailed-description)

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
|  [**SYN\_SockAddr**](structSYN__SockAddr.md) | [**server\_addr**](#variable-server_addr)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |
|  uint32\_t | [**start\_ms**](#variable-start_ms)  <br> |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**status**](#variable-status)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_len**](#variable-tx_len)  <br> |












































## Detailed Description


Populate the input fields, then register as a scheduler task using syn\_coap\_request\_task. On completion, check `status` and read the response from `resp_msg`. 


    
## Public Attributes Documentation




### variable backoff 

```C++
SYN_Backoff SYN_CoapRequest::backoff;
```



Retransmit backoff state 
 


        

<hr>



### variable req\_msg 

```C++
const SYN_CoapMsg* SYN_CoapRequest::req_msg;
```



Request message to send 
 


        

<hr>



### variable req\_option\_count 

```C++
size_t SYN_CoapRequest::req_option_count;
```



Number of request options 
 


        

<hr>



### variable req\_options 

```C++
const SYN_CoapOption* SYN_CoapRequest::req_options;
```



Request options array 
 


        

<hr>



### variable resp\_buf 

```C++
uint8_t SYN_CoapRequest::resp_buf[256];
```



Raw response packet buffer 
 


        

<hr>



### variable resp\_len 

```C++
size_t SYN_CoapRequest::resp_len;
```



Raw response length in bytes 
 


        

<hr>



### variable resp\_msg 

```C++
SYN_CoapMsg SYN_CoapRequest::resp_msg;
```



Parsed response message 
 


        

<hr>



### variable resp\_option\_count 

```C++
size_t SYN_CoapRequest::resp_option_count;
```



Number of parsed response options 
 


        

<hr>



### variable resp\_options 

```C++
SYN_CoapOption SYN_CoapRequest::resp_options[8];
```



Parsed response options 
 


        

<hr>



### variable server\_addr 

```C++
SYN_SockAddr SYN_CoapRequest::server_addr;
```



Destination UDP address 
 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_CoapRequest::sock;
```



UDP socket handle 
 


        

<hr>



### variable start\_ms 

```C++
uint32_t SYN_CoapRequest::start_ms;
```



Tick at start of current send attempt 


        

<hr>



### variable status 

```C++
SYN_Status SYN_CoapRequest::status;
```



Final result (SYN\_OK / SYN\_TIMEOUT / SYN\_ERROR) 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_CoapRequest::tx_buf[256];
```



Serialized request (persists across retries) 


        

<hr>



### variable tx\_len 

```C++
size_t SYN_CoapRequest::tx_len;
```



Length of serialized request 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.h`

