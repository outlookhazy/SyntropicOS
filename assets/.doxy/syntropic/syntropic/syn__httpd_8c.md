

# File syn\_httpd.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_httpd.c**](syn__httpd_8c.md)

[Go to the source code of this file](syn__httpd_8c_source.md)

_Minimal HTTP/1.1 server — fully non-blocking implementation._ [More...](#detailed-description)

* `#include "syn_httpd.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`
* `#include "../util/syn_fmt.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_httpd\_body**](#function-syn_httpd_body) ([**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, const void \* data, size\_t len) <br>_Send response body data._  |
|  void | [**syn\_httpd\_body\_str**](#function-syn_httpd_body_str) ([**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, const char \* str) <br>_Send a string as the response body._  |
|  void | [**syn\_httpd\_header**](#function-syn_httpd_header) (const [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, const char \* name, const char \* value) <br>_Add a response header._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_httpd\_init**](#function-syn_httpd_init) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv, uint16\_t port, const [**SYN\_HttpdRoute**](structSYN__HttpdRoute.md) \* routes, size\_t route\_count, uint8\_t \* work\_buf, size\_t work\_buf\_size) <br>_Initialize and start the HTTP server._  |
|  int | [**syn\_httpd\_read\_body**](#function-syn_httpd_read_body) (const [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req, const [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, void \* buf, size\_t max\_len) <br>_Read request body data (for POST/PUT)._  |
|  void | [**syn\_httpd\_status**](#function-syn_httpd_status) (const [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, int code, const char \* reason) <br>_Begin the response with a status line._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_httpd\_step**](#function-syn_httpd_step) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv) <br>_Handle one incoming request (non-protothread version)._  |
|  void | [**syn\_httpd\_stop**](#function-syn_httpd_stop) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv) <br>_Stop the server and close the listener socket._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_httpd\_task**](#function-syn_httpd_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Protothread task function for the HTTP server._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**dispatch\_request**](#function-dispatch_request) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv) <br>_Handle the DISPATCHING phase — parse, route, respond, close._  |
|  void | [**drop\_client**](#function-drop_client) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv) <br>_Drop the active client and reset to IDLE._  |
|  void | [**finalize\_headers**](#function-finalize_headers) ([**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp) <br>_Flush the header block — sends the blank line after headers._  |
|  const [**SYN\_HttpdRoute**](structSYN__HttpdRoute.md) \* | [**match\_route**](#function-match_route) (const [**SYN\_Httpd**](structSYN__Httpd.md) \* srv, const [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req) <br>_Match a route against the request._  |
|  int | [**parse\_headers\_from\_buf**](#function-parse_headers_from_buf) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req, uint8\_t \* buf, size\_t total) <br>_Parse request line + headers from already-buffered data._  |
|  [**SYN\_HttpMethod**](syn__httpd_8h.md#enum-syn_httpmethod) | [**parse\_method**](#function-parse_method) (const char \* str, size\_t len) <br>_Parse the method string into enum._  |
|  uint32\_t | [**parse\_uint**](#function-parse_uint) (const char \* s) <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**prefix\_icase**](#function-prefix_icase) (const char \* str, const char \* prefix) <br> |
|  void | [**send\_error**](#function-send_error) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, int code, const char \* reason) <br>_Send a simple error response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**sock\_write**](#function-sock_write) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, const char \* str) <br>_Write a null-terminated string to the socket._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**HTTPD\_CLIENT\_TIMEOUT\_MS**](syn__httpd_8c.md#define-httpd_client_timeout_ms)  `5000`<br> |

## Detailed Description


Designed around the cooperative scheduler: every call to [**syn\_httpd\_step()**](syn__httpd_8c.md#function-syn_httpd_step) does a bounded amount of work and returns immediately. Socket I/O uses timeout=0 exclusively. Stale connections are timed out via the tick clock. 


    
## Public Functions Documentation




### function syn\_httpd\_body 

_Send response body data._ 
```C++
void syn_httpd_body (
    SYN_HttpdResponse * resp,
    const void * data,
    size_t len
) 
```



Automatically finalizes headers on first call. Can be called multiple times for streaming responses.




**Parameters:**


* `resp` HTTP response state to write to. 
* `data` Pointer to binary data to transmit. 
* `len` Length of data in bytes. 




        

<hr>



### function syn\_httpd\_body\_str 

_Send a string as the response body._ 
```C++
void syn_httpd_body_str (
    SYN_HttpdResponse * resp,
    const char * str
) 
```



Convenience wrapper for [**syn\_httpd\_body()**](syn__httpd_8h.md#function-syn_httpd_body).




**Parameters:**


* `resp` HTTP response state to write to. 
* `str` Null-terminated string to send. 




        

<hr>



### function syn\_httpd\_header 

_Add a response header._ 
```C++
void syn_httpd_header (
    const SYN_HttpdResponse * resp,
    const char * name,
    const char * value
) 
```



Must be called after [**syn\_httpd\_status()**](syn__httpd_8h.md#function-syn_httpd_status) and before [**syn\_httpd\_body()**](syn__httpd_8h.md#function-syn_httpd_body).




**Parameters:**


* `resp` HTTP response state to write to. 
* `name` Header key string (e.g. "Content-Type"). 
* `value` Header value string (e.g. "text/plain"). 




        

<hr>



### function syn\_httpd\_init 

_Initialize and start the HTTP server._ 
```C++
SYN_Status syn_httpd_init (
    SYN_Httpd * srv,
    uint16_t port,
    const SYN_HttpdRoute * routes,
    size_t route_count,
    uint8_t * work_buf,
    size_t work_buf_size
) 
```



Creates a listening socket on the given port.




**Parameters:**


* `srv` Server instance to initialize. 
* `port` Port to listen on (typically 80). 
* `routes` Route table (must outlive the server). 
* `route_count` Number of routes. 
* `work_buf` Shared work buffer for request parsing + responses. 
* `work_buf_size` Buffer size (512+ recommended). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if bind/listen fails. 





        

<hr>



### function syn\_httpd\_read\_body 

_Read request body data (for POST/PUT)._ 
```C++
int syn_httpd_read_body (
    const SYN_HttpdRequest * req,
    const SYN_HttpdResponse * resp,
    void * buf,
    size_t max_len
) 
```





**Parameters:**


* `req` Request info. 
* `resp` Response writer context. 
* `buf` Buffer to read into. 
* `max_len` Buffer capacity in bytes. 



**Returns:**

Bytes read, 0 at end of body, -1 on error. 





        

<hr>



### function syn\_httpd\_status 

_Begin the response with a status line._ 
```C++
void syn_httpd_status (
    const SYN_HttpdResponse * resp,
    int code,
    const char * reason
) 
```



Must be called first. E.g., syn\_httpd\_status(resp, 200, "OK").




**Parameters:**


* `resp` HTTP response state to write to. 
* `code` HTTP numeric status code (e.g. 200). 
* `reason` Human readable status message (e.g. "OK"). 




        

<hr>



### function syn\_httpd\_step 

_Handle one incoming request (non-protothread version)._ 
```C++
SYN_Status syn_httpd_step (
    SYN_Httpd * srv
) 
```



Accepts one connection, parses the request, dispatches to the matching route handler, and closes the connection. Use this if you prefer manual control over the server loop.




**Parameters:**


* `srv` Server instance. 



**Returns:**

SYN\_OK if a request was handled, SYN\_TIMEOUT if no client connected, SYN\_ERROR on parse failure. 





        

<hr>



### function syn\_httpd\_stop 

_Stop the server and close the listener socket._ 
```C++
void syn_httpd_stop (
    SYN_Httpd * srv
) 
```





**Parameters:**


* `srv` Server instance to stop. 




        

<hr>



### function syn\_httpd\_task 

_Protothread task function for the HTTP server._ 
```C++
SYN_PT_Status syn_httpd_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Register this as a SYN\_TaskFunc with user\_data pointing to a [**SYN\_Httpd**](structSYN__Httpd.md) instance. The task accepts a connection, handles one request, then yields back to the scheduler.




**Parameters:**


* `pt` Pointer to the cooperative protothread context. 
* `task` Pointer to the scheduler task structure. 



**Returns:**

PT\_WAITING or PT\_EXITED status. 





        

<hr>
## Public Static Functions Documentation




### function dispatch\_request 

_Handle the DISPATCHING phase — parse, route, respond, close._ 
```C++
static void dispatch_request (
    SYN_Httpd * srv
) 
```



Called once when headers are fully buffered. Runs synchronously but all data is already in work\_buf — no blocking socket reads needed for headers.




**Parameters:**


* `srv` Server instance with complete headers in work\_buf. 




        

<hr>



### function drop\_client 

_Drop the active client and reset to IDLE._ 
```C++
static void drop_client (
    SYN_Httpd * srv
) 
```





**Parameters:**


* `srv` Server instance. 




        

<hr>



### function finalize\_headers 

_Flush the header block — sends the blank line after headers._ 
```C++
static void finalize_headers (
    SYN_HttpdResponse * resp
) 
```





**Parameters:**


* `resp` Response context. 




        

<hr>



### function match\_route 

_Match a route against the request._ 
```C++
static const SYN_HttpdRoute * match_route (
    const SYN_Httpd * srv,
    const SYN_HttpdRequest * req
) 
```





**Parameters:**


* `srv` Server instance. 
* `req` Parsed request. 



**Returns:**

Matching route, or NULL. 





        

<hr>



### function parse\_headers\_from\_buf 

_Parse request line + headers from already-buffered data._ 
```C++
static int parse_headers_from_buf (
    SYN_Socket sock,
    SYN_HttpdRequest * req,
    uint8_t * buf,
    size_t total
) 
```



This is a pure parsing function — no socket I/O. The caller guarantees that buf contains a complete header block ending with "\r\n\r\n".




**Parameters:**


* `sock` Client socket (stored in req for body reads). 
* `req` [out] Parsed request. 
* `buf` Work buffer containing the raw request. 
* `total` Number of valid bytes in buf. 



**Returns:**

0 on success, -1 on parse error. 





        

<hr>



### function parse\_method 

_Parse the method string into enum._ 
```C++
static SYN_HttpMethod parse_method (
    const char * str,
    size_t len
) 
```





**Parameters:**


* `str` Method string (e.g. "GET"). 
* `len` Length of the method string. 



**Returns:**

Corresponding SYN\_HttpMethod. 





        

<hr>



### function parse\_uint 

```C++
static inline uint32_t parse_uint (
    const char * s
) 
```




<hr>



### function prefix\_icase 

```C++
static inline bool prefix_icase (
    const char * str,
    const char * prefix
) 
```




<hr>



### function send\_error 

_Send a simple error response._ 
```C++
static void send_error (
    SYN_Socket sock,
    int code,
    const char * reason
) 
```





**Parameters:**


* `sock` Client socket. 
* `code` HTTP status code. 
* `reason` Reason phrase (e.g. "Not Found"). 




        

<hr>



### function sock\_write 

_Write a null-terminated string to the socket._ 
```C++
static bool sock_write (
    SYN_Socket sock,
    const char * str
) 
```





**Parameters:**


* `sock` Socket to write to. 
* `str` String to send. 



**Returns:**

true if all bytes were sent. 





        

<hr>
## Macro Definition Documentation





### define HTTPD\_CLIENT\_TIMEOUT\_MS 

```C++
#define HTTPD_CLIENT_TIMEOUT_MS `5000`
```



Maximum time (ms) to wait for a client to send complete headers after connecting. Enforced non-blockingly via the tick clock — not passed to any socket recv call. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_httpd.c`

