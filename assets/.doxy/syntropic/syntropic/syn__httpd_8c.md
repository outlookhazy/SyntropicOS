

# File syn\_httpd.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_httpd.c**](syn__httpd_8c.md)

[Go to the source code of this file](syn__httpd_8c_source.md)

_Minimal HTTP/1.1 server — fully non-blocking implementation._ [More...](#detailed-description)

* `#include "../port/syn_port_system.h"`
* `#include "../sched/syn_sched.h"`
* `#include "../util/syn_assert.h"`
* `#include "syn_httpd.h"`
* `#include <string.h>`
* `#include "../util/syn_fmt.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_httpd\_body**](#function-syn_httpd_body) ([**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, const void \* data, size\_t len) <br>_Send response body data._  |
|  void | [**syn\_httpd\_body\_str**](#function-syn_httpd_body_str) ([**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, const char \* str) <br>_Send a string as the response body._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_httpd\_get\_query\_param**](#function-syn_httpd_get_query_param) (const char \* query, const char \* key, char \* val\_buf, size\_t max\_len) <br>_Find and extract a query parameter value by key._  |
|  void | [**syn\_httpd\_header**](#function-syn_httpd_header) (const [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, const char \* name, const char \* value) <br>_Add a response header._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_httpd\_init**](#function-syn_httpd_init) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv, uint16\_t port, const [**SYN\_HttpdRoute**](structSYN__HttpdRoute.md) \* routes, size\_t route\_count, uint8\_t \* work\_buf, size\_t work\_buf\_size) <br>_Initialize and start the HTTP server._  |
|  int | [**syn\_httpd\_read\_body**](#function-syn_httpd_read_body) (const [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req, [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, void \* buf, size\_t max\_len) <br>_Read request body data (for POST/PUT)._  |
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
|  [**bool**](syn__defs_8h.md#enum-bool) | [**httpd\_buffer\_full\_in\_reading**](#function-httpd_buffer_full_in_reading) (const [**SYN\_Httpd**](structSYN__Httpd.md) \* srv) <br>_Check if server is still accumulating headers/body but work\_buf has no remaining space._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**httpd\_check\_headers\_complete**](#function-httpd_check_headers_complete) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv) <br>_Inspect buffered headers without mutating work\_buf to compute expected\_total._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**httpd\_has\_work**](#function-httpd_has_work) (const [**SYN\_Httpd**](structSYN__Httpd.md) \* srv) <br>_Check if HTTP server task has actionable work._  |
|  void | [**httpd\_parse\_header\_line**](#function-httpd_parse_header_line) ([**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req, const char \* hdr\_start, const char \* next\_line) <br>_Parse a single HTTP header line into request metadata without mutating headers._  |
|  const char \* | [**httpd\_skip\_ows**](#function-httpd_skip_ows) (const char \* s) <br>_Advance past leading optional whitespace (SP or HTAB) in a header value._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**httpd\_step\_phase**](#function-httpd_step_phase) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv, [**bool**](syn__defs_8h.md#enum-bool) defer\_dispatch) <br>_Advance server state machine by one phase, optionally deferring route dispatch._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**httpd\_step\_progressed**](#function-httpd_step_progressed) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv) <br>_Run one cooperative HTTP server phase and report whether state or RX bytes progressed._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**httpd\_step\_reading**](#function-httpd_step_reading) ([**SYN\_Httpd**](structSYN__Httpd.md) \* srv, [**bool**](syn__defs_8h.md#enum-bool) defer\_dispatch) <br>_Non-blocking header and Content-Length body accumulation phase for an accepted client._  |
|  const [**SYN\_HttpdRoute**](structSYN__HttpdRoute.md) \* | [**match\_route**](#function-match_route) (const [**SYN\_Httpd**](structSYN__Httpd.md) \* srv, const [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req) <br>_Match a route against the request._  |
|  int | [**parse\_headers\_from\_buf**](#function-parse_headers_from_buf) ([**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) sock, [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req, uint8\_t \* buf, size\_t total) <br>_Parse request line + headers from already-buffered data._  |
|  [**SYN\_HttpMethod**](syn__httpd_8h.md#enum-syn_httpmethod) | [**parse\_method**](#function-parse_method) (const char \* str, size\_t len) <br>_Parse the method string into enum._  |
|  uint32\_t | [**parse\_uint**](#function-parse_uint) (const char \* s) <br>_Parse unsigned integer from string._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**prefix\_icase**](#function-prefix_icase) (const char \* str, const char \* prefix) <br>_Case-insensitive string prefix check._  |
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



### function syn\_httpd\_get\_query\_param 

_Find and extract a query parameter value by key._ 
```C++
bool syn_httpd_get_query_param (
    const char * query,
    const char * key,
    char * val_buf,
    size_t max_len
) 
```



E.g., for query string "channel=1&mode=auto", searching "mode" returns "auto".




**Parameters:**


* `query` Raw query string (from req-&gt;query). 
* `key` Key name to search (e.g. "channel"). 
* `val_buf` [out] Destination buffer for value. 
* `max_len` Capacity of destination buffer. 



**Returns:**

true if key was found and value copied, false if missing or buffer too small. 





        

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
    SYN_HttpdResponse * resp,
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



Register this as a [**SYN\_TaskFunc**](syn__task_8h.md#typedef-syn_taskfunc) with user\_data pointing to a [**SYN\_Httpd**](structSYN__Httpd.md) instance. The task accepts a connection, handles one request, then yields back to the scheduler.




**Parameters:**


* `pt` Pointer to the cooperative protothread context. 
* `task` Pointer to the scheduler task structure. 



**Returns:**

PT\_IDLE when waiting, PT\_MORE after handling a step, or PT\_ENDED. 





        

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



### function httpd\_buffer\_full\_in\_reading 

_Check if server is still accumulating headers/body but work\_buf has no remaining space._ 
```C++
static bool httpd_buffer_full_in_reading (
    const SYN_Httpd * srv
) 
```





**Parameters:**


* `srv` Server instance. 



**Returns:**

true if reading state has filled work\_buf capacity. 





        

<hr>



### function httpd\_check\_headers\_complete 

_Inspect buffered headers without mutating work\_buf to compute expected\_total._ 
```C++
static bool httpd_check_headers_complete (
    SYN_Httpd * srv
) 
```





**Parameters:**


* `srv` Server instance with NUL-terminated work\_buf. 



**Returns:**

true if header terminator found, false otherwise. 





        

<hr>



### function httpd\_has\_work 

_Check if HTTP server task has actionable work._ 
```C++
static bool httpd_has_work (
    const SYN_Httpd * srv
) 
```





**Parameters:**


* `srv` Pointer to HTTP server instance. 



**Returns:**

true if work pending, false otherwise. 





        

<hr>



### function httpd\_parse\_header\_line 

_Parse a single HTTP header line into request metadata without mutating headers._ 
```C++
static void httpd_parse_header_line (
    SYN_HttpdRequest * req,
    const char * hdr_start,
    const char * next_line
) 
```





**Parameters:**


* `req` Target parsed request structure. 
* `hdr_start` Start of header line. 
* `next_line` Pointer to terminating CRLF of header line. 




        

<hr>



### function httpd\_skip\_ows 

_Advance past leading optional whitespace (SP or HTAB) in a header value._ 
```C++
static const char * httpd_skip_ows (
    const char * s
) 
```





**Parameters:**


* `s` Pointer after header colon. 



**Returns:**

Pointer to first non-OWS character. 





        

<hr>



### function httpd\_step\_phase 

_Advance server state machine by one phase, optionally deferring route dispatch._ 
```C++
static SYN_Status httpd_step_phase (
    SYN_Httpd * srv,
    bool defer_dispatch
) 
```





**Parameters:**


* `srv` Server instance. 
* `defer_dispatch` true when called from cooperative syn\_httpd\_task. 



**Returns:**

SYN\_OK on phase completion, SYN\_TIMEOUT if awaiting I/O, SYN\_ERROR on failure. 





        

<hr>



### function httpd\_step\_progressed 

_Run one cooperative HTTP server phase and report whether state or RX bytes progressed._ 
```C++
static bool httpd_step_progressed (
    SYN_Httpd * srv
) 
```





**Parameters:**


* `srv` Server instance. 



**Returns:**

true if progress occurred, false otherwise. 





        

<hr>



### function httpd\_step\_reading 

_Non-blocking header and Content-Length body accumulation phase for an accepted client._ 
```C++
static SYN_Status httpd_step_reading (
    SYN_Httpd * srv,
    bool defer_dispatch
) 
```





**Parameters:**


* `srv` Server instance in SYN\_HTTPD\_READING\_HEADERS or SYN\_HTTPD\_READING\_BODY. 
* `defer_dispatch` When true, stop at SYN\_HTTPD\_DISPATCHING so syn\_httpd\_task can yield first. 



**Returns:**

SYN\_OK on dispatch/ready, SYN\_TIMEOUT if awaiting bytes/expired, SYN\_ERROR on failure. 





        

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

Corresponding [**SYN\_HttpMethod**](syn__httpd_8h.md#enum-syn_httpmethod). 





        

<hr>



### function parse\_uint 

_Parse unsigned integer from string._ 
```C++
static inline uint32_t parse_uint (
    const char * s
) 
```





**Parameters:**


* `s` Input string. 



**Returns:**

Parsed uint32\_t value. 





        

<hr>



### function prefix\_icase 

_Case-insensitive string prefix check._ 
```C++
static inline bool prefix_icase (
    const char * str,
    const char * prefix
) 
```





**Parameters:**


* `str` Input string. 
* `prefix` Prefix to test. 



**Returns:**

True if str starts with prefix (case-insensitive). 





        

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

