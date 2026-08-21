

# Struct SYN\_BLDC\_6Step



[**ClassList**](annotated.md) **>** [**SYN\_BLDC\_6Step**](structSYN__BLDC__6Step.md)



[More...](#detailed-description)

* `#include <syn_bldc_6step.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_BLDC\_Config**](structSYN__BLDC__Config.md) | [**config**](#variable-config)  <br> |
|  uint8\_t | [**current\_step**](#variable-current_step)  <br> |
|  [**SYN\_BLDC\_Direction**](syn__bldc__6step_8h.md#enum-syn_bldc_direction) | [**direction**](#variable-direction)  <br> |
|  uint16\_t | [**duty**](#variable-duty)  <br> |
|  uint8\_t | [**hall\_state**](#variable-hall_state)  <br> |
|  uint32\_t | [**hall\_transitions**](#variable-hall_transitions)  <br> |
|  uint32\_t | [**last\_hall\_tick\_ms**](#variable-last_hall_tick_ms)  <br> |
|  uint32\_t | [**rpm**](#variable-rpm)  <br> |
|  [**SYN\_PID**](structSYN__PID.md) | [**speed\_pid**](#variable-speed_pid)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**speed\_pid\_active**](#variable-speed_pid_active)  <br> |
|  [**SYN\_BLDC\_State**](syn__bldc__6step_8h.md#enum-syn_bldc_state) | [**state**](#variable-state)  <br> |












































## Detailed Description


6-Step BLDC Motor Instance. 


    
## Public Attributes Documentation




### variable config 

```C++
SYN_BLDC_Config SYN_BLDC_6Step::config;
```



Configuration snapshot. 


        

<hr>



### variable current\_step 

```C++
uint8_t SYN_BLDC_6Step::current_step;
```



Active 6-step commutation step (1..6). 


        

<hr>



### variable direction 

```C++
SYN_BLDC_Direction SYN_BLDC_6Step::direction;
```



Requested rotation direction. 


        

<hr>



### variable duty 

```C++
uint16_t SYN_BLDC_6Step::duty;
```



Target PWM duty cycle (0..1000). 


        

<hr>



### variable hall\_state 

```C++
uint8_t SYN_BLDC_6Step::hall_state;
```



Last sampled 3-bit Hall sensor state (1..6). 


        

<hr>



### variable hall\_transitions 

```C++
uint32_t SYN_BLDC_6Step::hall_transitions;
```



Total accumulated Hall edge transitions. 


        

<hr>



### variable last\_hall\_tick\_ms 

```C++
uint32_t SYN_BLDC_6Step::last_hall_tick_ms;
```



Timestamp of last Hall transition. 


        

<hr>



### variable rpm 

```C++
uint32_t SYN_BLDC_6Step::rpm;
```



Calculated motor speed in RPM. 


        

<hr>



### variable speed\_pid 

```C++
SYN_PID SYN_BLDC_6Step::speed_pid;
```



Integrated speed PID controller instance. 


        

<hr>



### variable speed\_pid\_active 

```C++
bool SYN_BLDC_6Step::speed_pid_active;
```



True if PID closed-loop speed control is enabled. 


        

<hr>



### variable state 

```C++
SYN_BLDC_State SYN_BLDC_6Step::state;
```



Current driver state. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_bldc_6step.h`

