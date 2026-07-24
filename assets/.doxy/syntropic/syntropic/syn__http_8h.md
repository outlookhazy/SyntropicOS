

# File syn\_http.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_http.h**](syn__http_8h.md)

[Go to the source code of this file](syn__http_8h_source.md)

_Cooperative HTTP/1.1 client — streaming, zero-alloc, non-blocking._ [More...](#detailed-description)

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
| struct | [**SYN\_HttpClient**](structSYN__HttpClient.md) <br>_HTTP client context structure._  |
| struct | [**SYN\_HttpHeader**](structSYN__HttpHeader.md) <br>_Key-value pair representing an HTTP header._  |
| struct | [**SYN\_HttpResponse**](structSYN__HttpResponse.md) <br>_HTTP response metadata._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_HttpBodyCallback**](#typedef-syn_httpbodycallback)  <br>_Callback invoked to stream chunks of the HTTP response body._  |
| enum  | [**SYN\_HttpState**](#enum-syn_httpstate)  <br>_States for the HTTP client cooperative state machine._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_http\_client\_init**](#function-syn_http_client_init) ([**SYN\_HttpClient**](structSYN__HttpClient.md) \* client, const char \* method, const char \* host, uint16\_t port, const char \* path, const char \* content\_type, const uint8\_t \* body, size\_t body\_len, const [**SYN\_HttpHeader**](structSYN__HttpHeader.md) \* headers, uint8\_t header\_count, [**SYN\_HttpBodyCallback**](syn__http_8h.md#typedef-syn_httpbodycallback) body\_cb, void \* cb\_ctx, uint8\_t \* work\_buf, size\_t work\_buf\_size) <br>_Initialize the HTTP client struct._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_http\_client\_task**](#function-syn_http_client_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative task to drive the HTTP client._  |




























## Detailed Description


Performs HTTP GET and POST requests over a TCP socket as a cooperative protothread task. Response bodies are delivered via streaming callback. 


    
## Public Types Documentation




### typedef SYN\_HttpBodyCallback 

_Callback invoked to stream chunks of the HTTP response body._ 
```C++
typedef bool(* SYN_HttpBodyCallback) (const uint8_t *data, size_t len, void *ctx);
```





**Parameters:**


* `data` Pointer to the received chunk of body data. 
* `len` Length of the data chunk in bytes. 
* `ctx` User-defined context pointer passed to [**syn\_http\_client\_init()**](syn__http_8h.md#function-syn_http_client_init). 



**Returns:**

true to continue reading body, false to abort. 





        

<hr>



### enum SYN\_HttpState 

_States for the HTTP client cooperative state machine._ 
```C++
enum SYN_HttpState {
    SYN_HTTP_STATE_IDLE,
    SYN_HTTP_STATE_CONNECTING,
    SYN_HTTP_STATE_SENDING_REQUEST,
    SYN_HTTP_STATE_READING_HEADERS,
    SYN_HTTP_STATE_READING_BODY,
    SYN_HTTP_STATE_DONE,
    SYN_HTTP_STATE_ERROR
};
```




<hr>
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_http.h`

