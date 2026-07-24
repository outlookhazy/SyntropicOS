

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
| struct | [**SYN\_Modbus\_DeviceInfo**](structSYN__Modbus__DeviceInfo.md) <br>_Device Information structure for FC 0x2B / 0x0E._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_Modbus\_ReadFileRecordCallback**](#typedef-syn_modbus_readfilerecordcallback)  <br>_Callback for reading file records (FC 0x14)._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_Modbus\_WriteCallback**](#typedef-syn_modbus_writecallback)  <br>_Called before a write to holding registers is applied._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_Modbus\_WriteFileRecordCallback**](#typedef-syn_modbus_writefilerecordcallback)  <br>_Callback for writing file records (FC 0x15)._  |




















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
| define  | [**SYN\_MB\_FC\_DIAGNOSTICS**](syn__modbus_8h.md#define-syn_mb_fc_diagnostics)  `0x08`<br> |
| define  | [**SYN\_MB\_FC\_GET\_COMM\_EVENT\_CNT**](syn__modbus_8h.md#define-syn_mb_fc_get_comm_event_cnt)  `0x0B`<br> |
| define  | [**SYN\_MB\_FC\_GET\_COMM\_EVENT\_LOG**](syn__modbus_8h.md#define-syn_mb_fc_get_comm_event_log)  `0x0C`<br> |
| define  | [**SYN\_MB\_FC\_MASK\_WRITE\_REGISTER**](syn__modbus_8h.md#define-syn_mb_fc_mask_write_register)  `0x16`<br> |
| define  | [**SYN\_MB\_FC\_READ\_COILS**](syn__modbus_8h.md#define-syn_mb_fc_read_coils)  `0x01`<br> |
| define  | [**SYN\_MB\_FC\_READ\_DEVICE\_INFO**](syn__modbus_8h.md#define-syn_mb_fc_read_device_info)  `0x2B`<br> |
| define  | [**SYN\_MB\_FC\_READ\_DISCRETE\_INPUTS**](syn__modbus_8h.md#define-syn_mb_fc_read_discrete_inputs)  `0x02`<br> |
| define  | [**SYN\_MB\_FC\_READ\_EXCEPTION\_STATUS**](syn__modbus_8h.md#define-syn_mb_fc_read_exception_status)  `0x07`<br> |
| define  | [**SYN\_MB\_FC\_READ\_FIFO\_QUEUE**](syn__modbus_8h.md#define-syn_mb_fc_read_fifo_queue)  `0x18`<br> |
| define  | [**SYN\_MB\_FC\_READ\_FILE\_RECORD**](syn__modbus_8h.md#define-syn_mb_fc_read_file_record)  `0x14`<br> |
| define  | [**SYN\_MB\_FC\_READ\_HOLDING**](syn__modbus_8h.md#define-syn_mb_fc_read_holding)  `0x03`<br> |
| define  | [**SYN\_MB\_FC\_READ\_INPUT**](syn__modbus_8h.md#define-syn_mb_fc_read_input)  `0x04`<br> |
| define  | [**SYN\_MB\_FC\_READ\_WRITE\_MULTIPLE**](syn__modbus_8h.md#define-syn_mb_fc_read_write_multiple)  `0x17`<br> |
| define  | [**SYN\_MB\_FC\_REPORT\_SERVER\_ID**](syn__modbus_8h.md#define-syn_mb_fc_report_server_id)  `0x11`<br> |
| define  | [**SYN\_MB\_FC\_WRITE\_FILE\_RECORD**](syn__modbus_8h.md#define-syn_mb_fc_write_file_record)  `0x15`<br> |
| define  | [**SYN\_MB\_FC\_WRITE\_MULTIPLE**](syn__modbus_8h.md#define-syn_mb_fc_write_multiple)  `0x10`<br> |
| define  | [**SYN\_MB\_FC\_WRITE\_MULTIPLE\_COILS**](syn__modbus_8h.md#define-syn_mb_fc_write_multiple_coils)  `0x0F`<br> |
| define  | [**SYN\_MB\_FC\_WRITE\_SINGLE**](syn__modbus_8h.md#define-syn_mb_fc_write_single)  `0x06`<br> |
| define  | [**SYN\_MB\_FC\_WRITE\_SINGLE\_COIL**](syn__modbus_8h.md#define-syn_mb_fc_write_single_coil)  `0x05`<br> |
| define  | [**SYN\_MB\_MEI\_TYPE\_READ\_DEVICE\_ID**](syn__modbus_8h.md#define-syn_mb_mei_type_read_device_id)  `0x0E`<br> |

## Detailed Description


Implements a Modbus RTU slave using UART, CRC-16/Modbus (in syn\_crc), and register/bit maps. Supports all 19 function codes:



* 0x01: Read Coils - 0x0F: Write Multiple Coils
* 0x02: Read Discrete Inputs - 0x10: Write Multiple Registers
* 0x03: Read Holding Registers - 0x11: Report Server ID
* 0x04: Read Input Registers - 0x14: Read File Record
* 0x05: Write Single Coil - 0x15: Write File Record
* 0x06: Write Single Register - 0x16: Mask Write Register
* 0x07: Read Exception Status - 0x17: Read/Write Multiple Registers
* 0x08: Serial Line Diagnostics - 0x18: Read FIFO Queue
* 0x0B: Get Comm Event Counter - 0x2B: Read Device Identification
* 0x0C: Get Comm Event Log




The register/coil maps are application-owned arrays. The Modbus module handles framing, CRC validation, and exception responses.




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




### typedef SYN\_Modbus\_ReadFileRecordCallback 

_Callback for reading file records (FC 0x14)._ 
```C++
typedef bool(* SYN_Modbus_ReadFileRecordCallback) (struct SYN_Modbus *mb, uint16_t file_num, uint16_t record_num, uint16_t record_len, uint16_t *record_data, void *ctx);
```




<hr>



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



### typedef SYN\_Modbus\_WriteFileRecordCallback 

_Callback for writing file records (FC 0x15)._ 
```C++
typedef bool(* SYN_Modbus_WriteFileRecordCallback) (struct SYN_Modbus *mb, uint16_t file_num, uint16_t record_num, uint16_t record_len, const uint16_t *record_data, void *ctx);
```




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



### define SYN\_MB\_FC\_DIAGNOSTICS 

```C++
#define SYN_MB_FC_DIAGNOSTICS `0x08`
```



Serial line diagnostics 
 


        

<hr>



### define SYN\_MB\_FC\_GET\_COMM\_EVENT\_CNT 

```C++
#define SYN_MB_FC_GET_COMM_EVENT_CNT `0x0B`
```



Get comm event counter 
 


        

<hr>



### define SYN\_MB\_FC\_GET\_COMM\_EVENT\_LOG 

```C++
#define SYN_MB_FC_GET_COMM_EVENT_LOG `0x0C`
```



Get comm event log 
 


        

<hr>



### define SYN\_MB\_FC\_MASK\_WRITE\_REGISTER 

```C++
#define SYN_MB_FC_MASK_WRITE_REGISTER `0x16`
```



Mask write register 
 


        

<hr>



### define SYN\_MB\_FC\_READ\_COILS 

```C++
#define SYN_MB_FC_READ_COILS `0x01`
```



Read coils (bits) 
 


        

<hr>



### define SYN\_MB\_FC\_READ\_DEVICE\_INFO 

```C++
#define SYN_MB_FC_READ_DEVICE_INFO `0x2B`
```



Encapsulated interface trans 


        

<hr>



### define SYN\_MB\_FC\_READ\_DISCRETE\_INPUTS 

```C++
#define SYN_MB_FC_READ_DISCRETE_INPUTS `0x02`
```



Read discrete inputs (bits) 
 


        

<hr>



### define SYN\_MB\_FC\_READ\_EXCEPTION\_STATUS 

```C++
#define SYN_MB_FC_READ_EXCEPTION_STATUS `0x07`
```



Read exception status 
 


        

<hr>



### define SYN\_MB\_FC\_READ\_FIFO\_QUEUE 

```C++
#define SYN_MB_FC_READ_FIFO_QUEUE `0x18`
```



Read FIFO queue 
 


        

<hr>



### define SYN\_MB\_FC\_READ\_FILE\_RECORD 

```C++
#define SYN_MB_FC_READ_FILE_RECORD `0x14`
```



Read file record 
 


        

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



### define SYN\_MB\_FC\_READ\_WRITE\_MULTIPLE 

```C++
#define SYN_MB_FC_READ_WRITE_MULTIPLE `0x17`
```



Read/Write multiple regs 
 


        

<hr>



### define SYN\_MB\_FC\_REPORT\_SERVER\_ID 

```C++
#define SYN_MB_FC_REPORT_SERVER_ID `0x11`
```



Report server ID 
 


        

<hr>



### define SYN\_MB\_FC\_WRITE\_FILE\_RECORD 

```C++
#define SYN_MB_FC_WRITE_FILE_RECORD `0x15`
```



Write file record 
 


        

<hr>



### define SYN\_MB\_FC\_WRITE\_MULTIPLE 

```C++
#define SYN_MB_FC_WRITE_MULTIPLE `0x10`
```



Write multiple registers 
 


        

<hr>



### define SYN\_MB\_FC\_WRITE\_MULTIPLE\_COILS 

```C++
#define SYN_MB_FC_WRITE_MULTIPLE_COILS `0x0F`
```



Write multiple coils (bits) 
 


        

<hr>



### define SYN\_MB\_FC\_WRITE\_SINGLE 

```C++
#define SYN_MB_FC_WRITE_SINGLE `0x06`
```



Write single register 
 


        

<hr>



### define SYN\_MB\_FC\_WRITE\_SINGLE\_COIL 

```C++
#define SYN_MB_FC_WRITE_SINGLE_COIL `0x05`
```



Write single coil (bit) 
 


        

<hr>



### define SYN\_MB\_MEI\_TYPE\_READ\_DEVICE\_ID 

```C++
#define SYN_MB_MEI_TYPE_READ_DEVICE_ID `0x0E`
```



Read Device Identification 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus.h`

