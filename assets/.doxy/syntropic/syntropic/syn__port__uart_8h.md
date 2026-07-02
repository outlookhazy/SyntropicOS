

# File syn\_port\_uart.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_uart.h**](syn__port__uart_8h.md)

[Go to the source code of this file](syn__port__uart_8h_source.md)

_UART port interface — functions the user must implement._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_deinit**](#function-syn_port_uart_deinit) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance) <br>_De-initialize a UART peripheral._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_init**](#function-syn_port_uart_init) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint32\_t baudrate) <br>_Initialize a UART peripheral._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_receive**](#function-syn_port_uart_receive) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t \* data, size\_t len, size\_t \* received, uint32\_t timeout\_ms) <br>_Receive bytes into a buffer (blocking)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_receive\_byte**](#function-syn_port_uart_receive_byte) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t \* byte, uint32\_t timeout\_ms) <br>_Receive a single byte (blocking)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_transmit**](#function-syn_port_uart_transmit) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, const uint8\_t \* data, size\_t len, uint32\_t timeout\_ms) <br>_Transmit a buffer of bytes (blocking)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_transmit\_byte**](#function-syn_port_uart_transmit_byte) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t byte) <br>_Transmit a single byte (blocking)._  |




























## Detailed Description


Provides the low-level byte/buffer transmit and receive primitives that the higher-level syn\_uart driver builds upon. 


    
## Public Functions Documentation




### function syn\_port\_uart\_deinit 

_De-initialize a UART peripheral._ 
```C++
SYN_Status syn_port_uart_deinit (
    SYN_UARTInstance instance
) 
```





**Parameters:**


* `instance` UART instance to de-initialize. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_uart\_init 

_Initialize a UART peripheral._ 
```C++
SYN_Status syn_port_uart_init (
    SYN_UARTInstance instance,
    uint32_t baudrate
) 
```





**Parameters:**


* `instance` UART instance number (0, 1, 2, …). 
* `baudrate` Desired baud rate. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_uart\_receive 

_Receive bytes into a buffer (blocking)._ 
```C++
SYN_Status syn_port_uart_receive (
    SYN_UARTInstance instance,
    uint8_t * data,
    size_t len,
    size_t * received,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `instance` UART instance. 
* `data` Buffer to receive into. 
* `len` Maximum number of bytes to receive. 
* `received` [out] Actual number of bytes received. 
* `timeout_ms` Timeout in milliseconds (0 = no timeout). 



**Returns:**

SYN\_OK on success, SYN\_TIMEOUT if the timeout elapsed. 





        

<hr>



### function syn\_port\_uart\_receive\_byte 

_Receive a single byte (blocking)._ 
```C++
SYN_Status syn_port_uart_receive_byte (
    SYN_UARTInstance instance,
    uint8_t * byte,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `instance` UART instance. 
* `byte` [out] Received byte. 
* `timeout_ms` Timeout in milliseconds (0 = no timeout). 



**Returns:**

SYN\_OK if a byte was received, SYN\_TIMEOUT otherwise. 





        

<hr>



### function syn\_port\_uart\_transmit 

_Transmit a buffer of bytes (blocking)._ 
```C++
SYN_Status syn_port_uart_transmit (
    SYN_UARTInstance instance,
    const uint8_t * data,
    size_t len,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `instance` UART instance. 
* `data` Pointer to data to transmit. 
* `len` Number of bytes to transmit. 
* `timeout_ms` Timeout in milliseconds (0 = no timeout). 



**Returns:**

SYN\_OK on success, SYN\_TIMEOUT if the timeout elapsed. 





        

<hr>



### function syn\_port\_uart\_transmit\_byte 

_Transmit a single byte (blocking)._ 
```C++
SYN_Status syn_port_uart_transmit_byte (
    SYN_UARTInstance instance,
    uint8_t byte
) 
```





**Parameters:**


* `instance` UART instance. 
* `byte` Byte to transmit. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_uart.h`

