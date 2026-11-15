

# File syn\_uart.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_uart.h**](syn__uart_8h.md)

[Go to the source code of this file](syn__uart_8h_source.md)

_UART driver — buffered I/O and formatted output._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_uart.h"`
* `#include "../util/syn_ringbuf.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_UART**](structSYN__UART.md) <br>_UART driver handle._  |
| struct | [**SYN\_UART\_Config**](structSYN__UART__Config.md) <br>_Configuration struct for UART initialization with optional DMA._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_uart\_deinit**](#function-syn_uart_deinit) ([**SYN\_UART**](structSYN__UART.md) \* uart) <br>_De-initialize a UART instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_uart\_init**](#function-syn_uart_init) ([**SYN\_UART**](structSYN__UART.md) \* uart, [**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint32\_t baudrate) <br>_Initialize a UART instance with buffered I/O._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_uart\_init\_config**](#function-syn_uart_init_config) ([**SYN\_UART**](structSYN__UART.md) \* uart, const [**SYN\_UART\_Config**](structSYN__UART__Config.md) \* cfg) <br>_Initialize a UART instance with custom configuration (optional DMA)._  |
|  size\_t | [**syn\_uart\_read**](#function-syn_uart_read) ([**SYN\_UART**](structSYN__UART.md) \* uart, uint8\_t \* data, size\_t max\_len) <br>_Read bytes from the UART RX ring buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uart\_rx\_isr\_feed**](#function-syn_uart_rx_isr_feed) ([**SYN\_UART**](structSYN__UART.md) \* uart, uint8\_t byte) <br>_Feed a received byte into the UART RX ring buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uart\_tx\_complete**](#function-syn_uart_tx_complete) (const [**SYN\_UART**](structSYN__UART.md) \* uart) <br>_Check if asynchronous TX is complete (tx\_rb drained AND hardware shift register clear)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uart\_tx\_isr\_flush**](#function-syn_uart_tx_isr_flush) ([**SYN\_UART**](structSYN__UART.md) \* uart) <br>_TX ISR drain callback — call from UART TXE interrupt._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_uart\_write**](#function-syn_uart_write) (const [**SYN\_UART**](structSYN__UART.md) \* uart, const uint8\_t \* data, size\_t len, uint32\_t timeout\_ms) <br>_Write a buffer of bytes to the UART (blocking)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_uart\_write\_async**](#function-syn_uart_write_async) ([**SYN\_UART**](structSYN__UART.md) \* uart, const uint8\_t \* data, size\_t len) <br>_Non-blocking write to UART TX ring buffer (all-or-nothing atomic)._  |
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



### function syn\_uart\_init\_config 

_Initialize a UART instance with custom configuration (optional DMA)._ 
```C++
SYN_Status syn_uart_init_config (
    SYN_UART * uart,
    const SYN_UART_Config * cfg
) 
```





**Parameters:**


* `uart` Pointer to a caller-owned [**SYN\_UART**](structSYN__UART.md) struct. 
* `cfg` Pointer to initialization configuration. 



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



### function syn\_uart\_tx\_complete 

_Check if asynchronous TX is complete (tx\_rb drained AND hardware shift register clear)._ 
```C++
bool syn_uart_tx_complete (
    const SYN_UART * uart
) 
```





**Parameters:**


* `uart` UART handle. 



**Returns:**

true if no bytes remain in software tx\_rb AND hardware TC flag is set. 





        

<hr>



### function syn\_uart\_tx\_isr\_flush 

_TX ISR drain callback — call from UART TXE interrupt._ 
```C++
bool syn_uart_tx_isr_flush (
    SYN_UART * uart
) 
```



Pops the next byte from tx\_rb into the hardware data register. When the ring buffer empties, disables the TXE interrupt.




**Parameters:**


* `uart` UART handle. 



**Returns:**

true if a byte was transmitted, false if tx\_rb is empty. 





        

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



### function syn\_uart\_write\_async 

_Non-blocking write to UART TX ring buffer (all-or-nothing atomic)._ 
```C++
SYN_Status syn_uart_write_async (
    SYN_UART * uart,
    const uint8_t * data,
    size_t len
) 
```



Pushes data into the internal TX ring buffer if space is available for all `len` bytes, and enables the TX interrupt to drain asynchronously. Returns SYN\_BUSY without modifying the buffer if space is insufficient.




**Parameters:**


* `uart` UART handle. 
* `data` Data to transmit. 
* `len` Number of bytes. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if ring buffer free space &lt; len. 





        

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

