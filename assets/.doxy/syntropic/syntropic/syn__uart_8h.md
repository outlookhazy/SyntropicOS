

# File syn\_uart.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_uart.h**](syn__uart_8h.md)

[Go to the source code of this file](syn__uart_8h_source.md)

_UART driver — buffered I/O and formatted output._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_ringbuf.h"`
* `#include "../port/syn_port_uart.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_UART**](structSYN__UART.md) <br>_UART driver handle._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_uart\_deinit**](#function-syn_uart_deinit) ([**SYN\_UART**](structSYN__UART.md) \* uart) <br>_De-initialize a UART instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_uart\_init**](#function-syn_uart_init) ([**SYN\_UART**](structSYN__UART.md) \* uart, [**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint32\_t baudrate) <br>_Initialize a UART instance with buffered I/O._  |
|  size\_t | [**syn\_uart\_read**](#function-syn_uart_read) ([**SYN\_UART**](structSYN__UART.md) \* uart, uint8\_t \* data, size\_t max\_len) <br>_Read bytes from the UART RX ring buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uart\_rx\_isr\_feed**](#function-syn_uart_rx_isr_feed) ([**SYN\_UART**](structSYN__UART.md) \* uart, uint8\_t byte) <br>_Feed a received byte into the UART RX ring buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_uart\_write**](#function-syn_uart_write) (const [**SYN\_UART**](structSYN__UART.md) \* uart, const uint8\_t \* data, size\_t len, uint32\_t timeout\_ms) <br>_Write a buffer of bytes to the UART (blocking)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_uart\_write\_str**](#function-syn_uart_write_str) (const [**SYN\_UART**](structSYN__UART.md) \* uart, const char \* str, uint32\_t timeout\_ms) <br>_Write a string to the UART (blocking)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_UART\_MAX\_INSTANCES**](syn__uart_8h.md#define-syn_uart_max_instances)  `2`<br>_Maximum UART instances supported._  |
| define  | [**SYN\_UART\_RX\_BUF\_SIZE**](syn__uart_8h.md#define-syn_uart_rx_buf_size)  `128`<br>_UART receive buffer size (bytes)._  |
| define  | [**SYN\_UART\_TX\_BUF\_SIZE**](syn__uart_8h.md#define-syn_uart_tx_buf_size)  `128`<br>_UART transmit buffer size (bytes)._  |

## Detailed Description


Provides interrupt-driven buffered UART on top of the port layer. Each UART instance gets a TX and RX ring buffer. The buffer sizes are configurable in syn\_config.h. 


    
## Public Functions Documentation




### function syn\_uart\_deinit 

_De-initialize a UART instance._ 
```C++
SYN_Status syn_uart_deinit (
    SYN_UART * uart
) 
```





**Parameters:**


* `uart` UART handle to deinitialize. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_uart\_init 

_Initialize a UART instance with buffered I/O._ 
```C++
SYN_Status syn_uart_init (
    SYN_UART * uart,
    SYN_UARTInstance instance,
    uint32_t baudrate
) 
```





**Parameters:**


* `uart` Pointer to a caller-owned [**SYN\_UART**](structSYN__UART.md) struct. 
* `instance` UART peripheral number (0, 1, …). 
* `baudrate` Desired baud rate. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_uart\_read 

_Read bytes from the UART RX ring buffer._ 
```C++
size_t syn_uart_read (
    SYN_UART * uart,
    uint8_t * data,
    size_t max_len
) 
```



Reads up to `max_len` bytes that have been received. Non-blocking: returns immediately with however many bytes are available.




**Parameters:**


* `uart` UART handle. 
* `data` Buffer to read into. 
* `max_len` Maximum number of bytes to read. 



**Returns:**

Number of bytes actually read. 





        

<hr>



### function syn\_uart\_rx\_isr\_feed 

_Feed a received byte into the UART RX ring buffer._ 
```C++
bool syn_uart_rx_isr_feed (
    SYN_UART * uart,
    uint8_t byte
) 
```



Call this from your UART RX ISR to push incoming data into the driver's buffer.




**Parameters:**


* `uart` UART handle. 
* `byte` The received byte. 



**Returns:**

true if the byte was stored, false if the RX buffer is full. 





        

<hr>



### function syn\_uart\_write 

_Write a buffer of bytes to the UART (blocking)._ 
```C++
SYN_Status syn_uart_write (
    const SYN_UART * uart,
    const uint8_t * data,
    size_t len,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `uart` UART handle. 
* `data` Data to transmit. 
* `len` Number of bytes. 
* `timeout_ms` Timeout in milliseconds (0 = no timeout). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_uart\_write\_str 

_Write a string to the UART (blocking)._ 
```C++
SYN_Status syn_uart_write_str (
    const SYN_UART * uart,
    const char * str,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `uart` UART handle. 
* `str` Null-terminated string. 
* `timeout_ms` Timeout in milliseconds (0 = no timeout). 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_UART\_MAX\_INSTANCES 

_Maximum UART instances supported._ 
```C++
#define SYN_UART_MAX_INSTANCES `2`
```




<hr>



### define SYN\_UART\_RX\_BUF\_SIZE 

_UART receive buffer size (bytes)._ 
```C++
#define SYN_UART_RX_BUF_SIZE `128`
```




<hr>



### define SYN\_UART\_TX\_BUF\_SIZE 

_UART transmit buffer size (bytes)._ 
```C++
#define SYN_UART_TX_BUF_SIZE `128`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_uart.h`

