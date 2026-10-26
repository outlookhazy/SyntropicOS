

# Struct SYN\_Modbus\_Config



[**ClassList**](annotated.md) **>** [**SYN\_Modbus\_Config**](structSYN__Modbus__Config.md)



_Modbus RTU slave configuration._ 

* `#include <syn_modbus.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**coils**](#variable-coils)  <br> |
|  uint16\_t | [**coils\_count**](#variable-coils_count)  <br> |
|  const [**SYN\_Modbus\_DeviceInfo**](structSYN__Modbus__DeviceInfo.md) \* | [**device\_info**](#variable-device_info)  <br> |
|  uint16\_t | [**discrete\_count**](#variable-discrete_count)  <br> |
|  uint8\_t \* | [**discrete\_inputs**](#variable-discrete_inputs)  <br> |
|  uint8\_t | [**exception\_status**](#variable-exception_status)  <br> |
|  uint16\_t | [**fifo\_count**](#variable-fifo_count)  <br> |
|  const uint16\_t \* | [**fifo\_queue**](#variable-fifo_queue)  <br> |
|  void \* | [**file\_cb\_ctx**](#variable-file_cb_ctx)  <br> |
|  uint16\_t | [**holding\_count**](#variable-holding_count)  <br> |
|  uint16\_t \* | [**holding\_regs**](#variable-holding_regs)  <br> |
|  uint16\_t | [**input\_count**](#variable-input_count)  <br> |
|  uint16\_t \* | [**input\_regs**](#variable-input_regs)  <br> |
|  [**SYN\_Modbus\_ReadFileRecordCallback**](syn__modbus_8h.md#typedef-syn_modbus_readfilerecordcallback) | [**on\_read\_file**](#variable-on_read_file)  <br> |
|  [**SYN\_Modbus\_WriteCallback**](syn__modbus_8h.md#typedef-syn_modbus_writecallback) | [**on\_write**](#variable-on_write)  <br> |
|  void \* | [**on\_write\_ctx**](#variable-on_write_ctx)  <br> |
|  [**SYN\_Modbus\_WriteFileRecordCallback**](syn__modbus_8h.md#typedef-syn_modbus_writefilerecordcallback) | [**on\_write\_file**](#variable-on_write_file)  <br> |
|  const uint8\_t \* | [**server\_id**](#variable-server_id)  <br> |
|  uint8\_t | [**server\_id\_len**](#variable-server_id_len)  <br> |
|  uint32\_t | [**silence\_ms**](#variable-silence_ms)  <br> |
|  uint8\_t | [**slave\_addr**](#variable-slave_addr)  <br> |
|  [**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) | [**uart**](#variable-uart)  <br> |












































## Public Attributes Documentation




### variable coils 

```C++
uint8_t* SYN_Modbus_Config::coils;
```



Read/write coils (1 byte = 8 bits) 


        

<hr>



### variable coils\_count 

```C++
uint16_t SYN_Modbus_Config::coils_count;
```



Total number of coil bits 
 


        

<hr>



### variable device\_info 

```C++
const SYN_Modbus_DeviceInfo* SYN_Modbus_Config::device_info;
```



Device Info for FC 0x2B/0x0E 


        

<hr>



### variable discrete\_count 

```C++
uint16_t SYN_Modbus_Config::discrete_count;
```



Total discrete input bits 
 


        

<hr>



### variable discrete\_inputs 

```C++
uint8_t* SYN_Modbus_Config::discrete_inputs;
```



Read-only discrete inputs (1 byte = 8 bits) 


        

<hr>



### variable exception\_status 

```C++
uint8_t SYN_Modbus_Config::exception_status;
```



Status byte for FC 0x07 
 


        

<hr>



### variable fifo\_count 

```C++
uint16_t SYN_Modbus_Config::fifo_count;
```



Number of elements in FIFO 
 


        

<hr>



### variable fifo\_queue 

```C++
const uint16_t* SYN_Modbus_Config::fifo_queue;
```



Pointer to FIFO queue elements 
 


        

<hr>



### variable file\_cb\_ctx 

```C++
void* SYN_Modbus_Config::file_cb_ctx;
```



Context for file callbacks 


        

<hr>



### variable holding\_count 

```C++
uint16_t SYN_Modbus_Config::holding_count;
```



Number of holding registers 
 


        

<hr>



### variable holding\_regs 

```C++
uint16_t* SYN_Modbus_Config::holding_regs;
```



Read/write holding registers 
 


        

<hr>



### variable input\_count 

```C++
uint16_t SYN_Modbus_Config::input_count;
```



Number of input registers 
 


        

<hr>



### variable input\_regs 

```C++
uint16_t* SYN_Modbus_Config::input_regs;
```



Read-only input registers 
 


        

<hr>



### variable on\_read\_file 

```C++
SYN_Modbus_ReadFileRecordCallback SYN_Modbus_Config::on_read_file;
```



File read callback FC 0x14 


        

<hr>



### variable on\_write 

```C++
SYN_Modbus_WriteCallback SYN_Modbus_Config::on_write;
```



Write pre-check callback 
 


        

<hr>



### variable on\_write\_ctx 

```C++
void* SYN_Modbus_Config::on_write_ctx;
```



Context for on\_write 
 


        

<hr>



### variable on\_write\_file 

```C++
SYN_Modbus_WriteFileRecordCallback SYN_Modbus_Config::on_write_file;
```



File write callback FC 0x15 


        

<hr>



### variable server\_id 

```C++
const uint8_t* SYN_Modbus_Config::server_id;
```



Server ID bytes for FC 0x11 (NULL = "SYN-MB") 


        

<hr>



### variable server\_id\_len 

```C++
uint8_t SYN_Modbus_Config::server_id_len;
```



Length of server\_id bytes (max 240) 


        

<hr>



### variable silence\_ms 

```C++
uint32_t SYN_Modbus_Config::silence_ms;
```



Custom inter-frame silence gap in ms (0 = default 5ms) 


        

<hr>



### variable slave\_addr 

```C++
uint8_t SYN_Modbus_Config::slave_addr;
```



Modbus slave address (1–247) 
 


        

<hr>



### variable uart 

```C++
SYN_UARTInstance SYN_Modbus_Config::uart;
```



UART instance to use 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus.h`

