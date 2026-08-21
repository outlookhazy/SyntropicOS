

# Struct SYN\_BLDC\_Config



[**ClassList**](annotated.md) **>** [**SYN\_BLDC\_Config**](structSYN__BLDC__Config.md)



[More...](#detailed-description)

* `#include <syn_bldc_6step.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**pole\_pairs**](#variable-pole_pairs)  <br> |
|  uint16\_t | [**pwm\_frequency**](#variable-pwm_frequency)  <br> |












































## Detailed Description


Configuration parameters for 6-Step BLDC driver. 


    
## Public Attributes Documentation




### variable pole\_pairs 

```C++
uint8_t SYN_BLDC_Config::pole_pairs;
```



Number of motor rotor pole pairs (default: 4). 


        

<hr>



### variable pwm\_frequency 

```C++
uint16_t SYN_BLDC_Config::pwm_frequency;
```



PWM carrier frequency in Hz (informative/diagnostic). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_bldc_6step.h`

