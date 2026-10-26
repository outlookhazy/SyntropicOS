

# Struct SYN\_CCP\_DAQList



[**ClassList**](annotated.md) **>** [**SYN\_CCP\_DAQList**](structSYN__CCP__DAQList.md)



_DAQ List structure._ 

* `#include <syn_ccp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**current\_entry\_idx**](#variable-current_entry_idx)  <br> |
|  uint8\_t | [**current\_odt\_idx**](#variable-current_odt_idx)  <br> |
|  uint16\_t | [**cycle\_counter**](#variable-cycle_counter)  <br> |
|  uint8\_t | [**event\_channel**](#variable-event_channel)  <br> |
|  uint8\_t | [**odt\_count**](#variable-odt_count)  <br> |
|  [**SYN\_CCP\_ODT**](structSYN__CCP__ODT.md) | [**odts**](#variable-odts)  <br> |
|  uint16\_t | [**prescaler**](#variable-prescaler)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**running**](#variable-running)  <br> |












































## Public Attributes Documentation




### variable current\_entry\_idx 

```C++
uint8_t SYN_CCP_DAQList::current_entry_idx;
```



Current entry index 


        

<hr>



### variable current\_odt\_idx 

```C++
uint8_t SYN_CCP_DAQList::current_odt_idx;
```



Current ODT index 


        

<hr>



### variable cycle\_counter 

```C++
uint16_t SYN_CCP_DAQList::cycle_counter;
```



Cycle counter 


        

<hr>



### variable event\_channel 

```C++
uint8_t SYN_CCP_DAQList::event_channel;
```



Event channel assignment 


        

<hr>



### variable odt\_count 

```C++
uint8_t SYN_CCP_DAQList::odt_count;
```



Configured ODT count 


        

<hr>



### variable odts 

```C++
SYN_CCP_ODT SYN_CCP_DAQList::odts[SYN_CCP_MAX_ODT_PER_DAQ];
```



Array of ODT blocks 


        

<hr>



### variable prescaler 

```C++
uint16_t SYN_CCP_DAQList::prescaler;
```



Transmission prescaler 


        

<hr>



### variable running 

```C++
bool SYN_CCP_DAQList::running;
```



DAQ list running state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ccp.h`

