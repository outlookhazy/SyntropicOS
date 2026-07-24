

# Struct SYN\_DALI\_SlaveState



[**ClassList**](annotated.md) **>** [**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md)



_DALI Slave Runtime State._ 

* `#include <syn_dali.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**actual\_level**](#variable-actual_level)  <br> |
|  [**SYN\_DALI\_SlaveConfig**](structSYN__DALI__SlaveConfig.md) | [**cfg**](#variable-cfg)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**control\_gear\_failure**](#variable-control_gear_failure)  <br> |
|  uint8\_t | [**dtr0**](#variable-dtr0)  <br> |
|  uint8\_t | [**dtr1**](#variable-dtr1)  <br> |
|  uint8\_t | [**dtr2**](#variable-dtr2)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialise\_active**](#variable-initialise_active)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**lamp\_failure**](#variable-lamp_failure)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**lamp\_on**](#variable-lamp_on)  <br> |
|  uint32\_t | [**random\_address**](#variable-random_address)  <br> |
|  uint8\_t | [**scenes**](#variable-scenes)  <br> |
|  uint32\_t | [**search\_address**](#variable-search_address)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**write\_mem\_enabled**](#variable-write_mem_enabled)  <br> |












































## Public Attributes Documentation




### variable actual\_level 

```C++
uint8_t SYN_DALI_SlaveState::actual_level;
```



Current arc power level (0..254) 


        

<hr>



### variable cfg 

```C++
SYN_DALI_SlaveConfig SYN_DALI_SlaveState::cfg;
```




<hr>



### variable control\_gear\_failure 

```C++
bool SYN_DALI_SlaveState::control_gear_failure;
```



Gear failure flag 


        

<hr>



### variable dtr0 

```C++
uint8_t SYN_DALI_SlaveState::dtr0;
```



Data Transfer Register 0 


        

<hr>



### variable dtr1 

```C++
uint8_t SYN_DALI_SlaveState::dtr1;
```



Data Transfer Register 1 


        

<hr>



### variable dtr2 

```C++
uint8_t SYN_DALI_SlaveState::dtr2;
```



Data Transfer Register 2 


        

<hr>



### variable initialise\_active 

```C++
bool SYN_DALI_SlaveState::initialise_active;
```



True when initialisation active 


        

<hr>



### variable lamp\_failure 

```C++
bool SYN_DALI_SlaveState::lamp_failure;
```



Lamp failure flag 


        

<hr>



### variable lamp\_on 

```C++
bool SYN_DALI_SlaveState::lamp_on;
```



Lamp status flag 


        

<hr>



### variable random\_address 

```C++
uint32_t SYN_DALI_SlaveState::random_address;
```



24-bit Random Search Address 


        

<hr>



### variable scenes 

```C++
uint8_t SYN_DALI_SlaveState::scenes[16];
```



Scene levels 0..15 (255=MASK) 


        

<hr>



### variable search\_address 

```C++
uint32_t SYN_DALI_SlaveState::search_address;
```



24-bit Search Address comparator 


        

<hr>



### variable write\_mem\_enabled 

```C++
bool SYN_DALI_SlaveState::write_mem_enabled;
```



True when write memory enabled 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dali.h`

