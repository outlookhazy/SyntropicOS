

# File syn\_httpd.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_httpd.h**](syn__httpd_8h.md)

[Go to the source code of this file](syn__httpd_8h_source.md)

_Minimal HTTP/1.1 server — route-based, zero-alloc._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_task.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Httpd**](structSYN__Httpd.md) <br>_HTTP server context structure._  |
| struct | [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) <br>_Parsed HTTP request container presented to route handlers._  |
| struct | [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) <br>_HTTP response formatting state._  |
| struct | [**SYN\_HttpdRoute**](structSYN__HttpdRoute.md) <br>_An HTTP routing entry configuration._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_HttpMethod**](#enum-syn_httpmethod)  <br>_Supported HTTP request methods._  |
| typedef void(\* | [**SYN\_HttpdHandler**](#typedef-syn_httpdhandler)  <br>_Route handler function._  |
| enum  | [**SYN\_HttpdState**](#enum-syn_httpdstate)  <br>_Processing phase for the non-blocking request state machine._  |




















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




























## Detailed Description


Designed to run as a cooperative protothread task within the SyntropicOS scheduler. Yields between accepts so other tasks can run. Handles one request per scheduler tick.




**
**


```C++
static void handle_status(const SYN_HttpdRequest *req,
                           SYN_HttpdResponse *resp, void *ctx) {
    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/json");
    syn_httpd_body_str(resp, "{\"status\":\"ok\"}");
}

static const SYN_HttpdRoute routes[] = {
    { SYN_HTTP_GET, "/api/status", handle_status, NULL },
};

// In your task array:
static uint8_t httpd_buf[1024];
static SYN_Httpd httpd;
syn_httpd_init(&httpd, 80, routes, 1, httpd_buf, sizeof(httpd_buf));
syn_task_create(&tasks[N], "httpd", syn_httpd_task, 2, &httpd);
```
 





    
## Public Types Documentation




### enum SYN\_HttpMethod 

_Supported HTTP request methods._ 
```C++
enum SYN_HttpMethod {
    SYN_HTTP_GET = 0,
    SYN_HTTP_POST = 1,
    SYN_HTTP_PUT = 2,
    SYN_HTTP_DELETE = 3
};
```




<hr>



### typedef SYN\_HttpdHandler 

_Route handler function._ 
```C++
typedef void(* SYN_HttpdHandler) (const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx);
```



Called when a request matches the route. The handler sends the response using [**syn\_httpd\_status()**](syn__httpd_8h.md#function-syn_httpd_status), [**syn\_httpd\_header()**](syn__httpd_8h.md#function-syn_httpd_header), [**syn\_httpd\_body()**](syn__httpd_8h.md#function-syn_httpd_body). 


        

<hr>



### enum SYN\_HttpdState 

_Processing phase for the non-blocking request state machine._ 
```C++
enum SYN_HttpdState {
    SYN_HTTPD_IDLE,
    SYN_HTTPD_READING_HEADERS,
    SYN_HTTPD_DISPATCHING
};
```



[**syn\_httpd\_step()**](syn__httpd_8h.md#function-syn_httpd_step) advances through these phases without blocking: IDLE → READING\_HEADERS → DISPATCHING → IDLE 


        

<hr>
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_httpd.h`

