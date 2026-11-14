

# Struct UDS\_ReqCtx



[**ClassList**](annotated.md) **>** [**UDS\_ReqCtx**](structUDS__ReqCtx.md)



_Internal UDS request processing context._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_UDS\_AddrMode**](syn__uds_8h.md#enum-syn_uds_addrmode) | [**addr\_mode**](#variable-addr_mode)  <br> |
|  uint16\_t | [**max\_resp\_len**](#variable-max_resp_len)  <br> |
|  const uint8\_t \* | [**req**](#variable-req)  <br> |
|  uint16\_t | [**req\_len**](#variable-req_len)  <br> |
|  uint8\_t \* | [**resp\_buf**](#variable-resp_buf)  <br> |
|  uint16\_t \* | [**resp\_len**](#variable-resp_len)  <br> |
|  [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* | [**server**](#variable-server)  <br> |
|  uint8\_t | [**sid**](#variable-sid)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**success**](#variable-success)  <br> |












































## Public Attributes Documentation




### variable addr\_mode 

```C++
SYN_UDS_AddrMode UDS_ReqCtx::addr_mode;
```



Physical or functional addressing mode 


        

<hr>



### variable max\_resp\_len 

```C++
uint16_t UDS_ReqCtx::max_resp_len;
```



Capacity of resp\_buf in bytes 


        

<hr>



### variable req 

```C++
const uint8_t* UDS_ReqCtx::req;
```



Request PDU buffer 


        

<hr>



### variable req\_len 

```C++
uint16_t UDS_ReqCtx::req_len;
```



Request PDU byte length 


        

<hr>



### variable resp\_buf 

```C++
uint8_t* UDS_ReqCtx::resp_buf;
```



Response PDU output buffer 


        

<hr>



### variable resp\_len 

```C++
uint16_t* UDS_ReqCtx::resp_len;
```



Output response PDU byte length 


        

<hr>



### variable server 

```C++
SYN_UDS_Server* UDS_ReqCtx::server;
```



UDS server instance 


        

<hr>



### variable sid 

```C++
uint8_t UDS_ReqCtx::sid;
```



Request service identifier 


        

<hr>



### variable success 

```C++
bool UDS_ReqCtx::success;
```



Set true when positive response is built 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_uds.c`

