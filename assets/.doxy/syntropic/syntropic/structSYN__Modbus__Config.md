

# Struct SYN\_Modbus\_Config



[**ClassList**](annotated.md) **>** [**SYN\_Modbus\_Config**](structSYN__Modbus__Config.md)



_Modbus RTU slave configuration._ 

* `#include <syn_modbus.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**holding\_count**](#variable-holding_count)  <br> |
|  uint16\_t \* | [**holding\_regs**](#variable-holding_regs)  <br> |
|  uint16\_t | [**input\_count**](#variable-input_count)  <br> |
|  uint16\_t \* | [**input\_regs**](#variable-input_regs)  <br> |
|  [**SYN\_Modbus\_WriteCallback**](syn__modbus_8h.md#typedef-syn_modbus_writecallback) | [**on\_write**](#variable-on_write)  <br> |
|  void \* | [**on\_write\_ctx**](#variable-on_write_ctx)  <br> |
|  uint8\_t | [**slave\_addr**](#variable-slave_addr)  <br> |
|  [**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) | [**uart**](#variable-uart)  <br> |












































## Public Attributes Documentation




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

