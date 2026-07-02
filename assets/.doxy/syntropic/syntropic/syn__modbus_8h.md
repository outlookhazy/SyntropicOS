

# File syn\_modbus.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus.h**](syn__modbus_8h.md)

[Go to the source code of this file](syn__modbus_8h_source.md)

_Modbus RTU slave implementation._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_uart.h"`
* `#include "../util/syn_crc.h"`
* `#include "../port/syn_port_system.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Modbus**](structSYN__Modbus.md) <br>_Modbus RTU slave instance — frame buffer, config, and statistics._  |
| struct | [**SYN\_Modbus\_Config**](structSYN__Modbus__Config.md) <br>_Modbus RTU slave configuration._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_Modbus\_WriteCallback**](#typedef-syn_modbus_writecallback)  <br>_Called before a write to holding registers is applied._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_modbus\_feed**](#function-syn_modbus_feed) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, uint8\_t byte) <br>_Feed a byte to the Modbus receiver._  |
|  void | [**syn\_modbus\_init**](#function-syn_modbus_init) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, const [**SYN\_Modbus\_Config**](structSYN__Modbus__Config.md) \* cfg, uint8\_t \* buf, uint16\_t buf\_size) <br>_Initialize the Modbus RTU slave._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_modbus\_poll**](#function-syn_modbus_poll) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Poll for incoming Modbus requests and process them._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_modbus\_process**](#function-syn_modbus_process) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Process a complete frame (call after feed + silence detection)._  |
|  void | [**syn\_modbus\_reset**](#function-syn_modbus_reset) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Reset the receiver state._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MB\_EX\_DEVICE\_FAILURE**](syn__modbus_8h.md#define-syn_mb_ex_device_failure)  `0x04`<br> |
| define  | [**SYN\_MB\_EX\_ILLEGAL\_ADDR**](syn__modbus_8h.md#define-syn_mb_ex_illegal_addr)  `0x02`<br> |
| define  | [**SYN\_MB\_EX\_ILLEGAL\_FUNC**](syn__modbus_8h.md#define-syn_mb_ex_illegal_func)  `0x01`<br> |
| define  | [**SYN\_MB\_EX\_ILLEGAL\_VALUE**](syn__modbus_8h.md#define-syn_mb_ex_illegal_value)  `0x03`<br> |
| define  | [**SYN\_MB\_FC\_READ\_HOLDING**](syn__modbus_8h.md#define-syn_mb_fc_read_holding)  `0x03`<br> |
| define  | [**SYN\_MB\_FC\_READ\_INPUT**](syn__modbus_8h.md#define-syn_mb_fc_read_input)  `0x04`<br> |
| define  | [**SYN\_MB\_FC\_WRITE\_MULTIPLE**](syn__modbus_8h.md#define-syn_mb_fc_write_multiple)  `0x10`<br> |
| define  | [**SYN\_MB\_FC\_WRITE\_SINGLE**](syn__modbus_8h.md#define-syn_mb_fc_write_single)  `0x06`<br> |

## Detailed Description


Implements a Modbus RTU slave using UART, CRC-16/Modbus (already in syn\_crc), and a register map. Supports the most common function codes:



* 0x03: Read Holding Registers
* 0x04: Read Input Registers
* 0x06: Write Single Register
* 0x10: Write Multiple Registers




The register map is a flat array of uint16\_t that your application reads/writes directly. The Modbus module handles framing, CRC, and exception responses.




**
**


```C++
static uint16_t holding_regs[32];
static uint16_t input_regs[16];
static SYN_Modbus mb;
static uint8_t mb_buf[256];

SYN_Modbus_Config cfg = {
    .slave_addr = 1,
    .uart = SYN_UART0,
    .holding_regs = holding_regs,
    .holding_count = 32,
    .input_regs = input_regs,
    .input_count = 16,
};
syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));

// In main loop:
syn_modbus_poll(&mb);  // checks UART, processes requests

// Your app writes input registers:
input_regs[0] = adc_value;

// Your app reads holding registers (written by master):
uint16_t setpoint = holding_regs[0];
```
 





    
## Public Types Documentation




### typedef SYN\_Modbus\_WriteCallback 

_Called before a write to holding registers is applied._ 
```C++
typedef bool(* SYN_Modbus_WriteCallback) (struct SYN_Modbus *mb, uint16_t addr, uint16_t count, void *ctx);
```





**Parameters:**


* `mb` Modbus instance. 
* `addr` Register start address. 
* `count` Number of registers. 
* `ctx` User context. 



**Returns:**

true to allow the write, false to reject (exception 0x03). 





        

<hr>
## Public Functions Documentation




### function syn\_modbus\_feed 

_Feed a byte to the Modbus receiver._ 
```C++
void syn_modbus_feed (
    SYN_Modbus * mb,
    uint8_t byte
) 
```



Alternative to poll() — call from UART ISR if you want interrupt-driven reception.




**Parameters:**


* `mb` Modbus instance. 
* `byte` Received byte. 




        

<hr>



### function syn\_modbus\_init 

_Initialize the Modbus RTU slave._ 
```C++
void syn_modbus_init (
    SYN_Modbus * mb,
    const SYN_Modbus_Config * cfg,
    uint8_t * buf,
    uint16_t buf_size
) 
```





**Parameters:**


* `mb` Modbus instance. 
* `cfg` Configuration. 
* `buf` Frame buffer (must be at least 256 bytes). 
* `buf_size` Buffer capacity. 




        

<hr>



### function syn\_modbus\_poll 

_Poll for incoming Modbus requests and process them._ 
```C++
bool syn_modbus_poll (
    SYN_Modbus * mb
) 
```



Call from your main loop. This reads bytes from UART, detects frame boundaries (3.5 character times silence), validates CRC, and processes the request.




**Parameters:**


* `mb` Modbus instance. 



**Returns:**

true if a request was processed. 





        

<hr>



### function syn\_modbus\_process 

_Process a complete frame (call after feed + silence detection)._ 
```C++
bool syn_modbus_process (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 



**Returns:**

true if a response was sent. 





        

<hr>



### function syn\_modbus\_reset 

_Reset the receiver state._ 
```C++
void syn_modbus_reset (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 




        

<hr>
## Macro Definition Documentation





### define SYN\_MB\_EX\_DEVICE\_FAILURE 

```C++
#define SYN_MB_EX_DEVICE_FAILURE `0x04`
```



Device failure 
 


        

<hr>



### define SYN\_MB\_EX\_ILLEGAL\_ADDR 

```C++
#define SYN_MB_EX_ILLEGAL_ADDR `0x02`
```



Illegal data address 
 


        

<hr>



### define SYN\_MB\_EX\_ILLEGAL\_FUNC 

```C++
#define SYN_MB_EX_ILLEGAL_FUNC `0x01`
```



Illegal function code 
 


        

<hr>



### define SYN\_MB\_EX\_ILLEGAL\_VALUE 

```C++
#define SYN_MB_EX_ILLEGAL_VALUE `0x03`
```



Illegal data value 
 


        

<hr>



### define SYN\_MB\_FC\_READ\_HOLDING 

```C++
#define SYN_MB_FC_READ_HOLDING `0x03`
```



Read holding registers 
 


        

<hr>



### define SYN\_MB\_FC\_READ\_INPUT 

```C++
#define SYN_MB_FC_READ_INPUT `0x04`
```



Read input registers 
 


        

<hr>



### define SYN\_MB\_FC\_WRITE\_MULTIPLE 

```C++
#define SYN_MB_FC_WRITE_MULTIPLE `0x10`
```



Write multiple registers 
 


        

<hr>



### define SYN\_MB\_FC\_WRITE\_SINGLE 

```C++
#define SYN_MB_FC_WRITE_SINGLE `0x06`
```



Write single register 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus.h`

