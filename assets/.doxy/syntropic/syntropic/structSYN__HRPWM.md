

# Struct SYN\_HRPWM



[**ClassList**](annotated.md) **>** [**SYN\_HRPWM**](structSYN__HRPWM.md)



_High-Resolution PWM handle. Caller allocates; zero heap._ 

* `#include <syn_hrpwm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**channel**](#variable-channel)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enabled**](#variable-enabled)  <br> |
|  uint16\_t | [**fall\_ns**](#variable-fall_ns)  <br> |
|  uint32\_t | [**freq\_hz**](#variable-freq_hz)  <br> |
|  uint16\_t | [**phase\_deg**](#variable-phase_deg)  <br> |
|  uint16\_t | [**rise\_ns**](#variable-rise_ns)  <br> |












































## Public Attributes Documentation




### variable channel 

```C++
uint8_t SYN_HRPWM::channel;
```



Platform channel index 


        

<hr>



### variable enabled 

```C++
bool SYN_HRPWM::enabled;
```



Output state flag 


        

<hr>



### variable fall\_ns 

```C++
uint16_t SYN_HRPWM::fall_ns;
```



Fall dead-time in nanoseconds 


        

<hr>



### variable freq\_hz 

```C++
uint32_t SYN_HRPWM::freq_hz;
```



Switching frequency in Hz 


        

<hr>



### variable phase\_deg 

```C++
uint16_t SYN_HRPWM::phase_deg;
```



Phase offset in degrees 


        

<hr>



### variable rise\_ns 

```C++
uint16_t SYN_HRPWM::rise_ns;
```



Rise dead-time in nanoseconds 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_hrpwm.h`

