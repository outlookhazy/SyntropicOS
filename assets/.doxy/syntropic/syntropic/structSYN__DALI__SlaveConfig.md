

# Struct SYN\_DALI\_SlaveConfig



[**ClassList**](annotated.md) **>** [**SYN\_DALI\_SlaveConfig**](structSYN__DALI__SlaveConfig.md)



_DALI Slave Configuration._ 

* `#include <syn_dali.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**fade\_rate**](#variable-fade_rate)  <br> |
|  uint8\_t | [**fade\_time**](#variable-fade_time)  <br> |
|  uint16\_t | [**group\_mask**](#variable-group_mask)  <br> |
|  uint8\_t | [**max\_level**](#variable-max_level)  <br> |
|  uint8\_t | [**min\_level**](#variable-min_level)  <br> |
|  uint8\_t | [**power\_on\_level**](#variable-power_on_level)  <br> |
|  uint8\_t | [**short\_address**](#variable-short_address)  <br> |
|  uint8\_t | [**system\_failure\_level**](#variable-system_failure_level)  <br> |












































## Public Attributes Documentation




### variable fade\_rate 

```C++
uint8_t SYN_DALI_SlaveConfig::fade_rate;
```



Fade rate setting (1..15) 


        

<hr>



### variable fade\_time 

```C++
uint8_t SYN_DALI_SlaveConfig::fade_time;
```



Fade time setting (1..15) 


        

<hr>



### variable group\_mask 

```C++
uint16_t SYN_DALI_SlaveConfig::group_mask;
```



Bitmask of assigned Groups (0..15) 


        

<hr>



### variable max\_level 

```C++
uint8_t SYN_DALI_SlaveConfig::max_level;
```



Maximum physical arc level (1..254) 


        

<hr>



### variable min\_level 

```C++
uint8_t SYN_DALI_SlaveConfig::min_level;
```



Minimum physical arc level (1..254) 


        

<hr>



### variable power\_on\_level 

```C++
uint8_t SYN_DALI_SlaveConfig::power_on_level;
```



Power-on arc level (0..254, 255=MASK) 


        

<hr>



### variable short\_address 

```C++
uint8_t SYN_DALI_SlaveConfig::short_address;
```



Short Address (0..63 or 0xFF) 


        

<hr>



### variable system\_failure\_level 

```C++
uint8_t SYN_DALI_SlaveConfig::system_failure_level;
```



Failure arc level (0..254, 255=MASK) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dali.h`

