

# File syn\_ymodem.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ymodem.h**](syn__ymodem_8h.md)

[Go to the source code of this file](syn__ymodem_8h_source.md)

_YMODEM / XMODEM-1K Serial File Transfer Receiver Protocol._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) <br>_YMODEM Receiver Context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_YMODEM\_Event**](#enum-syn_ymodem_event)  <br>_YMODEM Event Types passed to callback._  |
| typedef int(\* | [**SYN\_YMODEM\_EventCallback**](#typedef-syn_ymodem_eventcallback)  <br>_Application callback for YMODEM transfer events._  |
| typedef int(\* | [**SYN\_YMODEM\_GetChar**](#typedef-syn_ymodem_getchar)  <br>_Read a byte from the serial link (UART RX)._  |
| typedef void(\* | [**SYN\_YMODEM\_PutChar**](#typedef-syn_ymodem_putchar)  <br>_Write a byte to the serial link (UART TX)._  |
| enum  | [**SYN\_YMODEM\_Status**](#enum-syn_ymodem_status)  <br>_YMODEM Session Status Codes._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_YMODEM\_Status**](syn__ymodem_8h.md#enum-syn_ymodem_status) | [**syn\_ymodem\_receive**](#function-syn_ymodem_receive) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx) <br>_Execute a YMODEM receive session._  |
|  void | [**syn\_ymodem\_receiver\_init**](#function-syn_ymodem_receiver_init) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx, [**SYN\_YMODEM\_PutChar**](syn__ymodem_8h.md#typedef-syn_ymodem_putchar) putchar\_fn, [**SYN\_YMODEM\_GetChar**](syn__ymodem_8h.md#typedef-syn_ymodem_getchar) getchar\_fn, [**SYN\_YMODEM\_EventCallback**](syn__ymodem_8h.md#typedef-syn_ymodem_eventcallback) event\_fn, void \* ctx) <br>_Initialize a YMODEM receiver instance._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_YMODEM\_ACK**](syn__ymodem_8h.md#define-syn_ymodem_ack)  `0x06U`<br> |
| define  | [**SYN\_YMODEM\_CAN**](syn__ymodem_8h.md#define-syn_ymodem_can)  `0x18U`<br> |
| define  | [**SYN\_YMODEM\_CRC**](syn__ymodem_8h.md#define-syn_ymodem_crc)  `0x43U`<br> |
| define  | [**SYN\_YMODEM\_EOT**](syn__ymodem_8h.md#define-syn_ymodem_eot)  `0x04U`<br> |
| define  | [**SYN\_YMODEM\_MAX\_BLOCK\_SIZE**](syn__ymodem_8h.md#define-syn_ymodem_max_block_size)  `1024U`<br> |
| define  | [**SYN\_YMODEM\_MAX\_FILENAME**](syn__ymodem_8h.md#define-syn_ymodem_max_filename)  `64U`<br> |
| define  | [**SYN\_YMODEM\_MAX\_RETRIES**](syn__ymodem_8h.md#define-syn_ymodem_max_retries)  `10U`<br> |
| define  | [**SYN\_YMODEM\_NAK**](syn__ymodem_8h.md#define-syn_ymodem_nak)  `0x15U`<br> |
| define  | [**SYN\_YMODEM\_SOH**](syn__ymodem_8h.md#define-syn_ymodem_soh)  `0x01U`<br> |
| define  | [**SYN\_YMODEM\_STX**](syn__ymodem_8h.md#define-syn_ymodem_stx)  `0x02U`<br> |

## Detailed Description


Implements a lightweight, zero-allocation YMODEM (and XMODEM-1K compatible) batch file transfer receiver state machine for microcontroller bootloaders. Supports Block 0 metadata (filename, file size parsing), 1024-byte (STX) and 128-byte (SOH) blocks, CRC-16/XMODEM error checking, and cancellation.




**
**


```C++
SYN_YMODEM_Receiver rx;
syn_ymodem_receiver_init(&rx, my_putchar, my_getchar, my_event_cb, user_ctx);
SYN_YMODEM_Status status = syn_ymodem_receive(&rx);
```
 





    
## Public Types Documentation




### enum SYN\_YMODEM\_Event 

_YMODEM Event Types passed to callback._ 
```C++
enum SYN_YMODEM_Event {
    SYN_YMODEM_EVENT_FILE_START = 0,
    SYN_YMODEM_EVENT_DATA = 1,
    SYN_YMODEM_EVENT_FILE_END = 2,
    SYN_YMODEM_EVENT_SESSION_END = 3
};
```




<hr>



### typedef SYN\_YMODEM\_EventCallback 

_Application callback for YMODEM transfer events._ 
```C++
typedef int(* SYN_YMODEM_EventCallback) (SYN_YMODEM_Event event, const uint8_t *data, size_t len, void *ctx);
```





**Parameters:**


* `event` Event type. 
* `data` Payload buffer (filename for FILE\_START, block payload for DATA). 
* `len` Payload size in bytes. 
* `ctx` User context. 



**Returns:**

0 on success, non-zero to cancel transfer. 





        

<hr>



### typedef SYN\_YMODEM\_GetChar 

_Read a byte from the serial link (UART RX)._ 
```C++
typedef int(* SYN_YMODEM_GetChar) (uint32_t timeout_ms, void *ctx);
```





**Parameters:**


* `timeout_ms` Read timeout in milliseconds. 
* `ctx` User context. 



**Returns:**

Byte value (0-255) on success, or negative value (&lt;0) on timeout/error. 





        

<hr>



### typedef SYN\_YMODEM\_PutChar 

_Write a byte to the serial link (UART TX)._ 
```C++
typedef void(* SYN_YMODEM_PutChar) (uint8_t byte, void *ctx);
```





**Parameters:**


* `byte` Data byte to send. 
* `ctx` User context. 




        

<hr>



### enum SYN\_YMODEM\_Status 

_YMODEM Session Status Codes._ 
```C++
enum SYN_YMODEM_Status {
    SYN_YMODEM_OK = 0,
    SYN_YMODEM_ERR_TIMEOUT = -1,
    SYN_YMODEM_ERR_CRC = -2,
    SYN_YMODEM_ERR_CANCEL = -3,
    SYN_YMODEM_ERR_SEQUENCE = -4,
    SYN_YMODEM_ERR_OVERFLOW = -5,
    SYN_YMODEM_ERR_PARAM = -6
};
```




<hr>
## Public Functions Documentation




### function syn\_ymodem\_receive 

_Execute a YMODEM receive session._ 
```C++
SYN_YMODEM_Status syn_ymodem_receive (
    SYN_YMODEM_Receiver * rx
) 
```



Runs the YMODEM handshake, processes Block 0 header, accepts data packets, verifies CRCs, and invokes event callbacks until all files in the batch are transferred or an unrecoverable error occurs.




**Parameters:**


* `rx` Receiver context. 



**Returns:**

SYN\_YMODEM\_OK on successful session completion, or error code (&lt;0). 





        

<hr>



### function syn\_ymodem\_receiver\_init 

_Initialize a YMODEM receiver instance._ 
```C++
void syn_ymodem_receiver_init (
    SYN_YMODEM_Receiver * rx,
    SYN_YMODEM_PutChar putchar_fn,
    SYN_YMODEM_GetChar getchar_fn,
    SYN_YMODEM_EventCallback event_fn,
    void * ctx
) 
```





**Parameters:**


* `rx` Receiver context. 
* `putchar_fn` Serial TX function. 
* `getchar_fn` Serial RX function with timeout. 
* `event_fn` Event callback handler. 
* `ctx` User context pointer. 




        

<hr>
## Macro Definition Documentation





### define SYN\_YMODEM\_ACK 

```C++
#define SYN_YMODEM_ACK `0x06U`
```



Acknowledge 


        

<hr>



### define SYN\_YMODEM\_CAN 

```C++
#define SYN_YMODEM_CAN `0x18U`
```



Cancel transmission 


        

<hr>



### define SYN\_YMODEM\_CRC 

```C++
#define SYN_YMODEM_CRC `0x43U`
```



ASCII 'C' to request CRC-16 mode 


        

<hr>



### define SYN\_YMODEM\_EOT 

```C++
#define SYN_YMODEM_EOT `0x04U`
```



End of transmission 


        

<hr>



### define SYN\_YMODEM\_MAX\_BLOCK\_SIZE 

```C++
#define SYN_YMODEM_MAX_BLOCK_SIZE `1024U`
```



Maximum packet block size (1024 B STX) 


        

<hr>



### define SYN\_YMODEM\_MAX\_FILENAME 

```C++
#define SYN_YMODEM_MAX_FILENAME `64U`
```



Maximum filename string buffer size 


        

<hr>



### define SYN\_YMODEM\_MAX\_RETRIES 

```C++
#define SYN_YMODEM_MAX_RETRIES `10U`
```



Maximum packet NAK retry attempts 


        

<hr>



### define SYN\_YMODEM\_NAK 

```C++
#define SYN_YMODEM_NAK `0x15U`
```



Negative acknowledge 


        

<hr>



### define SYN\_YMODEM\_SOH 

```C++
#define SYN_YMODEM_SOH `0x01U`
```



Start of 128-byte data block 


        

<hr>



### define SYN\_YMODEM\_STX 

```C++
#define SYN_YMODEM_STX `0x02U`
```



Start of 1024-byte data block 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ymodem.h`

