

# Struct SYN\_HttpClient



[**ClassList**](annotated.md) **>** [**SYN\_HttpClient**](structSYN__HttpClient.md)



_HTTP client context structure._ 

* `#include <syn_http.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t \* | [**body**](#variable-body)  <br> |
|  [**SYN\_HttpBodyCallback**](syn__http_8h.md#typedef-syn_httpbodycallback) | [**body\_cb**](#variable-body_cb)  <br> |
|  size\_t | [**body\_len**](#variable-body_len)  <br> |
|  uint32\_t | [**body\_remaining**](#variable-body_remaining)  <br> |
|  size\_t | [**body\_start**](#variable-body_start)  <br> |
|  uint32\_t | [**body\_timeout\_ms**](#variable-body_timeout_ms)  <br> |
|  size\_t | [**buf\_pos**](#variable-buf_pos)  <br> |
|  size\_t | [**buf\_used**](#variable-buf_used)  <br> |
|  void \* | [**cb\_ctx**](#variable-cb_ctx)  <br> |
|  char | [**chunk\_line**](#variable-chunk_line)  <br> |
|  size\_t | [**chunk\_line\_pos**](#variable-chunk_line_pos)  <br> |
|  uint32\_t | [**chunk\_remaining**](#variable-chunk_remaining)  <br> |
|  uint8\_t | [**chunk\_state**](#variable-chunk_state)  <br> |
|  const char \* | [**content\_type**](#variable-content_type)  <br> |
|  char | [**cur\_host**](#variable-cur_host)  <br> |
|  char | [**cur\_path**](#variable-cur_path)  <br> |
|  uint16\_t | [**cur\_port**](#variable-cur_port)  <br> |
|  uint8\_t | [**header\_count**](#variable-header_count)  <br> |
|  uint32\_t | [**header\_timeout\_ms**](#variable-header_timeout_ms)  <br> |
|  const [**SYN\_HttpHeader**](structSYN__HttpHeader.md) \* | [**headers**](#variable-headers)  <br> |
|  int | [**hops**](#variable-hops)  <br> |
|  const char \* | [**host**](#variable-host)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**known\_length**](#variable-known_length)  <br> |
|  size\_t | [**line\_len**](#variable-line_len)  <br> |
|  const char \* | [**method**](#variable-method)  <br> |
|  const char \* | [**path**](#variable-path)  <br> |
|  uint16\_t | [**port**](#variable-port)  <br> |
|  [**SYN\_HttpResponse**](structSYN__HttpResponse.md) | [**resp**](#variable-resp)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |
|  [**SYN\_HttpState**](syn__http_8h.md#enum-syn_httpstate) | [**state**](#variable-state)  <br> |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**status**](#variable-status)  <br> |
|  uint8\_t \* | [**work\_buf**](#variable-work_buf)  <br> |
|  size\_t | [**work\_buf\_size**](#variable-work_buf_size)  <br> |












































## Public Attributes Documentation




### variable body 

```C++
const uint8_t* SYN_HttpClient::body;
```



Request body buffer for POST, or NULL 


        

<hr>



### variable body\_cb 

```C++
SYN_HttpBodyCallback SYN_HttpClient::body_cb;
```



Callback function to stream response body 


        

<hr>



### variable body\_len 

```C++
size_t SYN_HttpClient::body_len;
```



Request body length, or 0 


        

<hr>



### variable body\_remaining 

```C++
uint32_t SYN_HttpClient::body_remaining;
```



Number of expected body bytes remaining 


        

<hr>



### variable body\_start 

```C++
size_t SYN_HttpClient::body_start;
```



Index in work\_buf where body data begins 


        

<hr>



### variable body\_timeout\_ms 

```C++
uint32_t SYN_HttpClient::body_timeout_ms;
```



Body receive timeout threshold 


        

<hr>



### variable buf\_pos 

```C++
size_t SYN_HttpClient::buf_pos;
```



Read cursor within the work\_buf 


        

<hr>



### variable buf\_used 

```C++
size_t SYN_HttpClient::buf_used;
```



Active bytes stored in work\_buf 


        

<hr>



### variable cb\_ctx 

```C++
void* SYN_HttpClient::cb_ctx;
```



User context for the body callback 


        

<hr>



### variable chunk\_line 

```C++
char SYN_HttpClient::chunk_line[32];
```



Parser buffer for reading chunk length text 


        

<hr>



### variable chunk\_line\_pos 

```C++
size_t SYN_HttpClient::chunk_line_pos;
```



Write cursor in chunk\_line 


        

<hr>



### variable chunk\_remaining 

```C++
uint32_t SYN_HttpClient::chunk_remaining;
```



Current chunk bytes remaining to read 


        

<hr>



### variable chunk\_state 

```C++
uint8_t SYN_HttpClient::chunk_state;
```



Internal parser state (0: size line, 1: data, 2: trailing CRLF) 


        

<hr>



### variable content\_type 

```C++
const char* SYN_HttpClient::content_type;
```



Content-Type for POST payload, or NULL 


        

<hr>



### variable cur\_host 

```C++
char SYN_HttpClient::cur_host[64];
```



Cached current redirect hostname 


        

<hr>



### variable cur\_path 

```C++
char SYN_HttpClient::cur_path[128];
```



Cached current redirect path 


        

<hr>



### variable cur\_port 

```C++
uint16_t SYN_HttpClient::cur_port;
```



Cached current redirect port 


        

<hr>



### variable header\_count 

```C++
uint8_t SYN_HttpClient::header_count;
```



Number of custom request headers 


        

<hr>



### variable header\_timeout\_ms 

```C++
uint32_t SYN_HttpClient::header_timeout_ms;
```



Header receive timeout threshold 


        

<hr>



### variable headers 

```C++
const SYN_HttpHeader* SYN_HttpClient::headers;
```



Optional array of custom request headers 


        

<hr>



### variable hops 

```C++
int SYN_HttpClient::hops;
```



Redirect hop counter to prevent crash loops 


        

<hr>



### variable host 

```C++
const char* SYN_HttpClient::host;
```



Hostname to query 


        

<hr>



### variable known\_length 

```C++
bool SYN_HttpClient::known_length;
```



True if Content-Length was provided 


        

<hr>



### variable line\_len 

```C++
size_t SYN_HttpClient::line_len;
```



Length of the current parsed line 


        

<hr>



### variable method 

```C++
const char* SYN_HttpClient::method;
```



HTTP method (e.g. "GET", "POST") 


        

<hr>



### variable path 

```C++
const char* SYN_HttpClient::path;
```



Resource path on the server (e.g. "/index.html") 


        

<hr>



### variable port 

```C++
uint16_t SYN_HttpClient::port;
```



Port to connect to (typically 80) 


        

<hr>



### variable resp 

```C++
SYN_HttpResponse SYN_HttpClient::resp;
```



Parsed response status and headers 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_HttpClient::sock;
```



Client TCP network socket handle 


        

<hr>



### variable state 

```C++
SYN_HttpState SYN_HttpClient::state;
```



Current client state machine state 


        

<hr>



### variable status 

```C++
SYN_Status SYN_HttpClient::status;
```



Final transaction execution status 


        

<hr>



### variable work\_buf 

```C++
uint8_t* SYN_HttpClient::work_buf;
```



Work buffer for socket transmission and parsing 


        

<hr>



### variable work\_buf\_size 

```C++
size_t SYN_HttpClient::work_buf_size;
```



Capacity of the work buffer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_http.h`

