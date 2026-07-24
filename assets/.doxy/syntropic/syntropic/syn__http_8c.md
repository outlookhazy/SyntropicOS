

# File syn\_http.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_http.c**](syn__http_8c.md)

[Go to the source code of this file](syn__http_8c_source.md)

_Cooperative HTTP/1.1 client implementation._ 

* `#include "syn_http.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_fmt.h"`
* `#include "../port/syn_port_system.h"`
* `#include <string.h>`
* `#include <stdlib.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_http\_client\_init**](#function-syn_http_client_init) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* client, const char \* method, const char \* host, uint16\_t port, const char \* path, const char \* content\_type, const uint8\_t \* body, size\_t body\_len, const [**SYN\_HttpHeader**](structSYN__HttpHeader.md) \* headers, uint8\_t header\_count, [**SYN\_HttpBodyCallback**](syn__http_8h.md#typedef-syn_httpbodycallback) body\_cb, void \* cb\_ctx, uint8\_t \* work\_buf, size\_t work\_buf\_size) <br>_Initialize the HTTP client struct._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_http\_client\_task**](#function-syn_http_client_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task to drive the HTTP client._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**parse\_redirect\_url**](#function-parse_redirect_url) (const char \* url, const char \* orig\_host, uint16\_t orig\_port, char \* host\_out, size\_t host\_sz, char \* path\_out, size\_t path\_sz, uint16\_t \* port\_out) <br>_Parse a redirect Location header URL._  |
|  uint32\_t | [**parse\_uint**](#function-parse_uint) (const char \* s) <br>_Parse a decimal unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**prefix\_icase**](#function-prefix_icase) (const char \* str, const char \* prefix) <br>_Case-insensitive prefix match._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**send\_request**](#function-send_request) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const char \* method, const char \* host, const char \* path, const [**SYN\_HttpHeader**](structSYN__HttpHeader.md) \* headers, uint8\_t header\_count, const char \* content\_type, size\_t content\_length) <br>_Build and send an HTTP request line + headers._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**sock\_write\_str**](#function-sock_write_str) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const char \* str) <br>_Write a null-terminated string to the socket._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**HTTP\_RECV\_TIMEOUT\_MS**](syn__http_8c.md#define-http_recv_timeout_ms)  `10000`<br> |

## Public Functions Documentation




### function syn\_http\_client\_init 

_Initialize the HTTP client struct._ 
```C++
SYN_Status syn_http_client_init (
    SYN_HttpClient * client,
    const char * method,
    const char * host,
    uint16_t port,
    const char * path,
    const char * content_type,
    const uint8_t * body,
    size_t body_len,
    const SYN_HttpHeader * headers,
    uint8_t header_count,
    SYN_HttpBodyCallback body_cb,
    void * cb_ctx,
    uint8_t * work_buf,
    size_t work_buf_size
) 
```



Configures the request metadata, target destination, custom headers, streaming callback, and temporary work buffer.




**Parameters:**


* `client` Pointer to the client context to initialize. 
* `method` HTTP method to perform (e.g. "GET" or "POST"). 
* `host` Destination hostname or IP address string. 
* `port` Destination TCP port (e.g. 80). 
* `path` Resource URL path (e.g. "/api/v1/update"). 
* `content_type` Type of content if body is present (e.g. "application/json"). 
* `body` Pointer to binary data to transmit as request body (or NULL). 
* `body_len` Length of the request body in bytes. 
* `headers` Optional array of custom HTTP headers to append. 
* `header_count` Number of custom headers in the array. 
* `body_cb` Callback function for streaming response chunks. 
* `cb_ctx` User context passed through to the body callback. 
* `work_buf` Working buffer for socket buffering and parsing. 
* `work_buf_size` Size of the working buffer in bytes. 



**Returns:**

SYN\_OK on success, or an error code on invalid parameters. 





        

<hr>



### function syn\_http\_client\_task 

_Cooperative task to drive the HTTP client._ 
```C++
SYN_PT_Status syn_http_client_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Yields while resolving, connecting, transmitting requests, and streaming the response body chunks. Must be run inside the cooperative scheduler.




**Parameters:**


* `pt` Pointer to the cooperative protothread structure. 
* `task` Pointer to the corresponding task control block. 



**Returns:**

PT\_WAITING, PT\_EXITED, or another protothread status code. 





        

<hr>
## Public Static Functions Documentation




### function parse\_redirect\_url 

_Parse a redirect Location header URL._ 
```C++
static void parse_redirect_url (
    const char * url,
    const char * orig_host,
    uint16_t orig_port,
    char * host_out,
    size_t host_sz,
    char * path_out,
    size_t path_sz,
    uint16_t * port_out
) 
```





**Parameters:**


* `url` Location URL string. 
* `orig_host` Original request host. 
* `orig_port` Original request port. 
* `host_out` [out] Redirected host. 
* `host_sz` Host buffer size. 
* `path_out` [out] Redirected path. 
* `path_sz` Path buffer size. 
* `port_out` [out] Redirected port. 




        

<hr>



### function parse\_uint 

_Parse a decimal unsigned integer._ 
```C++
static uint32_t parse_uint (
    const char * s
) 
```





**Parameters:**


* `s` Input string. 



**Returns:**

Parsed value. 





        

<hr>



### function prefix\_icase 

_Case-insensitive prefix match._ 
```C++
static bool prefix_icase (
    const char * str,
    const char * prefix
) 
```





**Parameters:**


* `str` String to test. 
* `prefix` Prefix to match. 



**Returns:**

true if match. 





        

<hr>



### function send\_request 

_Build and send an HTTP request line + headers._ 
```C++
static bool send_request (
    SYN_Socket sock,
    const char * method,
    const char * host,
    const char * path,
    const SYN_HttpHeader * headers,
    uint8_t header_count,
    const char * content_type,
    size_t content_length
) 
```





**Parameters:**


* `sock` Socket. 
* `method` HTTP method string. 
* `host` Host header value. 
* `path` Request path. 
* `headers` Custom headers array. 
* `header_count` Number of custom headers. 
* `content_type` Content-Type (or NULL). 
* `content_length` Body length (0 for no body). 



**Returns:**

true on success. 





        

<hr>



### function sock\_write\_str 

_Write a null-terminated string to the socket._ 
```C++
static bool sock_write_str (
    SYN_Socket sock,
    const char * str
) 
```





**Parameters:**


* `sock` Socket. 
* `str` String to send. 



**Returns:**

true if all bytes sent. 





        

<hr>
## Macro Definition Documentation





### define HTTP\_RECV\_TIMEOUT\_MS 

```C++
#define HTTP_RECV_TIMEOUT_MS `10000`
```



HTTP receive timeout (ms). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_http.c`

