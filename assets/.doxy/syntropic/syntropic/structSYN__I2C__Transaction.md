

# Struct SYN\_I2C\_Transaction



[**ClassList**](annotated.md) **>** [**SYN\_I2C\_Transaction**](structSYN__I2C__Transaction.md)



_I2C Transaction Descriptor._ 

* `#include <syn_i2c_queue.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**addr**](#variable-addr)  <br> |
|  uint8\_t | [**bus**](#variable-bus)  <br> |
|  [**SYN\_I2C\_Queue\_Callback**](syn__i2c__queue_8h.md#typedef-syn_i2c_queue_callback) | [**callback**](#variable-callback)  <br> |
|  uint32\_t | [**clock\_speed\_hz**](#variable-clock_speed_hz)  <br> |
|  uint8\_t \* | [**rx\_data**](#variable-rx_data)  <br> |
|  size\_t | [**rx\_len**](#variable-rx_len)  <br> |
|  const uint8\_t \* | [**tx\_data**](#variable-tx_data)  <br> |
|  size\_t | [**tx\_len**](#variable-tx_len)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable addr 

```C++
uint16_t SYN_I2C_Transaction::addr;
```



7-bit target slave address 


        

<hr>



### variable bus 

```C++
uint8_t SYN_I2C_Transaction::bus;
```



I2C bus index 


        

<hr>



### variable callback 

```C++
SYN_I2C_Queue_Callback SYN_I2C_Transaction::callback;
```



Completion callback 


        

<hr>



### variable clock\_speed\_hz 

```C++
uint32_t SYN_I2C_Transaction::clock_speed_hz;
```



Target clock frequency in Hz (0 = default 100 kHz) 


        

<hr>



### variable rx\_data 

```C++
uint8_t* SYN_I2C_Transaction::rx_data;
```



RX buffer (NULL if write-only) 


        

<hr>



### variable rx\_len 

```C++
size_t SYN_I2C_Transaction::rx_len;
```



RX byte count 


        

<hr>



### variable tx\_data 

```C++
const uint8_t* SYN_I2C_Transaction::tx_data;
```



TX buffer (NULL if read-only) 


        

<hr>



### variable tx\_len 

```C++
size_t SYN_I2C_Transaction::tx_len;
```



TX byte count 


        

<hr>



### variable user\_data 

```C++
void* SYN_I2C_Transaction::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_i2c_queue.h`

