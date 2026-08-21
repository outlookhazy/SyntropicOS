

# Struct SYN\_AutoTune



[**ClassList**](annotated.md) **>** [**SYN\_AutoTune**](structSYN__AutoTune.md)



_Auto-tuner instance._ 

* `#include <syn_autotune.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_AutoTune\_AbortReason**](syn__autotune_8h.md#enum-syn_autotune_abortreason) | [**abort\_reason**](#variable-abort_reason)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**above\_setpoint**](#variable-above_setpoint)  <br> |
|  uint8\_t | [**amplitude\_count**](#variable-amplitude_count)  <br> |
|  int32\_t | [**amplitude\_sum**](#variable-amplitude_sum)  <br> |
|  [**SYN\_AutoTune\_Config**](structSYN__AutoTune__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  [**SYN\_MotorCtrl**](structSYN__MotorCtrl.md) \* | [**ctrl**](#variable-ctrl)  <br> |
|  int32\_t | [**current\_output**](#variable-current_output)  <br> |
|  uint8\_t | [**half\_cycles**](#variable-half_cycles)  <br> |
|  int32\_t | [**history\_v**](#variable-history_v)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ka\_p1\_captured**](#variable-ka_p1_captured)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ka\_p2\_captured**](#variable-ka_p2_captured)  <br> |
|  uint32\_t | [**ka\_t1**](#variable-ka_t1)  <br> |
|  uint32\_t | [**ka\_t2**](#variable-ka_t2)  <br> |
|  int32\_t | [**ka\_v1**](#variable-ka_v1)  <br> |
|  int32\_t | [**ka\_v2**](#variable-ka_v2)  <br> |
|  uint32\_t | [**last\_check\_tick**](#variable-last_check_tick)  <br> |
|  uint32\_t | [**last\_cross\_tick**](#variable-last_cross_tick)  <br> |
|  uint32\_t | [**last\_update\_tick**](#variable-last_update_tick)  <br> |
|  int32\_t | [**osc\_peak\_neg**](#variable-osc_peak_neg)  <br> |
|  int32\_t | [**osc\_peak\_pos**](#variable-osc_peak_pos)  <br> |
|  uint8\_t | [**period\_count**](#variable-period_count)  <br> |
|  uint32\_t | [**period\_sum**](#variable-period_sum)  <br> |
|  uint32\_t | [**phase\_start\_tick**](#variable-phase_start_tick)  <br> |
|  int32\_t | [**relay\_output**](#variable-relay_output)  <br> |
|  [**SYN\_AutoTune\_Result**](structSYN__AutoTune__Result.md) | [**result**](#variable-result)  <br> |
|  int32\_t | [**start\_output**](#variable-start_output)  <br> |
|  int32\_t | [**start\_position**](#variable-start_position)  <br> |
|  [**SYN\_AutoTune\_State**](syn__autotune_8h.md#enum-syn_autotune_state) | [**state**](#variable-state)  <br> |
|  uint32\_t | [**velocity\_samples**](#variable-velocity_samples)  <br> |
|  int64\_t | [**velocity\_sum**](#variable-velocity_sum)  <br> |












































## Public Attributes Documentation




### variable abort\_reason 

```C++
SYN_AutoTune_AbortReason SYN_AutoTune::abort_reason;
```



Reason if aborted 
 


        

<hr>



### variable above\_setpoint 

```C++
bool SYN_AutoTune::above_setpoint;
```



Current side of setpoint 
 


        

<hr>



### variable amplitude\_count 

```C++
uint8_t SYN_AutoTune::amplitude_count;
```



Number of amplitude samples 


        

<hr>



### variable amplitude\_sum 

```C++
int32_t SYN_AutoTune::amplitude_sum;
```



Sum of oscillation amps 
 


        

<hr>



### variable cfg 

```C++
SYN_AutoTune_Config SYN_AutoTune::cfg;
```



Configuration snapshot 
 


        

<hr>



### variable ctrl 

```C++
SYN_MotorCtrl* SYN_AutoTune::ctrl;
```



Controlled motor 
 


        

<hr>



### variable current\_output 

```C++
int32_t SYN_AutoTune::current_output;
```



Current applied output percentage 
 


        

<hr>



### variable half\_cycles 

```C++
uint8_t SYN_AutoTune::half_cycles;
```



Half-cycle counter 
 


        

<hr>



### variable history\_v 

```C++
int32_t SYN_AutoTune::history_v;
```



Velocity sample from previous check 
 


        

<hr>



### variable ka\_p1\_captured 

```C++
bool SYN_AutoTune::ka_p1_captured;
```



Point 1 captured 
 


        

<hr>



### variable ka\_p2\_captured 

```C++
bool SYN_AutoTune::ka_p2_captured;
```



Point 2 captured 
 


        

<hr>



### variable ka\_t1 

```C++
uint32_t SYN_AutoTune::ka_t1;
```



Initial tick for Ka identification 
 


        

<hr>



### variable ka\_t2 

```C++
uint32_t SYN_AutoTune::ka_t2;
```



Final tick for Ka identification 
 


        

<hr>



### variable ka\_v1 

```C++
int32_t SYN_AutoTune::ka_v1;
```



Initial velocity for Ka identification 


        

<hr>



### variable ka\_v2 

```C++
int32_t SYN_AutoTune::ka_v2;
```



Final velocity for Ka identification 
 


        

<hr>



### variable last\_check\_tick 

```C++
uint32_t SYN_AutoTune::last_check_tick;
```



Tick of last steady-state check 
 


        

<hr>



### variable last\_cross\_tick 

```C++
uint32_t SYN_AutoTune::last_cross_tick;
```



Last zero-crossing tick 
 


        

<hr>



### variable last\_update\_tick 

```C++
uint32_t SYN_AutoTune::last_update_tick;
```



Tick count of last update() call 
 


        

<hr>



### variable osc\_peak\_neg 

```C++
int32_t SYN_AutoTune::osc_peak_neg;
```



Min position in half-cycle 


        

<hr>



### variable osc\_peak\_pos 

```C++
int32_t SYN_AutoTune::osc_peak_pos;
```



Max position in half-cycle 


        

<hr>



### variable period\_count 

```C++
uint8_t SYN_AutoTune::period_count;
```



Number of full periods 
 


        

<hr>



### variable period\_sum 

```C++
uint32_t SYN_AutoTune::period_sum;
```



Sum of full periods (ms) 
 


        

<hr>



### variable phase\_start\_tick 

```C++
uint32_t SYN_AutoTune::phase_start_tick;
```



Tick count when current phase started 


        

<hr>



### variable relay\_output 

```C++
int32_t SYN_AutoTune::relay_output;
```



Current relay sign × amp 
 


        

<hr>



### variable result 

```C++
SYN_AutoTune_Result SYN_AutoTune::result;
```



Computed results 
 


        

<hr>



### variable start\_output 

```C++
int32_t SYN_AutoTune::start_output;
```



Output at start of ramp-down 
 


        

<hr>



### variable start\_position 

```C++
int32_t SYN_AutoTune::start_position;
```



Position when tuning started 


        

<hr>



### variable state 

```C++
SYN_AutoTune_State SYN_AutoTune::state;
```



Current state machine state 
 


        

<hr>



### variable velocity\_samples 

```C++
uint32_t SYN_AutoTune::velocity_samples;
```



Number of velocity samples 


        

<hr>



### variable velocity\_sum 

```C++
int64_t SYN_AutoTune::velocity_sum;
```



Accumulated velocity sum 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_autotune.h`

