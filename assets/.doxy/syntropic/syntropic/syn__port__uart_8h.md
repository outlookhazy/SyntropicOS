

# File syn\_port\_uart.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_uart.h**](syn__port__uart_8h.md)

[Go to the source code of this file](syn__port__uart_8h_source.md)

_UART port interface — functions the user must implement._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_deinit**](#function-syn_port_uart_deinit) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance) <br>_De-initialize a UART peripheral._  |
|  void | [**syn\_port\_uart\_disable\_txe\_irq**](#function-syn_port_uart_disable_txe_irq) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance) <br>_Disable UART TXE interrupt._  |
|  void | [**syn\_port\_uart\_enable\_txe\_irq**](#function-syn_port_uart_enable_txe_irq) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance) <br>_Enable UART TXE (Transmit Data Register Empty) interrupt._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_init**](#function-syn_port_uart_init) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint32\_t baudrate) <br>_Initialize a UART peripheral._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_uart\_is\_tc\_set**](#function-syn_port_uart_is_tc_set) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance) <br>_Check if UART Transfer Complete (TC / Shift Register empty) flag is set._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_receive**](#function-syn_port_uart_receive) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t \* data, size\_t len, size\_t \* received, uint32\_t timeout\_ms) <br>_Receive bytes into a buffer (blocking)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_receive\_byte**](#function-syn_port_uart_receive_byte) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t \* byte, uint32\_t timeout\_ms) <br>_Receive a single byte (blocking)._  |
|  void | [**syn\_port\_uart\_set\_tc\_callback**](#function-syn_port_uart_set_tc_callback) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, void(\*)(void \*ctx) callback, void \* ctx) <br>_Register a Transfer Complete (TC) callback for RS485 DE pin control._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_transmit**](#function-syn_port_uart_transmit) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, const uint8\_t \* data, size\_t len, uint32\_t timeout\_ms) <br>_Transmit a buffer of bytes (blocking)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_transmit\_byte**](#function-syn_port_uart_transmit_byte) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t byte) <br>_Transmit a single byte (blocking)._  |
|  void | [**syn\_port\_uart\_write\_dr**](#function-syn_port_uart_write_dr) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t byte) <br>_Write a single byte to the hardware UART Data Register without waiting._  |




























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



### function syn\_port\_uart\_disable\_txe\_irq 

_Disable UART TXE interrupt._ 
```C++
void syn_port_uart_disable_txe_irq (
    SYN_UARTInstance instance
) 
```





**Parameters:**


* `instance` UART instance. 




        

<hr>



### function syn\_port\_uart\_enable\_txe\_irq 

_Enable UART TXE (Transmit Data Register Empty) interrupt._ 
```C++
void syn_port_uart_enable_txe_irq (
    SYN_UARTInstance instance
) 
```





**Parameters:**


* `instance` UART instance. 




        

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



### function syn\_port\_uart\_is\_tc\_set 

_Check if UART Transfer Complete (TC / Shift Register empty) flag is set._ 
```C++
bool syn_port_uart_is_tc_set (
    SYN_UARTInstance instance
) 
```





**Parameters:**


* `instance` UART instance. 



**Returns:**

true if hardware shift register is clear. 





        

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



### function syn\_port\_uart\_set\_tc\_callback 

_Register a Transfer Complete (TC) callback for RS485 DE pin control._ 
```C++
void syn_port_uart_set_tc_callback (
    SYN_UARTInstance instance,
    void(*)(void *ctx) callback,
    void * ctx
) 
```





**Parameters:**


* `instance` UART instance. 
* `callback` Function to call on TC, or NULL to disable. 
* `ctx` User context passed to callback. 




        

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



### function syn\_port\_uart\_write\_dr 

_Write a single byte to the hardware UART Data Register without waiting._ 
```C++
void syn_port_uart_write_dr (
    SYN_UARTInstance instance,
    uint8_t byte
) 
```





**Parameters:**


* `instance` UART instance. 
* `byte` Byte to write. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_uart.h`

