

# Struct SYN\_LIN\_Slave



[**ClassList**](annotated.md) **>** [**SYN\_LIN\_Slave**](structSYN__LIN__Slave.md)



_LIN Slave state machine handle._ 

* `#include <syn_lin.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_LIN\_ChecksumMode**](syn__lin_8h.md#enum-syn_lin_checksummode) | [**expected\_checksum\_mode**](#variable-expected_checksum_mode)  <br> |
|  uint8\_t | [**expected\_len**](#variable-expected_len)  <br> |
|  size\_t | [**frame\_count**](#variable-frame_count)  <br> |
|  [**SYN\_LIN\_SlaveFrame**](structSYN__LIN__SlaveFrame.md) | [**frames**](#variable-frames)  <br> |
|  uint8\_t | [**nad**](#variable-nad)  <br> |
|  [**SYN\_LIN\_Frame**](structSYN__LIN__Frame.md) | [**rx\_frame**](#variable-rx_frame)  <br> |
|  uint8\_t | [**rx\_idx**](#variable-rx_idx)  <br> |
|  [**SYN\_LIN\_State**](syn__lin_8h.md#enum-syn_lin_state) | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable expected\_checksum\_mode 

```C++
SYN_LIN_ChecksumMode SYN_LIN_Slave::expected_checksum_mode;
```



Active checksum mode 


        

<hr>



### variable expected\_len 

```C++
uint8_t SYN_LIN_Slave::expected_len;
```



Expected payload length for active frame 


        

<hr>



### variable frame\_count 

```C++
size_t SYN_LIN_Slave::frame_count;
```



Number of configured frame filters 


        

<hr>



### variable frames 

```C++
SYN_LIN_SlaveFrame SYN_LIN_Slave::frames[SYN_LIN_SLAVE_MAX_FRAMES];
```



Configured frame filter table 


        

<hr>



### variable nad 

```C++
uint8_t SYN_LIN_Slave::nad;
```



LIN Node Address (for 0x3C/0x3D diagnostics) 


        

<hr>



### variable rx\_frame 

```C++
SYN_LIN_Frame SYN_LIN_Slave::rx_frame;
```



Scratch RX frame buffer 


        

<hr>



### variable rx\_idx 

```C++
uint8_t SYN_LIN_Slave::rx_idx;
```



Current payload byte index 


        

<hr>



### variable state 

```C++
SYN_LIN_State SYN_LIN_Slave::state;
```



Current byte stream parser state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lin.h`

