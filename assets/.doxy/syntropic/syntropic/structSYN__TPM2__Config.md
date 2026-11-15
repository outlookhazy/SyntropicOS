

# Struct SYN\_TPM2\_Config



[**ClassList**](annotated.md) **>** [**SYN\_TPM2\_Config**](structSYN__TPM2__Config.md)



_TPM 2.0 Context Configuration Descriptor._ 

* `#include <syn_tpm2.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**transport**](#variable-transport)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |












































## Public Attributes Documentation




### variable rx\_buf 

```C++
uint8_t* SYN_TPM2_Config::rx_buf;
```



Scratch response buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_TPM2_Config::rx_buf_size;
```



Scratch response buffer capacity (&gt;= 256 bytes) 


        

<hr>



### variable transport 

```C++
SYN_Transport* SYN_TPM2_Config::transport;
```



Low-level SPI/I2C TCG FIFO transport interface 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_TPM2_Config::tx_buf;
```



Scratch command buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_TPM2_Config::tx_buf_size;
```



Scratch command buffer capacity (&gt;= 256 bytes) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_tpm2.h`

