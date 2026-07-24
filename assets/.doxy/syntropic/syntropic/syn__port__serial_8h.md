

# File syn\_port\_serial.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_serial.h**](syn__port__serial_8h.md)

[Go to the source code of this file](syn__port__serial_8h_source.md)

_Console serial port interface._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_serial\_init**](#function-syn_port_serial_init) (uint32\_t baudrate) <br>_Initialize the console serial port._  |
|  int | [**syn\_port\_serial\_read**](#function-syn_port_serial_read) (uint8\_t \* buf, size\_t max\_len) <br>_Read available bytes from the console (non-blocking)._  |
|  int | [**syn\_port\_serial\_write**](#function-syn_port_serial_write) (const uint8\_t \* data, size\_t len) <br>_Write bytes to the console._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int | [**syn\_port\_serial\_read\_byte**](#function-syn_port_serial_read_byte) (uint8\_t \* byte) <br>_Read a single byte from the console (non-blocking)._  |
|  int | [**syn\_port\_serial\_write\_byte**](#function-syn_port_serial_write_byte) (uint8\_t byte) <br>_Write a single byte to the console._  |


























## Detailed Description


Provides the low-level byte I/O primitives for the system console. The console is a singleton — there is exactly one per project. It may be backed by a hardware UART, USB CDC, or any other byte stream.


The CLI and logging system use this interface directly — no callback wiring is needed in application code.




**
**


* `write` may briefly block for buffer space but must not spin indefinitely.
* `read` is strictly **non-blocking**: returns 0 if nothing is available.






**
**

For non-console UARTs (Modbus, GPS, sensors), use [**syn\_port\_uart.h**](syn__port__uart_8h.md) which provides instance-based access to hardware UART peripherals. 





    
## Public Functions Documentation




### function syn\_port\_serial\_init 

_Initialize the console serial port._ 
```C++
SYN_Status syn_port_serial_init (
    uint32_t baudrate
) 
```





**Parameters:**


* `baudrate` Desired baud rate. Pass 0 for default (or if the underlying transport ignores baud rate, e.g. USB CDC). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_serial\_read 

_Read available bytes from the console (non-blocking)._ 
```C++
int syn_port_serial_read (
    uint8_t * buf,
    size_t max_len
) 
```



Returns immediately with however many bytes are available, up to `max_len`. Returns 0 if no data is available. **Never blocks.**




**Parameters:**


* `buf` Buffer to read into. 
* `max_len` Maximum bytes to read. 



**Returns:**

Number of bytes read (0 if nothing available), or -1 on error. 





        

<hr>



### function syn\_port\_serial\_write 

_Write bytes to the console._ 
```C++
int syn_port_serial_write (
    const uint8_t * data,
    size_t len
) 
```



Writes up to `len` bytes. May briefly block if the transmit buffer is full, but must not spin indefinitely.




**Parameters:**


* `data` Data to write. 
* `len` Number of bytes. 



**Returns:**

Number of bytes actually written, or -1 on error. 





        

<hr>
## Public Static Functions Documentation




### function syn\_port\_serial\_read\_byte 

_Read a single byte from the console (non-blocking)._ 
```C++
static inline int syn_port_serial_read_byte (
    uint8_t * byte
) 
```





**Parameters:**


* `byte` [out] Received byte. 



**Returns:**

1 if a byte was read, 0 if nothing available. 





        

<hr>



### function syn\_port\_serial\_write\_byte 

_Write a single byte to the console._ 
```C++
static inline int syn_port_serial_write_byte (
    uint8_t byte
) 
```





**Parameters:**


* `byte` Byte to write. 



**Returns:**

1 on success, -1 on error. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_serial.h`

