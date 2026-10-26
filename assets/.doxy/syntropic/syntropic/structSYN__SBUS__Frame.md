

# Struct SYN\_SBUS\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_SBUS\_Frame**](structSYN__SBUS__Frame.md)



[More...](#detailed-description)

* `#include <syn_sbus.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ch17**](#variable-ch17)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ch18**](#variable-ch18)  <br> |
|  uint16\_t | [**channels**](#variable-channels)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**failsafe**](#variable-failsafe)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**frame\_loss**](#variable-frame_loss)  <br> |












































## Detailed Description


Parsed SBUS Frame structure. 


    
## Public Attributes Documentation




### variable ch17 

```C++
bool SYN_SBUS_Frame::ch17;
```



Digital Channel 17 flag. 


        

<hr>



### variable ch18 

```C++
bool SYN_SBUS_Frame::ch18;
```



Digital Channel 18 flag. 


        

<hr>



### variable channels 

```C++
uint16_t SYN_SBUS_Frame::channels[SYN_SBUS_NUM_CHANNELS];
```



16 11-bit channel values (172..1811 raw). 


        

<hr>



### variable failsafe 

```C++
bool SYN_SBUS_Frame::failsafe;
```



True if receiver entered failsafe state. 


        

<hr>



### variable frame\_loss 

```C++
bool SYN_SBUS_Frame::frame_loss;
```



True if receiver detected frame loss. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_sbus.h`

