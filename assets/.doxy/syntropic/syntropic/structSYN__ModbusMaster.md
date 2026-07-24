

# Struct SYN\_ModbusMaster



[**ClassList**](annotated.md) **>** [**SYN\_ModbusMaster**](structSYN__ModbusMaster.md)



_Modbus Master instance configuration & transaction state._ 

* `#include <syn_modbus_master.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buf**](#variable-buf)  <br> |
|  uint16\_t | [**count**](#variable-count)  <br> |
|  uint8\_t | [**exception\_code**](#variable-exception_code)  <br> |
|  uint8\_t | [**func\_code**](#variable-func_code)  <br> |
|  uint32\_t | [**last\_byte\_tick\_ms**](#variable-last_byte_tick_ms)  <br> |
|  uint16\_t | [**read\_count**](#variable-read_count)  <br> |
|  uint16\_t | [**read\_data**](#variable-read_data)  <br> |
|  uint32\_t | [**request\_tick\_ms**](#variable-request_tick_ms)  <br> |
|  uint8\_t | [**response\_fc**](#variable-response_fc)  <br> |
|  uint16\_t | [**rx\_len**](#variable-rx_len)  <br> |
|  uint8\_t | [**slave\_addr**](#variable-slave_addr)  <br> |
|  uint16\_t | [**start\_addr**](#variable-start_addr)  <br> |
|  [**SYN\_ModbusMaster\_State**](syn__modbus__master_8h.md#enum-syn_modbusmaster_state) | [**state**](#variable-state)  <br> |
|  uint32\_t | [**timeout\_ms**](#variable-timeout_ms)  <br> |
|  uint16\_t | [**tx\_len**](#variable-tx_len)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
uint8_t SYN_ModbusMaster::buf[256];
```



Frame buffer 
 


        

<hr>



### variable count 

```C++
uint16_t SYN_ModbusMaster::count;
```



Register quantity / write value 
 


        

<hr>



### variable exception\_code 

```C++
uint8_t SYN_ModbusMaster::exception_code;
```



Exception code if error 
 


        

<hr>



### variable func\_code 

```C++
uint8_t SYN_ModbusMaster::func_code;
```



Active function code 
 


        

<hr>



### variable last\_byte\_tick\_ms 

```C++
uint32_t SYN_ModbusMaster::last_byte_tick_ms;
```



Tick of last received RX byte 
 


        

<hr>



### variable read\_count 

```C++
uint16_t SYN_ModbusMaster::read_count;
```



Number of registers read 
 


        

<hr>



### variable read\_data 

```C++
uint16_t SYN_ModbusMaster::read_data[125];
```



Received read data buffer 
 


        

<hr>



### variable request\_tick\_ms 

```C++
uint32_t SYN_ModbusMaster::request_tick_ms;
```



Tick when request was dispatched 
 


        

<hr>



### variable response\_fc 

```C++
uint8_t SYN_ModbusMaster::response_fc;
```



Response function code 
 


        

<hr>



### variable rx\_len 

```C++
uint16_t SYN_ModbusMaster::rx_len;
```



Received response length 
 


        

<hr>



### variable slave\_addr 

```C++
uint8_t SYN_ModbusMaster::slave_addr;
```



Target slave address (1–247) 
 


        

<hr>



### variable start\_addr 

```C++
uint16_t SYN_ModbusMaster::start_addr;
```



Target register start address 
 


        

<hr>



### variable state 

```C++
SYN_ModbusMaster_State SYN_ModbusMaster::state;
```



Master transaction state 
 


        

<hr>



### variable timeout\_ms 

```C++
uint32_t SYN_ModbusMaster::timeout_ms;
```



Response timeout limit in ms 
 


        

<hr>



### variable tx\_len 

```C++
uint16_t SYN_ModbusMaster::tx_len;
```



Transmitted request length 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus_master.h`

