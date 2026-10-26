

# Struct SYN\_EcatMaster



[**ClassList**](annotated.md) **>** [**SYN\_EcatMaster**](structSYN__EcatMaster.md)



_EtherCAT Master Instance Context (Zero-Heap)._ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**current\_slave\_idx**](#variable-current_slave_idx)  <br> |
|  uint32\_t | [**cycle\_count**](#variable-cycle_count)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**frame\_rx\_ready**](#variable-frame_rx_ready)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**frame\_tx\_pending**](#variable-frame_tx_pending)  <br> |
|  uint8\_t \* | [**input\_image**](#variable-input_image)  <br> |
|  uint16\_t | [**input\_size**](#variable-input_size)  <br> |
|  uint32\_t | [**logical\_addr**](#variable-logical_addr)  <br> |
|  [**SYN\_EcatState**](syn__ethercat_8h.md#enum-syn_ecatstate) | [**master\_state**](#variable-master_state)  <br> |
|  uint8\_t \* | [**output\_image**](#variable-output_image)  <br> |
|  uint16\_t | [**output\_size**](#variable-output_size)  <br> |
|  uint8\_t | [**pdo\_count**](#variable-pdo_count)  <br> |
|  uint8\_t | [**pdo\_entry\_idx**](#variable-pdo_entry_idx)  <br> |
|  uint16\_t | [**pdo\_map\_idx**](#variable-pdo_map_idx)  <br> |
|  size\_t | [**pdo\_read\_len**](#variable-pdo_read_len)  <br> |
|  uint32\_t | [**pdo\_sdo\_val**](#variable-pdo_sdo_val)  <br> |
|  uint16\_t | [**poll\_retries**](#variable-poll_retries)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint16\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  uint16\_t | [**rx\_frame\_len**](#variable-rx_frame_len)  <br> |
|  uint8\_t | [**slave\_count**](#variable-slave_count)  <br> |
|  [**SYN\_EcatSlaveInfo**](structSYN__EcatSlaveInfo.md) | [**slaves**](#variable-slaves)  <br> |
|  [**SYN\_PT**](structSYN__PT.md) | [**sub\_pt**](#variable-sub_pt)  <br> |
|  uint32\_t | [**total\_rx\_bits**](#variable-total_rx_bits)  <br> |
|  uint32\_t | [**total\_tx\_bits**](#variable-total_tx_bits)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  uint16\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  uint16\_t | [**tx\_frame\_len**](#variable-tx_frame_len)  <br> |
|  uint32\_t | [**wkc\_errors**](#variable-wkc_errors)  <br> |
|  uint16\_t | [**wkc\_expected**](#variable-wkc_expected)  <br> |
|  uint16\_t | [**wkc\_last**](#variable-wkc_last)  <br> |












































## Public Attributes Documentation




### variable current\_slave\_idx 

```C++
uint8_t SYN_EcatMaster::current_slave_idx;
```



Iteration index for multi-step slave tasks 


        

<hr>



### variable cycle\_count 

```C++
uint32_t SYN_EcatMaster::cycle_count;
```



Completed cyclic process exchange counter 


        

<hr>



### variable frame\_rx\_ready 

```C++
bool SYN_EcatMaster::frame_rx_ready;
```



Receive frame ready from PHY 


        

<hr>



### variable frame\_tx\_pending 

```C++
bool SYN_EcatMaster::frame_tx_pending;
```



Transmit frame ready for PHY 


        

<hr>



### variable input\_image 

```C++
uint8_t* SYN_EcatMaster::input_image;
```



Slaves -&gt; Master cyclic process input buffer 


        

<hr>



### variable input\_size 

```C++
uint16_t SYN_EcatMaster::input_size;
```



Cyclic process input size in bytes 


        

<hr>



### variable logical\_addr 

```C++
uint32_t SYN_EcatMaster::logical_addr;
```



Logical base address for LRW process exchange 


        

<hr>



### variable master\_state 

```C++
SYN_EcatState SYN_EcatMaster::master_state;
```



Current Master bus state 


        

<hr>



### variable output\_image 

```C++
uint8_t* SYN_EcatMaster::output_image;
```



Master -&gt; Slaves cyclic process output buffer 


        

<hr>



### variable output\_size 

```C++
uint16_t SYN_EcatMaster::output_size;
```



Cyclic process output size in bytes 


        

<hr>



### variable pdo\_count 

```C++
uint8_t SYN_EcatMaster::pdo_count;
```



Scratch PDO count for discovery 


        

<hr>



### variable pdo\_entry\_idx 

```C++
uint8_t SYN_EcatMaster::pdo_entry_idx;
```



Scratch PDO entry loop index 


        

<hr>



### variable pdo\_map\_idx 

```C++
uint16_t SYN_EcatMaster::pdo_map_idx;
```



Scratch PDO mapping index 


        

<hr>



### variable pdo\_read\_len 

```C++
size_t SYN_EcatMaster::pdo_read_len;
```



Scratch SDO read length 


        

<hr>



### variable pdo\_sdo\_val 

```C++
uint32_t SYN_EcatMaster::pdo_sdo_val;
```



Scratch SDO read value 


        

<hr>



### variable poll\_retries 

```C++
uint16_t SYN_EcatMaster::poll_retries;
```



Retry counter for hardware status polling loops 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_EcatMaster::rx_buf;
```



Pointer to static receive frame buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
uint16_t SYN_EcatMaster::rx_buf_size;
```



Receive buffer capacity 


        

<hr>



### variable rx\_frame\_len 

```C++
uint16_t SYN_EcatMaster::rx_frame_len;
```



Current received frame length 


        

<hr>



### variable slave\_count 

```C++
uint8_t SYN_EcatMaster::slave_count;
```



Discovered slave count 


        

<hr>



### variable slaves 

```C++
SYN_EcatSlaveInfo SYN_EcatMaster::slaves[SYN_ECAT_MAX_SLAVES];
```



Slave descriptor table 


        

<hr>



### variable sub\_pt 

```C++
SYN_PT SYN_EcatMaster::sub_pt;
```



Sub-protothread handle for nested tasks 


        

<hr>



### variable total\_rx\_bits 

```C++
uint32_t SYN_EcatMaster::total_rx_bits;
```



Accumulated RxPDO bits 


        

<hr>



### variable total\_tx\_bits 

```C++
uint32_t SYN_EcatMaster::total_tx_bits;
```



Accumulated TxPDO bits 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_EcatMaster::tx_buf;
```



Pointer to static transmit frame buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
uint16_t SYN_EcatMaster::tx_buf_size;
```



Transmit buffer capacity 


        

<hr>



### variable tx\_frame\_len 

```C++
uint16_t SYN_EcatMaster::tx_frame_len;
```



Current pending transmit frame length 


        

<hr>



### variable wkc\_errors 

```C++
uint32_t SYN_EcatMaster::wkc_errors;
```



Working Counter error accumulator 


        

<hr>



### variable wkc\_expected 

```C++
uint16_t SYN_EcatMaster::wkc_expected;
```



Expected Working Counter for cyclic frame 


        

<hr>



### variable wkc\_last 

```C++
uint16_t SYN_EcatMaster::wkc_last;
```



Last received Working Counter 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

