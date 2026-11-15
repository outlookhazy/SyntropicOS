

# File syn\_http.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_http.c**](syn__http_8c.md)

[Go to the source code of this file](syn__http_8c_source.md)

_Cooperative HTTP/1.1 client implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "../sched/syn_sched.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_fmt.h"`
* `#include "syn_http.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_http\_client\_init**](#function-syn_http_client_init) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* client, const char \* method, const char \* host, uint16\_t port, const char \* path, const char \* content\_type, const uint8\_t \* body, size\_t body\_len, const [**SYN\_HttpHeader**](structSYN__HttpHeader.md) \* headers, uint8\_t header\_count, [**SYN\_HttpBodyCallback**](syn__http_8h.md#typedef-syn_httpbodycallback) body\_cb, void \* cb\_ctx, uint8\_t \* work\_buf, size\_t work\_buf\_size) <br>_Initialize the HTTP client struct._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_http\_client\_task**](#function-syn_http_client_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task to drive the HTTP client._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_apply\_redirect**](#function-http_apply_redirect) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Apply redirect hop rules, stripping cross-origin headers and converting POST to GET._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_body\_wake\_ready**](#function-http_body_wake_ready) (const [**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Check wakeup predicate for body receive wait._  |
|  void | [**http\_close\_sock**](#function-http_close_sock) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Close and invalidate client socket if open._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_connect\_and\_send**](#function-http_connect_and_send) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Open connection, transmit request headers and optional body, and init header state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_consume\_chunk\_crlf**](#function-http_consume_chunk_crlf) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c, uint8\_t ch) <br>_Validate exact post-chunk CRLF delimiter bytes in chunk\_state 1/3._  |
|  int | [**http\_consume\_chunked\_buf**](#function-http_consume_chunked_buf) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Consume buffered bytes in c-&gt;work\_buf for chunked transfer decoding._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_drain\_initial\_body**](#function-http_drain_initial_body) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Deliver any initial body bytes buffered alongside HTTP response headers._  |
|  int | [**http\_fail**](#function-http_fail) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c, [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) status) <br>_Close socket, transition client to error state, and return -1._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_feed\_chunk\_size\_byte**](#function-http_feed_chunk_size_byte) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c, uint8\_t ch) <br>_Feed one byte in chunk\_state == 0 (chunk size header line)._  |
|  int | [**http\_feed\_trailer\_byte**](#function-http_feed_trailer_byte) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c, uint8\_t ch) <br>_Consume one trailer-section byte in chunk\_state 2 (line) or 4 (post-CR)._  |
|  int | [**http\_handle\_body\_recv\_neg**](#function-http_handle_body_recv_neg) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c, [**bool**](syn__defs_8h.md#enum-bool) was\_readable) <br>_Handle negative socket recv return during HTTP body streaming._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_header\_wake\_ready**](#function-http_header_wake_ready) (const [**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Check wakeup predicate for header receive wait._  |
|  int | [**http\_hex\_nibble**](#function-http_hex_nibble) (char hc) <br>_Convert ASCII hexadecimal digit to nibble value [0..15] or -1 if invalid._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_parse\_chunk\_hex**](#function-http_parse_chunk_hex) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Parse hexadecimal chunk length from c-&gt;chunk\_line into c-&gt;chunk\_remaining._  |
|  void | [**http\_parse\_header\_line**](#function-http_parse_header_line) ([**SYN\_HttpResponse**](structSYN__HttpResponse.md) \* resp, const char \* cur) <br>_Parse a single HTTP response header line into_ [_**SYN\_HttpResponse**_](structSYN__HttpResponse.md) _._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_parse\_port**](#function-http_parse_port) (const char \* p, uint16\_t \* port\_out) <br>_Parse a bounded TCP port [1..65535] stopping at '/' or NUL._  |
|  int | [**http\_prepare\_response**](#function-http_prepare_response) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Parse buffered response headers and handle redirects or body setup._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_response\_is\_bodyless**](#function-http_response_is_bodyless) (const char \* method, int status\_code) <br>_Return true if response never carries a body (HEAD, 1xx, 204, 304)._  |
|  const char \* | [**http\_skip\_spaces**](#function-http_skip_spaces) (const char \* s) <br>_Advance past leading optional whitespace (SP or HTAB) in a header value._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_sock\_readable**](#function-http_sock_readable) (const [**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Check whether client socket is valid and has pending readability._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_status\_is\_bodyless**](#function-http_status_is_bodyless) (int status\_code) <br>_Return true if response status code never carries a message body (RFC 9112 Section 6.3)._  |
|  int | [**http\_step\_body**](#function-http_step_body) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Dispatch one body streaming step according to response framing mode._  |
|  int | [**http\_step\_chunked\_body**](#function-http_step_chunked_body) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Advance chunked transfer decoding by consuming buffered bytes or reading socket._  |
|  int | [**http\_step\_headers**](#function-http_step_headers) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Perform one non-blocking header accumulation step._  |
|  int | [**http\_step\_stream\_body**](#function-http_step_stream_body) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* c) <br>_Advance Content-Length or EOF-delimited body streaming by one step._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**http\_task\_args\_valid**](#function-http_task_args_valid) (const [**SYN\_PT**](structSYN__PT.md) \* pt, const [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Validate task arguments and working buffer constraints._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**parse\_redirect\_url**](#function-parse_redirect_url) (const char \* url, const char \* orig\_host, uint16\_t orig\_port, char \* host\_out, size\_t host\_sz, char \* path\_out, size\_t path\_sz, uint16\_t \* port\_out) <br>_Parse a redirect Location header URL, rejecting non-http schemes and invalid ports._  |
|  uint32\_t | [**parse\_uint**](#function-parse_uint) (const char \* s) <br>_Parse a decimal unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**prefix\_icase**](#function-prefix_icase) (const char \* str, const char \* prefix) <br>_Case-insensitive prefix match._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**send\_content\_headers**](#function-send_content_headers) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const char \* content\_type, size\_t content\_length) <br>_Send optional Content-Type and Content-Length request headers._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**send\_custom\_headers**](#function-send_custom_headers) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const [**SYN\_HttpHeader**](structSYN__HttpHeader.md) \* headers, uint8\_t header\_count) <br>_Send optional user-supplied custom HTTP headers._  |
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

PT\_IDLE while waiting, PT\_ENDED when complete, or PT\_EXITED on abort. 





        

<hr>
## Public Static Functions Documentation




### function http\_apply\_redirect 

_Apply redirect hop rules, stripping cross-origin headers and converting POST to GET._ 
```C++
static bool http_apply_redirect (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

true if redirect hop is valid, false on hop limit or unsupported scheme. 





        

<hr>



### function http\_body\_wake\_ready 

_Check wakeup predicate for body receive wait._ 
```C++
static bool http_body_wake_ready (
    const SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

true if socket closed, readable, or body timeout elapsed. 





        

<hr>



### function http\_close\_sock 

_Close and invalidate client socket if open._ 
```C++
static void http_close_sock (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 




        

<hr>



### function http\_connect\_and\_send 

_Open connection, transmit request headers and optional body, and init header state._ 
```C++
static bool http_connect_and_send (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

true on success, false on connect/send failure. 





        

<hr>



### function http\_consume\_chunk\_crlf 

_Validate exact post-chunk CRLF delimiter bytes in chunk\_state 1/3._ 
```C++
static bool http_consume_chunk_crlf (
    SYN_HttpClient * c,
    uint8_t ch
) 
```





**Parameters:**


* `c` HTTP client instance. 
* `ch` Consumed delimiter byte. 



**Returns:**

true if delimiter byte matches expected CR/LF sequence, false otherwise. 





        

<hr>



### function http\_consume\_chunked\_buf 

_Consume buffered bytes in c-&gt;work\_buf for chunked transfer decoding._ 
```C++
static int http_consume_chunked_buf (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

1 if final trailer CRLF completed, 0 if buffer drained, -1 on error. 





        

<hr>



### function http\_drain\_initial\_body 

_Deliver any initial body bytes buffered alongside HTTP response headers._ 
```C++
static bool http_drain_initial_body (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

true on success, false if body\_cb aborted. 





        

<hr>



### function http\_fail 

_Close socket, transition client to error state, and return -1._ 
```C++
static int http_fail (
    SYN_HttpClient * c,
    SYN_Status status
) 
```





**Parameters:**


* `c` HTTP client instance. 
* `status` Error status code to record. 



**Returns:**

Always -1. 





        

<hr>



### function http\_feed\_chunk\_size\_byte 

_Feed one byte in chunk\_state == 0 (chunk size header line)._ 
```C++
static bool http_feed_chunk_size_byte (
    SYN_HttpClient * c,
    uint8_t ch
) 
```





**Parameters:**


* `c` HTTP client instance. 
* `ch` Byte consumed from work\_buf. 



**Returns:**

true to continue, false on invalid chunk hex. 





        

<hr>



### function http\_feed\_trailer\_byte 

_Consume one trailer-section byte in chunk\_state 2 (line) or 4 (post-CR)._ 
```C++
static int http_feed_trailer_byte (
    SYN_HttpClient * c,
    uint8_t ch
) 
```





**Parameters:**


* `c` HTTP client instance. 
* `ch` Consumed byte. 



**Returns:**

1 when terminating blank CRLF completes, 0 to continue, -1 on framing error. 





        

<hr>



### function http\_handle\_body\_recv\_neg 

_Handle negative socket recv return during HTTP body streaming._ 
```C++
static int http_handle_body_recv_neg (
    SYN_HttpClient * c,
    bool was_readable
) 
```





**Parameters:**


* `c` HTTP client instance. 
* `was_readable` Whether socket reported readability prior to recv. 



**Returns:**

0 to wait for socket/timeout, or -1 on error/timeout. 





        

<hr>



### function http\_header\_wake\_ready 

_Check wakeup predicate for header receive wait._ 
```C++
static bool http_header_wake_ready (
    const SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

true if socket closed, readable, or header timeout elapsed. 





        

<hr>



### function http\_hex\_nibble 

_Convert ASCII hexadecimal digit to nibble value [0..15] or -1 if invalid._ 
```C++
static int http_hex_nibble (
    char hc
) 
```





**Parameters:**


* `hc` ASCII character. 



**Returns:**

Nibble value 0..15, or -1 if non-hexadecimal. 





        

<hr>



### function http\_parse\_chunk\_hex 

_Parse hexadecimal chunk length from c-&gt;chunk\_line into c-&gt;chunk\_remaining._ 
```C++
static bool http_parse_chunk_hex (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

true on &gt;= 1 valid hex digit without uint32\_t overflow, false otherwise. 





        

<hr>



### function http\_parse\_header\_line 

_Parse a single HTTP response header line into_ [_**SYN\_HttpResponse**_](structSYN__HttpResponse.md) _._
```C++
static void http_parse_header_line (
    SYN_HttpResponse * resp,
    const char * cur
) 
```





**Parameters:**


* `resp` Target response metadata. 
* `cur` Start of header line. 




        

<hr>



### function http\_parse\_port 

_Parse a bounded TCP port [1..65535] stopping at '/' or NUL._ 
```C++
static bool http_parse_port (
    const char * p,
    uint16_t * port_out
) 
```





**Parameters:**


* `p` Pointer to first character after ':' in URL authority. 
* `port_out` [out] Parsed port in [1..65535]. 



**Returns:**

true if valid port in [1..65535], false on empty, non-digit, 0, or overflow. 





        

<hr>



### function http\_prepare\_response 

_Parse buffered response headers and handle redirects or body setup._ 
```C++
static int http_prepare_response (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

1 to stream body, 2 to follow redirect, -1 to exit task (done or error). 





        

<hr>



### function http\_response\_is\_bodyless 

_Return true if response never carries a body (HEAD, 1xx, 204, 304)._ 
```C++
static bool http_response_is_bodyless (
    const char * method,
    int status_code
) 
```





**Parameters:**


* `method` HTTP request method string. 
* `status_code` HTTP response status code. 



**Returns:**

true if response has no message body per RFC 9112 Section 6.3. 





        

<hr>



### function http\_skip\_spaces 

_Advance past leading optional whitespace (SP or HTAB) in a header value._ 
```C++
static const char * http_skip_spaces (
    const char * s
) 
```





**Parameters:**


* `s` Pointer after header colon. 



**Returns:**

Pointer to first non-OWS character. 





        

<hr>



### function http\_sock\_readable 

_Check whether client socket is valid and has pending readability._ 
```C++
static bool http_sock_readable (
    const SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

true if socket is valid and readable. 





        

<hr>



### function http\_status\_is\_bodyless 

_Return true if response status code never carries a message body (RFC 9112 Section 6.3)._ 
```C++
static bool http_status_is_bodyless (
    int status_code
) 
```





**Parameters:**


* `status_code` HTTP status code. 



**Returns:**

true for 1xx, 204, and 304 responses. 





        

<hr>



### function http\_step\_body 

_Dispatch one body streaming step according to response framing mode._ 
```C++
static int http_step_body (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

Step status code (1=done, 2/3=progress, 0=wait, -1=error). 





        

<hr>



### function http\_step\_chunked\_body 

_Advance chunked transfer decoding by consuming buffered bytes or reading socket._ 
```C++
static int http_step_chunked_body (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

1 when complete, 2 if socket still readable, 3 to parse read bytes, 0 to wait, -1 error. 





        

<hr>



### function http\_step\_headers 

_Perform one non-blocking header accumulation step._ 
```C++
static int http_step_headers (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

1 if headers complete, 2 to share/continue, 0 to wait, -1 on error/timeout. 





        

<hr>



### function http\_step\_stream\_body 

_Advance Content-Length or EOF-delimited body streaming by one step._ 
```C++
static int http_step_stream_body (
    SYN_HttpClient * c
) 
```





**Parameters:**


* `c` HTTP client instance. 



**Returns:**

1 when complete, 2 if socket still readable, 3 to continue, 0 to wait, -1 on error. 





        

<hr>



### function http\_task\_args\_valid 

_Validate task arguments and working buffer constraints._ 
```C++
static bool http_task_args_valid (
    const SYN_PT * pt,
    const SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread context. 
* `task` Scheduler task context. 



**Returns:**

true if arguments are valid, false otherwise. 





        

<hr>



### function parse\_redirect\_url 

_Parse a redirect Location header URL, rejecting non-http schemes and invalid ports._ 
```C++
static bool parse_redirect_url (
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



**Returns:**

true if URL uses [http://](http://) or relative path with valid port, false otherwise. 





        

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



### function send\_content\_headers 

_Send optional Content-Type and Content-Length request headers._ 
```C++
static bool send_content_headers (
    SYN_Socket sock,
    const char * content_type,
    size_t content_length
) 
```





**Parameters:**


* `sock` Connected socket. 
* `content_type` Optional Content-Type string. 
* `content_length` Request payload length in bytes. 



**Returns:**

true if all bytes sent, false on write error. 





        

<hr>



### function send\_custom\_headers 

_Send optional user-supplied custom HTTP headers._ 
```C++
static bool send_custom_headers (
    SYN_Socket sock,
    const SYN_HttpHeader * headers,
    uint8_t header_count
) 
```





**Parameters:**


* `sock` Connected socket. 
* `headers` Array of custom headers. 
* `header_count` Number of entries in headers array. 



**Returns:**

true if all bytes sent, false on write error. 





        

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

